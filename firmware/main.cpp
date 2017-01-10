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
#include <ctype.h>
#include <mbed.h>
#include "FrequencyGenerator.h"


#define AMPLITUDE_MIN 0
#define AMPLITUDE_MAX 100
#define FREQUENCY_MAX 100000
#define FREQUENCY_MIN 1


static Serial            g_serial(USBTX, USBRX);
static volatile uint32_t g_frequency = 1000;
static volatile uint32_t g_amplitude = 50;
static volatile bool     g_charsEchoed = false;


// Function Prototypes.
static void serialRxHandler(void);


int main()
{
    static   DigitalOut         myled(LED1);
    static   Timer              ledTimer;
    static   FrequencyGenerator freqGen(p18);
    uint32_t                    lastFrequency = 0;
    uint32_t                    lastAmplitude = 0;

    g_serial.baud(230400);
    g_serial.attach(serialRxHandler);

    freqGen.start();
    ledTimer.start();
    while(1)
    {
        uint32_t currFrequency = g_frequency;
        uint32_t currAmplitude = g_amplitude;

        if (currFrequency != lastFrequency)
        {
            freqGen.setFrequency(currFrequency);
            printf("%sFrequency=%lu\r\n", g_charsEchoed ? "\r\n" : "", currFrequency);
            lastFrequency = currFrequency;
            g_charsEchoed = false;
        }

        if (currAmplitude != lastAmplitude)
        {
            freqGen.setAmplitude(currAmplitude);
            printf("%sAmplitude=%lu%%\r\n", g_charsEchoed ? "\r\n" : "", currAmplitude);
            lastAmplitude = currAmplitude;
            g_charsEchoed = false;
        }

        if (ledTimer.read_ms() >= 250)
        {
            myled = !myled;
            ledTimer.reset();
        }
    }
}

static void serialRxHandler(void)
{
    static uint32_t frequency = 0;

    while (g_serial.readable())
    {
        char curr = g_serial.getc();
        char lower = tolower(curr);

        if (isdigit(curr))
        {
            g_serial.putc(curr);
            g_charsEchoed = true;

            frequency = frequency * 10 + (curr - '0');
        }
        else if (curr == '\n')
        {
            if (frequency > FREQUENCY_MAX)
                frequency = FREQUENCY_MAX;
            if (frequency < FREQUENCY_MIN)
                frequency = FREQUENCY_MIN;

            g_serial.putc('\r');
            g_serial.putc('\n');
            g_charsEchoed = false;

            g_frequency = frequency;
            frequency = 0;
        }
        else if (lower == 'a' && g_frequency > FREQUENCY_MIN)
        {
            g_frequency--;
            frequency = 0;

        }
        else if (lower == 'd' && g_frequency < FREQUENCY_MAX)
        {
            g_frequency++;
            frequency = 0;
        }
        else if ((curr == '+' || lower == 'w') && g_amplitude < AMPLITUDE_MAX)
        {
            g_amplitude++;
            frequency = 0;
        }
        else if ((curr == '-' || lower == 's') && g_amplitude > AMPLITUDE_MIN)
        {
            g_amplitude--;
            frequency = 0;
        }
    }
}
