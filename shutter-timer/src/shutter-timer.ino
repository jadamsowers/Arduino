/* shutter-timer.ino
A simple mechanism to test the shutters of old cameras
Requires a light source at one end of the camera and a photoresistor / LDR
on the other end.

Requires a simple voltage divider circuit:
+5V ---/\/\/\/---+---/\/\/\/---- ground
        LDR      |     10kΩ
                 |
          Arduino pin A0
             (analog)

Also requires a potentiometer to set the light threshold value:
+5V ----------/\/\/\/----------- ground
                 ^
                 |
          Arduino pin A1
             (analog)

A piezo speaker is optional, but nice to get audio feedback that the Arduino
detected the light pulse.

*/

const byte lightPin     = 0;
const byte thresholdPin = 1;
const byte speakerPin   = 10;

int lightValue = 0;
int threshold = 0;
boolean light = false;

unsigned long startMillis = 0;

float mean     = 0.0;
float deviance = 0.0;
int count = 0;

void setup()
{
  Serial.begin(9600);
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
  lightValue = analogRead(lightPin);
  threshold  = analogRead(thresholdPin);

  if(lightValue > threshold) // beginning of shutter event
  {
    if (light == false)
    {
      startMillis = millis();
      light = true;
      digitalWrite(speakerPin, HIGH); // start speaker
    }
  }
  else // lightValue < threshold
  {
    if(light == true) // are we seeing the end of the shutter event?
    {
      unsigned long duration = millis() - startMillis;

      // shutter values are generally shown on cameras as 1/x s
      float shutter = 1000.0 / duration;

      count += 1;
      if(count == 1)
      {
        mean = duration;
      }
      else
      {
        // thanks, Donald Knuth! (TAOCP vol 2, 3rd ed, pg 232 or so I've read)
        float oldmean = mean;
        mean += (duration - mean) / count;
        deviance += (duration - oldmean) * (duration - mean);
      }

      float avgShutter = 1000.0 / mean;
      float stdDevSample = sqrt( (deviance / (count - 1) ) );

      Serial.print("Shutter detected: 1/");
      Serial.print(shutter);
      Serial.print("s (");
      Serial.print(duration);
      Serial.print("ms)");

      Serial.print("    Avg (");
      Serial.print(count);
      Serial.print(") 1/");
      Serial.print(avgShutter);
      Serial.print("s (");
      Serial.print(mean);
      Serial.print("ms) σ:");
      Serial.println(stdDevSample);

      light = false;


    }
    digitalWrite(speakerPin, LOW); // stop speaker
  }

}
