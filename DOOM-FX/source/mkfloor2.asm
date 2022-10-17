;***************************************************************************
;*                                                                         *
;*                           M A K E   F L O O R                           *
;*                                                                         *
;*                         FLOOR DATA CREATION MODULE                      *
;*                                                                         *
;***************************************************************************

	include	mkfloor.i

	xref	DosBase
	xref	_LVOOpen,_LVOClose,_LVORead,_LVOWrite

	xref	Task
	xref	PrintMsg,VDTDebugOutC
	xref	MSGUserBreak,MSGNoFloorList
	xref	MSGPicError
	xref	MSGFloorDefError
	xref	MSGFloorList2Error

	xref	MsgBuffer

	xref	OpenPictureSNES,ClosePicture,PictureLoadSNES,SetPicPlanes

	xref	DoomPaletteName,DoomPalette

	xref	PicX,PicY,PicXBytes,PicNumPlanes,PicPalAmiga
	xref	PicPlanes

	xref	FloorList,FloorListName,FloorName,FloorFileName
	xref	FloorList2,FloorList2Name
	xref	FloorDefName,FloorDef,FloorDefPtr

	xref	FloorColour,FloorColourTally


	section	MKFLOOR,CODE

	xdef	DoMakeFloor


;
;	* * * * * * *       MAKE FLOOR DATA FILES       * * * * * * *
;
DoMakeFloor
	move.l	DosBase,a6				; Open FLOORLIST
	move.l	#FloorListName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGNoFloorList,d7
	move.l	d0,d4
	beq	DMF950
	move.l	d4,d1					; Read FLOORLIST
	move.l	FloorList,d2
	move.l	#16384,d3
	jsr	_LVORead(a6)
	move.l	d0,-(sp)
	move.l	d4,d1					; Close FLOORLIST
	jsr	_LVOClose(a6)
	move.l	FloorList,a0				; Terminate with $00
	add.l	(sp)+,a0
	clr.b	(a0)
;
	jsr	ReadDoomPalette				; Read PLAYPAL
;
	move.l	FloorList,a5				; A5 = FLOOR LIST
	move.l	FloorList2,a4				; A4 = FLOOR LIST2
DMF200
	move.l	#MSGUserBreak,d7			; Quit?
	move.l	Task,a0
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	DMF950					; Yes
;
DMF300
	move.b	(a5)+,d0				; Any more FLOORS?
	beq	DMF900					; No!
	move.b	d0,(a4)+
	cmp.b	#10,d0
	beq	DMF300
	cmp.b	#'<',d0					; Skip?
	bne.s	DMF380
DMF320
	move.b	(a5)+,d0				; Yes, Scan for EndSkip
	beq	DMF900
	move.b	d0,(a4)+
	cmp.b	#'>',d0
	bne.s	DMF320
	bra.s	DMF300
;
DMF380
	subq.w	#1,a5
	subq.w	#1,a4
	lea	FloorName,a0				; A0 = Floor Name
DMF400
	move.b	(a5)+,d0
	move.b	d0,(a0)+
	beq	DMF900
	move.b	d0,(a4)+
	cmp.b	#10,d0
	bne.s	DMF400
	clr.b	-1(a0)
;
	lea	FloorFileName,a0			; A0 = Floor FileName
DMF500
	move.b	(a5)+,d0
	move.b	d0,(a0)+
	beq	DMF900
	move.b	d0,(a4)+
	cmp.b	#10,d0
	bne.s	DMF500
	clr.b	-1(a0)
;
;	>>>   PROCESS A SINGLE FLOOR   <<<
;
	move.l	a5,-(sp)
	bsr	MakeAFloor				; Make a Single Floor
	move.l	(sp)+,a5
	tst.l	d7					; Error?
	bne	DMF950					; Yes!
;
;	>>>   COPY EXISTING FLOORCOLOUR TEXT   <<<
;
	tst.b	FloorColour				; Texture2/Colour?
	bne.s	DMF700					; Colour
DMF600
	move.b	(a5)+,d0				; Copy FloorColour Text
	beq	DMF900
	move.b	d0,(a4)+
	cmp.b	#10,d0
	bne.s	DMF600
	bra	DMF200					; Do Next Floor
