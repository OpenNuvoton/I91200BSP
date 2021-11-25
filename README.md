-------------
I91200 BSP -V3.03.001 2020/02/04
-------------
[Fix]
1.	I2S: Reset module issue in I2S_Open.
2.	ISP_UART: update APROM will erase data in dataflash.

[Add]
1.	"SPI0_Slave" Sample: Demonstrates how to setup SPI0 as slave device.
2.	"SPI1_Master" Sample: Demonstrates how to setup SPI1 as master device.

[Revised]
1.	Revise SPIFlash library setting:
	Recommend to predefine SPIFlash bit mode in project setting at using SPIFlash.c.
	Please add the following definition in 'Project'->'Options for target'->'C/C++'->'PreProcessor Symbols'->'Define'
	SPIFLASH_INTERFACE_MODE = 0 // One-bit Mode
	SPIFLASH_INTERFACE_MODE = 1 // Dual Mode
	SPIFLASH_INTERFACE_MODE = 2 // Quad Mode

-------------
I91200 BSP -V3.03.000 2019/06/27
-------------
[Revised]
1.	Bridge Sense: Revised BSP for 'I91200BS' version. Added new sample and project settings to 
	existed sample in order to be Compatible with 'I91200BS' version.

[Add]
1.	"BOD_Wakeup" Sample: Demonstrates how to setup and operate BOD wakeup function.
2.	"SDADC_BridgeSenseBasic" Sample: Demonstrate how to use 'SDADC_BridgeSense' basic function.
3.	"SPIFlash" Sample: Demonstrate how to read, write and erase external flash data using SPI function.
4.	"ISP_UART" Sample: I91200 firmware source code for ISP tool UART transmission.

-------------
I91200 BSP -V3.02.000 2017/12/15
-------------
[Add]
1.	IAR Supported demo sample.

[Fixed]
1.	PWM: Problem of output pin description(PA.12 correct to PA.10).

2.	RTC: External 32K crystal does not enable.

3.	SARADC_SingleEnd: Demo ends without while loop.

4.	WDT: Button pin configuration does not match with the EVB.