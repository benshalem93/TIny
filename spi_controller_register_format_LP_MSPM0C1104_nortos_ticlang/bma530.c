/*
 * BMA530 Accelerometer Driver Implementation
 * Bosch Sensortec BMA530 3-Axis Accelerometer
 */

#include "bma530.h"
#include "ti_msp_dl_config.h"

/* Timeout value for SPI operations */
#define BMA530_SPI_TIMEOUT              1000000

/* Delay helper function (simple delay loop) */
static void BMA530_Delay_ms(uint32_t ms)
{
    /* Assuming 24MHz CPU clock, approximate delay */
    uint32_t cycles = ms * (24000000 / 1000);
    delay_cycles(cycles);
}

/* State machine variables for SPI operations */
typedef enum {
    BMA530_SPI_IDLE,
    BMA530_SPI_TX_ADDR,
    BMA530_SPI_RX_DATA,
    BMA530_SPI_TX_DATA,
    BMA530_SPI_BUSY
} BMA530_SPI_State;

static volatile BMA530_SPI_State gBMA530_State = BMA530_SPI_IDLE;
static volatile uint8_t gBMA530_RxBuffer[16];
static volatile uint8_t gBMA530_TxData;
static volatile uint8_t gBMA530_RxCount;
static volatile uint8_t gBMA530_RxIndex;
static volatile bool gBMA530_TxComplete;
static volatile bool gBMA530_RxComplete;

/**
 * @brief Read a single register from BMA530
 */
bool BMA530_ReadRegister(uint8_t regAddr, uint8_t *data)
{
    if (data == NULL) {
        return false;
    }

    return BMA530_ReadMultipleRegisters(regAddr, data, 1);
}

/**
 * @brief Read multiple registers from BMA530
 */
bool BMA530_ReadMultipleRegisters(uint8_t regAddr, uint8_t *data, uint8_t length)
{
    if (data == NULL || length == 0 || length > 16) {
        return false;
    }

    uint32_t timeout = BMA530_SPI_TIMEOUT;

    /* Wait for SPI to be idle */
    while (gBMA530_State != BMA530_SPI_IDLE && timeout > 0) {
        timeout--;
    }
    if (timeout == 0) {
        return false;
    }

    /* Set read bit (MSB = 1) */
    uint8_t readCmd = regAddr | BMA530_SPI_READ_MASK;

    /* Initialize state machine */
    gBMA530_State = BMA530_SPI_TX_ADDR;
    gBMA530_RxCount = length;
    gBMA530_RxIndex = 0;
    gBMA530_RxComplete = false;

    /* Clear and enable interrupts */
    DL_SPI_clearInterruptStatus(SPI_0_INST, DL_SPI_INTERRUPT_TX | DL_SPI_INTERRUPT_RX);
    DL_SPI_enableInterrupt(SPI_0_INST, DL_SPI_INTERRUPT_TX | DL_SPI_INTERRUPT_RX);

    /* Send register address */
    DL_SPI_transmitData8(SPI_0_INST, readCmd);

    /* Wait for transaction to complete */
    timeout = BMA530_SPI_TIMEOUT;
    while (!gBMA530_RxComplete && timeout > 0) {
        __WFI();
        timeout--;
    }

    /* Disable interrupts */
    DL_SPI_disableInterrupt(SPI_0_INST, DL_SPI_INTERRUPT_TX);
    DL_SPI_clearInterruptStatus(SPI_0_INST, DL_SPI_INTERRUPT_TX | DL_SPI_INTERRUPT_RX);

    gBMA530_State = BMA530_SPI_IDLE;

    if (timeout == 0) {
        return false;
    }

    /* Copy received data */
    for (uint8_t i = 0; i < length; i++) {
        data[i] = gBMA530_RxBuffer[i];
    }

    return true;
}

/**
 * @brief Write a single register to BMA530
 */
bool BMA530_WriteRegister(uint8_t regAddr, uint8_t data)
{
    uint32_t timeout = BMA530_SPI_TIMEOUT;

    /* Wait for SPI to be idle */
    while (gBMA530_State != BMA530_SPI_IDLE && timeout > 0) {
        timeout--;
    }
    if (timeout == 0) {
        return false;
    }

    /* Clear write bit (MSB = 0) */
    uint8_t writeCmd = regAddr & BMA530_SPI_WRITE_MASK;

    /* Initialize state machine */
    gBMA530_State = BMA530_SPI_TX_ADDR;
    gBMA530_TxData = data;
    gBMA530_TxComplete = false;

    /* Clear and enable interrupts */
    DL_SPI_clearInterruptStatus(SPI_0_INST, DL_SPI_INTERRUPT_TX | DL_SPI_INTERRUPT_RX);
    DL_SPI_enableInterrupt(SPI_0_INST, DL_SPI_INTERRUPT_TX | DL_SPI_INTERRUPT_RX);

    /* Send register address */
    DL_SPI_transmitData8(SPI_0_INST, writeCmd);

    /* Wait for transaction to complete */
    timeout = BMA530_SPI_TIMEOUT;
    while (!gBMA530_TxComplete && timeout > 0) {
        __WFI();
        timeout--;
    }

    /* Disable interrupts */
    DL_SPI_disableInterrupt(SPI_0_INST, DL_SPI_INTERRUPT_TX);
    DL_SPI_clearInterruptStatus(SPI_0_INST, DL_SPI_INTERRUPT_TX | DL_SPI_INTERRUPT_RX);

    gBMA530_State = BMA530_SPI_IDLE;

    return (timeout > 0);
}

/**
 * @brief Read the BMA530 Chip ID
 */
