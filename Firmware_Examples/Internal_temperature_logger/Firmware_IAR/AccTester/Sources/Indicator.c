// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.01.08
// 15:39:03
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "app.h"
#include   "Indicator.h"

unsigned int pos;
unsigned char disp_buf[DISPLEN];
unsigned char ciphs[] =
{
  SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F,          //0
  SEG_B + SEG_C,                                          //1
  SEG_A + SEG_B + SEG_D + SEG_E + SEG_G,                  //2
  SEG_A + SEG_B + SEG_C + SEG_D + SEG_G,                  //3
  SEG_B + SEG_C +  SEG_F + SEG_G,                         //4
  SEG_A + SEG_C + SEG_D +  SEG_F + SEG_G,                 //5
  SEG_A + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G,          //6
  SEG_A + SEG_B + SEG_C,                                  //7
  SEG_A + SEG_B + SEG_C + SEG_D + SEG_E + SEG_F + SEG_G,  //8
  SEG_A + SEG_B + SEG_C + SEG_D + SEG_F + SEG_G,          //9
};

#define MAXSTRSZ 16
char nstr[MAXSTRSZ+1];
#define RTT_STR_SZ 128
char rtt_str[RTT_STR_SZ+1];

#define TSLOPE_LOW   0.003266
#define TSLOPE_HIGH  0.003638
#define T25VOLT      1.396
#define T25SMPL      1396

volatile unsigned int VAN_adcr;
volatile unsigned int VOLTAGE_adcr;
volatile unsigned int TEMPER_adcr;
volatile unsigned int INTTEMPER_adcr;

float int_temperature;
int   meas_done;

unsigned int tdiv;
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Get_unique_identificator(unsigned int *uid)
{
  SIM_MemMapPtr  SIM  = SIM_BASE_PTR;

  uid[0] = SIM->UUIDH;
  uid[1] = SIM->UUIDL;
}

/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
static void Set_LED(void)
{
  GPIOA_PSOR = BIT_PTD0;
}
/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
static void Clear_LED(void)
{
  GPIOA_PCOR = BIT_PTD0;
}


/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
static void Set_K1P(void)
{
  GPIOA_PSOR = BIT_PTD1;
}
/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
static void Set_K2P(void)
{
  GPIOA_PSOR = BIT_PTD2;
}
/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
static void Set_K3P(void)
{
  GPIOA_PSOR = BIT_PTD3;
}
/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
static void Clear_K1P(void)
{
  GPIOA_PCOR = BIT_PTD1;
}
/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
static void Clear_K2P(void)
{
  GPIOA_PCOR = BIT_PTD2;
}
/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
static void Clear_K3P(void)
{
  GPIOA_PCOR = BIT_PTD3;
}

/*------------------------------------------------------------------------------
 ��������� ��������� ���������


 \param val - ������� ���������� ���� ������������� ��������� - a, b, c, d, e, f, g, h
 ------------------------------------------------------------------------------*/
void Set_segments(unsigned char val)
{
  if (val & SEG_A)
  {
    GPIOA_PSOR = BIT_PTA0;
  }
  else
  {
    GPIOA_PCOR = BIT_PTA0;
  }
  if (val & SEG_B)
  {
    GPIOA_PSOR = BIT_PTA1;
  }
  else
  {
    GPIOA_PCOR = BIT_PTA1;
  }
  if (val & SEG_C)
  {
    GPIOA_PSOR = BIT_PTB7;
  }
  else
  {
    GPIOA_PCOR = BIT_PTB7;
  }
  if (val & SEG_D)
  {
    GPIOA_PSOR = BIT_PTB6;
  }
  else
  {
    GPIOA_PCOR = BIT_PTB6;
  }
  if (val & SEG_E)
  {
    GPIOA_PSOR = BIT_PTC6;
  }
  else
  {
    GPIOA_PCOR = BIT_PTC6;
  }
  if (val & SEG_F)
  {
    GPIOA_PSOR = BIT_PTC7;
  }
  else
  {
    GPIOA_PCOR = BIT_PTC7;
  }
  if (val & SEG_G)
  {
    GPIOA_PSOR = BIT_PTA6;
  }
  else
  {
    GPIOA_PCOR = BIT_PTA6;
  }
  if (val & SEG_H)
  {
    GPIOA_PSOR = BIT_PTA7;
  }
  else
  {
    GPIOA_PCOR = BIT_PTA7;
  }
}

/*------------------------------------------------------------------------------
   ����� ���������� ������ nstr ���������� ����� � ��������� ������ �� �������
 ------------------------------------------------------------------------------*/
static void NumToDisp()
{
  int i, k;
  unsigned char v;

  k = 0;
  for (i = 0; i < MAXSTRSZ; i++)
  {
    if ((nstr[i] >= '0') & (nstr[i] <= '9'))
    {
      v = nstr[i] - 0x30;
      disp_buf[k] = ciphs[v];
      k++;

    }
    else if (nstr[i] == '.')
    {
      if (k > 0)
      {
        disp_buf[k - 1] = disp_buf[k - 1] | SEG_H;
      }
    }
    else if (nstr[i] == '-')
    {
      disp_buf[k] = SEG_G;
      k++;
    }

    if (k >= DISPLEN) break;
  }
}

