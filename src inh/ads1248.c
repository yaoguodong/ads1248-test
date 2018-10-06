#include "ads1248.h"
 

 
void ADS1248_GPIO_Init(void){
  GPIO_InitTypeDef  GPIO_InitStructure; 
	
	RCC_AHBPeriphClockCmd(ADS1248_CS_GPIO_CLK|ADS1248_GPIO_CLK,ENABLE);
	RCC_AHBPeriphClockCmd(ADS1248_START_GPIO_CLK|ADS1248_DRDY_GPIO_CLK|ADS1248_RESET_GPIO_CLK,ENABLE);
  RCC_APB2PeriphClockCmd(SPI_ADS1248_CLK, ENABLE);    
	
	GPIO_InitStructure.GPIO_Pin = ADS1248_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_Init(ADS1248_CS_GPIO_PORT, &GPIO_InitStructure);
  GPIO_PinLockConfig(ADS1248_CS_GPIO_PORT,ADS1248_CS_PIN);
	
  GPIO_InitStructure.GPIO_Pin   = ADS1248_CLK_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(ADS1248_GPIO_PORT, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin = ADS1248_SO_PIN;
  GPIO_Init(ADS1248_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = ADS1248_SI_PIN;
  GPIO_Init(ADS1248_GPIO_PORT, &GPIO_InitStructure);
 
  GPIO_PinAFConfig(ADS1248_GPIO_PORT, ADS1248_CLK_SOURCE, ADS1248_CLK_AF); 
  GPIO_PinAFConfig(ADS1248_GPIO_PORT, ADS1248_SO_SOURCE,  ADS1248_SO_AF); 
  GPIO_PinAFConfig(ADS1248_GPIO_PORT, ADS1248_SI_SOURCE,  ADS1248_SI_AF);  

  GPIO_InitStructure.GPIO_Pin   = ADS1248_START_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(ADS1248_START_GPIO_PORT, &GPIO_InitStructure);
	
  GPIO_InitStructure.GPIO_Pin   = ADS1248_RESET_GPIO_PIN; 
  GPIO_Init(ADS1248_RESET_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin   = ADS1248_DRDY_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(ADS1248_DRDY_GPIO_PORT, &GPIO_InitStructure);
	
	/*Delay20ms();
  ADS1248_DISABLE();
	ADS1248_START_L();
	ADS1248_RST_L();
	Delay10ms();
  ADS1248_RST_H();
	ADS1248_START_H();
  Delay10ms();*/
	
  ADS1248_DISABLE();
	ADS1248_START_H();
	ADS1248_RST_H();
	Delay20ms(); 
}

void ADS1248_SPI_Init(void){
	SPI_InitTypeDef   SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(SPI_ADS1248_CLK, ENABLE); 
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI_ADS1248, &SPI_InitStructure);
  SPI_Cmd(SPI_ADS1248, ENABLE);
	SPI_CalculateCRC(SPI_ADS1248, ENABLE); 
}
 
static unsigned char ADS1248_SPI_SendByte(unsigned char byte){
	unsigned int timeout=0;
	 
  while (SPI_I2S_GetFlagStatus(SPI_ADS1248, SPI_I2S_FLAG_TXE) == RESET){
	  timeout++;
		if(timeout > 2000){
		  return 0;
		}
	}
  SPI_I2S_SendData(SPI_ADS1248, byte);
 	timeout = 0; 
	while (SPI_I2S_GetFlagStatus(SPI_ADS1248, SPI_I2S_FLAG_RXNE) == RESET){
	  timeout++;
		if(timeout > 2000){
		  return 0;
		}
	}
  
  return SPI_I2S_ReceiveData(SPI_ADS1248);
}

int ADS1248WaitForDataReady(int Timeout){
  if (Timeout > 0)
	{
		// wait for /DRDY = 1 to make sure it is high before we look for the transition low
		while ( !( IS_ADS1248_READY() ) && (Timeout-- >= 0));
		// wait for /DRDY = 0
		while ( (IS_ADS1248_READY()) && (Timeout-- >= 0));
		if (Timeout < 0)
			return ADS1248_ERROR; 					//ADS1248_TIMEOUT_WARNING;
	}
	else
	{
		// wait for /DRDY = 1
		while ( !( IS_ADS1248_READY() ) );
		// wait for /DRDY = 0
		while ( IS_ADS1248_READY() );
	}
	return 0;
}
  
void ADS1248ReadRegister(int StartAddress, int NumRegs, unsigned * pData)
{
	int i;
	 
	// assert CS to start transfer 
	ADS1248_ENABLE(); 
	ADS1248_START_H(); 
	// send the command byte
	ADS1248_SPI_SendByte(ADS1248_CMD_RREG | (StartAddress & 0x0f));
	ADS1248_SPI_SendByte((NumRegs-1) & 0x0f);
	// get the register content
	for (i=0; i< NumRegs; i++)
	{
		*pData++ = ADS1248_SPI_SendByte(0xFF);
	} 
	//ADS1248_START_L();
	ADS1248_DISABLE(); 
	
	return;
}

void ADS1248WriteRegister(int StartAddress, int NumRegs, unsigned * pData){
	int i;
	// set the CS low  
	ADS1248_ENABLE(); 
	ADS1248_START_H(); 
	// send the command byte
	ADS1248_SPI_SendByte(ADS1248_CMD_WREG | (StartAddress & 0x0f));
	ADS1248_SPI_SendByte((NumRegs-1) & 0x0f);
	// send the data bytes
	for (i=0; i < NumRegs; i++)
	{
		ADS1248_SPI_SendByte(*pData++);
	} 
	//ADS1248_START_L(); 
	ADS1248_DISABLE();
}
 
void ADS1248WriteSequence(int StartAddress, int NumReg, unsigned * pData){

}

void ADS1248SendRDATAC(void)
{
	// assert CS to start transfer
	ADS1248_ENABLE();
	// send the command byte
	ADS1248_SPI_SendByte(ADS1248_CMD_RDATAC);
	// de-assert CS
	ADS1248_DISABLE();
	return;
}

void ADS1248SendSDATAC(void)
{
	// assert CS to start transfer
	ADS1248_ENABLE();
	// send the command byte
	ADS1248_SPI_SendByte(ADS1248_CMD_SDATAC);
	// de-assert CS
	ADS1248_DISABLE();
	return;
}

void ADS1248SendSYSOCAL(void)
{
	// assert CS to start transfer
	ADS1248_ENABLE();
	// send the command byte
	ADS1248_SPI_SendByte(ADS1248_CMD_SYSOCAL);
	// de-assert CS
	ADS1248_DISABLE();
	return;
}

void ADS1248SendSYSGCAL(void)
{
	// assert CS to start transfer
	ADS1248_ENABLE();
	// send the command byte
	ADS1248_SPI_SendByte(ADS1248_CMD_SYSGCAL);
	// de-assert CS
	ADS1248_DISABLE();
	return;
}

void ADS1248SendSELFOCAL(void)
{
	// assert CS to start transfer
	ADS1248_ENABLE();
	// send the command byte
	ADS1248_SPI_SendByte(ADS1248_CMD_SELFOCAL);
	// de-assert CS
	ADS1248_DISABLE();
	return;
}
/*
 * Register Set Value Commands
 *
 * These commands need to strip out old settings (AND) and add (OR) the new contents to the register
 */
int ADS1248SetBurnOutSource(int BurnOut)
{
	unsigned int Temp;
	int dError = ADS1248_NO_ERROR;
	ADS1248ReadRegister(ADS1248_0_MUX0, 0x01, &Temp);
	Temp &= 0x3f;
	switch(BurnOut) {
		case 0:
			Temp |= ADS1248_BCS_OFF;
			break;
		case 1:
			Temp |= ADS1248_BCS_500nA;
			break;
		case 2:
			Temp |= ADS1248_BCS_2uA;
			break;
		case 3:
			Temp |= ADS1248_BCS_10uA;
			break;
		default:
			dError = ADS1248_ERROR;
			Temp |= ADS1248_BCS_OFF;
	}
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_0_MUX0, 0x01, &Temp);
	return dError;
}

int ADS1248SetChannel(int vMux, int pMux){
	unsigned int Temp;
	int dError = ADS1248_NO_ERROR;
	ADS1248ReadRegister(ADS1248_0_MUX0, 0x01, &Temp);
	if (pMux==1) {
		Temp &= 0xf8;
		switch(vMux) {
			case 0:
				Temp |= ADS1248_AINN0;
				break;
			case 1:
				Temp |= ADS1248_AINN1;
				break;
			case 2:
				Temp |= ADS1248_AINN2;
				break;
			case 3:
				Temp |= ADS1248_AINN3;
				break;
			case 4:
				Temp |= ADS1248_AINN4;
				break;
			case 5:
				Temp |= ADS1248_AINN5;
				break;
			case 6:
				Temp |= ADS1248_AINN6;
				break;
			case 7:
				Temp |= ADS1248_AINN7;
				break;
			default:
				Temp |= ADS1248_AINN0;
				dError = ADS1248_ERROR;
		}

	} 
	else {
		Temp &= 0xc7;
		switch(vMux) {
			case 0:
				Temp |= ADS1248_AINP0;
				break;
			case 1:
				Temp |= ADS1248_AINP1;
				break;
			case 2:
				Temp |= ADS1248_AINP2;
				break;
			case 3:
				Temp |= ADS1248_AINP3;
				break;
			case 4:
				Temp |= ADS1248_AINP4;
				break;
			case 5:
				Temp |= ADS1248_AINP5;
				break;
			case 6:
				Temp |= ADS1248_AINP6;
				break;
			case 7:
				Temp |= ADS1248_AINP7;
				break;
			default:
				Temp |= ADS1248_AINP0;
				dError = ADS1248_ERROR;
		}
	}
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_0_MUX0, 0x01, &Temp); 
	return dError;
}

