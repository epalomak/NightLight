; M‰‰rit‰ LED_PIN arvot, MOTION_SENSOR_PIN ja muut mahdolliset rekisteriarvot ennen koodin suorittamista


.export _init
.export _init_timer
.export _init_adc

; LEDien alustaminen
_init:
    LDI R16, (1 << 0) | (1 << 1) | (1 << 2)    ; LED_RED_PIN=0, LED_GREEN_PIN=1, LED_BLUE_PIN=2
    OUT PORTB_DIRSET, R16

    OUT PORTB_OUTCLR, R16

    LDI R16, (1 << 2) 
    OUT PORTD_DIRCLR, R16
    RET

; Ajastimen alustaminen
_init_timer:
    LDI R16, LOW(16000)
    OUT TCA0_SINGLE_PERL, R16
    LDI R16, HIGH(16000)
    OUT TCA0_SINGLE_PERH, R16

    LDI R16, (TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm)
    OUT TCA0_SINGLE_CTRLA, R16

    LDI R16, TCA_SINGLE_OVF_bm
    OUT TCA0_SINGLE_INTCTRL, R16
    RET

_init_adc:
    
    LDI R16, ADC_ENABLE_bm
    OUT ADC0_CTRLA, R16

    LDI R16, ADC_PRESC_DIV16_gc
    OUT ADC0_CTRLB, R16

    LDI R16, ADC_MUXPOS_AIN0_gc
    OUT ADC0_MUXPOS, R16

    LDI R16, ADC_REFSEL_VDDREF_gc
    OUT ADC0_CTRLC, R16
    RET
