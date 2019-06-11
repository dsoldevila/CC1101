/*
 * rf_driver.h
 *
 *  Created on: 29 mar. 2019
 *      Author: dsoldevila
 *      This code is based on the code made by SpaceTeddy (https://github.com/SpaceTeddy/CC1101)
 */

#ifndef RF_DRIVER_H_
#define RF_DRIVER_H_
#include <stdio.h>
#ifndef __STM32F4xx_HAL_H
#include "stm32f4xx_hal.h"
#endif

/** Pinout -----------------------------------------------/
 * VDD --> 3.3V
 * GND --> GND
 * CS --> PD14
 * MOSI --> PA7
 * MISO --> PA6
 * SCK --> PA5
 * GDO0 --> PD15 // PD5 (UART2 TX)
 * GDO2 --> PF12 // PA3 (UART2 RX)
*/

/* Private define ------------------------------------------------------------*/

#define TRUE 1
#define FALSE 0

/*---------------------------[DATA]------------------------------------------*/
#define FIFO_SIZE 64  //Size of TX and RX FIFO buffers
#define FIXED_LENGTH_LIMIT 255 //For packets with length>255, infinite packet length mode must be used
#define HEADER_SIZE 1
#define INITIAL_DATA_SIZE (FIFO_SIZE - HEADER_SIZE) //64-1(header byte)
#define DATA_CHUNK_SIZE 32

/*---------------------------[CC1100 - R/W offsets]---------------------------*/
#define WRITE(ADDR) ADDR
#define WRITE_BURST(ADDR) ADDR | 0x40
#define READ(ADDR) ADDR | 0x80
#define READ_BURST(ADDR) ADDR | 0xC0
/*---------------------------[END R/W offsets]--------------------------------*/

/*------------------------[CC1100 - FIFO commands]----------------------------*/

/*Not using write/read and burst offset in order to point out that TX and RX even having the "same" address are 2 different buffers.
 * So instead of writing WRITE(FIFO), TXFIFO
*/
#define TXFIFO_BURST        	0x7F    //write burst only
#define TXFIFO				0x3F    //write single only
#define RXFIFO_BURST        	0xFF    //read burst only
#define RXFIFO 0xBF    //read single only

#define PATABLE_BURST       	0x7E    //power control read/write
#define PATABLE_SINGLE_BYTE 	0xFE    //power control read/write
/*---------------------------[END FIFO commands]------------------------------*/

/*----------------------[CC1100 - config register]----------------------------*/
#define IOCFG2   0x00         // GDO2 output pin configuration
#define IOCFG1   0x01         // GDO1 output pin configuration
#define IOCFG0   0x02         // GDO0 output pin configuration
#define FIFOTHR  0x03         // RX FIFO and TX FIFO thresholds
#define SYNC1    0x04         // Sync word, high byte
#define SYNC0    0x05         // Sync word, low byte
#define PKTLEN   0x06         // Packet length
#define PKTCTRL1 0x07         // Packet automation control
#define PKTCTRL0 0x08         // Packet automation control
#define ADDR     0x09         // Device address
#define CHANNR   0x0A         // Channel number
#define FSCTRL1  0x0B         // Frequency synthesizer control
#define FSCTRL0  0x0C         // Frequency synthesizer control
#define FREQ2    0x0D         // Frequency control word, high byte
#define FREQ1    0x0E         // Frequency control word, middle byte
#define FREQ0    0x0F         // Frequency control word, low byte
#define MDMCFG4  0x10         // Modem configuration
#define MDMCFG3  0x11         // Modem configuration
#define MDMCFG2  0x12         // Modem configuration
#define MDMCFG1  0x13         // Modem configuration
#define MDMCFG0  0x14         // Modem configuration
#define DEVIATN  0x15         // Modem deviation setting
#define MCSM2    0x16         // Main Radio Cntrl State Machine config
#define MCSM1    0x17         // Main Radio Cntrl State Machine config
#define MCSM0    0x18         // Main Radio Cntrl State Machine config
#define FOCCFG   0x19         // Frequency Offset Compensation config
#define BSCFG    0x1A         // Bit Synchronization configuration
#define AGCCTRL2 0x1B         // AGC control
#define AGCCTRL1 0x1C         // AGC control
#define AGCCTRL0 0x1D         // AGC control
#define WOREVT1  0x1E         // High byte Event 0 timeout
#define WOREVT0  0x1F         // Low byte Event 0 timeout
#define WORCTRL  0x20         // Wake On Radio control
#define FREND1   0x21         // Front end RX configuration
#define FREND0   0x22         // Front end TX configuration
#define FSCAL3   0x23         // Frequency synthesizer calibration
#define FSCAL2   0x24         // Frequency synthesizer calibration
#define FSCAL1   0x25         // Frequency synthesizer calibration
#define FSCAL0   0x26         // Frequency synthesizer calibration
#define RCCTRL1  0x27         // RC oscillator configuration
#define RCCTRL0  0x28         // RC oscillator configuration
#define FSTEST   0x29         // Frequency synthesizer cal control
#define PTEST    0x2A         // Production test
#define AGCTEST  0x2B         // AGC test
#define TEST2    0x2C         // Various test settings
#define TEST1    0x2D         // Various test settings
#define TEST0    0x2E         // Various test settings
/*-------------------------[END config register]------------------------------*/


