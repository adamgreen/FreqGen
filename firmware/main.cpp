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
#include "DmaDac.h"


#define ONE_MICROSECOND 1
#define SAMPLE_COUNT    1000

// Function Prototypes.


int main()
{
    static   DigitalOut myled(LED1);
    static   Timer      ledTimer;
    static   DmaDac     dac(p18);
    uint32_t*           pSamples;

    pSamples = (uint32_t*)dmaHeap0Alloc(sizeof(*pSamples) * SAMPLE_COUNT);
    for (size_t i = 0 ; i < SAMPLE_COUNT ; i++)
    {
        pSamples[i] = 32767.5f + 32767.5f * sinf((float)i * (2.0f * M_PI / SAMPLE_COUNT));
    }
    dac.setSampleTime(ONE_MICROSECOND);
    dac.start(pSamples, SAMPLE_COUNT, true);

    ledTimer.start();
    while(1)
    {
        if (ledTimer.read_ms() >= 250)
        {
            myled = !myled;
            ledTimer.reset();
        }
    }
}
