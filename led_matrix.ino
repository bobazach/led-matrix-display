#include <avr/io.h>

#define ROW_1 A3
#define ROW_2 A1
#define ROW_3 13
#define ROW_4 11
#define ROW_5 2
#define ROW_6 4
#define ROW_7 6
#define ROW_8 8

#define COL_1 A2
#define COL_2 A0
#define COL_3 12
#define COL_4 10
#define COL_5 3
#define COL_6 5
#define COL_7 7
#define COL_8 9

const int rows[8] = {ROW_1, ROW_2, ROW_3, ROW_4, ROW_5, ROW_6, ROW_7, ROW_8};
// Arrays which will help call the correct AVR PORT function to drive row pins
char rowsPORTs[] = "CCBBDDDB";
const byte rowsPORTsValues[8] = {B00001000, B00000010, B00100000, B00001000, B00000100, B00010000, B01000000, B00000001};
const byte rowsPORTsValuesInverse[8] = {~B00001000, ~B00000010, ~B00100000, ~B00001000, ~B00000100, ~B00010000, ~B01000000, ~B00000001};
// Arrays which will help call the correct AVR PORT function to drive column pins
const int columns[8] = {COL_1, COL_2, COL_3, COL_4, COL_5, COL_6, COL_7, COL_8};
char colsPORTs[] = "CCBBDDDB";
const byte colsPORTsValues[8] = {B00000100, B00000001, B00010000, B00000100, B00001000, B00100000, B10000000, B00000010};
const byte colsPORTsValuesInverse[8] = {~B00000100, ~B00000001, ~B00010000, ~B00000100, ~B00001000, ~B00100000, ~B10000000, ~B00000010};

const byte OK[2][8] = {
  {0x7E, 0xE7, 0xC3, 0xC3, 0xC3, 0xC3, 0xE7, 0x7E}, // O
  {0x61, 0x63, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x63}  // K
};

const byte inverseOK[2][8] = {
  {0x81, 0x18, 0x3C, 0x3C, 0x3C, 0x3C, 0x18, 0x81}, // Inverse O
  {0x9E, 0x9C, 0x99, 0x93, 0x87, 0x93, 0x99, 0x9C}  // Inverse K
};

const byte name[4][8] = {
  {0b11111111,0b11111111,0b01110000,0b00111000,0b00011100,0b00001110,0b11111111,0b11111111},  // Z
  {0b11000011,0b11000011,0b11111111,0b11111111,0b11000011,0b11000011,0b11111111,0b01111110},  // A
  {0b01111111,0b11111111,0b11100000,0b11000000,0b11000000,0b11100000,0b11111111,0b01111111},  // C
  {0b11000011,0b11000011,0b11000011,0b11111111,0b11111111,0b11000011,0b11000011,0b11000011}	  // H
};	

void setup() {
    Serial.begin(9600);
    // Port D: pins 0-7
    // Port B: pins 8-13
    // Port C: pins A0-A5
    DDRD |= B11111100; // Sets pins 2-7 as OUTPUT
    DDRB |= B00111111; // Sets pins 8-13 as OUTPUT
    DDRC |= B00001111; // Sets pins A0-A3 as OUTPUT
}

void loop() {
  // displayMessage(OK, sizeof(OK)/sizeof(OK[0])); // OK
  // displayMessage(inverseOK, sizeof(inverseOK)/sizeof(inverseOK[0])); // inverse OK
  // displayMessage(name, sizeof(name)/sizeof(name[0])); // ZACH
  // Serial.print(sizeof(OK));
  displayMessage(OK);
  displayMessage(inverseOK);
  displayMessage(name);
  
  for(int row = 0; row < sizeof(OK)/sizeof(OK[0]); row++){ // Scrolls through letters in OK
    scrollLetter(OK[row]);
  }
  delay(1000);
  for(int row = 0; row < sizeof(name)/sizeof(name[0]); row++){ // Scrolls through letters in name, ZACH
    scrollLetter(name[row]);
  }
  delay(1000);
}

