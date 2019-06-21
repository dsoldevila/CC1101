/*
 * rf_driver.c
 *
 *  Created on: 29 mar. 2019
 *      Author: dsoldevila
 *      The following code was made by SpaceTeddy (https://github.com/SpaceTeddy/CC1101):
 *      	Patable presets
 *      	Modulation presets
 *      	rf_set_modulation_mode(mode);
			rf_set_ISMband(ism_band);
			rf_set_output_power_level(-30);
			wor_enable()
			Some enter x state functions (but they are so simple that it doesn't really matter)
		The following code is from EMCU.EU (http://www.emcu.eu/how-to-implement-printf-for-send-message-via-usb-on-stm32-nucleo-boards-using-atollic)
			_io_putchar()

		TODO NOTE: THE MODULATION PRESET GFSK_1_2KB HAS THE RIGHT VALUES. THE OTHER PRESETS MAY REQUIRE SOME MODIFICATION TO WORK.
		ONLY THE GFSK_1_2KB MODULATION HAS BEEN TESTED, DO NOT EXPECT OTHERS TO WORK.
 *
 */
//#define RF_DRIVER_H_
#include "../Inc/rf_driver.h"
#include "../Inc/dwt_stm32_delay.h"
#include <string.h>
#include <stdlib.h>

/* Private variables -------------------------------------------------------------*/
SPI_HandleTypeDef* hal_spi;
UART_HandleTypeDef* hal_uart;

uint16_t CS_Pin;
GPIO_TypeDef* CS_GPIO_Port;

uint16_t GDO0_Pin; //Board pin connected to CC1101's GDO0 pin

uint8_t GDO0_FLAG = 0; //Flag of the tx threshold interrupt. Used to control the reading and writting to the buffer and to check the
						//arrival of a packet.

uint32_t TimeOut = 0x6FF; //TODO It's hardcoded



//----------------------[PATABLES]---------------------------------------------
/*PATABLES Registers presets for various frequencies. This values are the (suposed) optimal values for -30, -20, -15,
-10, 0, 5, 7, 10 dBm for each carrier frequency.
*/
const uint8_t patable_power_315[]  = {0x17,0x1D,0x26,0x69,0x51,0x86,0xCC,0xC3};
const uint8_t patable_power_433[]  = {0x6C,0x1C,0x06,0x3A,0x51,0x85,0xC8,0xC0};
const uint8_t patable_power_868[]  = {0x03,0x17,0x1D,0x26,0x50,0x86,0xCD,0xC0};
const uint8_t patable_power_915[]  = {0x0B,0x1B,0x6D,0x67,0x50,0x85,0xC9,0xC1};


//----------------------[REGISTER BASIC CONFIGURATION]------------------------
//Preset for Gaussian Frequency Shift Keying mod at 1.2KBits/s
const uint8_t cc1100_GFSK_1_2_kb[] = {
                    0x07,  // IOCFG2        GDO2 Output Pin Configuration
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x80,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x07,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0x3E,  // PKTLEN        Packet Length
                    0xD8,  // PKTCTRL1      Packet Automation Control //TODO changed from DC to disable lqi and rssi appending
                    0x45,  // PKTCTRL0      Packet Automation Control
                    0xFF,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x08,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0xF5,  // MDMCFG4       Modem Configuration
                    0x83,  // MDMCFG3       Modem Configuration
                    0x13,  // MDMCFG2       Modem Configuration
                    0xC0,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x15,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x00,  // MCSM1         Main Radio Control State Machine Configuration //TODO was 0x0C
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x16,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x6C,  // BSCFG         Bit Synchronization Configuration
                    0x03,  // AGCCTRL2      AGC Control
                    0x40,  // AGCCTRL1      AGC Control
                    0x91,  // AGCCTRL0      AGC Control
                    0x02,  // WOREVT1       High Byte Event0 Timeout
                    0x26,  // WOREVT0       Low Byte Event0 Timeout
                    0x09,  // WORCTRL       Wake On Radio Control
                    0x56,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xA9,  // FSCAL3        Frequency Synthesizer Calibration
                    0x0A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x11,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control,
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x3F,  // TEST1         Various Test Settings
                    0x0B   // TEST0         Various Test Settings
                };

const uint8_t cc1100_GFSK_38_4_kb[] = {
                    0x07,  // IOCFG2        GDO2 Output Pin Configuration
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x80,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x07,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0x3E,  // PKTLEN        Packet Length
					0xDC,  // PKTCTRL1      Packet Automation Control
                    0x45,  // PKTCTRL0      Packet Automation Control
                    0xFF,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x06,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0xCA,  // MDMCFG4       Modem Configuration
                    0x83,  // MDMCFG3       Modem Configuration
                    0x13,  // MDMCFG2       Modem Configuration
                    0xA0,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x34,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x0C,  // MCSM1         Main Radio Control State Machine Configuration
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x16,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x6C,  // BSCFG         Bit Synchronization Configuration
                    0x43,  // AGCCTRL2      AGC Control
                    0x40,  // AGCCTRL1      AGC Control
                    0x91,  // AGCCTRL0      AGC Control
                    0x02,  // WOREVT1       High Byte Event0 Timeout
                    0x26,  // WOREVT0       Low Byte Event0 Timeout
                    0x09,  // WORCTRL       Wake On Radio Control
                    0x56,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xA9,  // FSCAL3        Frequency Synthesizer Calibration
                    0x0A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x11,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control,
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x3F,  // TEST1         Various Test Settings
                    0x0B   // TEST0         Various Test Settings
                };

