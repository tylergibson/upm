/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 * Copyright (c) 2014 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include "i2clcd_private.h"
#include "jhd1313m1.h"

using namespace upm;

Jhd1313m1::Jhd1313m1(int bus, int lcdAddress, int rgbAddress) : I2CLcd(bus, lcdAddress)
{
    m_rgb_address = rgbAddress;
    m_i2c_lcd_rgb = mraa_i2c_init(m_bus);

    mraa_result_t ret = mraa_i2c_address(m_i2c_lcd_rgb, m_rgb_address);
    if (ret != MRAA_SUCCESS) {
        fprintf(stderr, "Messed up i2c bus\n");
    }

    usleep(50000);
    ret = i2Cmd(m_i2c_lcd_control, LCD_FUNCTIONSET | LCD_2LINE);
    UPM_CHECK_MRAA_SUCCESS(ret, "Unable to initialise the LCD controller");

    usleep(4500);
    ret = i2Cmd(m_i2c_lcd_control, LCD_DISPLAYCONTROL | LCD_DISPLAYON);
    UPM_CHECK_MRAA_SUCCESS(ret, "Unable to initialise the LCD controller");

    ret = clear();
    UPM_CHECK_MRAA_SUCCESS(ret, "Unable to initialise the LCD controller");

    usleep(4500);
    ret = i2Cmd(m_i2c_lcd_control, LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
    UPM_CHECK_MRAA_SUCCESS(ret, "Unable to initialise the LCD controller");

    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 0, 0);
    UPM_CHECK_MRAA_SUCCESS(ret, "Unable to initialise the RGB controller");
    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 1, 0);
    UPM_CHECK_MRAA_SUCCESS(ret, "Unable to initialise the RGB controller");
    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 0x08, 0xAA);
    UPM_CHECK_MRAA_SUCCESS(ret, "Unable to initialise the RGB controller");

    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 0x04, 255);
    UPM_CHECK_MRAA_SUCCESS(ret, "Unable to initialise the RGB controller");
    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 0x03, 255);
    UPM_CHECK_MRAA_SUCCESS(ret, "Unable to initialise the RGB controller");
    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 0x02, 255);
    UPM_CHECK_MRAA_SUCCESS(ret, "Unable to initialise the RGB controller");
}

Jhd1313m1::~Jhd1313m1()
{
}

mraa_result_t
Jhd1313m1::setColor(uint8_t r, uint8_t g, uint8_t b)
{
    mraa_result_t ret;

    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 0, 0);
    UPM_GOTO_ON_MRAA_FAIL(ret, beach);
    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 1, 0);
    UPM_GOTO_ON_MRAA_FAIL(ret, beach);
    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 0x08, 0xAA);
    UPM_GOTO_ON_MRAA_FAIL(ret, beach);

    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 0x04, r);
    UPM_GOTO_ON_MRAA_FAIL(ret, beach);
    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 0x03, g);
    UPM_GOTO_ON_MRAA_FAIL(ret, beach);
    ret = i2cReg(m_i2c_lcd_rgb, m_rgb_address, 0x02, b);

beach:
    return ret;
}

mraa_result_t
Jhd1313m1::scroll(bool direction)
{
    if (direction)
        return i2Cmd(m_i2c_lcd_control, (LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT));
    return i2Cmd(m_i2c_lcd_control, (LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT));
}

/*
 * **************
 *  virtual area
 * **************
 */
mraa_result_t
Jhd1313m1::write(std::string msg)
{
    mraa_result_t ret = MRAA_SUCCESS;

    usleep(1000);

    for (std::string::size_type i = 0; i < msg.size(); ++i) {
        ret = i2cData(m_i2c_lcd_control, msg[i]);
        UPM_GOTO_ON_MRAA_FAIL(ret, beach);
    }

beach:
    return ret;
}

mraa_result_t
Jhd1313m1::setCursor(int row, int column)
{
    mraa_result_t ret;

    int row_addr[] = { 0x80, 0xc0, 0x14, 0x54 };
    uint8_t offset = ((column % 16) + row_addr[row]);

    ret = i2Cmd(m_i2c_lcd_control, offset);

    return ret;
}

mraa_result_t
Jhd1313m1::clear()
{
    return i2Cmd(m_i2c_lcd_control, LCD_CLEARDISPLAY);
}

mraa_result_t
Jhd1313m1::home()
{
    return i2Cmd(m_i2c_lcd_control, LCD_RETURNHOME);
}
