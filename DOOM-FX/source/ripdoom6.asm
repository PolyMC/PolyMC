;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                         CONVERT PLAYPAL MODULE                          *
;*                                                                         *
;***************************************************************************

	include	ripdoom.i


	xref	DosBase
	xref	_LVOOpen
	xref	_LVOClose
	xref	_LVORead
	xref	_LVOWrite

	xref	PrintMsg,VDTDebugOutC
	xref	FindIFFChunk

	xref	DoomWADData
	xref	DoomPaletteName,DoomReMapPaletteName,DoomColourMapName
	xref	DoomPalette,RLPalette,DoomReMapPalette
	xref	DoomReMapTable,DoomUnMapTable
	xref	DoomColourMap,RLColourMap

	xref	PicX,PicY,PicXBytes,PicNumPlanes,PicPalAmiga
	xref	PicPlanes

	xref	ImageReMap,Verbose,RoundRLPalette,GammaRLPalette
	xref	ConvertPlayPal,ConvertColourMap,ConvertColourReMap,ConvertRGBReMap


	section	PLAYPAL,CODE

	xdef	DoConvertPlayPal
	xdef	ReadDoomPalette
	xdef	ReadDoomColourMap
	xdef	SetPixReg
	xdef	GetGamma


;
;	* * * * * * *       CONVERT DOOM PLAYPALETTE/COLOURMAP       * * * * * * *
;
DoConvertPlayPal
	jsr	ReadDoomPalette				; Read PLAYPAL
	bne	DCPP900					; Error!
	jsr	ReadDoomColourMap			; Read COLOURMAP
	bne	DCPP900					; Error!

;
;	* * * * * * *       FILL RL PALETTE WITH UNUSED COLOUR       * * * * * * *
;
	move.w	#(256*14)-1,d7				; 14 Palettes, 256 Colours Each
	move.l	RLPalette,a0				; A0 = ReMapped PLAYPAL
DCPP100
	move.w	#$0080,d0				; INVALID DEFAULT
	move.b	d0,(a0)
	lsr.w	#8,d0
	move.b	d0,1(a0)
	addq.w	#2,a0
	dbf	d7,DCPP100

;
;	* * * * * * *       GENERATE RL PALETTE       * * * * * * *
;
	moveq.l	#14-1,d7				; 14 Palettes
	move.l	DoomPalette,a2				; A2 = PLAYPAL
	move.l	RLPalette,a3				; A3 = ReMapped PLAYPAL
DCPP200
	lea	DoomUnMapTable,a4			; A4 = UnMap Table
	moveq.l	#0,d6					; 256 Colours
;
DCPP210
	moveq.l	#0,d4					; Get Source Colour
	move.b	(a4)+,d4
	mulu	#3,d4
;
	tst.b	GammaRLPalette				; GAMMA CORRECT
	beq.s	DCPP220
;
GetGamma
	moveq.l	#0,d5					; RED CORRECTION
	move.b	(a2,d4.w),d5
	move.l	#$100,d2
	sub.l	d5,d2
	move.l	d2,d1
;
	moveq.l	#0,d0
	move.b	1(a2,d4.w),d0				; GREEN CORRECTION
	move.l	#$100,d2
	sub.l	d0,d2
	cmp.l	d2,d1
	blt.s	DCPP212
	move.l	d2,d1
	move.l	d0,d5
DCPP212
	move.b	2(a2,d4.w),d0				; BLUE CORRECTION
	move.l	#$100,d2
	sub.l	d0,d2
	cmp.l	d2,d1
	blt.s	DCPP214
	move.l	d2,d1
	move.l	d0,d5
DCPP214
	tst.l	d5
	bne.s	DCPP216
	moveq.l	#1,d5
DCPP216
	move.l	d5,d0					; LIMIT MAXIMUM GAMMA CORRECTION
	lsl.l	#2,d0
	cmp.l	d0,d1
	blt.s	DCPP218
	move.l	d0,d1
DCPP218
	lsl.l	#8,d1
	divu.l	d5,d1
	and.l	#$ffff,d1
	mulu	#$000c,d1				; GAMMA CORRECTION
	lsr.l	#8,d1
	and.l	#$ffff,d1
	add.w	#$0100,d1
DCPP220
	moveq.l	#0,d0					; RED
	move.b	(a2,d4.w),d0
	tst.b	GammaRLPalette
	beq.s	DCPP230
	mulu	d1,d0
	lsr.l	#8,d0
	and.l	#$ffff,d0
	cmp.w	#$100,d0
	bge.s	DCPP235
