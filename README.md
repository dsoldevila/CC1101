# CC1101

CC1101 Driver for STM32 (and in principle for any).

This driver has been tested with STM32F446ZETx (NUCLEO) and STM32F446RETx (OBC), but should be compatible with any STM32 board and should be easily portable to any board with an SPI interface.

The Inc and Src folders include the not-finnished driver for the CC1101 while Nucleo and OBC folders include the required files for the respective boards, which were built by STM32CubeMX, and the main functions. Builders folder include System Workbench for STM32 project files for both boards. GNURadio directory includes GNURadio flow charts used to capture and decode the radio signal (ASK mod) with an RTL-SDR and some output data. It was used for debuging the transmission of data.

Based on the work of https://github.com/SpaceTeddy/CC1101

**This project is not finnished yet and won't be in the short term as I no longer have neither the boards or the CC1101 to do so, but as I don't like to leave things have done I'm seeing myself retaking this project.** This time but with an Arduino and a Freescale board.

**Feel free to colaborate and send me a message for any doubts**

The commits are not explanatory at all as I worked on a different repo.
