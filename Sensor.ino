// Line Following Robot — Main File
// 5-sensor IR array, L298N driver, differential drive

// --- Pin assignments ---
// Speed pins need PWM (5, 10 work on Uno/Nano)
const int LM_SPD = 5;   //fixed: renamed from 'lms'
const int RM_SPD = 10;  //fixed: renamed from 'rms'

// Direction pins (any digital pin works)
const int LM_FWD = 6;   //fixed: renamed from 'lmf'
const int LM_BWD = 7;   //fixed: renamed from 'lmb'
const int RM_FWD = 8;   //fixed: renamed from 'rmf'
const int RM_BWD = 9;   //fixed: renamed from 'rmb'

const int NUM_SENSORS = 5;

// --- Tuning stuff (tweak these on the track) ---
int threshold = 512;        //fixed: renamed from 'th'
const int BASE_SPEED = 180; //fixed: new, replaces hardcoded 200
const int MAX_SPEED  = 255; //fixed: new

// PID gains //fixed: new, replaces if-else steering
float Kp = 30.0;
float Ki = 0.0;    // keep 0 unless it drifts to one side
float Kd = 20.0;

// --- State ---
int sensorValues[NUM_SENSORS]; //fixed: renamed from 's[5]'
int sensorBits = 0;            //fixed: renamed from 'sensor'
float lastError = 0;           //fixed: new, for PID
float integral = 0;            //fixed: new, for PID
int lastKnownDirection = 0;    //fixed: renamed from 'lastTurn'

void setup() {
  pinMode(LM_SPD, OUTPUT);   //fixed: was missing before
  pinMode(LM_FWD, OUTPUT);
  pinMode(LM_BWD, OUTPUT);
  pinMode(RM_SPD, OUTPUT);   //fixed: was missing before
  pinMode(RM_FWD, OUTPUT);
  pinMode(RM_BWD, OUTPUT);

  setMotor(0, 0);  //fixed: new, stops motors on boot
  Serial.begin(9600);
  delay(1000);      //fixed: new, wait before starting
}

void loop() {
  readSensors();  //fixed: renamed from 'reading()'

  int activeSensors = 0;
  float weightedSum = 0;

  //fixed: old if-else chain replaced with PID
  // sensor 0 (leftmost) = weight -2, sensor 4 (rightmost) = weight +2
  for (int i = 0; i < NUM_SENSORS; i++) {
    int bit = (sensorBits >> (4 - i)) & 1;
    if (bit) {
      weightedSum += (i - 2);
      activeSensors++;
    }
  }

  //fixed: new, handles junctions (all sensors on line)
  if (activeSensors == 5) {
    setMotor(BASE_SPEED, BASE_SPEED);
    return;
  }

  //fixed: now spins in place instead of slow curve
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
