#include <Wire.h>
#include <avr/wdt.h>

void setup(){
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  //  Serial.begin(9600);
  MCUSR = 0;  // clear out any flags of prior resets.
  for (int pin = 0; pin < 12; pin++) {
    pinMode(pin, OUTPUT); // Set direction and motor pin as output. Direction pin = pin * 2, Motor pin = pin * 2 + 1
  }
  for (int i = 8; i < 12; i++) {
    digitalWrite(i, LOW);
  }
  //  Serial.println("Ready");
}

void loop() {
  delay(100);
}

void receiveEvent(int howMany) {
  String instructions;
  while (Wire.available() > 0) {
    char newitem = Wire.read();
    instructions += newitem;
  }
  //  Serial.println(instructions);
  parseinstructions(instructions);
}

void parseinstructions(String instructions) {  // Parser function for each arduino that has already received instructions
  String dir = instructions.substring(0, 4); // String containing the direction of the 4 motors
  int steps[4];  // Array containing the number of steps of the 4 motors
  bool atstep = false;  // Flag to tell parser if it has reached the steps instructions
  int indice = 0;  // Indice of the array that the parser will add the next instruction to
  int i = 5;  // Indice of the instructions that the parser will start reading from
  int j;
  //  Serial.println("Parsing!");
  do {
    j = i;
    String numSteps = "";
    while (isDigit(instructions.charAt(j))) {  // Loop through the string from i until the next comma
      numSteps += instructions.charAt(j);
      j++;
    }
    steps[indice] = numSteps.toInt();  // Add it to the steps array
    indice++;  // Increase indice so the next instruction is added to the next indice
    i = j + 1;  // Increase the indice at which the parser will start from in the next loop
  } while (i < instructions.length());  // Ensure that you have not reached the end of the string of instructions
  execute(dir, steps);  // Call the executing function
}

void execute(String dir, int steps[4]) {  // Executing function to ensure that the motors take turns to step
  int sumofsteps;  // Initialize var for num of steps
  for (int i = 8; i < 12; i++) {
    digitalWrite(i, HIGH);
  }
  do {
    sumofsteps = 0;  // Set total number of steps left to 0
    for (int i = 0; i < 4; i++) {  // Loop through each motor
      if (steps[i] > 0) { // Make sure that the number of steps left for the motor is more than 0
        stepper(dir.substring(i, i + 1).toInt(), i); // Step the motor once in the direction
        steps[i] -= 1;  // Reduce the number of steps for the motor by 1
        sumofsteps += steps[i];  // Add the number of steps left for the motor to the total
      }
    }
  } while (sumofsteps != 0);  // Make sure that the total number of steps left is not 0
  wdt_enable(WDTO_15MS); // turn on the WatchDog and don't stroke it. RESET.
  for (;;) {
    // do nothing and wait for the eventual... reset.
  }
}

void stepper(int dir, int motornum) {
  digitalWrite(2 * motornum, dir); // Set the direction
  digitalWrite(2 * motornum + 1, HIGH); // Step the motor in the direction set
  delay(10);
  digitalWrite(2 * motornum + 1, LOW);
  delay(10);
}

//void calibrate() {
//  digitalWrite(12, HIGH);
//  for (int j = 0; j < 300; j++) {
//    for (int i = 1; i < 5; i++) { // Loop through each motor
//      stepper(1, i);  // Step the motor backwards (Todo check the direction of real stepper motor)
//    }
//  }
//  for (int j = 0; j < 300; j++) {
//    for (int i = 1; i < 5; i++) { // Loop through each motor
//      stepper(0, i);  // Step the motor backwards (Todo check the direction of real stepper motor)
//    }
//  }
//  digitalWrite(12, LOW);
//  wdt_enable(WDTO_15MS); // turn on the WatchDog and don't stroke it. RESET.
//  for (;;) {
//    // do nothing and wait for the eventual... reset.
//  }
//}

