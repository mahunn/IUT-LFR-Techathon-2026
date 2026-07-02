// Reads all 5 IR sensors and builds a 5-bit pattern
// bit 4 = leftmost sensor, bit 0 = rightmost

void readSensors() {
  sensorBits = 0;
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorValues[i] = analogRead(A0 + i);
    if (sensorValues[i] > threshold) {
      sensorBits |= (1 << (4 - i));
    }
  }
}

// prints raw sensor values + bit pattern + PID error to serial monitor
// useful for calibration, kill this for competition runs (slows the loop)
void debugPrint() {
  Serial.print("Sensors: ");
  for (int i = 0; i < NUM_SENSORS; i++) {
    Serial.print(sensorValues[i]);
    Serial.print("\t");
  }
  Serial.print("| Bits: ");
  Serial.print(sensorBits, BIN);
  Serial.print(" | Err: ");
  Serial.println(lastError);
}
