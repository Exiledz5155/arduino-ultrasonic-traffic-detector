/*
  Ultrasonic Sensor HC-SR04 and Arduino for Car detection
  Danny Bui, Yasuki Wu, Hunter Tu
*/
#include <movingAvg.h>
#include <SD.h>

// defines pins numbers
const int trigPin = 8;
const int echoPin = 9;

// defines variables
File file;
int chipSelect = 10;
unsigned long milliseconds_in_day = 3600000; // Update to actual day
unsigned long next_day = 3600000; // Advance to next day when this is passed
unsigned long delay_seconds = 4000; // In milliseconds
unsigned long time_passed; // In milliseconds
int comma_pos = 0;

long duration;
int distance;
double carCount = 0.0; 
bool carDetected = false;
int SENSE_DELAY_MILLIS = 0;
int cyclesSinceLastPulse = 0;
int carDet2 = 0;
int carSavchk = 0;
int day = 1;
  
movingAvg SLOW_AVG(250);
movingAvg FAST_AVG(25);

void setup() {
//MHz
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  SLOW_AVG.begin();
  FAST_AVG.begin();
  
  // Open serial communications and wait for port to open:
  Serial.println("start");
  Serial.begin(9600); 
  
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  // Check existence of file, delete the file if it exists to allow for starting over
  if(SD.exists("data.csv")) {
    Serial.println("File exists, will remove");
    if(SD.remove("data.csv") == true){
      Serial.println("file removed");
    }else{
      Serial.println("could not remove file");
    }
  }

  // Inital write to file, first rows
  file = SD.open("data.csv", FILE_WRITE);
  if(file){ 
    file.println(",cars");
    file.print("day 1,");
    comma_pos = file.position();
    file.print(carCount);
    file.close();
  }

}



void write_car() {
  file = SD.open("data.csv", O_RDWR);// Read write mode allows seeking?
  char character;
  if(file) { 
    file.seek(comma_pos);
    //Serial.println(carCount);
    file.print(carCount);
    file.close();
  } else {
    Serial.println("could not open for writing");
  }

}

void takeReading() {

  if (cyclesSinceLastPulse > SENSE_DELAY_MILLIS) {

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;

    FAST_AVG.reading(distance);
    SLOW_AVG.reading(distance);

    cyclesSinceLastPulse = 0;
  } else {
    cyclesSinceLastPulse++;
  }

}

void carSav() {

  if(carSavchk > SENSE_DELAY_MILLIS) {
    if(carDet()) {
      carDet2 = FAST_AVG.getAvg();
      } else { 
      carDet2 = 0;
      }
      carSavchk = 0;
  } else {
      carSavchk++;
  }

}

bool carDet() {

  int fastAvg = FAST_AVG.getAvg();
  int slowAvg = SLOW_AVG.getAvg();

  return fastAvg < slowAvg && fastAvg < (slowAvg + 12);
}

void updateNum(int distanceV) {

  if((carDetected == false) && (distanceV > 50) && (distanceV < 450)){
    carDetected = true;
  } 
  else if((carDetected == true) && (!carDet())) { // back to idle
    carDetected = false;
    carCount += 0.5;
    write_car();
  }

}

void loop() {
  
  takeReading();
  carSav();
  updateNum(carDet2);

//  // Prints the distance on the Serial Monitor
//  Serial.print("Distance: ");xq
//  Serial.println(distance);
//  Serial.print(carCount);

  time_passed = millis(); // time_passed is set to the amount of time in milliseconds that have passed since the arduino was connected
  // Advance to next day, write data
  if(time_passed > next_day) {
    //Serial.println(day);
    next_day += milliseconds_in_day;
    carCount = 0.0;
    day += 1;
    // Write
    // Day n: 
    // # Cars: 
    file = SD.open("data.csv", FILE_WRITE); // Open file in write mode
    if(file) {
      // Write data
      file.println();
      file.print("day ");
      file.print(day);
      file.print(",");
      comma_pos = file.position();
      file.print(carCount);
      file.close();
    } else {
      Serial.println("could not open file for writing");
    }
  
  }
}
