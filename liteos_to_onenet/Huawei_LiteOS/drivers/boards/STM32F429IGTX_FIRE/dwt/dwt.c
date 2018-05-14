#include "dwt.h"

static uint32_t cpuclkfeq;     //���ڱ���cpu����Ƶ�ʣ�������ʱ��̬�޸�

//��ʼ����ʱϵͳ������ΪCPUƵ��

void DelayInit(uint32_t clk)
{

    cpuclkfeq = clk;
//��CYCCNT����,���Ѽ��������㣬���򿪼�������cpuʱ�ӽ������ϼ���

    DEM_CR         |=  DEM_CR_TRCENA; 

    DWT_CYCCNT      = 0u;    //������Ҫ������ԣ�����������Ҫʹ��CYCCNTʱע�͵��������ֱ������ 

    DWT_CR         |= DWT_CR_CYCCNTENA;

}

//��ʱ����������Ϊ��Ҫ��ʱ��΢����
void Delayus(uint32_t usec)

{
     uint32_t startts,endts,ts;
     UINT32 uwIntSave;

  //������뺯��ʱ�ļ�����ֵ

     startts = DWT_CYCCNT;

     ts =  usec * (cpuclkfeq /(1000*1000));        //����ﵽ������ʱֵ��cpuʱ����,^-^�����Ҫ����ȷ�˴����Լ�ȥ����ǰ����������ʱ������

     endts = startts + ts;           //����ﵽ������ʱʱ���DWT_CYCCNT����ֵ������32bit���ܱ������ֵ2��32�η�-1���Զ��ƻض�����λ
     uwIntSave=LOS_IntLock();
      if(endts > startts)            //�ж��Ƿ��Խ���ֵ�߽�

      {

          while(DWT_CYCCNT < endts);        //�ȵ�������������ʱֵ��cpuʱ����ֵ

       }

       else

      {

           while(DWT_CYCCNT > endts);       //�ȴ�����32bit�����ֵ��2��32�η�-1

           while(DWT_CYCCNT < endts);        //�ȵ�������������ʱֵ��cpuʱ����ֵ

      }
    (VOID)LOS_IntRestore(uwIntSave);

}

void Delay10ms(__IO u32 nTime)
{
    Delayus(1000*nTime);
}
