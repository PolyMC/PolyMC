;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                            PICTURE MODULE                               *
;*                                                                         *
;***************************************************************************

	include	ripdoom.i


	xref	DrawIFFPic,FindIFFChunk

	xref	IntuitionBase,GraphicsBase,DosBase
	xref	PicScreen,PicWindow,PicVPort,PicRPort
	xref	PicPlanesOffset
	xref	NewSNESPicScreen,NewSNESPicScreenTags
	xref	NewSNESPicWindow,NewSNESPicWindowScreen

	xref	IFFMode,IFFMask,IFFComp

	xref	_LVOOpenScreen,_LVOCloseScreen
	xref	_LVOOpenWindow,_LVOCloseWindow
	xref	_LVOSetDrMd,_LVOSetAPen,_LVOMove,_LVODraw,_LVORectFill
	xref	_LVOModifyIDCMP
	xref	_LVOOpen,_LVOClose,_LVORead,_LVOSeek
	xref	_LVOAllocMem,_LVOFreeMem
	xref	_LVOOpenScreenTagList,_LVOLoadRGB4


	section	PIC,CODE

	xdef	OpenPictureSNES,PictureLoadSNES,ClosePicture,SetPicPlanes


;
;	* * * * * * *       OPEN SNES PICTURE       * * * * * * *
;
OpenPictureSNES
	move.l	IntuitionBase,a6		; intuition.library
	move.w	PicY,d1
	add.w	#11,d1
	move.w	PicX,d2
	move.w	PicNumPlanes,d3
	lea	NewSNESPicScreen,a0
	move.w	d1,6(a0)
	move.w	d2,4(a0)
OPSNS200
	move.w	d3,8(a0)
	lea	NewSNESPicScreenTags,a1
	jsr	_LVOOpenScreenTagList(a6)
	tst.l	d0				; Did the screen open?
	bne	OPSNS300			; Yes
	lea	NewSNESPicScreen,a0
	move.w	8(a0),d3			; D3 = #Planes
	subq.w	#1,d3
	bne	OPSNS200			; Try one less plane!
	bra	OPSNS800
OPSNS300
	move.l	d0,PicScreen
;
	lea	PicPlanes2,a2			; A2 = PicPlanes from AllocMem
	move.w	d3,d0
	lsl.w	#2,d0
	add.w	d0,a2
OPSNS400
	cmp.w	PicNumPlanes,d3			; Got all the planes we wanted?
	beq	OPSNS500			; Yes
	moveq.l	#0,d0				; D0 = #Bytes per plane
	move.w	PicY,d0
	add.w	#11,d0
	mulu	PicX,d0
	lsr.l	#3,d0
	move.l	a6,-(sp)
	move.l	4,a6
	move.l	#(MEMF_CLEAR!MEMF_PUBLIC),d1
	jsr	_LVOAllocMem(a6)
	move.l	(sp)+,a6
	tst.l	d0
	beq	OPSNS800			; Can't get the memory!
	move.l	d0,(a2)+			; Save PicPlanes2 Pointer
	addq.w	#1,d3				; One more plane added
	bra	OPSNS400
OPSNS500
	move.l	PicScreen,d0
	move.l	d0,NewSNESPicWindowScreen
	add.l	#$2c,d0
	move.l	d0,PicVPort
;
	move.l	GraphicsBase,a6			; Load 256 Colours
	move.l	d0,a0
	lea	PicPalAmiga,a1
	move.l	#256,d0
	jsr	_LVOLoadRGB4(a6)
;
	move.l	IntuitionBase,a6
	lea	NewSNESPicWindow,a0
	move.l	NewSNESPicWindowScreen,a1
	move.w	$c(a1),d0
	move.w	$e(a1),d1
	move.w	d0,4(a0)
	sub.w	#11,d1
	move.w	d1,6(a0)
	jsr	_LVOOpenWindow(a6)
	tst.l	d0
	beq	OPSNS800
	move.l	d0,PicWindow
	move.l	d0,a0
	move.l	$32(a0),PicRPort
;
	moveq.l	#0,d0
	rts
OPSNS800
	bsr	ClosePicture			; Close Anything Opened!
	moveq.l	#-1,d0
	rts


;
;	* * * * * * *       SET PIC PLANES       * * * * * * *
;
;	A0 = END of PicPlanes
;	A1 = END of PicPlanes2
;
SetPicPlanes
	lea	PicPlanes,a0			; A0 = PlanePointer from Screen
	lea	PicPlanes2,a1			; A1 = PlanePointer from AllocMem
	move.l	PicScreen,a2			; A2 = Screen
	move.l	$58(a2),a2			; A2 = RPort->BitMap
	addq.w	#4,a2
	move.w	(a2)+,d3			; D3 = #Valid Planes
	and.w	#$00ff,d3
	addq.w	#2,a2				; A2 = PLANES
	moveq.l	#0,d1
	move.w	PicXBytes,d1			; ScanLine Width
	mulu	#11,d1				; 11 lines down
	moveq.l	#8-1,d2				; D2 = #Planes
