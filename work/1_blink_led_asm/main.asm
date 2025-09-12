;
;main.asm
;
;     Created on: Sep 5, 2025
;     Author: Everton Marcelino Junior
;     Instituto Federal de Santa Catarina
;
;     This application tests blinking two LEDs with different intervals.
;     It uses the MSP-EXP430FR2355 launchpad.
;     This is the pinout used:
;
;     P1.0| --> Red LED
;     P6.6| --> Green LED
;

;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.
;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Inicializar stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Desativar WDT
			BIC.W   #1,&PM5CTL0
			bis.b 	#BIT0, &P1DIR
			bis.b 	#BIT6, &P6DIR

; Constantes
LED 		.equ BIT0
LED1 		.equ BIT6
DELAY 		.equ 100000
DELAY1 		.equ 75000
COUNTER		.equ R15

;-------------------------------------------------------------------------------
; Loop principal
;-------------------------------------------------------------------------------
MAIN		xor.b #LED, &P1OUT     ; comuta o LED
    		mov.w #DELAY, COUNTER  ; init. delay

L1			dec.w COUNTER	; Dec. contador
			jnz L1			; Delay terminado?
			jmp D2

D2			xor.b #LED1, &P6OUT
			mov.w #DELAY1, COUNTER  ; init. delay

L2			dec.w COUNTER
			jnz L2
			jmp MAIN


;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack

;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
