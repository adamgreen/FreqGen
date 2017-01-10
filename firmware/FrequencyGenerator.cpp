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
#include <mbed.h>
#include "FrequencyGenerator.h"


FrequencyGenerator::FrequencyGenerator(PinName pin) : DmaDac(pin)
{
    m_pSamples = (uint32_t*)dmaHeap0Alloc(sizeof(*m_pSamples) * SAMPLE_COUNT);
    m_isRunning = false;

    generateSineWave();
    setAmplitude(100);
    setFrequency(1000);
}

void FrequencyGenerator::generateSineWave()
{
    for (size_t i = 0 ; i < SAMPLE_COUNT ; i++)
    {
        m_sineWave[i] = 32767.5f + 32767.5f * sinf((float)i * (2.0f * M_PI / SAMPLE_COUNT));
    }
}

FrequencyGenerator::~FrequencyGenerator()
{
}

void FrequencyGenerator::setFrequency(uint32_t frequencyHz)
{
    uint32_t sampleTimeInNanoSeconds = (1000000000 / SAMPLE_COUNT) / frequencyHz;
    setSampleTime(sampleTimeInNanoSeconds);
}

void FrequencyGenerator::setAmplitude(uint32_t amplitudePercentage)
{
    uint32_t offset = 32768 - ((32768 * amplitudePercentage) / 100);
    for (int i = 0 ; i < SAMPLE_COUNT ; i++)
    {
        m_pSamples[i] = ((m_sineWave[i] * amplitudePercentage) / 100) + offset;
    }
}

void FrequencyGenerator::start()
{
    DmaDac::start(m_pSamples, SAMPLE_COUNT, true);
    m_isRunning = true;
}

void FrequencyGenerator::stop()
{
    DmaDac::stop();
    m_isRunning = false;
}
