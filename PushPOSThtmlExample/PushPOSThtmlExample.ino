#include <UIPEthernet.h> // Used for Ethernet
#include <OneWire.h>     // Used for temperature sensor(s)

// #define DEBUG

// **** ETHERNET SETTING ****
// Arduino Uno pins: 10 = CS, 11 = MOSI, 12 = MISO, 13 = SCK
// Ethernet MAC address - must be unique on your network - MAC Reads T4A001 in hex (unique in your network)
byte mac[] = { 0x54, 0x34, 0x41, 0x30, 0x30, 0x31 };                                       
// For the rest we use DHCP (IP address and such)

EthernetClient client;
char server[] = "192.168.1.100"; // IP Adres (or name) of server to dump data to
unsigned long PreviousMillis = 0;// For when millis goes past app 49 days. 
//unsigned long interval = 10000;  // Wait between dumps (10000 = 10 seconds)
unsigned long interval = 300000;  // Wait between dumps (1 min)
unsigned long intervalTime;      // Global var tracking Interval Time

// **** TEMPERATURE SETTINGS ****
// Sensor(s) data pin is connected to Arduino pin 2 in non-parasite mode!
OneWire  ds(2);

void setup() {
  #ifdef DEBUG
    Serial.begin(9600); // only use serial when debugging
  #endif
  
  Ethernet.begin(mac);
  
  intervalTime = millis();  // Set initial target trigger time (right NOW!)
  
  #ifdef DEBUG
    Serial.println("Tweaking4All.com - Temperature Drone - v2.3");
    Serial.println("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
    Serial.print("IP Address        : ");
    Serial.println(Ethernet.localIP());
    Serial.print("Subnet Mask       : ");
    Serial.println(Ethernet.subnetMask());
    Serial.print("Default Gateway IP: ");
    Serial.println(Ethernet.gatewayIP());
    Serial.print("DNS Server IP     : ");
    Serial.println(Ethernet.dnsServerIP());
  #endif
} 

void loop() {
  unsigned long CurrentMillis = millis();
  
  if ( CurrentMillis < PreviousMillis ) // millis reset to zero?
  {
    intervalTime = CurrentMillis+interval;
  }
  
  if ( CurrentMillis > intervalTime )  // Did we reach the target time yet?
  {
    intervalTime = CurrentMillis + interval;
    
    if (!client.connect(server, 80)) {
      #ifdef DEBUG
        Serial.println("-> Connection failure detected: Resetting ENC!");  // only use serial when debugging
      #endif
      Enc28J60.init(mac);
    } else {
      client.stop();
    } 
    
    // if you get a connection, report back via serial:
    if (client.connect(server, 80)) 
    {
      #ifdef DEBUG
        Serial.println("-> Connected");  // only use serial when debugging
      #endif
      
      // Make a HTTP request:
      client.print( "GET /testserver/arduino_temperatures/add_data.php?");
      
      TemperaturesToGetVariables(); // send serial and temperature readings
    
      client.println( " HTTP/1.1");
      client.println( "Host: 192.168.1.100" );
      client.print(" Host: ");
      client.println(server);
      client.println( "Connection: close" );
      client.println();
      client.println();
      client.stop();
    }
    else 
    {
      // you didn't get a connection to the server:
      #ifdef DEBUG
        Serial.println("--> connection failed !!");  // only use serial when debugging
      #endif
      
      //Enc28J60.init(mac);
    }
  }
  else
  {
    Ethernet.maintain();
  }
}

void TemperaturesToGetVariables(void) 
{
  byte counter;
  byte present = 0;
  byte sensor_type;
  byte data[12];
  byte addr[8];
  float celsius;
  byte sensorcounter;
  
  ds.reset_search();
  sensorcounter = 1; // we start counting with sensor number 1
  
  while ( ds.search(addr) ) 
  {
    if (sensorcounter>1) client.print("&"); // add ampersand if not first sensor
    
    client.print("serial"); // print: sensorx=
    client.print(sensorcounter);
    client.print("=");
    
    #ifdef DEBUG
      // Print Serial number
      Serial.print("   Sensor     : ");
      Serial.println(sensorcounter);
      Serial.print("   Serial     : ");
    #endif
    
    for( counter = 0; counter < 8; counter++) 
    {
      if (addr[counter]<10) client.print("0");
      client.print(String(addr[counter], HEX));
      if (counter<7) client.print("%20");
      
      #ifdef DEBUG 
        if (addr[counter]<10) Serial.print("0");
        Serial.print(String(addr[counter], HEX));
        if (counter<7) Serial.print(" ");
      #endif
    }
    
    #ifdef DEBUG
      Serial.println();   // only use serial when debugging
    #endif
    
    client.print("&temperature");  // print: &temperaturex=
    client.print(sensorcounter);
    client.print("=");
    
    // Check CRC
    if (OneWire::crc8(addr, 7) != addr[7]) // print ERROR if CRC error
    {
        client.println("ERROR");
    }
    else // CRC is OK
    {    
        // Removed sensor type detection and assumed DS18B20 sensor
        ds.reset();
        ds.select(addr);
        ds.write(0x44);  // start conversion, with regular (non-parasite!) power
        
        delay(750);     // maybe 750ms is enough, maybe not
        
        present = ds.reset();
        ds.select(addr);    
        ds.write(0xBE);  // Read Scratchpad
        
        // Get Raw Temp Data
        for ( counter = 0; counter < 9; counter++) 
        {           // we need 9 bytes
          data[counter] = ds.read();
        }
        
        // Convert the data to actual temperature
        int16_t raw = (data[1] << 8) | data[0];
    
        // at lower res, the low bits are undefined, so let's zero them
        byte cfg = (data[4] & 0x60);
        
        //// default is 12 bit resolution, 750 ms conversion time
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    
    
        celsius = (float)raw / 16.0;
        client.print(celsius);
    
        #ifdef DEBUG 
          Serial.print("   Temperature: ");
          Serial.print(celsius);
          Serial.println(" C");
        #endif
     } 
     
     sensorcounter++;
  }
  return;
}
