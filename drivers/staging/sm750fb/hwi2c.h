#ifndef HWI2C_H__
#define HWI2C_H__

/* hwi2c functions */
int hw_i2c_init(unsigned char bus_speed_mode);
void hw_i2c_close(void);

unsigned char hw_i2c_read_reg(unsigned char addr, unsigned char reg);
int hw_i2c_write_reg(unsigned char addr, unsigned char reg, unsigned char data);
#endif
