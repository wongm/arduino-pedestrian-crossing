//int btnPin = 9;
//int redRelayPin = 2;
//int grnRelayPin = 3;
int spkPin = 1;

/*
 * Tones values are based on Australian pedestrian crossings, reverse engineered by ronnied here:
 * https://github.com/ronnied/traffic-crossing
*/
int tones[] = {
  3500, 2850, 2333, 1956, 1638, 1380, 1161, 992, 814, 750, 700
}; 

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

void playIdle() {
  TinyTone(spkPin, 1000, 25);
  delay(25);
  noTone(spkPin);
}
void playChirp() {
  // Iterate through all tones
  for (int i = 0; i < 11; i++) {
    // Play the next tone
    TinyTone(spkPin, tones[i], 11);
    delay(11);
  }
}
void playWoodpecker() {
  // 17x = 2secs; 85x = 10 seconds; 25x = 3 seconds
  for (int i = 0; i <= 25; i++) {
    TinyTone(spkPin, 500, 30);
    delay(30);
    noTone(spkPin);
    delay(117); // wait
  }
}

// Start Flashing Red Light w/ Idle sound for 5 cycles
void doEndFlashing() {

  // Lights off

  // Cycle Red w/ Idle sound x 5
  for (int i = 0; i < 5; i++) {
    playIdle();
  }
}

void doTrafficLightCrossing() {

  // Red Light On

  // Idle Tone x 3
  playIdle();
  delay(1800);
  playIdle();
  delay(1800);
  playIdle();
  delay(800);

  // GO!
  playChirp();

  // Woodpecker for 3 seconds
  playWoodpecker();

  doEndFlashing();

  // Red light goes back on

  // Idle sound x 6
  playIdle();
  delay(1800);
  playIdle();
  delay(1800);
  playIdle();
  delay(1800);
  playIdle();
  delay(1800);
  playIdle();
  delay(1800);
  playIdle();
}

void setup() {
  // Setup pins
  pinMode(spkPin, OUTPUT);

 // pinMode(12, OUTPUT);  //dirA
 // pinMode(8, OUTPUT); //brakeA
  
  //digitalWrite(12, HIGH);  // turn motor ON  
 // digitalWrite(8, LOW);  // turn brake off
}

void loop() {
    // Run main program
    doTrafficLightCrossing();

  // Pause for 10s
  delay(10000); 
}
