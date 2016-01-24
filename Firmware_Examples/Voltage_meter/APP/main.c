#define  _GLOBALS_

#include "App.h"


/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
int main(void)
{
  unsigned int uuid[2];
  int          stcnt = 0;

  PE_low_level_init();
  EEPROM_init();

  ADC_prepere();
  Clear_display();
  // ������� � ��������� RTT ���������� � ���� � ��� ���������� �������������
  Get_unique_identificator(uuid);
  RTT_printf("\r\n\r\nS9KEAZN64AMLC (64 KB FLASH, 4 KB RAM) UUID=%08X-%08X\r\n", uuid[0], uuid[1]);


  // ������ � �������� ����
  for (;;)
  {
    float f;
    if (tick_500ms != 0) // ������� ��������� ����� �� ����������� ���������� ���
    {
      switch (stcnt)
      {
        case 0:
          Show_mnemonic(Mnemonic_t);
          stcnt++;
          break;
        case 1:
          f = Get_internal_temperature();
          Show_float("%0.1f", f);
          stcnt++;
          break;
        case 2:
          Show_mnemonic(Mnemonic_v);
          stcnt++;
          break;
        case 3:
          f = Get_power_voltage();
          Show_float("%0.1f", f);
          stcnt = 0;
          break;

      }

      RTT_printf("%0.3f\r\n", f); // ����� � ��������� RTT � ���������� ������������ �����
      tick_500ms = 0; // ���������� ����, ����� ����� ��������� ��� ��������� 
    }
  }


}

/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void Cpu_HardFault(void)
{

}
/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void Cpu_OnNMIINT(void)
{

}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Get_unique_identificator(unsigned int *uid)
{
  SIM_MemMapPtr  SIM  = SIM_BASE_PTR;

  uid[0] = SIM->UUIDH;
  uid[1] = SIM->UUIDL;
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

