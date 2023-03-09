int SPEAKER_PIN = 1; // physical pin 6
int WALK_PHASE_PIN = 3; // physical pin 2
int DONT_WALK_LED_PIN = 4; // physical pin 3
int DONT_WALK_MAX_FLASHES = 4;

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
volatile bool walkEnabled = false;
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
  unsigned long currentMillisFlash = millis();
  if (currentMillisFlash - previousFlashMillis >= 500)
  {
    previousFlashMillis = currentMillisFlash;
    dontWalkFlashCount++;
    digitalWrite(DONT_WALK_LED_PIN, !(dontWalkFlashCount % 2));

    // enough flashing, back to static
    if ((dontWalkFlashCount / 2) > DONT_WALK_MAX_FLASHES)
    {
      dontWalkFlashCount = 0;
      state = DONT_WALK;
    }
  }
  playIdleTone();
}

void runWalkPhase()
{
  // disable ACTIVE LOW don't walk LED
  digitalWrite(DONT_WALK_LED_PIN, HIGH);
  
  // if walk phase just started
  if (state == DONT_WALK)
  {
    // GO!
    state = WALK;
    playChirpTone();
  }

  playWalkTone();

  // if walk phase finished, exit
  if (digitalRead(WALK_PHASE_PIN) == LOW)
  {
    state = DONT_WALK_FLASH;
    walkEnabled = false;
  }
}

void runDontWalkPhase()
{
  // enable ACTIVE LOW don't walk LED
  digitalWrite(DONT_WALK_LED_PIN, LOW);
  playIdleTone();
}

void enablePinChangeInterrupt() {
  pinMode(WALK_PHASE_PIN, INPUT_PULLUP);
  cli();
  PCMSK |= (1 << digitalPinToPCMSKbit(WALK_PHASE_PIN)); // Pin Change Enable
  // equivalent to: PCMSK |= (1 <<PCINT3);
  GIMSK |= (1 << digitalPinToPCICRbit(WALK_PHASE_PIN)); // PCIE Pin Change Interrupt Enable
  // equivalent to: GIMSK |= (1 << PCIE);
  sei();
}

/*
 * Command: interrupt handler
 */
ISR(PCINT0_vect) {
  walkEnabled = true;
}

void setup() {
  // Setup pins
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(DONT_WALK_LED_PIN, OUTPUT);
  enablePinChangeInterrupt();
  state = DONT_WALK;
}

void loop() {
  if (walkEnabled)
  {
    runWalkPhase();
  }
  else
  {
    if (state == DONT_WALK_FLASH)
    {
      runFlashDontWalkPhase();
    }
    else
    {
      runDontWalkPhase();
    }
  }
}