int ADS1248SetBias(unsigned char vBias)
{
	unsigned int Temp;
	Temp = ADS1248_VBIAS_OFF;
	if (vBias & 0x80)
		Temp |=  ADS1248_VBIAS7;
	if (vBias & 0x40)
		Temp |=  ADS1248_VBIAS6;
	if (vBias & 0x20)
		Temp |=  ADS1248_VBIAS5;
	if (vBias & 0x10)
		Temp |=  ADS1248_VBIAS4;
	if (vBias & 0x08)
		Temp |=  ADS1248_VBIAS3;
	if (vBias & 0x04)
		Temp |=  ADS1248_VBIAS2;
	if (vBias & 0x02)
		Temp |=  ADS1248_VBIAS1;
	if (vBias & 0x01)
		Temp |=  ADS1248_VBIAS0;
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_1_VBIAS, 0x01, &Temp);
	return ADS1248_NO_ERROR;
}

// Relate to Mux1
int ADS1248SetIntRef(int sRef)
{
	unsigned int Temp;
	int dError = ADS1248_NO_ERROR;
	ADS1248ReadRegister(ADS1248_2_MUX1, 0x01, &Temp);
	Temp &= 0x1f;
	switch(sRef) {
		case 0:
			Temp |= ADS1248_INT_VREF_OFF;
			break;
		case 1:
			Temp |= ADS1248_INT_VREF_ON;
			break;
		case 2:
		case 3:
			Temp |= ADS1248_INT_VREF_CONV;
			break;
		default:
			Temp |= ADS1248_INT_VREF_OFF;
			dError = ADS1248_ERROR;

	}
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_2_MUX1, 0x01, &Temp);
	return dError;
}

