/**
 * @file xensiv_bgt60cutrxx.h
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

#ifndef XENSIV_BGT60CUTRXX_H_
#define XENSIV_BGT60CUTRXX_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


/**
 * \defgroup bgt60cutrxx_driver BGT60CUTR13 Driver
 * @{
 */

/** Reset pin inactive value. */
#define RESET_INACTIVE (1)
/** Reset pin active value. */
#define RESET_ACTIVE (0)
/** SPI chip select pin inactive value. */
#define CS_INACTIVE (1)
/** Reset pin active value. */
#define CS_ACTIVE (0)
/** Reset delay for radar. */
#define RESET_DELAY_MS (5U)
/** ADC calibration timeout. */
#define CALIBRATION_TIMEOUT_US (5000U)
/** delay for polling ADC calibration done flag */
#define ADC_CALIBRATION_POLLING_INTERVALL_US (100)

/**defines the waiting time to ensure that the main clock is ready after triggering the power up request*/
#define BGT60CUTRxx_CLK_SETUP_TIME_MS 1

/** Result code indicating successful operation. */
#define XENSIV_BGT60CUTRXX_STATUS_OK (0)
/** Result code indicating spi communication error. */
#define XENSIV_BGT60CUTRXX_STATUS_SPI_ERROR (1)
/** Result code indicating device detection error. */
#define XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR (2)
/** Result code indicating an operation timeout error. */
#define XENSIV_BGT60CUTRXX_STATUS_TIMEOUT_ERROR (3)
/** Result code indicating an ADC calibration error. */
#define XENSIV_BGT60CUTRXX_STATUS_ADC_CALIBRATION_ERROR (4)
/** Result code indicating an invalid range zone, i.e. angle is < -78.75 or > 78.75 */
#define XENSIV_BGT60CUTRXX_STATUS_RANGE_ZONE_ERROR (5)

/** bgt60cutrxx register values are 32 bits wide */
typedef uint32_t bgt60cutrxx_regval_t;


/** represents the sleep mode of the sensor */
typedef enum
{
    /** Active mode */
    ACTIVE_MODE = 0,
    /** Deep sleep mode */
    DEEP_SLEEP_MODE = 1
} xensiv_bgt60cutrxx_sleep_mode_t;

/** represents the FIFO events in interrupt status register of the sensor */
typedef enum
{
    /** No event */
    FIFO_EVENT_NONE = 0,
    /** FIFO threshold exceeded */
    FIFO_EVENT_THRESHOLD = 1,
    /** FIFO underflow occurred */
    FIFO_EVENT_UNDERFLOW = 2,
    /** FIFO overflow occurred */
    FIFO_EVENT_OVERFLOW = 3
} xensiv_bgt60cutrxx_fifo_events_t;
/** represents the source of the interrupt from radar*/
typedef enum
{
    /** ULP interrupt */
    IR_ID_ULP = 1,
    /** RD interrupt */
    IR_ID_RD = 2,
    /** FIFO interrupt */
    IR_ID_FIFO = 3
} xensiv_bgt60cutrxx_irq_src_t;

/** represents the type of reset to perform */
typedef enum
{
    /** Resets the sequencer, the FSM and stops active sequences */
    RESET_FSM = 1,
    /** Resets the chip. Clock settings are kept. */
    RESET_SW = 2,
    /** Resets the FIFO (data SRAM). */
    RESET_FIFO = 4,
    /** Resets the motion detection FSM and its status bits. */
    RESET_MD = 8
} xensiv_bgt60cutrxx_reset_t;

/** used to define the IO voltage level */
typedef enum
{
    /** IO voltage level is 1.2V */
    BGT60CUTRXX_IO_LEVEL_1V2 = 0,
    /** IO voltage level is 1.8V */
    BGT60CUTRXX_IO_LEVEL_1V8 = 1
} xensiv_bgt60cutrxx_io_level_t;

