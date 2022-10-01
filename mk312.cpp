#include <Arduino.h>
#include <HardwareSerial.h>
#include "mk312.h"
#include <esp_task_wdt.h>



SemaphoreHandle_t  semaphore_serial2;
byte key = 0x55;
const int retry_limit = 4;
String leftover;
const int timeout = 250;

/*
   0xGd 0xHH 0xII [0xJJ 0xKK...] 0xCC

  0xGd - High nibble is amount of data to write to address plus 0x3, low nibble is always 0x0d
  0xHH - High byte of address
  0xII - Low byte of address
  [0xJJ 0xKK]... - Value(s) to set address to
  0xCC - Checksum
*/

bool mk312_write_single (uint16_t address, byte* payload, size_t length)
{
  if (length > 8)
  {
    Serial.println("error: mk312_write longer than eight bytes");
    return false;
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
  leftover = "";
  if (xSemaphoreTake(semaphore_serial2,  portTICK_PERIOD_MS * 2 * timeout) == pdTRUE)
  {
    while (Serial2.available())
    {
      leftover = leftover + String(Serial2.read(), HEX);
    }
    if (leftover.length() > 0)
    {
      Serial.println("leftovers in write: " + leftover);
    }
    Serial2.write(c, length + 4);
    c[0] = '\0';
    Serial2.readBytes(c, 1);
    xSemaphoreGive(semaphore_serial2);
  }
  else
  {
    Serial.println("error: write didn't get semaphore");
    return false;
  }

  if (c[0] != 0x06)
  {
    Serial.println("error: received wrong write reply, got " + String(c[0], HEX));
    return false;
  }
  return true;
}

void mk312_write (uint16_t address, byte* payload, size_t length)
{
  for (int i = 0; i < retry_limit; i++)
  {
    esp_task_wdt_reset();
    if (mk312_write_single(address, payload, length))
    {
      return;
    }
    Serial.println("error: failed mk312_write " + String(address, HEX) +" try " + i);
    
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
bool mk312_read_single (uint16_t address, byte* retval)
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
  leftover = "";
  if (xSemaphoreTake(semaphore_serial2, portTICK_PERIOD_MS * 2 * timeout) == pdTRUE)
  {
    while (Serial2.available())
    {
      leftover = leftover + String(Serial2.read(), HEX);
    }
    if (leftover.length() > 0)
    {
      Serial.println("leftovers in read: " + leftover);
    }
    Serial2.write(c, 4);
    memset(c, '\0', 3);
    Serial2.readBytes(c, 3);
    xSemaphoreGive(semaphore_serial2);
  }
  sum = c[0] + c[1];
  if (sum != c[2])
  {
    Serial.println("error: wrong read checksum expected and got " + String(sum, HEX) + String(c[2], HEX));
    return false;
  }

  if (c[0] != 0x22)
  {
    Serial.println("error: wrong read reply got " + String(c[0], HEX));
    return false;
  }

  *retval = c[1];
  return true;
}


byte mk312_read (uint16_t address)
{
  byte retval = 0;
  for (int i = 0; i < retry_limit; i++)
  {
    esp_task_wdt_reset();
    if (mk312_read_single(address, &retval))
    {
      return retval;
    }
    Serial.println("error: failed mk312_read " + String(address, HEX) +" try " + i);
  }
  return retval;
}

/* deprecated for easyinit
//write 0x00 untill reading 0x07. must happen no later than 11 bytes
void mk312_sync() {
  int i;
  byte c;
  size_t count;

  for (i = 0; i < 11; i++)
  {
    c = 0x00 ^ key;
    if (xSemaphoreTake(semaphore_serial2, portTICK_PERIOD_MS * 2 * timeout) == pdTRUE)
    {
      while (Serial2.available())
      {
        leftover = leftover + String(Serial2.read());
      }
      if (leftover.length() > 0)
      {
        Serial.println("leftovers in sync: " + leftover);
      }
      Serial2.write(&c, 1);
      count = Serial2.readBytes(&c, 1);
      xSemaphoreGive(semaphore_serial2);
    }
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
*/

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

void mk312_all_off()
{
  mk312_set_a(0);
  mk312_set_b(0);
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
  Serial.printf("set_ma: max %02x min %02x val %02x\r\n", ma_max, ma_min, value);
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

void mk312_set_ramp_level()
{
  
}

void mk312_set_ramp_time()
{
  
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


void reinit_mk312_easy()
{
  int i = 0;
  byte buffer[16];
  const int retry_count = 11;
  Serial2.setTimeout(timeout);

  if (xSemaphoreTake(semaphore_serial2, portMAX_DELAY) == pdTRUE)
  {
    while (Serial2.available()) //flush
    {
      leftover = leftover + String(Serial2.read());
    }
    if (leftover.length() > 0)
    {
      Serial.println("leftovers in easyinit_1: " + leftover);
    }
    
    for (i = 0; i < retry_count; i++) //spam input untill device expecxts new command
    {
      Serial2.write(0x00);
      Serial2.readBytes(buffer, 1);

      if (buffer[0] == 0x07)
      {
        break;
      }
      else
      {
        Serial.println("easyinit: try" + String(i) + String(" got ") + String(buffer[0])); 
      }
    }

    //try to set host key to 0x00
    buffer[0] = 0x2f;
    buffer[1] = 0x00;
    buffer[2] = 0x2f;

    while (Serial2.available()) //flush
    {
      leftover = leftover + String(Serial2.read());
    }
    if (leftover.length() > 0)
    {
     Serial.println("leftovers in easyinit21: " + leftover);
    }
    Serial2.write(buffer, 3);
    Serial2.readBytes(buffer, 3); //dont realy care
    
    while (Serial2.available()) //flush
    {
      leftover = leftover + String(Serial2.read());
    }
    if (leftover.length() > 0)
    {
      Serial.println("leftovers in easyinit_3: " + leftover);
    }
    for (i = 0; i < retry_count; i++)
    {
      Serial2.write(0x55);
      //serial.write('.');
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
    Serial.println("synced");
  }
  Serial2.setTimeout(timeout);

}

void init_mk312_easy()
{
  Serial2.begin(19200);
  semaphore_serial2 = xSemaphoreCreateBinary();
  xSemaphoreGive(semaphore_serial2);
  reinit_mk312_easy();  
}


void mk312_inc_a()
{
  int a = mk312_get_a();
  if (a < 99 )
  {
    a++;
    mk312_set_a(a);  
  }
}

void mk312_inc_b()
{
  int b = mk312_get_b();
  if (b < 99 )
  {
    b++;
    mk312_set_b(b);  
  }
}

void mk312_inc_ma()
{
  int ma = mk312_get_ma();
//  Serial.print("ma inc: ");
//Serial.println(ma);

  if (ma < 99 )
  {
    ma++;
    mk312_set_ma(ma);  
 //   Serial.print("ma inced: ");
//Serial.println(ma);

  }
}

void mk312_dec_a()
{
  int a = mk312_get_a();
  if (a > 0 )
  {
    a--;
    mk312_set_a(a);  
  }
}

void mk312_dec_b()
{
  int b = mk312_get_b();
  if (b > 0 )
  {
    b--;
    mk312_set_b(b);  
  }
}

void mk312_dec_ma()
{
  int ma = mk312_get_ma();
  //  Serial.print("ma dec: ");
//Serial.println(ma);

  if (ma > 0 )
  {
    ma--;
    mk312_set_ma(ma);  
  }
}
