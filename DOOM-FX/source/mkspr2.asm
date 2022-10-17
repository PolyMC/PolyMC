;***************************************************************************
;*                                                                         *
;*                           M A K E   S P R I T E                         *
;*                                                                         *
;*                        SPRITE DATA CREATION MODULE                      *
;*                                                                         *
;***************************************************************************

	include	mkspr.i

	xref	DosBase
	xref	_LVOOpen,_LVOClose,_LVORead,_LVOWrite

	xref	Task
	xref	PrintMsg,VDTDebugOutC
	xref	MSGUserBreak,MSGNoSpriteList,MSGSpriteImgErr

	xref	OpenPictureSNES,ClosePicture,PictureLoadSNES,SetPicPlanes

	xref	DoomPaletteName,DoomPalette

	xref	PicX,PicY,PicXBytes,PicNumPlanes,PicPalAmiga
	xref	PicPlanes

	xref	SpriteList,SpriteListName,SpriteName,SpriteFileName
	xref	SpriteDefFileName,SpriteDefFileName0,SpriteDef,SpriteDefPtr
	xref	SpriteMinX,SpriteMaxX

	section	MKSPRITE,CODE

	xdef	DoMakeSprite


;
;	* * * * * * *       MAKE SPRITE DATA FILES       * * * * * * *
;
DoMakeSprite
	move.l	DosBase,a6				; Open SPRITELIST
	move.l	SpriteListName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGNoSpriteList,d7
	move.l	d0,d4
	beq	DMW950
	move.l	d4,d1					; Read SPRITELIST
	move.l	SpriteList,d2
	move.l	#16384,d3
	jsr	_LVORead(a6)
	move.l	d0,-(sp)
	move.l	d4,d1					; Close SPRITELIST
	jsr	_LVOClose(a6)
	move.l	SpriteList,a0				; Terminate with $00
	add.l	(sp)+,a0
	clr.b	(a0)
;
	jsr	ReadDoomPalette				; Read PLAYPAL
;
	move.l	SpriteList,a5				; A5 = SPRITE LIST
DMW200
	move.l	#MSGUserBreak,d7			; Quit?
	move.l	Task,a0
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	DMW950					; Yes
;
DMW300
	move.b	(a5)+,d0				; Any more SPRITES?
	beq	DMW900					; No!
	cmp.b	#10,d0
	beq	DMW300
	cmp.b	#'<',d0					; Skip?
	bne.s	DMW380
DMW320
	move.b	(a5)+,d0				; Yes, Scan for EndSkip
	beq	DMW900
	cmp.b	#'>',d0
	bne.s	DMW320
	bra.s	DMW300
;
DMW380
	subq.w	#1,a5
	lea	SpriteName,a0				; A0 = Sprite Name
	lea	SpriteDefFileName,a1			; A1 = Sprite Def FileName
DMW400
	move.b	(a5)+,d0
	move.b	d0,(a0)+
	move.b	d0,(a1)+
	beq	DMW900
	cmp.b	#10,d0
	bne.s	DMW400
	clr.b	-1(a0)
	subq.w	#1,a1					; Add .DEF Suffix
	move.l	#'.DEF',(a1)+
	clr.b	(a1)+
;
	lea	SpriteFileName,a0			; A0 = Sprite FileName
DMW500
	move.b	(a5)+,d0
	move.b	d0,(a0)+
	beq	DMW900
	cmp.b	#10,d0
	bne.s	DMW500
	clr.b	-1(a0)
;
	move.l	a5,-(sp)
	bsr	MakeASprite				; Make a Single Sprite
	move.l	(sp)+,a5
	tst.l	d7
	bne.s	DMW950
