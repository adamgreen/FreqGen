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
#ifndef FREQUENCY_GENERATOR_H_
#define FREQUENCY_GENERATOR_H_

#include <mbed.h>
#include "DmaDac.h"


class FrequencyGenerator : protected DmaDac
{
public:
    FrequencyGenerator(PinName pin);
    ~FrequencyGenerator();

    void start();
    void stop();
    bool isRunning()
    {
        return m_isRunning;
    }

    void setFrequency(uint32_t frequencyHz);
    void setAmplitude(uint32_t amplitudePercentage);

protected:
    enum { SAMPLE_COUNT = 1000 };

    void generateSineWave();
    void refresh();

    uint32_t* m_pSamples;
    uint32_t  m_sineWave[SAMPLE_COUNT];
    uint32_t  m_currSampleCount;
    uint32_t  m_currAmplitude;
    uint32_t  m_frequency;
    uint32_t  m_amplitude;
    bool      m_isRunning;
};

#endif // FREQUENCY_GENERATOR_H_
