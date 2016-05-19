#include <Arduino.h>

/*
 *  Using an Arduino to drive a piezoelectric speaker
 *  Normally these speakers emit a single tone but by bit-banging
 *  we can approximate other tones via PWM
 *
 *  Expects a linear potentiometer output on the analogIn pin (0-5v)
 *  and a piezo on the speaker pin. Add another pot to the speaker pin
 *  for volume control. You'll thank me later.
 *
 *  the v[] array represents a series of durations (in μs) for which
 *  the pin should be held HIGH, and then held LOW for the same amount
 *  of time. This should look like a square wave.
 *
 *  The analogIn signal (0-1023) is scaled to the size of the array,
 *  and the appropriate duration is a simple lookup rather than a
 *  computation.
 */



// 128 steps of more or less continuous tones
/*
const long v[] = {100000, 50000, 33333, 25000, 20000, 16666, 14285, 12500,
                  11111, 10000, 9090, 8333, 7692, 7142, 6666, 6250,
                  5882, 5555, 5263, 5000, 4761, 4545, 4347, 4166,
                  4000, 3846, 3703, 3571, 3448, 3333, 3225, 3125,
                  3030, 2941, 2857, 2777, 2702, 2631, 2564, 2500,
                  2439, 2380, 2325, 2272, 2222, 2173, 2127, 2083,
                  2040, 2000, 1960, 1923, 1886, 1851, 1818, 1785,
                  1754, 1724, 1694, 1666, 1639, 1612, 1587, 1562,
                  1538, 1515, 1492, 1470, 1449, 1428, 1408, 1388,
                  1369, 1351, 1333, 1315, 1298, 1282, 1265, 1250,
                  1234, 1219, 1204, 1190, 1176, 1162, 1149, 1136,
                  1123, 1111, 1098, 1086, 1075, 1063, 1052, 1041,
                  1030, 1020, 1010, 1000, 990, 980, 970, 961,
                  952, 943, 934, 925, 917, 909, 900, 892,
                  884, 877, 869, 862, 854, 847, 840, 833,
                  826, 819, 813, 806, 800, 793, 787, 781};
*/


// C major octave: C3,   D3,   E3,   F3,   G3,   A4,   B4,   C4
const long v[] = {3822, 3405, 3033, 2863, 2551, 2272, 2024, 1911};

const int analogIn = A0;
const int speaker = 9;
const int led = 13;

// beware: DEBUG throws off the timing
const int DEBUG = false;

void setup()
{
  pinMode(analogIn, INPUT);
  pinMode(speaker, OUTPUT);
  if(DEBUG) Serial.begin(115200);

  // Scale the step size based on the number of elements in the v[] array.
  // analog i/o on Arduino range is 0-1023.
  int scaling = (1024 * sizeof(long)) / sizeof(v);

  // hack; supposedly loops run tighter in the setup() function than loop() ¯\_(ツ)_/¯
  while(true)
  {
    int  a = 0;
    long d = 0;
    a = analogRead(analogIn) / scaling;
    d = v[a];
    if(DEBUG) debugValues(a, d);

    // simulating a square wave: on for v[a] μs, off for v[a] μs
    digitalWrite(speaker, HIGH);
    delayMicroseconds(d);
    digitalWrite(speaker, LOW);
    delayMicroseconds(d);

  }
}

void debugValues(int a, int d)
{
  // you crazy, arduino string concatenation
  String debug = "[" + a;
  debug = debug + "] -> [" + d;
  debug = debug + "]";
  Serial.println(debug);
}

void loop()
{
  // this function never runs since it never leaves setup()
}
