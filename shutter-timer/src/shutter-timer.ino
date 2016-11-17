
/* shutter-timer.ino
A simple mechanism to test the shutters of old cameras
Requires a light source at one end of the camera and a photoresistor / LDR
on the other end.

Now using a simple 555 timer circuit acting as a Schmitt trigger.
A couple advantages to this method:
 * Digital signal eliminates the slow, noisy analogRead() functions
 * No need to debounce the signal due to hysteresis

A piezo speaker is optional, but nice to get audio feedback that the Arduino
detected the light pulse.

---

I used these libraries to get my OLED screen working.
https://github.com/adafruit/Adafruit_SSD1306.git
https://github.com/adafruit/Adafruit-GFX-Library.git

*/

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>


// note that enabling the debugs will throw off timing.
//#define DEBUGRAW
//#define DEBUGSTATE

const byte lightPin       = 7;
const byte speakerPin     = 10;

bool lightValue           = false;
bool trigger              = false;

unsigned long startTime   = 0;
unsigned long endTime     = 0;

float mean                = 0.0;
float deviance            = 0.0;
int count                 = 0;


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

  Serial.print("Shutter: 1/"); Serial.print(shutter);
  Serial.print("s (");   Serial.print(duration / 1000.0);
  Serial.print("ms)");   Serial.print("    ("); Serial.print(count);
  Serial.print(") 1/");  Serial.print(avgShutter);
  Serial.print("s µ:");  Serial.print(mean / 1000.0 );
  Serial.print("ms σ:"); Serial.print( count == 1 ? 0.0 : stdDevSample / 1000 );
  Serial.println("ms");
}


void setup()
{
  Serial.begin(9600);
  pinMode(lightPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  for (int i = 0; i < 5; i++)
  {
    digitalWrite(speakerPin, HIGH);
    delay(50);
    digitalWrite(speakerPin, LOW);
    delay(50);
  }
}

void loop()
{
  lightValue = digitalRead(lightPin);
  //Serial.println(lightValue);

  if (!lightValue) // the output from the 555 is inverted from what we expect
  {
    if(!trigger)
    {
      // this is the first time we've seen the signal go low
      startTime = micros();
      trigger = true;
      digitalWrite(speakerPin, HIGH); // start speaker
    }
  }
  else
  {
    if(trigger)
    {
      // this is the first time we've seen the signal go high
      endTime = micros();
      trigger = false;
      digitalWrite(speakerPin, LOW); // stop speaker

      calcStats(endTime - startTime);
    }
  }
}
