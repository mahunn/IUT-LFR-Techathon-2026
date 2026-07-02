// Line Following Robot — Main File
// 5-sensor IR array, L298N driver, differential drive

// --- Pin assignments ---
int lms = 5, lmf = 6, lmb = 7, rmf = 8, rmb = 9, rms = 10;

// --- Tuning stuff (tweak these on the track) ---
int th = 512;
const int BASE_SPEED = 180; //fixed: new, replaces hardcoded 200
const int MAX_SPEED  = 255; //fixed: new

// PID gains //fixed: new, replaces if-else steering
float Kp = 30.0;
float Ki = 0.0;    // keep 0 unless it drifts to one side
float Kd = 20.0;

// --- State ---
int s[5];
int sensor = 0;
float lastError = 0;           //fixed: new, for PID
float integral = 0;            //fixed: new, for PID
int lastTurn = 0;  // -1 left, +1 right

void setup() {
  pinMode(lmf, OUTPUT);
  pinMode(lmb, OUTPUT);
  pinMode(rmf, OUTPUT);
  pinMode(rmb, OUTPUT);

  pinMode(lms, OUTPUT);   //fixed: was missing before
  pinMode(rms, OUTPUT);   //fixed: was missing before

  motor(0, 0);  //fixed: new, stops motors on boot
  Serial.begin(9600);
  delay(1000);      //fixed: new, wait before starting
}

void loop() {
  reading();

  int activeSensors = 0;
  float weightedSum = 0;

  //fixed: old if-else chain replaced with PID
  // sensor 0 (leftmost) = weight -2, sensor 4 (rightmost) = weight +2
  for (int i = 0; i < 5; i++) {
    int bit = (sensor >> (4 - i)) & 1;
    if (bit) {
      weightedSum += (i - 2);
      activeSensors++;
    }
  }

  //fixed: handles junctions, continues moving straight (when all sensors on line)
  if (activeSensors == 5) {
    motor(BASE_SPEED, BASE_SPEED);
    return;
  }

  //fixed: now spins in place instead of slow curve
  if (activeSensors == 0) {
    if (lastTurn <= 0)
      motor(-120, 120);   // spin left
    else
      motor(120, -120);   // spin right
    return;
  }

  // --- PID ---
  float error = weightedSum / activeSensors;

  if (error < -0.1) lastTurn = -1;
  else if (error > 0.1) lastTurn = 1;

  integral += error;
  integral = constrain(integral, -50, 50);
  float derivative = error - lastError;

  float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);
  lastError = error;

  int leftSpeed  = BASE_SPEED - (int)correction;
  int rightSpeed = BASE_SPEED + (int)correction;

  leftSpeed  = constrain(leftSpeed,  -MAX_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);

  motor(leftSpeed, rightSpeed);

  debugPrint();  // comment this out during competition
}