SPPS200
	tst.w	d3				; Any More Valid Planes?
	beq	SPPS250				; No
	move.l	(a2)+,d0			; D0 = PlanePtr from Screen
	subq.w	#1,d3				; One less plane
	bra	SPPS300
SPPS250
	move.l	(a1),d0				; D0 = PlanePtr from AllocMem
	beq	SPPS400				; Plane not needed!
SPPS300
	add.l	d1,d0				; Add Offset to Window
	add.l	PicPlanesOffset,d0		; Add Planes Offset
SPPS400
	move.l	d0,(a0)+			; Save PLANEPTR
	addq.w	#4,a1
	dbf	d2,SPPS200
	rts


;
;	* * * * * * *       CLOSE PICTURE       * * * * * * *
;
ClosePicture
	move.l	4,a6					; Release the Memory
	lea		PicPlanes2,a2			; A2 = PicPlanes from AllocMem
	moveq.l	#8-1,d3						; D3 = Planes to scan through
CPE400
	move.l	(a2),d0						; D0 = PlanePtr
	clr.l	(a2)+
	tst.l	d0
	beq	CPE420						; No plane here
	move.l	d0,a1						; Release the plane
	moveq.l	#0,d0						; D0 = #Bytes per plane
	move.w	PicY,d0
	add.w	#11,d0
	mulu	PicX,d0
	lsr.l	#3,d0
	jsr		_LVOFreeMem(a6)
CPE420
	dbf		d3,CPE400					; Next Plane
CPE500
	move.l	GraphicsBase,a6			; graphics.library
	move.l	IntuitionBase,a6		; intuition.library
	lea		PicWindow,a1			; Window Open?
	tst.l	(a1)
	beq	CPE700
	move.l	(a1),a0
	clr.l	(a1)
	clr.l	$56(a0)
	jsr		_LVOCloseWindow(a6)
CPE700
	lea		PicScreen,a1			; Screen Open?
	tst.l	(a1)
	beq	CPE800
	move.l	(a1),a0
	clr.l	(a1)
	jsr		_LVOCloseScreen(a6)
CPE800
	rts


;
;	* * * * * * *       LOAD A SNES PICTURE       * * * * * * *
;
;	D1 = Name of PICTURE to load
;
PictureLoadSNES
;
;	>>>   READ THE IFF FILE   <<<
;
	move.l	DosBase,a6				; dos.library
	move.l	#1005,d2					; MODE_OLDFILE
	jsr		_LVOOpen(a6)
	tst.l	d0
	beq	PLSNS900					; File doesn't exist!
	lea		PicFIB,a0				; Save FIB
	move.l	d0,(a0)
	move.l	d0,d1						; Move to end
	moveq.l	#0,d2
	move.l	#1,d3
	jsr		_LVOSeek(a6)
	move.l	PicFIB,d1				; Move back to beginning
	moveq.l	#0,d2
	moveq.l	#-1,d3
	jsr		_LVOSeek(a6)				; D0 = SIZE OF FILE
	lea		PicIFFSize,a0
	move.l	d0,(a0)
	move.l	4,a6
	moveq.l	#0,d1
	jsr		_LVOAllocMem(a6)
	tst.l	d0
	beq	PLSNS900					; Error!
	lea		PicIFF,a0
	move.l	d0,(a0)
	move.l	DosBase,a6				; Read the entire file in
	move.l	PicFIB,d1
	move.l	d0,d2
	move.l	PicIFFSize,d3
	jsr		_LVORead(a6)
	cmp.l	PicIFFSize,d0
	bne	PLSNS900					; Error with file read
	lea		PicFIB,a0
	move.l	(a0),d1
	clr.l	(a0)
	jsr		_LVOClose(a6)
;
;	>>>   DETERMINE PICTURE FORMAT   <<<
;
	move.l	PicIFF,a2				; >>>ILBM/PBM <<<
	move.l	8(a2),d0
	moveq.l	#0,d1
	cmp.l	#'ILBM',d0
	beq	PLSNS200
	moveq.l	#1,d1
	cmp.l	#'PBM ',d0
	bne	PLSNS900
PLSNS200
	move.b	d1,IFFMode					; 0=ILBM,1=PBM
;
;	>>>   DETERMINE PICTURE CONFIGURATION   <<<
;
	move.l	PicIFF,a2				; >>>BMHD<<<
	move.l	#'BMHD',d0
	jsr		FindIFFChunk
	bne	PLSNS900					; Error!
;
;	>>>   GET # BITPLANES   <<<
;
	moveq.l	#0,d0
	move.b	$c(a2),d0					; Get #BitPlanes
	cmp.b	#8,d0						; 8 BitPlanes?
	bgt	PLSNS900					; Too many
	lea		PicNumPlanes,a0
	move.w	d0,(a0)
;
;	>>>   GET STENCIL(MASK) / COMPRESSION STATUS   <<<
;
	move.b	$d(a2),IFFMask				; STENCIL Mask?
	move.b	$e(a2),IFFComp				; Compressed data?
;
;	>>>   GET DIMENSIONS   <<<
;
	moveq.l	#0,d0						; Save RASTER X and Y
	lea		PicX,a0
	move.w	4(a2),d0