const uint8_t cc1100_GFSK_100_kb[]  = {
                    0x07,  // IOCFG2        GDO2 Output Pin Configuration
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x80,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x07,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0x3E,  // PKTLEN        Packet Length
					0xDC,  // PKTCTRL1      Packet Automation Control
                    0x45,  // PKTCTRL0      Packet Automation Control
                    0xFF,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x08,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0x5B,  // MDMCFG4       Modem Configuration
                    0xF8,  // MDMCFG3       Modem Configuration
                    0x13,  // MDMCFG2       Modem Configuration
                    0xA0,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x47,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x0C,  // MCSM1         Main Radio Control State Machine Configuration
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x1D,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x1C,  // BSCFG         Bit Synchronization Configuration
                    0xC7,  // AGCCTRL2      AGC Control
                    0x00,  // AGCCTRL1      AGC Control
                    0xB2,  // AGCCTRL0      AGC Control
                    0x02,  // WOREVT1       High Byte Event0 Timeout
                    0x26,  // WOREVT0       Low Byte Event0 Timeout
                    0x09,  // WORCTRL       Wake On Radio Control
                    0xB6,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xEA,  // FSCAL3        Frequency Synthesizer Calibration
                    0x0A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x11,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control,
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x3F,  // TEST1         Various Test Settings
                    0x0B   // TEST0         Various Test Settings
                };

const uint8_t cc1100_MSK_250_kb[] = {
                    0x07,  // IOCFG2        GDO2 Output Pin Configuration
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x80,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x07,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0x3E,  // PKTLEN        Packet Length
					0xDC,  // PKTCTRL1      Packet Automation Control
                    0x45,  // PKTCTRL0      Packet Automation Control
                    0xFF,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x0B,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0x2D,  // MDMCFG4       Modem Configuration
                    0x3B,  // MDMCFG3       Modem Configuration
                    0x73,  // MDMCFG2       Modem Configuration
                    0xA0,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x00,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x0C,  // MCSM1         Main Radio Control State Machine Configuration
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x1D,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x1C,  // BSCFG         Bit Synchronization Configuration
                    0xC7,  // AGCCTRL2      AGC Control
                    0x00,  // AGCCTRL1      AGC Control
                    0xB2,  // AGCCTRL0      AGC Control
                    0x02,  // WOREVT1       High Byte Event0 Timeout
                    0x26,  // WOREVT0       Low Byte Event0 Timeout
                    0x09,  // WORCTRL       Wake On Radio Control
                    0xB6,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xEA,  // FSCAL3        Frequency Synthesizer Calibration
                    0x0A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x11,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control,
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x3F,  // TEST1         Various Test Settings
                    0x0B   // TEST0         Various Test Settings
                };

const uint8_t cc1100_MSK_500_kb[] = {
                    0x07,  // IOCFG2        GDO2 Output Pin Configuration
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x80,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x07,  // FIFOTHR       RX FIFO and TX FIFO Thresholds
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0x3E,  // PKTLEN        Packet Length
					0xDC,  // PKTCTRL1      Packet Automation Control
                    0x45,  // PKTCTRL0      Packet Automation Control
                    0xFF,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x0C,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0x0E,  // MDMCFG4       Modem Configuration
                    0x3B,  // MDMCFG3       Modem Configuration
                    0x73,  // MDMCFG2       Modem Configuration
                    0xA0,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x00,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x0C,  // MCSM1         Main Radio Control State Machine Configuration
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x1D,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x1C,  // BSCFG         Bit Synchronization Configuration
                    0xC7,  // AGCCTRL2      AGC Control
                    0x40,  // AGCCTRL1      AGC Control
                    0xB2,  // AGCCTRL0      AGC Control
                    0x02,  // WOREVT1       High Byte Event0 Timeout
                    0x26,  // WOREVT0       Low Byte Event0 Timeout
                    0x09,  // WORCTRL       Wake On Radio Control
                    0xB6,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xEA,  // FSCAL3        Frequency Synthesizer Calibration
                    0x0A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x19,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control,
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x3F,  // TEST1         Various Test Settings
                    0x0B   // TEST0         Various Test Settings
                };

