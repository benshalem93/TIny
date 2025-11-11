/*
 * BMA530 Accelerometer Driver Header
 * Bosch Sensortec BMA530 3-Axis Accelerometer
 */

#ifndef BMA530_H_
#define BMA530_H_

#include <stdint.h>
#include <stdbool.h>

/* BMA530 Register Addresses */
#define BMA530_REG_CHIP_ID              0x00
#define BMA530_REG_ERR_REG              0x01
#define BMA530_REG_STATUS               0x02

/* Accelerometer Data Registers */
#define BMA530_REG_ACC_DATA_X_LSB       0x0C
#define BMA530_REG_ACC_DATA_X_MSB       0x0D
#define BMA530_REG_ACC_DATA_Y_LSB       0x0E
#define BMA530_REG_ACC_DATA_Y_MSB       0x0F
#define BMA530_REG_ACC_DATA_Z_LSB       0x10
#define BMA530_REG_ACC_DATA_Z_MSB       0x11

/* Temperature Data Register */
#define BMA530_REG_TEMP_DATA            0x12

/* Configuration Registers */
#define BMA530_REG_ACC_CONF             0x20
#define BMA530_REG_ACC_RANGE            0x21

/* Interrupt Registers */
#define BMA530_REG_INT_CONF             0x1F
#define BMA530_REG_INT1_MAP             0x23
#define BMA530_REG_INT2_MAP             0x24

/* Control Registers */
#define BMA530_REG_PWR_CTRL             0x7D
#define BMA530_REG_CMD                  0x7E

/* BMA530 Chip ID Value */
#define BMA530_CHIP_ID                  0x43

/* SPI Read/Write Bit Masks */
#define BMA530_SPI_READ_MASK            0x80
#define BMA530_SPI_WRITE_MASK           0x7F

/* Power Control Register Settings */
#define BMA530_PWR_CTRL_ACC_ENABLE      0x04
#define BMA530_PWR_CTRL_ACC_DISABLE     0x00

/* Accelerometer Configuration - Output Data Rate (ODR) */
#define BMA530_ACC_ODR_0_78125_HZ       0x01
#define BMA530_ACC_ODR_1_5625_HZ        0x02
#define BMA530_ACC_ODR_3_125_HZ         0x03
#define BMA530_ACC_ODR_6_25_HZ          0x04
#define BMA530_ACC_ODR_12_5_HZ          0x05
#define BMA530_ACC_ODR_25_HZ            0x06
#define BMA530_ACC_ODR_50_HZ            0x07
#define BMA530_ACC_ODR_100_HZ           0x08
#define BMA530_ACC_ODR_200_HZ           0x09
#define BMA530_ACC_ODR_400_HZ           0x0A
#define BMA530_ACC_ODR_800_HZ           0x0B
#define BMA530_ACC_ODR_1600_HZ          0x0C

/* Accelerometer Range Settings */
#define BMA530_ACC_RANGE_2G             0x00
#define BMA530_ACC_RANGE_4G             0x01
#define BMA530_ACC_RANGE_8G             0x02
#define BMA530_ACC_RANGE_16G            0x03

/* Accelerometer Bandwidth Parameter */
#define BMA530_ACC_BWP_OSR4_AVG1        0x00
#define BMA530_ACC_BWP_OSR2_AVG2        0x10
#define BMA530_ACC_BWP_NORMAL_AVG4      0x20
#define BMA530_ACC_BWP_CIC_AVG8         0x30

/* Soft Reset Command */
#define BMA530_CMD_SOFT_RESET           0xB6

/* Data Structure for Accelerometer Data */
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} BMA530_AccelData;

/* Function Prototypes */

/**
 * @brief Initialize the BMA530 sensor
 * @return true if initialization successful, false otherwise
 */
bool BMA530_Init(void);

/**
 * @brief Read the BMA530 Chip ID
 * @param chipId Pointer to store the chip ID value
 * @return true if read successful, false otherwise
 */
bool BMA530_ReadChipID(uint8_t *chipId);

/**
 * @brief Perform soft reset of BMA530
 * @return true if reset successful, false otherwise
 */
bool BMA530_SoftReset(void);

/**
 * @brief Enable accelerometer measurement
 * @return true if successful, false otherwise
 */
bool BMA530_EnableAccelerometer(void);

/**
 * @brief Configure accelerometer settings
 * @param odr Output data rate setting
 * @param range Measurement range setting
 * @param bandwidth Bandwidth parameter setting
 * @return true if configuration successful, false otherwise
 */
bool BMA530_ConfigureAccel(uint8_t odr, uint8_t range, uint8_t bandwidth);

/**
 * @brief Read accelerometer data
 * @param accelData Pointer to structure to store acceleration data
 * @return true if read successful, false otherwise
 */
bool BMA530_ReadAccelData(BMA530_AccelData *accelData);

/**
 * @brief Read a single register from BMA530
 * @param regAddr Register address to read
 * @param data Pointer to store the read data
 * @return true if read successful, false otherwise
 */
bool BMA530_ReadRegister(uint8_t regAddr, uint8_t *data);

/**
 * @brief Write a single register to BMA530
 * @param regAddr Register address to write
 * @param data Data to write
 * @return true if write successful, false otherwise
 */
bool BMA530_WriteRegister(uint8_t regAddr, uint8_t data);

/**
 * @brief Read multiple registers from BMA530
 * @param regAddr Starting register address
 * @param data Pointer to buffer to store read data
 * @param length Number of bytes to read
 * @return true if read successful, false otherwise
 */
bool BMA530_ReadMultipleRegisters(uint8_t regAddr, uint8_t *data, uint8_t length);

#endif /* BMA530_H_ */