int ADS1248SetVoltageReference(int VoltageRef)
{
	unsigned int temp;
	int dError = ADS1248_NO_ERROR;
	ADS1248ReadRegister(ADS1248_2_MUX1, 0x01, &temp);
	temp &= 0xe7;
	switch(VoltageRef) {
		case 0:
			temp |= ADS1248_REF0;
			break;
		case 1:
			temp |= ADS1248_REF1;
			break;
		case 2:
			temp |= ADS1248_INT;
			break;
		case 3:
			temp |= ADS1248_INT_REF0;
			break;
		default:
			temp |= ADS1248_REF0;
			dError = ADS1248_ERROR;
	}
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_2_MUX1, 0x01, &temp);
	return dError;
}

int ADS1248SetSystemMonitor(int Monitor)
{
	unsigned int Temp;
	int dError = ADS1248_NO_ERROR;
	ADS1248ReadRegister(ADS1248_2_MUX1, 0x01, &Temp);
	Temp &= 0x78;
	switch(Monitor) {
		case 0:
			Temp |= ADS1248_MEAS_NORM;
			break;
		case 1:
			Temp |= ADS1248_MEAS_OFFSET;
			break;
		case 2:
			Temp |= ADS1248_MEAS_GAIN;
			break;
		case 3:
			Temp |= ADS1248_MEAS_TEMP;
			break;
		case 4:
			Temp |= ADS1248_MEAS_REF1;
			break;
		case 5:
			Temp |= ADS1248_MEAS_REF0;
			break;
		case 6:
			Temp |= ADS1248_MEAS_AVDD;
			break;
		case 7:
			Temp |= ADS1248_MEAS_DVDD;
			break;
		default:
			Temp |= ADS1248_MEAS_NORM;
			dError = ADS1248_ERROR;
	}
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_2_MUX1, 0x01, &Temp);
	return dError;
}

