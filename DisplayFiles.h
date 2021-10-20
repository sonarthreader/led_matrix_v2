void * myOpen(const char *filename, int32_t *size) {
  //Serial.printf("Attempting to open %s\n", filename);
  myfile = SPIFFS.open(filename);
  *size = myfile.size();
  return &myfile;
}
void myClose(void *handle) {
  if (myfile) myfile.close();
}
int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length) {
  if (!myfile) return 0;
  return myfile.read(buffer, length);
}
int32_t mySeek(PNGFILE *handle, int32_t position) {
  if (!myfile) return 0;
  return myfile.seek(position);
}

// Function to draw pixels to the display
void PNGDraw(PNGDRAW *pDraw) {
uint16_t usPixels[M_WIDTH];

  png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);

  // Convert to RGB888 (true color)
  for(int i = 0; i < M_WIDTH; i++) {
    uint8_t r = ((((usPixels[i] >> 11) & 0x1F) * 527) + 23) >> 6;
    uint8_t g = ((((usPixels[i] >> 5) & 0x3F) * 259) + 33) >> 6;
    uint8_t b = (((usPixels[i] & 0x1F) * 527) + 23) >> 6;

    uint32_t RGB888 = r << 16 | g << 8 | b;

    // write into LED array
    ledpic[ i + M_WIDTH*pDraw->y ] = RGB888;
  }
}


void readFiles() {
  int rc;
  int filenum = 0;
  Serial.println("Reading Files from SPIFFS");
  File root = SPIFFS.open("/");
  File foundfile = root.openNextFile();
  // 
  while (foundfile) {
    if (mode > 0) { // stop loop if mode was changed
      break;
    }
    if (foundfile.isDirectory() == false) {
      //Serial.print("Filename :" + String(foundfile.name()) + "\n");
      const char *name = foundfile.name();
      const int len = strlen(name);
      if (len > 3 && strcmp(name + len - 3, "png") == 0) {
        // it's a PNG ;-)
        rc = png.open((const char *)name, myOpen, myClose, myRead, mySeek, PNGDraw);
        if (rc == PNG_SUCCESS) {
          //Serial.printf("image number %d\n", filenum);
          //Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d, buffer size: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType(), png.getBufferSize());
          // dump color values from PNG
          rc = png.decode(NULL, 0);
          // close file and increase counter
          png.close();
          filenum++;

          // now display on LED matrix
          printArray(ledpic, 1000);
        }
      }
    }
    
    foundfile = root.openNextFile();
  }
  foundfile.close();
  root.close();
}
