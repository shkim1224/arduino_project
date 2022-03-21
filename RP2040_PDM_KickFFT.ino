/*
  This example reads audio data from the on-board PDM microphones, and prints
  out the samples to the Serial console. The Serial Plotter built into the
  Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)

  Circuit:
  - Arduino Nano RP2040 connect board or
  - Arduino Portenta H7 board plus Portenta Vision Shield

  This example code is in the public domain.
*/

#include <PDM.h>
#include "KickFFT.h"

// default number of output channels
static const char channels = 1;

// default PCM output frequency

const uint16_t samples = 64; //This value MUST ALWAYS be a power of 2
const double frequency = 16000;   // this frequency(16Khz) makes 64 samples from PDM module 
const float Fs = frequency;  // FFT 결과의 Full Scale => 16Khz 까지의 주파수 성분 검출 가능 그러나 최종적으로는 8Khz까지의 정보만 볼 수 있음
uint32_t mag[samples] = {0};  // mag[] 어레이에는 총 64개의 FFT 결과가 저장됨 ( onPDMdata() callback 함수에서 저장됨 (c++에서 참조형 변수로 사용됨)
uint16_t startIndex = 0;
uint16_t endIndex = 0;

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[samples];   // 참조형 global 변수 => 여기에 onPDMdata() 콜백함수에서 ad 변환된 음성 데이터의 값이 저장됨 

// Number of audio samples read
volatile int samplesRead;    // 음성신호의 ad 변환 및 sampleBuffer[]로의 저장이 끝났을 때 onPDMdata() 함수로부터 리턴 되는 참조형 변수  

void setup() {
  Serial.begin(9600);
  
  while (!Serial);
  // Configure the data receive callback
  PDM.onReceive(onPDMdata);    // PDM 클래스에 있는 onReceive() method에서 onPDMdata() 콜백함수를 call 함
  // Optionally set the gain
  // Defaults to 20 on the BLE Sense and -10 on the Portenta Vision Shield
  // PDM.setGain(30);
  // Initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate for the Arduino Nano 33 BLE Sense
  // - a 32 kHz or 64 kHz sample rate for the Arduino Portenta Vision Shield
  if (!PDM.begin(channels, frequency)) {  // 16K samples per sec -> 64 samples 
    Serial.println("Failed to start PDM!");
    //while (1);  // 무한 반복 수행
  }
}

void loop() {
  // Wait for samples to be read
  if (samplesRead) {
    KickFFT<int16_t>::fft(Fs, 0, Fs/2, samples, sampleBuffer, mag, startIndex, endIndex);

  //Print to Serial Monitor and copy and paste
  //into a .csv file to display in Excel
  Serial.println("Freq(Hz),Magnitude"); //Use SerialUSB for SparkFun SAMD21 boards
  for(uint16_t i = startIndex; i < endIndex; i++)
  {
    //Peak should be around 1.3 Hz or so
    
    Serial.print(i*Fs/samples); //Use SerialUSB for SparkFun SAMD21 boards
    Serial.print(","); //Use SerialUSB for SparkFun SAMD21 boards
    Serial.print(mag[i]); //Use SerialUSB for SparkFun SAMD21 boards
    Serial.println(); //Use SerialUSB for SparkFun SAMD21 boards
  }
  samplesRead = 0;
  // while(1);  // 한번 수행수행
  }
}

/**
 * Callback function to process the data from the PDM microphone.
 * NOTE: This callback is executed as part of an ISR.
 * Therefore using `Serial` to print messages inside this function isn't supported.
 * */
void onPDMdata() {
  // Query the number of available bytes
  int bytesAvailable = PDM.available();  // call back함수내에서 PDM 모쥴의 사용이 가능함을 check함 가능하다면 byteAvailable = 1이 됨

  // Read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable); // PDM 클래스의 read() static method를 수행함 => 이 것이 수행되면 sampleBuffer 어레이에 음성 샘플이 저장됨 !!!

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;   // sampleRead 도 참조변수로 전역변수임 !!!
}
