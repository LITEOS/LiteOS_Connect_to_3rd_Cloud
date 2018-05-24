#include "bsp_adc.h"

uint16_t ADC_ConvertedValue;

void Rheostat_ADC_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // ʹ�� GPIO ʱ��
    RCC_AHB1PeriphClockCmd(RHEOSTAT_ADC_GPIO_CLK, ENABLE);

    // ���� IO
    GPIO_InitStructure.GPIO_Pin = RHEOSTAT_ADC_GPIO_PIN;
    // ����Ϊģ������
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    // ������������
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
    GPIO_Init(RHEOSTAT_ADC_GPIO_PORT, &GPIO_InitStructure);
}

void Rheostat_ADC_Mode_Config(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;

    // ���� ADC ʱ��
    RCC_APB2PeriphClockCmd(RHEOSTAT_ADC_CLK , ENABLE);

    // -------------------ADC Common �ṹ�� ���� ��ʼ��--------------------
    // ���� ADC ģʽ
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    // ʱ��Ϊ fpclk x ��Ƶ
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
    // ��ֹ DMA ֱ�ӷ���ģʽ
    ADC_CommonInitStructure.ADC_DMAAccessMode=ADC_DMAAccessMode_Disabled;
    // ����ʱ����
    ADC_CommonInitStructure.ADC_TwoSamplingDelay=
    ADC_TwoSamplingDelay_10Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    // -------------------ADC Init �ṹ�� ���� ��ʼ��---------------------
    // ADC �ֱ���
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    // ��ֹɨ��ģʽ����ͨ���ɼ�����Ҫ
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    // ����ת��
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    //��ֹ�ⲿ���ش���
    ADC_InitStructure.ADC_ExternalTrigConvEdge =
    ADC_ExternalTrigConvEdge_None;
    //ʹ������������ⲿ�����������ã�ע�͵�����
    //ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_T1_CC1;
    //�����Ҷ���
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    //ת��ͨ�� 1 ��
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(RHEOSTAT_ADC, &ADC_InitStructure);
    //------------------------------------------------------------------
    // ���� ADC ͨ��ת��˳��Ϊ 1����һ��ת��������ʱ��Ϊ 56 ��ʱ������
    ADC_RegularChannelConfig(RHEOSTAT_ADC, RHEOSTAT_ADC_CHANNEL,
    1, ADC_SampleTime_56Cycles);

    // ADC ת�����������жϣ����жϷ�������ж�ȡת��ֵ
    //ADC_ITConfig(RHEOSTAT_ADC, ADC_IT_EOC, ENABLE);
    // ʹ�� ADC
    //ADC_Cmd(RHEOSTAT_ADC, ENABLE);
    //��ʼ adc ת�����������
    //ADC_SoftwareStartConv(RHEOSTAT_ADC);
}

uint16_t Rheostat_ADC_StartConv(void)
{
    // ʹ�� ADC
    ADC_Cmd(RHEOSTAT_ADC, ENABLE);
    //��ʼ adc ת�����������
    ADC_SoftwareStartConv(RHEOSTAT_ADC);

    while(ADC_GetFlagStatus(RHEOSTAT_ADC,ADC_FLAG_EOC) == RESET){
    }
    
    ADC_ConvertedValue = ADC_GetConversionValue(RHEOSTAT_ADC);

    return ADC_ConvertedValue;
}

void Rheostat_ADC_EndConv(void)
{
    ADC_Cmd(RHEOSTAT_ADC, DISABLE);
}

void Rheostat_ADC_Init()
{
    Rheostat_ADC_GPIO_Config();
    Rheostat_ADC_Mode_Config();
}
