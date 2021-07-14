#include "POWERMGNT.h"
#include "DAC.h"
#include "targets.h"


#if defined(Regulatory_Domain_AU_915) || defined(Regulatory_Domain_EU_868) || defined(Regulatory_Domain_FCC_915) || defined(Regulatory_Domain_AU_433) || defined(Regulatory_Domain_EU_433)
extern SX127xDriver Radio;
#elif Regulatory_Domain_ISM_2400
extern SX1280Driver Radio;
#endif

PowerLevels_e POWERMGNT::CurrentPower = (PowerLevels_e)DefaultPowerEnum;

PowerLevels_e POWERMGNT::incPower()
{
    if (CurrentPower < MaxPower)
    {
        setPower((PowerLevels_e)((uint8_t)CurrentPower + 1));
    }
    return CurrentPower;
}

PowerLevels_e POWERMGNT::decPower()
{
    if (CurrentPower > 0)
    {
        setPower((PowerLevels_e)((uint8_t)CurrentPower - 1));
    }
    return CurrentPower;
}

PowerLevels_e POWERMGNT::currPower()
{
    return CurrentPower;
}

uint8_t POWERMGNT::powerToCrsfPower(PowerLevels_e Power)
{
    // Crossfire's power levels as defined in opentx:radio/src/telemetry/crossfire.cpp
    //static const int32_t power_values[] = { 0, 10, 25, 100, 500, 1000, 2000, 250 };
    switch (Power)
    {
    case PWR_10mW: return 1;
    case PWR_25mW: return 2;
    case PWR_100mW: return 3;
    case PWR_250mW: return 7;
    case PWR_500mW: return 4;
    case PWR_1000mW: return 5;
    case PWR_2000mW: return 6;
    case PWR_50mW:
    default:
        return 0;
    }
}

void POWERMGNT::init()
{
#if DAC_IN_USE
    TxDAC.init();
#endif // DAC_IN_USE
#ifdef TARGET_R9M_LITE_PRO_TX
    //initialize both 12 bit DACs
    pinMode(GPIO_PIN_RFamp_APC1, OUTPUT);
    pinMode(GPIO_PIN_RFamp_APC2, OUTPUT);
    analogWriteResolution(12);
#endif
#if defined(GPIO_PIN_FAN_EN) && (GPIO_PIN_FAN_EN != UNDEF_PIN)
    pinMode(GPIO_PIN_FAN_EN, OUTPUT);
#endif
    CurrentPower = PWR_COUNT;
}

void POWERMGNT::setDefaultPower()
{
    setPower((PowerLevels_e)DefaultPowerEnum);
}

