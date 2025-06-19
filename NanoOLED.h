/*
 * SeeedOLED.h => NanoOLED.h
 * SSD130x & SH1106 OLED Driver Library
 *
 * Copyright (c) 2011 seeed technology inc.
 * Author        :   Visweswara R
 * Create Time   :   Dec 2011
 * Change Log    :   Support SH1106 (2019-7)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef NANOOLED_H
#define NANOOLED_H

// NanoOLED Instruction set addresses
#include <stdint.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include <WProgram.h>
#include "microWire.h"  // лёгкая библиотека Wire
//#include "Wire.h"
#include "Print.h"
#endif

#ifndef MAX_I2C_TRANSFER_BYTES
#define MAX_I2C_TRANSFER_BYTES 16
#endif

#define NANOLED_PRINTF

#ifdef NANOLED_PRINTF
#ifndef NANOLED_FMT_BUF_SIZE
#define NANOLED_FMT_BUF_SIZE 32
#endif
#endif

const char OLED_Address = 0x3c;
const uint8_t OLED_WIDTH = 128;
const uint8_t OLED_HEIGHT = 8;

enum OLED_CHIP
{
    SSD1306,
    SH1106
};

enum OLED_MEMODE
{
    PAGE_MODE = 0x02,
    VERTICAL_MODE = 0x01,
    HORIZONTAL_MODE = 0x00,
};

enum OLED_ADDRMSK
{
    OLED_MSK_Page_Addr = 0xB0,
    OLED_MSK_Col_LowAddr = 0x00,
    OLED_MSK_Col_HighAddr = 0x10,
};

enum OLED_CMD
{
    OLED_Command_Mode = 0x00,
    OLED_Data_Mode = 0x40,
    OLED_Memory_Mode = 0x20,
    OLED_Display_Off_Cmd = 0xAE,
    OLED_Display_On_Cmd = 0xAF,
    OLED_Normal_Display_Cmd = 0xA6,
    OLED_Inverse_Display_Cmd = 0xA7,
    OLED_Activate_Scroll_Cmd = 0x2F,
    OLED_Dectivate_Scroll_Cmd = 0x2E,
    OLED_Set_Brightness_Cmd = 0x81,

    // Only SSD1306
    OLED_Set_Col_Range = 0x21,
    OLED_Set_Page_Range = 0x22,

};

enum SCROLL_DIR
{
    Scroll_Left = 0x27,
    Scroll_Right = 0x26,
};

enum SCROLL_SPEED
{
    Scroll_2Frames = 0x7,
    Scroll_3Frames = 0x4,
    Scroll_4Frames = 0x5,
    Scroll_5Frames = 0x0,
    Scroll_25Frames = 0x6,
    Scroll_64Frames = 0x1,
    Scroll_128Frames = 0x2,
    Scroll_256Frames = 0x3,
};

class NanoOLED : public Print
{
private:
    OLED_MEMODE memMode;
    OLED_CHIP chipType;

    uint8_t cursor_row, cursor_col;

    void defaultInit();
    void setDisplayArea(uint8_t page_start, uint8_t page_end, uint8_t col_start, uint8_t col_end);
    void resetDisplayArea() { setDisplayArea(0, OLED_HEIGHT - 1, 0, OLED_WIDTH - 1); };
    size_t repeat(const uint8_t d, size_t len);

public:
    NanoOLED() : chipType(SSD1306){};
    NanoOLED(OLED_CHIP chip) : chipType(chip){};

    void moveCursor(uint8_t len);

    void init();

    virtual size_t write(uint8_t byte);
    using Print::write;

    void setNormalDisplay();
    void setInverseDisplay();

#ifdef NANOLED_PRINTF
    void Printf(const char *fmt, ...);
#endif

    void sendCommand(uint8_t command);
    void sendData(uint8_t Data);
    size_t sendPixels(const uint8_t *pix, size_t len);

    void setPageMode();
    void setHorizontalMode();

    void setCursor(uint8_t row, uint8_t col);
    void clearDisplay();
    void setBrightness(uint8_t Brightness);
    void putChar(uint8_t c);
    void drawBitmap(const uint8_t *bitmaparray, uint8_t row_start, uint8_t col_start, uint8_t row, uint8_t col);

    void setHorizontalScrollProperties(SCROLL_DIR direction, uint8_t startPage, uint8_t endPage, SCROLL_SPEED scrollSpeed);
    void activateScroll();
    void deactivateScroll();
};

#endif
