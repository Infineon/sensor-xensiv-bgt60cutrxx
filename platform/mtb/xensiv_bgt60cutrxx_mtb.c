/**
 * @file xensiv_bgt60cutrxx_mtb.c
 * @brief Implementation file for XENSIV BGT60CUTRXX sensor integration with PSoC and Airoc.
 *
 * This file contains the implementation of functions and macros required for
 * integrating the XENSIV BGT60CUTRXX sensor with a PSoC MCUs.
 *
 * @note Ensure that the correct file path is used when including this file in your project.
 */


/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdint.h>
#include "cybsp.h"
#include "cyhal.h"

/**
 * Macro to enable or disable debug print statements
 * If the macro DEBUG_PRINT_DISABLE is defined, then no code will be generated for the debug_print macro.
 * If the macro DEBUG_PRINT_ENABLE is defined, then printf and retarget io will be used  for printing debug messages.
 *
 * Note: the DEBUG_PRINT_ENABLE macro is usually defined in the Makefile of the application if debug printing is
 * desired.
 */
#ifdef DEBUG_PRINT_ENABLE
#include "cy_retarget_io.h"
#endif

#include "xensiv_bgt60cutrxx_mtb.h"
#include "xensiv_bgt60cutrxx_platform.h"
#include "xensiv_bgt60cutrxx_regs.h"

/*******************************************************************************
* Macros
*******************************************************************************/

/*******************************************************************************
* Global Variables
*******************************************************************************/

/*******************************************************************************
* Types
*******************************************************************************/

/*******************************************************************************
* Function Definitions
*******************************************************************************/

static int32_t xensiv_bgt60cutrxx_platform_gpio_init(xensiv_bgt60cutrxx_mtb_iface_config_t* config);
static int32_t xensiv_bgt60cutrxx_platform_spi_init(xensiv_bgt60cutrxx_mtb_iface_t* mcu_iface,
                                                    xensiv_bgt60cutrxx_mtb_iface_config_t* config);

/**
 * @brief Initializes the XENSIV BGT60CUTRXX platform GPIO pins.
 *
 * This function initializes the GPIO pins required for the XENSIV BGT60CUTRXX sensor,
 * including the LDO enable pin, SPI chip select pin, reset pin, and other necessary
 * GPIO pins based on the target platform.
 *
 * @param config Pointer to the configuration structure containing pin definitions and settings.
 * @return int32_t Returns XENSIV_BGT60CUTRXX_STATUS_OK (0) on success, otherwise returns
 *                 XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR.
 */
static int32_t xensiv_bgt60cutrxx_platform_gpio_init(xensiv_bgt60cutrxx_mtb_iface_config_t* config)
{
    CY_ASSERT(config->selpin != NC);
    CY_ASSERT(config->rstpin != NC);

    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Initialize CS Pin */
    result |= cyhal_gpio_init(config->selpin, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);

    /* Initialize Radar RST pin */
    result |= cyhal_gpio_init(config->rstpin, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);

    return ((CY_RSLT_SUCCESS == result) ?
            XENSIV_BGT60CUTRXX_STATUS_OK :
            XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR);
}


/**
 * @brief Initializes the XENSIV BGT60CUTRXX platform SPI interface.
 *
 * This function initializes the SPI interface for the XENSIV BGT60CUTRXX sensor
 * and configures the SPI pins and frequency.
 *
 * @param mcu_iface Pointer to the MCU interface.
 * @param config Pointer to the configuration structure containing pin definitions and settings.
 * @return int32_t Returns XENSIV_BGT60CUTRXX_STATUS_OK (0) on success, otherwise returns
 *                 XENSIV_BGT60CUTRXX_STATUS_SPI_ERROR.
 */
