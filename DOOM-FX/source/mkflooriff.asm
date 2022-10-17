;***************************************************************************
;*                                                                         *
;*                            M A K E   F L O O R                          *
;*                                                                         *
;*                             IFF IMAGE MODULE                            *
;*                                                                         *
;***************************************************************************

	include	mkfloor.i

	xref	SetPicPlanes

	xref	DosBase
	xref	_LVOOpen
	xref	_LVOClose
	xref	_LVOSeek
	xref	_LVORead
	xref	_LVOWrite
	xref	_LVOAllocMem,_LVOFreeMem

	xref	IFFMode,IFFMask,IFFComp,IFFCompLen,IFFCompData

	xref	PicY,PicYBytes
	xref	PicX,PicXBytes,PicXNBytes
	xref	PicNumPlanes,PicPlanes
	xref	PicIFF,PicIFFSize
	xref	DoomPalette
	xref	PicUCopList,PicFIB


	section	IFF,CODE

	xdef	FindIFFChunk,DrawIFFPic,WriteIFF


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


;
;	* * * * * * *       GET IFF BYTE       * * * * * * *
;
;	A2 = IFF Data Pointer
;
;	D0 = IFF Byte returned
;
GetIFFByte
	tst.b	IFFComp						; IFF Compression ON?
	beq	GIB900						; No
	move.b	IFFCompLen,d0				; Any pending length?
	bmi	GIB100
	beq	GIB200
	subq.b	#1,d0						; LEN > 0 = LITERAL RUN
	move.b	d0,IFFCompLen
	move.b	(a2)+,d0
	rts
GIB100
	addq.b	#1,d0						; LEN < 0 = REPEAT RUN
	move.b	d0,IFFCompLen
	move.b	IFFCompData,d0
	rts
GIB200
	move.b	(a2)+,d0					; LEN = 0 = NEXT RUN
	cmp.b	#$80,d0						; NOP?
	beq	GIB200
	move.b	d0,IFFCompLen
	tst.b	d0
	bmi	GIB300
	move.b	(a2)+,d0					; Get LITERAL Byte
	rts
GIB300
	move.b	(a2)+,d0					; Get REPEATED Byte
	move.b	d0,IFFCompData
	rts
GIB900
	move.b	(a2)+,d0					; Return RAW value
	rts


;
;	* * * * * * *       DRAW IFF PICTURE       * * * * * * *
;
;	A2 = IFF Data
;	A5 = PlanePtrs
;	D5 = #Planes
;	D6 = #Bytes
;	D7 = #ScanLines
;
DrawIFFPic
	move.l	#'BODY',d0					; >>>BODY<<<
	bsr		FindIFFChunk
	bne	DIP990						; Error!
	addq.w	#4,a2						; Skip LENGTH of BODY Chunk
	clr.b	IFFCompLen					; No Compressed Data Pending
;
;	>>>   NEXT LINE   <<<
;
DIP200
	tst.b	IFFMode						; ILBM/PBM?
	bne	DIP400						; PBM
;
;	>>>   ILBM   <<<
;
	move.w	d5,d2						; D2 = PlaneCounter
	move.l	a5,a1						; A1 = PlanePtr
DIP300
	move.l	(a1),a0						; A0 = Destination Plane
DIP310
	move.w	d6,d1						; D1 = ByteCounter
	tst.w	d2						; Doing MASK Plane?
	bmi	DIP340						; Yes
	move.l	a0,d0						; Invalid Plane?
	beq	DIP340						; Yes
;
;	>>>   REGULAR PLANE   <<<
;
DIP320
	bsr		GetIFFByte					; Get Next IFF Byte
	move.b	d0,(a0)+
	dbf		d1,DIP320					; Next Byte
	bra	DIP380						; Finished this line
;
;	>>>   MASK PLANE / INVALID PLANE   <<<
;
DIP340
	bsr		GetIFFByte					; Get Next IFF Byte
	dbf		d1,DIP340					; Next Byte
	tst.w	d2							; Doing MASK Plane?
	bmi	DIP800						; Yes, done!
	bra	DIP390
