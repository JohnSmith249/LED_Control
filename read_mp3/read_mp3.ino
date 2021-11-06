#define FASTLED_ESP8266_RAW_PIN_ORDER<br>#include 
"FastLED.h"<br>FASTLED_USING_NAMESPACE<br><br>#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif<br>#include "Wire.h"<br>#define BLYNK_PRINT Serial<br><br>#include "ESP8266WiFi.h"<br>#include "BlynkSimpleEsp8266.h"<br>#include 
"SimpleTimer.h"<br>SimpleTimer timer;<br>/*********************************** FastLED Defintions ********************************/<br>#define NUM_LEDSB    28<br>#define NUM_LEDSA    33<br>#define NUM_LEDST    19<br>#define NUM_LEDSC    9
#define NUM_LEDS    (NUM_LEDSB + NUM_LEDST + NUM_LEDSC)<br>#define DATA_PIN    5
#define COLOR_ORDER BRG
#define SLAVE_ADDRESS 8
#define NUM_DATA 8<br>/********************************GLOBAL SETTINGS**********************************/struct CRGB leds [NUM_LEDS];
struct CRGB ledsT[NUM_LEDST];
struct CRGB ledsB[NUM_LEDSB];
struct CRGB ledsC[NUM_LEDSC];
int mode = 2;<br>const int allEffects = 15;<br>int newMode;<br>bool ambi = false;<br>int music = 0;
int brightness = 255;
struct CRGB color;
int speed = 100;
/**********************************************BLYNK*************************************************/r auth[] = "bff47b8282bf4e129226fe8ab96460b2";
char ssid[] = "WLAN-DC2A79";
char pass[] = "7190774630806407";<br>BLYNK_WRITE(V1){brightness = param.asInt(); delay(10);}<br>

BLYNK_WRITE(V0){if(param.asInt()==0)
{
      if (mode != -1) {change(-1);}
    }
  else{
    Blynk.syncVirtual(V3);
  }
}

BLYNK_WRITE(V6) { // when Button Widget or Web API called
  if (param.asInt() == 1) {
    digitalWrite(15, HIGH);
    timer.setTimeout(200L, []() {  // reset pin in 200ms
    digitalWrite(15, LOW);
    });
  }
}

BLYNK_WRITE(V2)
{
  speed = param.asInt();
}

BLYNK_WRITE(V3)
{
  int in = param.asInt();
  if(in!=mode)
  {
    change(in);
  }
}
BLYNK_WRITE(V4)
{
  ambi = param.asInt() == 1;
}
BLYNK_WRITE(V5)
{
  music = (param.asInt()-1) % 5 ;
}

BLYNK_WRITE(V10)
{
  color.r = param[0].asInt();
  color.g = param[1].asInt();
  color.b = param[2].asInt();

if(14!=mode)
  {
    change(14);
  }
}
/******************************** GLOBALS for fade *******************************/

CRGB oldLeds[NUM_LEDS];
const int fadeTime = 1200;
unsigned long fadeStart = 0;
bool fading = true;
const bool directFade = false;

/********************************** GLOBALS for EFFECTS ******************************/
//RAINBOW
uint8_t thishue = 0;                                          // Starting hue value.
uint8_t deltahue = 10;

//CANDYCANE
CRGBPalette16 currentPalettestriped;

//NOISE
static uint16_t dist;         // A random number for our noise generator.
const uint16_t scale = 30;          // Wouldn't recommend changing this on the fly, or the animation will be really blocky.
const uint8_t maxChanges = 38;      // Value for blending between palettes.
CRGBPalette16 targetPalette(OceanColors_p);
CRGBPalette16 currentPalette(CRGB::Black);

//TWINKLE
#define DENSITY     80
int twinklecounter = 0;

//RIPPLE
uint8_t colour;                                               // Ripple colour is randomized.
int center = 0;                                               // Center of the current ripple.
int step = -1;                                                // -1 is the initializing step.
uint8_t myfade = 255;                                         // Starting brightness.
#define maxsteps 16                                           // Case statement wouldn't allow a variable.
uint8_t bgcol = 0;                                            // Background colour rotates.
int thisdelay = 20;                                           // Standard delay value.
int rippleFramerate = 40;

//LIGHTNING
uint8_t frequency = 50;                                       // controls the interval between strikes
uint8_t flashes = 8;                                          //the upper limit of flashes per strike
unsigned int dimmer = 1;
uint8_t ledstart;                                             // Starting location of a flash
uint8_t ledlen;
int lightningcounter = 0;

