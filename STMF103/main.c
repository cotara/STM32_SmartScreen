#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "user_USART.h"
#include "user_GPIO.h"
#include "user_TIMER.h"
#include "myNextion.h"
#include "user_ds18b20.h"
#include "LED_user.h"
#include "i2c_ee.h"
#include "bme68x.h"
#include "userBME680.h"

int errorSensor =0;
uint16_t temper;

uint8_t isEmpty=1;
uint16_t temper=0;
uint16_t tempBuf[512];
uint32_t average=0;

ErrorStatus HSEStartUpStatus;
struct bme68x_dev bme;
struct bme68x_conf conf;
struct bme68x_heatr_conf heatr_conf;
struct bme68x_data data;
float iaq=0;

void RCC_Configuration(void)
{
    /*RCC system reset(for debug purpose) */
    RCC_DeInit();

    RCC_HSICmd(DISABLE);
    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if (HSEStartUpStatus == SUCCESS)
    {
        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        /* PCLK2 = HCLK*/
        RCC_PCLK2Config(RCC_HCLK_Div1);

        /* PCLK1 = HCLK*/
        RCC_PCLK1Config(RCC_HCLK_Div1);

        //ADC CLK
        RCC_ADCCLKConfig(RCC_PCLK2_Div2);

        RCC->CFGR|= 0x01<<17;
          
        /* PLLCLK = 8MHz * 3 = 24 MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

        /* Enable PLL */
        RCC_PLLCmd(ENABLE);

        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while (RCC_GetSYSCLKSource() != 0x08) {}
    }
}
void bme680Init(){
  int8_t rslt;
  
  rslt = bme68x_interface_init(&bme);
    bme68x_check_rslt("bme68x_interface_init", rslt);

    rslt = bme68x_init(&bme);
    bme68x_check_rslt("bme68x_init", rslt);

    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    conf.filter = BME68X_FILTER_SIZE_3;
    conf.odr = BME68X_ODR_20_MS;
    conf.os_hum = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_temp = BME68X_OS_16X;
    rslt = bme68x_set_conf(&conf, &bme);
    bme68x_check_rslt("bme68x_set_conf", rslt);

    
    /* Check if rslt == BME68X_OK, report or handle if otherwise */
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 150;
    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme);
    bme68x_check_rslt("bme68x_set_heatr_conf", rslt);

}
void getTHPG(){
    
    int8_t rslt;
    float gasTemp=0;
    uint32_t del_period;
    uint8_t n_fields;
    uint16_t sample_count = 0;

    while (sample_count < 1)
    {
        rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme);
        bme68x_check_rslt("bme68x_set_op_mode", rslt);

        /* Calculate delay period in microseconds */
        del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme) + (heatr_conf.heatr_dur * 1000);
        bme.delay_us(del_period, bme.intf_ptr);

        /* Check if rslt == BME68X_OK, report or handle if otherwise */
        rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme);
        bme68x_check_rslt("bme68x_get_data", rslt);

        if (n_fields)
        {
            sample_count++;
            gasTemp+=data.gas_resistance;
        }
    }
    data.gas_resistance = gasTemp/1;
}

int main()
{
  uint8_t status;
  
  SystemInit();
  
  
  //RCC_ClocksTypeDef RCC_Clocks;
  //RCC_GetClocksFreq(&RCC_Clocks);
  //RCC_Configuration();
  SysTick_Config(SystemCoreClock/1000);//1ms
  tim4_init();
  tim2_init();
  
  LEDInit();   
  GPIO_init();     
  usart_init();
  //I2C_EE_Init();
  
  bme680Init();
  while(1){
    getTHPG();
    iaq=iaqCalc(data.gas_resistance,data.humidity);
    delay_1_ms(50);
  }
  
  status=ds18b20_init(DS18B20_Resolution_12_bit);
    if (status){
      while(1);
    }
   
    
  while(1){
    if(getFLAG_END_LINE() == 3)
      nextionEvent();
    
    temper = ds18b20_get_temperature();
    setToEE(temper);
    
    if (getNowPage()==0){
     Nextion_SetValue_Number("x0.val=",temper);
       if(temper>=-300)
         Nextion_SetValue_Number("temperature.val=",temper*3/10+90);
       else
         Nextion_SetValue_Number("temperature.val=",temper*3/10+450);
     }
       
     else if(getNowPage()== 1){
       Nextion_SetValue_Number("add 2,0,",temper/2);
       
     }
     else if(getNowPage()== 2 && getReqBigBuf()){
       resetReqBigBuf();
       getFromEE(tempBuf,512);
       for(int i=0;i<512;i++){
        Nextion_SetValue_Number("add 1,0,",tempBuf[i]*0.667);
        average+=tempBuf[i];
       }
        Nextion_SetValue_Number("n0.val=",average/5120);
     }
       //delay_1_ms(500);
    }   

  
  
}
