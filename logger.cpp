#include <RingBuf.h>
#include <Arduino.h>
#include "logger.h"

RingBuf<byte, LOG_SIZE> log_buffer;


void free_log_buffer()
{
  byte d;
  while (log_buffer.pop(d))
  {
    if (d == '\n')
    {
      break;
    }
  }
}

void log(String msg)
{
  for (int i = 0; i < msg.length(); i++)
  {
    if (!log_buffer.push(msg[i]))
    {
      free_log_buffer();
      free_log_buffer();
      log_buffer.push(msg[i]);
    }
  }
  log_buffer.push('\n');
}

int dump_log(char* buffer, int buffer_size)
{
  int i = 0;
  int ring_size = log_buffer.size();
  int limit = min(ring_size,buffer_size);
   for (i = 0; i < limit; i++)
    {
      buffer[i] = log_buffer[i];
    }
    buffer[limit - 1] = '\0';

  return limit;
  
}
