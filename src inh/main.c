#include "main.h" 
#include "ads1248.h"


float f_adc=0,v1,bat=0;
int i_adc=0;
int main(void){

	ADS1248_SPI_Init();
	ADS1248_GPIO_Init(); 
	ADS1248SetVoltageReference(1); 
	ADS1248SetDataRate(ADS1248_DR_2000);
  while(1){
		 
	  ADS1248SetChannel(0,0);
	  ADS1248SetChannel(1,1);
    //Delay20ms();//Delay20ms();Delay20ms();Delay20ms();Delay20ms();Delay20ms();Delay20ms();Delay20ms();Delay20ms();Delay20ms();
		v1 = (float)ADS1248RDATARead()*3/0x7FFFFF;
	 
		ADS1248SetChannel(2,0);
	  ADS1248SetChannel(3,1);
	  //Delay20ms();//Delay20ms();Delay20ms();Delay20ms();Delay20ms();Delay20ms();Delay20ms();Delay20ms();Delay20ms();Delay20ms();
    i_adc = ADS1248RDATARead();
	  f_adc = (float)i_adc*3/0x7FFFFF;
	  f_adc = f_adc*268/68; 
		bat = f_adc; 
	}
  return 0;
}