//FUNKBOX
int idex = 0;                //-LED INDEX (0 to NUM_LEDS-1
int TOP_INDEX = int(NUM_LEDS / 2);
int thissat = 255;           //-FX LOOPS DELAY VAR
uint8_t thishuepolice = 0;
int antipodal_index(int i) {
  int iN = i + TOP_INDEX;
  if (i >= TOP_INDEX) {
    iN = ( i + TOP_INDEX ) % NUM_LEDS;
  }
  return iN;
}

//FIRE
#define COOLING  60
#define SPARKING 128
const bool gReverseDirection = true;
CRGBPalette16 gPal = HeatColors_p; //gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White); // for blue flames
bool advancedFire = false;
const uint8_t fireFramerate = 40;

//BPM
uint8_t gHue = 0;
uint8_t beatsPerMinute = 48;

//POLICE
int policeFramerate = 55;

//CONFETTI
int confettiFramerate = 60;

/**********************************************MUSIC************************************/
uint8_t volume = 0;   //Holds the volume level read from the sound detector.
uint8_t last = 0;
float avgBump = 0;    //Holds the "average" volume-change to trigger a "bump."
float avgVol = 0;
bool bump = false;
float avgTime = 0;
float maxVol = 15;
float timeBump = 0;

int table[] = {0, 0, 0}; //the data will be transmited via table as to allow different data to be transfer.

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3); 
  Blynk.syncVirtual(V10);

Serial.println("successfully connected, values are up to date");
}

void setup() {
  Wire.begin (D3, D4);

  FastLED.addLeds(ledsB, NUM_LEDSB).setCorrection(TypicalLEDStrip);
  FastLED.addLeds(ledsT, NUM_LEDST).setCorrection(TypicalLEDStrip);
  FastLED.addLeds(ledsC, NUM_LEDSC).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);

setupStripedPalette( CRGB::Red, CRGB::Red, CRGB::White, CRGB::White); //for CANDY CANE

fill_rainbow(ledsB, NUM_LEDSB, 0, 10);
  FastLED.show();
  delay(1000);
  LEDS.showColor(CRGB(0, 0, 0));

Serial.begin(115200);
  delay(20);
  Blynk.begin(auth, ssid, pass);
  Serial.println("exiting setup");
}

void loop() {
  Blynk.run();

if (!ambi)
  {
    FastLED.setBrightness(brightness);
    random16_add_entropy(rand());

/*EVERY_N_SECONDS(20) {
      if (mode != -1)
        change(mode + 1);
      }*/

switch (mode)
    {
      default: FastLED.showColor(CRGB(255, 255, 255)); break;
      case -1: memset(leds, 0, 3 * NUM_LEDS * sizeof(uint8_t)); break;
      case 0: fire(); advancedFire = false; break;
      case 1: fire(); advancedFire = true; break;
      case 2: bpm(); break;
      case 3: policeAll(); break;
      case 4: rainbowSwitchUpdate(); EVERY_N_SECONDS(2) {
          RainbowSwitch();
        } break;
      case 5: rainbow(); break;
      case 6: sinelon(); break;
      case 7: confetti(); break;
      case 8: cyclonRainbow(); break;
      case 9: ripple(); break;
      case 10: noise(); break;
      case 11: lightning(); break;
      case 12: candyCane(); break;
      case 13: policeOne(); break;
      case 14: fill_solid(leds, NUM_LEDS, color); break;
    }
    showLeds();
  }
  else {
    FastLED.setBrightness(255);
    EVERY_N_MILLISECONDS(5) {
      getAmbilightValues();
      switch (music) {
        default: showLeds(); break;
        case 0: showLeds(); break;
        case 1: adjustForMusic(); break;
        case 2: getBpm(); break;
        case 3: averageMusic(); break;
        case 4: importedMusic(); break;
      }
    }
  }
  EVERY_N_MILLISECONDS( 35 ) {
    gHue++;
  }
}

//*********************************ChangeEffect***************************************

void change(int in)
{
  fading = true;
  fadeStart = millis();
  for (int i = 0; i < NUM_LEDS; i++)
  {
    oldLeds[i] = leds[i];
  }

mode = in % allEffects;
}

//*********************************FADE***********************************************

