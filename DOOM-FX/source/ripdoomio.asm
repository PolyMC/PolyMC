;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                               I/O MODULE                                *
;*                                                                         *
;***************************************************************************

	include	ripdoom.i

	xref	_LVOWrite
	xref	_LVORawDoFmt

	xref	Msgs
	xref	argv
	xref	argc
	xref	OutputFIB
	xref	MsgBuffer
	xref	DosBase


	section	IO,CODE

	xdef	PrintMsg
	xdef	VDTDebugOutC
	xdef	ParseLine
	xdef	ToUpper
	xdef	GetArgs0
	xdef	ParseNum
	xdef	ParseArg


;
;	***   PRINT MESSAGE NUMBER IN (D7) TO OUTPUTFIB   ***
;
PrintMsg
	movem.l	d2-d3/a6,-(a7)
	lea	MsgBuffer,a0
	cmp.l	#-1,d7
	beq	PrintMsg300
	move.l	d7,a0
PrintMsg300
	move.l	a0,d2							; D2 = ADDRESS
	moveq.l	#-1,d3							; D3 = LENGTH
PrintMsg500
	addq.l	#1,d3
	tst.b	(a0)+
	bne	PrintMsg500
	move.l	OutputFIB,d1				; D1 = FIB
	move.l	DosBase,a6
	jsr		_LVOWrite(a6)					; Send it to the user
	movem.l	(a7)+,d2-d3/a6
	rts


;
;	***   STANDARD OUTPUT ROUTINE   ***
;
VDTDebugOutC
	movem.l	d0-d1/a0-a3/a6,-(a7)
	lea		VDTDebugOutChar,a2
	lea		MsgBuffer,a3
	move.l	4,a6
	jsr		_LVORawDoFmt(a6)
	movem.l	(a7)+,d0-d1/a0-a3/a6
	rts
VDTDebugOutChar
	move.b	d0,(a3)+
	rts


;
;	***   PARSE LINE   ***
;
;	GET ARGUMENTS FROM (A5)
;
;	RETURNS	D2 = NUMBER OF ARGUMENTS, ARGV ARRAY FILLED WITH POINTERS
;
ParseLine
	lea		argv,a0						; A1 points to argv table
	move.l	a0,a1
	clr.l	(a1)+							; Label
	clr.l	(a1)+							; OpCode/PseudoOpCode
	clr.l	(a1)+							; Parameter
	clr.l	(a1)+							; Comment
	move.l	a0,a1
	moveq.l	#0,d2							; D2 = Argument count
	moveq.l	#0,d3							; D3 = Column count
	moveq.l	#0,d4							; D4 = String Mode running
	move.b	(a5)+,d0
	and.b	#$7f,d0
	cmp.b	#';',d0							; Comment?
	beq	PLEnd100
	cmp.b	#9,d0							; TAB?
	beq	PLSkipCol
	cmp.b	#' ',d0							; SPACE?
	beq	PLSkipCol
	cmp.b	#13,d0							; EOL?
	blt	PLDone
	cmp.b	#$7f,d0
	bge	PLDone
PLNext
	lea		-1(a5),a5						; Move back to first character
	move.l	a5,(a1)
	addq.l	#1,d2
PLFix
	move.b	(a5)+,d0
	and.b	#$7f,d0
	cmp.b	#9,d0							; TAB?
	beq	PLSkipCol
	tst.b	d4
	bne		PLFix2
	cmp.b	#' ',d0							; SPACE?
	beq	PLSkipCol
PLFix2
	cmp.b	#' ',d0
	blt	PLDone
	cmp.b	#$22,d0							; QUOTE?
	beq		PLQuote
	cmp.b	#$27,d0
	bne		PLNotQuote
PLQuote
	tst.b	d4								; STRING mode on?
	bne		PLQuoteOn						; Yes
	move.b	d0,d5							; Save type of quote
	moveq.l	#1,d4							; STRING mode ON
	bra	PLNotQuote
PLQuoteOn
	cmp.b	d0,d5							; Ending quote?
	bne		PLNotQuote						; No, Not matching quote
	moveq.l	#0,d4							; STRING mode OFF
PLNotQuote
	cmp.b	#$7f,d0
	bge	PLDone
	cmp.b	#';',d0
	beq	PLEnd							; Stop if we hit comments
	tst.b	d4								; Quote MODE, don't CONVERT!
	bne		PLFix100
	bsr		ToUpper
PLFix100
	move.b	d0,-1(a5)
	bra	PLFix
