/*
  This example reads audio data from the on-board PDM microphones, and prints
  out the samples to the Serial console. The Serial Plotter built into the
  Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)

  Circuit:
  - Arduino Nano 33 BLE board or
  - Arduino Portenta H7 board plus Portenta Vision Shield

  This example code is in the public domain.
*/

#include <PDM.h>
#include "arduinoFFT.h"
arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

// default number of output channels
static const char channels = 1;

// default PCM output frequency
//static const int frequency = 16000;
const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
const double frequency = 16000;   // this frequency makes 64 samples from PDM module
const uint8_t amplitude = 100;
double vReal[samples];
double vImag[samples];
#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[samples];

// Number of audio samples read
volatile int samplesRead;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Configure the data receive callback
  PDM.onReceive(onPDMdata);

  // Optionally set the gain
  // Defaults to 20 on the BLE Sense and -10 on the Portenta Vision Shield
  // PDM.setGain(30);

  // Initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate for the Arduino Nano 33 BLE Sense
  // - a 32 kHz or 64 kHz sample rate for the Arduino Portenta Vision Shield
  if (!PDM.begin(channels, frequency)) {  // 16K samples per sec -> 64 samples 
    Serial.println("Failed to start PDM!");
    while (1);
  }
}

void loop() {
  // Wait for samples to be read
  if (samplesRead) {
      //Serial.println(samplesRead);
     for (uint16_t i = 0; i < samplesRead; i++)
  {
    vReal[i] = sampleBuffer[i]; //  Build data with positive and negative values*/
    vImag[i] = 0.0; //Imaginary part must be zeroed in case of looping to avoid wrong calculations and overflows
  }

  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);  /* Weigh data */
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
  PrintVector(vReal, samples>>1, SCL_PLOT);
  double x = FFT.MajorPeak(vReal, samples, frequency);
    
    // Clear the read count
  samplesRead = 0;
  }
}

/**
 * Callback function to process the data from the PDM microphone.
 * NOTE: This callback is executed as part of an ISR.
 * Therefore using `Serial` to print messages inside this function isn't supported.
 * */
void onPDMdata() {
  // Query the number of available bytes
  int bytesAvailable = PDM.available();

  // Read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
        break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / frequency);
        break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * frequency) / samples);
        break;
    }
    if(scaleType!=SCL_PLOT)
    {
      Serial.print(abscissa, 6);
      if(scaleType==SCL_FREQUENCY)
        Serial.print("Hz");
      Serial.print(" ");
    }
    Serial.println(vData[i], 4);
  }
  Serial.println();
}
