void setupSpectrumAnalyzer() {
  setupAudio();
  if (M_WIDTH == 8) numBands = 8;
  else numBands = 16;
  barWidth = M_WIDTH / numBands;

  EEPROM.begin(EEPROM_SIZE);
  
  // It should not normally be possible to set the gain to 255
  // If this has happened, the EEPROM has probably never been written to
  // (new board?) so reset the values to something sane.
  if (EEPROM.read(EEPROM_GAIN) == 255) {
    EEPROM.write(EEPROM_BRIGHTNESS, 32);
    EEPROM.write(EEPROM_GAIN, 0);
    EEPROM.write(EEPROM_SQUELCH, 0);
    EEPROM.write(EEPROM_PATTERN, 0);
    EEPROM.write(EEPROM_DISPLAY_TIME, 10);
    EEPROM.write(EEPROM_MODE, 0);
    EEPROM.commit();
  }

  // Read saved values from EEPROM
  FastLED.setBrightness(pow(2, EEPROM.read(EEPROM_BRIGHTNESS)) - 1);
  brightness = FastLED.getBrightness();
  gain = EEPROM.read(EEPROM_GAIN);
  squelch = EEPROM.read(EEPROM_SQUELCH);
  pattern = EEPROM.read(EEPROM_PATTERN);
  displayTime = EEPROM.read(EEPROM_DISPLAY_TIME);
  mode = EEPROM.read(EEPROM_MODE);
}





//////////// Patterns ////////////

void rainbowBars(uint8_t band, uint8_t barHeight) {
  int xStart = barWidth * band;
  for (int x = xStart; x < xStart + barWidth; x++) {
    for (int y = 0; y <= barHeight; y++) {
      leds(x,y) = CHSV((x / barWidth) * (255 / numBands), 255, 255);
    }
  }
}

void purpleBars(int band, int barHeight) {
  int xStart = barWidth * band;
  for (int x = xStart; x < xStart + barWidth; x++) {
    for (int y = 0; y < barHeight; y++) {
      leds(x,y) = ColorFromPalette(purplePal, y * (255 / barHeight));
    }
  }
}

void changingBars(int band, int barHeight) {
  int xStart = barWidth * band;
  for (int x = xStart; x < xStart + barWidth; x++) {
    for (int y = 0; y < barHeight; y++) {
      leds(x,y) = CHSV(y * (255 / M_HEIGHT) + colorTimer, 255, 255); 
    }
  }
}

void centerBars(int band, int barHeight) {
  int xStart = barWidth * band;
  for (int x = xStart; x < xStart + barWidth; x++) {
    if (barHeight % 2 == 0) barHeight--;
    int yStart = ((M_HEIGHT - barHeight) / 2 );
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      leds(x,y) = ColorFromPalette(heatPal, colorIndex);
    }
  }
}

void whitePeak(int band) {
  int xStart = barWidth * band;
  int peakHeight = peak[band];
  for (int x = xStart; x < xStart + barWidth; x++) {
    leds(x,peakHeight) = CRGB::White;
  }
}

void outrunPeak(int band) {
  int xStart = barWidth * band;
  int peakHeight = peak[band];
  for (int x = xStart; x < xStart + barWidth; x++) {
    leds(x,peakHeight) = ColorFromPalette(outrunPal, peakHeight * (255 / M_HEIGHT));
  }
}

void createWaterfall(int band) {
  int xStart = barWidth * band;
  // Draw bottom line
  for (int x = xStart; x < xStart + barWidth; x++) {
    leds(x,0) = CHSV(constrain(map(fftResult[band],0,254,160,0),0,160), 255, 255);
  }
}

void moveWaterfall() {
  // Move screen up starting at 2nd row from top
  for (int y = M_HEIGHT - 2; y >= 0; y--) {
    for (int x = 0; x < M_WIDTH; x++) {
      leds(x,y+1) = leds(x,y);
    }
  }
}

void drawPatterns(uint8_t band) {
  
  uint8_t barHeight = barHeights[band];
  
  // Draw bars
  switch (pattern) {
    case 0:
      rainbowBars(band, barHeight);
      break;
    case 1:
      // No bars on this one
      break;
    case 2:
      purpleBars(band, barHeight);
      break;
    case 3:
      centerBars(band, barHeight);
      break;
    case 4:
      changingBars(band, barHeight);
      EVERY_N_MILLISECONDS(10) { colorTimer++; }
      break;
    case 5:
      createWaterfall(band);
      EVERY_N_MILLISECONDS(30) { moveWaterfall(); }
      break;
  }

  // Draw peaks
  switch (pattern) {
    case 0:
      whitePeak(band);
      break;
    case 1:
      outrunPeak(band);
      break;
    case 2:
      whitePeak(band);
      break;
    case 3:
      // No peaks
      break;
    case 4:
      // No peaks
      break;
    case 5:
      // No peaks
      break;
  }
}

void showAudio(){
  if (pattern != 5) FastLED.clear();
  
  uint8_t divisor = 1;                                                    // If 8 bands, we need to divide things by 2
  if (numBands == 8) divisor = 2;                                         // and average each pair of bands together
  
  for (int i = 0; i < 16; i += divisor) {
    uint8_t fftValue;
    
    if (numBands == 8) fftValue = (fftResult[i] + fftResult[i+1]) / 2;    // Average every two bands if numBands = 8
    else fftValue = fftResult[i];

    fftValue = ((prevFFTValue[i/divisor] * 3) + fftValue) / 4;            // Dirty rolling average between frames to reduce flicker
    barHeights[i/divisor] = fftValue / (255 / M_HEIGHT);                  // Scale bar height
    
    if (barHeights[i/divisor] > peak[i/divisor])                          // Move peak up
      peak[i/divisor] = min(M_HEIGHT, (int)barHeights[i/divisor]);
      
    prevFFTValue[i/divisor] = fftValue;                                   // Save prevFFTValue for averaging later
    
  }

  // Draw the patterns
  for (int band = 0; band < numBands; band++) {
    drawPatterns(band);
  }

  // Decay peak
  EVERY_N_MILLISECONDS(60) {
    for (uint8_t band = 0; band < numBands; band++)
      if (peak[band] > 0) peak[band] -= 1;
  }

  EVERY_N_SECONDS(30) {
    // Save values in EEPROM. Will only be commited if values have changed.
    EEPROM.write(EEPROM_BRIGHTNESS, brightness);
    EEPROM.write(EEPROM_GAIN, gain);
    EEPROM.write(EEPROM_SQUELCH, squelch);
    EEPROM.write(EEPROM_PATTERN, pattern);
    EEPROM.write(EEPROM_DISPLAY_TIME, displayTime);
    EEPROM.write(EEPROM_MODE, mode);
    EEPROM.commit();
  }
  
  EVERY_N_SECONDS_I(timingObj, displayTime) {
    timingObj.setPeriod(displayTime);
    if (autoChangePatterns) pattern = (pattern + 1) % 6;
  }
  
  FastLED.setBrightness(pow(2, brightness) - 1);
  FastLED.show();
}
