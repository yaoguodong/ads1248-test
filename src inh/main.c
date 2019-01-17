#include "main.h" 
#include "ads1248.h"

void USER_GPIO_Init(void){ 
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,ENABLE);
   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_4|GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure); 
	
	GPIO_SetBits(GPIOC,GPIO_Pin_4);
	GPIO_SetBits(GPIOC,GPIO_Pin_5);
}

double f_adc=0,v[6][20],bat=0;
int i_adc=0,i=0,j=0,k=0,m=0,n=0,g=0;
int main(void){
  int t;
	
	USER_GPIO_Init();
	ADS1248_SPI_Init();
	ADS1248_GPIO_Init(); 
	ADS1248SetVoltageReference(1);  
	ADS1248SetDataRate(8); 
	Delay200us(); 
  while(1){
		GPIO_ToggleBits(GPIOC,GPIO_Pin_0);
	  ADS1248SetChannel(0,0);
	  ADS1248SetChannel(1,1);
		ADS1248SetGain(0); 
    for(t=0;t<10;t++){
		  //Delay20ms();
			Delay200us();
		} 
		v[0][i] = (float)ADS1248RDATACRead()*3/0x7FFFFF;
	  v[0][i] += 0;
		v[0][i++] += 0.00044;
		
		ADS1248SetGain(1); 
		for(t=0;t<10;t++){
		  //Delay20ms();
			Delay200us();
		} 
		v[1][j] = (float)ADS1248RDATACRead()*3/0x7FFFFF;
		v[1][j] /= 2;
		v[1][j++] += 0.00044;
		
		ADS1248SetGain(2); 
		for(t=0;t<10;t++){
			//Delay20ms();
		  Delay200us();
		} 
		v[2][k] = (float)ADS1248RDATACRead()*3/0x7FFFFF;
		v[2][k] /= 4;
		v[2][k++] += 0.00044;
		
		ADS1248SetGain(3); 
		for(t=0;t<10;t++){
			//Delay20ms();
		  Delay200us();
		} 
		v[3][m] = (float)ADS1248RDATACRead()*3/0x7FFFFF;
		v[3][m] /= 8;
		v[3][m++] += 0.00044;
		
		ADS1248SetGain(4); 
		for(t=0;t<10;t++){
			//Delay20ms();
		  Delay200us();
		} 
		v[4][n] = (float)ADS1248RDATACRead()*3/0x7FFFFF;
		v[4][n] /= 16;
		v[4][n++] += 0.00044;
		
		ADS1248SetGain(6); 
		for(t=0;t<10;t++){
			//Delay20ms();
		  Delay200us();
		} 
		v[5][g] = (float)ADS1248RDATACRead()*3/0x7FFFFF;
		v[5][g] /= 64;
		v[5][g++] += 0.00044;
		//v[2][k++] += 0.0031;
		/*ADS1248SetChannel(0,0);
	  ADS1248SetChannel(3,1);
		Delay200us(); Delay200us();Delay200us();//Delay10ms(); Delay200us(); Delay200us(); 
		v[1][j++] = (float)ADS1248RDATACRead()/0x7FFFFF*3;
		
		ADS1248SetChannel(1,0);
	  ADS1248SetChannel(3,1);
		Delay200us(); Delay200us();Delay200us();//Delay10ms();Delay200us();Delay200us(); Delay200us(); 
		v[2][k++] = (float)ADS1248RDATACRead()/0x7FFFFF*3;
		
		ADS1248SetChannel(2,0);
	  ADS1248SetChannel(3,1);
		ADS1248SetGain(0);
	  Delay200us(); Delay200us();Delay200us();//Delay10ms(); Delay200us();Delay200us();Delay200us(); 
    i_adc = ADS1248RDATACRead();
	  f_adc = (float)i_adc/0x7FFFFF*3;
	  f_adc = f_adc*251/51; 
		bat = f_adc;*/
		if(i == 20){
		 i=0; //while(1);
		}
	}
  return 0;
}