/*------------------------[CC1100-command strobes]----------------------------*/
#define SRES     0x30         // Reset chip
#define SFSTXON  0x31         // Enable/calibrate freq synthesizer
#define SXOFF    0x32         // Turn off crystal oscillator.
#define SCAL     0x33         // Calibrate freq synthesizer & disable
#define SRX      0x34         // Enable RX.
#define STX      0x35         // Enable TX.
#define SIDLE    0x36         // Exit RX / TX
#define SAFC     0x37         // AFC adjustment of freq synthesizer
#define SWOR     0x38         // Start automatic RX polling sequence
#define SPWD     0x39         // Enter pwr down mode when CSn goes hi
#define SFRX     0x3A         // Flush the RX FIFO buffer.
#define SFTX     0x3B         // Flush the TX FIFO buffer.
#define SWORRST  0x3C         // Reset real time clock.
#define SNOP     0x3D         // No operation.
/*-------------------------[END command strobes]------------------------------*/

/*----------------------[CC1100 - status register]----------------------------*/
#define PARTNUM        0xF0   // Part number
#define VERSION        0xF1   // Current version number
#define FREQEST        0xF2   // Frequency offset estimate
#define LQI            0xF3   // Demodulator estimate for link quality
#define RSSI           0xF4   // Received signal strength indication
#define MARCSTATE      0xF5   // Control state machine state
#define WORTIME1       0xF6   // High byte of WOR timer
#define WORTIME0       0xF7   // Low byte of WOR timer
#define PKTSTATUS      0xF8   // Current GDOx status and packet status
#define VCO_VC_DAC     0xF9   // Current setting from PLL cal module
#define TXBYTES        0xFA   // Underflow and # of bytes in TXFIFO
#define RXBYTES        0xFB   // Overflow and # of bytes in RXFIFO
#define RCCTRL1_STATUS 0xFC   //Last RC Oscillator Calibration Result
#define RCCTRL0_STATUS 0xFD   //Last RC Oscillator Calibration Result
//--------------------------[END status register]-------------------------------

/*----------------------[CC1100 - misc]---------------------------------------*/
#define CRYSTAL_FREQUENCY 26000
#define CRYSTAL_FREQUENCY_M CRYSTAL_FREQUENCY/1000
#define CFG_REGISTER              0x2F  //47 registers
#define MAX_CHANNEL_SPACING (0xFF+256)*CRYSTAL_FREQUENCY*(1<<3)/(1<<18)
#define MIN_CHANNEL_SPACING (0x0+256)*CRYSTAL_FREQUENCY/(1<<18) //Min value possible, but min legal value?

#define RSSI_OFFSET_868MHZ        0x4E  //dec = 74
#define TX_RETRIES_MAX            0x05  //tx_retries_max
#define ACK_TIMEOUT                200  //ACK timeout in ms
#define CC1100_COMPARE_REGISTER   0x00  //register compare 0=no compare 1=compare
//#define BROADCAST_ADDRESS         0x00  //broadcast address
//#define CC1100_FREQ_315MHZ        0x01
//#define CC1100_FREQ_434MHZ        0x02
//#define CC1100_FREQ_868MHZ        0x03
//#define CC1100_FREQ_915MHZ        0x04
//#define CC1100_FREQ_2430MHZ       0x05
#define CC1100_TEMP_ADC_MV        3.225 //3.3V/1023 . mV pro digit
#define CC1100_TEMP_CELS_CO 2.47 //Temperature coefficient 2.47mV per Grad Celsius

