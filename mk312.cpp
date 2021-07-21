#include <Arduino.h>
#include <HardwareSerial.h>
#include "mk312.h"

char key = 0x00;
const int retry_limit = 11;


/*
   0xGd 0xHH 0xII [0xJJ 0xKK...] 0xCC

  0xGd - High nibble is amount of data to write to address plus 0x3, low nibble is always 0x0d
  0xHH - High byte of address
  0xII - Low byte of address
  [0xJJ 0xKK]... - Value(s) to set address to
  0xCC - Checksum
*/
void mk312_write (uint16_t address, char* payload, size_t length)
{
  char c[16];
  char sum;
  size_t count;
  int i;
  while (Serial2.available())
  {
    Serial2.read();
  }

  c[0] = 0x3d + (length << 4);
  c[1] = address & 0xff;
  c[2] = (address >> 8) & 0xff;
  sum = c[0] + c[1] + c[2];
  for (i = 0; i < length; i++)
  {
    c[i + 3] = payload[i];
    sum = sum + payload[i];
  }
  c[length + 4] = sum;
  for (i = 0; i < length + 4; i++)
  {
    c[i] = c[i] ^ key;
  }
  Serial2.write(c, length + 4);
  count = Serial2.readBytes(c, 16); //FIXME
  if (count > 0)
  {
    Serial.printf("got %d %02x%02x%02x%02x%02x%02x\n", count, c[0], c[1], c[2], c[3], c[4], c[5]);
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
  char c[4];
  char sum;
  size_t count;
  int i;
  while (Serial2.available())
  {
    Serial2.read();
  }

  c[0] = 0x3c;
  c[1] = address & 0xff;
  c[2] = (address >> 8) & 0xff;
  c[3] = c[0] + c[1] + c[2];

  for (i = 0; i < 4; i++)
  {
    c[i] = c[i] ^ key;
  }

  Serial2.write(c, 4);
  count = Serial2.readBytes(c, 3);
  if (count > 0)
  {
    Serial.printf("got %d %02x%02x%02x\n", count, c[0], c[1], c[2]);
  }
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
  char c[3] = {0x2f, 0x00, 0x2f};
  char sum;
  size_t count;
  while (Serial2.available())
  {
    Serial2.read();
  }
  Serial2.write(c, 3);
  count = Serial2.readBytes(c, 3);
  if (count > 0)
  {
    Serial.printf("got %d %02x%02x%02x\n", count, c[0], c[1], c[2]);
  }
  sum = c[0] + c[1];
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
  char c;
  size_t count;
  while (Serial2.available())
  {
    Serial2.read();
  }
  Serial.printf("mk312 sync. key %02x\n", key);
  for (i = 0; i < retry_limit; i++)
  {
    Serial.print("i: "); Serial.println(i);

    c = 0x00 ^ key;

    Serial2.write(&c, 1);
    Serial.printf("sent %02x\n", c);

    count = Serial2.readBytes(&c, 1);
    if (count > 0)
    {
      Serial.printf("got %02x\n", c);
      break;
    }
  }

  if (i >= retry_limit)
  {
    Serial.println("error: mk312 sync no reply");
  }
  if (c != 0x07)
  {
    Serial.println("error: mk312 sync wrong reply");
  }
}


void mk312_set_a(int percent)
{

}

void mk312_set_b(int percent)
{

}

void mk312_set_ma(int percent)
{

}

void init_mk312() {
  Serial2.begin(19200);
  Serial2.setTimeout(300);
  mk312_sync();
  mk312_key_exchange();
}
