
/* Reach out to almonds@mit.edu or jodalyst@mit.edu for help */

#include "Six302.h"

/* PUBLIC ROUTINES */

/* Initialization */

CommManager::CommManager(uint32_t sp, uint32_t rp) {
   _step_period = sp;
   _report_period = rp;
#ifdef S302_WEBSOCKETS
   _wss = WebSocketsServer(S302_PORT);
#endif
   strcpy(_build_string, "\fB");
   _debug_string[0] = '\0';
}

/* :: connect( &Serial, baud ) 
             ( "ssid", "p/w" ) */

#ifdef S302_SERIAL
#ifdef TEENSYDUINO
void CommManager::connect(usb_serial_class* s, uint32_t baud)
#else
void CommManager::connect(HardwareSerial* s, uint32_t baud)
#endif
{
   _serial = s;
   _baud = baud;
   _serial->begin(baud);
   while(!_serial);
   // Serial tends to fill with garbage for the first half-second
   delay(500);
   while( _serial->available() )
      _serial->read();
#elif defined S302_WEBSOCKETS
void CommManager::connect(const char* ssid, const char* pw) {
   // Serial should be ready to go
   Serial.printf("Connecting to %s WiFi ", ssid);
   
   // Fix for open networks - ensure pw is not NULL
   if (pw == NULL) {
      // Use empty string for open networks
      WiFi.begin(ssid, "");
   } else {
      WiFi.begin(ssid, pw);
   }
   
   WiFi.setTxPower(WIFI_POWER_8_5dBm);
   
   // Add timeout to prevent infinite loop
   int timeout = 0;
   while (WiFi.status() != WL_CONNECTED && timeout < 30) {
      Serial.print('.');
      delay(1000);
      timeout++;
   }
   
   // Check if connected successfully
   if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\nFailed to connect to WiFi!");
      return; // Early return if WiFi connection fails
   }
   
   Serial.println(" connected!");
   // Print my IP!
   IPAddress ip = WiFi.localIP();
   Serial.printf("--> %d.%d.%d.%d:%d <--\n", ip[0], ip[1], ip[2], ip[3], S302_PORT);
   
#ifdef ESP32
   // Create semaphore BEFORE setting up WebSocket server
   _baton = xSemaphoreCreateMutex();
   if (_baton == NULL) {
      Serial.println("Failed to create semaphore!");
   }
#endif

   // Initialize timers early
#ifdef TEENSYDUINO
   _main_timer = 0;
#else
   _main_timer = micros();
#endif
#ifdef ESP32
   _secondary_timer = micros();
#endif

   // Start the WebSocket server - MOVED AFTER semaphore and timer initialization
   _wss.begin();

   // Fix: Use a lambda expression instead of std::bind to avoid potential memory issues
   _wss.onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
      this->_on_websocket_event(num, type, payload, length);
   });

   // Mark manager as ready
   _ready = true;
}

/* :: pinToCore( coreID ) */

#if defined ESP32
void CommManager::pinToCore(uint8_t xCoreID) {
   if( !_ready ) return;
   xTaskCreatePinnedToCore( _walk,
                            "6302view",
                            4096, /* Stack size, in bytes, can probably be decreased */
                            this,
                            1,
                            &_six302_task,
                            xCoreID );
   //vTaskDelay(50);
}
#endif

/* To add CONTROLS */

/* :: addToggle( link, title ) */

bool CommManager::addToggle(bool* linker, const char* title) {
   if( _total_controls + 1 > MAX_CONTROLS )
      return false;
      
   _controls[_total_controls++] = (float*)linker;
#ifdef S302_UNO
   strcpy(_buf, "T\r");
   strncat(_buf, title, MAX_TITLE_LEN);
   strcat(_buf, "\r");
#else
   sprintf(_buf, "T\r%.*s\r", MAX_TITLE_LEN, title);
#endif
   strcat(_build_string, _buf);
   
   return true;
}

/* :: addButton( link, title ) */

bool CommManager::addButton(bool* linker, const char* title) {
   if( _total_controls + 1 > MAX_CONTROLS )
      return false;
      
   _controls[_total_controls++] = (float*)linker;
#ifdef S302_UNO
   strcpy(_buf, "B\r");
   strncat(_buf, title, MAX_TITLE_LEN);
   strcat(_buf, "\r");
#else
   sprintf(_buf, "B\r%.*s\r", MAX_TITLE_LEN, title);
#endif
   strcat(_build_string, _buf);

   return true;
}

/* :: addSlider( link, title, range, resoultion ) */