// Relate to SYS0
int ADS1248SetGain(int Gain)
{
	unsigned int Temp;
	int dError = ADS1248_NO_ERROR;
	ADS1248ReadRegister(ADS1248_3_SYS0, 0x01, &Temp);
	Temp &= 0x0f;
	switch(Gain) {
		case 0:
			Temp |= ADS1248_GAIN_1;
			break;
		case 1:
			Temp |= ADS1248_GAIN_2;
			break;
		case 2:
			Temp |= ADS1248_GAIN_4;
			break;
		case 3:
			Temp |= ADS1248_GAIN_8;
			break;
		case 4:
			Temp |= ADS1248_GAIN_16;
			break;
		case 5:
			Temp |= ADS1248_GAIN_32;
			break;
		case 6:
			Temp |= ADS1248_GAIN_64;
			break;
		case 7:
			Temp |= ADS1248_GAIN_128;
			break;
		default:
			Temp |= ADS1248_GAIN_1;
			dError = ADS1248_ERROR;
		}
		// write the register value containing the new value back to the ADS
		ADS1248WriteRegister(ADS1248_3_SYS0, 0x01, &Temp);
		return dError;
}

int ADS1248SetDataRate(int DataRate)
{
	unsigned int Temp;
	int dError = ADS1248_NO_ERROR;
	ADS1248ReadRegister(ADS1248_3_SYS0, 0x01, &Temp);
	Temp &= 0x70;
	switch(DataRate) {
		case 0:
			Temp |= ADS1248_DR_5;
			break;
		case 1:
			Temp |= ADS1248_DR_10;
			break;
		case 2:
			Temp |= ADS1248_DR_20;
			break;
		case 3:
			Temp |= ADS1248_DR_40;
			break;
		case 4:
			Temp |= ADS1248_DR_80;
			break;
		case 5:
			Temp |= ADS1248_DR_160;
			break;
		case 6:
			Temp |= ADS1248_DR_320;
			break;
		case 7:
			Temp |= ADS1248_DR_640;
			break;
		case 8:
			Temp |= ADS1248_DR_1000;
			break;
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			Temp |= ADS1248_DR_2000;
			break;
		default:
			Temp |= ADS1248_DR_5;
			dError = ADS1248_ERROR;
	}
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_3_SYS0, 0x01, &Temp);
	return dError;
}

// Relate to OFC (3 registers)
int ADS1248SetOFC(long RegOffset)
{
	// find the pointer to the variable so we can write the value as bytes
	unsigned *cptr=(unsigned *)(&RegOffset);
	int i;

	for (i=0; i<3; i++)
	{
		// write the register value containing the new value back to the ADS
		ADS1248WriteRegister((ADS1248_4_OFC0 + i), 0x01, &cptr[i]);
	}
	return ADS1248_NO_ERROR;
}

// Relate to FSC (3 registers)
int ADS1248SetFSC(long RegGain)
{
	// find the pointer to the variable so we can write the value as bytes
	unsigned *cptr=(unsigned *)(&RegGain);
	int i;
	for (i=0; i<3; i++)
	{
		// write the register value containing the new value back to the ADS
		ADS1248WriteRegister((ADS1248_7_FSC0 + i), 0x01, &cptr[i]);
	}
	return ADS1248_NO_ERROR;
}

// Relate to IDAC0
int ADS1248SetDRDYMode(int DRDYMode)
{
	unsigned int Temp;
	int dError = ADS1248_NO_ERROR;
	ADS1248ReadRegister(ADS1248_10_IDAC0, 0x01, &Temp);
	Temp &= 0xf7;
	switch(DRDYMode) {
		case 0:
			Temp |= ADS1248_DRDY_OFF;
			break;
		case 1:
			Temp |= ADS1248_DRDY_ON;
			break;
		default:
			Temp |= ADS1248_DRDY_OFF;
			dError = ADS1248_ERROR;
	}
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_10_IDAC0, 0x01, &Temp);
	return dError;
}

