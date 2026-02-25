/**
 * @file xensiv_bgt60cutrxx_platform.h
 * @brief Header file for the XENSIV(TM) BGT60CUTRXX module.
 *
 * This file contains the declarations and definitions for the XENSIV(TM) BGT60CUTRXX module.
 * The module provides functionality related to the XENSIV(TM) BGT60CUTRXX sensor, including
 * initialization, configuration, and data retrieval.
 *
 ********************************************************************************
 * Copyright (c) (2016-2022), Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#ifndef XENSIV_BGT60CUTRXX_PLATFORM_H_
#define XENSIV_BGT60CUTRXX_PLATFORM_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * Macro to enable or disable debug print statements
 * If the macro DEBUG_PRINT_DISABLE is defined, then no code will be generated for the debug_print macro.
 * If the macro DEBUG_PRINT_ENABLE is defined, then printf and retarget io will be used  for printing debug messages.
 */
#ifdef DEBUG_PRINT_ENABLE
#include <stdio.h>  // Include stdio.h for printf
/**
 * Macro to capture debug output message.
 * In this case, the debug_print sends the messages to usb serial port via printf
 */
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
/**
 * In case macro is not defined, then the calls for debug_print will be empty and no code will be generated.
 */
#define DEBUG_PRINT(...)
#endif

/**
 * \defgroup platform_specific Platform Specific Interface
 * @{
 */

/**
 * @brief Delays the execution for the specified number of milliseconds.
 *
 * This function provides a platform-specific delay implementation for the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * @param ms The number of milliseconds to delay.
 */
void xensiv_bgt60cutrxx_platform_delay(uint32_t ms);

/**
 * @brief Delays the execution for the specified number of microseconds.
 *
 * This function provides a platform-specific delay implementation for the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * @param us The number of microseconds to delay.
 */
void xensiv_bgt60cutrxx_platform_delay_us(uint32_t us);

/**
 * @brief Sets the reset pin for the xensiv_bgt60cutrxx to given value.
 *
 * This function is used to set the reset pin for the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * @param iface Pointer to the interface structure for the XENSIV(TM) BGT60CUTRXX sensor.
 * @param val The value to set for the reset pin.
 */
void xensiv_bgt60cutrxx_platform_rst_set(const void* iface, bool val);

/**
 * @brief Sets the spi chip select pin for the xensiv_bgt60cutrxx to given value.
 *
 * This function is used to set the SPI CS pin for the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * @param iface Pointer to the interface structure for the XENSIV(TM) BGT60CUTRXX sensor.
 * @param val The value to set for the chip select pin.
 */
void xensiv_bgt60cutrxx_platform_spi_cs_set(const void* iface, bool val);

/**
 * @brief Performs a SPI transfer of 16-bit data using the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function transfers data from the transmit buffer (`txd`) to the receive buffer (`rxd`) using SPI communication.
 * The transfer length is specified by the `len` parameter.
 *
 * @param iface Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @param txd Pointer to the transmit buffer containing the data to be sent.
 * @param rxd Pointer to the receive buffer where the received data will be stored.
 * @param len The length of the data to be transferred, in number of 16-bit words.
 *
 * @return Returns an int32_t value indicating the success or failure of the operation.
 *         A return value of 0 indicates success, while a non-zero value indicates failure.
 */
int32_t xensiv_bgt60cutrxx_platform_spi_transfer16(void* iface, const uint16_t* txd, uint16_t* rxd, uint32_t len);
/**
 * @brief Reads data from the specified memory of the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function reads data from the memory of the XENSIV(TM) BGT60CUTRXX sensor.
 * If the DMA is configured and enabled, the data is read via DMA.
 *
 * @param iface Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @param txd Pointer to the transmit buffer containing the data to be sent.
 * @param rxd Pointer to the buffer where the received data will be stored.
 * @param len The number of data elements to read from the device in 16bit words.
 *
 * @return Returns an int32_t value indicating the success or failure of the operation.
 *         A return value of 0 indicates success, while a non-zero value indicates failure.
 */
int32_t xensiv_bgt60cutrxx_platform_spi_read_block(void* iface, const uint16_t* txd, uint16_t* rxd, uint32_t len);

/**@}*/
#endif /* XENSIV_BGT60CUTRXX_PLATFORM_H_ */