;
;	>>>   CREATE NEW FLOORCOLOUR TEXT   <<<
;
DMF700
	move.b	(a5)+,d0				; Skip Existing FloorColour Text
	beq	DMF900
	cmp.b	#10,d0
	bne.s	DMF700
;
	move.l	d2,-(sp)				; Create FloorColour Text
	lea	FloorColourMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	addq.w	#4,sp
;
	lea	MsgBuffer,a0				; Copy FloorColour Text
DMF720
	move.b	(a0)+,(a4)+
	bne.s	DMF720
	subq.w	#1,a4					; Move back overtop $00
	bra	DMF200					; Do Next Floor

;
;	>>>   COMPLETED MAKING FLOORS   <<<
;
DMF900
	move.l	DosBase,a6
	move.l	#FloorDefName,d1			; Save FLOORDEF
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGFloorDefError,d7
	move.l	d0,d4
	beq	DMF950
	move.l	d4,d1
	move.l	FloorDef,d2
	move.l	FloorDefPtr,d3
	sub.l	d2,d3
	move.l	d3,-(sp)
	jsr	_LVOWrite(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
	move.l	(sp)+,d3
	cmp.l	d0,d3
	bne.s	DMF950
;
	move.l	#FloorList2Name,d1			; Save FLOORLIST2
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGFloorList2Error,d7
	move.l	d0,d4
	beq	DMF950
	move.l	d4,d1
	move.l	FloorList2,d2
	move.l	a4,d3
	sub.l	d2,d3
	move.l	d3,-(sp)
	jsr	_LVOWrite(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
	move.l	(sp)+,d3
	cmp.l	d0,d3
	bne.s	DMF950
;
	moveq.l	#0,d7
DMF950
	rts


;
;	* * * * * * *       MAKE SINGLE FLOOR       * * * * * * *
;
;	D2 = FloorColour
;
MakeAFloor
	pea	FloorFileName				; Send FloorName and PathName
	pea	FloorName
	lea	FloorMsg0(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	addq.w	#8,sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	#FloorFileName,d1			; Load Floor IFF
	jsr	PictureLoadSNES
	move.l	#MSGPicError,d7
	tst.l	d0
	beq	MAW900					; Error!
	jsr	SetPicPlanes				; Set Picture Planes
;
	move.w	#256-1,d0				; Clear ColourTally
	lea	FloorColourTally,a0
MAW100
	clr.w	(a0)+
	dbf	d0,MAW100
;
	move.l	FloorDefPtr,a3				; A3 = FLOORDEF Pointer
	moveq.l	#0,d5					; Y Coordinate
MAW200
	moveq.l	#0,d4					; X Coordinate
MAW300
	bsr	GetPixReg				; Get Colour
	move.b	d0,(a3)+
;
	lea	FloorColourTally,a0
	lsl.w	#1,d0					; ColourTally[Colour]++
	addq.w	#1,(a0,d0.w)
;
	addq.w	#1,d4					; Next Pixel
	cmp.w	#64,d4
	bne.s	MAW300
	addq.w	#1,d5					; Next Line
	cmp.w	#64,d5
	bne.s	MAW200
	move.l	a3,FloorDefPtr				; Save Updated FLOORDEF Pointer
;
	jsr	ClosePicture
	moveq.l	#0,d7
;
	tst.b	FloorColour				; Texture2/Colour?
	beq.s	MAW900					; Texture2
;
	lea	FloorColourTally,a0
	moveq.l	#0,d3					; D3 = Colour Counter
	moveq.l	#0,d1					; D1 = Best ColourTally
MAW700
	move.w	(a0)+,d0				; D0 = Get ColourTally
	cmp.w	d1,d0					; Higher than Best so far?
	ble.s	MAW720					; No
	move.w	d0,d1					; Save Best ColourTally
	move.l	d3,d2					; Save Best Colour
MAW720
	addq.w	#1,d3					; Next Colour
	cmp.w	#256,d3
	bne.s	MAW700
;
	move.l	d2,-(sp)				; Send FloorColour
	lea	FloorMsg1(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	(sp)+,d2
	moveq.l	#0,d7
;
MAW900
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
FloorMsg0	dc.b	10
		dc.b	'Processing Floor  <%s>',10
		dc.b	'File PathName     <%s>',10,0

FloorMsg1	dc.b	'FloorColour       <%ld>',10,0

FloorColourMsg	dc.b	'%ld',10,0


	end
