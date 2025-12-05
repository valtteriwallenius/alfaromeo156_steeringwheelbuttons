#define DEBUG  // Comment this line to disable debug prints

// Output pins

const int RADIO_REMOTE_A1 = 5; // = Volume down
const int RADIO_REMOTE_A2 = 4; // = Volume up
const int RADIO_REMOTE_A3 = 3; // = Mute

const int RADIO_REMOTE_B1 = 8; // = Channel down
const int RADIO_REMOTE_B2 = 7; // = Channel up
const int RADIO_REMOTE_B3 = 6; // = Source

const int BLUETOOTH_FORWARD = 9;
const int BLUETOOTH_PAUSE   = 10;
const int BLUETOOTH_REVERSE = 11;

// Input pins

const int BUTTON_RADIO_REMOTE_A = A1;
const int BUTTON_RADIO_REMOTE_B = A2;
const int BUTTON_PADDLE_SHIFTER = A0;

// Expected ADC values for the buttons:

const int BUTTON1_ADC = 64;
const int BUTTON2_ADC = 147;
const int BUTTON3_ADC = 286;

// Expected ADC values for the paddles:

const int PADDLE_ADC_PLUS  = 254;
const int PADDLE_ADC_MINUS = 512;
const int PADDLE_ADC_NONE  = 780;

// Timing:

const unsigned long DEBOUNCE_DELAY = 20; // debounce delay
const unsigned long BLUETOOTH_PRESS_TIME = 100; // min time to press bluetooth buttons



#ifdef DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif



// Input analog values
int button_radio_remote_a_value = 0;
int button_radio_remote_b_value = 0;
int button_paddle_shifter_value = 0;

// Button readings
int radio_a_reading = 0;
int radio_b_reading = 0;
int paddle_reading = 0;

// Button states
int radio_a_button = 0;
int radio_b_button = 0;
int paddle_button = 0;

// Previous states for debounce
int previous_radio_a_button = 0;
int previous_radio_b_button = 0;
int previous_paddle_button  = 0;

// Debounce timing
unsigned long last_debounce_time_radio_a = 0;
unsigned long last_debounce_time_radio_b = 0;
unsigned long last_debounce_time_paddle  = 0;

// Button timing
unsigned long bluetooth_last_pressed = 0;

// Output states
int radio_remote_a1_state = LOW;
int radio_remote_a2_state = LOW;
int radio_remote_a3_state = LOW;

int radio_remote_b1_state = LOW;
int radio_remote_b2_state = LOW;
int radio_remote_b3_state = LOW;

int bluetooth_forward_state = LOW;
int bluetooth_pause_state   = LOW;
int bluetooth_reverse_state = LOW;



// Dynamically calculated thresholds
const int THRESHOLD_BUTTON_1   = (BUTTON1_ADC + 0) / 2;
const int THRESHOLD_BUTTON_1_2 = (BUTTON1_ADC + BUTTON2_ADC) / 2;
const int THRESHOLD_BUTTON_2_3 = (BUTTON2_ADC + BUTTON3_ADC) / 2;
const int THRESHOLD_BUTTON_3   = (BUTTON3_ADC + 1023) / 2;

const int THRESHOLD_PADDLE_PLUS       = (PADDLE_ADC_PLUS + 0) / 2;
const int THRESHOLD_PADDLE_PLUS_MINUS = (PADDLE_ADC_PLUS + PADDLE_ADC_MINUS) / 2;
const int THRESHOLD_PADDLE_MINUS_NONE = (PADDLE_ADC_MINUS + PADDLE_ADC_NONE) / 2;

// Function to detect button based on analog value
int detectButton(int adc_value) {
  if (adc_value >= THRESHOLD_BUTTON_3) return 0;        // No button pressed
  else if (adc_value < THRESHOLD_BUTTON_1) return 0;    // The wire is shorted to ground
  else if (adc_value < THRESHOLD_BUTTON_1_2) return 1;  // Closest to button 1
  else if (adc_value < THRESHOLD_BUTTON_2_3) return 2;  // Between button 1 and 3
  else return 3;                                        // Closest to button 3
}

// Paddle detection function
int detectPaddle(int adc_value) {
  if (adc_value < THRESHOLD_PADDLE_PLUS) return 0;             // The wire is shorted to ground
  else if (adc_value < THRESHOLD_PADDLE_PLUS_MINUS) return 1;  // Plus
  else if (adc_value < THRESHOLD_PADDLE_MINUS_NONE) return 2;  // Minus
  else return 0;                                               // No press
}



void setup() {

  // Begin serial if debug is enabled
#ifdef DEBUG
  Serial.begin(115200);
#endif

  // Output pins
  pinMode(RADIO_REMOTE_A1, OUTPUT);
  pinMode(RADIO_REMOTE_A2, OUTPUT);
  pinMode(RADIO_REMOTE_A3, OUTPUT);

  pinMode(RADIO_REMOTE_B1, OUTPUT);
  pinMode(RADIO_REMOTE_B2, OUTPUT);
  pinMode(RADIO_REMOTE_B3, OUTPUT);

  pinMode(BLUETOOTH_FORWARD, OUTPUT);
  pinMode(BLUETOOTH_PAUSE, OUTPUT);
  pinMode(BLUETOOTH_REVERSE, OUTPUT);

  // Input pins
  pinMode(BUTTON_RADIO_REMOTE_A, INPUT);
  pinMode(BUTTON_RADIO_REMOTE_B, INPUT);
  pinMode(BUTTON_PADDLE_SHIFTER, INPUT);

}

