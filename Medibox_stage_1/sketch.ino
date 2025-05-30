#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>

// Define oled parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Pin definitions
#define BUZZER 5
#define LED_1 15
#define PB_CANCEL 34
#define PB_OK 32
#define PB_UP 33
#define PB_DOWN 35
#define DHTPIN 12

// NTP Server settings
#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET 19800        // Default to UTC
#define UTC_OFFSET_DST 0

// Temperature and humidity limits
#define TEMP_MIN 24.0
#define TEMP_MAX 32.0
#define HUMIDITY_MIN 65.0
#define HUMIDITY_MAX 80.0

// Declare objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

// Global variables for time
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

// Alarm settings
bool alarm_enabled = true;
const int n_alarms = 2;
int alarm_hours[n_alarms] = {0, 1};
int alarm_minutes[n_alarms] = {1, 10};
bool alarm_triggered[n_alarms] = {false, false};

// Buzzer tones
int n_notes = 8;
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int C_H = 523;
int notes[] = {C, D, E, F, G, A, B, C_H};

// Menu modes
int current_mode = 0;
const int max_modes = 4;
String modes[] = {"1 - Set Timezone", "2 - Set Alarm 1", "3 - Set Alarm 2", "4 - View/Delete Alarms"};

// WiFi credentials
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Function declarations
void print_line(String text, int column, int row, int text_size);
void update_time();
int wait_for_button_press();
void set_timezone();
void set_alarm(int alarm);
void view_delete_alarms();
void ring_alarm();
void check_environment();

void setup() {
  // Initialize hardware
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(PB_CANCEL, INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(PB_DOWN, INPUT);
  pinMode(PB_UP, INPUT);

  dhtSensor.setup(DHTPIN, DHTesp::DHT22);
  Serial.begin(9600);

  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.display();
  delay(1000);
  
  // Connect to WiFi
  display.clearDisplay();
  print_line("Connecting to WiFi", 0, 0, 1);
  WiFi.begin(ssid, password, 6);
  
  int retry_count = 0;
  while (WiFi.status() != WL_CONNECTED && retry_count < 20) {
    delay(500);
    display.print(".");
    display.display();
    retry_count++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    display.clearDisplay();
    print_line("WiFi Connected!", 0, 0, 1);
    
    // Configure time
    configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
    print_line("Time synchronized", 0, 16, 1);
  } else {
    display.clearDisplay();
    print_line("WiFi connection failed", 0, 0, 1);
    print_line("Check credentials", 0, 16, 1);
  }
  
  delay(2000);
  
  // Welcome message
  display.clearDisplay();
  print_line("Welcome", 24, 10, 2);
  print_line("to MediBox", 2, 30, 2);
  delay(2000);
  display.clearDisplay();
}

void loop() {
  // Update time and check for alarms
  update_time_with_alarm_check();
  
  // Check for menu button press
  if (digitalRead(PB_OK) == LOW) {
    delay(200);
    go_to_menu();
  }
  
  // Check temperature and humidity
  check_environment();
  
  // Small delay to prevent CPU hogging
  delay(100);
}

void print_line(String text, int column, int row, int text_size) {
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column, row);
  display.println(text);
  display.display();
}

void print_time_now() {
  display.clearDisplay();
  
  // Format time with leading zeros
  String hourStr = (hours < 10) ? "0" + String(hours) : String(hours);
  String minStr = (minutes < 10) ? "0" + String(minutes) : String(minutes);
  String secStr = (seconds < 10) ? "0" + String(seconds) : String(seconds);
  
  print_line(hourStr + ":" + minStr + ":" + secStr, 20, 5, 2);
  
}

void update_time() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeHour[3];
    strftime(timeHour, 3, "%H", &timeinfo);
    hours = atoi(timeHour);

    char timeMinute[3];
    strftime(timeMinute, 3, "%M", &timeinfo);
    minutes = atoi(timeMinute);

    char timeSecond[3];
    strftime(timeSecond, 3, "%S", &timeinfo);
    seconds = atoi(timeSecond);

    char timeDay[3];
    strftime(timeDay, 3, "%d", &timeinfo);
    days = atoi(timeDay);
  }
}