bool CommManager::addSlider(float* linker, const char* title,
                            float range_min, float range_max,
                            float resolution, bool toggle) {
   if( _total_controls + 1 > MAX_CONTROLS )
      return false;

   _controls[_total_controls] = linker;
   _ctrl_types[_total_controls++] = true; // float
#ifdef S302_UNO
   strcpy(_buf, "S\r");
   strncat(_buf, title, MAX_TITLE_LEN);
   strcat(_buf, "\r");
   dtostrf(range_min, 0, MAX_PREC, _tmp);
   strcat(_buf, _tmp);
   strcat(_buf, "\r");
   dtostrf(range_max, 0, MAX_PREC, _tmp);
   strcat(_buf, _tmp);
   strcat(_buf, "\r");
   dtostrf(resolution, 0, MAX_PREC, _tmp);
   strcat(_buf, _tmp);
   strcat(_buf, "\r");
   strcat(_buf, toggle? "True":"False");
   strcat(_buf, "\r");
#else
   sprintf(_buf, "S\r%.*s\r%f\r%f\r%f\r%s\r",
      MAX_TITLE_LEN, title, range_min, range_max,
      resolution, toggle? "True":"False");
#endif
   strcat(_build_string, _buf);
   
   return true;
}

///* :: addJoystick( link, link, title, xrange, yrange, resolution ) */
//
//bool CommManager::addJoystick(float* linker_x, float* linker_y,
//                              const char* title,
//                              float xrange_min, float xrange_max,
//                              float yrange_min, float yrange_max,
//                              float resolution, bool sticky) {
//   if( _total_controls + 2 > MAX_CONTROLS )
//      return false;
//      
//   _controls[_total_controls] = linker_x;
//   _ctrl_types[_total_controls++] = true; // float
//   _controls[_total_controls] = linker_y;
//   _ctrl_types[_total_controls++] = true; // float
//#ifdef S302_UNO
//   dtostrf(xrange_min, 0, MAX_PREC, _tmp);
//   sprintf(_buf, "J\r%.*s\r%s\r", MAX_TITLE_LEN, title, _tmp);
//   strcat(_build_string, _buf);
//   dtostrf(xrange_max, 0, MAX_PREC, _tmp);
//   sprintf(_buf, "%s\r", _tmp);
//   strcat(_build_string, _buf);
//   dtostrf(yrange_min, 0, MAX_PREC, _tmp);
//   sprintf(_buf, "%s\r", _tmp);
//   strcat(_build_string, _buf);
//   dtostrf(yrange_max, 0, MAX_PREC, _tmp);
//   sprintf(_buf, "%s\r", _tmp);
//   strcat(_build_string, _buf);
//   dtostrf(resolution, 0, MAX_PREC, _tmp);
//   sprintf(_buf, "%s\r%s\r",
//      _tmp, sticky? "True":"False");
//   strcat(_build_string, _buf);
//#else
//   sprintf(_buf, "J\r%.*s\r%f\r%f\r%f\r%f\r%f\r%s\r",
//      MAX_TITLE_LEN, title, xrange_min, xrange_max,
//                            yrange_min, yrange_max,
//      resolution, sticky? "True":"False");
//   strcat(_build_string, _buf);
//#endif
//   
//   return true;
//}

/* :: addPlot( link, title, yrange ) */

bool CommManager::addPlot(float* linker, const char* title,
                          float yrange_min, float yrange_max,
                          uint8_t steps_displayed,
                          uint8_t burst,
                          uint8_t num_plots) {
   if( _total_reporters >= MAX_REPORTERS
   ||  burst == 0
   ||  burst > (float)_report_period / (float)_step_period )
      return false;

   _reporters[_total_reporters] = linker;
   _bursts[_total_reporters++] = burst;
#ifdef S302_UNO
   strcpy(_buf, "P\r");
   strncat(_buf, title, MAX_TITLE_LEN);
   strcat(_buf, "\r");
   dtostrf(yrange_min, 0, MAX_PREC, _tmp);
   strcat(_buf, _tmp);
   strcat(_buf, "\r");
   dtostrf(yrange_max, 0, MAX_PREC, _tmp);
   strcat(_buf, _tmp);
   strcat(_buf, "\r");
   itoa(steps_displayed, _tmp, 10);
   strcat(_buf, _tmp);
   strcat(_buf, "\r");
   itoa(burst, _tmp, 10);
   strcat(_buf, _tmp);
   strcat(_buf, "\r");
   itoa(num_plots, _tmp, 10);
   strcat(_buf, _tmp);
   strcat(_buf, "\r");
#else
   sprintf(_buf, "P\r%.*s\r%f\r%f\r%d\r%d\r%d\r",
      MAX_TITLE_LEN, title, yrange_min, yrange_max,
      steps_displayed, burst, num_plots);
#endif
   strcat(_build_string, _buf);
   
   return true;
}

