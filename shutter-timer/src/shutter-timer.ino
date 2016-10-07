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

void setup()
{
  Serial.begin(9600);
  pinMode(speakerPin, OUTPUT);
  digitalWrite(speakerPin, LOW);
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
    }
    digitalWrite(speakerPin, HIGH); // start speaker
  }
  else // lightValue < threshold
  {
    if(light == true) // are we seeing the end of the shutter event?
    {
      unsigned long duration = millis() - startMillis;

      // shutter values are listed in 1/x s
      float shutter = 1000.0 / duration;

      Serial.print("Shutter detected: 1/");
      Serial.print(shutter);
      Serial.print("s (");
      Serial.print(duration);
      Serial.println("ms)");
      light = false;

    }
    digitalWrite(speakerPin, LOW); // stop speaker
  }

}