/** used to select 40 or 80MHz */
typedef enum
{
    /** 40MHz clock */
    BGT60CUTRXX_CLK_40MHZ = 0,
    /** 80MHz clock */
    BGT60CUTRXX_CLK_80MHZ = 1
} xensiv_bgt60cutrxx_clk_t;

/** represents the type and size of one FIFO element*/
typedef uint32_t fifo_entry_t;

/** XENSIV(TM) BGT60CUTRxx sensor device object.
 *
 * Application code should not rely on the specific content of this struct.
 * They are considered an implementation detail which is subject to change
 * between platforms and/or library releases.
 */
typedef struct
{
    void* iface;     /**< Pointer to platform-specific SPI interface */
    /** Flag indicating the state of the XOSC (External Oscillator) */
    bool xosc_active;
    /** Device ID of the XENSIV BGT60CUTRXX sensor */
    uint32_t device_id;
    /** Flag indicating the sleep mode of the XENSIV BGT60CUTRXX sensor */
    xensiv_bgt60cutrxx_sleep_mode_t mode;
    /**XOSC Clock frequency of the XENSIV BGT60CUTRXX sensor */
    xensiv_bgt60cutrxx_clk_t clk_freq_xtal;
    /** IO voltage level of the XENSIV BGT60CUTRXX sensor */
    xensiv_bgt60cutrxx_io_level_t io_level;
} xensiv_bgt60cutrxx_t;


/** Represents the calibration data for the XENSIV BGT60CUTRXX sensor
 */
typedef struct xensiv_bgt60cutrxx_calibration_data_t
{
    /** Address to read calibration data for RX0 */
    const uint16_t rx0_address;
    /** Address to read calibration data for RX1 */
    const uint16_t rx1_address;
    /** Address to read calibration data for RX2 */
    const uint16_t rx2_address;
    /** Number of bytes to read from each RX channel */
    const uint16_t bytes_to_read;
} xensiv_bgt60cutrxx_calibration_data_t;


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Initializes the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function initializes the XENSIV(TM)  BGT60CUTRXX sensor and prepares it for operation.
 * @param dev Pointer to the XENSIV BGT60CUTRXX device structure.
 * @param iface Pointer to the SPI peripheral structure.
 * @param clk The external crystal frequency to be set (40MHz or 80MHz).
 * @param io_level The IO voltage level to be set (1.2V or 1.8V).
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_init(xensiv_bgt60cutrxx_t* dev, void* iface, xensiv_bgt60cutrxx_clk_t clk,
                                xensiv_bgt60cutrxx_io_level_t io_level);

/**
 * @brief Detects the presence of a XENSIV BGT60CUTRXX device.
 *
 * This function is used to detect the presence of a XENSIV BGT60CUTRXX device
 * connected to the specified device structure.
 *
 * @param dev Pointer to the XENSIV BGT60CUTRXX device structure.
 * @return Returns an int32_t value indicating the detection status:
 *         - 0 if the device is detected successfully.
 *         - A negative value if an error occurs during detection.
 */
int32_t xensiv_bgt60cutrxx_detect_device(xensiv_bgt60cutrxx_t* dev);

/**
 * @brief Sets the value of a register in the XENSIV(TM)  BGT60CUTRXX sensor.
 *
 * This function allows you to set the value of a specific register in the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * @param dev Pointer to the XENSIV BGT60CUTRXX device structure.
 * @param addr The address of the register to be set.
 * @param val The data to be written to the register.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_set_reg(xensiv_bgt60cutrxx_t* dev, uint16_t addr, uint32_t val);

/**
 * @brief Get the value of a register in the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function retrieves the value of a register specified by the reg_addr parameter.
 *
 * @param dev Pointer to the XENSIV BGT60CUTRXX device structure.
 * @param addr The address of the register to read.
 * @param val Pointer to a variable where the register value will be stored.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_get_reg(xensiv_bgt60cutrxx_t* dev, uint16_t addr, uint32_t* val);

/**
 * @brief Resets the XENSIV(TM) BGT60CUTRXX device.
 *
 * This function resets the XENSIV(TM) BGT60CUTRXX device by performing the necessary
 * operations to restore it to its default state.
 *
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 */
void xensiv_bgt60cutrxx_hard_reset(xensiv_bgt60cutrxx_t* dev);

