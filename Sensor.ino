// Line Following Robot — Main File
// 5-sensor IR array, L298N driver, differential drive

// --- Pin assignments ---
// Speed pins need PWM (5, 10 work on Uno/Nano)
const int LM_SPD = 5;   //fixed: was 'lms', made const
const int RM_SPD = 10;  //fixed: was 'rms', made const

// Direction pins (any digital pin works)
const int LM_FWD = 6;   //fixed: was 'lmf', made const
const int LM_BWD = 7;   //fixed: was 'lmb', made const
const int RM_FWD = 8;   //fixed: was 'rmf', made const
const int RM_BWD = 9;   //fixed: was 'rmb', made const

const int NUM_SENSORS = 5;

// --- Tuning stuff (tweak these on the track) ---
int threshold = 512;        //fixed: was 'th'
const int BASE_SPEED = 180; //fixed: was hardcoded 200 everywhere
const int MAX_SPEED  = 255; //fixed: added

// PID gains //fixed: added PID (original had if-else chain)
float Kp = 30.0;
float Ki = 0.0;    // keep 0 unless it drifts to one side
float Kd = 20.0;

// --- State ---
int sensorValues[NUM_SENSORS]; //fixed: was 's[5]'
int sensorBits = 0;            //fixed: was 'sensor'
float lastError = 0;           //fixed: added for PID
float integral = 0;            //fixed: added for PID
int lastKnownDirection = 0;    //fixed: was 'lastTurn'

void setup() {
  pinMode(LM_SPD, OUTPUT);   //fixed: was missing
  pinMode(LM_FWD, OUTPUT);
  pinMode(LM_BWD, OUTPUT);
  pinMode(RM_SPD, OUTPUT);   //fixed: was missing
  pinMode(RM_FWD, OUTPUT);
  pinMode(RM_BWD, OUTPUT);

  setMotor(0, 0);  //fixed: added — don't move on startup
  Serial.begin(9600);
  delay(1000);      //fixed: added — time to place on track
}

void loop() {
  readSensors();  //fixed: was 'reading()'

  int activeSensors = 0;
  float weightedSum = 0;

  //fixed: replaced entire if-else chain with weighted average + PID
  // sensor 0 (leftmost) = weight -2, sensor 4 (rightmost) = weight +2
  for (int i = 0; i < NUM_SENSORS; i++) {
    int bit = (sensorBits >> (4 - i)) & 1;
    if (bit) {
      weightedSum += (i - 2);
      activeSensors++;
    }
  }

  //fixed: added — original had no junction handling
  if (activeSensors == 5) {
    setMotor(BASE_SPEED, BASE_SPEED);
    return;
  }

  //fixed: was gentle curve motor(80,200), now spins in place for faster recovery
  if (activeSensors == 0) {
    if (lastKnownDirection <= 0)
      setMotor(-120, 120);   // spin left
    else
      setMotor(120, -120);   // spin right
    return;
  }

  // --- PID ---
  float error = weightedSum / activeSensors;

  if (error < -0.1) lastKnownDirection = -1;
  else if (error > 0.1) lastKnownDirection = 1;

  integral += error;
  integral = constrain(integral, -50, 50);
  float derivative = error - lastError;

  float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);
  lastError = error;

  int leftSpeed  = BASE_SPEED - (int)correction;
  int rightSpeed = BASE_SPEED + (int)correction;

  leftSpeed  = constrain(leftSpeed,  -MAX_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);

  setMotor(leftSpeed, rightSpeed);

  debugPrint();  // comment this out during competition
}