/* Private typedef ---------------------------------------------------------*/
typedef enum{
	//SLEEP = 0x00, //Can't be accessed
	IDLE = 0x01,
	//XOFF = 0x02,
	VCOON_MC = 0x03, //MANCAL
	REGON_MC = 0x04, //MANCAL
	MANCAL = 0x05,
	VCOON = 0x06, //FS_WAKEUP
	REGON = 0x07, //FS_WAKEUP
	STARTCAL = 0x08, //CALIBRATE
	BWBOOST = 0x09, //SETTLIG
	FS_LOCK = 0x0A, //SETTLIG
	IFADCON = 0x0B, //SETTLIG
	ENDCAL = 0x0C, //CALIBRATE
	RX = 0x0D,
	RX_END = 0x0E,	//RX
	RX_RST = 0x0F,	//RX
	TXRX_SWITCH = 0x10, //TXRX_SETTLING
	RXFIFO_OVERFLOW = 0x11,
	FSTXON = 0x12,
	TX = 0x13,
	TX_END = 0x14, //TX
	RXTX_SWITCH = 0x15,	//RXTX_SETTLING
	TXFIFO_UNDERFLOW = 0x16
}STATE_TypeDef;

typedef enum{
	GFSK_1_2_kb = 1,
	GFSK_38_4_kb,
	GFSK_100_kb,
	MSK_250_kb,
	MSK_500_kb,
	OOK_4_8_kb
}MODULATION_TypeDef;

typedef enum{
	MHz315 = 1,
	MHz434,
	MHz868,
	MHz915,
}ISMBAND_TypeDef;


/* Private function prototypes -----------------------------------------------*/

/* SPI data flow-------------*/
void rf_write_strobe(uint8_t strobe);
uint8_t rf_read_register(uint8_t reg);
void rf_write_register(uint8_t reg, uint8_t data);
void rf_read_data(uint8_t addr, uint8_t* data, uint8_t size);
void rf_write_data(uint8_t addr, uint8_t* data, uint8_t size);
HAL_StatusTypeDef __spi_write(uint8_t* addr, uint8_t *pData, uint16_t size);
HAL_StatusTypeDef __spi_read(uint8_t* addr, uint8_t *pData, uint16_t size);


/* Driver init-------------*/
uint8_t rf_check();
void rf_reset();
uint8_t rf_begin(SPI_HandleTypeDef* hspi, MODULATION_TypeDef mode, ISMBAND_TypeDef ism_band,  GPIO_TypeDef* cs_port, uint16_t cs_pin, uint16_t gdo0_pin);

/* Driver Config -------*/
void rf_set_modulation_mode(MODULATION_TypeDef mod);
void rf_set_ISMband(ISMBAND_TypeDef band);
void rf_set_channel(uint8_t channel);
void rf_set_output_power_level(int8_t);
void rf_set_syncword(uint16_t syncword);
//packet control
void rf_set_addr(uint8_t addr);
//FSCTRL1 – Frequency Synthesizer Control??
float rf_set_carrier_offset(float offset);
float rf_set_carrier_frequency(float target_freq);
float rf_set_channel_spacing(float cpsacing);


/*State --------------*/
void sidle();
void rf_power_down();
void rf_wakeup();
uint8_t rf_get_settings();
void rf_wor_enable();
void rf_wor_disable();
void rf_wor_reset();



/* Comm -------------*/
uint16_t _get_frame_size(uint8_t* header, uint8_t data_len_loc);
uint8_t _keep_transmiting_data(uint8_t* data, int len);
uint8_t send_frame(uint8_t* frame, int len);
uint8_t  _keep_receiving_data(uint8_t* data, int len);
uint8_t* receive_frame(uint8_t header_len, uint8_t data_len_loc);
uint8_t polling(uint8_t reg, uint8_t size);

/* Misc -------------*/
void init_serial(UART_HandleTypeDef* huart);
int __io_putchar(int ch);
int _write(int file,char *ptr, int len);

void init_led();





/*
uint8_t __io_getchar();
int _read(int file,char *ptr, int len);
*/

#endif /* RF_DRIVER_H_ */
