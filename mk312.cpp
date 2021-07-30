#include <Arduino.h>
#include <HardwareSerial.h>
#include "mk312.h"



SemaphoreHandle_t  semaphore_serial2;
byte key = 0x55;
const int retry_limit = 11;
byte brutenow = 0x00;
byte brute[2];


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

  if (xSemaphoreTake(semaphore_serial2, portMAX_DELAY) == pdTRUE)
  {
    while (Serial2.available())
    {
      Serial2.read();
    }
    Serial2.write(c, length + 4);
    Serial2.readBytes(c, 1);
    xSemaphoreGive(semaphore_serial2);
  }

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
  int i;

  c[0] = 0x3c;
  c[1] = (address >> 8) & 0xff;
  c[2] = address & 0xff;
  c[3] = c[0] + c[1] + c[2];

  for (i = 0; i < 4; i++)
  {
    c[i] = c[i] ^ key;
  }

  if (xSemaphoreTake(semaphore_serial2, portMAX_DELAY) == pdTRUE)
  {
    while (Serial2.available())
    {
      Serial2.read();
    }
    Serial2.write(c, 4);
    Serial2.readBytes(c, 3);
    xSemaphoreGive(semaphore_serial2);
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

//write 0x00 untill reading 0x07. must happen no later than 11 bytes
void mk312_sync() {
  int i;
  byte c;
  size_t count;

  Serial.printf("mk312 sync. key %02x\n", key);
  for (i = 0; i < retry_limit; i++)
  {
    c = 0x00 ^ key;

    while (Serial2.available())
    {
      Serial2.read();
    }
    Serial2.write(&c, 1);
    count = Serial2.readBytes(&c, 1);

    if (count > 0)
    {
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
  byte value = map(percent, 0, 99, 0, 255);
  mk312_write(ADDRESS_LEVELA, &value, 1);
}

void mk312_set_b(int percent)
{
  if (!mk312_get_adc_disabled())
  {
    return;
  }
  byte value = map(percent, 0, 99, 0, 255);
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

byte mk312_get_ramp_level()
{
  return mk312_read(ADDRESS_RAMP_LEVEL);
}

byte mk312_get_ramp_time()
{
  return mk312_read(ADDRESS_RAMP_TIME);

}

void mk312_ramp_start()
{
  byte c[2] = { COMMAND_START_RAMP, 0x02};
  mk312_write(ADDRESS_COMMAND_1, c, 2);
}

int mk312_get_battery_level()
{
  return map(mk312_read(ADDRESS_BATTERY_LEVEL), 0, 255, 0, 99);
}

int mk312_get_a()
{
  int value;
  value = mk312_read(ADDRESS_LEVELA);
  value = map(value, 0, 255, 0, 99);
  return value;
}

int mk312_get_b()
{
  int value;
  value = mk312_read(ADDRESS_LEVELB);
  value = map(value, 0, 255, 0, 99);
  return value;
}

int mk312_get_ma()
{
  byte ma_max;
  byte ma_min;
  byte value;

  ma_max = mk312_read(ADDRESS_MA_MAX_VALUE);
  ma_min = mk312_read(ADDRESS_MA_MIN_VALUE);
  value = mk312_read(ADDRESS_LEVELMA);
  value = map(value,  ma_max, ma_min, 0, 99);
  return value;
}

byte mk312_get_mode()
{
  return mk312_read(ADDRESS_CURRENT_MODE);
}

void init_mk312_easy()
{
  int i;
  byte buffer[16];
  int retry_count = 11;
  Serial2.begin(19200);
  Serial2.setTimeout(500);
  semaphore_serial2 = xSemaphoreCreateBinary();
  xSemaphoreGive(semaphore_serial2);


  Serial.write("mk312_init_easy first");
  if (xSemaphoreTake(semaphore_serial2, portMAX_DELAY) == pdTRUE)
  {
    while (Serial2.available()) //flush
    {
      Serial2.read();
    }
    for (i = 0; i < retry_count; i++)
    {
      Serial2.write(0x00);
      Serial.write('.');
      Serial2.readBytes(buffer, 1);

      if (buffer[0] == 0x07)
      {
        break;
      }
    }
    xSemaphoreGive(semaphore_serial2);
  }
  buffer[0] = 0x2f;
  buffer[1] = 0x00;
  buffer[2] = 0x2f;

  if (xSemaphoreTake(semaphore_serial2, portMAX_DELAY) == pdTRUE)
  {
    while (Serial2.available()) //flush
    {
      Serial2.read();
    }
    Serial2.write(buffer, 3);
    Serial2.readBytes(buffer, 3);
    xSemaphoreGive(semaphore_serial2);
  }
  Serial.printf("got %02x%02x%02x\n", buffer[0], buffer[1], buffer[2]);
  Serial.write("mk312_init_easy second");

  if (xSemaphoreTake(semaphore_serial2, portMAX_DELAY) == pdTRUE)
  {
    while (Serial2.available()) //flush
    {
      Serial2.read();
    }
    for (i = 0; i < retry_count; i++)
    {
      Serial2.write(0x55);
      Serial.write('.');
      Serial2.readBytes(buffer, 1);
      if (buffer[0] == 0x07)
      {
        break;
      }
    }
    xSemaphoreGive(semaphore_serial2);
  }

  if (i >= retry_count)
  {
    Serial.println("sync error");
  }
  else
  {
    Serial.println("hi");
  }
  Serial2.setTimeout(500);

}