PLSkipCol
	moveq.l	#0,d4							; Not in QUOTE mode
	addq.w	#4,a1							; Skip to next column
	addq.l	#1,d3
	cmp.l	#3,d3							; Have we got 3 arguments?
	beq	PLEnd100						; Yes, find end of line
	clr.b	-1(a5)
PLSkip
	move.b	(a5)+,d0
	and.b	#$7f,d0
	cmp.b	#9,d0
	beq	PLSkip							; Skip over TABs
	cmp.b	#' ',d0
	beq	PLSkip							; Skip over SPACEs
	blt	PLDone							; End of Line
	cmp.b	#$7f,d0
	bge	PLDone
	bra	PLNext
PLDone
	clr.b	-1(a5)
	rts
PLEnd
	clr.l	(a1)							; Comment Column doesn't exist
	subq.l	#1,d2							; One less column exists
PLEnd100
	move.b	(a5)+,d0						; Skip to end of line
	and.b	#$7f,d0
	beq	PLDone
	cmp.b	#';',d0
	bne	PLEnd200						; Found comment, skip it!
	clr.b	-1(a5)
	bra	PLEnd100
PLEnd200
	bsr		ToUpper							; Convert to UpperCase
	move.b	d0,-1(a5)
	cmp.b	#$0a,d0
	bne	PLEnd100						; Not end of line
	bra	PLDone							; Terminate end of line with $00

;
;	CONVERT TO UPPERCASE
;
ToUpper
	cmp.b	#'a',d0							; <a ?
	blt	ToUpper800
	cmp.b	#'z',d0
	bgt	ToUpper800						; >z ?
	sub.b	#$20,d0
ToUpper800
	rts


;
;	GET ARGUMENTS FROM (A0)
;
;	RETURNS	D2 = NUMBER OF ARGUMENTS, ARGV ARRAY FILLED WITH POINTERS
;
GetArgs0
	move.l	argc,a0						; A0 points to LineText
	lea		argv,a1						; A1 points to argv table
	moveq.l	#0,d2							; D2 = Argument count
	bra	NextArg
GetArgs
	move.b	(a0)+,d0						; D0 = Character
	cmp.b	#9,d0
	beq	NextArg
	cmp.b	#' ',d0							; Space?
	beq	NextArg
GetArgs100
	blt	GotArgs							; Non-ASCII, end of line
	cmp.b	#$7b,d0
	bge	GotArgs
	lea		-1(a0),a0						; A0 points to START of Arg
	move.l	a0,(a1)+						; Save address of argument
	addq.l	#1,d2							; One more argument
SkipArg
	move.b	(a0)+,d0						; Skip argument between spaces
	cmp.b	#9,d0
	beq	EndArg
	cmp.b	#' ',d0
	beq	EndArg
	blt	GotArgs
	cmp.b	#$7b,d0
	bge	GotArgs
	bsr		ToUpper							; Convert to UpperCase
	move.b	d0,-1(a0)						; Save corrected character
	bra	SkipArg
EndArg
	clr.b	-1(a0)							; Set up $00 between args
NextArg
	move.b	(a0)+,d0						; Skip spaces between args
	cmp.b	#9,d0
	beq	NextArg
	cmp.b	#' ',d0
	beq	NextArg
	bgt	GetArgs100						; Get next argument
GotArgs
	clr.b	-1(a0)							; Set up $00 between args
	clr.l	(a1)							; Terminate list with $0
	rts

;
;	* * * * * * *       PARSE NUMBER       * * * * * * *
;
;	(A0) = Numerical TextString
;
ParseNum
	moveq.l	#0,d2
PNM200
	moveq.l	#0,d0
	move.b	(a0)+,d0					; Get next character
	beq	PNM900						; EOT?
	cmp.b	#10,d0
	beq	PNM900
	cmp.b	#'0',d0
	blt	PNM800
	cmp.b	#'9',d0
	bgt	PNM800
	sub.b	#'0',d0
	mulu	#10,d2
	add.l	d0,d2
	bra	PNM200
PNM800
	cmp.b	#',',d0
	beq	PNM900
	moveq.l	#-1,d0
PNM900
	rts


;
;	* * * * * * *       PARSE ARGUMENT       * * * * * * *
;
;	(A0) points to Argument Line, scans and finds next argument
;
ParseArg
	moveq.l	#0,d0				; PreClear upper byte
PAG200
	move.b	(a0)+,d0			; Try to find Seperator
	beq	PAG800				; EOL
	cmp.b	#' ',d0				; Spaces,TABS,etc?
	ble	PAG200
	cmp.b	#',',d0				; Comma?
	beq	PAG200
	subq.w	#1,a0				; Move to first character
PAG800
	rts


	end
