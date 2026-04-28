//Rename\HID-Project\src\MultiReport\to( ImprovedKeyboard.cpp.bak)
#include <HID-Project.h>
// ------------------- Pin setup -------------------
  const int dataPin1 = 14;      // 74HC595 #1 DS
  const int latchPin1 = 15;     // 74HC595 #1 ST_CP
  const int dataPin2 = 10;      // 74HC595 #2 DS
  const int latchPin2 = 16;     // 74HC595 #2 ST_CP
  const int clockPin  = A0;     // shared SH_CP

  const int extraColPins[2] = {0, 1};   // Columns 17, 18
  const int rowPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};

  const int capsLED   = A2;
  const int numLED    = A3;
  const int scrollLED = A1;

  const int NUM_SHIFT_COLS = 16;
  const int NUM_COLS = NUM_SHIFT_COLS + 2;

// ------------------- State -------------------
  bool keyState[8][NUM_COLS] = {false};
  bool lastKeyState[8][NUM_COLS] = {false};

  bool capsLockOn = false;
  bool numLockOn  = false;
  bool scrollLockOn = false;

// Repeat timing for media keys
  unsigned long lastRepeatTime = 0;
  const unsigned long REPEAT_DELAY = 400;    // Initial delay before repeat starts (ms)
  const unsigned long REPEAT_RATE = 50;     // Repeat interval (ms)
  bool repeatActive = false;
  uint8_t repeatKeycode = 0;
  bool repeatIsConsumer = false;

// ------------------- KeyMap -------------------
  struct KeyMap {
  uint8_t row;
  uint8_t col;
  uint8_t keycode;      // HID keycode or ASCII----------
  bool isModifier;      // true if shift/ctrl/alt/GUI
  bool isLock;          // true if Caps/Num/Scroll Lock
  bool isConsumer;
  };
// Full key mapping from your list
  KeyMap keyMappings[] = {
  // Function keys
  {7, 1, KEY_ESC},
  {6, 1, KEY_F1},
  {7, 2, KEY_F2},
  {6, 2, KEY_F3},
  {1, 0, KEY_F4},
  {7, 4, KEY_F5},
  {7, 15, KEY_F6},
  {7, 12, KEY_F7},
  {6, 10, KEY_F8},
  {0, 10, KEY_F9},
  {0, 9, KEY_F10},
  {1, 10, KEY_F11},
  {5, 10, KEY_F12},

  // Misc keys
  {7,17, KEY_PRINT_SCREEN},
  {6,17, KEY_SCROLL_LOCK},
  {1,8, KEY_PAUSE},
  {4,5, KEY_INSERT},
  {7,6, KEY_HOME},
  {4,11, KEY_PAGE_UP},
  {5,5, KEY_DELETE},
  {5,11, KEY_END},
  {3,11, KEY_PAGE_DOWN},
  {7,16, KEY_UP_ARROW},
  {0,12, KEY_LEFT_ARROW},
  {0,11, KEY_DOWN_ARROW},
  {6,12, KEY_RIGHT_ARROW},

  // Main keys
  {0,0,'`'},
  {5,1,'1'},
  {5,2,'2'},
  {4,2,'3'},
  {5,17,'4'},
  {4,17,'5'},
  {6,15,'6'},
  {5,15,'7'},
  {5,6,'8'},
  {4,6,'9'},
  {5,4,'0'},
  {5,3,'-'},
  {4,3,'='},
  {3,5,'\\'},
  {6,5,KEY_BACKSPACE},
  {6,3,KEY_TAB},
  {4,1,'q'},
  {3,2,'w'},
  {2,2,'e'},
  {3,17,'r'},
  {4,15,'t'},
  {3,15,'y'},
  {3,6,'u'},
  {2,6,'i'},
  {4,4,'o'},
  {3,3,'p'},
  {2,3,'['},
  {1,5,']'},
  {2,5,KEY_RETURN},
  {5,8,KEY_CAPS_LOCK,0,true},
  {3,1,'a'},
  {2,1,'s'},
  {1,2,'d'},
  {2,17,'f'},
  {2,15,'g'},
  {1,15,'h'},
  {1,6,'j'},
  {3,4,'k'},
  {0,4,'l'},
  {1,3,';'},
  {0,5,'\''},
  {1,1,'z'},
  {0,1,'x'},
  {0,2,'c'},
  {1,17,'v'},
  {0,17,'b'},
  {0,15,'n'},
  {0,6,'m'},
  {2,4,','},
  {1,4,'.'},
  {0,3,'/'},

  // Modifiers
  {6,7, KEY_LEFT_SHIFT,true},
  {2,7, KEY_RIGHT_SHIFT,true},
  {6,8, KEY_LEFT_CTRL,true},
  {2,8, KEY_RIGHT_CTRL,true},
  {7,0, KEY_LEFT_GUI,true},
  {5,16, KEY_LEFT_GUI,true},
  {0,16,' '},
  {4,9, KEY_LEFT_ALT,true},
  {3,9, KEY_RIGHT_ALT,true},
  {6,13, KEY_MENU,true}, 

  // Numpad keys
  {2,11,KEY_NUM_LOCK,0,true},
  {7,5,220},
  {7,10,221},
  {4,13,222},
  {6,4,231}, // home/num logic
  {5,12,232},
  {6,11,233},
  {2,10,223},
  {4,12,228},
  {3,12,229},
  {4,10,230},
  {2,12,225},
  {1,11,226},
  {3,10,227},
  {0,13,224},
  {1,16,234},
  {1,12,235},

  //Special keys
  {5,14,MEDIA_VOLUME_DOWN,0,false,true}, // vol-
  {6,14,MEDIA_VOLUME_UP,0,false,true}, // vol+
  {7,14,MEDIA_VOL_MUTE,0,false,true}, // mute
  {7,13,HID_CONSUMER_EJECT,0,false,true}, // eject
  {3,8,MEDIA_PLAY_PAUSE,0,false,true}, // play/pause
  {3,13,MEDIA_STOP,0,false,true}, // stop
  {4,8,MEDIA_PREVIOUS,0,false,true}, // previous
  {4,14,MEDIA_NEXT,0,false,true}, // next

  {2,0,KEY_STOP,0,false,true}, // i
  {4,7,HID_CONSUMER_AC_LOCK,0,false,true}, // email
  {5,13,HID_CONSUMER_SLEEP,0,false,true}, // sleep

  {7,11,HID_CONSUMER_AC_SEARCH,0,false,true}, // help
  {1,13,CONSUMER_EXPLORER,0,false,true}, // searchbar
  {2,13,HID_CONSUMER_AC_PASTE,0,false,true}, // paste to folder
  {1,7,CONSUMER_BROWSER_BOOKMARKS,0,false,true}, // nextpage
  
  {1,14,CONSUMER_BRIGHTNESS_DOWN,0,false,true}, // pg1
  {2,14,CONSUMER_BRIGHTNESS_UP,0,false,true}, // pg2
  {3,14,HID_CONSUMER_AL_LOGOFF,0,false,true}, // pg3
  {7,8,HID_CONSUMER_AC_ZOOM_IN,0,false,true}, // pg4
  {3,0,HID_CONSUMER_AC_ZOOM_OUT,0,false,true}  // pg5
  };
