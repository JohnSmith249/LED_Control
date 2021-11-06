#define RED_LED 3
#define BLUE_LED 6
#define GREEN_LED 5

int brightness = 255;
int gBright = 0;
int rBright = 0;
int bBright = 0;
int fadeSpeed = 10;
int Start[3] = {255,255,255};
int Stop[3] = {0,0,0};
void setup() {
  Serial.begin(9600);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  //  TurnOn();
  delay(1000);
  //  TurnOff();
}

void TurnOn() {
  for (int i = 0; i < 256; i++) {
    analogWrite(RED_LED, rBright);
    rBright += 1;
    delay(fadeSpeed);
  }

  for (int i = 0; i < 256; i++) {
    analogWrite(BLUE_LED, bBright);
    bBright += 1;
    delay(fadeSpeed);
  }
  for (int i = 0; i < 256; i++) {
    analogWrite(GREEN_LED, gBright);
    gBright += 1;
    delay(fadeSpeed);
  }
}

void TurnOff() {
  for (int i = 0; i < 256; i++) {
    analogWrite(GREEN_LED, brightness);
    analogWrite(RED_LED, brightness);
    analogWrite(BLUE_LED, brightness);

    brightness -= 1;
    delay(fadeSpeed);
  }
}

void fade(int start_value, int stop_value, int pin, long fade_Speed) {
  if (start_value < stop_value) {
    for (int i = start_value; i < stop_value; i++) {
      analogWrite(pin, gBright);
      gBright += 1;
      delay(fade_Speed);
    }
  }
  else if (start_value > stop_value) {
    for (int i = stop_value; i < start_value; i++) {
      analogWrite(pin, gBright);
      start_value = start_value - 1;
      delay(fade_Speed);
    }
  }
  else if ( start_value == stop_value){
    delay(100);
  }
}
 
void loop() {
  Stop[0] = random(256);
  Stop[1] = random(256);
  Stop[2] = random(256);
  fade(Start[0], Stop[0], RED_LED, fadeSpeed);
  fade(Start[1], Stop[1], GREEN_LED, fadeSpeed);
  fade(Start[2], Stop[2], BLUE_LED, fadeSpeed);
  Start[0] = Stop[0];
  Start[1] = Stop[1];
  Start[2] = Stop[2];
}
