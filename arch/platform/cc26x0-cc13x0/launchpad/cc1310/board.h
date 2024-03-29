/*
 * Copyright (c) 2016, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/** \addtogroup launchpad-peripherals
 * @{
 *
 * \defgroup launchpad-cc1310-specific CC1310 LaunchPad Peripherals
 *
 * Defines related to the CC1310 LaunchPad
 *
 * This file provides connectivity information on LEDs, Buttons, UART and
 * other peripherals
 *
 * This file is not meant to be modified by the user.
 * @{
 *
 * \file
 * Header file with definitions related to the I/O connections on the TI
 * CC1310 LaunchPad
 *
 * \note   Do not include this file directly. It gets included by contiki-conf
 *         after all relevant directives have been set.
 */
/*---------------------------------------------------------------------------*/
#ifndef BOARD_H_
#define BOARD_H_
/*---------------------------------------------------------------------------*/
#include "ioc.h"
/*---------------------------------------------------------------------------*/
/**
 * \name LED HAL configuration
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define LEDS_CONF_COUNT                 2
#define LEDS_CONF_RED                   1
#define LEDS_CONF_GREEN                 2
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name LED IOID mappings
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_LED_1          IOID_6
#define BOARD_IOID_LED_2          IOID_7
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UART IOID mapping
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_UART_RX        IOID_2
#define BOARD_IOID_UART_TX        IOID_3
#define BOARD_IOID_UART_RTS       IOID_18
#define BOARD_IOID_UART_CTS       IOID_19
#define BOARD_UART_RX             (1 << BOARD_IOID_UART_RX)
#define BOARD_UART_TX             (1 << BOARD_IOID_UART_TX)
#define BOARD_UART_RTS            (1 << BOARD_IOID_UART_RTS)
#define BOARD_UART_CTS            (1 << BOARD_IOID_UART_CTS)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Button IOID mapping
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_KEY_LEFT       IOID_13
#define BOARD_IOID_KEY_RIGHT      IOID_14
#define BOARD_KEY_LEFT            (1 << BOARD_IOID_KEY_LEFT)
#define BOARD_KEY_RIGHT           (1 << BOARD_IOID_KEY_RIGHT)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name External flash IOID mapping
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define EXT_FLASH_SPI_CONTROLLER    SPI_CONTROLLER_SPI0

#define EXT_FLASH_SPI_PIN_SCK       IOID_10
#define EXT_FLASH_SPI_PIN_MOSI      IOID_9
#define EXT_FLASH_SPI_PIN_MISO      IOID_8
#define EXT_FLASH_SPI_PIN_CS        IOID_20

#define EXT_FLASH_DEVICE_ID         0x14
#define EXT_FLASH_MID               0xC2

#define EXT_FLASH_PROGRAM_PAGE_SIZE 256
#define EXT_FLASH_ERASE_SECTOR_SIZE 4096
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \brief I2C IOID mappings
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_SCL            IOID_4
#define BOARD_IOID_SDA            IOID_5
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \brief ROM bootloader configuration
 *
 * Change CCXXWARE_CONF_BL_PIN_NUMBER to BOARD_IOID_KEY_xyz to select which
 * button triggers the bootloader on reset. Use CCXXWARE_CONF_BL_LEVEL to
 * control the pin level that enables the bootloader (0: low, 1: high). It is
 * also possible to use any other externally-controlled DIO.
 * @{
 */
#define CCXXWARE_CONF_BL_PIN_NUMBER   BOARD_IOID_KEY_LEFT
#define CCXXWARE_CONF_BL_LEVEL        0
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \brief Remaining pins
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_DIO1           IOID_1
#define BOARD_IOID_CS             IOID_11
#define BOARD_IOID_TDO            IOID_16
#define BOARD_IOID_TDI            IOID_17
#define BOARD_IOID_DIO12          IOID_12
#define BOARD_IOID_DIO15          IOID_15
#define BOARD_IOID_DIO21          IOID_21
#define BOARD_IOID_DIO22          IOID_22
#define BOARD_IOID_DIO23          IOID_23
#define BOARD_IOID_DIO24          IOID_24
#define BOARD_IOID_DIO25          IOID_25
#define BOARD_IOID_DIO26          IOID_26
#define BOARD_IOID_DIO27          IOID_27
#define BOARD_IOID_DIO28          IOID_28
#define BOARD_IOID_DIO29          IOID_29
#define BOARD_IOID_DIO30          IOID_30

#define BOARD_UNUSED_PINS { \
    BOARD_IOID_CS, BOARD_IOID_TDO, BOARD_IOID_TDI, \
    BOARD_IOID_DIO12, BOARD_IOID_DIO15, BOARD_IOID_DIO21, BOARD_IOID_DIO22, \
    BOARD_IOID_DIO23, BOARD_IOID_DIO24,BOARD_IOID_DIO25, BOARD_IOID_DIO26,\
    BOARD_IOID_DIO27, BOARD_IOID_DIO28, BOARD_IOID_DIO29, BOARD_IOID_DIO30, \
    IOID_UNUSED \
  }
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \brief Board indices for the button HAL
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_BUTTON_HAL_INDEX_KEY_LEFT   0x00
#define BOARD_BUTTON_HAL_INDEX_KEY_RIGHT  0x01
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Device string used on startup
 * @{
 */
#define BOARD_STRING "TI CC1310 LaunchPad"

/** @} */
/*---------------------------------------------------------------------------*/
#endif /* BOARD_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