void update_time_with_alarm_check() {
  update_time();
  print_time_now();

  // Check for alarms
  if (alarm_enabled) {
    for (int i = 0; i < n_alarms; i++) {
      if (!alarm_triggered[i] && hours == alarm_hours[i] && minutes == alarm_minutes[i] && seconds < 10) {
        ring_alarm(i);
        alarm_triggered[i] = true;
      }
      
      // Reset triggered status at midnight for the next day
      if (hours == 0 && minutes == 0 && seconds < 5) {
        alarm_triggered[i] = false;
      }
    }
  }
}

int wait_for_button_press() {
  while (true) {
    if (digitalRead(PB_UP) == LOW) {
      delay(200);
      return PB_UP;
    } else if (digitalRead(PB_DOWN) == LOW) {
      delay(200);
      return PB_DOWN;
    } else if (digitalRead(PB_OK) == LOW) {
      delay(200);
      return PB_OK;
    } else if (digitalRead(PB_CANCEL) == LOW) {
      delay(200);
      return PB_CANCEL;
    }
    // Keep time updated while waiting
    update_time();
    delay(50);
  }
}

void go_to_menu() {
  bool exit_menu = false;
  
  while (!exit_menu) {
    display.clearDisplay();
    print_line(modes[current_mode], 0, 0, 1);
    print_line("UP/DOWN: Navigate", 0, 25, 1);
    print_line("OK: Select", 0, 35, 1);
    print_line("CANCEL: Exit", 0, 45, 1);

    int pressed = wait_for_button_press();

    switch (pressed) {
      case PB_UP:
        current_mode = (current_mode + 1) % max_modes;
        break;
      
      case PB_DOWN:
        current_mode = (current_mode - 1 + max_modes) % max_modes;
        break;
      
      case PB_OK:
        // Execute the selected mode
        switch (current_mode) {
          case 0:
            set_timezone();
            break;
          case 1:
            set_alarm(0);
            break;
          case 2:
            set_alarm(1);
            break;
          case 3:
            view_delete_alarms();
            break;
        }
        break;
      
      case PB_CANCEL:
        exit_menu = true;
        break;
    }
  }
  
  display.clearDisplay();
}

void set_timezone() {
  int hour_offset = UTC_OFFSET / 3600;
  int minute_offset = (UTC_OFFSET % 3600) / 60;
  bool setting_hours = true;
  
  while (true) {
    display.clearDisplay();
    
    if (setting_hours) {
      print_line("UTC Hour Offset:", 0, 0, 1);
      print_line(String(hour_offset), 55, 16, 2);
      print_line("UP/DOWN to change", 0, 40, 1);
      print_line("OK for minutes", 0, 50, 1);
    } else {
      print_line("UTC Minute Offset:", 0, 0, 1);
      print_line(String(minute_offset), 55, 16, 2);
      print_line("UP/DOWN to change", 0, 40, 1);
      print_line("OK to save", 0, 50, 1);
    }
    
    int pressed = wait_for_button_press();
    
    if (pressed == PB_UP) {
      if (setting_hours) {
        hour_offset = (hour_offset + 1 > 14) ? -12 : hour_offset + 1;
      } else {
        minute_offset = (minute_offset + 15 >= 60) ? 0 : minute_offset + 15;
      }
    } else if (pressed == PB_DOWN) {
      if (setting_hours) {
        hour_offset = (hour_offset - 1 < -12) ? 14 : hour_offset - 1;
      } else {
        minute_offset = (minute_offset - 15 < 0) ? 45 : minute_offset - 15;
      }
    } else if (pressed == PB_OK) {
      if (setting_hours) {
        setting_hours = false;
      } else {
        int total_offset = (hour_offset * 3600) + (minute_offset * 60);
        configTime(total_offset, UTC_OFFSET_DST, NTP_SERVER);
        
        display.clearDisplay();
        String sign = (hour_offset >= 0) ? "+" : "";
        String min_str = (minute_offset < 10) ? "0" + String(minute_offset) : String(minute_offset);
        String timezone = "UTC" + sign + String(hour_offset) + ":" + min_str;
        
        print_line("Timezone set to:", 0, 0, 1);
        print_line(timezone, 10, 16, 2);
        print_line("Time updated", 0, 40, 1);
        
        delay(2000);
        break;
      }
    } else if (pressed == PB_CANCEL) {
      if (!setting_hours) {
        setting_hours = true;
      } else {
        break;
      }
    }
  }
}