static int32_t xensiv_bgt60cutrxx_platform_spi_init(xensiv_bgt60cutrxx_mtb_iface_t* mcu_iface,
                                                    xensiv_bgt60cutrxx_mtb_iface_config_t* config)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    result = cyhal_spi_init(&mcu_iface->spi,
                            config->spi_mosi,
                            config->spi_miso,
                            config->spi_clk,
                            NC,
                            NULL,
                            16U,
                            CYHAL_SPI_MODE_00_LSB,
                            false);
    if (CY_RSLT_SUCCESS != result)
    {
        DEBUG_PRINT("Error initializing SPI\n");
        return XENSIV_BGT60CUTRXX_STATUS_SPI_ERROR;
    }

    #if defined(COMPONENT_CAT1A) || defined(COMPONENT_CAT1B) || defined(COMPONENT_CAT1C)
    // Reduce drive strength to improve EMI (CAT1 devices only)
    Cy_GPIO_SetSlewRate(CYHAL_GET_PORTADDR(config->spi_mosi),
                        CYHAL_GET_PIN(config->spi_mosi), CY_GPIO_SLEW_FAST);
    Cy_GPIO_SetDriveSel(CYHAL_GET_PORTADDR(config->spi_mosi),
                        CYHAL_GET_PIN(config->spi_mosi), CY_GPIO_DRIVE_1_8);
    Cy_GPIO_SetSlewRate(CYHAL_GET_PORTADDR(config->spi_clk),
                        CYHAL_GET_PIN(config->spi_clk), CY_GPIO_SLEW_FAST);
    Cy_GPIO_SetDriveSel(CYHAL_GET_PORTADDR(config->spi_clk),
                        CYHAL_GET_PIN(config->spi_clk), CY_GPIO_DRIVE_1_8);
    #endif

    result = cyhal_spi_set_frequency(&mcu_iface->spi, config->spi_speed_hz);
    if (CY_RSLT_SUCCESS != result)
    {
        DEBUG_PRINT("Error setting spi frequency\n");
        return XENSIV_BGT60CUTRXX_STATUS_SPI_ERROR;
    }

    return XENSIV_BGT60CUTRXX_STATUS_OK;
}


/*******************************************************************************
 * Public interface implementation
 ********************************************************************************/

int32_t xensiv_bgt60cutrxx_platform_open(xensiv_bgt60cutrxx_mtb_iface_t* mcu_iface,
                                         xensiv_bgt60cutrxx_mtb_iface_config_t* config)
{
    CY_ASSERT(mcu_iface != NULL);
    CY_ASSERT(config != NULL);

    int32_t result;

    result = xensiv_bgt60cutrxx_platform_gpio_init(config);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != result)
    {
        DEBUG_PRINT("Error initializing platform GPIOs\n");
        return result;
    }

    result = xensiv_bgt60cutrxx_platform_spi_init(mcu_iface, config);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != result)
    {
        DEBUG_PRINT("Error initializing platform SPI\n");
    }

    return result;
}


int32_t xensiv_bgt60cutrxx_platform_init(xensiv_bgt60cutrxx_mtb_t* radar_sensor,
                                         xensiv_bgt60cutrxx_mtb_iface_config_t* config,
                                         const uint32_t register_list[][2],
                                         uint32_t register_list_len)
{
    int32_t result;

    radar_sensor->iface.selpin = config->selpin;
    radar_sensor->iface.rstpin = config->rstpin;

    // we have a 80MHz crystal on the board
    // and the VDDIO voltage level of the sensor is 1.8V
    result = xensiv_bgt60cutrxx_init(&radar_sensor->dev, &radar_sensor->iface.spi, BGT60CUTRXX_CLK_80MHZ,
                                     BGT60CUTRXX_IO_LEVEL_1V8);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != result)
    {
        DEBUG_PRINT("xensiv_bgt60cutrxx_init failed\n");
        return result;
    }

    result = xensiv_bgt60cutrxx_detect_device(&radar_sensor->dev);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != result)
    {
        DEBUG_PRINT("xensiv_bgt60cutrxx_detect_device failed\n");
        return result;
    }

    /* Write the given register list to the sensor */
    result = xensiv_bgt60cutrxx_config(&radar_sensor->dev, register_list, register_list_len);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != result)
    {
        DEBUG_PRINT("xensiv_bgt60cutrxx_config failed\n");
        return result;
    }
    DEBUG_PRINT("XENSIV radar configuration done\n");

    /* Wait for sensor to apply configuration */
    xensiv_bgt60cutrxx_platform_delay(50);

    /* Perform ADC calibration */
    result = xensiv_bgt60cutrxx_perform_adc_calibration(&radar_sensor->dev);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != result)
    {
        uint32_t adc_err;
        xensiv_bgt60cutrxx_get_reg(&radar_sensor->dev, BGT60CUTR13B_REG_ADC_ERR_CTRL, &adc_err);
        DEBUG_PRINT("xensiv_bgt60cutrxx_perform_adc_calibration error: 0x%08lX\n", adc_err);
        return result;
    }

    return XENSIV_BGT60CUTRXX_STATUS_OK;
}