const uint8_t cc1100_OOK_4_8_kb[] = { //In fact it's 2.4Kb/s because of the Manhattan codification, see Datasheet.
                    0x02,  // IOCFG2        GDO2 Output Pin Configuration //0x06 --> 0x02 (deasserts when below threshold)
                    0x2E,  // IOCFG1        GDO1 Output Pin Configuration
                    0x06,  // IOCFG0        GDO0_Pin Output Pin Configuration
                    0x48,  // FIFOTHR       RX FIFO and TX FIFO Thresholds //0x47 --> 0x48
                    0x57,  // SYNC1         Sync Word, High Byte
                    0x43,  // SYNC0         Sync Word, Low Byte
                    0xFF,  // PKTLEN        Packet Length
					0xDC,  // PKTCTRL1      Packet Automation Control
                    0x05,  // PKTCTRL0      Packet Automation Control
                    0x00,  // ADDR          Device Address
                    0x00,  // CHANNR        Channel Number
                    0x06,  // FSCTRL1       Frequency Synthesizer Control
                    0x00,  // FSCTRL0       Frequency Synthesizer Control
                    0x21,  // FREQ2         Frequency Control Word, High Byte
                    0x65,  // FREQ1         Frequency Control Word, Middle Byte
                    0x6A,  // FREQ0         Frequency Control Word, Low Byte
                    0x87,  // MDMCFG4       Modem Configuration
                    0x83,  // MDMCFG3       Modem Configuration
                    0x3B,  // MDMCFG2       Modem Configuration
                    0x22,  // MDMCFG1       Modem Configuration
                    0xF8,  // MDMCFG0       Modem Configuration
                    0x15,  // DEVIATN       Modem Deviation Setting
                    0x07,  // MCSM2         Main Radio Control State Machine Configuration
                    0x30,  // MCSM1         Main Radio Control State Machine Configuration
                    0x18,  // MCSM0         Main Radio Control State Machine Configuration
                    0x14,  // FOCCFG        Frequency Offset Compensation Configuration
                    0x6C,  // BSCFG         Bit Synchronization Configuration
                    0x07,  // AGCCTRL2      AGC Control
                    0x00,  // AGCCTRL1      AGC Control
                    0x92,  // AGCCTRL0      AGC Control
                    0x87,  // WOREVT1       High Byte Event0 Timeout
                    0x6B,  // WOREVT0       Low Byte Event0 Timeout
                    0xFB,  // WORCTRL       Wake On Radio Control
                    0x56,  // FREND1        Front End RX Configuration
                    0x17,  // FREND0        Front End TX Configuration
                    0xE9,  // FSCAL3        Frequency Synthesizer Calibration
                    0x2A,  // FSCAL2        Frequency Synthesizer Calibration
                    0x00,  // FSCAL1        Frequency Synthesizer Calibration
                    0x1F,  // FSCAL0        Frequency Synthesizer Calibration
                    0x41,  // RCCTRL1       RC Oscillator Configuration
                    0x00,  // RCCTRL0       RC Oscillator Configuration
                    0x59,  // FSTEST        Frequency Synthesizer Calibration Control
                    0x7F,  // PTEST         Production Test
                    0x3F,  // AGCTEST       AGC Test
                    0x81,  // TEST2         Various Test Settings
                    0x35,  // TEST1         Various Test Settings
                    0x09,  // TEST0         Various Test Settings
};
//----------------------[END REGISTER BASIC CONFIGURATION]--------------------

/* Private user code ---------------------------------------------------------*/

/* RF DRIVER ----------------------------------------------------------------------------------------------------------------------*/

/*--------------------------[CC1101 Init and Settings]------------------------------*/
uint8_t rf_begin(SPI_HandleTypeDef* hspi, MODULATION_TypeDef mode, ISMBAND_TypeDef ism_band, GPIO_TypeDef* cs_port, uint16_t cs_pin, uint16_t gdo0){
	/**
	 * @brief Calls all the functions needed to make the RF chip operative. This should be the first function used when
	 * using the RF chip.
	 * @param hspi: Pointer to the spi handler
	 * @param mode: Modulation used
	 * @param ism_band Frequency used
	 * @param cs_port: Chip Select (SPI) Pin Port (ie: GPIOD)
	 * @param cs_pin: Chip Select (SPI) Pin number (ie: GPIO_Pin_14)
	 * @param gdo0: Pin number of the pin connected to C1101 CGDO0, used for interruptions. Interruption is configured as FALLING EDGE.
	 *
	 */

	//Pinout linking
	hal_spi = hspi;
	CS_GPIO_Port = cs_port;
	CS_Pin = cs_pin;
	GDO0_Pin = gdo0;

	//Turn on the chip
	rf_reset();

	//Check that the SPI works
	if(!rf_check()){
		return FALSE;
	}


	rf_write_strobe(SFTX); //Flush TX FIFO
	HAL_Delay(1); //TODO I don't think this is really needed
	rf_write_strobe(SFRX); //Flush RX FIFO
	HAL_Delay(1);

	rf_set_modulation_mode(mode);

	rf_set_ISMband(ism_band);
	rf_set_channel(0);
	rf_set_output_power_level(-30);
	return TRUE;


}

void rf_reset(){
	/**
	 * @brief Turns on the RF chip with a specific sequence on the CS pin and a SRES command.
	 * The former is only needed on a cold start.
	 */
	DWT_Delay_Init();

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	DWT_Delay_us(10);

	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	DWT_Delay_us(40);

	rf_write_strobe(SRES);
	HAL_Delay(1);

}

uint8_t rf_check(){
	/**
	 * @brief Checks the version of the RF chip to check if SPI is OK. It checks 10 times to make sure wires are really OK.
	 */

	uint8_t ok = TRUE;
	uint8_t i;
	uint8_t version;
	for(i=0; i<10; i++){
		version = rf_read_register(VERSION);
		if(version!=0x14)
			ok = FALSE;
	}

	if (ok){
		printf("RF check: OK\n\r");
	}else{
		printf("RF check: No luck :(\n\r");
	}
	return ok;

}

