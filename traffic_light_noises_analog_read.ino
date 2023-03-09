int SPEAKER_PIN = 1; // ATtiny85 pin 6, output to motor shield board which drives the speaker
int WALK_PHASE_PIN = A3; // ATtiny85 pin 2, input from 4017 decade counter IC running the lights themselves
int DONT_WALK_LED_PIN = 4; // ATtiny85 pin 3, output to driver transistor
int DONT_WALK_MAX_FLASHES = 3;

/*
 * Tones values are based on Australian pedestrian crossings, reverse engineered by ronnied here:
 * https://github.com/ronnied/traffic-crossing
*/
int tones[] = {
  3500, 2850, 2333, 1956, 1638, 1380, 1161, 992, 814, 750, 700
};

// for state machine
enum State_enum {DONT_WALK, WALK, DONT_WALK_FLASH};

// save current system state in these
int state;
int dontWalkFlashCount = 0;
unsigned long previousToneMillis = 0; 
unsigned long previousFlashMillis = 0; 

/*
 * TinyTone implementation for ATtiny85 from here:
 * http://www.technoblogy.com/show?KVO
 */
void TinyTone(unsigned int pin, unsigned char divisor, unsigned long duration)
{
  TCCR1 = 0x90 | (11-4); // for 8MHz clock
  OCR1C = divisor-1;         // set the OCR
  delay(duration);
  TCCR1 = 0x90;              // stop the counter
}

void playIdleTone() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousToneMillis >= 1800)
  {
    previousToneMillis = currentMillis;
    TinyTone(SPEAKER_PIN, 1000, 25);
    delay(25);
    noTone(SPEAKER_PIN);
  }
}

void playChirpTone() {
  // Iterate through all tones
  for (int i = 0; i < 11; i++) {
    // Play the next tone
    TinyTone(SPEAKER_PIN, tones[i], 11);
    delay(11);
  }
}

void playWalkTone() {
    TinyTone(SPEAKER_PIN, 500, 30);
    delay(30);
    noTone(SPEAKER_PIN);
    delay(117); // wait
}

void runFlashDontWalkPhase()
{
  playIdleTone();
  
  unsigned long currentMillisFlash = millis();
  if (currentMillisFlash - previousFlashMillis >= 500)
  {
    previousFlashMillis = currentMillisFlash;
    dontWalkFlashCount++;
    // cause don't walk LED to flash
    if ((dontWalkFlashCount % 2) == 0)
    {
      digitalWrite(DONT_WALK_LED_PIN, LOW);
    }
    else
    {
      digitalWrite(DONT_WALK_LED_PIN, HIGH);
    }    

    // enough flashing, back to static red
    if ((dontWalkFlashCount / 2) > DONT_WALK_MAX_FLASHES)
    {
      dontWalkFlashCount = 0;
      state = DONT_WALK;
    }
  }
}

void runWalkPhase()
{
  // disable ACTIVE HIGH don't walk LED
  digitalWrite(DONT_WALK_LED_PIN, LOW);
  
  // if walk phase just started
  if (state == DONT_WALK)
  {
    // GO!
    state = WALK;
    playChirpTone();
  }

  playWalkTone();

  // exit walk phase if HIGH signal removed
  if (analogRead(WALK_PHASE_PIN) < 200)
  {
    state = DONT_WALK_FLASH;
  }
}

void runDontWalkPhase()
{
  // enable ACTIVE HIGH don't walk LED
  digitalWrite(DONT_WALK_LED_PIN, HIGH);
  playIdleTone();

  // move to walk phase if HIGH signal received
  bool walkEnabled = (analogRead(WALK_PHASE_PIN) > 725) && (millis() > 100);
  if (walkEnabled)
  {
    state = WALK;
  }
}

void setup() {
  // Setup pins
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(DONT_WALK_LED_PIN, OUTPUT);
  pinMode(WALK_PHASE_PIN, INPUT);
  state = DONT_WALK;
}

// Core of code is a state machine
void loop() {
  switch (state)
  {
    case WALK:
      runWalkPhase();
      break;
    case DONT_WALK_FLASH:
      runFlashDontWalkPhase();
      break;
    case DONT_WALK:
      runDontWalkPhase();
      break;
  }
}
