#include "tuna.h"
#include "Arduino.h"

// PINS:
// 49 : ENAble
// 51 : DIRection
// 53 : PULse

void setup(void)
{
  pinMode(49, OUTPUT);
  pinMode(53, OUTPUT);
  digitalWrite(49, HIGH);
}

void loop(void)
{
  printf("looping\n");
  digitalWrite(49, HIGH);
  digitalWrite(53, HIGH);
  delay(1_u8);
  digitalWrite(53, LOW);
  delay(250_u8);
}