void rf_set_modulation_mode(MODULATION_TypeDef mode){
	/*
	 * @brief Loads the wanted modulation preset to the CC1101.
	 */

    const uint8_t* cfg_reg;

    switch (mode)
    {
        case GFSK_1_2_kb:
        			cfg_reg = cc1100_GFSK_1_2_kb;
                    break;
        case GFSK_38_4_kb:
                    cfg_reg = cc1100_GFSK_38_4_kb;
                    break;
        case GFSK_100_kb:
        			cfg_reg = cc1100_GFSK_100_kb;
                    break;
        case MSK_250_kb:
        			cfg_reg = cc1100_MSK_250_kb;
                    break;
        case MSK_500_kb:
        			cfg_reg = cc1100_MSK_500_kb;
                    break;
        case OOK_4_8_kb:
        			cfg_reg = cc1100_OOK_4_8_kb;
                    break;
        default:
        			cfg_reg = cc1100_GFSK_38_4_kb;
                    break;
    }

    rf_write_data(WRITE_BURST(0), cfg_reg, CFG_REGISTER);                            //writes all 47 config register


}

//(Semi)DEPRECATED
void rf_set_ISMband(ISMBAND_TypeDef band){
	/*
	 * Deprecated by rf_set_frequency(float), although the second still doesn't configure the PATABLES registers, so it is still needed.
	 */
    uint8_t freq2, freq1, freq0;
    const uint8_t* patable;

    switch (band)
    {
        case MHz315:
                    freq2=0x0C;
                    freq1=0x1D;
                    freq0=0x89;
                    patable = patable_power_315;
                    break;
        case MHz434:                                                          //433.92MHz
                    freq2=0x10;
                    freq1=0xB0;
                    freq0=0x71;
                    patable = patable_power_433;
                    break;
        case MHz868:                                                          //868.3MHz
                    freq2=0x21;
                    freq1=0x65;
                    freq0=0x6A;
                    patable = patable_power_868;
                    break;
        case MHz915:
                    freq2=0x23;
                    freq1=0x31;
                    freq0=0x3B;
                    patable = patable_power_915;
                    break;
        default:                                                          //868.3MHz
					freq2=0x21;
					freq1=0x65;
					freq0=0x6A;
					patable = patable_power_868;
					break;
    }
    rf_write_register(FREQ2,freq2);
    rf_write_register(FREQ1,freq1);
    rf_write_register(FREQ0,freq0);
    rf_write_data(PATABLE_BURST, patable, 8);
}

void rf_set_channel(uint8_t channel){
	/*
	 * @brief Set channel number.
	 */
	rf_write_register(CHANNR, channel);
}

void rf_set_output_power_level(int8_t dBm)
/*
 * @brief Selects the entry of the PATABLES preset selected previously.
 */
{
    uint8_t pa = 0xC0;

    if      (dBm <= -30) pa = 0x00;
    else if (dBm <= -20) pa = 0x01;
    else if (dBm <= -15) pa = 0x02;
    else if (dBm <= -10) pa = 0x03;
    else if (dBm <= 0)   pa = 0x04;
    else if (dBm <= 5)   pa = 0x05;
    else if (dBm <= 7)   pa = 0x06;
    else if (dBm <= 10)  pa = 0x07;

    rf_write_register(FREND0,pa);
}

float rf_set_carrier_offset(float offset){
	/*
	 * @Brief Configures frequency offset register to achieve the tergeted offset.
	 * @param offset Desired offset. Should be between -200KHz and +200KHz, depends on crystal.
	 * @returns The actual offset
	 */
	//rf_write_register(FSCTRL0, offset);
	int8_t freqoff = offset*(1<<14)/CRYSTAL_FREQUENCY;
	rf_write_register(FSCTRL0, freqoff);
	return freqoff*(CRYSTAL_FREQUENCY/(1<<14));
}

float rf_set_carrier_frequency(float target_freq){
	/* Note that this functions depends on the value of CRYSTAL_FREQUENCY_M.
	 * @param target_freq Frequency targeted, in MHz. Positive number. Note that the actual frequency may vary.
	 * @return Actual configured frequency.
	 */
	target_freq = target_freq*1000000;
	float freqf = target_freq*65536.0/(float)CRYSTAL_FREQUENCY_M;
	uint32_t freq = (uint32_t)freqf;
	freq = freq&0x00FFFFFF;
	rf_write_register(FREQ0, freq);
	rf_write_register(FREQ1, (freq>>8));
	rf_write_register(FREQ2, (freq>>16));
	float t = ((float)freq*(float)CRYSTAL_FREQUENCY_M)/65536.0;

	return t;
}

float rf_set_channel_spacing(float cspacing){
	/*
	 * @brief Configures channel spacing registers to achieve the closer spacing possible to the target spacing
	 * Note that this functions depends on the value of CRYSTAL_FREQUENCY_M.
	 * @param cspacing Target spacing, in KHz. Positive number.
	 * @returns The actual configured spacing, in KHz
	 */
	uint8_t chanspc_e = 0;
	uint8_t chanspc_m = 0;
	float tmp;

	tmp = cspacing*((1<<18)/((float)CRYSTAL_FREQUENCY*(1<<chanspc_e)))-256.0;
	while(tmp>256 && chanspc_e<4){
		chanspc_e++;
		tmp = cspacing*((1<<18)/((float)CRYSTAL_FREQUENCY*(1<<chanspc_e)))-256.0;
	}
	chanspc_m = (uint8_t)tmp;
	rf_write_register(MDMCFG0, chanspc_m);

	uint8_t mdmcfg1 = rf_read_register(MDMCFG1);
	mdmcfg1 &= 0xFC;
	mdmcfg1 |= (chanspc_e & 0x2);
	rf_write_register(MDMCFG1, mdmcfg1);

	cspacing = ((float)CRYSTAL_FREQUENCY/(1<<18))*((float)chanspc_m+256.0)*(1<<chanspc_e);
	return cspacing;
}