;	cmp.w	#1024,d0					; Must be at least 1024 pixels wide!
;	bcs	PLSNS900
	move.w	d0,(a0)
	lea		PicXBytes,a0
	lsr.w	#4,d0						; #Words
	add.w	d0,d0						; #Bytes
	move.w	d0,(a0)
	lea		PicY,a0
	move.w	6(a2),d0
;	cmp.w	#1024,d0					; Must be at least 1024 pixels tall!
;	bcs	PLSNS900
	move.w	d0,(a0)
	lea		PicYBytes,a0
	lsr.w	#3,d0						; #Lines
	move.w	d0,(a0)
;
;	>>>   EXTRACT PALETTE INFORMATION   <<<
;
	move.l	PicIFF,a2				; >>>CMAP<<<
	move.l	#'CMAP',d0
	jsr		FindIFFChunk
	bne	PLSNS900					; Error!
;
	move.l	(a2)+,d6					; D6 = Size of CMAP Chunk
	addq.l	#1,d6
	and.l	#$fffffffe,d6
	divu	#3,d6						; Get #Colours in palette
	cmp.l	#256,d6						; Too many colours?
	bgt	PLSNS900					; Yes
	subq.w	#1,d6						; Adjust for DBF Loop
;
;	>>>   CONVERT IFF COLOURS TO AMIGA COLOURS   <<<
;
	lea	PicPalAmiga,a0					; A0 = AMIGA Colours
PLSNS350
	moveq.l	#0,d1						; Convert IFF Colour
	move.b	(a2),d1
	and.b	#$f0,d1
	lsl.w	#4,d1
	move.b	1(a2),d1
	and.b	#$f0,d1
	move.b	2(a2),d0
	and.b	#$f0,d0
	lsr.b	#4,d0
	or.b	d0,d1
	move.w	d1,(a0)+					; Save AMIGA Colour
	addq.w	#3,a2
	dbf	d6,PLSNS350
;	move.w	#$fff,-2(a0)					; Last AMIGA Colour is WHITE!
;
;	>>>   OPEN PICTURE SCREEN   <<<
;
;	bsr	OpenPictureSNES					; Can we open the picture?
;	bne	PLSNS900					; No, Error!
;
;	>>>   SET UP PICTURE PLANES   <<<
;
;	bsr	SetPicPlanes					; Set PicPlanes
;
;	>>>   UNPACK THE IFF IMAGERY   <<<
;
	move.l	PicIFF,a2					; A2 = IFF Data Block
	moveq.l	#0,d7						; D7 = #Lines-1
	move.w	PicY,d7
	subq.w	#1,d7
	moveq.l	#0,d6						; D6 = #Bytes-1
	move.w	PicXBytes,d6
	subq.w	#1,d6
	moveq.l	#0,d5						; D5 = #Planes-1
	move.w	PicNumPlanes,d5
	subq.w	#1,d5
	lea	PicPlanes,a5					; A5 = PlanePointer
	moveq.l	#0,d4						; D4 = #Bytes-1
	move.w	#(512/16*2),d4
	jsr	DrawIFFPic					; Draw the IFF Imagery
	bra	PLSNS910					; COMPLETED!
;
;	>>>   FINISHED   <<<
;
PLSNS900
	moveq.l	#0,d0						; D0 = 0 = ERROR!
PLSNS910
	move.l	d0,-(a7)					; Save Return Code
	move.l	4,a6						; Release the Memory
	move.l	PicIFFSize,d0
	lea		PicIFF,a0
	tst.l	(a0)
	beq	PLSNS920
	move.l	(a0),a1
	clr.l	(a0)
	jsr		_LVOFreeMem(a6)
PLSNS920
	move.l	DosBase,a6
	lea		PicFIB,a0
	move.l	(a0),d1						; Any file opened?
	clr.l	(a0)
	beq	PLSNS980
	jsr		_LVOClose(a6)
PLSNS980
	move.l	(a7)+,d0					; D0 = Error Code
	rts



	section	__MERGED,DATA

	xdef	PicY,PicYBytes
	xdef	PicX,PicXBytes
	xdef	PicNumPlanes,PicPlanes
	xdef	PicIFF,PicIFFSize
	xdef	PicPalAmiga
	xdef	PicFIB

;
;	* * * * * * *       VARIABLES       * * * * * * *
;
PicY			dc.w	0			; Y Dimension
PicYBytes		dc.w	0			; Y Dimension #Lines
PicX			dc.w	0			; X Dimension
PicXBytes		dc.w	0			; X Dimension #Bytes
PicXNBytes		dc.w	0			; NES/SNES X Dimension #Bytes
PicNumPlanes		dc.w	0			; Number of Planes in IFF Imagery
PicIFF			dc.l	0			; IFF File
PicIFFSize		dc.l	0			; Size of IFF File
PicPalAmiga		ds.w	256			; 256 Amiga Colour Words
PicPlanes		ds.l	8			; 8 PlanePointers
PicPlanes2		ds.l	8			; 8 PlanePointers from AllocMem
PicFIB			dc.l	0			; FIB for Picture


	end