/*------------------------------------------------------------------------------
 \param fmt - ������ ��������������
 \param val - �������� � ��������� ������
 ------------------------------------------------------------------------------*/
void Show_float(const char *fmt, float val)
{
  snprintf(nstr, MAXSTRSZ, fmt, val);
  NumToDisp();
}
/*------------------------------------------------------------------------------
 \param fmt - ������ ��������������
 \param val - �������� � ��������� ������
 ------------------------------------------------------------------------------*/
void Show_int(const char *fmt, int val)
{
  snprintf(nstr, MAXSTRSZ, fmt, val);
  NumToDisp();
}

/*------------------------------------------------------------------------------
  ������� ��������������� ������ � RTT
 \param BufferIndex
 \param sFormat
 ------------------------------------------------------------------------------*/
void RTT_printf(const char *fmt, ...)
{
  int n;
  va_list           ap;
  va_start(ap, fmt);
  n = vsnprintf(rtt_str, RTT_STR_SZ, (char *)fmt, ap);
  va_end(ap);
  SEGGER_RTT_Write(0, rtt_str, n);
}
/*------------------------------------------------------------------------------
 ���������� ADC � ������

 ------------------------------------------------------------------------------*/
void ADC_prepere(void)
{
  // ��������� �������� ����������� �������� ��� ADC
  // ��� ����� PIT0 ����� 0
  SIM_SOPT |= LSHIFT(1, 8);
}
/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void Clear_display(void)
{
  int i;
  for (i = 0; i < DISPLEN; i++)
    disp_buf[i] = 0x0;
}
/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void Application(void)
{
  unsigned int uuid[2];
  ADC_prepere();
  Clear_display();
  // ������� � ��������� RTT ���������� � ���� � ��� ���������� �������������
  Get_unique_identificator(uuid);
  RTT_printf("\r\n\r\nS9KEAZN64AMLC (64 KB FLASH, 4 KB RAM) UUID=%08X-%08X\r\n", uuid[0], uuid[1]);

  // ������ � �������� ����
  for (;;)
  {
    if (meas_done != 0) // ������� ��������� ����� �� ����������� ���������� ���
    {
      // �������� �������� ������� ������� ��� � ���������� � ����� float
      if (INTTEMPER_adcr > T25SMPL)
      {
        //  ���� 25 �
        int_temperature = 25 - (INTTEMPER_adcr - T25SMPL) / (1000.0 * TSLOPE_LOW);
      }
      else
      {
        // ���� 25 �
        int_temperature = (T25SMPL - INTTEMPER_adcr) / (1000.0 * TSLOPE_HIGH) + 25;
      }
      Show_float("%0.1f", int_temperature);     // ����� ��  �������
      RTT_printf("%0.3f\r\n", int_temperature); // ����� � ��������� RTT � ���������� ������������ �����
      meas_done = 0; // ���������� ����, ����� ����� ��������� ��� ���������
    }
  }
}

/*------------------------------------------------------------------------------
   ���������� ���������� �� ADC

 ------------------------------------------------------------------------------*/
void ADC_isr_procedure(void)
{
  Set_LED();
  VAN_adcr = ADC_R;
  VOLTAGE_adcr = ADC_R;
  TEMPER_adcr = ADC_R;
  INTTEMPER_adcr = ADC_R;

  ADC_SC1 = ADC_SC1_ADCH(9);
  ADC_SC1 = ADC_SC1_ADCH(10);
  ADC_SC1 = ADC_SC1_ADCH(11);
  ADC_SC1 = ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(22);

  Clear_LED();
}
/*------------------------------------------------------------------------------
  ������������ ����������
  �������� �� ���������� ������ 1 ��
 ------------------------------------------------------------------------------*/
void SysTick_ISR_procedure(void)
{
  // .....................................................................
  // ���� ������������ ����������

  Clear_K1P(); // ���������� ������� ���� 3-� �����������
  Clear_K2P(); // ���������� ��������
  Clear_K3P(); //

  Set_segments(disp_buf[pos]); // ���������� ��������� ��������� � ������� �� ������� � ������� pos

  switch (pos) // ���������� ����� ��������� ����������
  {
  case 0:
    Set_K1P();
    break;
  case 1:
    Set_K2P();
    break;
  case 2:
    Set_K3P();
    break;
  }

  pos++;  // �������� ������ pos ����������� �� ������� ������� �� �������
  if (pos > 2) pos = 0;

  // ��������� ����� ������������ ����������
  // .....................................................................

  tdiv++;
  if (tdiv > 500)
  {
    tdiv = 0;
    // ������� ����������� ������ 0.5 ���
    meas_done = 1;
  }

}


