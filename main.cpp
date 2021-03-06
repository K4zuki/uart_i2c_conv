/** uart_i2c_conv for LPC824
Copyright 2016-2017 K4ZUKI(K4ZUKI@github.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "mbed.h"
#include "settings.h"
// Table 3. ASCII commands supported by SC18IM700
// ASCII command Hex value Command function
// [X] S 0x53 I2C-bus START
// [X] P 0x50 I2C/SPI-bus STOP, end of packet
// [X] R 0x52 read SC18IM700 internal register
// [X] W 0x57 write to internal register(s)
// [X] I 0x49 read GPIO port
// [X] O 0x4F write to GPIO port
// [_] Z 0x5A power down
// [X] C 0x43 change channel
// [X] E 0x45 SPI transfer start
// [_] V 0x__ enable VDDIO output to chip

/**
"C| '0'| P"
"C| '1'| P"
"C| '2'| P"
"C| '3'| P"
"S| 0x_8 _0| 0x_0 _4| 0x_D _E _A _D _B _E _A _F| P"
"S| 0x_8 _0| 0x_0 _4| 0x_D _E _A _D _B _E _A _F| S| 0x_8 _1| 0x_0 _4| P"
"S| 0x_8 _1| 0x_0 _4| P"
"R| '0'| P"
"R| '0'| '1'| ...| P"
"W| '0' 0x_a _a| P"
"W| '0' 0x_a _a| '1' 0x_b _b| ...| P"
"I| '0'| P"
"O| '0'| 0x_a _a| P"
"E| 0x_0 _4| 0x_0 _0| 0x_D _E _A _D _B _E _A _F| P" //write
"E| 0x_0 _4| 0x_0 _4| 0x_D _E _A _D _B _E _A _F| P" //write and read
*/
int main() {
  I2C *dev = &dev1;

#ifdef isUART
  pc.baud(115200);
#endif
  _spi.frequency(8000000);

  bool s = false;
  dev1.frequency(400000);
#if defined(TARGET_SSCI824) || defined(TARGET_LP824MAX)
  dev1.frequency(800000); //  800k; works around 940kHz with 200ohm pullups/ not
                          //  work at 1M?
  LPC_IOCON->PIO0_11 &= ~(0x03 << 8);
  LPC_IOCON->PIO0_11 |= (0x02 << 8);
  LPC_IOCON->PIO0_10 &= ~(0x03 << 8);
  LPC_IOCON->PIO0_10 |= (0x02 << 8);
#elif defined(TARGET_MCU_LPC11U35_501) || defined(TARGET_LPC11U35_401)
  dev1.frequency(800000); //  800k; works around 940kHz with 200ohm pullups/ not
                          //  work at 1M?
  LPC_IOCON->PIO0_4 &= ~(0x03 << 8);
  LPC_IOCON->PIO0_4 |= (0x02 << 8);
  LPC_IOCON->PIO0_5 &= ~(0x03 << 8);
  LPC_IOCON->PIO0_5 |= (0x02 << 8);
#endif

#ifdef isI2C2
  dev2.frequency(400000); //  400k
#endif
#ifdef isI2C3
  dev3.frequency(400000); //  400k
#endif
#ifdef isI2C4
  dev4.frequency(400000); //  400k
#endif
#ifdef isGPIO1
  DigitalInOut *gpio1[] = {
      &_GPIO10, &_GPIO11, &_GPIO12, &_GPIO13,
      &_GPIO14, &_GPIO15, &_GPIO16, &_GPIO17,
  };
  for (int k = 0; k < 8; k++) {
    gpio1[k]->input();
    gpio1[k]->mode(PullUp);
  }
#endif

  DigitalInOut *gpio0[] = {
      &_GPIO00, &_GPIO01, &_GPIO02, &_GPIO03,
      &_GPIO04, &_GPIO05, &_GPIO06, &_GPIO07,
  };
  for (int k = 0; k < 8; k++) {
    gpio0[k]->input();
    gpio0[k]->mode(PullUp);
  }

  int ack = 0;
  int plength = 0;
  int recieve[256];
  char send[256];
  for (int k = 0; k < 256; k += 4) {
    // cafe moca
    recieve[k + 0] = send[k + 0] = 0xC4;
    recieve[k + 1] = send[k + 1] = 0xFE;
    recieve[k + 2] = send[k + 2] = 0xE0;
    recieve[k + 3] = send[k + 3] = 0xCA;
  }

  int read = 0;
  int address = 0;
  int data = 0;
  int _data = 0;
  int length = 0;
  int channel = 0;
  int format = 8;
  int enabled = 0;
  int disabled = 0;
  enum command_e {
    CMD_S = 'S',
    CMD_P = 'P',
    CMD_C = 'C',
    CMD_R = 'R',
    CMD_W = 'W',
    CMD_I = 'I',
    CMD_O = 'O',
    CMD_E = 'E',
  };
  enum channel_e {
    CH0 = '0',
    CH1 = '1',
    CH2 = '2',
    CH3 = '3',
  };
  enum register_e {
    CHIP_ID = '0',
    GPIO0_STAT = '1',
    GPIO1_STAT = '2',
    GPIO0_CONF = '3',
    GPIO1_CONF = '4',
    I2C_CONF = '5',
    SPI_CONF = '6',
    REG7,
    REG8,
    REG9,
  };
  static uint8_t registers[] = {
      chip_id, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x70, REG7, REG8, REG9,
  };

  int i = 0;
  while (1) {
    i = 0;
    length = 0;
    s = false;
    while (true) {
      read = pc.getc();
      recieve[i] = read;
      i++;
      if (read == 'P') {
        plength = i;
        break;
      }
    }
    i = 0;
    while (i < plength) {
      switch (recieve[i]) {
      case CMD_C: {
        s = false;
        channel = recieve[i + 1];
        switch (channel) {
        case CH0: {
          channel = CH0;
          dev = &dev1;
          break;
        }
#ifdef isI2C2
        case CH1: {
          channel = CH1;
          dev = &dev2;
          break;
        }
#endif
#ifdef isI2C3
        case CH2: {
          channel = CH2;
          dev = &dev3;
          break;
        }
#endif
#ifdef isI2C4
        case CH3: {
          channel = CH3;
          dev = &dev4;
          break;
        }
#endif
        default: {
          channel = CH0;
          dev = &dev1;
          break;
        }
        }
        i += 2;
        break;
      }
      case CMD_S: {
        s = true;
        ack = plength - 2 - (i + 1) + (recieve[i + 2] & 0x01);
        if (ack >= 4) { // valid packet
          address = 0xff & (recieve[i + 1] << 4 | (recieve[i + 2] & 0x0F));
          length = 0xff & (recieve[i + 3] << 4 | (recieve[i + 4] & 0x0F));

          if (address & 0x01) {                            // read
            ack = dev->read(address, send, length, false); // added
            send[length] = ack;
            length += 1;
            i += 5;
          } else { // write
            for (int j = 0; j < (length * 2); j += 2) {
              ack = 0xff & ((recieve[5 + j] << 4) | (recieve[6 + j] & 0x0F));
              *(send + (j / 2)) = ack; // added
            }
            ack = dev->write(address, send, length, true); // added
            i += (5 + length * 2);
            send[0] = ack;
            length = 1;
          }
        } else {
          pc.printf("bad packet! %d, %d, %02X, %d\n\r", plength, i,
                    recieve[(i + 2)] & 0x0F, ack);
          s = false;
          i = plength;
        }
        break;
      }
      case CMD_P: {
        if (s) {
          dev->stop();
          s = false;
          if (send[length - 1] == 0) {
            pc.printf("ACK,");
          } else {
            pc.printf("NAK,");
          }
          length--;
        }
        i = plength;
        for (int j = 0; j < length; j++) {
          pc.printf("%02X,", send[j]);
        }
        pc.printf("ok\n\r");
        break;
      }
      case CMD_R: {
        s = false;
        length = plength - 2;
        if (length < 1) {
          pc.printf("bad packet! %d\n\r", length);
          i = plength + 1;
          length = 0;
        } else {
          for (int j = 0; j < length; j++) {
            address = recieve[i + 1 + j];
            switch (address) {
            case CHIP_ID: {
              data = chip_id;
              break;
            }
            case GPIO0_STAT: {
              for (int k = 0; k < 8; k++) {
                _data = gpio0[k]->read();
                data |= (_data << k);
              }
              registers[GPIO0_STAT - '0'] = data;
              break;
            }
            case GPIO0_CONF: {
              data = registers[GPIO0_CONF - '0'];
              break;
            }
#ifdef isGPIO1
            case GPIO1_STAT: {
              for (int k = 0; k < 8; k++) {
                _data = gpio1[k]->read();
                data |= (_data << k);
              }
              registers[GPIO1_STAT - '0'] = data;
              break;
            }
            case GPIO1_CONF: {
              data = registers[GPIO1_CONF - '0'];
              break;
            }
#endif
            case I2C_CONF: {
              data = registers[I2C_CONF - '0'];
              break;
            }
            case SPI_CONF: {
              data = registers[SPI_CONF - '0'];
              break;
            }
            default: {
              data = 0xAA;
              break;
            }
            }
            send[j] = (char)data;
            data = 0;
          }
          i += (length + 1);
        }
        break;
      }
      case CMD_W: {
        s = false;
        length = plength - 2;
        if (length < 3) {
          pc.printf("bad packet! %d\n\r", length);
          i = plength + 1;
          length = 0;
        } else {
          for (int j = 0; j < length; j += 3) {
            address = recieve[i + 1 + j];
            data =
                0xff & (recieve[i + 2 + j] << 4 | (recieve[i + 3 + j] & 0x0F));
            _data = 0;
            switch (address) {
            case CHIP_ID: {
              // READ ONLY: do nothing
              data = registers[CHIP_ID - '0'];
              break;
            }
            case GPIO0_STAT: {
              _data = registers[GPIO0_CONF - '0'];
              for (int k = 0; k < 8; k++) {
                if (_data & 0x01) { // output
                  gpio0[k]->write((data >> k) & 0x01);
                } else { // input
                  ;      // do nothing
                }
                _data >>= 1;
              }
              break;
            }
            case GPIO0_CONF: {
              registers[GPIO0_CONF - '0'] = data;
              for (int k = 0; k < 8; k++) {
                if (data & 0x01) { // output
                  gpio0[k]->output();
                } else { // input
                  gpio0[k]->input();
                  gpio0[k]->mode(PullUp);
                }
                data >>= 1;
              }
              data = registers[GPIO0_CONF - '0'];
              break;
            }
#ifdef isGPIO1
            case GPIO1_STAT: {
              _data = registers[GPIO1_CONF - '0'];
              for (int k = 0; k < 8; k++) {
                if (_data & 0x01) { // output
                  gpio1[k]->write((data >> k) & 0x01);
                } else { // input
                  ;      // do nothing
                }
                _data >>= 1;
              }
              break;
            }
            case GPIO1_CONF: {
              registers[GPIO1_CONF - '0'] = data;
              for (int k = 0; k < 6; k++) {
                if (data & 0x01) { // output
                  gpio1[k]->output();
                } else { // input
                  gpio1[k]->input();
                  gpio1[k]->mode(PullUp);
                }
                data >>= 1;
              }
              data = registers[GPIO1_CONF - '0'];
              break;
            }
#endif
            case I2C_CONF: {
              registers[I2C_CONF - '0'] = data;
#if defined(TARGET_LPC1768)
              dev1.frequency(100000 * ((0x03 & (data >> 6)) + 1));
#else
              dev1.frequency(200000 * ((0x03 & (data >> 6)) + 1));
#endif
#ifdef isI2C2
              dev2.frequency(100000 * ((0x03 & (data >> 4)) + 1));
#endif
#ifdef isI2C3
              dev3.frequency(100000 * ((0x03 & (data >> 2)) + 1));
#endif
#ifdef isI2C4
              dev4.frequency(100000 * ((0x03 & (data >> 0)) + 1));
#endif
              break;
            }
            case SPI_CONF: {
              registers[SPI_CONF - '0'] = data;
              format = ((data & 0x04) + 4) << 1;
              _spi.format(format, 0x03 & (data));
              _spi.frequency(1000000 * ((0x07 & (data >> 4)) + 1));
              enabled = (data & 0x08) >> 3;
              /*
              7 not used
              6:4 frequency
              3 CE pol
              2 word size(0=8bit,1=16bit)
              1:0 pol(corresponds to spi.format())
              */
              disabled = ~enabled;
              break;
            }
            default: { break; }
            }
            send[j / 3] = data;
          }
          i += (length + 1);
          length /= 3;
        }
        break;
      }
      case CMD_I: {
        s = false;
        length = plength - 2;
        if (length < 1) {
          pc.printf("bad packet! %d\n\r", length);
          i = plength + 1;
          length = 0;
        } else {
          for (int j = 0; j < length; j++) {
            address = recieve[i + 1 + j];
            _data = 0;
            switch (address) {
            case GPIO0_STAT: {
              for (int k = 0; k < 8; k++) {
                _data = gpio0[k]->read();
                data |= (_data << k);
              }
              registers[GPIO0_STAT - '0'] = data;
              break;
            }
#ifdef isGPIO1
            case GPIO1_STAT: {
              for (int k = 0; k < 8; k++) {
                _data = gpio1[k]->read();
                data |= (_data << k);
              }
              registers[GPIO1_STAT - '0'] = data;
              break;
            }
#endif
            default: {
              data = 0xAA;
              break;
            }
            }
            send[j] = (char)data;
            data = 0;
          }
          i += (length + 1);
        }
        break;
      }
      case CMD_O: {
        s = false;
        length = plength - 2;
        if (length < 3) {
          pc.printf("bad packet! %d\n\r", length);
          i = plength + 1;
          length = 0;
        } else {
          for (int j = 0; j < length; j += 3) {
            address = recieve[i + 1 + j];
            data =
                0xff & (recieve[i + 2 + j] << 4 | (recieve[i + 3 + j] & 0x0F));
            switch (address) {
            case GPIO0_STAT: {
              _data = registers[GPIO0_CONF - '0'];
              for (int k = 0; k < 8; k++) {
                if (_data & 0x01) { // output
                  gpio0[k]->write(data & 0x01);
                } else { // input
                  ;      // do nothing
                }
                data >>= 1;
                _data >>= 1;
              }
              break;
            }
#ifdef isGPIO1
            case GPIO1_STAT: {
              _data = registers[GPIO1_CONF - '0'];
              for (int k = 0; k < 8; k++) {
                if (_data & 0x01) { // output
                  gpio1[k]->write(data & 0x01);
                } else { // input
                  ;      // do nothing
                }
                data >>= 1;
                _data >>= 1;
              }
              break;
            }
#endif
            default: { break; }
            }
            send[j / 3] = data;
          }
        }
        i += (length + 1);
        length /= 3;
        //                    pc.printf("command O is not implemented, ");
        break;
      }
      case CMD_E: {
        s = false;
        /*
        "0|   1   2|   3   4|   5   6  7  8  9 10 11 12|13" //plength=14
        "E| 0x_0 _1| 0x_0 _0| 0x_D _E| P"                   //minimum plength=8
        "E| 0x_0 _1| 0x_0 _0| 0x_D _E|_A _D| P"             //minimum
        plength=10(16bit)
        "E| 0x_0 _4| 0x_0 _0| 0x_D _E _A _D _B _E _A _F| P" //write
        "E| 0x_0 _4| 0x_0 _4| 0x_D _E _A _D _B _E _A _F| P" //write and read
        */
        length = plength - 2; // 6
        if (length < 6) {
          pc.printf("bad packet! %d\n\r", length);
          i = plength + 1;
          length = 0;
        } else {
          length = length - 4; // actual data in packet
          data = 0xff & ((recieve[i + 1] << 4) |
                         (recieve[i + 2] & 0x0F)); // write length
          read = 0xff & ((recieve[i + 3] << 4) |
                         (recieve[i + 4] & 0x0F)); // read length
          switch (format) {
          case 8: {
            _cs.write(enabled);
            for (int j = 0; j < length; j += 2) {
              _data = 0xff & ((recieve[i + 5 + j + 0] << 4) |
                              (recieve[i + 5 + j + 1] & 0x0F));
              ack = _spi.write(_data);
              // pc.printf("s%02X,",_data);
              send[j / 2] = ack;
            }
            for (int j = length; j < (length + 2 * read); j += 2) {
              ack = _spi.write(0xAA); // dummy data to write
              // pc.printf("a%02X,",ack);
              send[j / 2] = ack;
            }
            _cs.write(disabled);
            break;
          }
          case 16: {
            if ((data % 2) || (read % 2)) { // invalid
              pc.printf("bad packet! %d, %d\n\r", data, read);
              i = plength + 1;
              length = 0;
            } else {
              _cs.write(enabled);
              for (int j = 0; j < length; j += 4) {
                _data = 0xffff & (((recieve[i + 5 + j + 0] & 0x0F) << 12) |
                                  ((recieve[i + 5 + j + 1] & 0x0F) << 8) |
                                  ((recieve[i + 5 + j + 2] & 0x0F) << 4) |
                                  ((recieve[i + 5 + j + 3] & 0x0F) << 0));
                ack = _spi.write(_data);
                // pc.printf("s%04X,",_data);
                send[(j / 2) + 0] = 0xFF & (ack >> 8);
                send[(j / 2) + 1] = 0xFF & (ack >> 0);
              }
              for (int j = length; j < (length + 2 * read); j += 4) {
                ack = _spi.write(0xAAAA); // dummy data to write
                //                                        pc.printf("a%04X,",ack);
                send[(j / 2) + 0] = 0xFF & (ack >> 8);
                send[(j / 2) + 1] = 0xFF & (ack >> 0);
              }
              _cs.write(disabled);
            }
            break;
          }
          default: {
            pc.printf("this shold not happen %d\n\r", format);
            break;
          }
          }
          //                        pc.printf("command E is for SPI
          //                        transmission\n\r");
          length = read + data;
          i = (plength - 1);
        }
        break;
      }
      case 'Z': {
        s = false;
        pc.printf("command Z is not implemented\n\r");
        i = plength;
        break;
      }
      case 'V': {
        s = false;
        pc.printf("command V is not implemented\n\r");
        i = plength;
        break;
      }
      default: {
        s = false;
        pc.printf("command %c is not implemented\n\r", recieve[i]);
        i = plength;
        break;
      }
      }
    }
    i = 0;
    length = 0;
  }
}