void set_alarm(int alarm_index) {
  int temp_hour = alarm_hours[alarm_index];
  
  // Set hour
  while (true) {
    display.clearDisplay();
    print_line("Alarm " + String(alarm_index + 1) + " Hour:", 0, 0, 1);
    
    // Format with leading zero
    String hourStr = (temp_hour < 10) ? "0" + String(temp_hour) : String(temp_hour);
    print_line(hourStr, 55, 16, 2);
    
    print_line("UP/DOWN to change", 0, 40, 1);
    print_line("OK to confirm", 0, 50, 1);

    int pressed = wait_for_button_press();
    
    if (pressed == PB_UP) {
      temp_hour = (temp_hour + 1) % 24;
    } else if (pressed == PB_DOWN) {
      temp_hour = (temp_hour - 1 + 24) % 24;
    } else if (pressed == PB_OK) {
      alarm_hours[alarm_index] = temp_hour;
      break;
    } else if (pressed == PB_CANCEL) {
      return;
    }
  }

  // Set minute - increment by 5 minutes
  int temp_minute = alarm_minutes[alarm_index];
  temp_minute = (temp_minute / 5) * 5;
  
  while (true) {
    display.clearDisplay();
    print_line("Alarm " + String(alarm_index + 1) + " Minute:", 0, 0, 1);
    
    // Format with leading zero
    String minStr = (temp_minute < 10) ? "0" + String(temp_minute) : String(temp_minute);
    print_line(minStr, 55, 16, 2);
    
    print_line("UP/DOWN to change", 0, 40, 1);
    print_line("OK to save", 0, 50, 1);

    int pressed = wait_for_button_press();
    
    if (pressed == PB_UP) {
      // Increment by 5 minutes
      temp_minute = (temp_minute + 5) % 60;
    } else if (pressed == PB_DOWN) {
      // Decrement by 5 minutes with wraparound
      temp_minute = (temp_minute - 5 + 60) % 60;
    } else if (pressed == PB_OK) {
      alarm_minutes[alarm_index] = temp_minute;
      
      // Reset trigger status when alarm is changed
      alarm_triggered[alarm_index] = false;
      
      // Enable alarms if they were disabled
      alarm_enabled = true;
      
      display.clearDisplay();
      print_line("Alarm " + String(alarm_index + 1) + " set for:", 0, 0, 1);
      
      String timeStr = (alarm_hours[alarm_index] < 10 ? "0" : "") + 
                       String(alarm_hours[alarm_index]) + ":" + 
                       (alarm_minutes[alarm_index] < 10 ? "0" : "") + 
                       String(alarm_minutes[alarm_index]);
      
      print_line(timeStr, 20, 20, 2);
      delay(2000);
      break;
    } else if (pressed == PB_CANCEL) {
      return;
    }
  }
}

void view_delete_alarms() {
  int selected_alarm = 0;
  bool delete_mode = false;
  
  while (true) {
    display.clearDisplay();
    
    if (!delete_mode) {
      print_line("Active Alarms:", 0, 0, 1);
      
      for (int i = 0; i < n_alarms; i++) {
        String prefix = (i == selected_alarm) ? "> " : "  ";
        String alarm_text = prefix + "Alarm " + String(i + 1) + ": ";
        
        // Format with leading zeros
        alarm_text += (alarm_hours[i] < 10 ? "0" : "") + String(alarm_hours[i]) + ":";
        alarm_text += (alarm_minutes[i] < 10 ? "0" : "") + String(alarm_minutes[i]);
        
        print_line(alarm_text, 0, 16 + i * 12, 1);
      }
      
      print_line("UP/DOWN: Select", 0, 42, 1);
      print_line("OK: Delete mode", 0, 52, 1);
    } else {
      print_line("Delete Alarm " + String(selected_alarm + 1) + "?", 0, 0, 1);
      
      String alarm_text = (alarm_hours[selected_alarm] < 10 ? "0" : "") + 
                         String(alarm_hours[selected_alarm]) + ":" + 
                         (alarm_minutes[selected_alarm] < 10 ? "0" : "") + 
                         String(alarm_minutes[selected_alarm]);
      
      print_line(alarm_text, 40, 16, 2);
      
      print_line("OK: Confirm Delete", 0, 40, 1);
      print_line("CANCEL: Back", 0, 50, 1);
    }
    
    int pressed = wait_for_button_press();
    
    if (!delete_mode) {
      if (pressed == PB_UP || pressed == PB_DOWN) {
        selected_alarm = (selected_alarm + 1) % n_alarms;
      } else if (pressed == PB_OK) {
        delete_mode = true;
      } else if (pressed == PB_CANCEL) {
        break;
      }
    } else {
      if (pressed == PB_OK) {
        // Reset the alarm
        alarm_hours[selected_alarm] = 0;
        alarm_minutes[selected_alarm] = 0;
        alarm_triggered[selected_alarm] = false;
        
        display.clearDisplay();
        print_line("Alarm " + String(selected_alarm + 1), 0, 0, 1);
        print_line("Deleted", 30, 20, 2);
        delay(1500);
        
        delete_mode = false;
      } else if (pressed == PB_CANCEL) {
        delete_mode = false;
      }
    }
  }
}

