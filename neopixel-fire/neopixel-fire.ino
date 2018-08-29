/* 
 *  Make sure you install the Adafruit NeoPixel library first!
 */
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// change to whichever pin you're using on your board
#define PIN 6

// change if you want more variance in your flickering
#define FLICKER_MAX 40

/* 
 *  Change to the number of LEDs in the NeoPixel(s) you're using.
 *  In my case, I'm using a NeoPixel Ring (24 LEDs) and a NeoPixel Jewel (7 LEDs).
 */
#define NEOPIXEL_COUNT 31

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_COUNT - 1, PIN, NEO_GRB + NEO_KHZ800);

/* 
 *  IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
 *  pixel power leads, add 300 – 500 Ohm resistor on first pixel’s data input
 *  and minimize distance between Arduino and first pixel. Avoid connecting
 *  on a live circuit…if you must, connect GND first.
 */

void setup() 
{
  strip.begin();
  strip.show(); // Initialize all pixels to ‘off’
}

void loop() 
{
  int r = 255;
  int g = 96;
  int b = 12;

  /* In my case, I wanted all of the pixels to have the same value so the entire thing would flicker at the same time.
   * If you want different parts to flicker, change the following loop to assign random values to each pixel.
   */
  
  for(int x = 0; x < NEOPIXEL_COUNT; x++)
  {
    int flicker = random(0, FLICKER_MAX);
  
    int r1 = r - flicker;
    int g1 = g - flicker;
    int b1 = b - flicker;
  
    if (r1 < 0) r1 = 0;
    if (g1 < 0) g1 = 0;
    if (b1 < 0) b1 = 0;
  
    strip.setPixelColor(x, r1, g1, b1);
  }

  strip.show(); 
  delay(random(50,150));
}