void rf_set_preamble(uint8_t nbytes){
	/*
	 * @brief Sets the preamble size. The preamble is a stream of 1s and 0s that is sent before the packet.
	 */
	rf_sidle();
	//TODO Rright now it is harcoded to be 8 bytes.

}

void rf_set_preamble_threshold(uint8_t nbytes){
	/*
	 * @brief Sets the minimum preamble bytes to detect.
	 */
	rf_sidle();
	//TODO Right now it is hardocded to 4 bytes.

}


/*----------------------------[CC1101 States]----------------------------------------------*/
void rf_sidle(){
	/**
	 * @brief Set RF chip to idle state
	 */
    uint8_t marcstate;

    rf_write_strobe(SIDLE);              //sets to idle first. must be in

    marcstate = 0xFF;                     //set unknown/dummy state value

    while(marcstate != IDLE)
    {
        marcstate = (rf_read_register(MARCSTATE) & 0x1F);
    }

}

void rf_receive(){
	/*
	 * @brief Set RF chip to receive state (RX)
	 */
	//configure interruption, 1 when incoming packet
	rf_write_register(IOCFG0, 0x46);
	rf_write_strobe(SFRX);
	rf_write_strobe(SRX);

	uint8_t marcstate = 0xFF;
	while(marcstate != RX){
		marcstate = (rf_read_register(MARCSTATE)); //read out state of cc1100 to be sure in RX
	}
	GDO0_FLAG = 0;

}

//TODO Not tested
void rf_power_down(){
    rf_sidle();
    rf_write_strobe(SPWD);               // CC1100 Power Down
}

//TODO Not tested
void rf_wakeup(void){
	/*
	 * @brief Wakes up the c1101 from power down.
	 */
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
    //TODO rf_receive();                            // go to RX Mode
}

//TODO Not tested
uint8_t rf_get_settings(){
   static uint8_t settings[CFG_REGISTER];
   rf_read_data(0, settings, CFG_REGISTER);
   return settings;
}

//TODO Not tested
void rf_wor_enable(){
	/*
	 * @brief enables WOR Mode  EVENT0 ~1890ms; rx_timeout ~235ms
	 */
	/*
		EVENT1 = WORCTRL[6:4] -> Datasheet page 88
		EVENT0 = (750/Xtal)*(WOREVT1<<8+WOREVT0)*2^(5*WOR_RES) = (750/26Meg)*65407*2^(5*0) = 1.89s
							(WOR_RES=0;RX_TIME=0)               -> Datasheet page 80
	i.E RX_TimeOut = EVENT0*       (3.6038)      *26/26Meg = 235.8ms
							(WOR_RES=0;RX_TIME=1)               -> Datasheet page 80
	i.E.RX_TimeOut = EVENT0*       (1.8029)      *26/26Meg = 117.9ms
	*/
    rf_sidle();

    rf_write_register(MCSM0, 0x18);    //FS Autocalibration
    rf_write_register(MCSM2, 0x01);    //MCSM2.RX_TIME = 1b

    // configure EVENT0 time
    rf_write_register(WOREVT1, 0xFF);  //High byte Event0 timeout
    rf_write_register(WOREVT0, 0x7F);  //Low byte Event0 timeout

    // configure EVENT1 time
    rf_write_register(WORCTRL, 0x78);  //WOR_RES=0b; tEVENT1=0111b=48d -> 48*(750/26MHz)= 1.385ms

    rf_write_strobe(SFRX);             //flush RX buffer
    rf_write_strobe(SWORRST);          //resets the WOR timer to the programmed Event 1
    rf_write_strobe(SWOR);             //put the radio in WOR mode when CSn is released

    HAL_Delay(1); //TODO Really necessary?
}

//TODO Not tested
void rf_wor_disable(){
	rf_sidle();                            //exit WOR Mode
	rf_write_register(MCSM2, 0x07); //stay in RX. No RX timeout
}

//TODO Not tested
void rf_wor_reset(){
    rf_sidle();                            //go to IDLE
    rf_write_register(MCSM2, 0x01);    //MCSM2.RX_TIME = 1b
    rf_write_strobe(SFRX);             //flush RX buffer
    rf_write_strobe(SWORRST);          //resets the WOR timer to the programmed Event 1
    rf_write_strobe(SWOR);             //put the radio in WOR mode when CSn is released

    HAL_Delay(100); //Really necessary?
}

/*----------------------------[CC1101 Data Flow]----------------------------------------------*/

