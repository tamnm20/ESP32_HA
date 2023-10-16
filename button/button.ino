int PressButton= 0;
const int Button = 0;
const int Led = 2;

void setup() {
	Serial.begin(115200);
	// Khởi tạo Button pin (GPIO 0) là chân Input
	pinMode(Button, INPUT_PULLUP);
	// Khởi tạo Led pin (GPIO 2) là chân Output
	pinMode(Led, OUTPUT);
}

void loop() {
	PressButton = digitalRead(Button);
	Serial.println(PressButton);
	if (PressButton == LOW) {
	  digitalWrite(Led, HIGH);
	} else {
	  digitalWrite(Led, LOW);
	}
}