void showLeds()
{
  //CRGB tmpLeds[NUM_LEDS];
  if (fading)
  {
    unsigned long currentMillis = millis();
    unsigned int diff = currentMillis - fadeStart;

float fraction = (float)diff / (float)fadeTime;
    uint8_t amountOfFade = fraction * 255;

if (directFade) {
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = blend(oldLeds[i], leds[i], amountOfFade);
        //leds[i] = CRGB(amountOfFade, amountOfFade, amountOfFade);
      }
    }
    else {
      if (amountOfFade < 85) {
        for (int i = 0; i < NUM_LEDS; i++)
        {
          leds[i] = blend(oldLeds[i], CRGB(0, 0, 0), amountOfFade * 3);
        }
      }
      else if (amountOfFade < 128) {
        memset(leds, 0, 3 * NUM_LEDS * sizeof(uint8_t));
      }
      else {
        for (int i = 0; i < NUM_LEDS; i++)
        {
          leds[i] = blend(CRGB(0, 0, 0), leds[i], amountOfFade * 2 - 256);
        }
      }
    }

 if (diff >= fadeTime)
    {
      fading = false;
      twinklecounter = 0;
    }
  }

/*********************************set the independent led arrays, get the order right******************************/
  for (int i = 0; i < NUM_LEDST; i++)
  {
    ledsT[i] = leds[NUM_LEDSB - 1 + i];
  }
  for (int i = 0; i < NUM_LEDSB; i++)
  {
    ledsB[i] = leds[NUM_LEDSB - 1 - i];
  }
  for (int i = 0; i < NUM_LEDSC; i++)
  {
    ledsC[i] = leds[NUM_LEDSB + NUM_LEDST - 1 + i];
  }

  FastLED.show();
}
//*********************************EFFECTS*********************************************

void fire()
{
  EVERY_N_MILLISECONDS( 1000 / fireFramerate ) {
    if (advancedFire)
    { static uint8_t hue = 0;
      hue++;
      CRGB darkcolor  = CHSV(hue, 255, 192); // pure hue, three-quarters brightness
      CRGB lightcolor = CHSV(hue, 128, 255); // half 'whitened', full brightness
      gPal = CRGBPalette16( CRGB::Black, darkcolor, lightcolor, CRGB::White); \
    }

static byte heat[NUM_LEDS];

// Step 1.  Cool down every cell a little
    for ( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }

 // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for ( int k = NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if ( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160, 255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for ( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      if ( gReverseDirection ) {
        pixelnumber = (NUM_LEDS - 1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }

    //FastLED.delay(1000/fireFramerate);
  }
}

void bpm()
{
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(beatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void policeAll()
{
  EVERY_N_MILLISECONDS(1000 / policeFramerate) {
    idex++;
    if (idex >= NUM_LEDS) {
      idex = 0;
    }
    int idexR = idex;
    int idexB = antipodal_index(idexR);
    int thathue = (thishuepolice + 160) % 255;
    leds[idexR] = CHSV(thishuepolice, thissat, 255);
    leds[idexB] = CHSV(thathue, thissat, 255);
  }
}

void policeOne()
  {
  EVERY_N_MILLISECONDS(1000 / policeFramerate) {
    idex++;
    if (idex >= NUM_LEDS) {
      idex = 0;
    }
    int idexR = idex;
    int idexB = antipodal_index(idexR);
    int thathue = (thishuepolice + 160) % 255;
    for (int i = 0; i < NUM_LEDS; i++ ) {
      if (i == idexR) {
        leds[i] = CHSV(thishuepolice, thissat, 255);
      }
      else if (i == idexB) {
        leds[i] = CHSV(thathue, thissat, 255);
      }
      else {
        leds[i] = CHSV(0, 0, 0);
      }
    }
  }
  }

void rainbow()
{
  EVERY_N_MILLISECONDS(speed) {
    thishue++;
  }
  fill_rainbow(leds, NUM_LEDS, thishue, deltahue);
}

void sinelon()
{
  fadeToBlackBy( leds, NUM_LEDS, 2);
  int pos = beatsin16((15 * speed) / 100, 0, NUM_LEDS - 1);
  leds[pos] += CHSV( gHue, 255, 255);
}

void confetti()
{
  EVERY_N_MILLISECONDS(1000 / confettiFramerate) {
    fadeToBlackBy( leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV( gHue + random8(64), 200, 255);
  }
}

void cyclonRainbow()
{
  static uint8_t hue = 0;
  // First slide the led in one direction
  for (int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to red
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    showLeds();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
  for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
    // Set the i'th led to red
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    showLeds();
    // now that we've shown the leds, reset the i'th led to black
    // leds[i] = CRGB::Black;
    fadeall();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }
}

void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);  //for CYCLon
  }
}

