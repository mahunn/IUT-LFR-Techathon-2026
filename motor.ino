// Motor control — handles direction + PWM speed for both motors
// positive = forward, negative = reverse, 0 = stop (coast)

void setMotor(int leftSpeed, int rightSpeed) {
  // left motor
  if (leftSpeed > 0) {
    digitalWrite(LM_FWD, HIGH);
    digitalWrite(LM_BWD, LOW);
  } else if (leftSpeed < 0) {
    digitalWrite(LM_FWD, LOW);
    digitalWrite(LM_BWD, HIGH);
  } else {
    digitalWrite(LM_FWD, LOW);
    digitalWrite(LM_BWD, LOW);
  }
  analogWrite(LM_SPD, constrain(abs(leftSpeed), 0, 255));

  // right motor
  if (rightSpeed > 0) {
    digitalWrite(RM_FWD, HIGH);
    digitalWrite(RM_BWD, LOW);
  } else if (rightSpeed < 0) {
    digitalWrite(RM_FWD, LOW);
    digitalWrite(RM_BWD, HIGH);
  } else {
    digitalWrite(RM_FWD, LOW);
    digitalWrite(RM_BWD, LOW);
  }
  analogWrite(RM_SPD, constrain(abs(rightSpeed), 0, 255));
}
