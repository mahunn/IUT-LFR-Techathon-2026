// Line Following Robot — Main File
// 5-sensor IR array, L298N driver, differential drive

// --- Pin assignments ---
// Speed pins need PWM (5, 10 work on Uno/Nano)
const int LM_SPD = 5;
const int RM_SPD = 10;

// Direction pins (any digital pin works)
const int LM_FWD = 6;
const int LM_BWD = 7;
const int RM_FWD = 8;
const int RM_BWD = 9;

const int NUM_SENSORS = 5;

// --- Tuning stuff (tweak these on the track) ---
int threshold = 512;
const int BASE_SPEED = 180;
const int MAX_SPEED  = 255;

// PID gains
float Kp = 30.0;
float Ki = 0.0;    // keep 0 unless it drifts to one side
float Kd = 20.0;

// --- State ---
int sensorValues[NUM_SENSORS];
int sensorBits = 0;
float lastError = 0;
float integral = 0;
int lastKnownDirection = 0;  // -1 left, +1 right

void setup() {
  pinMode(LM_SPD, OUTPUT);
  pinMode(LM_FWD, OUTPUT);
  pinMode(LM_BWD, OUTPUT);
  pinMode(RM_SPD, OUTPUT);
  pinMode(RM_FWD, OUTPUT);
  pinMode(RM_BWD, OUTPUT);

  setMotor(0, 0);  // don't move on startup
  Serial.begin(9600);
  delay(1000);      // time to place on track
}

void loop() {
  readSensors();

  int activeSensors = 0;
  float weightedSum = 0;

  // weighted average to get position error
  // sensor 0 (leftmost) = weight -2, sensor 4 (rightmost) = weight +2
  for (int i = 0; i < NUM_SENSORS; i++) {
    int bit = (sensorBits >> (4 - i)) & 1;
    if (bit) {
      weightedSum += (i - 2);
      activeSensors++;
    }
  }

  // all sensors on line = junction, just go straight
  if (activeSensors == 5) {
    setMotor(BASE_SPEED, BASE_SPEED);
    return;
  }

  // lost the line completely — spin in place to recover
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