bool BMA530_ReadChipID(uint8_t *chipId)
{
    return BMA530_ReadRegister(BMA530_REG_CHIP_ID, chipId);
}

/**
 * @brief Perform soft reset of BMA530
 */
bool BMA530_SoftReset(void)
{
    if (!BMA530_WriteRegister(BMA530_REG_CMD, BMA530_CMD_SOFT_RESET)) {
        return false;
    }

    /* Wait for reset to complete */
    BMA530_Delay_ms(10);

    return true;
}

/**
 * @brief Enable accelerometer measurement
 */
bool BMA530_EnableAccelerometer(void)
{
    return BMA530_WriteRegister(BMA530_REG_PWR_CTRL, BMA530_PWR_CTRL_ACC_ENABLE);
}

/**
 * @brief Configure accelerometer settings
 */
bool BMA530_ConfigureAccel(uint8_t odr, uint8_t range, uint8_t bandwidth)
{
    /* Configure output data rate and bandwidth */
    uint8_t accConf = (bandwidth & 0xF0) | (odr & 0x0F);
    if (!BMA530_WriteRegister(BMA530_REG_ACC_CONF, accConf)) {
        return false;
    }

    /* Configure measurement range */
    if (!BMA530_WriteRegister(BMA530_REG_ACC_RANGE, range)) {
        return false;
    }

    /* Small delay for settings to take effect */
    BMA530_Delay_ms(1);

    return true;
}

/**
 * @brief Read accelerometer data
 */
bool BMA530_ReadAccelData(BMA530_AccelData *accelData)
{
    if (accelData == NULL) {
        return false;
    }

    uint8_t rawData[6];

    /* Read 6 bytes starting from X-axis LSB register */
    if (!BMA530_ReadMultipleRegisters(BMA530_REG_ACC_DATA_X_LSB, rawData, 6)) {
        return false;
    }

    /* Combine LSB and MSB for each axis */
    accelData->x = (int16_t)((rawData[1] << 8) | rawData[0]);
    accelData->y = (int16_t)((rawData[3] << 8) | rawData[2]);
    accelData->z = (int16_t)((rawData[5] << 8) | rawData[4]);

    return true;
}

/**
 * @brief Initialize the BMA530 sensor
 */
bool BMA530_Init(void)
{
    uint8_t chipId = 0;

    /* Small delay after power-up */
    BMA530_Delay_ms(10);

    /* Perform soft reset */
    if (!BMA530_SoftReset()) {
        return false;
    }

    /* Read and verify chip ID */
    if (!BMA530_ReadChipID(&chipId)) {
        return false;
    }

    if (chipId != BMA530_CHIP_ID) {
        return false;
    }

    /* Configure accelerometer: 100Hz ODR, 2G range, normal bandwidth */
    if (!BMA530_ConfigureAccel(BMA530_ACC_ODR_100_HZ,
                                BMA530_ACC_RANGE_2G,
                                BMA530_ACC_BWP_NORMAL_AVG4)) {
        return false;
    }

    /* Enable accelerometer */
    if (!BMA530_EnableAccelerometer()) {
        return false;
    }

    /* Wait for accelerometer to stabilize */
    BMA530_Delay_ms(50);

    return true;
}

/**
 * @brief SPI Interrupt Handler Extension for BMA530
 * This should be called from the main SPI_0_INST_IRQHandler
 */
void BMA530_SPI_IRQHandler(void)
{
    switch (DL_SPI_getPendingInterrupt(SPI_0_INST)) {
        case DL_SPI_IIDX_TX:
            if (gBMA530_State == BMA530_SPI_TX_ADDR) {
                /* Address sent, now send data or receive data */
                if (gBMA530_RxCount > 0) {
                    /* Read operation - send dummy byte */
                    gBMA530_State = BMA530_SPI_RX_DATA;
                    DL_SPI_transmitData8(SPI_0_INST, 0xFF);
                } else {
                    /* Write operation - send data byte */
                    gBMA530_State = BMA530_SPI_TX_DATA;
                    DL_SPI_transmitData8(SPI_0_INST, gBMA530_TxData);
                }
            } else if (gBMA530_State == BMA530_SPI_RX_DATA) {
                /* Continue sending dummy bytes to receive more data */
                if (gBMA530_RxIndex < gBMA530_RxCount) {
                    DL_SPI_transmitData8(SPI_0_INST, 0xFF);
                }
            } else if (gBMA530_State == BMA530_SPI_TX_DATA) {
                /* Write complete */
                gBMA530_TxComplete = true;
                gBMA530_State = BMA530_SPI_IDLE;
            }
            break;

        case DL_SPI_IIDX_RX:
            if (gBMA530_State == BMA530_SPI_TX_ADDR) {
                /* Discard dummy byte received after address */
                DL_SPI_receiveData8(SPI_0_INST);
            } else if (gBMA530_State == BMA530_SPI_RX_DATA) {
                /* Store received data */
                if (gBMA530_RxIndex < gBMA530_RxCount) {
                    gBMA530_RxBuffer[gBMA530_RxIndex++] = DL_SPI_receiveData8(SPI_0_INST);

                    if (gBMA530_RxIndex >= gBMA530_RxCount) {
                        /* All data received */
                        gBMA530_RxComplete = true;
                        gBMA530_State = BMA530_SPI_IDLE;
                    }
                }
            } else if (gBMA530_State == BMA530_SPI_TX_DATA) {
                /* Discard dummy byte received during write */
                DL_SPI_receiveData8(SPI_0_INST);
            }
            break;

        default:
            break;
    }
}