// Displays custom message with each letter lasting a second (# of cols * loops = 125 * 8 = 1000 miliseconds)
void displayMessage(byte message[][8]){
  int size = sizeof(message);
  for(int row = 0; row < size; row++){
    for (int i = 0; i < 125; i++) {
      displayLetter(message[row]);
      delay(1); // Short delay to avoid flickering
    }
  }
    delay(1000);
  }

// Displays a letter given a byte array representation of it
void displayLetter(byte letter[]){
  /* Can drive a row at a time by
     setting column pins to match
     row image and just one row
     low (rest NC or High) */
  for (int col = 0; col < 8; col++) {
    write("columns", col, "LOW"); // HIGH rows that overlap with this column light up
    for (int row = 0; row < 8; row++) {
      if (bitRead(letter[row], col) == 1) { // Light column pins by settings rows to match byte representation, O:LOW 1:HIGH
        write("rows", row, "HIGH");
      }else{
        write("rows", row, "LOW");
      }
    }
    delay(1); // Small delay to avoid flickering
    for (int row = 0; row < 8; row++) { // Reset columns and rows to light up next row
      write("rows", row, "LOW");
    }
    write("columns", col, "HIGH");
  }
}

// Scrolls letters in an upwards pattern
void scrollLetter(byte letter[]){
  byte tempLetter[9] = {0b00000000, letter[0], letter[1], letter[2], letter[3], letter[4], letter[5], letter[6], letter[7]}; // Include 0 at the beginning to add spacing between letters
  byte temp;
  for(int i = 0; i < 9; i++){ // Displays each sequential frame/pattern, 9 in total
    for(int t = 0; t < 10; t++){ // Duration of each frame/pattern
      displayLetter(tempLetter);
      delay(1); // Short delay to avoid flickering
    }
    for(int row = 8; row > 0; row--){ // Shift rows to the left, creating a scrolling effect
      temp = tempLetter[0]; // Store beginning row to wrap to the end
      tempLetter[row] = tempLetter[row-1];
      tempLetter[9] = temp; // Wrap beginning row to the end
    }
  }
}

/* Helper method to determine the right port, byte pattern, and byte arithmetic
   to drive the correct row or column, uses AVR library methods.
   e.g. write("rows", 5, "HIGH") => PORTD |= B00000100 => digitalWrite(ROW_5, HIGH) */
void write(char rowOrCol[], int index, char status[]){
  if(rowOrCol == "columns"){
    if(status == "LOW"){
      if(colsPORTs[index] == 'B'){
        PORTB &= colsPORTsValuesInverse[index];
      }else if(colsPORTs[index] == 'C'){
        PORTC &= colsPORTsValuesInverse[index];
      }else{
        PORTD &= colsPORTsValuesInverse[index];
      }
    }else{ // status == "HIGH"
      if(colsPORTs[index] == 'B'){
        PORTB |= colsPORTsValues[index];
      }else if(colsPORTs[index] == 'C'){
        PORTC |= colsPORTsValues[index];
      }else{
        PORTD |= colsPORTsValues[index];
      }
    }
  }else{ // rowOrCol == "rows"
    if(status == "LOW"){
      if(rowsPORTs[index] == 'B'){
        PORTB &= rowsPORTsValuesInverse[index];
      }else if(rowsPORTs[index] == 'C'){
        PORTC &= rowsPORTsValuesInverse[index];
      }else{
        PORTD &= rowsPORTsValuesInverse[index];
      }
    }else{ // status == "HIGH"
      if(rowsPORTs[index] == 'B'){
        PORTB |= rowsPORTsValues[index];
      }else if(rowsPORTs[index] == 'C'){
        PORTC |= rowsPORTsValues[index];
      }else{
        PORTD |= rowsPORTsValues[index];
      }
    } 
  }
}
