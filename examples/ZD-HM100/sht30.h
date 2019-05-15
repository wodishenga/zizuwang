#ifndef SHT30_H_
#define SHT30_H_

//-- Defines -------------------------------------------------------------------
// CRC
#define SHT30_POLYNOMIAL 0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001
// ID register mask (bits 0...5 are SHT30-specific product code
#define SHT30_PRODUCT_ID_MASK	0x003F // SHT30-product ID = xxxx'xxxx'xx00'0111 (where x=unspecific information)

//-- Enumerations --------------------------------------------------------------
// Sensor Commands
typedef enum{
	CB_eSht30SoftReset = 0x30A2, // soft reset
	CB_eSht30MeasureTHSingleShotHighRepClockstr = 0x2C06, // meas. read T first, clock stretching disabled, high repeatibility
	CB_eSht30MeasureTHSingleShotMedRepClockstr = 0x2C0D, // meas. read T first, clock stretching disabled, high repeatibility
	CB_eSht30MeasureTHSingleShotLowRepClockstr = 0x2C10, // meas. read T first, clock stretching disabled, high repeatibility
	CB_eSht30MeasureTHSingleShotHighRep = 0x2400, // meas. read T first, clock stretching disabled, high repeatibility
	CB_eSht30MeasureTHSingleShotMedRep = 0x240B, // meas. read T first, clock stretching disabled, high repeatibility
	CB_eSht30MeasureTHSingleShotLowRep = 0x2416, // meas. read T first, clock stretching disabled, high repeatibility
}CB_eSht30Commands_t;

// I2C address
typedef enum{
	CB_eSht30I2CAddress = 0x44, // sensor I2C address (7-bit)
	CB_eSht30I2cAddressAndWriteBit = 0x88, // sensor I2C address + write bit
	CB_eSht30I2cAddressAndReadBit = 0x89 // sensor I2C address + read bit
}CB_eSht30I2cAddress_t;
//==============================================================================
void SHT30_Init(void);
int SHT30_GetTempAndHumi(void);

#endif
