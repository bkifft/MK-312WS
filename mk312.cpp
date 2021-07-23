#include <Arduino.h>
#include <HardwareSerial.h>
#include "mk312.h"

byte key = 0x00;
const int retry_limit = 11;


/*
   0xGd 0xHH 0xII [0xJJ 0xKK...] 0xCC

  0xGd - High nibble is amount of data to write to address plus 0x3, low nibble is always 0x0d
  0xHH - High byte of address
  0xII - Low byte of address
  [0xJJ 0xKK]... - Value(s) to set address to
  0xCC - Checksum
*/
void mk312_write (uint16_t address, byte* payload, size_t length)
{
  if (length > 8)
  {
    Serial.println("error: mk312_write longer than eight bytes");
    return;
  }
  byte c[16];
  byte sum;
  int i;
  size_t count;
  c[0] = 0x3d + (length << 4);
  c[1] = (address >> 8) & 0xff;
  c[2] = address & 0xff;
  sum = (c[0] + c[1] + c[2]) & 0xff;
  for (i = 0; i < length; i++)
  {
    c[i + 3] = payload[i];
    sum = (sum + payload[i]) & 0xff;
  }
  c[length + 3] = sum;
  for (i = 0; i < length + 4; i++)
  {
    c[i] = c[i] ^ key;
  }
  while (Serial2.available())
  {
    Serial2.read();
  }
  delay(5);
  Serial2.read();
  Serial2.write(c, length + 4);
 // Serial.printf("sent %02x%02x%02x%02x%02x%02x\n", c[0], c[1], c[2], c[3], c[4], c[5]);
  delay(20);
  count = Serial2.readBytes(c, 1); //FIXME
  if (c[0] != 0x06)
  {
    Serial.printf("error: received wrong write reply, got %02x\n", c[0]);
  }

}
/*
  0x3c 0xGG 0xHH 0xCC
  0xHH - High byte of address
  0xII - Low byte of address
  0xCC - Checksum
  The box will then respond with two bytes (plus checksum, as above)

  0x22 0xVV 0xCC
  0xVV - Content of requested address
  0xCC - Checksum

*/
char mk312_read (uint16_t address)
{
  byte c[4];
  byte sum;
  size_t count;
  int i;

  c[0] = 0x3c;
  c[1] = (address >> 8) & 0xff;
  c[2] = address & 0xff;
  c[3] = c[0] + c[1] + c[2];

  for (i = 0; i < 4; i++)
  {
    c[i] = c[i] ^ key;
  }

  while (Serial2.available())
  {
    Serial2.read();
  }
  delay(5);
  Serial2.read();
  Serial2.write(c, 4);
  delay(20);
  count = Serial2.readBytes(c, 4);
 /* if (count > 0)
  {
    Serial.printf("got %d %02x%02x%02x\n", count, c[0], c[1], c[2]);
  }*/
  sum = c[0] + c[1];

  if (sum != c[2])
  {
    Serial.printf("error: wrong read checksum got %02x calc %02x\n", c[2], sum);
  }

  if (c[0] != 0x22)
  {
    Serial.printf("error: wrong read reply got %02x\n", c[0]);
  }

  return c[1];

}

//[0x2f, 0xVV, 0xWW] sent to ET312
//[0x21, 0xXX, 0xYY] is read from ET312
void mk312_key_exchange()
{
  byte c[3] = {0x00};
  byte sum;
  size_t count;

  c[0] = 0x2f; //setkey command
  c[1] = 0x00; //hostkey
  c[2] = 0x2f; //checksum

  while (Serial2.available()) //flush
  {
    Serial2.read();
  }

  Serial2.write(c, 3);

  count = Serial2.readBytes(c, 3);
  /*if (count > 0)
  {
    Serial.printf("got %d %02x%02x%02x\n", count, c[0], c[1], c[2]);
  }*/
  sum = (c[0] + c[1]) & 0xff;;
  if (sum != c[2])
  {
    Serial.printf("error: wrong key exchange checksum got %02x calc %02x\n", c[2], sum);
  }
  if (c[0] != 0x21)
  {
    Serial.printf("error: wrong key exchange reply got %02x\n", c[0]);
  }

  key = 0x55 ^ c[1];
  Serial.printf("set key to %02x\n", key);

}


//write 0x00 untill reading 0x07. must happen no later than 11 bytes
void mk312_sync() {
  int i;
  byte c;
  size_t count;

  Serial.printf("mk312 sync. key %02x\n", key);
  for (i = 0; i < retry_limit; i++)
  {
    Serial.print("i: "); Serial.println(i);

    c = 0x00 ^ key;

    while (Serial2.available())
    {
      Serial2.read();
    }
    Serial2.write(&c, 1);
    //Serial.printf("sent %02x\n", c);

    count = Serial2.readBytes(&c, 1);
    /*if (count > 0)
    {
      Serial.printf("got %02x\n", c);
      break;
    }*/
  }

  if (i >= retry_limit)
  {
    Serial.println("error: mk312 sync no reply");
  }
  if (c != 0x07)
  {
    Serial.println("error: mk312 sync wrong reply, trying default key");
    if (key != 0x55)
    {
      key = 0x55; //only works with modified firmware that allways uses 00. resulting key is 00^00^55
      mk312_sync();
    }
  }
}



void mk312_enable_adc()
{
  byte c = mk312_read(ADDRESS_R15);
  c = c & ~(1 << REGISTER_15_ADCDISABLE);
  mk312_write(ADDRESS_R15, &c, 1);
}

void mk312_disable_adc()
{
  byte c = mk312_read(ADDRESS_R15);
  c = c | (1 << REGISTER_15_ADCDISABLE);
  mk312_write(ADDRESS_R15, &c, 1);
}

bool mk312_get_adc_disabled()
{
  byte c = mk312_read(ADDRESS_R15);

  if (c & (1 << REGISTER_15_ADCDISABLE))
  {
    return true;
  }
  return false;
}


void mk312_set_a(int percent)
{
  if (!mk312_get_adc_disabled())
  {
    return;
  }
  byte value = map(percent, 0, 100, 0, 255);
  mk312_write(ADDRESS_LEVELA, &value, 1);
}

void mk312_set_b(int percent)
{
  if (!mk312_get_adc_disabled())
  {
    return;
  }
  byte value = map(percent, 0, 100, 0, 255);
  mk312_write(ADDRESS_LEVELB, &value, 1);
}

void mk312_set_ma(int percent)
{
  if (!mk312_get_adc_disabled())
  {
    return;
  }
  byte ma_max;
  byte ma_min;
  byte value;

  ma_max = mk312_read(ADDRESS_MA_MAX_VALUE);
  ma_min = mk312_read(ADDRESS_MA_MIN_VALUE);
  value = map(percent, 0, 100, ma_max, ma_min);
  Serial.printf("set_ma: max %02x min %02x val %02x", ma_max, ma_min, value);
  mk312_write(ADDRESS_LEVELMA, &value, 1);

}

void mk312_set_mode(byte newmode)
{
  if (newmode == mk312_read(ADDRESS_CURRENT_MODE))
  {
    return;
  }
  byte commands[2] =
  {
    COMMAND_EXIT_MENU,
    COMMAND_NEW_MODE
  };
  mk312_write(ADDRESS_CURRENT_MODE, &newmode, 1);
  mk312_write(ADDRESS_COMMAND_1, commands, 2);
}

void init_mk312() {
  Serial2.begin(19200);
  Serial2.setTimeout(500);
  mk312_sync();
  mk312_key_exchange();
  // mk312_write (0x4010, "\xFE\xFF", 2);
}