uint8_t _keep_transmiting_data(uint8_t* data, int len){
	/**
	 * @brief This function CONTINUES the transmission of data, but DOES NOT start it. Controls the data flow from the MCU to C1101 once
	 * started.
	 */
	int len_transmited = 0;
	uint32_t start_tick = HAL_GetTick();
	uint8_t last_chunk = len%DATA_CHUNK_SIZE;
	GDO0_FLAG = 0;
	while(len_transmited <len-last_chunk){
		if(GDO0_FLAG){
		//while(rf_read_register(TXBYTES)>DATA_CHUNK_SIZE); //Suing polling because OBC Int. does not work.
			GDO0_FLAG = 0;
			rf_write_data(TXFIFO, &data[len_transmited], DATA_CHUNK_SIZE);
			len_transmited +=DATA_CHUNK_SIZE;
			start_tick = HAL_GetTick();

		}
		if(HAL_GetTick()-start_tick> TimeOut) return FALSE;
	}
	if(last_chunk){
		while(!GDO0_FLAG);
		//while(rf_read_register(TXBYTES)>DATA_CHUNK_SIZE); //Using polling because OBC Int. does not work.
		GDO0_FLAG = 0;
		rf_write_data(TXFIFO, &data[len_transmited], last_chunk);
		if(HAL_GetTick()-start_tick> TimeOut) return FALSE;
	}

	return TRUE;
}


FRAMESTATUS_TypeDef send_frame(uint8_t* frame, int len){
	/**
	 * @brief Sends and unlimited length frame
	 * TODO RSSI and LQI values are appended to the packet, what to do with them?
	 */

	rf_sidle(); //Sets RF to idle state
	uint8_t pktcrtl0 = rf_read_register(PKTCTRL0);
	uint8_t frame_len = len%256;
	pktcrtl0 = pktcrtl0 & 0b11111100; //reset len mode
	int len_sent = 0;

	//configure interruption, high to low when below threshold
	uint8_t iocfg0 = 0x2;
	rf_write_register(IOCFG0, iocfg0);
	GDO0_FLAG = 0;

	rf_write_strobe(SFTX); //flush TX
	//TODO check if flushed

	//set packet length
	rf_write_register(PKTLEN, frame_len);

	if(len>FIXED_LENGTH_LIMIT){ //Use infinite packet length mode
		//Set len mode to infinit
		pktcrtl0 = pktcrtl0 | 0x2;
		rf_write_register(PKTCTRL0, pktcrtl0);

		//we need to fill the buffer before activating TX mode, or the chip will get into tx underflow state.
		rf_write_data(TXFIFO, frame, FIFO_SIZE); //fill the buffer completely
		rf_write_strobe(STX); //Start transmision
		len_sent +=FIFO_SIZE;

		int times = (len-len_sent)/FIFO_SIZE-1; //-1 to assure at bytes left to send them in receive mode
		//transmit (len -d -255) bytes of data, where d are the number of bytes already sent
		if(!_keep_transmiting_data(&frame[len_sent], times*FIFO_SIZE)) return TIMEOUT;
		len_sent += times*FIFO_SIZE;

		//transmit remaining bytes in fixed length mode.

		//Set len mode to fixed
		pktcrtl0 = pktcrtl0 & 0b11111100;
		rf_write_register(PKTCTRL0, pktcrtl0);

		if(!_keep_transmiting_data(&frame[len_sent], len-len_sent)) return TIMEOUT;

	}else{
		//Set len mode to fixed mode (default)
		rf_write_register(PKTCTRL0, pktcrtl0);

		if(len>FIFO_SIZE){ //Use variable packet length mode
			rf_write_data(TXFIFO, frame, FIFO_SIZE);
			rf_write_strobe(STX);
			len_sent+=FIFO_SIZE;
			if(!_keep_transmiting_data(&frame[len_sent], len-len_sent)) return TIMEOUT;
		}else{ //If len <= FIFO_SIZE, the FIFO needs to be filled once
			rf_write_data(TXFIFO, frame, len);
			rf_write_strobe(STX);
		}
	}

	uint32_t start_tick = HAL_GetTick();
	uint8_t state = rf_read_register(MARCSTATE);

	while(state!=IDLE){
		//printf("%#20x\n\r", state);
		state = rf_read_register(MARCSTATE);
		HAL_Delay(100);
		if(HAL_GetTick()- start_tick > TimeOut){
			if(state==TXFIFO_UNDERFLOW){
				rf_write_strobe(SFTX);
			}else{
				rf_sidle();
			}
			return FRAME_BAD;
		}
	}

	printf("FRAME SENDED\n\r");
    return FRAME_OK;
}

uint8_t rf_incoming_packet(){

	return GDO0_FLAG;
}

uint8_t  _keep_receiving_data(uint8_t* data, int len){
	/**
	 * @brief This function CONTINUES the reception of data, but DOES NOT start it. Controls the data flow from the C1101 to MCU
	 * TODO RSSI and LQI values are appended to the packet, what to do with them?
	 */
	int len_received = 0;
	uint32_t start_tick = HAL_GetTick();
	uint8_t last_chunk = len%DATA_CHUNK_SIZE;
	GDO0_FLAG = 0;
	while(len_received <len-last_chunk){
		//printf("%d\n\r", rf_read_register(PKTSTATUS)&1);
		if(GDO0_FLAG){ //if buffer is half empty
			GDO0_FLAG = 0;
			rf_read_data(RXFIFO, &data[len_received], DATA_CHUNK_SIZE);
			len_received +=DATA_CHUNK_SIZE;
			start_tick = HAL_GetTick();
		}
		if(HAL_GetTick()-start_tick> TimeOut) return FALSE;
	}
	if(last_chunk){
		if(!polling_while_lower(RXBYTES, last_chunk)) return FALSE; //Polling because it won't trigger the threshold.
		GDO0_FLAG = 0;
		rf_read_data(RXFIFO, &data[len_received], last_chunk);
	}

	return TRUE;
}