;
;	>>>   MOVE TO NEXT SCANLINE OF REGULAR PLANE   <<<
;
DIP380
;	move.l	d6,d0						; Move to next PlaneLine
;	addq.l	#1,d0
	move.l	#(320/8),d0
	add.l	d0,(a1)+
;
;	>>>   DO NEXT PLANE   <<<
;
DIP390
	dbf		d2,DIP300					; Next Plane
	tst.b	IFFMask						; MASK?
	bne	DIP310						; Yes, skip data for a single plane
	bra	DIP800
;
;	>>>   PBM   <<<
;
DIP400
	move.w	d6,d1						; D1 = PixelCounter
	moveq.l	#8-1,d3						; D3 = Mod8 PixelCount
DIP420
	bsr		GetIFFByte					; Get Next IFF Byte
	moveq.l	#0,d2						; D2 = PlaneCounter
	move.l	a5,a1						; A1 = PlanePtr
DIP430
	tst.l	(a1)						; Any plane here?
	beq	DIP450						; No
	btst	d2,d0						; Bit Set?
	beq	DIP440						; No
	move.l	(a1),a0						; A0 = PicPlane
	bset	d3,(a0)
DIP440
	tst.b	d3							; At end of byte?
	bne	DIP450
	addq.l	#1,(a1)						; Yes, move over one
DIP450
	addq.w	#4,a1						; Move to next plane
	addq.w	#1,d2
	cmp.w	#8,d2
	bne	DIP430						; Next Plane Bit
;
;	>>>   NEXT PIXEL   <<<
;
	subq.b	#1,d3						; Next Pixel Over
	bpl	DIP420
	and.b	#$7,d3
	dbf		d1,DIP420					; Next Byte
;
;	>>>   NEXT SCANLINE   <<<
;
DIP800
	dbf		d7,DIP200					; Next Line
	moveq.l	#-1,d0						; No Problems!
	rts
DIP990
	moveq.l	#0,d0						; Error!
	rts


;
;	* * * * * * *       WRITE OUT IFF       * * * * * * *
;
;	D1 = Output Name
;
WriteIFF
	move.l	DosBase,a6
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4					; Can't Open It!
	beq	WIF900
	jsr	SetPicPlanes				; Set PicPlanes
	bsr	WriteIFFFORM				; FORM ILBM
	bsr	WriteIFFBMHD				; BMHD
	bsr	WriteIFFCMAP				; CMAP
	bsr	WriteIFFBODY				; BODY
	move.l	d7,d0
	and.l	#1,d0
	beq	WIF800
	addq.l	#1,d7					; PAD to word size!
	move.l	d4,d1
	moveq.l	#1,d3
	jsr	_LVOWrite(a6)
WIF800
	bsr	WriteIFFFORMAgain
	bsr	WriteIFFBODYAgain
	move.l	d4,d1
	jsr	_LVOClose(a6)
WIF900
	rts

;
;	* * * * * * *       WRITE IFF FORM       * * * * * * *
;
WriteIFFFORM
	move.l	d4,d1					; Save FORM
	move.l	#IFFFORM,d2
	move.l	#12,d3
	jmp	_LVOWrite(a6)
WriteIFFFORMAgain
	move.l	d4,d1					; Move to end
	moveq.l	#0,d2
	move.l	#1,d3
	jsr	_LVOSeek(a6)
	move.l	d4,d1					; Move back to beginning+4
	moveq.l	#4,d2
	moveq.l	#-1,d3
	jsr	_LVOSeek(a6)				; D0 = SIZE OF FILE
	lea	IFFFORM+4,a0				; Update size of form
	subq.l	#8,d0
	move.l	d0,(a0)
	move.l	a0,d2
	move.l	d4,d1					; Save FORM
	moveq.l	#4,d3
	jmp	_LVOWrite(a6)
