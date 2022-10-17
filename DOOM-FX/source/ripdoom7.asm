;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                        CONVERT PATCHLIST MODULE                         *
;*                                                                         *
;***************************************************************************

	include	ripdoom.i


	xref	DosBase,_LVOOpen,_LVOClose,_LVORead,_LVOWrite
	xref	VDTDebugOutC,PrintMsg

	xref	GraphicsBase
	xref	_LVOAllocBitMap,_LVOFreeBitMap,_LVOBltBitMap,_LVOBltMaskBitMapRastPort
	xref	_LVOInitRastPort,_LVOSetAPen,_LVORectFill

	xref	DoomPatchListName,DoomTexture1Name,DoomTexture2Name
	xref	PatchList,DoomPatchList
	xref	TextureList,DoomTexture1,DoomTexture2
	xref	Texture1Name,Texture2Name,Texture3Name
	xref	DoomFileName

	xref	ReadDoomPalette
	xref	OpenPictureSNES,WriteIFF,ClosePicture,SetPicPlanes,PictureLoadSNES
	xref	PicX,PicY,PicXBytes,PicNumPlanes,PicPalAmiga
	xref	PicPlanes,PicRPort,PicScreen
	xref	PicDim,PicDimWidth,PicDimHeight,PicDimPlanes,PicPlanesOffset

	xref	MSGPicError
	xref	MSGUserBreak

	xref	Task


	section	PATCHLIST,CODE

	xdef	DoConvertTextures
	xdef	ReadDoomPNAMES

	xdef	ConvertPNAMES
	xdef	ConvertTEXTURE1
	xdef	ConvertTEXTURE2
	xdef	ConvertTEXTURE


;
;	* * * * * * *       CONVERT DOOM TEXTURES       * * * * * * *
;
DoConvertTextures
	move.l	#768,d0				; Width
	move.l	#768,d1				; Height
	move.l	#8,d2				; #Planes
	moveq.l	#0,d3				; Flags
	move.l	#0,a0				; Friend BitMap
	move.l	GraphicsBase,a6
	jsr	_LVOAllocBitMap(a6)		; Allocate BitMap
	tst.l	d0
	beq	DCTS900
	move.l	d0,PatchBitMap			; Save BitMap
;
	move.l	#512,d0				; Width
	move.l	#256+11,d1			; Height
	move.l	#1,d2				; #Planes
	moveq.l	#0,d3				; Flags
	move.l	#0,a0				; Friend BitMap
	jsr	_LVOAllocBitMap(a6)		; Allocate MaskBitMap
	tst.l	d0
	beq	DCTS900
	move.l	d0,PatchMaskBitMap		; Save MaskBitMap
;
	lea	PatchRPort(pc),a1		; Initialize Patch RastPort
	jsr	_LVOInitRastPort(a6)
	lea	PatchRPort(pc),a1		; Initialize PatchRPort.BitMap
	move.l	PatchBitMap,d0
	move.l	d0,4(a1)
	move.l	#0,d0
	jsr	_LVOSetAPen(a6)
;
	move.w	#512,PicX
	move.w	#(512/16*2),PicXBytes
	move.w	#256,PicY
	move.w	#8,PicNumPlanes
	jsr	ReadDoomPalette			; Read PLAYPAL
	bne	DCTS900				; Error!
	jsr	OpenPictureSNES			; Open the picture
	move.l	#MSGPicError,d7
	tst.l	d0
	bne	DCTS900				; Error!
;
	move.l	GraphicsBase,a6
	move.l	PicRPort,a1
	move.l	#0,d0
	jsr	_LVOSetAPen(a6)
;
	bsr	ConvertPNAMES			; Convert PNAMES
	tst.l	d7
	bne.s	DCTS900
;
	bsr	ConvertTEXTURES
	beq.s	DCTS800
