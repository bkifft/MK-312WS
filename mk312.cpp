#include "mk312.h"

char key;
extern const int retry_limit;

void connect_mk312() {


}

void handshake() {
  int i;
  char c;
  char sum;
  for (i = 0; i < retry_limit; i++)
  {
    Serial2.print('\x00');


    while (Serial2.available() == 0)
    {
    };

    c = Serial2.read();
    if (c == '\x00')
    {
      break;
    }
  }

}
