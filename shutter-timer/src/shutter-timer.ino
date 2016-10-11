/* shutter-timer.ino
A simple mechanism to test the shutters of old cameras
Requires a light source at one end of the camera and a photoresistor / LDR
on the other end.



          +5V
           |
  \        |
   \|   -------
\  -\  /    |  \
 \|   /     |   \
 -\  /      |    \
     |    |/     |
     |    |      |   phototransistor
     |    |\     |
     \      |    /
      \     |   /
       \    |  /
        -------
           |
           |
           +-------- Analog pin 0
           |
          <|  -/
           |> /|
          <| /
           |>
          </         potentiometer
          /|>
         /<|
        /  |>
           |
           |
         -----       ground
          ---


Also requires a potentiometer to set the light threshold value:
+5V ----------/\/\/\/----------- ground
                 ^
                 |
          Arduino pin A1
             (analog)

A piezo speaker is optional, but nice to get audio feedback that the Arduino
detected the light pulse.

*/
const byte debugRaw       = false;
const byte debugState     = false;

const byte lightPin       = 0;
const byte thresholdPin   = 1;
const byte speakerPin     = 10;
const int minSamples      = 3;

int lightValue            = 0;
int threshold             = 0;
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
  Serial.print(duration / 1000);
  Serial.print("ms)");

  Serial.print("    Avg (");
  Serial.print(count);
  Serial.print(") 1/");
  Serial.print(avgShutter);
  Serial.print("s (");
  Serial.print(mean / 1000 );
  Serial.print("ms) σ:");
  Serial.println(stdDevSample / 1000);
}

void debugRawValues()
{
  Serial.print("    DEBUG: light [");
  Serial.print(lightValue);
  Serial.print("] threshold [");
  Serial.print(threshold);
  Serial.println("]");
}

void debugStateInfo()
{
  switch (state)
  {
    case low:
      Serial.println ("DEBUG state: LOW");
      break;
    case rising:
      Serial.println ("DEBUG state: RISING");
      break;
    case high:
      Serial.println ("DEBUG state: HIGH");
      break;
    case falling:
      Serial.println ("DEBUG state: FALLING");
      break;
  }
}

void setup()
{
  Serial.begin(19200);
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

  if (debugRaw)   { debugRawValues(); }
  if (debugState) { debugStateInfo(); }

  /*
    Simple state engine
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
        state = rising;
        lightSamples++;
      }
      break;
    case rising:
      if(lightValue > threshold)
      {
        lightSamples++;
        if(lightSamples >= minSamples)
        {
          startTime = micros();
          digitalWrite(speakerPin, HIGH); // start speaker
          state = high;
          lightSamples = 0;
        }
      }
      else // need to see several samples to change state
      {
        state = low;
        lightSamples = 0;
      }
      break;
    case high:
    if(lightValue < threshold)
    {
      state = falling;
      lightSamples++;
    }
    break;
  case falling:
    if(lightValue < threshold)
    {
      lightSamples++;
      if(lightSamples >= minSamples)
      {
        endTime = micros();
        digitalWrite(speakerPin, LOW); // stop speaker
        state = low;
        lightSamples = 0;
        calcStats(endTime - startTime);
      }
    }
    else // need to see several samples to change state
    {
      state = high;
      lightSamples = 0;
    }
  }
}