/* :: addNumber( link, title ) */

bool CommManager::addNumber(float* linker,
                            const char* title,
                            uint8_t burst) {
   if( _total_reporters >= MAX_REPORTERS
   ||  burst == 0
   ||  burst > (float)_report_period / (float)_step_period )
      return false;
      
   _reporters[_total_reporters] = linker;
   _bursts[_total_reporters++] = burst;
#ifdef S302_UNO
   strcpy(_buf, "N\r");
   strncat(_buf, title, MAX_TITLE_LEN);
   strcat(_buf, "\r");
   itoa(burst, _tmp, 10);
   strcat(_buf, _tmp);
   strcat(_buf, "\rfloat\r");
#else
   sprintf(_buf, "N\r%.*s\r%d\rfloat\r",
      MAX_TITLE_LEN, title, burst);
#endif
   strcat(_build_string, _buf);
   
   return true;
}

bool CommManager::addNumber(int32_t* linker,
                            const char* title,
                            uint8_t burst) {
   if( _total_reporters >= MAX_REPORTERS
   ||  burst == 0
   ||  burst > (float)_report_period / (float)_step_period )
      return false;

   _reporters[_total_reporters] = (float*)linker;
   _bursts[_total_reporters++] = burst;
#ifdef S302_UNO
   strcpy(_buf, "N\r");
   strncat(_buf, title, MAX_TITLE_LEN);
   strcat(_buf, "\r");
   itoa(burst, _tmp, 10);
   strcat(_buf, _tmp);
   strcat(_buf, "\rint\r");
#else
   sprintf(_buf, "N\r%.*s\r%d\rint\r",
      MAX_TITLE_LEN, title, burst);
#endif
   strcat(_build_string, _buf);

   return true;
}

/* THE MITOCHONDRIA */

void CommManager::step() {

   if( _total_reporters ) {

      if( _time_to_talk(_report_period) )
         _report();

      for( uint8_t reporter = 0; reporter < _total_reporters; reporter++ )
         _record(reporter);

   }

   _control();

#ifdef ESP32

   int32_t leftover = _step_period - (micros() - _secondary_timer);
   _headroom = leftover > 0? leftover : 0;
   _headroom_rp = (float)(min((int32_t)_headroom_rp, _headroom));
   if( leftover > 1000 ) {
      vTaskDelay(leftover / 1000);
      leftover = leftover % 1000;
   }
   if( leftover > 0 )
      delayMicroseconds(leftover);
   _secondary_timer = micros();

#else

   _wait(); // loop control

#endif
   
}

#ifdef ESP32
void CommManager::_walk(void* param) {
   CommManager* ptr = (CommManager*)param;
   uint32_t watchdogTimer = millis();
   for(;;) {
      /*if( millis() - watchdogTimer >= 4500 ) {
         vTaskDelay(1); // feed watchdog
         watchdogTimer = millis();
      }*/
      ptr->step();
   }
}
#endif

/* :: headroom() */

uint32_t CommManager::headroom() {
   return _headroom;
}

/* PRIVATE ROUTINES */

/* :: _control() */

void CommManager::_control() {

   // READ incoming message, if any
   _buf[0] = '\0';
#ifdef S302_SERIAL
   if( !_serial->available() )
      return;
   uint8_t i = 0;
   for(;;) {
      if( _serial->available() ) {
         _buf[i] = (char)(_serial->read());
         if( _buf[i++] == '\n' )
            break; // we need a newline
      }
   }
   _buf[i] = '\0';
#elif defined S302_WEBSOCKETS
   _wss.loop();
#endif

   // PARSE the message
   switch(_buf[0]) {
      
      case '\0': {
         // (No message)
         return;
      } break;
      
      case '\n': {
         // (GUI is asking for the build string!)

         // prepare current values!
         strcpy(_buf, "#\r");
         for( uint8_t i = 0; i < _total_controls; i++ ) {
            if( _ctrl_types[i] ) {
               // if float
#ifdef S302_UNO
               dtostrf(*_controls[i], 0, MAX_PREC, _tmp);
#else
               sprintf(_tmp, "%f", *_controls[i]);
#endif
               strcat(_buf, _tmp);
            } else {
               // if bool
               strcat(_buf, *((bool*)_controls[i])? "true":"false");
            }
            strcat(_buf, "\r");
         }

         // send buildstring!
         BROADCAST(_build_string, strlen(_build_string));
         BROADCAST(_buf, strlen(_buf));
         BROADCAST("\n", 2);
         return;
      } break;

      default: {
         // (update the value)
         if( _buf[strlen(_buf)-1] != '\n' )
            break; // only structured code allowed beyond this point
      
         int id = atoi(strtok(_buf, ":"));
         char val[24];
         strcpy(val, strtok(NULL, "\n"));
         if( !strcmp(val, "true") ) {
            *(bool*)_controls[id] = true;
         } else if ( !strcmp(val, "false") ) {
            *(bool*)_controls[id] = false;
         } else { // float
            *_controls[id] = atof(val);
         }

      } break;
   }
   
}

