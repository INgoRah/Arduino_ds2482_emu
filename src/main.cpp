/*
 * Includes
 */
#include <Arduino.h>

/*
 * Library classs includes
 */
#include <Wire.h>
#include <OneWire.h>

/*
 * Local constants
 */
#define MAX_DATA 16

#define DS2482_CMD_RESET               0xF0	/* No param */
#define DS2482_CMD_SET_READ_PTR        0xE1	/* Param: DS2482_PTR_CODE_xxx */
#define DS2482_CMD_CHANNEL_SELECT      0xC3	/* Param: Channel byte - DS2482-800 only */
#define DS2482_CMD_WRITE_CONFIG        0xD2	/* Param: Config byte */
#define DS2482_CMD_1WIRE_RESET         0xB4	/* Param: None */
#define DS2482_CMD_1WIRE_SINGLE_BIT    0x87	/* Param: Bit byte (bit7) */
#define DS2482_CMD_1WIRE_WRITE_BYTE    0xA5	/* Param: Data byte */
#define DS2482_CMD_1WIRE_READ_BYTE     0x96	/* Param: None */
/* Note to read the byte, Set the ReadPtr to Data then read (any addr) */
#define DS2482_CMD_1WIRE_TRIPLET       0x78	/* Param: Dir byte (bit7) */

/* Values for DS2482_CMD_SET_READ_PTR */
#define DS2482_STATUS_REGISTER         		 0xF0
#define DS2482_READ_DATA_REGISTER            0xE1
#define DS2482_DEVICE_CONFIGURATION_REGISTER 0xC3
#define DS2482_CHANNEL_SELECTION_REGISTER    0xD2	/* DS2482-800 only */
#define DS2482_PORT_CONFIGURATION_REGISTER   0xB4   /* DS2483 only */

/**
 * Status Register bit definitions (read only)
 */
#define DS2482_REG_STS_DIR     0x80
#define DS2482_REG_STS_TSB     0x40
#define DS2482_REG_STS_SBR     0x20
#define DS2482_REG_STS_RST     0x10
#define DS2482_REG_STS_LL      0x08
#define DS2482_REG_STS_SD      0x04
#define DS2482_REG_STS_PPD     0x02
#define DS2482_REG_STS_1WB     0x01

/*
 * Objects
 */
// on pin PIN_OW (a max 4.7K resistor is necessary)
OneWire ds(4);

/*
 * Local variables
 */
const byte slave_adr = 0x18;
byte rd_data[MAX_DATA];
byte wr_data[MAX_DATA];
byte wr_ptr;
byte x_ptr;
byte ch, reg;
byte status;
/*
* Function declarations
*/
void receiveEvent(int howMany);
void requestEvent();

void setup() {
  Wire.begin(slave_adr);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  Serial.begin(115200);
  Serial.println("I2C Slave active");
  wr_ptr = 0;
  status = DS2482_REG_STS_LL;
}

void loop() {
  int i = 0, cnt;

  if (wr_ptr) {
    Serial.print("Rx [");
    Serial.print(wr_ptr);
    Serial.print("]: ");
    for (i = 0; i < wr_ptr; i++) {
      Serial.print(wr_data[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    x_ptr = 0;
    switch (wr_data[0])
    {
      case DS2482_CMD_1WIRE_RESET:
        Serial.println("1W Reset");
        wr_ptr = 0;
        break;
      case DS2482_CMD_CHANNEL_SELECT:
        x_ptr++;
        while (x_ptr >= wr_ptr) {
          // wait
        }
        ch = wr_data[x_ptr];
        x_ptr = 0;
        Serial.print("CH=");
        Serial.println(ch);
        break;
      case DS2482_CMD_SET_READ_PTR:
        x_ptr++;
        while (x_ptr >= wr_ptr) {
          // wait
        }
        reg = wr_data[x_ptr];
        wr_ptr = 0;
        break;
      default:
        Serial.println("unknown");
        break;
    }
  } else {
    delay(100);
  }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  if (Wire.available())
    wr_ptr = 0;
  while (Wire.available() && wr_ptr < MAX_DATA - 1) {
    wr_data[wr_ptr++] = Wire.read();
  }
  if (wr_data[0] == DS2482_CMD_RESET) {
    Serial.println("RESET");
    wr_ptr = 0;
    x_ptr = 0;
    status = DS2482_REG_STS_LL | DS2482_REG_STS_RST;
    return;
  }
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
    switch (reg)
    {
      case DS2482_STATUS_REGISTER:
        Wire.write(status);
        Serial.print("status=");
        Serial.println(status);
        status = DS2482_REG_STS_LL;
        break;
    }
}