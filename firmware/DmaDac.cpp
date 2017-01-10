/* Copyright (C) 2017  Adam Green (https://github.com/adamgreen)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include <assert.h>
#include <mbed.h>
#include "DmaDac.h"


// This class utilizes DMA based DAC hardware. It was only coded to work on the LPC1768.
#ifndef TARGET_LPC176X
    #error("This DmaDac class was only coded to work on the LPC1768.")
#endif


DmaDac::DmaDac(PinName pin) : AnalogOut(pin)
{
    // Setup GPDMA module.
    enableGpdmaPower();
    enableGpdmaInLittleEndianMode();

    // Allocate DMA channel for transmitting.
    m_channelTx = allocateDmaChannel(GPDMA_CHANNEL_LOW);
    m_pChannelTx = dmaChannelFromIndex(m_channelTx);

    m_isLooping = false;

    // Default to a sample frequency of 100kHz (10 microseconds/sample);
    setSampleTime(10);
}

DmaDac::~DmaDac()
{
    stop();
    freeDmaChannel(m_channelTx);
}

void DmaDac::setSampleTime(uint32_t sampleTimeInNanoSeconds)
{
    // Note: DAC runs at 1/4 the CPU core clock.
    uint32_t dacTicksPerSample = (uint32_t)(((uint64_t)sampleTimeInNanoSeconds * (uint64_t)SystemCoreClock) / (uint64_t)4000000000) - 1;
    LPC_DAC->DACCNTVAL = dacTicksPerSample;
}

void DmaDac::start(uint32_t* pSamples, size_t sampleLength, bool loopSamples)
{
    stop();
    convertSamplesToDacValues(pSamples, sampleLength);

    // Configure DMA channel options required for both looping and non-looping samples.
    uint32_t srcAddr = (uint32_t)pSamples;
    uint32_t destAddr = (uint32_t)&LPC_DAC->DACR;
    uint32_t control  = DMACCxCONTROL_SI |
                     (DMACCxCONTROL_BURSTSIZE_1 << DMACCxCONTROL_SBSIZE_SHIFT) |
                     (DMACCxCONTROL_BURSTSIZE_1 << DMACCxCONTROL_DBSIZE_SHIFT) |
                     (DMACCxCONTROL_WIDTH_WORD << DMACCxCONTROL_SWIDTH_SHIFT) |
                     (DMACCxCONTROL_WIDTH_WORD << DMACCxCONTROL_DWIDTH_SHIFT) |
                     (sampleLength & DMACCxCONTROL_TRANSFER_SIZE_MASK);

    m_pChannelTx->DMACCSrcAddr  = srcAddr;
    m_pChannelTx->DMACCDestAddr = destAddr;
    m_pChannelTx->DMACCControl  = control;
    if (loopSamples)
    {
        // Prepare transmit channel DMA circular linked list.
        m_dmaListItem.DMACCxSrcAddr  = srcAddr;
        m_dmaListItem.DMACCxDestAddr = destAddr;
        m_dmaListItem.DMACCxLLI      = (uint32_t)&m_dmaListItem;
        m_dmaListItem.DMACCxControl  = control;

        m_pChannelTx->DMACCLLI      = (uint32_t)&m_dmaListItem;
    }
    else
    {
        m_pChannelTx->DMACCLLI      = 0;
    }

    // Enable transmit channel.
    m_pChannelTx->DMACCConfig = DMACCxCONFIG_ENABLE |
                   (DMA_PERIPHERAL_DAC << DMACCxCONFIG_DEST_PERIPHERAL_SHIFT) |
                   DMACCxCONFIG_TRANSFER_TYPE_M2P;

    // Turn on DMA transmit requests in DAC.
    static const uint32_t CNT_ENA = (1 << 2);
    static const uint32_t DMA_ENA = (1 << 3);
    LPC_DAC->DACCTRL = CNT_ENA | DMA_ENA;

    m_isLooping = loopSamples;
}

void DmaDac::convertSamplesToDacValues(uint32_t* pSamples, size_t sampleLength)
{
    for (size_t i = 0; i < sampleLength ; i++)
    {
        // NOTE: Keeping BIAS bit cleared to allow for 1MHz operation and clearing out lowest 6 bits.
        pSamples[i] = pSamples[i] & (((1 << 10) -1) << 6);
    }
}

void DmaDac::stop()
{
    haltDma();
    while (isTransferring())
    {
    }
    LPC_DAC->DACCTRL = 0;
    m_pChannelTx->DMACCConfig &= ~DMACCxCONFIG_ENABLE;

    m_isLooping = false;
}

void DmaDac::haltDma()
{
    m_pChannelTx->DMACCConfig |= DMACCxCONFIG_HALT;
}

bool DmaDac::isTransferring()
{
    uint32_t isStillActive = m_pChannelTx->DMACCConfig & DMACCxCONFIG_ACTIVE;
    if (isStillActive)
        return true;

    return false;
}