// ------------------- Column activation -------------------
void setColumn(int col) {
  uint8_t reg1 = 0xFF;
  uint8_t reg2 = 0xFF;
  digitalWrite(extraColPins[0], HIGH);
  digitalWrite(extraColPins[1], HIGH);

  if (col >=0 && col<8) reg1 &= ~(1<<col);
  else if(col>=8 && col<16) reg2 &= ~(1<<(col-8));
  else if(col==16) digitalWrite(extraColPins[0], LOW);
  else if(col==17) digitalWrite(extraColPins[1], LOW);

  digitalWrite(latchPin1, LOW);
  shiftOut(dataPin1, clockPin, LSBFIRST, reg1);
  digitalWrite(latchPin1, HIGH);

  digitalWrite(latchPin2, LOW);
  shiftOut(dataPin2, clockPin, LSBFIRST, reg2);
  digitalWrite(latchPin2, HIGH);
}
// ------------------- Key handling -------------------
void handleMappedKeyDown(const KeyMap &km){
    if(km.isConsumer) {
      Consumer.write(km.keycode);
      // Start repeat timer for consumer keys
      repeatActive = true;
      repeatKeycode = km.keycode;
      repeatIsConsumer = true;
      lastRepeatTime = millis();
    }
    else Keyboard.press(km.keycode);
}
    
void handleMappedKeyUp(const KeyMap &km){
    if(km.isConsumer) {
      Consumer.release(km.keycode);
      // Stop repeat when key is released
      if(repeatKeycode == km.keycode) repeatActive = false;
    }
    else Keyboard.release(km.keycode);
}
// ------------------- Setup -------------------
void setup() {
  Keyboard.begin();

  pinMode(dataPin1, OUTPUT);
  pinMode(latchPin1, OUTPUT);
  pinMode(dataPin2, OUTPUT);
  pinMode(latchPin2, OUTPUT);
  pinMode(clockPin, OUTPUT);

  for(int r=0;r<8;r++) pinMode(rowPins[r], INPUT_PULLUP);
  for(int c=0;c<2;c++) pinMode(extraColPins[c], OUTPUT);
  digitalWrite(extraColPins[0], HIGH);
  digitalWrite(extraColPins[1], HIGH);

  pinMode(capsLED, OUTPUT);
  pinMode(numLED, OUTPUT);
  pinMode(scrollLED, OUTPUT);

  digitalWrite(capsLED, LOW);
  digitalWrite(numLED, LOW);
  digitalWrite(scrollLED, LOW);
  

  setColumn(-1);
}
// ------------------- Loop -------------------
void loop() {
  uint8_t leds = BootKeyboard.getLeds();
  // Update LEDs using a single pass
  digitalWrite(capsLED,   (leds & LED_CAPS_LOCK)   ? LOW : HIGH);
  digitalWrite(numLED,    (leds & LED_NUM_LOCK)    ? LOW : HIGH);
  digitalWrite(scrollLED, (leds & LED_SCROLL_LOCK) ? LOW : HIGH);

  for(int c=0;c<NUM_COLS;c++){  //scan each column
    setColumn(c);

  delayMicroseconds(50);

  for(int r=0;r<8;r++){   // scan each row in the active column
    bool pressed = (digitalRead(rowPins[r])==LOW);
    keyState[r][c]=pressed;

      for(auto &km: keyMappings){
        if(km.row==r && km.col==c){
          if(pressed && !lastKeyState[r][c]) handleMappedKeyDown(km);
          else if(!pressed && lastKeyState[r][c]) handleMappedKeyUp(km);
        }
      }
      lastKeyState[r][c]=pressed;
    }
  }
  setColumn(-1);
  // Handle key repeat for consumer keys
  if(repeatActive) {
    unsigned long now = millis();
    unsigned long elapsed = now - lastRepeatTime;
    
    // Initial delay, then repeat at regular intervals
    if(elapsed >= REPEAT_DELAY) {
      Consumer.write(repeatKeycode);
      lastRepeatTime = now - (elapsed - REPEAT_RATE); // Adjust for consistent rate
    }
  }
}