void loop() {

  // Read inputs
  button_radio_remote_a_value = analogRead(BUTTON_RADIO_REMOTE_A);
  button_radio_remote_b_value = analogRead(BUTTON_RADIO_REMOTE_B);
  button_paddle_shifter_value = analogRead(BUTTON_PADDLE_SHIFTER);

  // Detect button presses
  radio_a_reading = detectButton(button_radio_remote_a_value);
  radio_b_reading = detectButton(button_radio_remote_b_value);
  paddle_reading  = detectPaddle(button_paddle_shifter_value);

  // --- Radio A ---
  if (radio_a_reading != previous_radio_a_button) {
    // Reset debounce timer if value has changed
    last_debounce_time_radio_a = millis();
  }
  if (millis() - last_debounce_time_radio_a > DEBOUNCE_DELAY && radio_a_reading != radio_a_button) {
    radio_a_button = radio_a_reading;

    // If the debounce delay has been passed, change the value
    DEBUG_PRINT("Radio A button changed to: ");
    DEBUG_PRINT(radio_a_button);
    DEBUG_PRINT(" with analog value: ");
    DEBUG_PRINTLN(button_radio_remote_a_value);

    switch (radio_a_button) {
      case 1: // Volume up
        radio_remote_a1_state = HIGH;
        radio_remote_a2_state = LOW;
        radio_remote_a3_state = LOW;
        break;
      case 2: // Volume down
        radio_remote_a1_state = LOW;
        radio_remote_a2_state = HIGH;
        radio_remote_a3_state = LOW;
        break;
      case 3: // Mute
        radio_remote_a1_state = LOW;
        radio_remote_a2_state = LOW;
        radio_remote_a3_state = HIGH;
        break;
      case 0: // Released
        radio_remote_a1_state = LOW;
        radio_remote_a2_state = LOW;
        radio_remote_a3_state = LOW;
        break;
    }
  }
  // Set the last button state for next cycle
  previous_radio_a_button = radio_a_reading;

  // --- Radio B ---
  // Works same as above
  if (radio_b_reading != previous_radio_b_button) {
    last_debounce_time_radio_b = millis();
  }
  if (millis() - last_debounce_time_radio_b > DEBOUNCE_DELAY && radio_b_reading != radio_b_button) {
    radio_b_button = radio_b_reading;

    DEBUG_PRINT("Radio B button changed to: ");
    DEBUG_PRINT(radio_b_button);
    DEBUG_PRINT(" with analog value: ");
    DEBUG_PRINTLN(button_radio_remote_b_value);

    switch (radio_b_button) {
      case 1: // Channel up
        radio_remote_b1_state = HIGH;
        radio_remote_b2_state = LOW;
        bluetooth_pause_state = LOW;
        break;
      case 2: // Channel down
        radio_remote_b1_state = LOW;
        radio_remote_b2_state = HIGH;
        bluetooth_pause_state = LOW;
        break;
      case 3: // Source
        radio_remote_b1_state = LOW;
        radio_remote_b2_state = LOW;
        bluetooth_pause_state = HIGH;
        bluetooth_last_pressed = millis();
        break;
      case 0: // Released
        radio_remote_b1_state = LOW;
        radio_remote_b2_state = LOW;
        bluetooth_pause_state = LOW;
        break;
    }
  }
  previous_radio_b_button = radio_b_reading;

  // --- Paddle Shifter ---
  // Works same as above
  if (paddle_reading != previous_paddle_button) {
    last_debounce_time_paddle = millis();
  }
  if (millis() - last_debounce_time_paddle > DEBOUNCE_DELAY && paddle_reading != paddle_button) {
    paddle_button = paddle_reading;

    DEBUG_PRINT("Paddle  button changed to: ");
    DEBUG_PRINT(paddle_button);
    DEBUG_PRINT(" with analog value: ");
    DEBUG_PRINTLN(button_paddle_shifter_value);

    switch (paddle_button) {
      case 1: // Paddle +
        bluetooth_forward_state = HIGH;
        bluetooth_reverse_state = LOW;
        bluetooth_last_pressed = millis();
        break;
      case 2: // Paddle -
        bluetooth_forward_state = LOW;
        bluetooth_reverse_state = HIGH;
        bluetooth_last_pressed = millis();
        break;
      case 0: // Released
        bluetooth_forward_state = LOW;
        bluetooth_reverse_state = LOW;
        break;
    }
  }
  previous_paddle_button = paddle_reading;

  // Write output values:
  digitalWrite(RADIO_REMOTE_A1, radio_remote_a1_state);
  digitalWrite(RADIO_REMOTE_A2, radio_remote_a2_state);
  digitalWrite(RADIO_REMOTE_A3, radio_remote_a3_state);

  digitalWrite(RADIO_REMOTE_B1, radio_remote_b1_state);
  digitalWrite(RADIO_REMOTE_B2, radio_remote_b2_state);
  digitalWrite(RADIO_REMOTE_B3, radio_remote_b3_state);

  // Extend bluetooth box presses
  if (millis() > bluetooth_last_pressed + BLUETOOTH_PRESS_TIME && 
  (bluetooth_reverse_state == LOW || bluetooth_pause_state == LOW || bluetooth_forward_state == LOW) || 
  (bluetooth_reverse_state == HIGH || bluetooth_pause_state == HIGH || bluetooth_forward_state == HIGH)) {
    digitalWrite(BLUETOOTH_FORWARD, bluetooth_forward_state);
    digitalWrite(BLUETOOTH_PAUSE, bluetooth_pause_state);
    digitalWrite(BLUETOOTH_REVERSE, bluetooth_reverse_state);
  }

}
