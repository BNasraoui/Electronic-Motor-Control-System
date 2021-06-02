#include "current.h"
#include "sensors/sensors.h"

void InitADC1_CurrentSense() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_7);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    ADCSequenceConfigure(ADC1_BASE, ADC_SEQ , ADC_TRIGGER_PROCESSOR, 0);

    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQ, 0, ADC_CTL_CH0);
    ADCSequenceStepConfigure(ADC1_BASE, ADC_SEQ, 1, ADC_CTL_IE | ADC_CTL_CH4 | ADC_CTL_END);

    ADCSequenceEnable(ADC1_BASE, ADC_SEQ);
    ADCIntEnable(ADC1_BASE, ADC_SEQ);
    ADCIntClear(ADC1_BASE, ADC_SEQ);
    IntEnable(INT_ADC1SS1);
}

void ADC1_Read() {
    ADCIntClear(ADC1_BASE, ADC_SEQ);
    ADCSequenceDataGet(ADC1_BASE, ADC_SEQ , pui32ADC1Value);
    Swi_post(swiHandle_ADC1DataProc);
}

void ADC1_FilterFxn() {
    double Vref = 3.3;
    double Gcsa = 10;
    double Rsense = 0.007;

    ADC1Window.sum = ADC1Window.sum - ADC1Window.data[ADC1Window.index];
    ADC0Window.sum = ADC0Window.sum - ADC0Window.data[ADC0Window.index];

    ADC1Window.data[ADC1Window.index] = (int32_t)pui32ADC1Value[0];
    ADC0Window.data[ADC0Window.index] = (int32_t)pui32ADC1Value[1];

    ADC1Window.sum = ADC1Window.sum + ADC1Window.data[ADC1Window.index];
    ADC0Window.sum = ADC0Window.sum + ADC0Window.data[ADC0Window.index];

    if(ADC1Window.sum < 0) {
        int i = 7;
    }

    ADC1Window.index = (ADC1Window.index + 1) % CURRENT_WINDOW_SIZE;
    ADC0Window.index = (ADC0Window.index + 1) % CURRENT_WINDOW_SIZE;

    if(ADC1Window.startFilter) {
        ADC1Window.avg = (float)ADC1Window.sum / CURRENT_WINDOW_SIZE;
        ADC0Window.avg = (float)ADC0Window.sum / CURRENT_WINDOW_SIZE;

        ADC1Window.voltage = ADC1Window.avg * ADC_RESOLUTION;
        ADC0Window.voltage = ADC0Window.avg * ADC_RESOLUTION;

        ADC1Window.current = ((Vref/2) - ADC1Window.voltage) / (Gcsa * Rsense);
        ADC0Window.current = ((Vref/2) - ADC0Window.voltage) / (Gcsa * Rsense);

        ADC1Window.power = ((ADC1Window.voltage * ADC1Window.current) + (ADC0Window.voltage * ADC0Window.current))/2;
        ADC0Window.power = ADC0Window.voltage * ADC0Window.current;

        Event_post(sensors_eventHandle, NEW_ADC1_DATA);
    }

    if((ADC1Window.index + 1) == CURRENT_WINDOW_SIZE && ADC1Window.startFilter == false) {
        ADC1Window.startFilter = true;
    }
}