DCPP230
	tst.b	RoundRLPalette
	beq.s	DCPP240
	add.w	#$4,d0
	cmp.w	#$100,d0
	blt.s	DCPP240
DCPP235
	move.w	#$ff,d0
DCPP240
	lsr.w	#3,d0
	and.w	#%000000000011111,d0
	move.w	d0,d2
;
	moveq.l	#0,d0					; GREEN
	move.b	1(a2,d4.w),d0
	tst.b	GammaRLPalette
	beq.s	DCPP330
	mulu	d1,d0
	lsr.l	#8,d0
	and.l	#$ffff,d0
	cmp.w	#$100,d0
	bge.s	DCPP335
DCPP330
	tst.b	RoundRLPalette
	beq.s	DCPP340
	add.w	#$4,d0
	cmp.w	#$100,d0
	blt.s	DCPP340
DCPP335
	move.w	#$ff,d0
DCPP340
	lsr.b	#3,d0
	lsl.w	#5,d0
	and.w	#%000001111100000,d0
	or.w	d0,d2
;
	moveq.l	#0,d0					; BLUE
	move.b	2(a2,d4.w),d0
	tst.b	GammaRLPalette
	beq.s	DCPP430
	mulu	d1,d0
	lsr.l	#8,d0
	and.l	#$ffff,d0
	cmp.w	#$100,d0
	bge.s	DCPP435
DCPP430
	tst.b	RoundRLPalette
	beq.s	DCPP440
	add.w	#$4,d0
	cmp.w	#$100,d0
	blt.s	DCPP440
DCPP435
	move.w	#$ff,d0
DCPP440
	lsr.b	#3,d0
	lsl.w	#5,d0
	lsl.w	#5,d0
	and.w	#%111110000000000,d0
	or.w	d0,d2
;
	move.b	d2,(a3)+				; Save new SNES Colour
	lsr.w	#8,d2
	move.b	d2,(a3)+
;
	addq.w	#1,d6					; Next Colour
	cmp.w	#256,d6
	bne	DCPP210
	add.w	#(256*3),a2				; Next Set
	dbf	d7,DCPP200

;
;	* * * * * * *       GENERATE RL COLOURMAP       * * * * * * *
;
	moveq.l	#34-1,d7				; 34 ColourMaps
	move.l	DoomColourMap,a2			; A2 = COLOURMAP
	move.l	RLColourMap,a3				; A3 = ReMapped COLOURMAP
	lea	DoomReMapTable,a4			; A4 = ReMap Table
DCPP500
	moveq.l	#0,d6					; 256 Colours
	moveq.l	#0,d0					; PreClear High Bytes
	moveq.l	#0,d1					; PreClear High Bytes
DCPP600
	move.b	(a4,d6.w),d1				; D1 = ORIGINAL Colour ReMapped
;
	move.b	(a2,d6.w),d0				; D0 = ORIGINAL Colour Mapping
	move.b	(a4,d0.w),d0				; D0 =      NEW Colour Mapping
;
	move.b	d0,(a3,d1.w)				; Save NEW Colour Mapping
;
	addq.w	#1,d6					; Next Colour
	cmp.w	#256,d6
	bne	DCPP600
	clr.b	(a3)		; ,d1.w)		; Colour#0 Maps to Colour#0 ALWAYS!
	add.w	#256,a2					; Next Set
	add.w	#256,a3
	dbf	d7,DCPP500

;
;	* * * * * * *       WRITE OUT RL PALETTE       * * * * * * *
;
	move.l	DosBase,a6				; Open PLAYPAL
	move.l	ConvertPlayPal,d1
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	DCPP900
	move.l	d4,d1					; WRITE PLAYPAL
	move.l	RLPalette,d2
	move.l	#(256*2*14),d3
	jsr	_LVOWrite(a6)
	move.l	d4,d1
	jsr	_LVOClose(a6)

;
;	* * * * * * *       GENERATE STATUS TEXT       * * * * * * *
;
	tst.b	Verbose
	beq	DCPP800
;
	moveq.l	#0,d6					; 14 Palettes
	move.l	RLPalette,a3				; A3 = ReMapped PLAYPAL
DCPP700
	moveq.l	#0,d5					; 256 Colours to Scan
	moveq.l	#0,d4					; Duplicate ColourCount
