;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                         CONVERT IMAGE MODULE                            *
;*                                                                         *
;***************************************************************************

	include	ripdoom.i


	xref	PrintMsg,VDTDebugOutC
	xref	OpenPictureSNES,WriteIFF,ClosePicture
	xref	SetPicPlanes,SetPixReg,ReadDoomPalette

	xref	DosBase
	xref	_LVOOpen
	xref	_LVOClose
	xref	_LVOSeek
	xref	_LVORead
	xref	_LVOWrite

	xref	Task
	xref	DoomWADData,DoomPalette,ConvertImage,DoomReMapTable
	xref	DoomPaletteName,OutputName

	xref	PicX,PicY,PicXBytes,PicNumPlanes,PicPalAmiga
	xref	PicPlanes

	xref	ImageBGReMap

	xref	MSGUserBreak

	section	IMAGE,CODE

	xdef	DoConvertImage
	xdef	ConvertImageReMapBG


;
;	* * * * * * *       CONVERT DOOM IMAGE       * * * * * * *
;
DoConvertImage
	move.w	#320,PicX
	move.w	#(320/16*2),PicXBytes
	move.w	#200,PicY
	move.w	#8,PicNumPlanes
	jsr	ReadDoomPalette				; Read PLAYPAL
	bne	DCI900					; Error!
	jsr	OpenPictureSNES				; Open the picture
	bne	DCI900					; Error!
;
	move.l	DosBase,a6
	move.l	ConvertImage,d1				; Open Image
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	DCI800
	move.l	d4,d1					; Read Image
	move.l	DoomWADData,d2
	move.l	#131072,d3
	jsr	_LVORead(a6)
	move.l	d4,d1					; Close Image
	jsr	_LVOClose(a6)
;
	jsr	SetPicPlanes				; Set Picture Planes
;
	tst.b	ImageBGReMap				; ReMap BG from 0 to 255?
	beq.s	DCI180					; No
	jsr	ConvertImageReMapBG			; Yes!
DCI180
	move.l	DoomWADData,a4
	move.b	1(a4),d6				; D6 = #Columns of Image Data
	lsl.w	#8,d6
	move.b	(a4),d6
	addq.w	#8,a4
	move.l	a4,a3					; A3 = APTR to Offset Pointers
	move.l	d6,d0					; Move Past Offset Pointers
	lsl.l	#2,d0
	add.l	d0,a4					; A4 = APTR DataBlock
	moveq.l	#0,d4					; X Coordinate = 0
	moveq.l	#0,d7					; Tallest Strip
DCI200
	move.b	3(a3),d0				; Get Next Column Pointer
	lsl.l	#8,d0
	move.b	2(a3),d0
	lsl.l	#8,d0
	move.b	1(a3),d0
	lsl.l	#8,d0
	move.b	0(a3),d0
	addq.w	#4,a3
	add.l	DoomWADData,d0
	move.l	d0,a4
DCI300
	moveq.l	#0,d5
	move.b	(a4)+,d5				; Row#
	cmp.b	#$ff,d5
	beq	DCI500
	moveq.l	#0,d3
	move.b	(a4)+,d3				; #Pixels to draw
	subq.w	#1,d3
	addq.w	#1,a4
DCI400
	moveq.l	#0,d0
	move.b	(a4)+,d0
	lea	DoomReMapTable,a0			; REMAP Table
	move.b	(a0,d0.w),d0
	movem.l	d3/d7,-(sp)
	bsr	SetPixReg
	movem.l	(sp)+,d3/d7
	addq.w	#1,d5
	dbf	d3,DCI400
	cmp.l	d5,d7					; Taller Strip?
	bge.s	DCI420
	move.l	d5,d7
DCI420
	addq.w	#1,a4
	bra.s	DCI300
DCI500
	addq.w	#1,d4					; Next Column
	cmp.w	d4,d6
	bne.s	DCI200
	move.l	d7,d5
;
	move.l	d5,-(sp)
	move.l	d4,-(sp)
	lea	ImageDimMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(2*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	OutputName,d1				; Save Output File
	jsr	WriteIFF
DCI800
	jsr	ClosePicture
DCI900
	moveq.l	#0,d7
	rts


;
;	* * * * * * *       REMAP BACKGROUND       * * * * * * *
;
ConvertImageReMapBG
	lea	PicPlanes,a1			; A1 = PlanePointer
	move.w	PicNumPlanes,d7
	subq.w	#1,d7
CIRMB200
	tst.l	(a1)				; Any Plane Here?
	beq	CIRMB800			; No
	move.l	(a1)+,a0			; A0 = PlanePtr
	moveq.l	#0,d0				; D0 = #Lines
	move.w	PicY,d0
	moveq.l	#0,d1				; D1 = #Bytes per Line
	move.w	PicXBytes,d1
	mulu	d0,d1				; D1 = #Bytes per Plane
	lsr.l	#2,d1				; D1 = #Longs per Plane
	subq.w	#1,d1
	moveq.l	#-1,d0				; D0 = Fill LongWord
CIRMB300
	move.l	d0,(a0)+			; Fill Plane
	dbf	d1,CIRMB300
	dbf	d7,CIRMB200			; Next Plane
CIRMB800
	rts


;
;	* * * * * * *       TEXT MESSAGES       * * * * * * *
;
ImageDimMsg
	dc.b	'Image Dimensions (%ld,%ld)',10,0


	end