int ADS1248SetCurrentDACOutput(int CurrentOutput)
{
	unsigned int Temp;
	int dError = ADS1248_NO_ERROR;
	ADS1248ReadRegister(ADS1248_10_IDAC0, 0x01, &Temp);
	Temp &= 0xf8;
	switch(CurrentOutput) {
		case 0:
			Temp |= ADS1248_IDAC_OFF;
			break;
		case 1:
			Temp |= ADS1248_IDAC_50;
			break;
		case 2:
			Temp |= ADS1248_IDAC_100;
			break;
		case 3:
			Temp |= ADS1248_IDAC_250;
			break;
		case 4:
			Temp |= ADS1248_IDAC_500;
			break;
		case 5:
			Temp |= ADS1248_IDAC_750;
			break;
		case 6:
			Temp |= ADS1248_IDAC_1000;
			break;
		case 7:
			Temp |= ADS1248_IDAC_1500;
			break;
		default:
			Temp |= ADS1248_IDAC_OFF;
			dError = ADS1248_ERROR;
	}
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_10_IDAC0, 0x01, &Temp);
	return dError;
}
// Relate to IDAC1
int ADS1248SetIDACRouting(int IDACroute, int IDACdir)		// IDACdir (0 = I1DIR, 1 = I2DIR)
{
	unsigned int Temp;
	int dError = ADS1248_NO_ERROR;
	ADS1248ReadRegister(ADS1248_11_IDAC1, 0x01, &Temp);
	if (IDACdir>0){
		Temp &= 0xf0;
		switch(IDACroute) {
			case 0:
				Temp |= ADS1248_IDAC2_A0;
				break;
			case 1:
				Temp |= ADS1248_IDAC2_A1;
				break;
			case 2:
				Temp |= ADS1248_IDAC2_A2;
				break;
			case 3:
				Temp |= ADS1248_IDAC2_A3;
				break;
			case 4:
				Temp |= ADS1248_IDAC2_A4;
				break;
			case 5:
				Temp |= ADS1248_IDAC2_A5;
				break;
			case 6:
				Temp |= ADS1248_IDAC2_A6;
				break;
			case 7:
				Temp |= ADS1248_IDAC2_A7;
				break;
			case 8:
				Temp |= ADS1248_IDAC2_EXT1;
				break;
			case 9:
				Temp |= ADS1248_IDAC2_EXT2;
				break;
			case 10:
				Temp |= ADS1248_IDAC2_EXT1;
				break;
			case 11:
				Temp |= ADS1248_IDAC2_EXT2;
				break;
			case 12:
			case 13:
			case 14:
			case 15:
				Temp |= ADS1248_IDAC2_OFF;
				break;
			default:
				Temp |= ADS1248_IDAC2_OFF;
				dError = ADS1248_ERROR;
		}

	} else {
		Temp &= 0x0f;
		switch(IDACroute) {
			case 0:
				Temp |= ADS1248_IDAC1_A0;
				break;
			case 1:
				Temp |= ADS1248_IDAC1_A1;
				break;
			case 2:
				Temp |= ADS1248_IDAC1_A2;
				break;
			case 3:
				Temp |= ADS1248_IDAC1_A3;
				break;
			case 4:
				Temp |= ADS1248_IDAC1_A4;
				break;
			case 5:
				Temp |= ADS1248_IDAC1_A5;
				break;
			case 6:
				Temp |= ADS1248_IDAC1_A6;
				break;
			case 7:
				Temp |= ADS1248_IDAC1_A7;
				break;
			case 8:
				Temp |= ADS1248_IDAC1_EXT1;
				break;
			case 9:
				Temp |= ADS1248_IDAC1_EXT2;
				break;
			case 10:
				Temp |= ADS1248_IDAC1_EXT1;
				break;
			case 11:
				Temp |= ADS1248_IDAC1_EXT2;
				break;
			case 12:
			case 13:
			case 14:
			case 15:
				Temp |= ADS1248_IDAC1_OFF;
				break;
			default:
				Temp |= ADS1248_IDAC1_OFF;
				dError = ADS1248_ERROR;
		}
	}
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_11_IDAC1, 0x01, &Temp);
	return dError;
}