;
	bsr	ConvertTEXTURE1			; Convert TEXTURE1
	tst.l	d7
	bne.s	DCTS900
	bsr	ConvertTEXTURE2			; Convert TEXTURE2
	tst.l	d7
	bne.s	DCTS900
DCTS800
	moveq.l	#0,d7
DCTS900
	move.l	d7,-(sp)
;
	jsr	ClosePicture
;
	move.l	GraphicsBase,a6
	lea	PatchBitMap(pc),a0
	move.l	(a0),d0
	beq.s	DCTS960
	move.l	d0,a0
	jsr	_LVOFreeBitMap(a6)
DCTS960
;
	lea	PatchMaskBitMap(pc),a0
	move.l	(a0),d0
	beq.s	DCTS980
	move.l	d0,a0
	jsr	_LVOFreeBitMap(a6)
DCTS980
;
	move.l	(sp)+,d7
	rts


;
;	* * * * * * *       CONVERT TEXTURE       * * * * * * *
;
ConvertTEXTURE
	moveq.l	#0,d5				; D5 = #TEXTURES
	move.b	3(a5),d5
	lsl.w	#8,d5
	move.b	2(a5),d5
	lsl.l	#8,d5
	move.b	1(a5),d5
	lsl.l	#8,d5
	move.b	0(a5),d5
	addq.w	#4,a5				; Skip #Entries
	move.l	d5,d0				; Skip Entry Pointers
	lsl.l	#2,d0
	add.l	d0,a5
	subq.w	#1,d5
CTXE2000
	move.l	a5,a1				; A1 = TEXTUREName
	lea	Texture1Name,a0
	moveq.l	#8-1,d0				; 8 Characters Maximum
