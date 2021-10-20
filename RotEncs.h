// interrupt handling
void IRAM_ATTR buttonPressed0() {
  // BUTTON0
  if (millis() - prevMillis > debounceTime) {
    mode = (mode + 1) % maxmode;
    prevMillis = millis();
    Serial.print("Mode       : "); Serial.println(mode);
  }
}

void IRAM_ATTR buttonPressed1() {
  // BUTTON1
  if (millis() - prevMillis > debounceTime) {
    pattern = (pattern + 1) % maxpatterns;
    prevMillis = millis();
    Serial.print("Pattern    : "); Serial.println(pattern);
  }
}

void IRAM_ATTR turnedKnob0() {
  if (millis() - prevMillis > debounceTime) {
    prevMillis = millis();
    
    // ROTATION DIRECTION
    if (digitalRead(ROTENC_0_DT) == HIGH) {      // If Pin B is HIGH
      if (brightness < 8) {
        brightness += 1;        
      }
    } else {
      if (brightness > 0) {
        brightness -=1;
      }
    } 
    Serial.print("Brightness : "); Serial.println(brightness);
  }
}

void IRAM_ATTR turnedKnob1() {
  if (millis() - prevMillis > debounceTime) {
    prevMillis = millis();

    // ROTATION DIRECTION
    if (digitalRead(ROTENC_1_DT) == HIGH) {      // If Pin B is HIGH
      if (gain < 20) {
        gain += 1;        
      }
    } else {
      if (gain > 0) {
        gain -=1;
      }
    } 
    Serial.print("Mic Gain   : "); Serial.println(gain);
  }
}

void setupRotEncs() {
  pinMode(ROTENC_0_SW, INPUT);
  pinMode(ROTENC_0_CLK, INPUT);
  pinMode(ROTENC_0_DT, INPUT);
  pinMode(ROTENC_1_SW, INPUT);
  pinMode(ROTENC_1_CLK, INPUT);
  pinMode(ROTENC_1_DT, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(ROTENC_0_SW), buttonPressed0, RISING);
  attachInterrupt(digitalPinToInterrupt(ROTENC_1_SW), buttonPressed1, RISING);
  attachInterrupt(digitalPinToInterrupt(ROTENC_0_CLK), turnedKnob0, FALLING);
  attachInterrupt(digitalPinToInterrupt(ROTENC_1_CLK), turnedKnob1, FALLING);
}