DCPP740
	move.w	(a3)+,d0				; D0 = Current Colour
	cmp.w	#$0080,d0
	beq.s	DCPP770					; Invalid DUPLICATE Colour
	move.l	a3,a0
	move.w	#256-1,d2				; D2 = #Colours LEFT
	sub.w	d5,d2
	bra.s	DCPP760
DCPP750
	move.w	(a0)+,d1				; D1 = NEXT Colour
	cmp.w	d0,d1
	bne.s	DCPP760
	addq.l	#1,d4					; One More Duplicate!
	move.w	#$0080,-2(a0)				; REMOVE This Colour!
DCPP760
	dbf	d2,DCPP750				; Next Colour
DCPP770
	addq.w	#1,d5					; Next Colour
	cmp.w	#256,d5
	bne	DCPP740
;
	tst.l	d4					; ANY Duplicate Colours?
	beq.s	DCPP780					; NO!
	move.l	d6,-(sp)
	move.l	d4,-(sp)
	lea	RLPaletteDupMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(2*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
DCPP780
	addq.w	#1,d6					; Next RL Palette
	cmp.w	#14,d6
	bne.s	DCPP700

;
;	* * * * * * *       WRITE OUT RL COLOURMAP       * * * * * * *
;
DCPP800
	move.l	DosBase,a6				; Open COLOURMAP
	move.l	ConvertColourMap,d1
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	DCPP900
	move.l	d4,d1					; WRITE COLOURMAP
	move.l	RLColourMap,d2
	move.l	#(256*34),d3
	jsr	_LVOWrite(a6)
	move.l	d4,d1
	jsr	_LVOClose(a6)
;
;	* * * * * * *       WRITE OUT RL COLOURREMAP       * * * * * * *
;
	move.l	DosBase,a6				; Open COLOURREMAP
	move.l	ConvertColourReMap,d1
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	DCPP900
	move.l	d4,d1					; WRITE COLOURREMAP
	move.l	#DoomReMapTable,d2
	move.l	#256,d3
	jsr	_LVOWrite(a6)
	move.l	d4,d1
	jsr	_LVOClose(a6)
;
;	* * * * * * *       WRITE OUT RL RGBREMAP       * * * * * * *
;
	move.l	DosBase,a6				; Open RGBREMAP
	move.l	ConvertRGBReMap,d1
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	DCPP900
	move.l	d4,d1					; WRITE RGBREMAP
	move.l	#DoomUnMapTable,d2
	move.l	#256,d3
	jsr	_LVOWrite(a6)
	move.l	d4,d1
	jsr	_LVOClose(a6)
;
;	* * * * * * *       DONE CONVERTING PALETTES       * * * * * * *
;
DCPP900
	moveq.l	#0,d7
	rts


;
;	* * * * * * *       READ DOOM PALETTE       * * * * * * *
;
ReadDoomPalette
	move.l	DosBase,a6				; Open PLAYPAL
	move.l	#DoomPaletteName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	RDP950
	move.l	d4,d1					; Read PLAYPAL
	move.l	DoomPalette,d2
	move.l	#(256*3*14),d3
	jsr	_LVORead(a6)
	move.l	d4,d1					; Close PLAYPAL
	jsr	_LVOClose(a6)
	move.l	#256-1,d4
	move.l	DoomPalette,a0				; Convert Palette to Amiga
	lea	PicPalAmiga,a1
	lea	DoomReMapPalette,a2			; Copy Palette to ReMap Palette
RDP200
	move.b	2(a0),d0				; BLUE
	lsr.b	#4,d0
	and.l	#$f,d0
	move.b	1(a0),d1				; GREEN
	and.b	#$f0,d1
	or.b	d1,d0
	move.b	0(a0),d1				; RED
	lsl.w	#4,d1
	and.w	#$f00,d1
	or.w	d1,d0
	move.w	d0,(a1)+
	move.b	(a0)+,(a2)+				; Copy PALETTE to REMAP PALETTE
	move.b	(a0)+,(a2)+
	move.b	(a0)+,(a2)+
	dbf	d4,RDP200
;
;	* * * * * * *       GENERATE ONE-TO-ONE REMAP TABLE       * * * * * * *
;
	lea	DoomReMapTable,a0			; Generate DEFAULT ReMap Table
	lea	DoomUnMapTable,a1			; Generate DEFAULT UnMap Table
	move.w	#256-1,d4
	moveq.l	#0,d0
RDP400
	move.b	d0,(a0)+
	move.b	d0,(a1)+
	addq.b	#1,d0
	dbf	d4,RDP400
;
	tst.b	ImageReMap				; REMAP Palette?
	beq	RDP900					; No!
;
;	* * * * * * *       READ IN REMAPPING PALETTE       * * * * * * *
;
	move.l	DosBase,a6
	move.l	DoomReMapPaletteName,d1			; Open REMAP Palette
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	RDP900
	move.l	d4,d1					; Read REMAP Palette
	move.l	DoomWADData,d2
	move.l	#1024*1024,d3
	jsr	_LVORead(a6)
	move.l	d4,d1					; Close REMAP Palette
	jsr	_LVOClose(a6)
;
	move.l	DoomWADData,a2				; >>>CMAP<<<
	move.l	#'CMAP',d0
	jsr	FindIFFChunk
	bne	RDP900					; Error!
;
	move.l	(a2)+,d6				; D6 = Size of CMAP Chunk
	addq.l	#1,d6
	and.l	#$fffffffe,d6
	divu	#3,d6					; Get #Colours in palette
	cmp.l	#256,d6					; Too many colours?
	bne	RDP900					; Yes
	subq.w	#1,d6					; Adjust for DBF Loop
;
	lea	DoomReMapPalette,a0			; A0 = REMAP Palette
RDP450
	move.b	(a2)+,(a0)+				; Copy RGB Triplet
	move.b	(a2)+,(a0)+
	move.b	(a2)+,(a0)+
	dbf	d6,RDP450
;
	move.l	#256-1,d4
	lea	DoomReMapPalette,a0			; Convert Palette to Amiga
	lea	PicPalAmiga,a1
RDP470
	move.b	2(a0),d0				; BLUE
	lsr.b	#4,d0
	and.l	#$f,d0
	move.b	1(a0),d1				; GREEN
	and.b	#$f0,d1
	or.b	d1,d0
	move.b	0(a0),d1				; RED
	lsl.w	#4,d1
	and.w	#$f00,d1
	or.w	d1,d0
	addq.w	#3,a0
	move.w	d0,(a1)+
	dbf	d4,RDP470

;
;	* * * * * * *       GENERATE IMAGE REMAP TABLE       * * * * * * *
;
	move.l	DoomPalette,a2				; ORIGINAL Palette
	lea	DoomReMapTable,a3			; REMAP Table
	moveq.l	#0,d4					; D4 = Colour Count
RDP500
	move.l	(a2),d0					; D0 = ORIGINAL RGB TRIPLET
	lsr.l	#8,d0					; D0 = Red.B/Green.B/Blue.B
;
	lea	DoomReMapPalette+3,a1			; REMAP Palette
	moveq.l	#1,d3
RDP540
	move.l	(a1),d1					; D1 = REMAP RGB TRIPLET
	lsr.l	#8,d1					; D1 = Red.B/Green.B/Blue.B
	cmp.l	d0,d1					; Same?
	beq.s	RDP600					; Found It!
	add.w	#3,a1
	addq.w	#1,d3
	cmp.w	#256,d3
	bne.s	RDP540
;
	move.l	d4,-(sp)
	lea	PaletteReMapErrMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	bra	RDP950					; ERROR!
;
RDP600
	move.b	d3,(a3)+				; REMAP Table
	add.w	#3,a2
	addq.w	#1,d4
	cmp.w	#255,d4
	bne.s	RDP500					; Next Colour
	move.b	#$1b,(a3)+				; ReMap $FF->$1B ALWAYS
;
;	* * * * * * *       DISPLAY REMAP INFORMATION       * * * * * * *
;
	tst.b	Verbose					; VERBOSE?
	beq	RDP900					; No
;
	moveq.l	#0,d5					; UNUSED ReMap Colour Count
	moveq.l	#0,d4					; ReMap Colour Index
RDP700
	lea	DoomReMapTable,a3			; REMAP Table
	move.w	#256-1,d3
RDP710
	move.b	(a3)+,d0				; Found It?
	cmp.b	d0,d4
	beq.s	RDP720					; Yes, It's Used!
	dbf	d3,RDP710
	addq.w	#1,d5
RDP720
	addq.w	#1,d4					; Next Colour
	cmp.w	#256,d4
	bne.s	RDP700
;
	move.l	d5,-(sp)
	lea	PaletteUnUsedReMapMsg(pc),a0		; Print #Colours NOT Used
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	tst.l	d5					; ANY UnUsed?
	beq	RDP900					; NO!
;
	moveq.l	#0,d4					; ReMap Colour Index
RDP750
	lea	DoomReMapTable,a3			; REMAP Table
	move.w	#256-1,d3
RDP760
	move.b	(a3)+,d0				; Found It?
	cmp.b	d0,d4
	beq.s	RDP770					; Yes!  It's Used
	dbf	d3,RDP760
;
	move.l	d4,-(sp)				; NOT USED!
	lea	PaletteUnUsedColourMsg(pc),a0		; Print Colour# NOT Used
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
RDP770
	addq.w	#1,d4					; Next Colour
	cmp.w	#256,d4
	bne.s	RDP750

;
;	* * * * * * *       GENERATE IMAGE UNMAP TABLE       * * * * * * *
;
	lea	DoomReMapPalette,a2			; REMAP Palette
	lea	DoomUnMapTable,a3			; UNMAP Table
	moveq.l	#0,d4					; D4 = Colour Count
RDP800
	move.l	(a2),d0					; D0 = REMAP RGB TRIPLET
	lsr.l	#8,d0					; D0 = Red.B/Green.B/Blue.B
;
	move.l	DoomPalette,a1				; ORIGINAL Palette
	moveq.l	#0,d3
RDP820
	move.l	(a1),d1					; D1 = REMAP RGB TRIPLET
	lsr.l	#8,d1					; D1 = Red.B/Green.B/Blue.B
	cmp.l	d0,d1					; Same?
	beq	RDP860					; Yes!
	add.w	#3,a1					; Next Colour
	addq.w	#1,d3
	cmp.w	#256,d3
	bne.s	RDP820
RDP860
	move.b	d3,(a3)+				; REMAP Table
	add.w	#3,a2					; Next Colour
	addq.w	#1,d4
	cmp.w	#256,d4
	bne.s	RDP800

;
;	* * * * * * *       DONE LOADING/REMAPPING       * * * * * * *
;
RDP900
	moveq.l	#0,d0					; OK!
	rts
RDP950
	moveq.l	#-1,d0					; ERROR!
	rts


;
;	* * * * * * *       READ DOOM COLOURMAP       * * * * * * *
;
ReadDoomColourMap
	move.l	DosBase,a6				; Open COLOURMAP
	move.l	#DoomColourMapName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	RDCM950
	move.l	d4,d1					; Read COLOURMAP
	move.l	DoomColourMap,d2
	move.l	#(256*34),d3
	jsr	_LVORead(a6)
	move.l	d4,d1					; Close COLOURMAP
	jsr	_LVOClose(a6)
RDCM900
	moveq.l	#0,d0
	rts
RDCM950
	moveq.l	#-1,d0
	rts


;
;	* * * * * * *       SET A PIXEL'S REGISTER NUMBER       * * * * * * *
;
;	D4 = X Coordinate
;	D5 = Y Coordinate
;	D0 = Pixel's Register (COLOUR)
;
SetPixReg
	lea	PicPlanes,a5			; A5 = PlanePointer
	move.l	d5,d3				; D3 = YLine
	mulu	PicXBytes,d3			; * #Bytes per ScanLine
	move.l	d4,d1				; D1 = XLine
	lsr.l	#3,d1				; / #Pixels per Byte
	add.l	d1,d3				; D3 = Offset to Byte
	move.l	#7,d2				; D2 = 7 - MOD(X/8)
	sub.l	d4,d2
	and.l	#$07,d2				; D2 = Pixel Number
	move.l	a5,a1				; A1 = PlanePicsPointer
	moveq.l	#0,d1				; D1 = Plane#
SPR200
	tst.l	(a1)				; Any Plane Here?
	beq	SPR800				; No
	move.l	(a1)+,a0			; A0 = PicPlane
	move.b	(a0,d3.l),d7			; D7 = PicPlaneByte
	bclr	d2,d7
	btst	d1,d0
	beq	SPR700
	bset	d2,d7
SPR700
	move.b	d7,(a0,d3.l)
	addq.w	#1,d1				; Next Plane
	cmp.w	#8,d1
	bne	SPR200
SPR800
	rts


;
;	* * * * * * *       TEXT MESSAGES       * * * * * * *
;
PaletteReMapErrMsg
	dc.b	'Error!  Colour %ld has no REMAP Match!',10,0
PaletteUnUsedReMapMsg
	dc.b	'%ld Colours UnUsed in REMAP',10,0
PaletteUnUsedColourMsg
	dc.b	'Colour %ld',10,0
;
RLPaletteDupMsg
	dc.b	'%ld Colours Duplicated in RLPALETTE %ld',10,0

	dc.w	0


	end
