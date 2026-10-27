#pragma once

#include <helpers/ui/DisplayDriver.h>
#include <M5Cardputer.h>

// Keep one owner for the LCD bus: M5Cardputer initializes SPI3.
class CardputerDisplay : public DisplayDriver {
  M5Canvas canvas;
  bool on = false;
  ColorVal color = 0xFFFF;
  int cursorX = 0, cursorY = 0;
  int textSize = 1;
public:
  CardputerDisplay() : DisplayDriver(128, 64), canvas(&M5Cardputer.Display) {}
  bool begin() {
    canvas.setColorDepth(16);
    if (!canvas.createSprite(width(), height())) return false;
    canvas.setTextWrap(false);
    canvas.setPivot(0, 0);
    turnOn();
    return true;
  }
  bool isOn() override { return on; }
  void turnOn() override { M5Cardputer.Display.setBrightness(128); on = true; }
  void turnOff() override { M5Cardputer.Display.setBrightness(0); on = false; }
  void clear() override { canvas.fillScreen(UIColor::window_bkg); }
  void startFrame(ColorVal bkg = UIColor::window_bkg) override {
    canvas.fillScreen(bkg);
    setTextSize(1);
    setColor(UIColor::primary_txt);
  }
  void setTextSize(int size) override { textSize = size; canvas.setTextSize(size); }
  void setColor(ColorVal value) override { color = value; canvas.setTextColor(value); }
  void setCursor(int x, int y) override { cursorX = x; cursorY = y; }
  void print(const char* text) override { canvas.drawString(text, cursorX, cursorY); }
  void printWordWrap(const char* text, int maxWidth) override {
    char line[256];
    while (*text && cursorY < height()) {
      size_t len = 0, lastSpace = 0;
      while (text[len] && text[len] != '\n' && len < sizeof(line) - 1) {
        line[len] = text[len];
        line[len + 1] = 0;
        if (canvas.textWidth(line) > maxWidth) break;
        if (text[len] == ' ') lastSpace = len;
        ++len;
      }
      if (!len && *text != '\n') len = 1;
      if (text[len] && text[len] != '\n' && lastSpace) len = lastSpace;
      memcpy(line, text, len);
      line[len] = 0;
      print(line);
      text += len;
      if (*text == '\n' || *text == ' ') ++text;
      cursorY += 8 * textSize;
    }
  }
  void fillRect(int x, int y, int w, int h) override { canvas.fillRect(x, y, w, h, color); }
  void drawRect(int x, int y, int w, int h) override { canvas.drawRect(x, y, w, h, color); }
  void drawXbm(int x, int y, const uint8_t* bits, int w, int h) override {
    // MeshCore's UI icons are packed MSB-first.
    for (int row = 0; row < h; ++row)
      for (int col = 0; col < w; ++col)
        if (pgm_read_byte(bits + row * ((w + 7) / 8) + col / 8) & (0x80 >> (col & 7)))
          canvas.drawPixel(x + col, y + row, color);
  }
  uint16_t getTextWidth(const char* text) override { return canvas.textWidth(text); }
  void endFrame() override {
    canvas.pushRotateZoom(0, 0, 0, 240.0f / width(), 135.0f / height());
  }
};
