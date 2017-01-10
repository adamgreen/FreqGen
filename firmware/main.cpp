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

// Function Prototypes.

//#define FREQUENCY

int main()
{
    static   DigitalOut         myled(LED1);
    static   Timer              ledTimer;
    static   FrequencyGenerator freqGen(p18);
    uint32_t amplitude = 100;
    uint32_t frequency = 1000;
    bool     isDecreasing = true;

    freqGen.start();

    ledTimer.start();
    while(1)
    {
        if (ledTimer.read_ms() >= 250)
        {
            myled = !myled;
            ledTimer.reset();

            if (isDecreasing)
            {
#ifdef FREQUENCY
                frequency-=50;
                if (frequency == 50)
                {
                    isDecreasing = false;
                }
#else
                amplitude--;
                if (amplitude == 0)
                {
                    isDecreasing = false;
                }
#endif
            }
            else
            {
#ifdef FREQUENCY
                frequency+=50;
                if (frequency == 1000)
                {
                    isDecreasing = true;
                }
#else
                amplitude++;
                if (amplitude == 100)
                {
                    isDecreasing = true;
                }
#endif
            }
#ifdef FREQUENCY
            freqGen.setFrequency(frequency);
#else
            freqGen.setAmplitude(amplitude);
#endif
        }
    }
}
