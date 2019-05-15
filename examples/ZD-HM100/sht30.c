#include "sht30.h"
#include "contiki.h"
#include "ti-lib.h"
#include "dev/gpio-hal.h"
#include "lpm.h"
#include "driverlib/prcm.h"
#include <stdio.h>
#include "sht30-i2c.h"

#define SHT30_ADDRESS 0x44

/* Sensor selection/deselection */
#define SENSOR_SELECT()     board_i2c_select(BOARD_I2C_INTERFACE_0, SHT30_ADDRESS)
#define SENSOR_DESELECT()   board_i2c_deselect()

#define delay_ms(i)		(ti_lib_cpu_delay(12000 * (i)))
#define delay_us(i) 	(ti_lib_cpu_delay(12 * (i)))

int16_t temperatureValue = 0;
uint16_t humiditValue = 0;

int TemperaturePrintFloat(float value)
{
	int tmp,tmp1,result;
	tmp = (int)value;
	tmp1=(int)((value-tmp)*10)%10;
	//tmp2=(int)((value-tmp)*100)%10;
	result = (tmp + (float)tmp1/10)*10;
	printf("Temperature=%d.%d,result=%d\r\n",tmp,tmp1,result);
	return result;
}

int HumiditPrintFloat(float value)
{
	int tmp,tmp1,result;
	tmp = (int)value;
	tmp1=(int)((value-tmp)*10)%10;
	//tmp2=(int)((value-tmp)*100)%10;
	result = (tmp + (float)tmp1/10)*10;
	//printf("Humidit=%d.%d,result = %d\r\n",tmp,tmp1,result);
	return result;
}



float CB_SHT30_CalcTemperature(const uint16_t rawValue){
	// calculate temperature [Â°C]
	// T = -45 + 175 * rawValue / 2^16
	return 175 * (float)rawValue / 65536 - 45;

}

float CB_SHT30_CalcHumidity(const uint16_t rawValue){
	// calculate relative humidity [%RH]
	// RH = rawValue / 2^16 * 100
	return 100 * (float)rawValue / 65536;
}

void SHT30_Init(void)
{
	uint8_t i2cData[2];
	i2cData[0] = (CB_eSht30SoftReset >> 8);
	i2cData[1] = (0xFF & CB_eSht30SoftReset);

	SENSOR_SELECT();
	board_i2c_write(i2cData, 2);
	SENSOR_DESELECT();
	delay_ms(1000);
}

bool SHT30_CheckCRC(const uint8_t data[], const uint8_t nbrOfBytes, uint8_t const checksum){

	uint8_t bit; // bit mask
	uint8_t crc = 0xFF; // calculated checksum
	uint8_t byteCtr; // byte counter

	// calculates 8-Bit checksum with given polynomial
	for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
	{
		crc ^= (data[byteCtr]);
		for(bit = 8; bit > 0; --bit)
		{
			if(crc & 0x80) crc = (crc << 1) ^ SHT30_POLYNOMIAL;
			else crc = (crc << 1);
		}
	}

	// verify checksum
	if(crc != checksum) {
		return false;
	}else{
		return true;
	}
}


int SHT30_GetTempAndHumi(void){

	uint16_t rawValueTemp; // temperature raw value from sensor
	uint16_t rawValueHumi; // humidity raw value from sensor
	uint8_t i2cData[6];
	bool status = false;

	i2cData[0] = (CB_eSht30MeasureTHSingleShotHighRepClockstr >> 8);
	i2cData[1] = (0xFF & CB_eSht30MeasureTHSingleShotHighRepClockstr);

	SENSOR_SELECT();
	status = board_i2c_write(i2cData, 2);
	SENSOR_DESELECT();

	if(status){
		SENSOR_SELECT();
		status = board_i2c_read(i2cData, 6);
		SENSOR_DESELECT();
	}

	//printf("status = %d\n", status);
	rawValueTemp = ((uint16_t)(i2cData[0]) << 8) + (uint16_t)(i2cData[1]);
	rawValueHumi = ((uint16_t)(i2cData[3]) << 8) + (uint16_t)(i2cData[4]);
	printf("rawValueTemp: %d\r\n", rawValueTemp);
	//printf("rawValueHumi: %d\r\n", rawValueHumi);
	temperatureValue = TemperaturePrintFloat(CB_SHT30_CalcTemperature(rawValueTemp));
	humiditValue = HumiditPrintFloat(CB_SHT30_CalcHumidity(rawValueHumi));

	return status;
}