int32_t xensiv_bgt60cutrxx_platform_irq_init(xensiv_bgt60cutrxx_mtb_t* radar_sensor,
                                             xensiv_bgt60cutrxx_mtb_iface_config_t* config,
                                             cyhal_gpio_event_callback_t callback,
                                             void* callback_arg)
{
    cy_rslt_t result = cyhal_gpio_init(config->irqpin, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, 0);
    if (CY_RSLT_SUCCESS != result)
    {
        DEBUG_PRINT("Error initializing IRQ pin\n");
        return XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
    }

    radar_sensor->iface.irq_callback_data.pin = config->irqpin;
    radar_sensor->iface.irq_callback_data.callback = callback;
    radar_sensor->iface.irq_callback_data.callback_arg = callback_arg;

    // Register and enable interrupt
    cyhal_gpio_register_callback(config->irqpin, &radar_sensor->iface.irq_callback_data);

    // Configure IRQ pin for rising edge trigger
    cyhal_gpio_enable_event(config->irqpin, CYHAL_GPIO_IRQ_RISE, config->irq_priority, true);

    return XENSIV_BGT60CUTRXX_STATUS_OK;
}


/*******************************************************************************
 * Platform functions implementation
 ********************************************************************************/
void xensiv_bgt60cutrxx_platform_delay(uint32_t ms)
{
    (void)cyhal_system_delay_ms(ms);
}


void xensiv_bgt60cutrxx_platform_delay_us(uint32_t us)
{
    (void)cyhal_system_delay_us(us);
}


void xensiv_bgt60cutrxx_platform_rst_set(const void* iface, bool val)
{
    CY_ASSERT(iface != NULL);

    const xensiv_bgt60cutrxx_mtb_iface_t* mcu_iface = iface;

    CY_ASSERT(mcu_iface->rstpin != NC);

    cyhal_gpio_write(mcu_iface->rstpin, val);
}


void xensiv_bgt60cutrxx_platform_spi_cs_set(const void* iface, bool val)
{
    CY_ASSERT(iface != NULL);

    const xensiv_bgt60cutrxx_mtb_iface_t* mcu_iface = iface;

    CY_ASSERT(mcu_iface->selpin != NC);

    cyhal_gpio_write(mcu_iface->selpin, val);
}


int32_t xensiv_bgt60cutrxx_platform_spi_read_block(void* iface, const uint16_t* txd, uint16_t* rxd, uint32_t len)
{
    int32_t result = XENSIV_BGT60CUTRXX_STATUS_OK;

    result = xensiv_bgt60cutrxx_platform_spi_transfer16(iface, txd, NULL, 2);
    if (XENSIV_BGT60CUTRXX_STATUS_OK == result)
    {
        result = xensiv_bgt60cutrxx_platform_spi_transfer16(iface, NULL, rxd, len);
    }
    return result;
}


int32_t xensiv_bgt60cutrxx_platform_spi_transfer16(void* iface, const uint16_t* txd, uint16_t* rxd, uint32_t len)
{
    xensiv_bgt60cutrxx_mtb_iface_t* mcu_iface = (xensiv_bgt60cutrxx_mtb_iface_t*)iface;

    /* MISRA 11.8: Cy_SCB_SPI_Transfer does not modify txd, but API doesn't use const */
    uint16_t* txd_non_const = (uint16_t*)(uintptr_t)txd;
    cy_en_scb_spi_status_t status =
        Cy_SCB_SPI_Transfer(mcu_iface->spi.base, txd_non_const, rxd, len, &(mcu_iface->spi.context));

    if (CY_SCB_SPI_SUCCESS == status)
    {
        while (0UL !=
               (CY_SCB_SPI_TRANSFER_ACTIVE &
                Cy_SCB_SPI_GetTransferStatus(mcu_iface->spi.base, &(mcu_iface->spi.context))))
        {
        }
    }

    return ((CY_SCB_SPI_SUCCESS == status) ?
            XENSIV_BGT60CUTRXX_STATUS_OK :
            XENSIV_BGT60CUTRXX_STATUS_SPI_ERROR);
}
