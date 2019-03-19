#include "HID-Project.h"

void setup() {
  // put your setup code here, to run once:
  Serial1.begin(115200);
  Keyboard.begin();
}


void loop() {
  // put your main code here, to run repeatedly:
  char c = Serial1.read();
//  Keyboard.print(c);
  if(c == 'U'){
    Keyboard.write(KEY_ENTER);
    delay(750);
    Keyboard.println("YOUR_PASSWORD_HERE");
    delay(100);
  }
}
