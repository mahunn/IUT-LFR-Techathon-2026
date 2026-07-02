// Motor control — handles direction + PWM speed for both motors
// positive = forward, negative = reverse, 0 = stop (coast)

void motor(int a, int b) { //fixed: kept original motor name
  // left motor
  if (a > 0) {
    digitalWrite(lmf, HIGH);
    digitalWrite(lmb, LOW);
  } else if (a < 0) {
    digitalWrite(lmf, LOW);
    digitalWrite(lmb, HIGH);
  } else {
    digitalWrite(lmf, LOW);
    digitalWrite(lmb, LOW);
  }
  analogWrite(lms, constrain(abs(a), 0, 255));

  // right motor
  if (b > 0) {
    digitalWrite(rmf, HIGH);
    digitalWrite(rmb, LOW);
  } else if (b < 0) {
    digitalWrite(rmf, LOW);
    digitalWrite(rmb, HIGH);
  } else {
    digitalWrite(rmf, LOW);
    digitalWrite(rmb, LOW);
  }
  analogWrite(rms, constrain(abs(b), 0, 255));
}
