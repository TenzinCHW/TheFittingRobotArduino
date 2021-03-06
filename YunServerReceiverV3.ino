/*
  Arduino Yún Bridge example

  This example for the Arduino Yún shows how to use the
  Bridge library to access the digital and analog pins
  on the board through REST calls. It demonstrates how
  you can create your own API when using REST style
  calls through the browser.

  Possible commands created in this shetch:

  "/arduino/digital/13"     -> digitalRead(13)
  "/arduino/digital/13/1"   -> digitalWrite(13, HIGH)
  "/arduino/analog/2/123"   -> analogWrite(2, 123)
  "/arduino/analog/2"       -> analogRead(2)
  "/arduino/mode/13/input"  -> pinMode(13, INPUT)
  "/arduino/mode/13/output" -> pinMode(13, OUTPUT)

  This example code is part of the public domain

  http://www.arduino.cc/en/Tutorial/Bridge

*/

// A5 is SCL, A4 is SDA.
// I found that after 1 request to the Bridge REST API, I get a 500 error (internal server error)
// To solve this, I used the soft reset, avr/wdt.h SoftReset Library

#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <Process.h>
#include <Wire.h>

// Listen to the default port 5555, the Yún webserver
// will forward there all the HTTP requests you send
BridgeServer server;

void setup() {
  // Bridge startup
  Bridge.begin();
  Serial.begin(9600);
  Wire.begin();
  for (int motornum = 1; motornum < 6; motornum++) {
    pinMode(2 * motornum, OUTPUT); // Set direction pin as output. Direction pin = motornumber * 2
    pinMode(2 * motornum + 1, OUTPUT); // Set motor pin as output. Motor pin = motornumber * 2 + 1
  }
  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();
  Serial.println("Meow is ready.");
}

// The instuctions on the server will be in the form of "slave1/0,1,1,0/30,24,70,25*slave2/1,0,0,1/57,45,3,54"
// where the * sign delineates the instructions for two different slaves

void loop() {
  // Get clients coming from server
  BridgeClient client = server.accept();  // Is there a new client?
  if (client) {  // Process request
    process(client);  // Close connection and free resources.
    client.stop();
    Serial.println("Completed processing of client and restarting to prevent Error 500 on Bridge.");
  }
  delay(50);
}

void process(BridgeClient client) {
  String instructions = client.readString();
  String instructionlist[5];
  String subinstructions = "";  // Initialize subinstructions to be used to send info to the slaves
  unsigned short int counter = 0;
  for (int i = 0; i < instructions.length() - 2; i++) { // Loop through all but the \r\n characters
    if (instructions.charAt(i) != 'c') { // As long as you have not reached the next slave's commands
      subinstructions += instructions.charAt(i);  // Add on the characters at the ith index to the subinstructions
    }
    else if (instructions.charAt(i) == 'c') {
      instructionlist[counter] = subinstructions; // Append the subinstructions to the list of instructions
      counter += 1;
      subinstructions = "";  // Reset the subinstructions
    }
  }
  sendinstructions(instructionlist);  // Call the sending function
}

void sendinstructions(String instructionlist[]) {
  String commands = "";
  String theinstructions = "";
  for (int i = 0; i < 5; i++) {
    theinstructions = instructionlist[i];
    if (theinstructions.substring(0, 5) == "slave") {
      Serial.println("Sending!");
      Serial.println("Sending to " + (String)i);
      commands = theinstructions.substring(5);  // Slice the string to get just the commands
      Serial.println("Sending " + commands);
      if (i == 0 || i == 1) {
        Wire.beginTransmission(8); // Begin transmitting
        for (int j = 0; j < commands.length(); j++) {
          Wire.write(commands.charAt(j));  // Send the commands
        }
        Wire.endTransmission();  // Stop transmitting

//        theinstructions = instructionlist[i+1];
//        Wire.beginTransmission(8);
//        for (int j = 0; j < commands.length(); j++) {
//          Wire.write(commands.charAt(j));  // Send the commands
//        }
//        Wire.endTransmission();  // Stop transmitting
      }
      else{
        Wire.beginTransmission(9); // Begin transmitting
        for (int j = 0; j < commands.length(); j++) {
          Wire.write(commands.charAt(j));  // Send the commands
        }
        Wire.endTransmission();  // Stop transmitting
      }
    }
    else {
      commands = theinstructions.substring(7);
      parseinstructions(commands);
    }
  }
}

void parseinstructions(String instructions) {  // Parser function for each arduino that has already received instructions
  String dir = instructions.substring(0, 4); // String containing the direction of the 4 motors
  int steps[4];  // Array containing the number of steps of the 4 motors
  bool atstep = false;  // Flag to tell parser if it has reached the steps instructions
  int indice = 0;  // Indice of the array that the parser will add the next instruction to
  int i = 5;  // Indice of the instructions that the parser will start reading from
  int j;
  Serial.println("Entered parsing");
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
  Serial.println("Entered execution");
  int sumofsteps;  // Initialize var for num of steps
  do {
    sumofsteps = 0;  // Set total number of steps left to 0
    for (int i = 0; i < 4; i++) {  // Loop through each motor
      if (steps[i] > 0) { // Make sure that the number of steps left for the motor is more than 0
        stepper(dir.substring(i, i + 1).toInt(), i + 1); // Step the motor once in the direction
        steps[i] -= 1;  // Reduce the number of steps for the motor by 1
        sumofsteps += steps[i];  // Add the number of steps left for the motor to the total
      }
    }
  } while (sumofsteps != 0);  // Make sure that the total number of steps left is not 0
  delay(100);
}

void stepper(int dir, int motornum) {
  digitalWrite(2 * motornum, dir); // Set the direction
  digitalWrite(2 * motornum + 1, HIGH); // Step the motor in the direction set
  delay(0.5);
  digitalWrite(2 * motornum + 1, LOW);
  delay(0.5);
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
//  delay(100);
//}