WriteIFFBODYAgain
	move.l	d4,d1					; Move back to beginning
	move.l	#(12+28+8+(256*3)+4),d2
	moveq.l	#-1,d3
	jsr	_LVOSeek(a6)				; D0 = SIZE OF FILE
	lea	IFFBODY+4,a0				; Update size of BODY
	move.l	d7,(a0)
	move.l	d4,d1
	move.l	a0,d2
	moveq.l	#4,d3
	jmp	_LVOWrite(a6)

;
;	* * * * * * *       WRITE IFF BMHD       * * * * * * *
;
WriteIFFBMHD
	lea	IFFBMHD,a0				; Save BMHD
	move.w	PicX,0+8(a0)
	move.w	PicY,2+8(a0)
	move.l	d4,d1
	move.l	a0,d2
	move.l	#28,d3
	jmp	_LVOWrite(a6)

;
;	* * * * * * *       WRITE IFF CMAP       * * * * * * *
;
WriteIFFCMAP
	lea	IFFCMAP+8,a0				; A0 = CMAP Pointer
	move.l	DoomPalette,a1				; A1 = Palette Pointer
	move.l	#256-1,d2
WICP200
	move.b	(a1)+,(a0)+				; RED
	move.b	(a1)+,(a0)+				; GREEN
	move.b	(a1)+,(a0)+				; BLUE
	dbf	d2,WICP200
	move.l	d4,d1					; Save CMAP
	move.l	#IFFCMAP,d2
	move.l	#8+(256*3),d3
	jmp	_LVOWrite(a6)

;
;	* * * * * * *       WRITE IFF BODY       * * * * * * *
;
;	D4 = FIB
;
WriteIFFBODY
	move.l	d4,d1					; Save BODY
	move.l	#IFFBODY,d2
	move.l	#8,d3
	jsr	_LVOWrite(a6)
	move.l	4,a6					; exec.library
	move.l	#(16*8*32),d0				; This many pixels wide maximum
	moveq.l	#0,d1
	jsr	_LVOAllocMem(a6)
	move.l	DosBase,a6				; dos.library
	tst.l	d0
	beq	WIBY900
	lea	IFFPicRow,a0
	move.l	d0,(a0)
;
	lea	IFFPicY,a0				; Start at top line
	clr.w	(a0)
	moveq.l	#0,d7					; D7 = BODY Size
WIBY200
	lea	PicPlanes,a5				; A5 = PlanePointer
	moveq.l	#8-1,d5
WIBY300
	move.l	(a5),a0					; A0 = SourceRow
	bsr	PackBODYRow				; Pack a single body row
	move.l	a1,(a5)+				; A1 = Updated RowPointer
	move.l	d4,d1
	move.l	IFFPicRow,d2
	add.l	d3,d7					; Add To Total BODY Size
	jsr	_LVOWrite(a6)
	dbf	d5,WIBY300				; Next Plane
WIBY700
	lea	IFFPicY,a0				; Next Line
	move.w	(a0),d0
	addq.w	#1,d0
	move.w	d0,(a0)
	cmp.w	PicY,d0
	bne	WIBY200
WIBY900
	move.l	4,a6					; exec.library
	lea	IFFPicRow,a0				; COMPRESSED ROW
	tst.l	(a0)
	beq	WIBY920
	move.l	(a0),a1
	clr.l	(a0)
	move.l	#(16*8*32),d0				; This many pixels wide maximum
	jsr	_LVOFreeMem(a6)
	move.l	DosBase,a6				; dos.library
WIBY920
	rts
;
;	* * * * * * *       PACK BODY ROW       * * * * * * *
;
;	A0 = Source Row
;
PackBODYRow
	move.l	IFFPicRow,a4				; A4 = Destination
	move.l	a0,a2					; A2 = IP (Start of LITERAL)
	move.l	a0,a3					; A3 = IQ (End+1 of LITERAL)
	move.l	a0,a1					; A1 = End of Source
	add.w	PicXBytes,a1