;
	pea	SpriteDefFileName0			; Create Error Message
	lea	SpriteDefError(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	addq.w	#4,sp
;
	move.l	DosBase,a6
	move.l	#SpriteDefFileName0,d1			; Save SPRITEDEF
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	moveq.l	#-1,d7
	move.l	d0,d4
	beq	DMW950
	move.l	d4,d1
	move.l	SpriteDef,d2
	move.l	SpriteDefPtr,d3
	sub.l	d2,d3
	move.l	d3,-(sp)
	jsr	_LVOWrite(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
	move.l	(sp)+,d3
	cmp.l	d0,d3
	bne.s	DMW950					; ERROR!
;
	bra	DMW200					; Do Next Sprite
;
;	>>>   COMPLETED MAKING SPRITES   <<<
;
DMW900
	moveq.l	#0,d7
DMW950
	rts


;
;	* * * * * * *       MAKE SINGLE SPRITE       * * * * * * *
;
MakeASprite
	pea	SpriteFileName				; Send SpriteName and PathName
	pea	SpriteName
	lea	SpriteMsg0(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	addq.w	#8,sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	#SpriteFileName,d1			; Load Sprite IFF
	jsr	PictureLoadSNES
	beq	MAW950					; Error!
	jsr	SetPicPlanes				; Set Picture Planes
;
;	>>>   FIND BOTTOM EDGE   <<<
;
	moveq.l	#0,d5					; Y Coordinate
	move.w	PicY,d5
	subq.w	#1,d5
MAW200
	moveq.l	#0,d4					; X Coordinate
	move.w	PicX,d4
	subq.w	#1,d4
MAW220
	bsr	GetPixReg				; Get Colour
	cmp.b	#255,d0
	bne.s	MAW250					; Non-Blank
	dbf	d4,MAW220
	dbf	d5,MAW200				; Move to next line
	bra	MAW950
MAW250
	move.l	d5,d7					; D7 = Maximum Line
	addq.l	#1,d7
	move.w	PicX,SpriteMinX
	clr.w	SpriteMaxX
;
;	>>>   MAKE SPRITE HEADER   <<<
;
	move.l	SpriteDef,a3				; A3 = SPRITEDEF Pointer
	clr.b	(a3)+					; MAXWIDTH
	move.b	d7,(a3)+				; HEIGHT
	move.l	a3,a6					; A6 = LINETABLE Pointer
	move.l	d7,d0					; Allocate space for LINETABLE
	mulu	#4,d0
	add.w	d0,a3
	moveq.l	#0,d5					; Y Coordinate
;
;	>>>   START NEXT LINE   <<<
;
MAW300
	move.l	a3,d0					; #Bytes to skip
	sub.l	a6,d0
	subq.l	#4-1,d0
	move.b	d0,2(a6)
	lsr.w	#8,d0
	move.b	d0,3(a6)
	clr.w	0(a6)					; No Offset, No Width
;
;	>>>   FIND LEFT EDGE   <<<
;
	moveq.l	#0,d4					; X Coordinate
MAW320
	bsr	GetPixReg				; Get Colour
	cmp.b	#255,d0
	bne.s	MAW340					; Non-Blank
	addq.w	#1,d4
	cmp.w	PicX,d4
	bne.s	MAW320
	bra	MAW700					; NO WIDTH!
MAW340
	move.w	d4,d6					; D6 = LEFT-EDGE
	cmp.w	SpriteMinX,d6
	bge.s	MAW345
	move.w	d6,SpriteMinX
MAW345
;
;	>>>   FIND RIGHT EDGE   <<<
;
	move.w	PicX,d4
	subq.w	#1,d4
MAW350
	bsr	GetPixReg				; Get Colour
	cmp.b	#255,d0
	bne.s	MAW370					; Non-Blank
	dbf	d4,MAW350
MAW370
	cmp.w	SpriteMaxX,d4
	blt.s	MAW375
	move.w	d4,SpriteMaxX
MAW375
	sub.w	d6,d4					; D4 = Width
	addq.w	#1,d4
	move.b	d6,0(a6)				; LEFT OFFSET
	move.b	d4,1(a6)				; WIDTH
	addq.w	#4,a6
	move.l	d6,d4					; Starting X Coordinate
;
;	>>>   FIND NEXT LEFT EDGE   <<<
;
MAW400
	bsr	GetPixReg				; Get Colour
	cmp.b	#255,d0
	bne.s	MAW450					; Non-Blank
	addq.w	#1,d4
	cmp.w	PicX,d4
	bne.s	MAW400
	bra	MAW700					; END!
MAW450
	move.l	a3,a2					; A2 = LENGTH BYTE
	clr.b	(a3)+
	move.b	d4,(a3)+				; OFFSET
	moveq.l	#0,d3					; RunLength = 0
;
;	>>>   COMPRESS PIXELSTRIP   <<<
;
MAW500
	movem.l	d2-d3,-(sp)
	bsr	GetPixReg				; Get Colour
	movem.l	(sp)+,d2-d3
	cmp.b	#255,d0					; Blank?
	beq.s	MAW600					; YES!  Done this strip!
	addq.b	#1,(a2)					; TotalLength++
	tst.l	d3					; No RunLength?
	beq.s	MAW550
	cmp.b	d0,d2					; Same Colour?
	beq.s	MAW550					; Yes
	move.l	d0,-(sp)
	bsr	MAWFlush				; Flush Pending Colours
	move.l	(sp)+,d0
	moveq.l	#0,d3					; RunLength = 0
MAW550
	move.b	d0,d2					; Save Colour
	addq.w	#1,d3					; RunLength++
	addq.w	#1,d4					; Next Pixel
	cmp.w	PicX,d4					; At End?
	bne.s	MAW500					; No
MAW600
	bsr	MAWFlush				; Flush Pending Colours
	cmp.w	PicX,d4					; At End?
	bne.s	MAW400					; No
MAW700
	clr.b	(a3)+					; NO LENGTH!
;
;	>>>   NEXT LINE   <<<
;
MAW800
	addq.w	#1,d5					; Next Line
	cmp.w	d5,d7					; At Bottom?
	bne	MAW300					; No
	move.l	a3,SpriteDefPtr				; Save Updated SPRITEDEF Pointer
;
	move.l	SpriteDef,a0				; Save Width of Sprite
	moveq.l	#0,d1
	move.w	SpriteMaxX,d1
	sub.w	SpriteMinX,d1
	addq.w	#1,d1
	move.b	d1,0(a0)
;
	move.l	a3,d0					; #Bytes
	sub.l	a0,d0
	move.l	d0,-(sp)
	move.l	d7,-(sp)				; Height
	move.l	d1,-(sp)				; Width
	lea	SpriteMsg1(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#12,sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	jsr	ClosePicture
MAW900
	moveq.l	#0,d7
	rts
MAW950
	move.l	#MSGSpriteImgErr,d7
	rts


;
;	* * * * * * *       FLUSH PENDING COLOURS       * * * * * * *
;
MAWFlush
	tst.l	d3					; Any Length?
	beq.s	MAWF800					; No
	cmp.l	#3,d3					; At Least 3 colours?
	bge.s	MAWF700					; Yes
	sub.l	d3,d4
	subq.l	#1,d3
MAWF500
	move.l	d3,-(sp)
	bsr	GetPixReg				; Get Colour
	addq.w	#1,d4
	move.l	(sp)+,d3
	move.b	d0,(a3)+				; Send Colour
	dbf	d3,MAWF500
	rts
MAWF700
	move.b	#$ff,(a3)+				; REPEAT TAG
	subq.b	#1,d3
	move.b	d3,(a3)+				; #REPEATS-1
	move.b	d2,(a3)+				; COLOUR
MAWF800
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
	beq	RDP800
	move.l	d4,d1					; Read PLAYPAL
	move.l	DoomPalette,d2
	move.l	#(256*3),d3
	jsr	_LVORead(a6)
	move.l	d4,d1					; Close PLAYPAL
	jsr	_LVOClose(a6)
	move.l	#256-1,d4
	move.l	DoomPalette,a0				; Convert Palette to Amiga
	lea	PicPalAmiga,a1
RDP500
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
	dbf	d4,RDP500
RDP800
	rts


;
;	* * * * * * *       GET A PIXEL'S REGISTER NUMBER       * * * * * * *
;
;	D4 = X Coordinate
;	D5 = Y Coordinate
;
;	D0 = Pixel's Register (COLOUR)
;
GetPixReg
	move.l	d7,-(sp)
	lea	PicPlanes,a5			; A5 = PlanePointer
	move.l	d5,d3				; D3 = YLine
	mulu	#(320/8),d3			; * #Bytes per ScanLine
	move.l	d4,d0				; D0 = XLine
	lsr.l	#3,d0				; / #Pixels per Byte
	add.l	d0,d3				; D3 = Offset to Byte
	move.l	#7,d2				; D2 = 7 - MOD(X/8)
	sub.l	d4,d2
	and.l	#$07,d2				; D2 = Pixel Number
	move.l	a5,a1				; A1 = PlanePicsPointer
	moveq.l	#0,d1				; D1 = Plane#
	moveq.l	#0,d0				; D0 = Pixel's Register
GPR200
	tst.l	(a1)				; Any Plane Here?
	beq	GPR800				; No
	move.l	(a1)+,a0			; A0 = PicPlane
	move.b	(a0,d3.l),d7			; D7 = PicPlaneByte
	btst	d2,d7
	beq	GPR700
	bset	d1,d0
GPR700
	addq.w	#1,d1				; Next Plane
	cmp.w	#8,d1
	bne	GPR200
GPR800
	move.l	(sp)+,d7
	rts


;
;	* * * * * * *       SET A PIXEL'S REGISTER NUMBER       * * * * * * *
;
;	D4 = X Coordinate
;	D5 = Y Coordinate
;	D0 = Pixel's Register (COLOUR)
;
SetPixReg
	move.l	d7,-(sp)
	lea	PicPlanes,a5			; A5 = PlanePointer
	move.l	d5,d3				; D3 = YLine
	mulu	#(320/8),d3			; * #Bytes per ScanLine
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
	move.l	(sp)+,d7
	rts


;
;	* * * * * * *       TEXT MESSAGES       * * * * * * *
;
SpriteMsg0	dc.b	10
		dc.b	'Processing Sprite <%s>',10
		dc.b	'File PathName     <%s>',10,0
SpriteMsg1	dc.b	'Width %ld, Height %ld, Bytes %ld',10,0

SpriteDefError	dc.b	'Error with %s datafile!',10,0

		dc.w	0


	end
