/**
 * @file xensiv_bgt60cutrxx.c
 * @brief This file contains the implementation of the XENSIV BGT60CUTRXX module.
 *
 * The XENSIV BGT60CUTRXX module provides functions to interact with the BGT60CUTRXX sensor.
 * It includes functions for initialization, configuration, and data retrieval from the sensor.
 * This module is specifically designed for the MCU7 Radar project.
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


#include <xensiv_bgt60cutrxx_regs.h>
#include "xensiv_bgt60cutrxx.h"
#include "xensiv_bgt60cutrxx_platform.h"

/**
 * \addtogroup bgt60cutrxx_driver BGT60CUTR13 Driver
 * @{
 */

/** Defines the maximum number of bytes that can be read from the FIFO within one block read.*/
#define FIFO_MAX_SPI_BURST_SIZE_BYTES (BGT60CUTR13B_RD_MAP_ADDRESS - BGT60CUTR13B_FIFO_ADDRESS)
/** Defines the maximum number of bytes that can be read from RX Buffer.*/
#define RX_BUFFER_MAX_SIZE_BYTES 0x6000
/** Radar write register sequence length in words */
#define REG_WRITE_CMD_WORDS 2
/** Radar read register sequence length in words */
#define REG_READ_CMD_WORDS 2
/** Word size in bytes */
#define REG_SIZE_WORDS 2
/** Radar write register and value length in words*/
#define REG_WRITE_VALUE_WORDS (REG_WRITE_CMD_WORDS + REG_SIZE_WORDS)

/** defines the waiting time for initiating the hardware reset*/
#define BGT60CUTR13_RESET_DURATION_TIME_MS 1


/** Read command address sequence */
#define SMARTAR_READ(address)                   \
{                                           \
    (((uint16_t)((address) & 0xFFU)) << 8), ((address) >> 8) \
}
/** Write command address sequence */
#define SMARTAR_WRITE(address, value)                        \
{                                                        \
    (1U << 0) | (((uint16_t)((address) & 0xFFU)) << 8), ((address) >> 8), \
    (uint16_t)((value) & 0xFFFFLU), (uint16_t)((value) >> 16)                  \
}


/**
 * @brief Reads a block of registers from the XENSIV(TM) BGT60CUTRXX sensor.
 *
 * This function reads a block of registers starting from the specified address
 * and stores the values in the provided data array.
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @param start_address The starting address (in bytes) of the register block to read.
 * @param data Pointer to the array where the read register values will be stored. Each register is 32 bits.
 * @param length The number of bytes to read. This values must be a multiple of 4 bytes.
 * @return int32_t Returns 0 on success, or a negative error code on failure
 */
static int32_t xensiv_bgt60cutrxx_read_register_block(xensiv_bgt60cutrxx_t* dev, uint16_t start_address, uint32_t* data,
                                                      uint32_t length);

/**
 * This function checks if the read hardware id matches the expected values of the bgt60cutrxx
 *
 * @param id: register content of BGT60CUTRxx_REG_ID_HW
 * @return true if the bgt60cutr13B is connected
 */

static bool xensiv_bgt60cutrxx_is_bgt60cutrxx(uint32_t id)
{
    if (id == BGT60CUTR13B_REG_ID_HW_RESET_VAL)
    {
        return true;
    }
    else
    {
        return false;
    }
}


/**
 * This function writes the clock registers of radar for device startup,
 * By default, the radar sensor uses LSB first for configuration
 *
 * @param dev Pointer to the XENSIV(TM) BGT60CUTRXX device structure.
 * @return int32_t Returns 0 on success, or a negative error code on failure.
 */
