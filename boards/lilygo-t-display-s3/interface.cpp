#include "core/powerSave.h"
#include "interface.h"
#include <globals.h>
#include <Button.h>
volatile bool nxtPress=false;
volatile bool prvPress=false;
volatile bool ecPress=false;
volatile bool slPress=false;
static void onButtonSingleClickCb1(void *button_handle, void *usr_data) {
  nxtPress = true;
}
static void onButtonDoubleClickCb1(void *button_handle, void *usr_data) {
  slPress=true;
}
static void onButtonHoldCb1(void *button_handle, void *usr_data)
{
  slPress=true;
}


static void onButtonSingleClickCb2(void *button_handle, void *usr_data) {
  prvPress=true;
}
static void onButtonDoubleClickCb2(void *button_handle, void *usr_data) {
  ecPress=true;
}
static void onButtonHoldCb2(void *button_handle, void *usr_data)
{
  ecPress=true;
}

Button *btn1;
Button *btn2;

#if defined(T_DISPLAY_S3)
#include <esp_adc_cal.h>
#endif

/***************************************************************************************
** Function name: _setup_gpio()
** Description:   initial setup for the device
***************************************************************************************/
void _setup_gpio()
{
  // setup buttons
  button_config_t bt1 = {
    .type = BUTTON_TYPE_GPIO,
    .long_press_time = 600,
    .short_press_time = 120,
    .gpio_button_config = {
        .gpio_num = DW_BTN,
        .active_level = 0,
    },
  };
  button_config_t bt2 = {
    .type = BUTTON_TYPE_GPIO,
    .long_press_time = 600,
    .short_press_time = 120,
    .gpio_button_config = {
        .gpio_num = UP_BTN,
        .active_level = 0,
    },
  };
  pinMode(SEL_BTN, INPUT_PULLUP);

  btn1 = new Button(bt1);

  //btn->attachPressDownEventCb(&onButtonPressDownCb, NULL);
  btn1->attachSingleClickEventCb(&onButtonSingleClickCb1,NULL);
  btn1->attachDoubleClickEventCb(&onButtonDoubleClickCb1,NULL);
  btn1->attachLongPressStartEventCb(&onButtonHoldCb1,NULL);
  
  btn2 = new Button(bt2);

  //btn->attachPressDownEventCb(&onButtonPressDownCb, NULL);
  btn2->attachSingleClickEventCb(&onButtonSingleClickCb2,NULL);
  btn2->attachDoubleClickEventCb(&onButtonDoubleClickCb2,NULL);
  btn2->attachLongPressStartEventCb(&onButtonHoldCb2,NULL);  

  // setup POWER pin required by the vendor
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  // setup Battery pin for reading voltage value
  pinMode(BAT_PIN, INPUT);

  // Start with default IR, RF and RFID Configs, replace old
  bruceConfig.rfModule = CC1101_SPI_MODULE;
  bruceConfig.rfidModule = PN532_I2C_MODULE;

  bruceConfig.irRx = RXLED;
  bruceConfig.irTx = LED;
}

/***************************************************************************************
** Function name: getBattery()
** Description:   Delivers the battery value from 1-100
***************************************************************************************/
int getBattery()
{
  int percent = 0;
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  uint32_t raw = analogRead(BAT_PIN);
  uint32_t v1 = esp_adc_cal_raw_to_voltage(raw, &adc_chars) * 2;

  if (v1 > 4150)
  {
    percent = 0;
  }
  else
  {
    percent = map(v1, 3200, 4150, 0, 100);
  }

  return (percent < 0)      ? 0
         : (percent >= 100) ? 100
                            : percent;
}

/*********************************************************************
**  Function: setBrightness
**  set brightness value
**********************************************************************/
void _setBrightness(uint8_t brightval)
{
  if (brightval == 0)
  {
    analogWrite(TFT_BL, brightval);
  }
  else
  {
    int bl = MINBRIGHT + round(((255 - MINBRIGHT) * brightval / 100));
    analogWrite(TFT_BL, bl);
  }
}

/*********************************************************************
** Function: InputHandler
** Handles the variables PrevPress, NextPress, SelPress, AnyKeyPress and EscPress
**********************************************************************/


void InputHandler(void)
{
  static bool btn_pressed=false;
  if(nxtPress || prvPress || ecPress || slPress) btn_pressed=true;
  bool selPressed = (digitalRead(SEL_BTN) == BTN_ACT);

  bool anyPressed = nxtPress || selPressed || prvPress || ecPress || slPress;
  if (anyPressed && wakeUpScreen()) return;

  AnyKeyPress = anyPressed;
  SelPress = selPressed || slPress;
  EscPress = ecPress;
  NextPress = nxtPress;
  PrevPress = prvPress;

  if(btn_pressed) {
    btn_pressed=false;
    nxtPress=false;
    prvPress=false;
    ecPress=false;
    slPress=false;
  }

  if (AnyKeyPress) {
    long tmp = millis();
    while ((millis() - tmp) < 200 && (digitalRead(SEL_BTN) == BTN_ACT)) {
      vTaskDelay(pdMS_TO_TICKS(5));  // Small delay instead of busy wait
    }
  }
}

void powerOff()
{
#ifdef T_DISPLAY_S3
  tft.fillScreen(bruceConfig.bgColor);
  digitalWrite(PIN_POWER_ON, LOW);
  digitalWrite(TFT_BL, LOW);
  tft.writecommand(0x10);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)SEL_BTN, BTN_ACT);
  esp_deep_sleep_start();
#endif
}

void checkReboot()
{
#ifdef T_DISPLAY_S3
  int countDown;
  /* Long press power off */
  if (digitalRead(UP_BTN) == BTN_ACT && digitalRead(DW_BTN) == BTN_ACT)
  {
    uint32_t time_count = millis();
    while (digitalRead(UP_BTN) == BTN_ACT && digitalRead(DW_BTN) == BTN_ACT)
    {
      // Display poweroff bar only if holding button
      if (millis() - time_count > 500)
      {
        tft.setTextSize(1);
        tft.setTextColor(bruceConfig.priColor, bruceConfig.bgColor);
        countDown = (millis() - time_count) / 1000 + 1;
        if (countDown < 4)
          tft.drawCentreString("PWR OFF IN " + String(countDown) + "/3", tftWidth / 2, 12, 1);
        else
        {
          tft.fillScreen(bruceConfig.bgColor);
          while (digitalRead(UP_BTN) == BTN_ACT || digitalRead(DW_BTN) == BTN_ACT)
            ;
          delay(200);
          powerOff();
        }
        delay(10);
      }
    }

    // Clear text after releasing the button
    delay(30);
    tft.fillRect(60, 12, tftWidth - 60, tft.fontHeight(1), bruceConfig.bgColor);
  }
#endif
}