// Relate to GPIOCFG
int ADS1248SetGPIOConfig(unsigned char cdata)
{
	unsigned int Temp;
	Temp = 0x00;
	if (cdata & 0x80)
		Temp |=  ADS1248_GPIO_7;
	if (cdata & 0x40)
		Temp |=  ADS1248_GPIO_6;
	if (cdata & 0x20)
		Temp |=  ADS1248_GPIO_5;
	if (cdata & 0x10)
		Temp |=  ADS1248_GPIO_4;
	if (cdata & 0x08)
		Temp |=  ADS1248_GPIO_3;
	if (cdata & 0x04)
		Temp |=  ADS1248_GPIO_2;
	if (cdata & 0x02)
		Temp |=  ADS1248_GPIO_1;
	if (cdata & 0x01)
		Temp |=  ADS1248_GPIO_0;
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_12_GPIOCFG, 0x01, &Temp);
	return ADS1248_NO_ERROR;
}

// Relate to GPIODIR
int ADS1248SetGPIODir(unsigned char cdata)
{
	unsigned int Temp;
	Temp = 0x00;
	if (cdata & 0x80)
		Temp |=  ADS1248_IO_7;
	if (cdata & 0x40)
		Temp |=  ADS1248_IO_6;
	if (cdata & 0x20)
		Temp |=  ADS1248_IO_5;
	if (cdata & 0x10)
		Temp |=  ADS1248_IO_4;
	if (cdata & 0x08)
		Temp |=  ADS1248_IO_3;
	if (cdata & 0x04)
		Temp |=  ADS1248_IO_2;
	if (cdata & 0x02)
		Temp |=  ADS1248_IO_1;
	if (cdata & 0x01)
		Temp |=  ADS1248_IO_0;
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_13_GPIODIR, 0x01, &Temp);
	return ADS1248_NO_ERROR;
}

// Relate to GPIODAT
int ADS1248SetGPIO(unsigned char cdata)
{
	unsigned int Temp;
	Temp = 0x00;
	if (cdata & 0x80)
		Temp |=  ADS1248_OUT_7;
	if (cdata & 0x40)
		Temp |=  ADS1248_OUT_6;
	if (cdata & 0x20)
		Temp |=  ADS1248_OUT_5;
	if (cdata & 0x10)
		Temp |=  ADS1248_OUT_4;
	if (cdata & 0x08)
		Temp |=  ADS1248_OUT_3;
	if (cdata & 0x04)
		Temp |=  ADS1248_OUT_2;
	if (cdata & 0x02)
		Temp |=  ADS1248_OUT_1;
	if (cdata & 0x01)
		Temp |=  ADS1248_OUT_0;
	// write the register value containing the new value back to the ADS
	ADS1248WriteRegister(ADS1248_14_GPIODAT, 0x01, &Temp);
	return ADS1248_NO_ERROR;
}

/* Register Get Value Commands */
// Relate to MUX0
int ADS1248GetBurnOutSource(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_0_MUX0, 0x01, &Temp);
	return ((Temp >> 6) & 0x03);
}

int ADS1248GetChannel(int cMux)			// cMux = 0, AINP; cMux = 1, AINN
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_0_MUX0, 0x01, &Temp);
	if (cMux==0)
		return ((Temp >> 3) & 0x07);
	else
		return (Temp  & 0x07);
}

// Relate to VBIAS
unsigned char ADS1248GetBias(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_1_VBIAS, 0x01, &Temp);
	return (Temp & 0xff);
}

//Relate to MUX1
int ADS1248GetCLKSTAT(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_2_MUX1, 0x01, &Temp);
	return ((Temp >> 7) & 0x01);
}

int ADS1248GetIntRef(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_2_MUX1, 0x01, &Temp);
	return ((Temp >> 5) & 0x03);
}

int ADS1248GetVoltageReference(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_2_MUX1, 0x01, &Temp);
	return ((Temp >> 3) & 0x03);
}

int ADS1248GetSystemMonitor(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_2_MUX1, 0x01, &Temp);
	return (Temp & 0x07);
}

// Relate to SYS0
int ADS1248GetGain(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_3_SYS0, 0x01, &Temp);
	return ((Temp >> 4) & 0x07);
}

int ADS1248GetDataRate(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_3_SYS0, 0x01, &Temp);
	return (Temp & 0x0f);
}

