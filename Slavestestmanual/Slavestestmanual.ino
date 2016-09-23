void setup() {
  // put your setup code here, to run once:
  for (int motornum = 0; motornum < 4; motornum++) {
    pinMode(2 * motornum, OUTPUT); // Set direction pin as output. Direction pin = motornumber * 2
    pinMode(2 * motornum + 1, OUTPUT); // Set motor pin as output. Motor pin = motornumber * 2 + 1
    pinMode(motornum + 8, OUTPUT);
    digitalWrite(2 * motornum, 1);
  }
}

void loop() {
//  for (int i=0; i<4; i++){
//    digitalWrite(2*i+1, 1);
//    digitalWrite(2*i+1, 0);
//    delay(1);
//  }

  for (int motornum = 0; motornum < 4; motornum++){
  digitalWrite(motornum+8, HIGH);
  digitalWrite(2 * motornum, 1); // Set the direction
  digitalWrite(2 * motornum + 1, 1); // Step the motor in the direction set
  digitalWrite(2 * motornum + 1, 0);
//  digitalWrite(motornum+8, LOW);
  delay(10);
  }
}