static int32_t xensiv_bgt60cutrxx_startup(xensiv_bgt60cutrxx_t* dev)
{
    if (dev == NULL)
    {
        return XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
    }

    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;

    if (dev->clk_freq_xtal == BGT60CUTRXX_CLK_40MHZ)
    {
        xensiv_bgt60cutrxx_set_reg(dev, BGT60CUTR13B_REG_CLK_FD, BGT60CUTR13B_REG_CLK_FD_VAL_40MHz);
    }
    else
    {
        xensiv_bgt60cutrxx_set_reg(dev, BGT60CUTR13B_REG_CLK_FD, BGT60CUTR13B_REG_CLK_FD_VAL_80MHz);
    }

    xensiv_bgt60cutrxx_set_reg(dev, BGT60CUTR13B_REG_CLK_RCOSC_CTRL, BGT60CUTR13B_REG_CLK_RCOSC_CTRL_VAL);
    // delay is required to ensure that the sensor clock is started
    xensiv_bgt60cutrxx_platform_delay(BGT60CUTRxx_CLK_SETUP_TIME_MS);
    xensiv_bgt60cutrxx_set_reg(dev, BGT60CUTR13B_REG_CLK_WUC, BGT60CUTR13B_REG_CLK_WUC_VAL);
    // delay is required to ensure that the sensor clock is started and access to the registers is possible
    xensiv_bgt60cutrxx_platform_delay(BGT60CUTRxx_CLK_SETUP_TIME_MS);

    // after the clock are set, the IO voltage level of the sensor must be set
    status = xensiv_bgt60cutrxx_set_io_level(dev, dev->io_level);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
    {
        DEBUG_PRINT("xensiv_bgt60cutrxx_set_io_level failed\n");
    }

    return status;
}


int32_t xensiv_bgt60cutrxx_detect_device(xensiv_bgt60cutrxx_t* dev)
{
    uint32_t id = 0;

    int32_t status = xensiv_bgt60cutrxx_get_reg(dev, (BGT60CUTR13B_REG_ID_HW), &id);
    if (XENSIV_BGT60CUTRXX_STATUS_OK == status)
    {
        if (xensiv_bgt60cutrxx_is_bgt60cutrxx(id))
        {
            dev->device_id = id;
        }
        else
        {
            status = XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
        }
    }
    return status;
}