PowerLevels_e POWERMGNT::setPower(PowerLevels_e Power)
{
    if (Power == CurrentPower)
        return CurrentPower;

    if (Power > MaxPower)
    {
        Power = (PowerLevels_e)MaxPower;
    }


#if DAC_IN_USE
    // DAC is used e.g. for R9M, ES915TX and Voyager
    Radio.SetOutputPower(0b0000);
    TxDAC.setPower((DAC_PWR_)Power);
#elif defined(TARGET_TX_ESP32_SX1280_V1) || defined(TARGET_RX_ESP8266_SX1280_V1)
    switch (Power)
    {
    case PWR_10mW:
        Radio.SetOutputPower(8);
        break;
    case PWR_25mW:
    default:
        Radio.SetOutputPower(13);
        Power = PWR_25mW;
        break;
    }
#elif defined(TARGET_NAMIMNORC_TX)
    // Control Flash 2.4GHz TX module
    int8_t rfpower = -18;
    switch (Power)
    {
    case PWR_10mW:
        rfpower = -18;
        break;
    case PWR_25mW:
        rfpower = -18;
        break;
    case PWR_100mW:
        rfpower = -12;
        break;
    case PWR_250mW:
        rfpower = -8;
        break;
    case PWR_500mW:
        rfpower = -5;
        break;
    case PWR_1000mW:
        rfpower = 3;
        break;
    case PWR_50mW:
    default:
        rfpower = -15;
        Power = PWR_50mW;
        break;
    }
    Radio.SetOutputPower(rfpower);

#elif defined(TARGET_TX_GHOST)
    switch (Power)
    {
    case PWR_10mW:
        #ifdef TARGET_TX_GHOST_LITE
            Radio.SetOutputPower(-16);
        #else
            Radio.SetOutputPower(-16);
        #endif
        break;
    case PWR_25mW:
        #ifdef TARGET_TX_GHOST_LITE
            Radio.SetOutputPower(-14);
        #else
            Radio.SetOutputPower(-14);
        #endif
        break;
    case PWR_100mW:
        #ifdef TARGET_TX_GHOST_LITE
            Radio.SetOutputPower(-8);
        #else
            Radio.SetOutputPower(-8);
        #endif
        break;
    case PWR_250mW:
        #ifdef TARGET_TX_GHOST_LITE
            Radio.SetOutputPower(-4);
        #else
            Radio.SetOutputPower(-4);
        #endif
        break;
    case PWR_50mW:
    default:
        Power = PWR_50mW;
        #ifdef TARGET_TX_GHOST_LITE
            Radio.SetOutputPower(-11);
        #else
            Radio.SetOutputPower(-11);
        #endif
        break;
    }
#elif defined(TARGET_R9M_LITE_PRO_TX)
    Radio.SetOutputPower(0b0000);
    //Set DACs PA5 & PA4
    switch (Power)
    {
    case PWR_100mW:
        analogWrite(GPIO_PIN_RFamp_APC1, 3350); //0-4095 2.7V
        analogWrite(GPIO_PIN_RFamp_APC2, 732); //0-4095  590mV
        CurrentPower = PWR_100mW;
        break;
    case PWR_250mW:
        analogWrite(GPIO_PIN_RFamp_APC1, 3350); //0-4095 2.7V
        analogWrite(GPIO_PIN_RFamp_APC2, 1080); //0-4095 870mV this is actually 200mw
        CurrentPower = PWR_250mW;
        break;
    case PWR_500mW:
        analogWrite(GPIO_PIN_RFamp_APC1, 3350); //0-4095 2.7V
        analogWrite(GPIO_PIN_RFamp_APC2, 1356); //0-4095 1.093V
        CurrentPower = PWR_500mW;
        break;
    case PWR_1000mW:
        analogWrite(GPIO_PIN_RFamp_APC1, 3350); //0-4095 2.7V
        analogWrite(GPIO_PIN_RFamp_APC2, 1853); //0-4095 1.493V
        CurrentPower = PWR_1000mW;
        break;
    default:
        CurrentPower = PWR_100mW;
        analogWrite(GPIO_PIN_RFamp_APC1, 3350); //0-4095 2.7V
        analogWrite(GPIO_PIN_RFamp_APC2, 732);  //0-4095 590mV
        break;
    }
#elif defined(TARGET_100mW_MODULE) || defined(TARGET_R9M_LITE_TX)
    switch (Power)
    {
    case PWR_10mW:
        Radio.SetOutputPower(0b1000);
        CurrentPower = PWR_10mW;
        break;
    case PWR_25mW:
        Radio.SetOutputPower(0b1100);
        CurrentPower = PWR_25mW;
        break;
    case PWR_50mW:
    default:
        Power = PWR_50mW;
        Radio.SetOutputPower(0b1111); //15
        break;
    }
#elif defined(TARGET_1000mW_MODULE)
    switch (Power)
    {
    case PWR_100mW:
        Radio.SetOutputPower(0b0101);
        break;
    case PWR_250mW:
        Radio.SetOutputPower(0b1000);
        break;
    case PWR_500mW:
        Radio.SetOutputPower(0b1100);
        break;
    case PWR_1000mW:
        Radio.SetOutputPower(0b1111);
        break;
    case PWR_50mW:
    default:
        Radio.SetOutputPower(0b0010);
        Power = PWR_50mW;
        break;
    }
#elif defined(TARGET_TX_ESP32_E28_SX1280_V1)
    switch (Power)
    {
    case PWR_10mW:
        #ifdef TARGET_HappyModel_ES24TX_2400_TX
            Radio.SetOutputPower(-17);
        #else
            Radio.SetOutputPower(-15);
        #endif
        break;
    case PWR_25mW:
        #ifdef TARGET_HappyModel_ES24TX_2400_TX
            Radio.SetOutputPower(-13);
        #else
            Radio.SetOutputPower(-11);
        #endif
        break;
    case PWR_50mW:
        #ifdef TARGET_HappyModel_ES24TX_2400_TX
            Radio.SetOutputPower(-9);
        #else
            Radio.SetOutputPower(-8);
        #endif
        break;
    case PWR_100mW:
        #ifdef TARGET_HappyModel_ES24TX_2400_TX
            Radio.SetOutputPower(-6);
        #else
            Radio.SetOutputPower(-5);
        #endif
        break;
    case PWR_250mW:
        #ifdef TARGET_HappyModel_ES24TX_2400_TX
            Radio.SetOutputPower(-2);
        #else
            Radio.SetOutputPower(-1);
        #endif
        break;
    default:
        Power = PWR_50mW;
        #ifdef TARGET_HappyModel_ES24TX_2400_TX
            Radio.SetOutputPower(-9);
        #else
            Radio.SetOutputPower(-8);
        #endif
        break;
    }
#elif defined(TARGET_TX_ESP32_LORA1280F27)
    switch (Power)
    {
    case PWR_10mW:
        Radio.SetOutputPower(-4);
        break;
    case PWR_25mW:
        Radio.SetOutputPower(0);
        break;
    case PWR_50mW:
        Radio.SetOutputPower(3);
        break;
    case PWR_100mW:
        Radio.SetOutputPower(6);
        break;
    case PWR_250mW:
        Radio.SetOutputPower(12);
        break;
    default:
        Power = PWR_50mW;
        Radio.SetOutputPower(3);
        break;
    }
#elif defined(TARGET_TX_FM30) || defined(TARGET_RX_FM30_MINI)
    switch (Power)
    {
    case PWR_10mW:
        Radio.SetOutputPower(-15); // ~10.5mW
        break;
    case PWR_25mW:
        Radio.SetOutputPower(-11); // ~26mW
        break;
    case PWR_100mW:
        Radio.SetOutputPower(-1);  // ~99mW
        break;
    case PWR_250mW:
        // The original FM30 can somehow put out +22dBm but the 2431L max input
        // is +6dBm, and even when SetOutputPower(13) you still only get 150mW
        Radio.SetOutputPower(6);  // ~150mW
        break;
    case PWR_50mW:
    default:
        Power = PWR_50mW;
        Radio.SetOutputPower(-7); // -7=~55mW, -8=46mW
        break;
    }
#elif defined(TARGET_RX)
#ifdef TARGET_SX1280
    Radio.SetOutputPower(13); //default is max power (12.5dBm for SX1280 RX)
#else
    Radio.SetOutputPower(0b1111); //default is max power (17dBm for SX127x RX@)
#endif
#else
#error "[ERROR] Unknown power management!"
#endif
    CurrentPower = Power;
    return Power;
}
