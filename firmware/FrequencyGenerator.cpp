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
    m_currSampleCount = 0;
    m_currAmplitude = 0;

    generateSineWave();
    setFrequency(1000);
    setAmplitude(100);
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
    m_frequency = frequencyHz;
    refresh();
}

void FrequencyGenerator::setAmplitude(uint32_t amplitudePercentage)
{
    m_amplitude = amplitudePercentage;
    refresh();
}

void FrequencyGenerator::refresh()
{
    uint32_t sampleCount = 0;
    uint32_t sampleTimeInNanoSeconds = 0;
    uint32_t ratio = 0;

    if (!m_isRunning)
        return;

    if (m_frequency <= 1000)
    {
        // Used fixed resolution of 1000 samples per period when frequency <= 1000.
        sampleCount = SAMPLE_COUNT;
        ratio = 1 << 22;
        sampleTimeInNanoSeconds = (1000000000 / sampleCount) / m_frequency;
    }
    else
    {
        // Use variable (lower) resolution of samples per period when frequency > 1000.
        sampleTimeInNanoSeconds = 1000;
        sampleCount = 1000000 / m_frequency;
        ratio = (1000ULL << 22) / sampleCount;
    }

    if (m_currSampleCount != sampleCount)
    {
        DmaDac::stop();
    }

    if (m_currSampleCount != sampleCount || m_currAmplitude != m_amplitude)
    {
        uint32_t offset = 32768 - ((32768 * m_amplitude) / 100);
        uint32_t srcIndex = 0;
        for (uint32_t i = 0 ; i < sampleCount ; i++)
        {
            // Round fixed point value and convert to integer.
            uint32_t src = (srcIndex + (1 << 21)) >> 22;
            m_pSamples[i] = ((m_sineWave[src] * m_amplitude) / 100) + offset;
            srcIndex += ratio;
        }
    }

    setSampleTime(sampleTimeInNanoSeconds);

    if (m_currSampleCount != sampleCount)
    {
        DmaDac::start(m_pSamples, sampleCount, true);
    }

    m_currSampleCount = sampleCount;
    m_currAmplitude = m_amplitude;
}

void FrequencyGenerator::start()
{
    m_isRunning = true;
    refresh();
}

void FrequencyGenerator::stop()
{
    DmaDac::stop();
    m_currSampleCount = 0;
    m_isRunning = false;
}