void ripple()
{
  EVERY_N_MILLISECONDS(1000 / rippleFramerate) {
    for (int i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(bgcol++, 255, 15);  // Rotate background colour.
    switch (step) {
      case -1:                                                          // Initialize ripple variables.
        center = random(NUM_LEDS);
        colour = random8();
        step = 0;
        break;
      case 0:
        leds[center] = CHSV(colour, 255, 255);                          // Display the first pixel of the ripple.
        step ++;
        break;
      case maxsteps:                                                    // At the end of the ripples.
        step = -1;
        break;
      default:                                                             // Middle of the ripples.
        leds[(center + step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, myfade / step * 2);   // Simple wrap from Marc Miller
        leds[(center - step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, myfade / step * 2);
        step ++;                                                         // Next step.
        break;
    }
  }
}

void noise()
{
  for (int i = 0; i < NUM_LEDS; i++) {                                     // Just onE loop to fill up the LED array as all of the pixels change.
    uint8_t index = inoise8(i * scale, dist + i * scale) % 255;            // Get a value from the noise function. I'm using both x and y axis.
    leds[i] = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }
  dist += beatsin8(10, 1, 4);

EVERY_N_MILLISECONDS(10) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);  // FOR NOISE ANIMATIon
    {
      gHue++;
    }
  }
  EVERY_N_SECONDS(5) {
    targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));
  }
}

void lightning()
{
  twinklecounter++;                     //Resets strip if previous animation was running
  if (twinklecounter <= 2) {
    FastLED.clear();
    FastLED.showColor(CRGB(0, 0, 0));
    FastLED.show();
  }
  ledstart = random8(NUM_LEDS - 1);         // Determine starting location of flash
  ledlen = random8(NUM_LEDS - ledstart);  // Determine length of flash (not to go beyond NUM_LEDS-1)
  for (int flashCounter = 0; flashCounter < random8(3, flashes); flashCounter++) {
    if (flashCounter == 0) dimmer = 5;    // the brightness of the leader is scaled down by a factor of 5
    else dimmer = random8(1, 3);          // return strokes are brighter than the leader
    fill_solid(leds + ledstart, ledlen, CHSV(255, 0, 255 / dimmer));
    showLeds();    // Show a section of LED's
    delay(random8(4, 10));                // each flash only lasts 4-10 milliseconds
    fill_solid(leds + ledstart, ledlen, CHSV(255, 0, 0)); // Clear the section of LED's
    showLeds();
    if (flashCounter == 0) delay (130);   // longer delay until next flash after the leader
    delay(50 + random8(100));             // shorter delay between strokes
  }
  delay(random8(frequency) * 100);
}

void candyCane()
{
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* higher = faster motion */
  fill_palette( leds, NUM_LEDS,
                startIndex, 16, /* higher = narrower stripes */
                currentPalettestriped, 255, LINEARBLEND);
}

void RainbowSwitch()
{
  leds[0] = ColorFromPalette(RainbowStripeColors_p, 50, 255, LINEARBLEND);
}

void rainbowSwitchUpdate()
{
  for (int i = 0; i < NUM_LEDS - 1; i++)
  {
    leds[i + 1] = leds[i];
  }
}
/**************************** START STRIPLED PALETTE *****************************************/
void setupStripedPalette( CRGB A, CRGB AB, CRGB B, CRGB BA) {
  currentPalettestriped = CRGBPalette16(
                            A, A, A, A, A, A, A, A, B, B, B, B, B, B, B, B
                            //    A, A, A, A, A, A, A, A, B, B, B, B, B, B, B, B
                          );
}

/**************************** AMBILIGHT **********************************************/
void getAmbilightValues()
{
  Wire.beginTransmission(8); // transmit to device #8
  Wire.write("x5");      // restarts the transmission loop
  Wire.endTransmission();

 for (int i = 0; i < NUM_LEDSA; i++)
  {
    Wire.requestFrom(SLAVE_ADDRESS, 3);// request 3 bytes from slave device #8
    for (int i = 0; i < 3; i++) //organizes the data from the slave in the table
    {
      int c = Wire.read(); // receive a byte as an int
      table[i] = c;
    }
    if (i < NUM_LEDSA)
    {
      leds[i].r = table[0];
      leds[i].g = table[1];
      leds[i].b = table[2];
    }
  }
}