/* :: _record( reporter ) */

void CommManager::_record(uint8_t reporter) {
   float burst;
#ifdef TEENSYDUINO
   burst = (float)_report_timer
#else
   burst = (float)(micros() - _report_timer)
#endif
       * (float)_bursts[reporter] / (float)_report_period;
   uint8_t index = (int)burst; // round down to nearest index
   memcpy(&_recordings[reporter][index], (char*)_reporters[reporter], 4);
}

/* :: _report() */

void CommManager::_report() {
   // Send debug messages if any
   // Then send data report

   TAKE
   
   // Debug messages
   uint16_t n = strlen(_debug_string);
   if( n > 2 ) {
      BROADCAST("\fD", 2);
      BROADCAST(&_headroom_rp, 4);
      BROADCAST(_debug_string, n);
      BROADCAST("\n", 2);
      _debug_string[0] = '\0';
   }
   _headroom_rp = (float)INT32_MAX;

   GIVE

   // Data report
   strcpy(_buf, "\fR");
   BROADCAST(_buf, 2);
   
   for( uint8_t reporter = 0; reporter < _total_reporters; reporter++ )
      for( uint8_t burst = 0; burst < _bursts[reporter]; burst++ ) 
         BROADCAST(_recordings[reporter][burst], 4);
         
   BROADCAST("\n", 2);

}

/* :: _time_to_talk( time_to_wait ) */

bool CommManager::_time_to_talk(uint32_t time_to_wait) {
   // Whether or not enough time has passed according to the given
   // time period. Determines when to report data.
   // depends on the microcontroller
#ifdef TEENSYDUINO
   if( time_to_wait <= _report_timer ) {
      _report_timer = _report_timer - time_to_wait;
#else
   if( time_to_wait <= (micros() - _report_timer) ) {
      _report_timer = _report_timer + time_to_wait;
#endif
      return true;
   }
   return false;
}

/* :: _wait() */

void CommManager::_wait() {
   // How we wait depends on the microcontroller
#ifdef TEENSYDUINO
   uint32_t before = _main_timer;
   while(_step_period > _main_timer);
   _headroom = _main_timer - before;
   _main_timer = 0;
#else
   _headroom = _step_period - (micros() - _main_timer);
   if( _headroom > 0 )
      delayMicroseconds(_headroom);
   _main_timer = micros();
#endif
   _headroom_rp = (float)(min((int32_t)_headroom_rp, _headroom));
}

/* WebSocket event */

#ifdef S302_WEBSOCKETS
void CommManager::_on_websocket_event(
   uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
   
   switch(type) {
#ifdef S302_VERBOSE
      case WStype_DISCONNECTED: {
         Serial.printf("[%u] Disconnected\n", num);
      } break;
      case WStype_CONNECTED: {
         Serial.printf("[%u] Connected from %s\n",
            num, _wss.remoteIP(num).toString().c_str());
      } break;
#endif
      case WStype_TEXT: {
         if( payload[0] != '\0' ) {
            memcpy(_buf, payload, length);
            _buf[length] = '\0';
#ifdef S302_VERBOSE
            Serial.printf("[%u] Received: ", num);
            Serial.println((char*)payload);
         } else {
            Serial.printf("[%u] Received empty message!\n", num);
#endif
         }
      } break;

      default: break;
   }
}
#endif

/* Else */

void CommManager::debug(char* line) {

   // Add to the debug string buffer
   // (cuts off if too long though)

   TAKE

   uint16_t m = strlen(line);
   uint16_t n = strlen(_debug_string);
   uint16_t i = 0;
   while( i < m && n <= MAX_DEBUG_LEN-2 ) {
      _debug_string[n] = (line[i] != '\n'? line[i]:'\r');
      i++; n++;
   }

   _debug_string[n]   = '\r';
   _debug_string[n+1] = '\0';

   GIVE
   
   // _debug_string will be sent at each report period
}

void CommManager::debug(String line) {
   int len = line.length()+5;
   line.toCharArray(_buf, len);
   debug(_buf);
}

void CommManager::_NOT_IMPLEMENTED_YET() {

   // I'm not ready yet
   // Let me clean the living room
   // Then you can come back

}