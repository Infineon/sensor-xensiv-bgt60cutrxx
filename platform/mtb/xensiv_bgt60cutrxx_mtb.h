/**
 * @file xensiv_bgt60cutrxx_mtb.h
 * @brief Header file for XENSIV BGT60CUTRXX PSoC interface.
 *
 * This file contains the type definitions and function declarations for interfacing
 * with the XENSIV BGT60CUTRXX radar sensor using PSoC.
 */

#ifndef XENSIV_BGT60CUTRXX_MTB_H_
#define XENSIV_BGT60CUTRXX_MTB_H_

#include "cyhal.h"
#include "xensiv_bgt60cutrxx.h"

/**
 * @def ARRAY_SIZE(a)
 * @brief Macro to calculate the number of elements in an array.
 *
 * This macro takes an array as input and returns the number of elements in the array.
 *
 * @param a The array whose size is to be determined.
 * @return The number of elements in the array.
 */
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/** XENSIV(TM) BGT60CUTRxx sensor device interface.
 *
 * Platform specific interface
 */
typedef struct
{
    /** Platform-specific SPI interface  */
    cyhal_spi_t spi;
    /** MCU pin for spi chip select */
    cyhal_gpio_t selpin;
    /** MCU pin for radar reset */
    cyhal_gpio_t rstpin;
    /** Pointer to interrupt callback data*/
    cyhal_gpio_callback_data_t irq_callback_data;
} xensiv_bgt60cutrxx_mtb_iface_t;

/** XENSIV(TM) BGT60CUTRxx sensor device configuration.
 *
 * Configuration for platform specific interface
 */
typedef struct
{
    /** MCU pin for SPI chip select */
    cyhal_gpio_t selpin;
    /** MCU pin for radar sensor reset */
    cyhal_gpio_t rstpin;
    /** MCU pin connected to radar sensor interrupt pin */
    cyhal_gpio_t irqpin;
    /** Priority for radar sensor interrupt events */
    uint8_t irq_priority;
    /** MCU pin for SPI MOSI */
    cyhal_gpio_t spi_mosi;
    /** MCU pin for SPI MISO */
    cyhal_gpio_t spi_miso;
    /** MCU pin for SPI clock */
    cyhal_gpio_t spi_clk;
    /** SPI speed in Hz */
    uint32_t spi_speed_hz;
} xensiv_bgt60cutrxx_mtb_iface_config_t;

/**
 * @struct xensiv_bgt60cutrxx_mtb_t
 * @brief Structure to hold the device and interface information for XENSIV BGT60CUTRXX.
 *
 * This structure contains the device instance and the interface instance required
 * to communicate with the XENSIV BGT60CUTRXX radar sensor.
 *
 * @var xensiv_bgt60cutrxx_mtb_t::dev
 * Device instance for XENSIV BGT60CUTRXX.
 *
 * @var xensiv_bgt60cutrxx_mtb_t::iface
 * Interface instance for XENSIV BGT60CUTRXX.
 */
typedef struct
{
    /** Pointer to radar device object */
    xensiv_bgt60cutrxx_t dev;
    /** Pointer to platform interfaces connected to radar device */
    xensiv_bgt60cutrxx_mtb_iface_t iface;
} xensiv_bgt60cutrxx_mtb_t;

/** represents the endianness of sensor SPI interface */
typedef enum
{
    /** SPI MSB Mode */
    MSB,
    /** SPI LSB Mode */
    LSB
} xensiv_bgt60cutrxx_spi_mode_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opens the XENSIV BGT60CUTRXX platform.
 *
 * This function initializes the platform-specific GPIO and SPI interfaces
 * required for communication with the XENSIV BGT60CUTRXX sensor.
 *
 * @param mcu_iface Pointer to the MCU interface.
 * @param config Pointer to the configuration structure containing pin definitions and settings.
 * @return Returns 0 on success, or an error code on failure.
 */
int32_t xensiv_bgt60cutrxx_platform_open(xensiv_bgt60cutrxx_mtb_iface_t* mcu_iface,
                                         xensiv_bgt60cutrxx_mtb_iface_config_t* config);

/**
 * @brief Initializes the XENSIV BGT60CUTRXX hardware platform.
 *
 * This function initializes the XENSIV BGT60CUTRXX platform by performing necessary setups and configuration that
 * depend on the used hardware.
 * It enables the LDO, releases the reset pin, initializes the sensor, set the SPI baudrate to
 * XENSIV_BGT60CUTRXX_MAX_SPEED, downloads the register configuration list, and it performs the ADC calibration.
 *
 * @param radar_sensor Pointer to the XENSIV BGT60CUTRXX object.
 * @param config Pointer to the configuration structure containing pin definitions and settings.
 * @param register_list pointer to an two dimensional array containing the register address and its value
 * @param register_list_len specifies the number of address and value pairs that should be written to the sensor
 * @return Returns 0 on success, or an error code on failure.
 */
int32_t xensiv_bgt60cutrxx_platform_init(xensiv_bgt60cutrxx_mtb_t* radar_sensor,
                                         xensiv_bgt60cutrxx_mtb_iface_config_t* config,
                                         const uint32_t register_list[][2], uint32_t register_list_len);

/**
 * @brief Initializes the platform-specific interrupt for the XENSIV BGT60CUTRXX sensor.
 *
 * This function sets up the interrupt handler for the XENSIV BGT60CUTRXX sensor on the MCU platform.
 * The provided callback function will be called when an interrupt occurs, with two parameters: the first parameter
 * represents the interrupt source, and the second parameter represents additional interrupt information.
 *
 * @param radar_sensor Pointer to the XENSIV BGT60CUTRXX object.
 * @param config Pointer to the configuration structure containing pin definitions and settings.
 * @param callback A function pointer to the callback function that will be called when an interrupt occurs.
 * @param callback_arg An argument that will be passed to the callback function.
 * @return Returns 0 on success, or an error code on failure.
 */
int32_t xensiv_bgt60cutrxx_platform_irq_init(xensiv_bgt60cutrxx_mtb_t* radar_sensor,
                                             xensiv_bgt60cutrxx_mtb_iface_config_t* config,
                                             cyhal_gpio_event_callback_t callback,
                                             void* callback_arg);


#ifdef __cplusplus
}
#endif
#endif /* XENSIV_BGT60CUTRXX_MTB_H_ */
