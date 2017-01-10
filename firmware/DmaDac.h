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
#ifndef DMA_DAC_H_
#define DMA_DAC_H_

#include <mbed.h>
#include "GPDMA.h"


class DmaDac : public AnalogOut
{
public:
    DmaDac(PinName pin);
    ~DmaDac();

    void stop();
    void start(uint32_t* pSamples, size_t sampleLength, bool loopSamples);
    void setSampleTime(uint32_t sampleTimeInMicroSeconds);
    bool isTransferring();

protected:
    void convertSamplesToDacValues(uint32_t* pSamples, size_t sampleLength);
    void haltDma();

    LPC_GPDMACH_TypeDef*        m_pChannelTx;
    DmaLinkedListItem           m_dmaListItem;
    uint32_t                    m_channelTx;
    uint32_t                    m_dacTicksPerMicrosecond;
    uint32_t                    m_dacTicksPerSample;
    bool                        m_isLooping;
};

#endif // DMA_DAC_H_
