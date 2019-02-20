; 7-segments multiplexer for an ATtiny45
;
; Register usage
; R1: Digit on AFF1 (rightmost, QH on the SR)
; R2: Digit on AFF2 (QG on the SR)
; R3: Digit on AFF3 (QF on the SR)
; R4: Digit on AFF4 (leftmost, QE on the SR)
; R5: input mode counter
; R6: generic tmp value
; R16: generic tmp value
; R17: hold SREG during interrupts
; R18: value to send to the SR. cleared at every SENDSR call
; R30: (low Z) current digit being refreshed. cycles from 0 to 3
; 
; Flags on GPIOs
; GPIOR0 - bit 0: Whether we need to refresh the display
; GPIOR0 - bit 1: Whether we're in input mode

; Notes on register usage
; R1 - R4: 4 low bits are for digit, 5th bit is for dot. other bits are unused.
;
; Notes on AFF1-4
; They are reversed (depending on how you see things...). They read right to
; left. That means that AFF1 is least significant, AFF4 is most.
;
; Input mode counter
; When in input mode, TIMER0_OVF, instead of setting the refresh flag, increases
; the counter. When it reaches 3, we timeout and consider input invalid.

; I/O registers
.equ GPIOR0, 0x11
.equ DDRB, 0x17
.equ PORTB, 0x18
.equ TCNT0, 0x32
.equ TCCR0B, 0x33
.equ MCUCR, 0x35
.equ TIMSK, 0x39
.equ GIMSK, 0x3b
.equ SPL, 0x3d
.equ SPH, 0x3e
.equ SREG, 0x3f

; pins
.equ RCLK, 0	; on PORTB
.equ SRCLK, 3	; on PORTB
.equ SER_DP, 4	; on PORTB

; Misc
.equ RAMEND, 0x015f

; Let's begin!

.org 0x0000
        RJMP    MAIN
	RJMP	INT0
	RETI	; PCINT0
	RETI	; TIMER1_COMPA
	RETI	; TIMER1_OVF
	RJMP	TIMER0_OVF

MAIN:
	LDI	R16, lo8(RAMEND)
        OUT	SPL, R16
        LDI	R16, hi8(RAMEND)
	OUT	SPH, R16

	SBI	DDRB, RCLK
	SBI	DDRB, SRCLK
	SBI	DDRB, SER_DP

	; we generally keep SER_DP high to avoid lighting DP
	SBI	PORTB, SER_DP

	; target delay: 600us. At 1Mhz, that's 75 ticks with a 1/8 prescaler.
	LDI	R16, 0x02	; CS01, 1/8 prescaler
	OUT	TCCR0B, R16
	LDI	R16, 0xb5	; TOP - 75 ticks
	OUT	TCNT0, R16

	; Enable TIMER0_OVF
	IN	R16, TIMSK
	ORI	R16, 0x02	; TOIE0
	OUT	TIMSK, R16

	; Generate interrupt on rising edge of INT0
	IN	R16, MCUCR
	ORI	R16, 0b00000011	; ISC00 + ISC01
	OUT	MCUCR, R16
	IN	R16, GIMSK
	ORI	R16, 0b01000000	; INT0
	OUT	GIMSK, R16

	; we never use indirect addresses above 0xff through Z and never use
	; R31 in other situations. We can set it once and forget about it.
	CLR	R31	; high Z
	
	# put 4321 in R2-5
	CLR	R30	; low Z
	LDI	R16, 0x04
	ST	Z+, R16		; 4
	DEC	R16
	ST	Z+, R16		; 3
	DEC	R16
	ST	Z+, R16		; 2
	DEC	R16
	ORI	R16, 0b00010000	; DP
	ST	Z, R16		; 1
	CLR	R30		; replace Z to 0

	SEI

LOOP:
	SLEEP			; no use doing anything until we have an interrupt
	SBIC	GPIOR0, 1	; input mode cleared? skip next
	RJMP	LOOP_INPT	; input mode? jump!
	SBIC	GPIOR0, 0	; refesh flag cleared? skip next
	RCALL	RDISP
        RJMP    LOOP

; loop in input mode. When in input mode, we don't refresh the display, we use
; all our processing power to process input. Also, we don't sleep
LOOP_INPT:	
	SBIS	GPIOR0, 0	; refesh flag set? skip next
	RJMP	LOOP_INPT	; cleared? nothing to do
	CBI	GPIOR0, 0	; clear refresh flag
	INC	R5		; increase timeout

	; did we reached our threshold yet?
	SBRS	R5, 3		; bit 3 is set? threshold reached, skip RJMP
	RJMP	LOOP_INPT	; not reached? loop

	; We reached timeout. let's go back in normal mode
	CLR	R5
	CBI	GPIOR0, 1	; disable input mode
	
	; re-enable refresh flag so that we refresh right after timeout rather
	; than witing the next TIMER0_OVF.
	SBI	GPIOR0, 0
        RJMP    LOOP+2		; +2 so we skip SLEEP


