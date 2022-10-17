;***************************************************************************
;*                                                                         *
;*                              I F F   D I M                              *
;*                                                                         *
;*                              CONVERT MODULE                             *
;*                                                                         *
;***************************************************************************

	include	iffdim.i

	xref	_LVOOpen
	xref	_LVOClose
	xref	_LVORead
	xref	_LVOSeek

	xref	VDTDebugOutC
	xref	PrintMsg

	xref	Task,DosBase

	xref	MSGBadLevel
	xref	MSGBadList
	xref	MSGUserBreak

	xref	ListName,ListData,LevelData

	xref	MsgBuffer


	section	LEVELDIM,CODE

	xdef	DoLEVELDIM


;
;	* * * * * * *       LEVEL DIMENSIONS       * * * * * * *
;
DoLEVELDIM
;
;	>>>   READ IMAGELIST   <<<
;
	move.l	DosBase,a6				; Open LEVEL
	move.l	ListName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGBadList,d7
	move.l	d0,d4
	beq	DLM900
	move.l	d4,d1					; Read LEVEL
	move.l	ListData,d2
	move.l	#(32*1024),d3
	jsr	_LVORead(a6)
	move.l	d0,-(sp)
	move.l	d4,d1					; Close LEVEL
	jsr	_LVOClose(a6)
	move.l	ListData,a0				; Terminate with $00
	add.l	(sp)+,a0
	clr.b	(a0)
;
	move.l	ListData,a4				; A4 = LEVEL IMAGE LIST
	moveq.l	#0,d5					; D5 = TOTAL LEVEL SIZE
	moveq.l	#0,d6					; D6 = TOTAL RL SIZE
;
;	>>>   READ NEXT LEVEL FILE   <<<
;
DLM200
	move.l	#MSGUserBreak,d7			; Quit?
	move.l	Task,a0
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	DLM900					; Yes
;
	tst.b	(a4)					; ANY MORE FILES?
	beq	DLM800					; NO!
	moveq.l	#0,d0
DLM220
	move.b	(a4)+,d0				; Find Start of Name
	beq	DLM800
	cmp.b	#32,d0
	ble.s	DLM220
	lea	-1(a4),a3				; A3 = CURRENT LEVEL NAME
DLM240
	move.b	(a4)+,d0				; Find end of name
	beq	DLM800
	cmp.b	#32,d0
	bgt.s	DLM240
	clr.b	-1(a4)					; Terminate Name
;
	lea	LevelNames(pc),a2			; LevelNames
	moveq.l	#0,d3					; D3 = LEVEL TOTAL
	moveq.l	#0,d4					; D4 = LEVEL RL TOTAL
DLM400
	move.l	a2,a5
	tst.l	(a2)+					; FINISHED THIS LEVEL?
	beq	DLM700					; YES!
;
	move.l	a2,-(sp)
	move.l	a3,-(sp)
	move.l	sp,a1
	lea	LevelNameMsg(pc),a0
	jsr	VDTDebugOutC
	add.w	#(2*4),sp
DLM420
	tst.b	(a2)+					; Move to next Name
	bne.s	DLM420
	move.l	a2,d0
	and.l	#$1,d0
	beq.s	DLM430
	addq.w	#1,a2
DLM430
;
	move.l	DosBase,a6				; Open LEVEL FILE
	lea	MsgBuffer,a0
	move.l	a0,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGBadLevel,d7
	tst.l	d0
	beq	DLM900
;
	move.l	d0,d7
	move.l	d3,-(sp)
;
	move.l	d7,d1					; Move to end
	moveq.l	#0,d2
	move.l	#1,d3
	jsr	_LVOSeek(a6)
	move.l	d7,d1					; Move back to beginning
	moveq.l	#0,d2
	moveq.l	#-1,d3
	jsr	_LVOSeek(a6)				; D0 = SIZE OF FILE
	move.l	d0,-(sp)
	move.l	d7,d1					; Close LEVEL
	jsr	_LVOClose(a6)
;
	move.l	(sp)+,d2				; D2 = SIZE OF LEVEL FILE
	move.l	(sp)+,d3
;
	move.l	d2,d1					; Get RL Size
	move.l	(a5),d0					; Get Multiplier
	mulu.l	d0,d7:d1
	moveq.l	#16,d0
	lsr.l	d0,d1
	lsl.l	d0,d7
	or.l	d7,d1
;
;	>>>   NEXT FILE   <<<
;
	add.l	d2,d3					; LEVEL SIZE
	add.l	d1,d4					; RL SIZE
	bra	DLM400

;
;	>>>   NEXT LEVEL   <<<
;
DLM700
	move.l	d4,-(sp)
	move.l	d3,-(sp)
	move.l	a3,-(sp)
	move.l	sp,a1
	lea	LevelDimMsg(pc),a0
	jsr	VDTDebugOutC
	add.w	#(3*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	add.l	d4,d6					; TOTAL RL SIZE
	add.l	d3,d5					; TOTAL LEVEL SIZE
;
	bra	DLM200					; Process next Level

;
;	>>>   FINISHED CALCULATING AREA   <<<
;
DLM800
	move.l	d6,-(sp)				; TOTAL RL SIZE
	move.l	d5,-(sp)				; TOTAL LEVEL SIZE
	move.l	sp,a1
	lea	LevelDimMsg2(pc),a0
	jsr	VDTDebugOutC
	add.w	#(2*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	moveq.l	#0,d7					; No Error!
DLM900
	rts


LevelDimMsg	dc.b	'%s',9,9,'%7ld',9,'%7ld',10,0
LevelDimMsg2	dc.b	'TOTAL',9,9,'%7ld',9,'%7ld',10,0

LevelNameMsg	dc.b	'%s/%s',0

		dc.w	0

LevelNames
		dc.l	$00010000		; 100%
		dc.b	'VERTEXES',0

		dc.l	$00010000		; 100%
		dc.b	'NODES',0

		dc.l	$00010000		; 100%
		dc.b	'REJECT',0

		dc.l	$00012b85		; 117%
		dc.b	'SEGS',0

		dc.l	$00006e14		; 43%
		dc.b	'LINEDEFS',0

		dc.l	$00006b85		; 42%
		dc.b	'SECTORS',0

		dc.l	$0000cccc		; 80%
		dc.b	'THINGS',0

		dc.l	$0000c000		; 75%
		dc.b	'BLOCKMAP',0

		dc.l	$00014000		; 125%
		dc.b	'SSECTORS',0

		dc.l	$000035c2		; 21%
		dc.b	'SIDEDEFS',0

		dc.l	0			; END

	end
