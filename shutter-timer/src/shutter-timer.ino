/* shutter-timer.ino
A simple mechanism to test the shutters of old cameras
Requires a light source at one end of the camera and a photoresistor / LDR
on the other end.


               +5V
                |
  \             |
   \|    /-------------\
\  -\   /        |      \
 \|    /         |       \
 -\   /          |        \
      |        |/         |
      |        |          |   phototransistor
      |        |\         |
      \          |        /
       \         |       /
        \        |      /
         \-------------/
                |
                |
  +---/\/\/\/\--+-------- Analog pin 0
  |    10kΩ
  |
-----
 ---
ground

A piezo speaker is optional, but nice to get audio feedback that the Arduino
detected the light pulse.

*/

// note that enabling the debugs will throw off timing.
//#define DEBUGRAW
//#define DEBUGSTATE

const byte lightPin       = 0;
const byte thresholdPin   = 1;
const byte speakerPin     = 10;
const int minSamples      = 2;

int lightValue            = 0;
int threshold             = 255;
int lightSamples          = 0;

unsigned long startTime   = 0;
unsigned long endTime     = 0;

float mean                = 0.0;
float deviance            = 0.0;
int count                 = 0;

enum lightState
{
  low,
  rising,
  high,
  falling
};

lightState state = low;

void calcStats(long duration)
{
  float shutter = 1000000.0 / duration;

  count += 1;
  if(count == 1)
  {
    mean = duration;
  }
  else
  {
    // thanks, Donald Knuth! (TAOCP vol 2, 3rd ed, pg 232 or so I've read)
    // calculate running mean and deviance.
    float oldmean = mean;
    mean += (duration - mean) / count;
    deviance += (duration - oldmean) * (duration - mean);
  }

  float avgShutter = 1000000.0 / mean;
  float stdDevSample = sqrt( (deviance / (count - 1) ) );

  Serial.print("Shutter detected: 1/");
  Serial.print(shutter);
  Serial.print("s (");
  Serial.print(duration / 1000.0);
  Serial.print("ms)");

  Serial.print("    (");
  Serial.print(count);
  Serial.print(") 1/");
  Serial.print(avgShutter);
  Serial.print("s µ:");
  Serial.print(mean / 1000.0 );
  Serial.print("ms σ:");
  Serial.print( count == 1 ? 0.0 : stdDevSample / 1000 );
  Serial.println("ms");
}

void debugRawValues()
{
  Serial.print("    DEBUG: light [");
  Serial.print(lightValue);
  Serial.print("] threshold [");
  Serial.print(threshold);
  Serial.println("]");
  delay(10);
}

void setup()
{
  Serial.begin(9600);
  pinMode(speakerPin, OUTPUT);
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(speakerPin, high);
    delay(50);
    digitalWrite(speakerPin, low);
    delay(50);
  }

}

void loop()
{
  lightValue = analogRead(lightPin);
  //threshold  = analogRead(thresholdPin);

  #ifdef DEBUGRAW
    debugRawValues();
  #endif

  /*
    Simple finite state automaton
            _______________
           /               \
          /                 \
    _____/                   \_____
     ^   ^        ^        ^   ^
     |   |        |        |   |
    (a) (b)      (c)      (d) (a)

    (a) low (signal below threshold)
    (b) rising (waiting to see {minSamples} consecutive samples > threshold)
    (c) high (signal above threshold)
    (d) falling (waiting to see {minSamples} consecutive samples < threshold)

    If the device doesn't see {minSamples} in the right direction it reverts
    to the previous state.

  */

  switch (state)
  {
    case low:
      if(lightValue > threshold) // see a signal, change state to rising
      {
        lightSamples++;
        #ifdef DEBUGSTATE
          Serial.println ("DEBUG state: low->rising");
        #endif
        state = rising;
      }
      break;
    case rising:
      if(lightValue > threshold)
      {
        lightSamples++;
        if(lightSamples >= minSamples)
        {
          startTime = micros();
          digitalWrite(speakerPin, high); // start speaker
          lightSamples = 0;
          #ifdef DEBUGSTATE
            Serial.println ("DEBUG state: rising->high");
          #endif
          state = high;
        }
      }
      else // need to see several samples to change state
      {
        lightSamples = 0;
        #ifdef DEBUGSTATE
          Serial.println ("DEBUG state: rising->low");
        #endif
        state = low;
      }
      break;
    case high:
      if(lightValue < threshold)
      {
        lightSamples++;
        #ifdef DEBUGSTATE
          Serial.println ("DEBUG state: high->falling");
        #endif
        state = falling;
      }
      break;
    case falling:
      if(lightValue < threshold)
      {
        lightSamples++;
        if(lightSamples >= minSamples)
        {
          endTime = micros();
          digitalWrite(speakerPin, low); // stop speaker
          #ifdef DEBUGSTATE
            Serial.println ("DEBUG state: falling->low");
          #endif
          state = low;
          lightSamples = 0;
          calcStats(endTime - startTime);
        }
      }
      else // need to see several samples to change state
      {
        #ifdef DEBUGSTATE
          Serial.println ("DEBUG state: falling->high");
        #endif
        state = high;
        lightSamples = 0;
      }
      break;
  }
}