uint8_t polling_while_lower(uint8_t reg, uint8_t size){
	uint8_t t = rf_read_register(reg);
	uint32_t start_tick = HAL_GetTick();
	while(t<size){
		t = rf_read_register(reg);
		//printf("POLLING: %d\n\r", t);
		HAL_Delay(10);
		if(HAL_GetTick()-start_tick>TimeOut) return FALSE;
	}
	return TRUE;
}

uint8_t polling_while_bigger(uint8_t reg, uint8_t size){
	uint8_t t = rf_read_register(reg);
	while(t>size){
		t = rf_read_register(reg);
		HAL_Delay(10);
	}
	return TRUE;
}

uint16_t _get_frame_size(uint8_t* header, uint8_t data_len_loc, uint8_t data_len_size){
	/*
	 * @Returns The length of the frame.
	 */
	uint16_t mask = 1;
	for(int i = 1; i<data_len_size; i++){
		mask= mask <<1;
		mask+=1;
	}
	uint16_t frame_size;
	frame_size = (header[data_len_loc] & 0xFF) | ((header[data_len_loc+1]<<8) & 0xFF00);
	frame_size &=mask;
	return frame_size;
}

FRAMESTATUS_TypeDef receive_frame(uint8_t* frame_buffer, uint16_t* len, uint8_t data_len_loc, uint8_t data_len_size, uint8_t* lqi, uint8_t* rssi){
	/*
	 * @Brief Receives a frame. When this function returns, the chip goes back to IDLE mode. In principle it should be possible to
	 * maintain the RX mode to receive another packet if I understood correctly, but I haven't been able to achieve that.
	 * @param frame_buffer Buffer to store the received data.
	 * @param len The maximum len allowed (aka the buffer len). Used also to return the len of the received packet.
	 * @param position of the data length field in the frame header. Must be within the first 64 bytes.
	 * @param data_len_size Length of the data lengh field in bits. Used to mask 2 Bytes for a custom field size.
	 * @param lqi Link Quality indicator, the lower the better.
	 * @param rssi Received Signal Strengh Indicator.
	 * @Return CRC checksum ok?
	 */

	//init some variables
	uint16_t max_len = *len;
	*len = 0;
	uint16_t frame_len;
	uint8_t data_field_size = sizeof(*len);

	//Clear flag, since this function should have been called because of it
	GDO0_FLAG = 0;

	//configure interruption. Trigger when RX Buffer above threshold.
	rf_write_register(IOCFG0, 0x40);

	//Set to infinite len mode
	uint8_t pktcrtl0 = rf_read_register(PKTCTRL0);
	pktcrtl0 = pktcrtl0 & 0b11111100; //reset len mode
	pktcrtl0 = pktcrtl0 | 0x2;
	rf_write_register(PKTCTRL0, pktcrtl0);


	//check if receiving something
	uint8_t SFD = 0b00001000; //Sync Word OK? Addr (if enabled) OK?
	uint8_t status = 1; //rf_read_register(PKTSTATUS);

	/*
	while(!(status&SFD)){
			status = rf_read_register(PKTSTATUS);
	}
	*/
	//printf("SFD OK\n\r");

	//get frame size
	if(!polling_while_lower(RXBYTES, data_len_loc+data_field_size)) return TIMEOUT; //TODO reconfigure RX Threshold to detect the first bytes??
	rf_read_data(RXFIFO, frame_buffer, data_len_loc+data_field_size);
	frame_len = _get_frame_size(frame_buffer, data_len_loc, data_len_size);
	*len +=data_len_loc+data_field_size;
	if(frame_len > max_len) frame_len = max_len;
	printf("FRAME LEN: %d\n\r", frame_len);


	uint16_t remaining_len = frame_len-*len;
	//set packet length
	rf_write_register(PKTLEN, (frame_len)%256);


	if(remaining_len>FIXED_LENGTH_LIMIT){
		int times = (remaining_len)/FIFO_SIZE;
		if(!_keep_receiving_data(&frame_buffer[*len], times*FIFO_SIZE)) return TIMEOUT;
		*len += times*FIFO_SIZE;

		//set packet length to fixed
		pktcrtl0 = pktcrtl0 & 0b11111100;
		rf_write_register(PKTCTRL0, pktcrtl0);

		//receive remaining
		remaining_len = frame_len-*len;
		_keep_receiving_data(&frame_buffer[*len], remaining_len);
		*len+=remaining_len;

	}else if(remaining_len>DATA_CHUNK_SIZE){
		//set packet length to fixed
		pktcrtl0 = pktcrtl0 & 0b11111100;
		rf_write_register(PKTCTRL0, pktcrtl0);

		//receive remaining
		if(!_keep_receiving_data(&frame_buffer[*len], remaining_len)) return TIMEOUT;
		*len+=remaining_len;

	}else{
		//set packet length to fixed
		pktcrtl0 = pktcrtl0 & 0b11111100;
		rf_write_register(PKTCTRL0, pktcrtl0);

		//TODO using polling to not reconfigure interrupt
		if(!polling_while_lower(RXBYTES, remaining_len)) return TIMEOUT;
		rf_read_data(RXFIFO, &frame_buffer[*len], remaining_len);
		*len+=remaining_len;
	}
	/*
	int i = 0;
	for(i=2; i<frame_len; i++)
		//if(i%256!=frame_buffer[i])
			printf("%d: %d \r", i, frame_buffer[i]);
	*/
	//if rssi and lqi enabled

	*lqi = rf_read_register(LQI);
	uint8_t crc = (*lqi) & 0x80;
	*lqi = *lqi & 0x7F;

	*rssi = rf_read_register(RSSI);
	while((status&SFD)){
				status = rf_read_register(PKTSTATUS);
	}

	status = 0xFF;
	uint32_t start_tick = HAL_GetTick();
	while(status!=IDLE){ //PKTCTRL0 configured to go back to IDLE when reception finnished
					status = rf_read_register(MARCSTATE);
					HAL_Delay(100);
					if(HAL_GetTick()- start_tick > TimeOut){
						if(status==RXFIFO_OVERFLOW){
							rf_write_strobe(SFRX);
						}else{
							rf_sidle();
						}
						return FRAME_BAD;
					}
	}
	//printf("%d CRC: %d\n\r", *len, crc);
	FRAMESTATUS_TypeDef frame_status;
	if(crc){
		frame_status = FRAME_OK;
	}else{
		frame_status = FRAME_BAD;
	}
	return frame_status;
}


