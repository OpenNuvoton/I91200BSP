/**************************************************
 *
 * Part one of the system initialization code, contains low-level
 * initialization, plain thumb variant.
 *
 * Copyright 2010 IAR Systems. All rights reserved.
 *
 * $Revision: 34539 $
 *
 **************************************************/

;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
;        EXTERN  SystemInit
        PUBLIC  __vector_table

        DATA
__vector_table
                DCD     sfe(CSTACK)               ; Top of Stack
                DCD     Reset_Handler       	  ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts
                                                  ; maximum of 32 External Interrupts are possible
                DCD		BOD_IRQHandler    ; 16. Brownout low voltage detector interrupt                  
                DCD		WDT_IRQHandler	  ; 17. Watch Dog Timer interrupt
                DCD		EINT0_IRQHandler  ; 18. External signal interrupt from PB.0 pin
                DCD		EINT1_IRQHandler  ; 19. External signal interrupt from PB.1 pin    
                DCD		GPAB_IRQHandler	  ; 20. External signal interrupt from PA[15:0] / PB[7:2]
                DCD		ALC_IRQHandler    ; 21. Automatic Level Control Interrupt
                DCD		PWM0_IRQHandler   ; 22. PWM0 channel 0/1/2/3 interrupt
                DCD		Default_Handler   ; 23. Reserved
                DCD		TMR0_IRQHandler   ; 24. Timer 0 interrupt
                DCD		TMR1_IRQHandler	  ; 25. Timer 1 interrupt
                DCD		Default_Handler	  ; 26. Reserved
                DCD		UART1_IRQHandler  ; 27. UART 1 interrupt	
                DCD		UART0_IRQHandler  ; 28.	UART0 interrupt
                DCD		SPI1_IRQHandler	  ; 29. SPI1 interrupt
                DCD		SPI0_IRQHandler	  ; 30. SPI0 interrupt
                DCD		DPWM_IRQHandler   ; 31. DPWM interrupt
                DCD		Default_Handler   ; 32. Reserved
                DCD		Default_Handler   ; 33. Reserved
                DCD		I2C0_IRQHandler   ; 34. I2C0 interrupt                
                DCD		Default_Handler	  ; 35. Reserved
                DCD		Default_Handler	  ; 36. Reserved
                DCD		CMP_IRQHandler    ; 37. CMP interrupt
                DCD		Default_Handler   ; 38. Reserved
                DCD		Default_Handler   ; 39. Reserved
                DCD		Default_Handler   ; 40. Reserved
                DCD		SARADC_IRQHandler ; 41. SARADC interrupt
                DCD		PDMA_IRQHandler   ; 42. PDMA interrupt
                DCD		I2S0_IRQHandler   ; 43. I2S0 interrupt
                DCD		CAPS_IRQHandler   ; 44. Capacitive Touch Sensing Relaxation Oscillator Interrupt               
                DCD		SDADC_IRQHandler  ; 45. Audio ADC interrupt
                DCD		Default_Handler   ; 46. Reserved
                DCD		RTC_IRQHandler    ; 47. Real time clock interrupt  
                DCD		0  ;ROM/RAM configuration, 0:  N571P032 mode ;32K ROM; 4K RAM.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
      PUBWEAK Reset_Handler
      PUBWEAK NMI_Handler
      PUBWEAK HardFault_Handler
      PUBWEAK SVC_Handler
      PUBWEAK PendSV_Handler
      PUBWEAK SysTick_Handler
      PUBWEAK BOD_IRQHandler    ; 16. Brownout low voltage detector interrupt                  
      PUBWEAK WDT_IRQHandler    ; 17. Watch Dog Timer interrupt
      PUBWEAK EINT0_IRQHandler  ; 18. External signal interrupt from PB.0 pin
      PUBWEAK EINT1_IRQHandler  ; 19. External signal interrupt from PB.1 pin    
      PUBWEAK GPAB_IRQHandler	; 20. External signal interrupt from PA[15:0] / PB[7:2]
      PUBWEAK ALC_IRQHandler    ; 21. Automatic Level Control Interrupt
      PUBWEAK PWM0_IRQHandler   ; 22. PWM0 channel 0/1/2/3 interrupt
      PUBWEAK TMR0_IRQHandler   ; 24. Timer 0 interrupt
      PUBWEAK TMR1_IRQHandler	; 25. Timer 1 interrupt
      PUBWEAK UART1_IRQHandler  ; 27. UART 1 interrupt	
      PUBWEAK UART0_IRQHandler  ; 28.	UART0 interrupt
      PUBWEAK SPI1_IRQHandler   ; 29. SPI1 interrupt
      PUBWEAK SPI0_IRQHandler   ; 30. SPI0 interrupt
      PUBWEAK DPWM_IRQHandler   ; 31. DPWM interrupt
      PUBWEAK I2C0_IRQHandler   ; 34. I2C0 interrupt                
      PUBWEAK CMP_IRQHandler    ; 37. CMP interrupt
      PUBWEAK SARADC_IRQHandler ; 41. SARADC interrupt
      PUBWEAK PDMA_IRQHandler   ; 42. PDMA interrupt
      PUBWEAK I2S0_IRQHandler   ; 43. I2S0 interrupt
      PUBWEAK CAPS_IRQHandler   ; 44. Capacitive Touch Sensing Relaxation Oscillator Interrupt               
      PUBWEAK SDADC_IRQHandler  ; 45. Audio ADC interrupt
      PUBWEAK RTC_IRQHandler    ; 47. Real time clock interrupt  
	
        THUMB
        SECTION .text:CODE:REORDER(2)