/**
 * @brief Performs a soft reset on the XENSIV(TM) BGT60CUTRXX device.
 *
 *
 * This function resets the XENSIV(TM) BGT60CUTRXX sensor using a software reset.
 *
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @param reset_type The type of reset to perform.
 * @return Returns an int32_t value indicating the status of the reset operation.
 *         A return value of 0 indicates success, while a non-zero value indicates an error.
 */
int32_t xensiv_bgt60cutrxx_soft_reset(xensiv_bgt60cutrxx_t* dev, xensiv_bgt60cutrxx_reset_t reset_type);
/**
 * Configures the XENSIV(TM) BGT60CUTRXX sensor with the provided register settings.
 *
 * @param dev Pointer to the XENSIV BGT60CUTRXX device structure.
 * @param regs The array of register settings to be applied to the sensor.
 * @param len The length of the register settings array.
 * @return Returns an int32_t value indicating the success or failure of the configuration.
 */
int32_t xensiv_bgt60cutrxx_config(xensiv_bgt60cutrxx_t* dev, const uint32_t regs[][2], uint32_t len);

/**
 * Performs ADC calibration for the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function calibrates the ADC (Analog-to-Digital Converter) for the XENSIV(TM) BGT60CUTRXX sensor
 * specified by the given sequence index. ADC calibration is always performed by sequence 3.
 *
 * @param dev Pointer to the XENSIV BGT60CUTRXX device structure.
 * @return An integer value indicating the status of the ADC calibration process.
 *         A positive value indicates a successful calibration, while a negative value indicates an error.
 */

int32_t xensiv_bgt60cutrxx_perform_adc_calibration(xensiv_bgt60cutrxx_t* dev);

/**
 * Sets the motion detection for the XENSIV(TM) BGT60CUTRXX device.
 *
 * This function enables or disables the motion detection feature for the specified timer index.
 *
 * @param dev The pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @param timer_index The index of the timer to set motion detection for.
 * @param enable Set to true to enable motion detection, false to disable.
 * @return Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_set_motion_detection(xensiv_bgt60cutrxx_t* dev, uint16_t timer_index, bool enable);

/**
 * @brief Get the state of the XOSC (External Oscillator) for the XENSIV(TM) BGT60CUTRXX device.
 *
 * This function retrieves the state of the XOSC for the XENSIV(TM) BGT60CUTRXX device.
 *
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @return Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_get_xosc_state(xensiv_bgt60cutrxx_t* dev);

/**
 * @brief Sets the XOSC (External Oscillator) value for the XENSIV(TM) BGT60CUTRXX device.
 *
 * This function is used to set the XOSC value for the XENSIV(TM) BGT60CUTRXX device.
 *
 * @param dev Pointer to the XENSIV BGT60CUTRXX device structure.
 * @param val The value to set for the XOSC. Use true to enable XOSC and false to disable XOSC.
 *
 * @return Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_set_xosc(xensiv_bgt60cutrxx_t* dev, bool val);

/**
 * @brief Reads the interrupt status register of the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function reads the interrupt status of the XENSIV(TM) BGT60CUTRXX sensor and stores it in the provided variable.
 *
 * @param dev Pointer to the XENSIV BGT60CUTRXX device structure.
 * @param val Pointer to a variable where the interrupt status values will be stored.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_get_irq_status(xensiv_bgt60cutrxx_t* dev, uint32_t* val);

/**
 * @brief Sets the IRQ status for the XENSIV(TM) BGT60CUTRXX device.
 *
 * This function sets the BGT60CUTRxx_REG_IR_CLEAR register of the XENSIV(TM) BGT60CUTRXX device to the specified value.
 *
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @param val The value to set the IR_CLEAR register to.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_set_irq_clear(xensiv_bgt60cutrxx_t* dev, uint32_t val);

/**
 * @brief Retrieves the RD (Range Doppler) map from the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function retrieves the RD map from the XENSIV(TM) BGT60CUTRXX sensor and stores it in the provided rd_data
 * buffer.
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @param rd_data Pointer to the buffer where the RD map will be stored.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_get_rd_map(xensiv_bgt60cutrxx_t* dev, uint32_t* rd_data);

/**
 * @brief Retrieves the motion detection zones from the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function retrieves the motion detection targets from the XENSIV(TM) BGT60CUTRXX sensor
 * and stores them in the provided targets array.
 *
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @param zones Pointer to an array where the motion detection targets will be stored.
 * @param zone_size The size of the targets array.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_get_md_zones(xensiv_bgt60cutrxx_t* dev, uint32_t* zones, uint16_t zone_size);

/**
 * @brief Retrieves the motion detection targets from the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function retrieves the motion detection targets from the XENSIV(TM) BGT60CUTRXX sensor
 * and stores them in the provided targets array.
 *
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @param targets Pointer to an array where the motion detection targets will be stored.
 * @param targets_size The size of the targets array.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_get_md_targets(xensiv_bgt60cutrxx_t* dev, uint32_t* targets, uint16_t targets_size);
/**
 * @brief Retrieves the FIFO data from the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function retrieves the data stored in the FIFO from the XENSIV(TM) BGT60CUTRXX sensor
 * into the provided data array.
 * Thereby, it reads the FIFO in blocks. The maximum block size is defined by the address space of
 * BGT60CUTR13B_RD_MAP_ADDRESS - BGT60CUTR13B_FIFO_ADDRESS.
 *
 * @note The caller must ensure the data array has enough space to hold the requested number of FIFO entries.
 *
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @param data Pointer to an array where the data will be stored.
 * @param num_of_fifo_entries The number of entries that should be read from the FIFO. One entry is from type
 * \ref fifo_entry_t and contains one or two radar samples depending on the configured ADC resolution.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */

