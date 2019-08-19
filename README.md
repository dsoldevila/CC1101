# CC1101

**This project is not finnished yet**

This driver has been tested with **STM32F446ZETx** (NUCLEO) and **STM32F446RETx** (OBC), but it should be compatible with any STM32 board and should be **easily portable to any board** with an SPI interface.

**Based on the work of https://github.com/SpaceTeddy/CC1101**

## What is this project:
This project aimed to develop a driver for the CC1101 radio transceiver. The driver was made to run on a STM32
Nucleo-F446ZE development board and thus compatible, in principle, with any STM32 board. Nevertheless, its layered nature eases its porting to other platforms. We could say it has 3 main layers: The first layer, which is platform-dependent, is the SPI communication, controlled (by default) by the STM32 HAL Driver. The second one abstracts the
data flow between the board and the CC1101 transceiver. The third one is compound by high level functions that handle the configuration of the transceiver and control the logic to perform the reception and transmission of data. 



## Directory tree
* **[Inc](Inc)/[Src](Src)** - include the not-finnished driver for the CC1101.
* **[Nucleo](Nucleo)/[OBC](OBC)** - include the required files for the respective boards, which were built by STM32CubeMX, and the main functions.
* **[Builders](Builders)** - includes System Workbench for STM32 project files for both boards.
* **[GNURadio](GNURadio)** - includes GNURadio flow charts used to capture and decode the radio signal (ASK mod) with an RTL-SDR and some output data. It was used for debuging the transmission of data.

## Future Work
This project is not finnished yet and won't be in the short term as I no longer have neither the boards or the CC1101 to do so, but as I don't like to leave things half done, I'm seeing myself retaking this project.

**Feel free to colaborate and to send me a message for any doubts**

## Other notes
The commits are not explanatory at all as I worked on a different repo...
