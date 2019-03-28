; 7-segments multiplexer for an ATtiny45
;
; Register usage
; R0: Digit on AFF1 (rightmost, QH on the SR)
; R1: Digit on AFF2 (QG on the SR)
; R2: Digit on AFF3 (QF on the SR)
; R3: Digit on AFF4 (leftmost, QE on the SR)
; R5: always zero
; R6: generic tmp value
; R16: generic tmp value
; R18: value to send to the SR. cleared at every SENDSR call
;      in input mode, holds the input buffer
; R30: (low Z) current digit being refreshed. cycles from 0 to 3
; 
; Flags on GPIOs
; GPIOR0 - bit 0: Whether we need to refresh the display
; GPIOR0 - bit 1: Set when INT_INT0 has received a new bit
; GPIOR0 - bit 2: The value of the new bit received
; GPIOR0 - bit 4: input mode enabled

; Notes on register usage
; R0 - R3: 4 low bits are for digit, 5th bit is for dot. other bits are unused.
;
; Notes on AFF1-4
; They are reversed (depending on how you see things...). They read right to
; left. That means that AFF1 is least significant, AFF4 is most.
;
; Input mode counter
; When in input mode, TIMER0_OVF, instead of setting the refresh flag, increases
; the counter. When it reaches 3, we timeout and consider input invalid.
;
; Input procedure
;
; Input starts at INT_INT0. What it does there is very simple: is sets up a flag
; telling it received something and conditionally sets another flag with the
; value of the received bit.
;
; While we do that, we have the input loop eagerly checking for that flag. When
; it triggers, it records the bit in R18. The way it does so is that it inits
; R18 at 1 (not 0), then for every bit, it left shifts R18, then adds the new
; bit. When the 6th bit of R18 is set, it means we have every bit we need, we
; can flush it into Z.

; Z points directly to R3, then R2, then R1, then R0. Because display refresh
; is disabled during input, it won't result in weird displays, and because
; partial numbers result in error display, then partial result won't lead to
; weird displays, just error displays.
;
; When input mode begins, we change Z to point to R3 (the first digit we
; receive) and we decrease the Z pointer after every digit we receive. When we
; receive the last bit of the last digit and that we see that R30 is 0, we know
; that the next (and last) digit is the checksum.


.include "tn45def.inc"

; pins
.equ RCLK = 0	; on PORTB
.equ SRCLK = 3	; on PORTB
.equ SER_DP = 4	; on PORTB
.equ INSER = 1	; on PORTB

; Let's begin!

.org 0x0000
        RJMP    MAIN
	RJMP	INT_INT0
	RETI	; PCINT0
	RETI	; TIMER1_COMPA
	RETI	; TIMER1_OVF
	RJMP	INT_TIMER0_OVF

MAIN:
	LDI	R16, low(RAMEND)
        OUT	SPL, R16
        LDI	R16, high(RAMEND)
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
	
	; put 4321 in R2-5
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
	RCALL	INPT_CHK	; verify that we shouldn't enter input mode
	SBIC	GPIOR0, 0	; refesh flag cleared? skip next
	RCALL	RDISP
        RJMP    LOOP

; ***** DISPLAY *****

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
RDISP1:
	DEC	R6
	BREQ	RDISP2		; == 0? we're finished
	LSL	R16
	RJMP	RDISP1

	; select a digit to display
	; we do so in a clever way: our registers just happen to be in SRAM
	; locations 0x00, 0x01, 0x02 and 0x03. Handy eh!
RDISP2:
	LD	R18, Z+		; Indirect load of Z into R18 then increment
	CPI	R30, 4
	BRLO	RDISP3		; lower than 4 ? don't reset
	CLR	R30		; not lower than 4? reset

	; in the next step, we're going to join R18 and R16 together, but
	; before we do, we have one thing to process: R18's 5th bit. If it's
	; high, it means that DP is highlighted. We have to store this
	; information in R6 and use it later. Also, we have to clear the higher
	; bits of R18.
RDISP3:
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
	BRNE	SENDSR+1	; not zero yet? loop! (+1 to avoid reset)
	RET

