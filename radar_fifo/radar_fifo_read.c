/**
 * @file radar_fifo_read.c
 * @brief C file for the XENSIV(TM) BGT60CUTRXX radar FIFO interface implementation.
 *
 * This file provides functionality related to the XENSIV(TM) BGT60CUTRXX sensor,
 * related to FIFO data retrieval.
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

#include "radar_fifo_read.h"
#include "xensiv_bgt60cutrxx_regs.h"

int32_t xensiv_bgt60cutrxx_get_fifo_event(xensiv_bgt60cutrxx_t* dev, xensiv_bgt60cutrxx_fifo_events_t* event)
{
    if ((dev == NULL) || (event == NULL))
    {
        return -1;
    }

    int32_t status;
    uint32_t irq_status;
    *event = FIFO_EVENT_NONE;

    status = xensiv_bgt60cutrxx_get_irq_status(dev, &irq_status);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
    {
        return status;
    }

    uint32_t clear_mask = 0;
    if (irq_status & BGT60CUTR13B_IR_STAT_FIFO_UF_ERR_MSK)
    {
        *event = FIFO_EVENT_UNDERFLOW;
        clear_mask = BGT60CUTR13B_IR_CLEAR_FIFO_UF_CLR_MSK;
    }
    else if (irq_status & BGT60CUTR13B_IR_STAT_FIFO_OF_ERR_MSK)
    {
        *event = FIFO_EVENT_OVERFLOW;
        clear_mask = BGT60CUTR13B_IR_CLEAR_FIFO_OF_CLR_MSK;
    }
    else if (irq_status & BGT60CUTR13B_IR_STAT_FIFO_TH_MSK)
    {
        *event = FIFO_EVENT_THRESHOLD;
        // note: FIFO threshold IRQ flag is cleared after reading the FIFO
    }

    if (clear_mask != 0)
    {
        status |= xensiv_bgt60cutrxx_set_irq_clear(dev, clear_mask);
    }

    return status == XENSIV_BGT60CUTRXX_STATUS_OK ? status : XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
}


int32_t xensiv_bgt60cutrxx_get_fifo_level(xensiv_bgt60cutrxx_t* dev, uint32_t* num_samples_available)
{
    if ((dev == NULL) || (num_samples_available == NULL))
    {
        return -1;
    }

    uint32_t fifo_status = 0;
    int32_t status = xensiv_bgt60cutrxx_get_reg(dev, BGT60CUTR13B_REG_FIFO_STAT, &fifo_status);
    if (XENSIV_BGT60CUTRXX_STATUS_OK != status)
    {
        *num_samples_available = 0;
        return status;
    }

    const uint32_t fifo_level = fifo_status & BGT60CUTR13B_FIFO_STAT_LEVEL_msk;
    *num_samples_available = fifo_level * 2;  // Two 16-bit samples are stored in one 32-bit FIFO word

    return status;
}


int32_t radar_fifo_read(xensiv_bgt60cutrxx_t* dev, uint16_t* frame_buffer, uint32_t num_samples)
{
    const uint32_t num_words = num_samples / 2;  // Two 16-bit samples are stored in one 32-bit FIFO word
    int32_t status = xensiv_bgt60cutrxx_get_fifo(dev, (fifo_entry_t*)frame_buffer, num_words);

    // clear FIFO threshold IRQ flag after reading
    status |= xensiv_bgt60cutrxx_set_irq_clear(dev, BGT60CUTR13B_IR_CLEAR_THRESHOLD_MSK);

    return status == XENSIV_BGT60CUTRXX_STATUS_OK ? status : XENSIV_BGT60CUTRXX_STATUS_DEV_ERROR;
}
