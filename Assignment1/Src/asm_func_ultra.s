/*
 * asm_func_ultra.s
 *
 *  Created on: 22/2/2025
 *
 *	Key differences from asm_func.s:
 *      - flexible F,S,E,M
 *		- return flags byte which contains bit flags if F=0 or S=0, E=0, entering cars overflow, exiting cars overflow
 */
  	.syntax unified
	.cpu cortex-m4
	.fpu softvfp
	.thumb

		.global asm_func_ultra

@ Start of executable code
.section .text

@ CG/[T]EE2028 Assignment 1, Sem 2, AY 2024/25
@ (c) ECE NUS, 2025

@ Write Student 1’s Name here: Loke Wan Ching
@ Write Student 2’s Name here: Tan Kai Cong

@ Look-up table for registers:
@ R0 = building arr addr
@ R1 = entry arr addr
@ R2 = exit arr addr
@ R3 = result arr addr
@ R4 = counter to count down to end of arr (holds no. of elems in arr at first)
@ R5 = 12 i.e. max cars per section
@ R6 = current exit elem & used to hold final value to be stored
@ R7 used as default frame pointer
@ R8 = current building element
@ R9 = sum of entries / excess cars to be distributed
@ R10 = error flags (bit 0:F=0,S=0; bit 1: E=0; bit 2:entering overflow; bit 3: exiting cars > building cars

@ write your program from here:

asm_func_ultra:
	@catch all for any F=0 or S=0 (if building, exit or result arr addresses are same, then F and/or S must be 0)
 	CMP R0, R2
 	ITTE NE
	CMPNE R0, R3			@continue comparing other addresses
 	CMPNE R2, R3
 	ORREQ R10, R10, #1		@set bit 0 if any of the addresses equal each other
 	BEQ RETURN

 	LDR R4, [R3]			@R4=F
	LDR R5, [R3, #4]		@R5=S
	MUL R4, R5				@R4=F*S, R5 can be reused
	LDR R6, [R3, #8]		@R6=E => Counter to decrement till end of entry arr
	LDR R5, [R3, #12]		@R5=M

	CMP R6, #0				@if E=0, go directly to exit loop
	ITTT EQ
		LDREQ R8, [R0], #-4
		ORREQ R10, #2
		BEQ EXITING_LOOP

	LDR R8, [R1], #4
	MOV R9, #0				@R9=entry elems
	MOV R10, #0				@R10=error flags

SUM_ENTRIES:
	ADD R9, R8
	LDR R8, [R1], #4
	SUBS R6, #1
	BNE SUM_ENTRIES			@loop until end of entry arr, R6&8 can be reused

	LDR R8, [R0]			@R8=current pointed elem in building arr to reg

DISTRIBUTION_LOOP:			@iterate until entry arr run out OR reach end of building arr
	LDR R6, [R2], #4		@R6=pointed elem of exit arr
	ADD R8, R9				@add entry cars to building cars, R8=sum of entry and curr building elem
	CMP R8, R5				@compare sum with max cars per section
	BLS	SKIP_SECTION_OVERFLOW	@if sum <= max, jump to skip_entry_overflow

SECTION_OVERFLOW:
	SUB R9, R8, R5			@place the excess cars back in reg
	SUBS R6, R5, R6			@subtract exiting cars from SECTION_MAX
	ITT MI
		MOVMI R6, #0		@zero out entry if exiting amt > building amt
		ORRMI R10, #8		@set bit 3 to declare exiting overflow
	STR R6, [R3], #4		@store final value into result arr (memory)
	LDR R8, [R0,#4]!		@shift building arr ptr to next elem AND load that elem to reg

	SUBS R4, #1				@if 0 means end of arr but still have excess cars, so section overflowed
	BNE DISTRIBUTION_LOOP
	ORR R10, #4				@set bit 2 to declare entering cars overflowed
	B RETURN

SKIP_SECTION_OVERFLOW:
	SUBS R6, R8, R6			@subtract exiting cars from last calculated entry number
	ITT MI
		MOVMI R6, #0		@zero out entry if exiting amt > building amt
		ORRMI R10, #8		@set bit 3 to declare exiting overflow
	STR R6, [R3], #4		@no more new entries, store last calc val in mem
	SUBS R4, #1
	BEQ RETURN

EXITING_LOOP:				@if here means entries ran out, but building & exit arr pointers not at end yet
	LDR R6, [R2], #4		@get exit_arr elem
	LDR R8, [R0,#4]!		@get building_arr elem
	SUBS R6, R8, R6
	ITT MI
		MOVMI R6, #0
		ORRMI R10, #8		@set bit 3 to declare exiting overflow
	STR R6, [R3], #4
	SUBS R4, #1
	BNE EXITING_LOOP
	B RETURN

RETURN:
	MOV R0, R10
	BX LR