int32_t xensiv_bgt60cutrxx_get_fifo(xensiv_bgt60cutrxx_t* dev, fifo_entry_t* data, uint32_t num_of_fifo_entries);
/**
 * @brief Retrieves the motion detection targets from the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function triggers the radar sequence 0.
 *
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */

int32_t xensiv_bgt60cutrxx_trigger_sequence0(xensiv_bgt60cutrxx_t* dev);

/** @brief Sets the IO level for the XENSIV(TM) BGT60CUTRXX device.
 *
 * This function sets the bit BGT60CUTRxx_REG_IO_CTRL.IO_LVL of the XENSIV(TM) BGT60CUTRXX device to the specified value
 * by performing a read modify write operation to the BGT60CUTRxx_REG_IO_CTRL register.
 *
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @param level The IO level to set (1.2V or 1.8V).
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
int32_t xensiv_bgt60cutrxx_set_io_level(xensiv_bgt60cutrxx_t* dev, xensiv_bgt60cutrxx_io_level_t level);


/**
 * @brief Reads the calibration data from the XENSIV(TM) BGT60CUTRXX sensor.
 * This function reads the calibration data from the specified addresses and stores it in the provided calibration_data
 * array.
 * @param dev Pointer to the XENSIV BGT60CUTRXX device structure.
 * @param calibration_settings Pointer to the calibration settings structure containing the addresses and byte count.
 * The structure must be defined by the caller.
 * @param calibration_data Pointer to the array where the calibration data will be stored. The memory for the array must
 * be allocated by the caller.
 * @return int32_t Returns 0 on success, or a negative error code on failure
 */
int32_t xensiv_bgt60cutrxx_read_calibration_data(xensiv_bgt60cutrxx_t* dev,
                                                 xensiv_bgt60cutrxx_calibration_data_t* calibration_settings,
                                                 uint32_t* calibration_data);

#ifdef __cplusplus
}
#endif

/**@} end of group bgt60cutrxx_driver*/
#endif /* XENSIV_BGT60CUTRXX_H_ */