; ***** ROUTINES *****

; refresh display with current number
RDISP:
	; First things first: setup the timer for the next time
	LDI	R16, 0xb5	; TOP - 75 ticks
	OUT	TCNT0, R16
	CBI	GPIOR0, 0	; Also, clear the refresh flag

	; Let's begin with the display selector. We select one display at once
	; (not ready for multi-display refresh operations yet). Let's decode our
	; binary value from R30 into R16.
	MOV	R6, R30
	INC	R6		; we need values 1-4, not 0-3
	LDI	R16, 0x01
	DEC	R6
	BREQ	.+4		; == 0? we're finished
	LSL	R16
	RJMP	.-8

	; select a digit to display
	; we do so in a clever way: our registers just happen to be in SRAM
	; locations 0x00, 0x01, 0x02 and 0x03. Handy eh!
	LD	R18, Z+		; Indirect load of Z into R18 then increment
	CPI	R30, 4
	BRLO	.+2		; lower than 4 ? don't reset
	CLR	R30		; not lower than 4? reset

	; in the next step, we're going to join R18 and R16 together, but
	; before we do, we have one thing to process: R18's 5th bit. If it's
	; high, it means that DP is highlighted. We have to store this
	; information in R6 and use it later. Also, we have to clear the higher
	; bits of R18.
	SBRC	R18, 4		; 5th bit cleared? skip next
	INC	R6		; if set, then set R6 as well
	ANDI	R18, 0xf	; clear higher bits

	; Now we have our display selector in R16 and our digit to display in
	; R18. We want it all in R18.
	SWAP	R18		; digit goes in high "nibble"
	OR	R18, R16

	; While we send value to the shift register, SER_DP will change.
	; Because we want to avoid falsely lighting DP, we need to disable
	; output (disable OE) while that happens. This is why we set RCLK,
	; which is wired to OE too, HIGH (OE disabled) at the beginning of
	; the SR operation.
	;
	; Because RCLK was low before, this triggers a "buffer clock" on
	; the SR, but it doesn't matter because the value that was there
	; before has just been invalidated.
	SBI	PORTB, RCLK	; high
	RCALL	SENDSR
	; Flush out the buffer with RCLK
	CBI	PORTB, RCLK	; OE enabled, but SR buffer isn't flushed
	NOP
	SBI	PORTB, RCLK	; SR buffer flushed, OE disabled
	NOP
	CBI	PORTB, RCLK	; OE enabled

	; We're finished! Oh no wait, one last thing: should we highlight DP?
	; If we should, then we should keep SER_DP low rather than high for this
	; SR round.
	SBI	PORTB, SER_DP	; SER_DP generally kept high
	SBRC	R6, 0		; R6 is cleared? skip DP set
	CBI	PORTB, SER_DP	; SER_DP low highlight DP

	RET			; finished for real this time!

; send R18 to shift register.
; We send highest bits first so that QH is the MSB and QA is the LSB
; low bits (QD - QA) control display's power
; high bits (QH - QE) select the glyph
SENDSR:
	LDI	R16, 8		; we will loop 8 times
	CBI	PORTB, SER_DP	; low
	SBRC	R18, 7	; if latest bit isn't cleared, set SER_DP high
	SBI	PORTB, SER_DP	; high
	RCALL	TOGCP
	LSL	R18		; shift our data left
	DEC	R16
	BRNE	SENDSR+2	; not zero yet? loop! (+2 to avoid reset)
	RET

; toggle SRCLK, waiting 1us between pin changes
TOGCP:
	CBI	PORTB, SRCLK	; low
	NOP			; At 1Mhz, this is enough for 1us
	SBI	PORTB, SRCLK	; high
	RET

; ***** INTERRUPTS *****

; Enable input mode
INT0:
	SBIC	GPIOR0, 1	; input mode off? continue further
	RETI			; on? do nothing
	IN	R17, SREG
	SBI	GPIOR0, 1	; input mode
	CLR	R5		; clear timeout counter
	OUT	SREG, R17
	RETI

; Set refresh flag whenever timer0 overflows
; no SREG fiddling because no SREG-modifying instruction
TIMER0_OVF:
	SBI	GPIOR0, 0
	RETI

