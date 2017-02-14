#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/input.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/kthread.h>

#define BMC_ID1_REG 0x00
#define BMC_ID2_REG 0x01
#define BMC_ID3_REG 0x02
#define BMC_ID4_REG 0x03
#define BMC_SOFTOFF_RQ_REG 0x04
#define BMC_PWROFF_RQ_REG 0x05

#define BMC_ID1_VAL 0x49
#define BMC_ID2_VAL 0x54
#define BMC_ID3_VAL 0x58
#define BMC_ID4_VAL 0x32

#define POLL_JIFFIES 100

struct bmc_poll_data {
  struct i2c_client *c;
};

static struct i2c_driver mitx2_bmc_i2c_driver;
static struct input_dev *button_dev;
static struct bmc_poll_data poll_data;
static struct task_struct *polling_task;
static const char input_name[] = "BMC input dev";
static uint8_t prev_ret;

int
bmc_pwroff_rq(void) {
  int ret = 0;
  printk(KERN_INFO "Write reg BMC_PWROFF_RQ_REG\n");
  ret = i2c_smbus_write_byte_data(poll_data.c, BMC_PWROFF_RQ_REG, 0x01);
  printk(KERN_INFO "ret: %i\n", ret);
  return ret;
}
//EXPORT_SYMBOL(bmc_pwroff_rq);

int
pwroff_rq_poll_fn(void *data) {
  int ret;
  while (1) {
    if (kthread_should_stop()) {
      break;
    }
    //dev_info(&poll_data.c->dev, "Polling\n");
    ret = i2c_smbus_read_byte_data(poll_data.c, BMC_SOFTOFF_RQ_REG);
    //dev_info(&poll_data.c->dev, "Polling returned: %i\n", ret);
    if (prev_ret!=ret) {
      dev_info(&poll_data.c->dev, "key change [%i]\n", ret);
      if (ret < 0) {
        dev_err(&poll_data.c->dev, "Could not read register %x\n", BMC_SOFTOFF_RQ_REG);
        return -EIO;
      }
      if (ret >= 0) {
        if (ret != 0) {
          dev_info(&poll_data.c->dev, "PWROFF \"irq\" detected [%i]\n", ret);
          //input_report_key(button_dev, KEY_POWER, (ret ? 0xff : 0));
          input_event(button_dev, EV_KEY, KEY_POWER, 1);
        } else {
          input_event(button_dev, EV_KEY, KEY_POWER, 0);
        }
        input_sync(button_dev);
      }
    }
    prev_ret = ret;
    msleep(100);
  }
  do_exit(1);
  return 0;
}

static int mitx2_bmc_validate(struct i2c_client *client) {
  int ret = 0;
  int i = 0 ;
  static const uint8_t regs[] = {BMC_ID1_REG, BMC_ID2_REG, BMC_ID3_REG, BMC_ID4_REG};
  static const uint8_t vals[] = {BMC_ID1_VAL, BMC_ID2_VAL, BMC_ID3_VAL, BMC_ID4_VAL};
  for (i=0; i<ARRAY_SIZE(regs); i++) {
    ret = i2c_smbus_read_byte_data(client, regs[i]);
    if (ret < 0) {
      dev_err(&client->dev, "Could not read register %x\n", regs[i]);
      return -EIO;
    }
    if (ret != vals[i]) {
			dev_err(&client->dev, "Bad value [0x%02x] in register 0x%02x, should be [0x%02x]\n", ret, regs[i], vals[i]);

			return -ENODEV;
		}

  }
  //printk(KERN_INFO "Try write reg\n");
  //ret = i2c_smbus_write_byte_data(client, BMC_PWROFF_RQ_REG, 0x01);
  //printk(KERN_INFO "ret: %i\n", ret);
  printk(KERN_INFO "BMC seems to be valid\n");
  return 0;
}

static int mitx2_bmc_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
	/* static const struct regmap_config config = { */
	/* 	.reg_bits = 8, */
	/* 	.val_bits = 8, */
	/* }; */
  int err = 0;

  printk(KERN_INFO "mitx2 bmc probe\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

  err = mitx2_bmc_validate(client);
  if (err) {
    return err;
  }

  button_dev = input_allocate_device();
	if (!button_dev) {
		dev_err(&client->dev, "Not enough memory\n");
    return -ENOMEM;
	}

  button_dev->id.bustype = BUS_I2C;
  button_dev->dev.parent = &client->dev;
  button_dev->name = input_name;
  button_dev->phys = "bmc-input0";
	button_dev->evbit[0] = BIT_MASK(EV_KEY);
	button_dev->keybit[BIT_WORD(KEY_POWER)] = BIT_MASK(KEY_POWER);

	err = input_register_device(button_dev);
	if (err) {
		dev_err(&client->dev, "Failed to register device\n");
    input_free_device(button_dev);
    return err;
	}

  dev_info(&client->dev, "Starting polling thread\n");
  polling_task = kthread_run(pwroff_rq_poll_fn, NULL, "BMC poll task");

  poll_data.c = client;

	return 0;
}

static int mitx2_bmc_i2c_remove(struct i2c_client *client)
{
  kthread_stop(polling_task);
  input_unregister_device(button_dev);
	return 0;
}


#ifdef CONFIG_OF
static const struct of_device_id mitx2_bmc_of_match[] = {
	{ .compatible = "t-platforms,mitx2-bmc" },
	{}
};
MODULE_DEVICE_TABLE(of, mitx2_bmc_of_match);
#endif

static const struct i2c_device_id mitx2_bmc_i2c_id[] = {
	{ "mitx2_bmc", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mitx2_bmc_i2c_id);


static struct i2c_driver mitx2_bmc_i2c_driver = {
	.driver		= {
		.name	= "mitx2-bmc",
		.of_match_table = of_match_ptr(mitx2_bmc_of_match),
	},
	.probe		= mitx2_bmc_i2c_probe,
  .remove	  = mitx2_bmc_i2c_remove,
	.id_table	= mitx2_bmc_i2c_id,
};
module_i2c_driver(mitx2_bmc_i2c_driver);


MODULE_AUTHOR("Konstantin Kirik");
MODULE_DESCRIPTION("mITX2 BMC driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("serial:bmc");
