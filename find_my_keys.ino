#include "BluetoothSerial.h"
#include <M5Core2.h>
#include <driver/i2s.h>

char Incoming_value = 0;

BluetoothSerial bluetooth;

extern const unsigned char previewR[120264];  



#define CONFIG_I2S_BCK_PIN 12 
#define CONFIG_I2S_LRCK_PIN 0
#define CONFIG_I2S_DATA_PIN 2
#define CONFIG_I2S_DATA_IN_PIN 34

#define Speak_I2S_NUMBER I2S_NUM_0  

#define MODE_MIC 0  
#define MODE_SPK 1
#define DATA_SIZE 1024

bool InitI2SSpeakOrMic(int mode){  
    esp_err_t err = ESP_OK;

    i2s_driver_uninstall(Speak_I2S_NUMBER); 
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER), 
        .sample_rate = 44100, 
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, 
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, 
        .communication_format = I2S_COMM_FORMAT_I2S,  
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, 
        .dma_buf_count = 2, 
        .dma_buf_len = 128, 
    };
    if (mode == MODE_MIC){
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
    }else{
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
        i2s_config.use_apll = false;  
        i2s_config.tx_desc_auto_clear = true; 
    }
    err += i2s_driver_install(Speak_I2S_NUMBER, &i2s_config, 0, NULL);

    i2s_pin_config_t tx_pin_config;
    tx_pin_config.bck_io_num = CONFIG_I2S_BCK_PIN;  
    tx_pin_config.ws_io_num = CONFIG_I2S_LRCK_PIN;  
    tx_pin_config.data_out_num = CONFIG_I2S_DATA_PIN; 
    tx_pin_config.data_in_num = CONFIG_I2S_DATA_IN_PIN; 
    err += i2s_set_pin(Speak_I2S_NUMBER, &tx_pin_config); 
    err += i2s_set_clk(Speak_I2S_NUMBER, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO); 
    return true;
}

void DisplayInit(void){ 
  M5.Lcd.fillScreen(WHITE); 
  M5.Lcd.setTextColor(BLACK); 
  M5.Lcd.setTextSize(2);  
}

void SpeakInit(void){ 
  M5.Axp.SetSpkEnable(true); 
  InitI2SSpeakOrMic(MODE_SPK);
}



void DingDong(void){
  size_t bytes_written = 0;
  i2s_write(Speak_I2S_NUMBER, previewR, 120264, &bytes_written, portMAX_DELAY);
}
                
void setup() 
{
  M5.begin(); 
  M5.Lcd.begin(); 
  M5.Lcd.setTextSize(4);
  DisplayInit();

  M5.Lcd.drawString("Device ON", 20, 15, 3);

  M5.Lcd.drawString("FIND MY KEYS", 40, 50, 3);
  M5.Lcd.print("Device ON\n");
  M5.Lcd.print("Bluetooth ON");
  M5.Lcd.drawString("FIND MY KEYS", 40, 50, 3);
  
SpeakInit();
  DingDong();
  
  Serial.begin(9600);         
  pinMode(14, OUTPUT);    
  pinMode(25, OUTPUT);

  digitalWrite(25, HIGH);

  bluetooth.begin("ESP32 Bluetooth");
  bluetooth.println("Bluetooth ready");
}

void loop()
{
  if(bluetooth.available())  
  {
    Incoming_value = bluetooth.read();      
    Serial.print(Incoming_value);        
    Serial.print("\n");        
    if(Incoming_value == '1')    
    {         
      digitalWrite(14, HIGH);
      M5.Axp.SetLDOEnable(3,true);  
      delay(100);
      M5.Axp.SetLDOEnable(3,false);
      DingDong();
    }
    else if(Incoming_value == '0')       
      digitalWrite(14, LOW);   
  }                            
} 