; toggle SRCLK, waiting 1us between pin changes
TOGCP:
	CBI	PORTB, SRCLK	; low
	NOP			; At 1Mhz, this is enough for 1us
	SBI	PORTB, SRCLK	; high
	RET

; ***** INPUT MODE *****

; check whether we should enter input mode and enter it if needed
INPT_CHK:
	SBIS	GPIOR0, 1	; did we just trigger INT_INT0?
	RET			; no? return
	; yes? continue in input mode

; Initialize input mode and start the loop
INPT_BEGIN:
	SBI	GPIOR0, 4	; enable input mode
	CBI	GPIOR0, 1	; The first trigger was an empty one

	; At 1/8 prescaler, a "full" counter overflow is 2048us. That sounds
	; about right for an input timeout. So we co the easy route and simply
	; clear TCNT0 whenever we want to reset the timer
	OUT	TCNT0, R5	; R5 == 0
	CBI	GPIOR0, 0	; clear refresh flag in case it was just set
	LDI	R30, 0x04	; make Z point on R3+1 (we use pre-decrement)
	LDI	R18, 0x01	; initialize input buffer

; loop in input mode. When in input mode, we don't refresh the display, we use
; all our processing power to process input.
INPT_LOOP:	
	RCALL	INPT_READ

	; Check whether we've reached timeout
	SBIC	GPIOR0, 0	; refesh flag cleared? skip next
	RCALL	INPT_TIMEOUT

	SBIC	GPIOR0, 4	; input mode cleared? skip next, to INPT_END
	RJMP	INPT_LOOP	; not cleared? loop

INPT_END:
	; We received all our date or reached timeout. let's go back in normal
	; mode.
	CLR	R30		; Ensure Z isn't out of bounds
	SBI	GPIOR0, 0	; set refresh flag so we start refreshing now
	RET

; Read, if needed, the last received bit
INPT_READ:
	SBIS	GPIOR0, 1
	RET			; flag cleared? nothing to do

	; Flag is set, we have to read
	CBI	GPIOR0, 1	; unset flag
	LSL	R18
	SBIC	GPIOR0, 2	; data flag cleared? skip next
	INC	R18
	
	; Now, let's check if we have our 5 digits
	SBRC	R18, 5		; 6th bit cleared? nothing to do
	RCALL	INPT_PUSH

	OUT	TCNT0, R5	; clear timeout counter

	RET

; Push the digit currently in R18 in Z and reset R18.
INPT_PUSH:
	ANDI	R18, 0b00011111	; Remove 6th bit flag

	TST	R30		; is R30 zero?
	BREQ	INPT_CHECKSUM	; yes? it means we're at checksum phase.

	; Otherwise, its a regular digit push
	ST	-Z, R18
	LDI	R18, 0x01
	RET

INPT_CHECKSUM:
	CBI	GPIOR0, 4	; clear input mode, whether we error or not
	MOV	R16, R0
	ADD	R16, R1
	ADD	R16, R2
	ADD	R16, R3
	; only consider the first 5 bits of the checksum since we can't receive
	; more. Otherwise, we couldn't possibly validate a value like 9999
	ANDI	R16, 0b00011111	
	CP	R16, R18
	BRNE	INPT_ERROR
	RET

INPT_TIMEOUT:
	CBI	GPIOR0, 4	; timeout reached, clear input flag
	; continue to INPT_ERROR

INPT_ERROR:
	LDI	R16, 0x0c	; some weird digit
	MOV	R0, R16
	MOV	R1, R16
	MOV	R2, R16
	MOV	R3, R16
	RET

; ***** INTERRUPTS *****

; Record received bit
; The main loop has to be fast enough to process that bit before we receive the
; next one!
; no SREG fiddling because no SREG-modifying instruction
INT_INT0:
	CBI	GPIOR0, 2	; clear received data
	SBIC	PINB, INSER	; INSER clear? skip next
	SBI	GPIOR0, 2	; INSER set? record this
	SBI	GPIOR0, 1	; indicate that we've received a bit
	RETI

; Set refresh flag whenever timer0 overflows
; no SREG fiddling because no SREG-modifying instruction
INT_TIMER0_OVF:
	SBI	GPIOR0, 0
	RETI