int32_t xensiv_bgt60cutrxx_init(xensiv_bgt60cutrxx_t* dev, void* iface, xensiv_bgt60cutrxx_clk_t clk,
                                xensiv_bgt60cutrxx_io_level_t io_level)
{
    if ((dev == NULL) || (iface == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
    }

    uint32_t spi_ctrl_val = 0;
    dev->iface = iface;
    dev->clk_freq_xtal = clk;
    dev->io_level = io_level;

    // perform hard reset before initializing radar via SPI
    xensiv_bgt60cutrxx_hard_reset(dev);

    // Detect the device
    xensiv_bgt60cutrxx_get_reg(dev, BGT60CUTR13B_REG_SPI_CTRL, &spi_ctrl_val);

    if (spi_ctrl_val == BGT60CUTR13B_REG_SPI_CTRL_RESET_VAL)
    {
        DEBUG_PRINT("Xensiv radar device detected\n\r");
    }
    else
    {
        DEBUG_PRINT("No device detected\n\r");
        return -1;
    }

    int32_t status = xensiv_bgt60cutrxx_startup(dev);

    return status;
}


int32_t xensiv_bgt60cutrxx_set_reg(xensiv_bgt60cutrxx_t* dev, uint16_t addr, uint32_t val)
{
    if ((dev == NULL) || (dev->iface == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
    }

    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    uint16_t temp[REG_WRITE_VALUE_WORDS] = SMARTAR_WRITE(addr, val);

    xensiv_bgt60cutrxx_platform_spi_cs_set(dev->iface, CS_ACTIVE);
    status = xensiv_bgt60cutrxx_platform_spi_transfer16(dev->iface, temp, NULL, REG_WRITE_VALUE_WORDS);

    xensiv_bgt60cutrxx_platform_spi_cs_set(dev->iface, CS_INACTIVE);

    if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
    {
        status = XENSIV_BGT60CUTRXX_STATUS_SPI_ERROR;
    }
    return status;
}


int32_t xensiv_bgt60cutrxx_get_reg(xensiv_bgt60cutrxx_t* dev, uint16_t addr, uint32_t* val)
{
    if ((dev == NULL) || (dev->iface == NULL) || (val == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
    }

    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    const uint16_t read_request[REG_READ_CMD_WORDS] = SMARTAR_READ(addr);

    xensiv_bgt60cutrxx_platform_spi_cs_set(dev->iface, CS_ACTIVE);
    // transfer the read request command
    status = xensiv_bgt60cutrxx_platform_spi_transfer16(dev->iface, read_request, NULL, REG_READ_CMD_WORDS);
    if (XENSIV_BGT60CUTRXX_STATUS_OK == status)
    {
        // read the data
        status = xensiv_bgt60cutrxx_platform_spi_transfer16(dev->iface, NULL, (uint16_t*)val, REG_SIZE_WORDS);
    }
    xensiv_bgt60cutrxx_platform_spi_cs_set(dev->iface, CS_INACTIVE);

    if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
    {
        status = XENSIV_BGT60CUTRXX_STATUS_SPI_ERROR;
    }
    return status;
}


void xensiv_bgt60cutrxx_hard_reset(xensiv_bgt60cutrxx_t* dev)
{
    if ((dev == NULL) || (dev->iface == NULL))
    {
        return;
    }

    xensiv_bgt60cutrxx_platform_rst_set(dev->iface, RESET_ACTIVE);

    xensiv_bgt60cutrxx_platform_delay(BGT60CUTR13_RESET_DURATION_TIME_MS);
    xensiv_bgt60cutrxx_platform_rst_set(dev->iface, RESET_INACTIVE);

    xensiv_bgt60cutrxx_platform_delay(RESET_DELAY_MS);
}


int32_t xensiv_bgt60cutrxx_soft_reset(xensiv_bgt60cutrxx_t* dev, xensiv_bgt60cutrxx_reset_t reset_type)
{
    return xensiv_bgt60cutrxx_set_reg(dev, BGT60CUTR13B_REG_CGU_RST, reset_type);
}


int32_t xensiv_bgt60cutrxx_config(xensiv_bgt60cutrxx_t* dev, const uint32_t regs[][2], uint32_t len)
{
    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    for (uint32_t reg_idx = 0; reg_idx < len; ++reg_idx)
    {
        status = xensiv_bgt60cutrxx_set_reg(dev, (uint16_t)regs[reg_idx][0], regs[reg_idx][1]);
        if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
        {
            break;
        }
    }
    return status;
}


int32_t xensiv_bgt60cutrxx_perform_adc_calibration(xensiv_bgt60cutrxx_t* dev)
{
    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    uint32_t ir_stat = 0;
    uint32_t value = 0;
    int32_t timeout = CALIBRATION_TIMEOUT_US;

    // enable ADC calibration
    status = xensiv_bgt60cutrxx_get_reg(dev, BGT60CUTR13B_REG_CLK_WUC, &value);
    if (XENSIV_BGT60CUTRXX_STATUS_OK == status)
    {
        value = value | (BGT60CUTR13B_CLK_WUC_SEQ3_TRIG_TRIGGER << BGT60CUTR13B_CLK_WUC_SEQ3_TRIG_pos);
        status = xensiv_bgt60cutrxx_set_reg(dev, BGT60CUTR13B_REG_CLK_WUC, value);
    }

    if (XENSIV_BGT60CUTRXX_STATUS_OK == status)
    {
        // wait until calibration sequence execution is completed
        while (timeout > 0)    // if the time lapsed is less than timeout
        {
            // wait for 100us to give the sensor time to execute the sequence
            xensiv_bgt60cutrxx_platform_delay_us(ADC_CALIBRATION_POLLING_INTERVALL_US);
            timeout -= ADC_CALIBRATION_POLLING_INTERVALL_US;
            // replace by reading the ir_stat register and check the sequence done bit 11
            status = xensiv_bgt60cutrxx_get_reg(dev, BGT60CUTR13B_REG_IR_STAT, &ir_stat);
            if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
            {
                DEBUG_PRINT("Failed to read IR_STAT register during ADC calibration\n\r");
                return status;
            }
            if (ir_stat & BGT60CUTR13B_IR_STAT_SEQ3_DONE_MSK)
            {
                // adc is calibrated
                break;
            }
        }
        if (timeout <= 0)
        {
            DEBUG_PRINT("ADC calibration timeout: ir_stat: 0x%08lX\n\r", ir_stat);
            return XENSIV_BGT60CUTRXX_STATUS_TIMEOUT_ERROR;
        }
    }
    //read calibration error, check bits 25-28
    // if error bit is set, then read ADCx_ERRFLAG_STAT
    if (ir_stat &
        (BGT60CUTR13B_IR_STAT_SADC_ERR_MSK | BGT60CUTR13B_IR_STAT_ADC1_ERR_MSK | BGT60CUTR13B_IR_STAT_ADC2_UF_ERR_MSK |
         BGT60CUTR13B_IR_STAT_ADC3_OF_ERR_MSK))
    {
        DEBUG_PRINT("ADC calibration error detected: ir_stat: 0x%08lX\n\r", ir_stat);
        return XENSIV_BGT60CUTRXX_STATUS_ADC_CALIBRATION_ERROR;
    }

    return status;
}


int32_t xensiv_bgt60cutrxx_trigger_sequence0(xensiv_bgt60cutrxx_t* dev)
{
    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    uint32_t value = 0;

    status = xensiv_bgt60cutrxx_get_reg(dev, BGT60CUTR13B_REG_CLK_WUC, &value);
    if (XENSIV_BGT60CUTRXX_STATUS_OK == status)
    {
        value = value |
                (BGT60CUTR13B_CLK_WUC_SEQ0_TRIG_TRIGGER <<
                 BGT60CUTR13B_CLK_WUC_SEQ0_TRIG_pos) |
                (BGT60CUTR13B_CLK_WUC_PURQ_TRIG << BGT60CUTR13B_CLK_WUC_PURQ_pos);
        status = xensiv_bgt60cutrxx_set_reg(dev, BGT60CUTR13B_REG_CLK_WUC, value);
    }

    return status;
}


int32_t xensiv_bgt60cutrxx_set_motion_detection(xensiv_bgt60cutrxx_t* dev, uint16_t timer_index, bool enable)
{
    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;

    uint16_t gtmr_ctrl_addr = BGT60CUTR13B_REG_GTMR0_CTRL + timer_index *
                              (BGT60CUTR13B_REG_GTMR1_CTRL - BGT60CUTR13B_REG_GTMR0_CTRL);

    // initializes timer used for measurement sequence
    uint32_t gtmr_ctrl =
        (BGT60CUTR13B_GTMR0_CTRL_SRC_SEL_SW <<
         BGT60CUTR13B_GTMR0_CTRL_SRC_SEL_pos) | (BGT60CUTR13B_GTMR0_CTRL_EN_ENABLED << BGT60CUTR13B_GTMR0_CTRL_EN_pos);

    if (enable)
    {
        status = xensiv_bgt60cutrxx_set_reg(dev, gtmr_ctrl_addr, gtmr_ctrl);
        if (XENSIV_BGT60CUTRXX_STATUS_OK == status)
        {
            // triggers timer used for measurement sequence
            gtmr_ctrl =
                (BGT60CUTR13B_GTMR0_CTRL_SRC_SEL_SW <<
                 BGT60CUTR13B_GTMR0_CTRL_SRC_SEL_pos) |
                (BGT60CUTR13B_GTMR0_CTRL_TRIG_TRIGGER <<
                 BGT60CUTR13B_GTMR0_CTRL_TRIG_pos) |
                (BGT60CUTR13B_GTMR0_CTRL_EN_ENABLED << BGT60CUTR13B_GTMR0_CTRL_EN_pos);
            status = xensiv_bgt60cutrxx_set_reg(dev, gtmr_ctrl_addr, gtmr_ctrl);
        }
    }
    else
    {
        status = xensiv_bgt60cutrxx_set_reg(dev, gtmr_ctrl_addr, BGT60CUTR13B_REG_GTMR0_CTRL_rst);
    }

    return status;
}


int32_t xensiv_bgt60cutrxx_get_xosc_state(xensiv_bgt60cutrxx_t* dev)
{
    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    uint32_t clk_state = 0;

    status = xensiv_bgt60cutrxx_get_reg(dev, BGT60CUTR13B_REG_CLK_STAT, &clk_state);
    if (XENSIV_BGT60CUTRXX_STATUS_OK == status)
    {
        dev->xosc_active = (clk_state & BGT60CUTR13B_CLK_STAT_XOSC_STATE_msk) ==
                           BGT60CUTR13B_CLK_STAT_XOSC_STATE_ACTIVE;
    }

    return status;
}


int32_t xensiv_bgt60cutrxx_set_xosc(xensiv_bgt60cutrxx_t* dev, bool val)
{
    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    uint32_t reg = 0;

    if (val)
    {
        status = xensiv_bgt60cutrxx_get_reg(dev, BGT60CUTR13B_REG_CLK_WUC, &reg);
        if (XENSIV_BGT60CUTRXX_STATUS_OK == status)
        {
            reg = (reg & ~BGT60CUTR13B_CLK_WUC_PURQ_msk) |
                  (BGT60CUTR13B_CLK_WUC_PURQ_TRIG<<BGT60CUTR13B_CLK_WUC_PURQ_pos);
            status = xensiv_bgt60cutrxx_set_reg(dev, BGT60CUTR13B_REG_CLK_WUC, reg);
        }
    }
    else
    {
        status = xensiv_bgt60cutrxx_get_reg(dev, BGT60CUTR13B_REG_CLK_WUC, &reg);
        if (XENSIV_BGT60CUTRXX_STATUS_OK == status)
        {
            reg = (reg & ~BGT60CUTR13B_CLK_WUC_PDRQ_msk) |
                  (BGT60CUTR13B_CLK_WUC_PDRQ_TRIG<<BGT60CUTR13B_CLK_WUC_PDRQ_pos);
            status = xensiv_bgt60cutrxx_set_reg(dev, BGT60CUTR13B_REG_CLK_WUC, reg);
            if (XENSIV_BGT60CUTRXX_STATUS_OK == status)
            {
                dev->xosc_active = 0;
            }
        }
    }

    return status;
}


int32_t xensiv_bgt60cutrxx_get_irq_status(xensiv_bgt60cutrxx_t* dev, uint32_t* val)
{
    return xensiv_bgt60cutrxx_get_reg(dev, BGT60CUTR13B_REG_IR_STAT, val);
}


int32_t xensiv_bgt60cutrxx_set_irq_clear(xensiv_bgt60cutrxx_t* dev, uint32_t val)
{
    return xensiv_bgt60cutrxx_set_reg(dev, BGT60CUTR13B_REG_IR_CLEAR, val);
}


int32_t xensiv_bgt60cutrxx_get_md_zones(xensiv_bgt60cutrxx_t* dev, uint32_t* zones, uint16_t zone_size)
{
    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    uint16_t md_zones_addr = BGT60CUTR13B_REG_MD_ZONE_STAT0;

    for (uint16_t i = 0; i < zone_size; i++)
    {
        status = xensiv_bgt60cutrxx_get_reg(dev, md_zones_addr, &zones[i]);
        md_zones_addr += sizeof(uint32_t);
        if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
        {
            break;
        }
    }
    return status;
}


int32_t xensiv_bgt60cutrxx_get_md_targets(xensiv_bgt60cutrxx_t* dev, uint32_t* targets, uint16_t targets_size)
{
    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    uint16_t md_targets_addr = BGT60CUTR13B_REG_MD_TARGET0_MAG;

    // read 32 registers from this base address of target registers
    for (uint16_t i = 0; i < targets_size; i++)
    {
        status = xensiv_bgt60cutrxx_get_reg(dev, md_targets_addr, &targets[i]);
        md_targets_addr += sizeof(uint32_t);
        if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
        {
            break;
        }
    }

    return status;
}


int32_t xensiv_bgt60cutrxx_get_rd_map(xensiv_bgt60cutrxx_t* dev, uint32_t* rd_data)
{
    if ((dev == NULL) || (dev->iface == NULL) || (rd_data == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
    }

    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;

    const uint16_t temp[REG_READ_CMD_WORDS] = SMARTAR_READ(BGT60CUTR13B_RD_MAP_ADDRESS);

    xensiv_bgt60cutrxx_platform_spi_cs_set(dev->iface, CS_ACTIVE);
    status = xensiv_bgt60cutrxx_platform_spi_read_block(dev->iface, temp, (uint16_t*)rd_data, RX_BUFFER_MAX_SIZE_BYTES);
    xensiv_bgt60cutrxx_platform_spi_cs_set(dev->iface, CS_INACTIVE);

    return status;
}


int32_t xensiv_bgt60cutrxx_get_fifo(xensiv_bgt60cutrxx_t* dev, fifo_entry_t* data, uint32_t num_of_fifo_entries)
{
    if ((dev == NULL) || (dev->iface == NULL) || (data == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
    }

    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    uint32_t bytes_to_read = num_of_fifo_entries * sizeof(fifo_entry_t);

    while (bytes_to_read > 0)
    {
        uint32_t len;
        if (bytes_to_read >= FIFO_MAX_SPI_BURST_SIZE_BYTES)
        {
            len = FIFO_MAX_SPI_BURST_SIZE_BYTES;
        }
        else
        {
            len = bytes_to_read;
        }

        xensiv_bgt60cutrxx_platform_spi_cs_set(dev->iface, CS_ACTIVE);
        const uint16_t read_request[REG_READ_CMD_WORDS] = SMARTAR_READ(BGT60CUTR13B_FIFO_ADDRESS);
        status =
            xensiv_bgt60cutrxx_platform_spi_read_block(dev->iface, read_request, (uint16_t*)data,
                                                       len / sizeof(uint16_t));
        xensiv_bgt60cutrxx_platform_spi_cs_set(dev->iface, CS_INACTIVE);
        if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
        {
            break;
        }
        bytes_to_read -= len;
        data += (len / sizeof(fifo_entry_t));
    }

    return status;
}


int32_t xensiv_bgt60cutrxx_set_io_level(xensiv_bgt60cutrxx_t* dev, xensiv_bgt60cutrxx_io_level_t level)
{
    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;
    uint32_t reg = 0;
    uint32_t value = 0;

    if (level == BGT60CUTRXX_IO_LEVEL_1V2)
    {
        value = BGT60CUTR13B_IO_CTRL_IO_LVL_1V2;
    }
    else
    {
        value = BGT60CUTR13B_IO_CTRL_IO_LVL_1V8;
    }

    status = xensiv_bgt60cutrxx_get_reg(dev, BGT60CUTR13B_REG_IO_CTRL, &reg);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
    {
        return status;
    }

    reg = (reg & ~BGT60CUTR13B_IO_CTRL_IO_LVL_msk) | (value << BGT60CUTR13B_IO_CTRL_IO_LVL_pos);
    status = xensiv_bgt60cutrxx_set_reg(dev, BGT60CUTR13B_REG_IO_CTRL, reg);

    return status;
}


int32_t xensiv_bgt60cutrxx_read_calibration_data(xensiv_bgt60cutrxx_t* dev,
                                                 xensiv_bgt60cutrxx_calibration_data_t* calibration_settings,
                                                 uint32_t* calibration_data)
{
    if ((dev == NULL) || (calibration_settings == NULL) || (calibration_data == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
    }

    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;

    status = xensiv_bgt60cutrxx_read_register_block(dev, calibration_settings->rx0_address, &calibration_data[0],
                                                    calibration_settings->bytes_to_read);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
    {
        return status;
    }

    status =
        xensiv_bgt60cutrxx_read_register_block(dev, calibration_settings->rx1_address,
                                               &calibration_data[calibration_settings->bytes_to_read/sizeof(uint32_t)],
                                               calibration_settings->bytes_to_read);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
    {
        return status;
    }

    status =
        xensiv_bgt60cutrxx_read_register_block(dev, calibration_settings->rx2_address,
                                               &calibration_data[2*calibration_settings->bytes_to_read/sizeof(uint32_t)],
                                               calibration_settings->bytes_to_read);
    return status;
}


static int32_t xensiv_bgt60cutrxx_read_register_block(xensiv_bgt60cutrxx_t* dev, uint16_t start_address, uint32_t* data,
                                                      uint32_t length)
{
    if ((dev == NULL) || (dev->iface == NULL) || (data == NULL))
    {
        return XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
    }

    int32_t status = XENSIV_BGT60CUTRXX_STATUS_OK;

    const uint16_t start_cmd[REG_READ_CMD_WORDS] = SMARTAR_READ(start_address);

    xensiv_bgt60cutrxx_platform_spi_cs_set(dev->iface, CS_ACTIVE);
    status = xensiv_bgt60cutrxx_platform_spi_read_block(dev->iface, start_cmd, (uint16_t*)data,
                                                        length / sizeof(uint16_t));
    xensiv_bgt60cutrxx_platform_spi_cs_set(dev->iface, CS_INACTIVE);

    return status;
}


/**@} end of group bgt60cutrxx_driver*/