Reset_Handler
;                LDR     R0, =SystemInit
;                BLX     R0
;*************Add by Nuvoton***************
        ; Unlock Register   
        LDR     R0, =0x50000100
        LDR     R1, =0x59
        STR     R1, [R0]
        LDR     R1, =0x16
        STR     R1, [R0]
        LDR     R1, =0x88
        STR     R1, [R0]
        
        ; Init POR
        LDR     R2, =0x50000024
        LDR     R1, =0x00005AA5
        STR     R1, [R2]
        
        ; Lock register
        MOVS    R1, #0
        STR     R1, [R0]  
        
	; Set event and execute WFE to ensure WIC is initialized.
	; Because WIC state doesn't synchronize with NVIC when M0 is in standby domain 
 	SEV                  
        WFE  
;******************************************
               LDR     R0, =__iar_program_start
               BX      R0
              SECTION .text:CODE:REORDER(2)
NMI_Handler
HardFault_Handler
        LDR    R0, [R13, #24]        ; Get previous PC
        LDRH   R1, [R0]              ; Get instruction
        LDR    R2, =0xBEAB           ; The sepcial BKPT instruction
        CMP    R1, R2                ; Test if the instruction at previous PC is BKPT
        BNE    HardFault_Handler_Ret ; Not BKPT

        ADDS   R0, #4                ; Skip BKPT and next line
        STR    R0, [R13, #24]        ; Save previous PC

        BX     LR
HardFault_Handler_Ret
        B      .

SVC_Handler
PendSV_Handler
SysTick_Handler
BOD_IRQHandler    ; 16. Brownout low voltage detector interrupt                  
WDT_IRQHandler    ; 17. Watch Dog Timer interrupt
EINT0_IRQHandler  ; 18. External signal interrupt from PB.0 pin
EINT1_IRQHandler  ; 19. External signal interrupt from PB.1 pin    
GPAB_IRQHandler	  ; 20. External signal interrupt from PA[15:0] / PB[7:2]
ALC_IRQHandler    ; 21. Automatic Level Control Interrupt
PWM0_IRQHandler   ; 22. PWM0 channel 0/1/2/3 interrupt
TMR0_IRQHandler   ; 24. Timer 0 interrupt
TMR1_IRQHandler	  ; 25. Timer 1 interrupt
UART1_IRQHandler  ; 27. UART 1 interrupt	
UART0_IRQHandler  ; 28.	UART0 interrupt
SPI1_IRQHandler   ; 29. SPI1 interrupt
SPI0_IRQHandler   ; 30. SPI0 interrupt
DPWM_IRQHandler   ; 31. DPWM interrupt
I2C0_IRQHandler   ; 34. I2C0 interrupt                
CMP_IRQHandler    ; 37. CMP interrupt
SARADC_IRQHandler ; 41. SARADC interrupt
PDMA_IRQHandler   ; 42. PDMA interrupt
I2S0_IRQHandler   ; 43. I2S0 interrupt
CAPS_IRQHandler   ; 44. Capacitive Touch Sensing Relaxation Oscillator Interrupt               
SDADC_IRQHandler  ; 45. Audio ADC interrupt
RTC_IRQHandler    ; 47. Real time clock interrupt  
Default_Handler
        B Default_Handler
        END
