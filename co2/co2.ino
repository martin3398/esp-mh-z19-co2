#include <SoftwareSerial.h>


class MHZ19C {
  private:
    const byte readCmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    const byte enableAutoCalibrationCmd[9] = { 0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6 };

    int temp = -1;
    int co2 = -1;

    SoftwareSerial *co2Serial;

    byte calcChecksum(byte *response) {
      byte checksum = 0;
      for (byte i = 1; i < 8; i++) {
        checksum += response[i];
      }
      return 0xff - checksum + 1;;
    }

  public:
    MHZ19C(int rx, int tx) {
      co2Serial = new SoftwareSerial(rx, tx);
      co2Serial->begin(9600);
    }

    bool read() {
      byte response[9];
      co2Serial->write(readCmd, 9);

      memset(response, 0, 9);
      delay(120);

      int i = 0;      
      while (co2Serial->available() == 0) {
        Serial.print(".");
        delay(100);
        if (++i > 20) {
          return false;
        }
      }

      int count = co2Serial->readBytes(response, 9);
      // less than 9 bytes received or wrong checksum or status not ok
      if (count < 9 || response[8] != calcChecksum(response) || response[5] != 0) {
        co2Serial->flush();
        return false;
      }

      temp = response[4] - 40;
      co2 = 256 * (int) response[2] + (int) response[3];

      return true;
    }

    bool isReady() {
      return millis() >= 60L * 1000L; // 60 seconds preheating time
    }

    void enableAutoCalibration() {
      co2Serial->write(enableAutoCalibrationCmd, 9);
    }

    int getTemp() {
      return temp;
    }

    int getCo2() {
      return co2;
    }
};


MHZ19C mhz19c(13, 15);

void setup() {
  Serial.begin(9600);
  
  Serial.print("Preheating");
  while (!mhz19c.isReady()) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Start MHZ 19C:");

  mhz19c.enableAutoCalibration();
}

void loop() {
  bool res = mhz19c.read();
  if (res) {
    Serial.print("co2: ");
    Serial.print(mhz19c.getCo2());
    Serial.print(", temp: ");
    Serial.println(mhz19c.getTemp());
  } else {
    Serial.println("Invalid data");
  }

  delay(1000);
}
