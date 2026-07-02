// Reads all 5 IR sensors and builds a 5-bit pattern
// bit 4 = leftmost sensor, bit 0 = rightmost

void reading() { //fixed: kept original reading name
  sensor = 0;
  for (int i = 0; i < 5; i++) {
    s[i] = analogRead(i);
    if (s[i] > th) {
      sensor |= (1 << (4 - i));
    }
  }
}

// prints raw sensor values + bit pattern + PID error to serial monitor
// useful for calibration, kill this for competition runs (slows the loop)
//fixed: added — original only had Serial.println(sensor, BIN)
void debugPrint() {
  Serial.print("Sensors: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(s[i]);
    Serial.print("\t");
  }
  Serial.print("| Bits: ");
  Serial.print(sensor, BIN);
  Serial.print(" | Err: ");
  Serial.println(lastError);
}
