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

	xref	MSGBadIFF
	xref	MSGBadList
	xref	MSGUserBreak

	xref	ListName,ListData,IFFData

	xref	MsgBuffer


	section	IFFDIM,CODE

	xdef	DoIFFDIM

;
;	* * * * * * *       IFF DIMENSIONS       * * * * * * *
;
DoIFFDIM
;
;	>>>   READ IMAGELIST   <<<
;
	move.l	DosBase,a6				; Open IFF
	move.l	ListName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGBadList,d7
	move.l	d0,d4
	beq	DIM900
	move.l	d4,d1					; Read IFF
	move.l	ListData,d2
	move.l	#(32*1024),d3
	jsr	_LVORead(a6)
	move.l	d0,-(sp)
	move.l	d4,d1					; Close IFF
	jsr	_LVOClose(a6)
	move.l	ListData,a0				; Terminate with $00
	add.l	(sp)+,a0
	clr.b	(a0)
;
	move.l	ListData,a4				; A4 = IFF IMAGE LIST
	moveq.l	#0,d5					; D5 = TOTAL AREA
;
;	>>>   READ NEXT IFF FILE   <<<
;
DIM200
	move.l	#MSGUserBreak,d7			; Quit?
	move.l	Task,a0
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	DIM900					; Yes
;
	tst.b	(a4)					; ANY MORE FILES?
	beq	DIM800					; NO!
	moveq.l	#0,d0
DIM220
	move.b	(a4)+,d0				; Find Start of Name
	beq	DIM800
	cmp.b	#32,d0
	ble.s	DIM220
	lea	-1(a4),a3				; A3 = CURRENT IFF NAME
DIM240
	move.b	(a4)+,d0				; Find end of name
	beq	DIM800
	cmp.b	#32,d0
	bgt.s	DIM240
	clr.b	-1(a4)					; Terminate Name
;
	move.l	DosBase,a6				; Open IFF
	move.l	a3,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGBadIFF,d7
	move.l	d0,d4
	beq	DIM900
	move.l	d4,d1					; Read IFF
	move.l	IFFData,d2
	move.l	#(4*1024),d3
	jsr	_LVORead(a6)
	move.l	d4,d1					; Close IFF
	jsr	_LVOClose(a6)
;
	move.l	IFFData,a2				; >>>BMHD<<<
	move.l	#'BMHD',d0
	jsr	FindIFFChunk
	bne	DIM900					; Error!
;
;	>>>   GET DIMENSIONS   <<<
;
	moveq.l	#0,d0					; Get X
	move.w	4(a2),d0
	moveq.l	#0,d1					; Get Y
	move.w	6(a2),d1
	move.l	d1,d2					; Get AREA
	mulu	d0,d2
;
	add.l	d2,d5					; TOTAL AREA
;
	move.l	d2,-(sp)
	move.l	d1,-(sp)
	move.l	d0,-(sp)
	move.l	a3,-(sp)
	move.l	sp,a1
	lea	IFFDimMsg(pc),a0
	jsr	VDTDebugOutC
	add.w	#(4*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
;	>>>   NEXT FILE   <<<
;
	bra	DIM200					; Process next File

;
;	>>>   FINISHED CALCULATING AREA   <<<
;
DIM800
	move.l	d5,-(sp)				; TOTAL AREA
	move.l	sp,a1
	lea	IFFDimMsg2(pc),a0
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	moveq.l	#0,d7					; No Error!
DIM900
	rts


;
;	* * * * * * *       FIND AN IFF CHUNK       * * * * * * *
;
;	A2 = IFF Data Block
;	D0 = Chunk ID to Find
;
FindIFFChunk
	move.l	4(a2),d2					; D2 = Length of IFF FORM
	add.w	#12,a2						; Skip FORM+Length+ILBM/PBM
	sub.l	#12,d2
FIC200
	subq.l	#4,d2
	beq	FIC900						; At end of file
	bmi	FIC900
	cmp.l	(a2)+,d0					; Same chunk?
	beq	FIC800						; Yes!
	move.l	(a2)+,d1					; D1 = Length
	addq.l	#1,d1
	and.l	#$fffffffe,d1
	sub.l	d1,d2
	add.l	d1,a2
	bra	FIC200
FIC800
	moveq.l	#0,d0
	rts
FIC900
	moveq.l	#-1,d0
	rts


IFFDimMsg	dc.b	'%s',9,9,'%3ldX,%3ldY',9,'%6ld',10,0
IFFDimMsg2	dc.b	'TOTAL',9,9,9,9,'%6ld',10,0

		dc.w	0


	end