/* SPI Comm ----------------------------------------------------------------*/

void rf_write_strobe(uint8_t strobe){
	/**
	 * @brief Writes command to the CC1101 to change its state-machine state.
	 */
	strobe = WRITE(strobe);
	__spi_write(&strobe, NULL, NULL);
}

uint8_t rf_read_register(uint8_t reg){
	/**
	 * @brief Reads the content of a single 1-byte register.
	 * @Returns The register value.
	 */
	uint8_t data;
	reg= READ(reg);
	__spi_read(&reg, &data, 1);
	return data;
}

void rf_write_register(uint8_t reg, uint8_t data){
	/**
	 * @brief Overwrites a register.
	 */
	reg = WRITE(reg);
	__spi_write(&reg, &data, 1);
}

void rf_read_data(uint8_t addr, uint8_t* data, uint8_t size){
	/**
	 * @brief Reads multiple data.
	 * @param addr Base address.
	 * @param data The buffer where the read data will be stored.
	 * @param size Number of bytes to be read.
	 */
	if(size>1){
		addr = READ_BURST(addr);
	}else{
		addr = READ(addr);
	}
	__spi_read(&addr, data, size);
}

void rf_write_data(uint8_t addr, uint8_t* data, uint8_t size){
	/**
	 * @brief Writes multiple data.
	 * @param addr Base address.
	 * @param data The buffer where the data to be written is located.
	 * @param size Number of bytes to be written.
	 */
	if(size>1){
		addr = WRITE_BURST(addr);
	}else{
		addr = WRITE(addr);
	}
	__spi_write(&addr, data, size);
}

/* SPI Handling -------------------------------------------------------------*/

HAL_StatusTypeDef __spi_write(uint8_t *addr, uint8_t *pData, uint16_t size){
	HAL_StatusTypeDef status;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET); //set Chip Select to Low
	status = HAL_SPI_Transmit(hal_spi, addr, 1, 0xFFFF);
	if(status==HAL_OK && pData!=NULL)
		status = HAL_SPI_Transmit(hal_spi, pData, size, 0xFFFF);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET); //set Chip Select to High
	return status;

}

HAL_StatusTypeDef __spi_read(uint8_t *addr, uint8_t *pData, uint16_t size){

	HAL_StatusTypeDef status;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET); //set Chip Select to Low
	status = HAL_SPI_Transmit(hal_spi, addr, 1, 0xFFFF);
	status = HAL_SPI_Receive(hal_spi, pData, size, 0xFFFF);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET); //set Chip Select to High

	return status;

}

/* Interrupts ---------------------------------------------------------------*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	/*
	if (GPIO_Pin == GPIO_PIN_15)
		TX_RX_BEGAN = 1;
	*/
	if(GPIO_Pin == GDO0_Pin){
		GDO0_FLAG = 1;
		//HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
	}
	/*
	if(GPIO_Pin == CS_Pin){ //User B1 Button (the blue one on F446ZE)
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
	}*/
}

/* MISCELLANEOUS -----------------------------------------------------------------------------------------------------------*/

void init_serial(UART_HandleTypeDef* huart){

	hal_uart = huart;
}


int __io_putchar(int ch)
{
 uint8_t c[1];
 c[0] = ch & 0x00FF;
 HAL_UART_Transmit(hal_uart, &*c, 1, 10);
 return ch;
}

int _write(int file,char *ptr, int len)
{
 int DataIdx;
 for(DataIdx= 0; DataIdx< len; DataIdx++)
 {
 __io_putchar(*ptr++);
 }
return len;
}


/*
uint8_t __io_getchar(){
	 uint8_t c;
	 HAL_UART_Receive(hal_uart, &c, 1, 10);
	 return c;
}

int _read(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		*ptr++ = __io_getchar();
	}

return len;
}
*/
