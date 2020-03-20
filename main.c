#include "stm32f4xx.h"                  
static __IO uint32_t usTicks;
 
//FUNCTIONS PROTOTYPE

void SysTick_Handler(void);
void DelayMs(uint32_t);//delay in ms
void h_drv_SPI_CS_Disable (void);
void HDriverSPIInitialization (void);
void h_drv_SPI_CS_Enable (void);
uint8_t h_drv_SPI_Write_Byte (uint8_t);
uint8_t h_drv_Read_Status_Register (void);
void DelayInitialization();
void write(long, uint8_t);//write
int8_t WriteDataToChip(long, uint8_t *, uint8_t);//write data to chip memory
void EnableWriteToChip (void);//enable write data to chip
int8_t ReadDataToChip (long, uint8_t *, uint8_t);//read data from chip memory
void DeleteAllDataFromChip(void);//delete all data from chip memory
uint8_t Read(long);//read


//GLOBAL VARIABLES
uint8_t data[] = {1,2,3,4,5,6,7,8,9,10};//data to writing to the chip
uint8_t statusRegister = 0;//status register
uint8_t read[10] = {0};//data read to the chip


//MAIN
int main()
{
	DelayInitialization();
	HDriverSPIInitialization();//Initialization
	EnableWriteToChip();//to make a chip ready to write data you need to enable writing
	statusRegister = h_drv_Read_Status_Register();
	
	DeleteAllDataFromChip();//to make a chip ready to write data you need to remove everything from it
	DelayMs(40);
	WriteDataToChip(0, data, sizeof(data));
	ReadDataToChip (0, read, sizeof(read));
	
	while (1);
	
}




//FUNCTIONS REALIAZTION

int8_t ReadDataToChip (long StartAdd, uint8_t *Data, uint8_t Len)
{
if ((StartAdd < 0) || (StartAdd> 0xFFFFFF))
		return -1; 
	if ((Len > 0) && ((StartAdd+Len) <  0xFFFFFF))
	{
		for (int i = 0; i<Len; i++)
		{
			*(Data+i) = Read(StartAdd+i);
		}
	}	
		else
			return -1;

}

uint8_t Read(long add)
{
	uint8_t result = 0;
	h_drv_SPI_CS_Enable();
	h_drv_SPI_Write_Byte(0x03);
	h_drv_SPI_Write_Byte ((add>>16)&0xFF);
	h_drv_SPI_Write_Byte ((add>>8)&0xFF);
	h_drv_SPI_Write_Byte (add&0xFF);
	
	result = h_drv_SPI_Write_Byte(0x00);	
	
	h_drv_SPI_CS_Disable();
	return result;
}

void h_drv_SPI_CS_Disable (void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_7);
}

uint8_t h_drv_SPI_Write_Byte (uint8_t Data)
{
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET){}
  SPI_I2S_SendData(SPI1, Data);
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET){}
  return SPI_I2S_ReceiveData(SPI1);
}

void SysTick_Handler()
{
    if (usTicks != 0)
    {
        usTicks--;
    }
}

void DelayMs(uint32_t ms)
{
    usTicks = ms;
    while (usTicks);
}

void DelayInitialization()
{
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);
}

void EnableWriteToChip (void)
{
	h_drv_SPI_CS_Enable();
	h_drv_SPI_Write_Byte(0x06);
	h_drv_SPI_CS_Disable();
	
	h_drv_SPI_CS_Enable();
  h_drv_SPI_Write_Byte(0x50);
	h_drv_SPI_CS_Disable();
	
	h_drv_SPI_CS_Enable();
	h_drv_SPI_Write_Byte(0x01);
	h_drv_SPI_Write_Byte(0x00);
	h_drv_SPI_CS_Disable();
}

void HDriverSPIInitialization (void)
{
	GPIO_InitTypeDef 	GPIO_Init_LED;
	SPI_InitTypeDef		SPI_Init_user;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	GPIO_Init_LED.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init_LED.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init_LED.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init_LED.GPIO_OType = GPIO_OType_PP;
	GPIO_Init_LED.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_Init_LED);
	
	GPIO_Init_LED.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_Init_LED.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init_LED.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init_LED.GPIO_OType = GPIO_OType_PP;
	GPIO_Init_LED.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_Init_LED);
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
	
	
	GPIO_Init_LED.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init_LED.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init_LED.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init_LED.GPIO_OType = GPIO_OType_PP;
	GPIO_Init_LED.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_Init_LED);
	
	h_drv_SPI_CS_Disable();
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	SPI_Init_user.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_Init_user.SPI_Mode = SPI_Mode_Master;
	SPI_Init_user.SPI_DataSize = SPI_DataSize_8b;
	SPI_Init_user.SPI_CPOL = SPI_CPOL_High;
	SPI_Init_user.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_Init_user.SPI_NSS = SPI_NSS_Soft;
	SPI_Init_user.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
	SPI_Init_user.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init_user.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_Init_user);
	SPI_Cmd(SPI1, ENABLE);
}


void h_drv_SPI_CS_Enable (void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_7);
}

uint8_t h_drv_Read_Status_Register (void)
{
	uint8_t Status = 0;
	
	h_drv_SPI_CS_Enable();
	h_drv_SPI_Write_Byte(0x05);
	Status = h_drv_SPI_Write_Byte(0);
	h_drv_SPI_CS_Disable();
	return Status;
}

int8_t WriteDataToChip(long StartAdd, uint8_t *Data, uint8_t Len)
{
	if ((StartAdd < 0) || (StartAdd> 0xFFFFFF))
		return -1; 
	if ((Len > 0) && ((StartAdd+Len) <  0xFFFFFF))
	{
		for (int i = 0; i<Len; i++)
		{
			write(StartAdd+i, *(Data+i));
		}
	}	
		else
			return -1;
}

void DeleteAllDataFromChip(void)
{
	h_drv_SPI_CS_Enable();
	h_drv_SPI_Write_Byte(0x06);
	h_drv_SPI_CS_Disable();
	
	h_drv_SPI_CS_Enable();
	h_drv_SPI_Write_Byte(0x50);
	h_drv_SPI_CS_Disable();
}

void write(long add, uint8_t data)
{
	h_drv_SPI_CS_Enable();
	h_drv_SPI_Write_Byte(0x06);
	h_drv_SPI_CS_Disable();	
	
	h_drv_SPI_CS_Enable();
	h_drv_SPI_Write_Byte(0x02);
	h_drv_SPI_Write_Byte(((add>>16)&0xFF));
	h_drv_SPI_Write_Byte(((add>>8)&0xFF));
	h_drv_SPI_Write_Byte((add&0xFF));
	
	h_drv_SPI_Write_Byte(data);
	
	h_drv_SPI_CS_Disable();
}