void ring_alarm(int alarm_index) {
  bool alarm_active = true;
  unsigned long start_time = millis();
  unsigned long led_time = 0;
  bool led_state = false;
  
  // Show alarm message
  display.clearDisplay();
  print_line("MEDICINE", 20, 0, 2);
  print_line("TIME!", 40, 20, 2);
  print_line("OK: Snooze 5min", 0, 40, 1);
  print_line("CANCEL: Stop", 0, 50, 1);
  
  while (alarm_active) {
    // Blink LED fast for alarm
    if (millis() - led_time > 200) {
      led_state = !led_state;
      digitalWrite(LED_1, led_state);
      led_time = millis();
    }
    
    // Play alarm melody
    for (int i = 0; i < n_notes && alarm_active; i++) {
      tone(BUZZER, notes[i]);
      
      // Check buttons while tone is playing
      unsigned long note_start = millis();
      while (millis() - note_start < 300 && alarm_active) {
        if (digitalRead(PB_CANCEL) == LOW) {
          delay(200);
          alarm_active = false;
          break;
        } else if (digitalRead(PB_OK) == LOW) {
          delay(200);
          
          // Snooze for 5 minutes
          display.clearDisplay();
          print_line("Snoozing for", 0, 0, 2);
          print_line("5 minutes", 20, 20, 2);
          
          // Reset the current alarm
          alarm_triggered[alarm_index] = false;
          
          // Set new alarm time (5 minutes later)
          int new_minutes = (minutes + 5) % 60;
          int new_hours = hours;
          if (minutes + 5 >= 60) {
            new_hours = (hours + 1) % 24;
          }
          
          alarm_minutes[alarm_index] = new_minutes;
          alarm_hours[alarm_index] = new_hours;
          
          delay(1500);
          alarm_active = false;
          break;
        }
        delay(10);
      }
      
      noTone(BUZZER);
      delay(50);
    }
    
    // Timeout after 10 minutes if no button pressed
    if (millis() - start_time > 600000) {
      alarm_active = false;
    }
  }
  
  // Turn off LED and buzzer
  digitalWrite(LED_1, LOW);
  noTone(BUZZER);
  display.clearDisplay();
}

void check_environment() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  bool warning = false;
  
  // Only show environmental warnings, not regular readings
  if (data.temperature > TEMP_MAX || data.temperature < TEMP_MIN || 
      data.humidity > HUMIDITY_MAX || data.humidity < HUMIDITY_MIN) {
    
    // Clear bottom part of display for warnings
    display.fillRect(0, 45, SCREEN_WIDTH, 19, SSD1306_BLACK);
    display.setTextSize(1);
    
    // Temperature warning if needed
    if (data.temperature > TEMP_MAX) {
      display.setCursor(0, 45);
      display.print("TEMP HIGH: " + String(data.temperature, 1) + "C");
      warning = true;
    } else if (data.temperature < TEMP_MIN) {
      display.setCursor(0, 45);
      display.print("TEMP LOW: " + String(data.temperature, 1) + "C");
      warning = true;
    }
    
    // Humidity warning if needed
    if (data.humidity > HUMIDITY_MAX) {
      display.setCursor(0, 55);
      display.print("HUM HIGH: " + String(data.humidity, 1) + "%");
      warning = true;
    } else if (data.humidity < HUMIDITY_MIN) {
      display.setCursor(0, 55);
      display.print("HUM LOW: " + String(data.humidity, 1) + "%");
      warning = true;
    }
    
    display.display();
  }
  
  // If there's a warning, blink LED
  if (warning) {
    static unsigned long last_blink = 0;
    if (millis() - last_blink > 500) { // Slow blink for environment warnings
      last_blink = millis();
      digitalWrite(LED_1, !digitalRead(LED_1));
    }
  }
}