;
;	LITERAL RUN
;
CAS1
	move.l	a3,a0					; A0 = PT (Start of REPLICATES)
	move.b	(a3)+,d3				; Character
	cmp.l	a3,a1					; At End of Input?
	beq	CAS5
	move.l	a3,d1					; Check for maximum overflow
	sub.l	a2,d1
	cmp.l	#128,d1
	beq	CAS6
	cmp.b	(a3),d3					; Next character same?
	bne	CAS1					; No!
;
;	AT LEAST 2 BYTE REPEAT
;
CAS2
	move.b	(a3)+,d3				; Get next character
	cmp.l	a3,a1					; End of Input?
	beq	CAS7
	move.l	a3,d1					; Check for maximum overflow
	sub.l	a2,d1
	cmp.l	#128,d1
	beq	CAS6
	cmp.b	(a3),d3					; Next character same?
	bne	CAS1					; No!
;
;	REPLICATE RUN
;
CAS3
	move.b	(a3)+,d3				; Get next character
	cmp.l	a3,a1					; End of Input?
	beq	CAS7					; Yes
	move.l	a3,d1					; Check for maximum overflow
	sub.l	a2,d1
	cmp.l	#128,d1
	beq	CAS4
	cmp.b	(a3),d3					; Next character same?
	beq	CAS3					; Yes
;
;	DUMP LITERAL/REPEAT AND CONTINUE
;
CAS4
	move.l	a0,d2					; D2 = #Characters to output
	sub.l	a2,d2
	beq	C41					; It's a REPLICATE Run
	subq.l	#1,d2
	move.b	d2,(a4)+				; Save # Characters
C40
	move.b	(a2)+,(a4)+				; Copy LITERAL Characters
	dbf	d2,C40
C41
	move.l	a0,d2					; D2 = -#Characters to output
	sub.l	a3,d2
	addq.l	#1,d2
	move.b	d2,(a4)+
	move.b	d3,(a4)+
	move.l	a3,a2
	bra	CAS1
;
;	LITERAL DUMP AND QUIT
;
CAS5
	move.l	a3,d2					; D2 = #Characters
	sub.l	a2,d2
	subq.l	#1,d2
	move.b	d2,(a4)+
C50
	move.b	(a2)+,(a4)+
	dbf	d2,C50
	bra	CAS8
;
;	LITERAL DUMP AND CONTINUE
;
CAS6
	move.l	a3,d2					; D2 = #Characters
	sub.l	a2,d2
	subq.l	#1,d2
	move.b	d2,(a4)+
C60
	move.b	(a2)+,(a4)+
	dbf	d2,C60
	bra	CAS1
;
;	LITERAL / REPEAT DUMP AND FINISH
;
CAS7
	move.l	a0,d2					; D2 = #Characters
	sub.l	a2,d2
	beq	C71					; REPLICATE
	subq.l	#1,d2
	move.b	d2,(a4)+
C70
	move.b	(a2)+,(a4)+
	dbf	d2,C70
C71
	move.l	a0,d2
	sub.l	a3,d2
	addq.l	#1,d2
	move.b	d2,(a4)+
	move.b	d3,(a4)+
;
;	FINISHED
;
CAS8
	move.l	a4,d3					; D3 = #Characters to Write
	sub.l	IFFPicRow,d3
	rts



	section	__MERGED,DATA

;
;	* * * * * * *       IFF VARIABLES       * * * * * * *
;
IFFPicY		dc.w	0				; IFF PicY Counter
IFFPicRow	dc.l	0				; IFF Compressed ROW
IFFFORM		dc.b	'FORM'				; FORM
		dc.l	$00000000
		dc.b	'ILBM'
IFFBMHD		dc.b	'BMHD'				; BMHD
		dc.l	$00000014
		dc.w	0,0				; Width and Height
		dc.w	0,0
		dc.b	8,0,1,0
		dc.w	$FFFF
		dc.b	10,11
		dc.w	320,200
IFFCMAP		dc.b	'CMAP'				; CMAP
		dc.l	$00000300
		ds.b	(256*3)
IFFBODY		dc.b	'BODY'				; BODY
		dc.l	$00000000



	end