CTXE2200
	move.b	(a1)+,(a0)+
	dbeq	d0,CTXE2200
	clr.b	(a0)				; Terminate TEXTURE Name
	moveq.l	#0,d0				; #Patches
	move.b	21(a5),d0
	lsl.w	#8,d0
	move.b	20(a5),d0
	move.l	d0,d4
	move.l	d0,-(sp)
	pea	Texture1Name
	move.b	15(a5),d0			; Height
	lsl.w	#8,d0
	move.b	14(a5),d0
	move.w	d0,WallHeight
	move.l	d0,-(sp)
	move.b	13(a5),d0			; Width
	lsl.w	#8,d0
	move.b	12(a5),d0
	move.w	d0,WallWidth
	move.l	d0,-(sp)
	add.w	#22,a5				; Skip Past Header
	move.l	d6,-(sp)
	lea	TEXTUREDataMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(5*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	subq.w	#1,d4				; #PATCHES-1
	moveq.l	#0,d3				; PATCH#
CTXE3000
	move.l	#MSGUserBreak,d7		; Quit?
	move.l	Task,a0
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	CTXE9000			; Yes
;
	moveq.l	#0,d0				; Patch#
	move.b	5(a5),d0
	lsl.w	#8,d0
	move.b	4(a5),d0
	move.l	d0,-(sp)
	move.l	DoomPatchList,a1		; PNAMES
	addq.w	#4,a1
	lsl.l	#3,d0
	add.l	d0,a1
	lea	Texture2Name,a0
	moveq.l	#8-1,d0				; 8 Characters Maximum
CTXE3200
	move.b	(a1)+,(a0)+
	dbeq	d0,CTXE3200
	clr.b	(a0)				; Terminate PATCH Name
	pea	Texture2Name
	move.b	3(a5),d0			; Y Offset
	lsl.w	#8,d0
	move.b	2(a5),d0
	ext.l	d0
	move.l	d0,PatchOffsetY
	move.l	d0,-(sp)
	move.b	1(a5),d0			; X Offset
	lsl.w	#8,d0
	move.b	0(a5),d0
	ext.l	d0
	move.l	d0,PatchOffsetX
	move.l	d0,-(sp)
	add.w	#10,a5				; Skip Past Header
	move.l	d3,-(sp)
	lea	TEXTUREDataMsg2(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(5*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
;	>>>   LOAD PATCH IMAGE   <<<
;
	lea	DOOMPATCHDir(pc),a0		; Load PATCH Image
	lea	DoomFileName,a1
CTXE4000
	move.b	(a0)+,(a1)+
	bne.s	CTXE4000
	subq.w	#1,a1
	lea	Texture2Name,a0
CTXE4100
	move.b	(a0)+,(a1)+
	bne.s	CTXE4100
;
	movem.l	d3-d6/a5,-(sp)
	move.w	#512,PicX
	move.w	#(512/16*2),PicXBytes
	move.w	#256,PicY
	move.w	#8,PicNumPlanes
	jsr	SetPicPlanes
;
	move.l	GraphicsBase,a6
	move.l	PicRPort,a1
	move.l	#0,d0
	move.l	#0,d1
	move.l	#0+255,d2
	move.l	#0+255,d3
	jsr	_LVORectFill(a6)
;
	move.l	#DoomFileName,d1		; Load Patch IFF
	jsr	PictureLoadSNES
	movem.l	(sp)+,d3-d6/a5
	move.l	#MSGPicError,d7
	tst.l	d0
	beq	CTXE9000			; Error!
;
;	>>>   CREATE A MASK BITMAP OF THE PATCH   <<<
;
	movem.l	d3-d6/a5,-(sp)
	move.l	GraphicsBase,a6
;
	move.l	PicScreen,a2			; A2 = Screen
	move.l	$58(a2),a2			; A2 = RPort->BitMap
	addq.w	#8,a2				; A2 = BitMap.Planes[8]
	move.l	PatchMaskBitMap,a3		; Destination BitMap
	move.l	8(a3),a3			; A3 = BitMap.Plane[0]
	move.w	PicY,d7
	subq.w	#1,d7
	move.l	#(11*(512/8)),d5		; D5 = Source BitMap Offset
CTXE5000
	moveq.l	#0,d6				; Width X
	move.w	PicX,d6
	lsr.w	#5,d6				; 32 pixels per long (DO ONE EXTRA!)
	move.l	d5,d4				; D4 = Source BitMap Offset
CTXE5100
	move.l	a2,a1				; A1 = Source PlanePtr
	moveq.l	#0,d0				; MASK LONGWORD
	moveq.l	#8-1,d3				; 8 Planes
CTXE5200
	move.l	(a1)+,a0			; A0 = Source Plane
	or.l	(a0,d4.l),d0			; Add Bits to Mask
	dbf	d3,CTXE5200
	move.l	d0,(a3,d4.l)			; SAVE MASK LONGWORD
	addq.l	#4,d4				; Next Source 32 Pixels
	dbf	d6,CTXE5100
	add.l	#(512/8),d5			; Next Source Line
	dbf	d7,CTXE5000
;
;	>>>   COPY THE PATCH TO THE BITMAP   <<<
;
	move.l	PicScreen,a0			; A0 = Screen
	move.l	$58(a0),a0			; A0 = RPort->BitMap
	move.l	#0,d0				; Source X
	move.l	#11,d1				; Source Y
	lea	PatchRPort(pc),a1		; Destination RastPort
	move.l	PatchOffsetX,d2			; Destination X
	add.l	#256,d2
	move.l	PatchOffsetY,d3			; Destination Y
	add.l	#256,d3
	moveq.l	#0,d4				; Width X
	move.w	PicX,d4
	moveq.l	#0,d5				; Height Y
	move.w	PicY,d5
	move.l	#$0e0,d6			; MinTerm (ABC|ABNC|ANBC)
	move.l	PatchMaskBitMap,a2		; BltMask
	move.l	8(a2),a2			; A2 = BitMap.Plane[0]
	jsr	_LVOBltMaskBitMapRastPort(a6)
;
;	>>>   COPY THE BITMAP TO THE WALL   <<<
;
	move.l	PatchBitMap,a0			; Source BitMap
	move.l	#256,d0				; Source X
	move.l	#256,d1				; Source Y
	move.l	PicScreen,a1			; A1 = Screen
	move.l	$58(a1),a1			; A1 = RPort->BitMap
	move.l	#256,d2				; Destination X
	move.l	#11,d3				; Destination Y
	moveq.l	#0,d4				; Width X
	move.w	WallWidth,d4
	moveq.l	#0,d5				; Height Y
	move.w	WallHeight,d5
	move.l	#$0c0,d6			; MinTerm (COPY)
	move.l	#$ff,d7				; Mask
	move.l	#0,a2				; TempA
	jsr	_LVOBltBitMap(a6)
	movem.l	(sp)+,d3-d6/a5
;
	addq.w	#1,d3				; Next Patch
	dbf	d4,CTXE3000
;
;	>>>   SAVE WALL IMAGE   <<<
;
	lea	DOOMWALLSDir(pc),a0		; Save WALL Image
	lea	DoomFileName,a1
CTXE7000
	move.b	(a0)+,(a1)+
	bne.s	CTXE7000
	subq.w	#1,a1
	lea	Texture1Name,a0
CTXE7100
	move.b	(a0)+,(a1)+
	bne.s	CTXE7100
;
	movem.l	d5-d6/a5,-(sp)
	move.w	#512,PicX
	move.w	#(512/16*2),PicXBytes
	move.w	#256,PicY
	move.w	#8,PicNumPlanes
	move.l	#(256/8),PicPlanesOffset	; PlanesOffset = 256 pixels
	moveq.l	#0,d4
	move.w	WallWidth,d4
	moveq.l	#0,d5
	move.w	WallHeight,d5
	move.l	#DoomFileName,d1
	jsr	WriteIFF
	clr.l	PicPlanesOffset			; PlanesOffset = 0
;
	move.l	GraphicsBase,a6
	move.l	PicRPort,a1
	move.l	#256,d0
	move.l	#0,d1
	move.l	#256+255,d2
	move.l	#0+255,d3
	jsr	_LVORectFill(a6)
;
	lea	PatchRPort(pc),a1
	move.l	#256,d0
	move.l	#256,d1
	move.l	#256+255,d2
	move.l	#256+255,d3
	jsr	_LVORectFill(a6)
;
	movem.l	(sp)+,d5-d6/a5
;
	addq.w	#1,d6				; Next Texture
	dbf	d5,CTXE2000
CTXE8000
	moveq.l	#0,d7
CTXE9000
	rts


;
;	* * * * * * *       CONVERT TEXTURES       * * * * * * *
;
ConvertTEXTURES
	move.l	DosBase,a6
	move.l	#DoomTexture1Name,d1		; Open TEXTURE1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	CTXS800
	move.l	d4,d1				; Read TEXTURE1
	move.l	DoomTexture1,d2
	move.l	#(64*1024),d3
	jsr	_LVORead(a6)
	move.l	d4,d1				; Close TEXTURE1
	jsr	_LVOClose(a6)
;
	move.l	#DoomTexture2Name,d1		; Open TEXTURE2
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	CTXS800
	move.l	d4,d1				; Read TEXTURE2
	move.l	DoomTexture2,d2
	move.l	#(64*1024),d3
	jsr	_LVORead(a6)
	move.l	d4,d1				; Close TEXTURE2
	jsr	_LVOClose(a6)
;
	move.l	DoomTexture2,a0			; Get #TEXTURES2
	moveq.l	#0,d1
	move.b	3(a0),d1
	lsl.w	#8,d1
	move.b	2(a0),d1
	lsl.l	#8,d1
	move.b	1(a0),d1
	lsl.l	#8,d1
	move.b	0(a0),d1
	move.l	d1,-(sp)
;
	move.l	DoomTexture1,a0			; Get #TEXTURES1
	moveq.l	#0,d0
	move.b	3(a0),d0
	lsl.w	#8,d0
	move.b	2(a0),d0
	lsl.l	#8,d0
	move.b	1(a0),d0
	lsl.l	#8,d0
	move.b	0(a0),d0
	move.l	d0,-(sp)
;
	add.l	d1,d0				; Get #TEXTURES
	move.l	d0,-(sp)
;
	lea	NumTEXTURESMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(3*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	moveq.l	#-1,d0
CTXS800
	rts


;
;	* * * * * * *       CONVERT TEXTURE1       * * * * * * *
;
ConvertTEXTURE1
	moveq.l	#0,d6				; Starting TEXTURE#
	move.l	DoomTexture1,a5
	bra	ConvertTEXTURE			; Convert TEXTURE

;
;	* * * * * * *       CONVERT TEXTURE2       * * * * * * *
;
ConvertTEXTURE2
	move.l	DoomTexture2,a5
	bra	ConvertTEXTURE			; Convert TEXTURE


;
;	* * * * * * *       CONVERT PNAMES PATCHLIST       * * * * * * *
;
ConvertPNAMES
	jsr	ReadDoomPNAMES
	move.l	DoomPatchList,a5
	moveq.l	#0,d5
	move.b	3(a5),d5
	lsl.w	#8,d5
	move.b	2(a5),d5
	lsl.l	#8,d5
	move.b	1(a5),d5
	lsl.l	#8,d5
	move.b	0(a5),d5
	addq.w	#4,a5
	move.l	d5,-(sp)
	lea	NumPATCHESMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	subq.w	#1,d5
	moveq.l	#0,d6
DCPL200
	move.l	#MSGUserBreak,d7		; Quit?
	move.l	Task,a0
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	DCPL800				; Yes
;
	move.l	a5,a1				; A1 = PatchName
	lea	Texture1Name,a0
	moveq.l	#8-1,d0				; 8 Characters Maximum
DCPL220
	move.b	(a1)+,(a0)+
	dbeq	d0,DCPL220
	clr.b	(a0)				; Terminate Patch Name
	pea	Texture1Name
	move.l	d6,-(sp)
	lea	PATCHDataMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(2*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	addq.w	#1,d6
	add.w	#8,a5				; Next PatchName
	dbf	d5,DCPL200
	moveq.l	#0,d7
DCPL800
	rts


;
;	* * * * * * *       READ DOOM PATCHLIST       * * * * * * *
;
ReadDoomPNAMES
	move.l	DosBase,a6				; Open PATCHES
	move.l	#DoomPatchListName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	RDPL800
	move.l	d4,d1					; Read PATCHES
	move.l	DoomPatchList,d2
	move.l	#(64*1024),d3
	jsr	_LVORead(a6)
	move.l	d4,d1					; Close PATCHES
	jsr	_LVOClose(a6)
RDPL800
	rts


;
;	* * * * * * *       TEXT MESSAGES       * * * * * * *
;
NumPATCHESMsg
	dc.b	12,'%ld PATCHES',10,0
PATCHDataMsg
	dc.b	'PATCH %3ld  <%8s>',10,0

NumTEXTURESMsg
	dc.b	12,'%ld TEXTURES (%ld TEXTURE1, %ld TEXTURE2)',10,0
TEXTUREDataMsg
	dc.b	10,'TEXTURE %3ld  (%5ld,%5ld)  <%8s>  [%ld]',10,0
TEXTUREDataMsg2
	dc.b	'  PATCH %3ld  (%5ld,%5ld)  <%8s>  [%ld]',10,0

DOOMPATCHDir
	dc.b	'DOOMIFF:PATCHES/',0
DOOMWALLSDir
	dc.b	'DOOMIFF:WALLS/',0

	dc.w	0

WallWidth	dc.w	0
WallHeight	dc.w	0

PatchOffsetX	dc.l	0
PatchOffsetY	dc.l	0

PatchBitMap	dc.l	0
PatchRPort	ds.b	100			; sizeof(RastPort)

PatchMaskBitMap	dc.l	0

	end
