#include <digitalWriteFast.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// PINY WEJSCIOWE
#define INPIN1  5     // kosz lewy
#define INPIN2  6     // kosz prawy
#define INPIN3  7     // wolne
#define INPIN4  8     // wolne

// PINY WYJSCIOWE NA PRZEKAZNIKI
#define OUTPIN1 16    //  kosz lewy
#define OUTPIN2 17    //  kosz prawy
#define OUTPIN3 18    //  podloga lewy
#define OUTPIN4 19    //  podloga prawy
#define OUTPIN5 0     //  wolne
#define OUTPIN6 1     //  wolne
#define OUTPIN7 2     //  wolne
#define OUTPIN8 3     //  wolne

#define NEOPIN  4   // NEOPIXEL
#define LEDPIN  14  // LED
#define BTNPIN  15  // w przyszlosci PC1/ pin 15, obecnie A7/ADC7 pin A7

int inPin1_State,
    inPin2_State,
    inPin3_State,
    inPin4_State;

// nRF24L01 DEFINICJE
const byte address[17] = "1100110011001100";  // adres nadajnika [todo: zrobic to bezpieczniejsze]
RF24 radio(9, 10); // CE, CSN 
float blow_data;

//
float bme_data;
int data=0;



void read_input_pins()
{
  inPin1_State = digitalReadFast(INPIN1);
  inPin2_State = digitalReadFast(INPIN2);
  inPin3_State = digitalReadFast(INPIN3);
  inPin4_State = digitalReadFast(INPIN4);

  if(inPin1_State == LOW || inPin2_State == LOW || inPin3_State == LOW || inPin4_State == LOW)
  {
    digitalWriteFast(OUTPIN1, LOW);
    digitalWriteFast(OUTPIN2, LOW);
    digitalWriteFast(OUTPIN3, LOW);
    digitalWriteFast(OUTPIN4, LOW);
    digitalWriteFast(OUTPIN5, LOW);
    digitalWriteFast(OUTPIN6, LOW);
    digitalWriteFast(OUTPIN7, LOW);
    digitalWriteFast(OUTPIN8, LOW);
  }
}

//void zaswiec(int )

void setup() 
{
  Serial.begin(115200);
  
  // PINS
  pinModeFast(INPIN1, INPUT_PULLUP);
  pinModeFast(INPIN2, INPUT_PULLUP);
  pinModeFast(INPIN3, INPUT_PULLUP);
  pinModeFast(INPIN4, INPUT_PULLUP);

  pinModeFast(OUTPIN1, OUTPUT);
  pinModeFast(OUTPIN2, OUTPUT);
  pinModeFast(OUTPIN3, OUTPUT);
  pinModeFast(OUTPIN4, OUTPUT);
  pinModeFast(OUTPIN5, OUTPUT);
  pinModeFast(OUTPIN6, OUTPUT);
  pinModeFast(OUTPIN7, OUTPUT);
  pinModeFast(OUTPIN8, OUTPUT);
  digitalWriteFast(OUTPIN1, HIGH);
  digitalWriteFast(OUTPIN2, HIGH);
  digitalWriteFast(OUTPIN3, HIGH);
  digitalWriteFast(OUTPIN4, HIGH);
  digitalWriteFast(OUTPIN5, HIGH);
  digitalWriteFast(OUTPIN6, HIGH);
  digitalWriteFast(OUTPIN7, HIGH);
  digitalWriteFast(OUTPIN8, HIGH);

  // nRF24L01
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
  pinMode(10, OUTPUT);
}

void loop() {
  if (radio.available()) 
  {
    radio.read(&bme_data, sizeof(bme_data));
    Serial.print("Odebrano:");
    Serial.println(bme_data);
    digitalWriteFast(OUTPIN1, LOW);
  }
  else
  {
    Serial.println("nRF:nyc");
  }

  read_input_pins();

}
