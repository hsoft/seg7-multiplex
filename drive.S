; Driver for seg7multiplexer
;
; Sends an increasing 4-digit figure through CLK and SER every second.

; Register usage
; R0: temporarily holds checksum during sending
; R16: generic tmp value
; R17: digit 1 (least significant)
; R18: digit 2 
; R19: digit 3
; R20: digit 4 (most significant)
; R22: digit to send when calling SEND
; R23: send loop counter

; Flags on GPIOs
; GPIOR0 - bit 0: Needs to send next digit group (every second)

.include "tn45def.inc"

; pins
.equ CLK = 0	; on PORTB
.equ SER = 1	; on PORTB

; Let's begin!

.org 0x0000
        RJMP    MAIN

MAIN:
	LDI	R16, low(RAMEND)
        OUT	SPL, R16
        LDI	R16, high(RAMEND)
	OUT	SPH, R16

	SBI	DDRB, CLK
	SBI	DDRB, SER

	; ATtiny's timer1's maximum prescaler is 1/16384. To reach one second,
	; we need 61 overflows, so we need a separate register to count
	; overflows. When it reaches 61, bingo.
	; XXX I seem to be having problems making simavr work with timer1.
	; it **looks** like a bug in simavr. Will look into it. Meanwhile, use
	; timer0 with a shorter delay
	LDI	R16, 0b00000101	; 1/1024 prescaler
	OUT	TCCR0B, R16

	; Starting digits
	LDI	R20, 5
	LDI	R19, 4
	LDI	R18, 3
	LDI	R17, 2

	SEI

LOOP:
	IN	R16, TIFR
	SBRS	R16, TOV0	; timer overflow flag set? skip next
	RJMP	LOOP

REFRESH:
	OUT	TIFR, R16	; reset TOV0

	; First, let's "wake up" the seg7
	RCALL	SENDCLK

	; While it wakes up, we will compute our resulting checksum and
	; increase our digits. That should take us around 50us. After that, we
	; have 150us left to kill before sending the data.
	INC	R17
	CPI	R17, 10
	BRNE	REFRESH1
	CLR	R17
	INC	R18
	CPI	R18, 10
	BRNE	REFRESH1
	CLR	R18
	INC	R19
	CPI	R19, 10
	BRNE	REFRESH1
	CLR	R19
	INC	R20
	CPI	R20, 10
	BRNE	REFRESH1
	CLR	R20

REFRESH1:
	MOV	R0, R17
	ADD	R0, R18
	ADD	R0, R19
	ADD	R0, R20

	RCALL	DELAY50US
	RCALL	DELAY50US
	RCALL	DELAY50US
	RCALL	DELAY50US

	MOV	R22, R20
	RCALL	SEND
	MOV	R22, R19
	RCALL	SEND
	MOV	R22, R18
	RCALL	SEND
	MOV	R22, R17
	RCALL	SEND
	MOV	R22, R0
	RCALL	SEND
	RJMP	LOOP

SEND:
	LDI	R23, 5		; we will loop 5 times
	CBI	PORTB, SER	; low
	SBRC	R22, 4		; if 5th bit isn't set, skip next
	SBI	PORTB, SER	; 5th bit set? set high!
	RCALL	SENDCLK
	RCALL	DELAY50US	; seg7 needs a little delay
	LSL	R22		; shift our data left
	DEC	R23
	BRNE	SEND+1		; not zero yet? loop! (+1 to avoid reset)
	RET

SENDCLK:
	SBI	PORTB, CLK
	NOP
	CBI	PORTB, CLK
	RET

DELAY50US:
	LDI	R16, 20
	DEC	R16
	BRNE	DELAY50US+1
	RET

