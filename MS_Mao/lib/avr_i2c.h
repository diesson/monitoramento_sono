#ifndef I2C_H_
#define I2C_H_

#define HMC5883L_ADDR 0x3C
#define ADXL345_ADDR 0xA6
#define L3G4200D_ADDR 0xD2
#define BMP085_ADDR 0xEE

void i2c_init();
uint8_t i2c_read(uint8_t addr, uint8_t i2c_addr);
void i2c_write(uint8_t addr, uint8_t data, uint8_t i2c_addr);

#endif /* I2C_H_ */
