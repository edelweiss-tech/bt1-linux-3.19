#include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regmap.h>

#define BMC_ID1_REG 0x00
#define BMC_ID2_REG 0x01
#define BMC_ID3_REG 0x02
#define BMC_ID4_REG 0x03

#define BMC_ID1_VAL 0x49
#define BMC_ID2_VAL 0x54
#define BMC_ID3_VAL 0x58
#define BMC_ID4_VAL 0x32

static struct i2c_driver mitx2_bmc_i2c_driver;

static int mitx2_bmc_validate(struct i2c_client *client) {
  int ret = 0;
  int i = 0 ;
  static const uint8_t regs[] = {BMC_ID1_REG, BMC_ID2_REG, BMC_ID3_REG, BMC_ID4_REG};
  static const uint8_t vals[] = {BMC_ID1_VAL, BMC_ID2_VAL, BMC_ID3_VAL, BMC_ID4_VAL};
  for (i=0; i<ARRAY_SIZE(regs); i++) {
    ret = i2c_smbus_read_byte_data(client, regs[i]);
    
    if (ret < 0) {
      dev_err(&client->dev, "%s: could not read register %x\n", __func__, regs[i]);
    
      return -EIO;
    }
    if (ret != vals[i+1]) {
			dev_err(&client->dev, "%s: bad value [0x%02x] in register 0x%02x, should be [0x%02x]", __func__, regs[i], ret, vals[i]);

			return -ENODEV;
		}

  }
  printk(KERN_INFO "BMC seems to be valid\n");
  return 0;
}

static int mitx2_bmc_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
	static const struct regmap_config config = {
		.reg_bits = 8,
		.val_bits = 8,
	};
  int err = 0;

  printk(KERN_INFO "mitx2 bmc probe\n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

  err = mitx2_bmc_validate(client);
  if (err) {
    return err;
  }

	return 0;
}

static int mitx2_bmc_i2c_remove(struct i2c_client *client)
{
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