void adjustForMusic()
{
  static int TmpValues[NUM_LEDS];
  static CRGB TmpLeds[NUM_LEDS];

//create black  and white values/int from colors
  for (int i = 0; i < NUM_LEDS; i++) {
    TmpLeds[i] = leds[i];
    //TmpValues[i] = max(leds[i].r, TmpValues[i]);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  }
  //shift the hue for rainbow effect************
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((gHue + 4 * i) % 255, 255, TmpValues[i]);
  }
  //smooth out with neighbouring cells**********
  for (int i = 0; i < NUM_LEDS; i++) {
    //TmpValues[max(i - 1, 0)] += TmpValues[i] * 0.2;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //TmpValues[min(NUM_LEDS, i + 1)] += TmpValues[i] * 0.2;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  }
  // fade the leds and values*******************
  for (int i = 0; i < NUM_LEDS; i++) {
    TmpValues[i] -= 3;
  }
  fadeToBlackBy( TmpLeds, NUM_LEDS, 10);

showLeds();
  //reset the leds to normal********************
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = TmpLeds[i];
  }
}

void getBpm()
{/*
  static int TmpValues[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS; i++) {
    TmpValues[i] = leds[i].r;
  }

static int averageEnergyL;
  long averageL;
  static long lastBeat = 0;

for (int i = 6; i >= 2; i--)
  {
    averageL += TmpValues[i];
  }
  averageL = averageL / 4;

  averageEnergyL += (int)(averageL * 0.05);
  averageEnergyL = averageEnergyL * 0.95;

  static CRGB ileds[NUM_LEDS];
  for (int i = 1; i < NUM_LEDS; i++)
  {
    ileds[i - 1] = ileds[i];
  }
  if (averageL > 12 + averageEnergyL)
  {
    int value = (int)(averageL * 1.2) ^ 2;
    if (value > 90)
    {
      lastBeat = millis();
    }
    value = (int)(((int)(log(averageL / 5) * 65)) + 2 * (int)(averageL * 1.2) ^ 2) / 3;
    ileds[NUM_LEDS - 1] = CHSV(gHue, 255, min(255, value));
  }

  if ((millis() - lastBeat) > 2000)
  {
    static int averageEnergyH;
    long averageH = 0;
    for (int i = 25; i >= 10; i--)
    {
      averageH += TmpValues[i];
    }
    averageH = min(255, averageH / 10);
    averageEnergyH += (int)(averageH * 0.05);
    averageEnergyH = averageEnergyH * 0.95;

   int a = min(255, (log(averageEnergyH / 2)) * 55);

    for (int i = 0; i < NUM_LEDS; i++)
    {
      ileds[i] = CRGB(a, a, a * 0.9);
    }
  }

  //add to original led array
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ileds[i];
  }

  showLeds();*/
}

void averageMusic()
{
  static int TmpValues[NUM_LEDSA];
  long avg = 0;
  for (int i = 0; i < NUM_LEDSA; i++) {
    TmpValues[i] = leds[i].r;
    avg += TmpValues[i];
  }
  avg = avg / NUM_LEDSA;
  avg = log(avg / 4) * 50;
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CHSV(gHue + 2 * i, 160, avg);
  }
  showLeds();
}

void importedMusic()
{

  static int TmpValues[NUM_LEDSA];
    long avg = 0;
    for (int i = 0; i < NUM_LEDSA; i++) {
    TmpValues[i] = leds[i].r;
    avg += TmpValues[i];
    }
    avg = avg / NUM_LEDSA;
  
  EVERY_N_MILLISECONDS(30) {
    volume = avg;
    
    if (volume < avgVol / 2.0 || volume < 15) volume = 0;
    else avgVol = (avgVol + volume) / 2.0; //If non-zeo, take an "average" of volumes.
    if (volume > maxVol) maxVol = volume;

   if (volume - last > 10) avgBump = (avgBump + (volume - last)) / 2.0;
    bump = (volume - last > avgBump * .9);

   //If a "bump" is triggered, average the time between bumps
    if (bump) {
      avgTime = (((millis() / 1000.0) - timeBump) + avgTime) / 2.0;
      timeBump = millis() / 1000.0;
    }
    last = volume;
  }
  if (volume > 0) {
    Visualize();
  }
  fadeall();
  showLeds();
}

void Visualize() {
  CRGBPalette16 palette = PartyColors_p;
  //uint8_t beat = beatsin8(max(120 / avgTime, 1), 64, 255);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    //leds[i] = CHSV(64, min(255,avgTime*255), beat - gHue + (i * 10));//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  }
}