// Relate to OFC (3 registers)
long ADS1248GetOFC(void)
{
	long rData=0;
	unsigned rValue=0;
	unsigned regArray[3];
	int i;
	//write the desired default register settings for the first 4 registers NOTE: values shown are the POR values as per datasheet
	regArray[0] = 0x00;
	regArray[1] = 0x00;
	regArray[2] = 0x00;
	for (i=0; i<3; i++)
	{
		// read the register value for the OFC
		ADS1248ReadRegister((ADS1248_4_OFC0 + i), 0x01, &rValue);
		regArray[i] = rValue;
	}
	rData = regArray[2];
	rData = (rData<<8) | regArray[1];
	rData = (rData<<8) | regArray[0];
	return rData;
}

// Relate to FSC (3 registers)
long ADS1248GetFSC(void)
{
	long rData=0;
	unsigned rValue=0;
	unsigned regArray[3];
	int i;
	//write the desired default register settings for the first 4 registers NOTE: values shown are the POR values as per datasheet
	regArray[0] = 0x00;
	regArray[1] = 0x00;
	regArray[2] = 0x00;
	for (i=0; i<3; i++)
	{
		// read the register value for the OFC
		ADS1248ReadRegister((ADS1248_7_FSC0 + i), 0x01, &rValue);
		regArray[i] = rValue;
	}
	rData = regArray[2];
	rData = (rData<<8) | regArray[1];
	rData = (rData<<8) | regArray[0];
	return rData;
}

int ADS1248GetDRDYMode(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_10_IDAC0, 0x01, &Temp);
	return ((Temp>>3) & 0x01);
}

int ADS1248GetCurrentDACOutput(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_10_IDAC0, 0x01, &Temp);
	return (Temp & 0x07);
}

// Relate to IDAC1
int ADS1248GetIDACRouting(int WhichOne) 		// IDACRoute (0 = I1DIR, 1 = I2DIR)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_11_IDAC1, 0x01, &Temp);
	if (WhichOne==0)
		return ((Temp>>4) & 0x0f);
	else
		return (Temp & 0x0f);
}

// Relate to GPIOCFG
unsigned char ADS1248GetGPIOConfig(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_12_GPIOCFG, 0x01, &Temp);
	return (Temp & 0xff);
}

// Relate to GPIODIR
unsigned char ADS1248GetGPIODir(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_13_GPIODIR, 0x01, &Temp);
	return (Temp & 0xff);
}

// Relate to GPIODAT
unsigned char ADS1248GetGPIO(void)
{
	unsigned int Temp;
	ADS1248ReadRegister(ADS1248_14_GPIODAT, 0x01, &Temp);
	return (Temp & 0xff);
}

int ADS1248RDATACRead(void)		// reads data directly based on RDATAC mode (writes NOP) and 32 SCLKs
{
	int data;
	// assert CS to start transfer
	ADS1248_ENABLE();
	// get the conversion result
	data = ADS1248_SPI_SendByte(0xFF);
	data = (data << 8) | ADS1248_SPI_SendByte(0xFF);
	data = (data << 8) | ADS1248_SPI_SendByte(0xFF);
	// sign extend data if the MSB is high (24 to 32 bit sign extension)
	if (data & 0x800000)
		data |= 0xff000000;
	// de-assert CS
	ADS1248_DISABLE();
	return data;
}

int ADS1248RDATARead(void)		// reads data directly based on RDATAC mode (writes NOP) and 32 SCLKs
{
	static int data;
	// assert CS to start transfer 
	ADS1248_ENABLE();  
	ADS1248_START_H(); 
	//__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	//ADS1248_START_L(); 
  //Delay200us();Delay200us();Delay200us();	
	while (0 == IS_ADS1248_READY()){}
	//ADS1248WaitForDataReady (0);
	// get the conversion result
	data = ADS1248_SPI_SendByte(0xFF);
	data = (data << 8) | ADS1248_SPI_SendByte(0xFF);
	data = (data << 8) | ADS1248_SPI_SendByte(0xFF);
	// sign extend data if the MSB is high (24 to 32 bit sign extension)
	if (data & 0x800000)
		data |= 0xff000000;
	// de-assert CS
	ADS1248_DISABLE();  
	
	return data;
}

void Delay1us(void){
  __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
}

void Delay10us(void){
	volatile int i=32; 
	while(i--);
	__nop();__nop();__nop();__nop();__nop();__nop();
}

void Delay200us(void){
  volatile int i=700;
	while(i--);
}

void Delay10ms(void){
  volatile int i=35555;
	while(i--);
}

void Delay20ms(void){
	volatile int i=71800;
	while(i--);
}
