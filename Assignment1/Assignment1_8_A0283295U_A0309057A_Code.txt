/*
 * asm_func.s
 *
 *  Created on: 7/2/2025
 *      Author: Hou Linxin
 */
  	.syntax unified
	.cpu cortex-m4
	.fpu softvfp
	.thumb

		.global asm_func

@ Start of executable code
.section .text

@ CG/[T]EE2028 Assignment 1, Sem 2, AY 2024/25
@ (c) ECE NUS, 2025

@ Write Student 1’s Name here: Loke Wan Ching (A0283295U)
@ Write Student 2’s Name here: Tan Kai Cong (A0309057A)

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
@ ...

@ write your program from here:

asm_func:
 	LDR R4, [R3]			@R4=F
	LDR R5, [R3,#4]			@R5=S

SUM_ENTRIES:				@assumes E=5, so can use loop unrolling to maximise pipelining
	LDR R6, [R1], #4
	LDR R8, [R1], #4
	LDR R9, [R1], #4
	LDR R10, [R1], #4
	LDR R11, [R1], #4
	ADD R9, R6
	ADD R9, R8
	ADD R9, R10
	ADD R9, R11				@R9=sum of all entries, now R6,8,10,11 can be reused

	MUL	R4, R5				@R4=F*S => Counter that decrements till end of building arr
	LDR R8, [R0]			@R8=current pointed elem in building arr to reg
	MOV R5, #12				@R5=Max no. of cars=12

DISTRIBUTION_LOOP:			@iterate until entry arr run out OR reach end of building arr
	LDR R6, [R2], #4		@R6= pointed elem of exit arr
	ADD R8, R9				@add entry cars to building cars, R8=sum of entry and curr building elem
	CMP R8, R5				@compare sum with max cars per section (assumed 12)
	BLS	SKIP_SECTION_OVERFLOW	@if sum <= max, jump to skip_esection_overflow

SECTION_OVERFLOW:
	SUB R9, R8, R5			@place the excess cars back in reg
	SUB R6, R5, R6			@subtract exiting cars from SECTION_MAX (assumes exiting <= SECTION_MAX)
	STR R6, [R3], #4		@store final value into result arr (memory)
	LDR R8, [R0,#4]!		@shift building arr ptr to next elem AND load that elem to reg

	SUBS R4, #1
	BEQ RETURN				@if arr length cnt is 0 means end of arr reached (means exiting arr also fully counted in), so exit program completely
	B DISTRIBUTION_LOOP		@repeat process without moving entry ptr

SKIP_SECTION_OVERFLOW:
	SUB R6, R8, R6			@subtract exiting cars from last calculated entry number (assumes exiting <= currently in building)
	STR R6, [R3], #4		@no more new entries, store last calc val in mem
	SUBS R4, #1
	BEQ RETURN

EXITING_LOOP:				@if here means entries ran out, but building & exit arr pointers not at end yet
	LDR R6, [R2], #4		@get exit_arr elem
	LDR R8, [R0,#4]!		@get building_arr elem
	SUB R6, R8, R6			@subtract exiting cars from current building number (assumes exiting <= currently in building)
	STR R6, [R3], #4
	SUBS R4, #1
	BNE EXITING_LOOP

RETURN:
	BX LR
