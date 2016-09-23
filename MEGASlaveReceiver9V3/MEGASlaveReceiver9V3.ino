#include <Wire.h>

void setup() {
  Wire.begin(9);                // join i2c bus with address #9
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);
  Serial.println("Ready");
}

void loop() {
  delay(100);
}

void receiveEvent(int meow){
  String instructions;
  while (Wire.available() > 0){
    char newitem = Wire.read();
    instructions += newitem;
  }
  parseinstructions(instructions);
}

void parseinstructions(String instructions) {  // Parser function for each arduino that has already received instructions
  String dir = instructions.substring(0,4);  // String containing the direction of the 4 motors
  int steps[4];  // Array containing the number of steps of the 4 motors
  bool atstep = false;  // Flag to tell parser if it has reached the steps instructions
  int indice = 0;  // Indice of the array that the parser will add the next instruction to
  int i = 5;  // Indice of the instructions that the parser will start reading from
  int j;
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

void execute(String dir, int steps[19]) {  // Executing function to ensure that the motors take turns to step
  int sumofsteps;  // Initialize var for num of steps
  do {
    sumofsteps = 0;  // Set total number of steps left to 0
    for (int i = 0; i < 19; i++) {  // Loop through each motor
      if (steps[i] > 0){  // Make sure that the number of steps left for the motor is more than 0
        if (i > 5) {
          stepper(dir.substring(i,i+1).toInt(), i+5);
        }
        else {
          stepper(dir.substring(i,i+1).toInt(), i+1);  // Step the motor once in the direction
        }
        steps[i] -= 1;  // Reduce the number of steps for the motor by 1
        sumofsteps += steps[i];  // Add the number of steps left for the motor to the total
      }
    }
  }while (sumofsteps != 0);  // Make sure that the total number of steps left is not 0
}

void stepper(int dir, int motornum){
  pinMode(2*motornum, OUTPUT);  // Set direction pin as output. Direction pin = motornumber * 2
  pinMode(2*motornum+1, OUTPUT);  // Set motor pin as output. Motor pin = motornumber * 2 + 1
  digitalWrite(2*motornum, dir);  // Set the direction
  digitalWrite(2*motornum, HIGH);
  digitalWrite(2*motornum+1, HIGH);  // Step the motor in the direction set
  delay(500);
  digitalWrite(2*motornum+1, LOW);
  digitalWrite(2*motornum, LOW);
  delay(500);
}
