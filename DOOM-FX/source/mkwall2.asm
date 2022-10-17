;***************************************************************************
;*                                                                         *
;*                             M A K E   W A L L                           *
;*                                                                         *
;*                         WALL DATA CREATION MODULE                       *
;*                                                                         *
;***************************************************************************

	include	mkwall.i

	xref	DosBase
	xref	_LVOOpen,_LVOClose,_LVORead,_LVOWrite

	xref	Task
	xref	PrintMsg,VDTDebugOutC,VDTDebugOutC2
	xref	MSGUserBreak,MSGNoWallList,MSGNoImageList
	xref	MSGWallTblError,MSGWallDataError
	xref	MSGImageTblError,MSGImageDataError
	xref	MSGFileError
	xref	MSGPicError

	xref	OpenPictureSNES,ClosePicture,PictureLoadSNES,SetPicPlanes

	xref	DoomPaletteName,DoomPalette

	xref	PicX,PicY,PicXBytes,PicNumPlanes,PicPalAmiga
	xref	PicPlanes

	xref	OutputFIB
	xref	MsgBuffer,MsgBuffer2

	xref	WallList,WallListName,WallName,WallFileName
	xref	WallTableName,WallTable,WallTablePtr
	xref	WallDataName,WallData,WallDataPtr
	xref	WallDefName,WallDef
	xref	WallStrip,WallStrip2
	xref	NumStripsNew,NumStripsOld,NumStripsPart
	xref	NumPixels,NumPixelsTotal
	xref	NumPixelsUsed,NumPixelsUsedTotal
	xref	ImageListName,ImageTableName,ImageDataName,ImageDefName
	xref	WallImgDefName,WallImgSourceName

	xref	DoImages,ImageComp


	section	MKWALL,CODE

	xdef	DoMakeWall
	xdef	MakeAWall
	xdef	BuildWallImageStrip,BuildImageStrip,BuildWallStrip


;
;	* * * * * * *       MAKE WALL DATA FILES       * * * * * * *
;
DoMakeWall
	move.l	DosBase,a6				; Open WALLLIST
	move.l	#WallListName,d1
	move.l	#MSGNoWallList,d7
	tst.b	DoImages				; Doing IMAGES?
	beq.s	DMW100					; No
	move.l	#ImageListName,d1
	move.l	#MSGNoImageList,d7
DMW100
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	DMW950
	move.l	d4,d1					; Read WALLLIST
	move.l	WallList,d2
	move.l	#(64*1024),d3
	jsr	_LVORead(a6)
	move.l	d0,-(sp)
	move.l	d4,d1					; Close WALLLIST
	jsr	_LVOClose(a6)
	move.l	WallList,a0				; Terminate with $00
	add.l	(sp)+,a0
	clr.b	(a0)
;
	tst.b	DoImages				; Doing IMAGES?
	beq.s	DMW180					; No
	bsr	LoadWalls				; Yes, LOAD WALLSDEF!
	bne	DMW950
DMW180
	jsr	ReadDoomPalette				; Read PLAYPAL
;
	move.l	WallList,a5				; A5 = WALL LIST
;
;	>>>   NEXT LINE   <<<
;
DMW200
	move.l	#MSGUserBreak,d7			; Quit?
	move.l	Task,a0
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	DMW950					; Yes
DMW220
	move.b	(a5)+,d0				; Any more WALLS/IMAGES?
	beq	DMW900					; No!
	cmp.b	#10,d0					; BLANK LINE?
	beq	DMW220
	cmp.b	#'<',d0					; SKIP?
	bne.s	DMW400
;
;	>>>   SKIP/ENDSKIP   <<<
;
DMW300
	move.b	(a5)+,d0				; Yes, Scan for EndSkip
	beq	DMW900
	cmp.b	#'>',d0
	bne.s	DMW300
	bra	DMW200
;
;	>>>   COMMENT   <<<
;
DMW400
	cmp.b	#';',d0					; COMMENT?
	bne.s	DMW700
DMW420
	move.b	(a5)+,d0				; Yes, Scan for EOL/EOT
	beq	DMW900
	cmp.b	#10,d0
	bne.s	DMW420
	bra	DMW200
;
;	>>>   NEXT WALL/IMAGE   <<<
;
DMW700
	subq.w	#1,a5
	lea	WallName,a0				; A0 = Wall Name
DMW720
	move.b	(a5)+,d0
	move.b	d0,(a0)+
	beq	DMW900
	cmp.b	#10,d0
	bne.s	DMW720
	clr.b	-1(a0)
;
	lea	WallFileName,a0				; A0 = Wall FileName
DMW740
	move.b	(a5)+,d0
	move.b	d0,(a0)+
	beq	DMW900
	cmp.b	#10,d0
	bne.s	DMW740
	clr.b	-1(a0)
;
	move.l	a5,-(sp)
	bsr	MakeAWall				; Make a Single Wall
	move.l	(sp)+,a5
	bne	DMW950					; ERROR!
;
	move.l	NumPixels,d0				; Add Total Pixels
	add.l	d0,NumPixelsTotal
	move.l	NumPixelsUsed,d0			; Add Total Used Pixels
	add.l	d0,NumPixelsUsedTotal
;
	bra	DMW200					; Do Next Wall
;
;	>>>   COMPLETED MAKING WALLS   <<<
;
DMW900
	move.l	NumPixelsUsedTotal,d1			; Display TOTALS
	move.l	NumPixelsTotal,d2
	move.l	d1,d0
	mulu.l	#100,d0
	divu.l	d2,d0
	and.l	#$ffff,d0
	move.l	d0,-(sp)
	move.l	d1,-(sp)
	move.l	d2,-(sp)
	lea	WallTotalMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(3*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	bsr	SaveWalls				; Save WALLS
DMW950
	jmp	ClosePicture


;
;	* * * * * * *       LOAD WALLS       * * * * * * *
;
LoadWalls
	moveq.l	#0,d6					; D6 = Bank#
	moveq.l	#0,d7					; NO ERRORS!
;
;	>>>   NEXT WALL BANK   <<<
;
LWS500
	lea	WallBankSizes,a0			; UnUsed Bank?
	tst.l	(a0,d6.w)
	bmi	LWS700					; Yes!
;
;	>>>   CREATE WALL BANK FILENAME   <<<
;
	lea	WallDefName,a0				; Create WALLDEFBANK FileName
	move.l	d6,d0
	lsr.w	#2,d0
	move.l	d0,-(sp)
	move.l	sp,a1
	jsr	VDTDebugOutC2
	add.w	#(1*4),sp
;
;	>>>   LOAD NEXT WALL BANK DATA FILE   <<<
;
	lea	MsgBuffer2,a0
	move.l	a0,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	LWS700
;
	move.l	WallDef,d2
	move.l	d6,d0
	move.l	#(15-2),d1
	lsl.l	d1,d0
	add.l	d0,d2
	move.l	#32768,d3
	move.l	d4,d1
	jsr	_LVORead(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
;
	lea	WallBankSizes,a0			; Save EXISTING Size of Bank
	move.l	d0,(a0,d6.w)
	lea	WallBankOldSizes,a0
	move.l	d0,(a0,d6.w)
;
;	>>>   NEXT WALL BANK   <<<
;
LWS700
	addq.l	#4,d6					; Next Bank
	cmp.w	#(64*4),d6				; End of Banks?
	bne	LWS500					; NO!
	bra.s	LWS900					; YES!  ALL FINISHED!
;
;	>>>   ERROR WITH BANK FILES!   <<<
;
LWS780
	lea	MSGFileError,a0
	pea	MsgBuffer2
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
;
;	>>>   FINISHED LOADING WALLS   <<<
;
LWS900
	tst.l	d7
	rts


;
;	* * * * * * *       SAVE WALLS       * * * * * * *
;
SaveWalls
;
;	>>>   SAVE WALL INDEX TABLE   <<<
;
	move.l	DosBase,a6				; Save WALLTABLE
	move.l	#WallTableName,d1
	move.l	#MSGWallTblError,d7
	tst.b	DoImages
	beq.s	SWS220
	move.l	#ImageTableName,d1
	move.l	#MSGImageTblError,d7
SWS220
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	SWS900
	move.l	d4,d1
	move.l	WallTable,d2
	move.l	WallTablePtr,d3
	sub.l	d2,d3
	move.l	d3,-(sp)
	jsr	_LVOWrite(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
	move.l	(sp)+,d3
	cmp.l	d0,d3
	bne	SWS900
;
;	>>>   SAVE WALL DATA TABLE   <<<
;
	move.l	#WallDataName,d1			; Save WALLDATA
	move.l	#MSGWallDataError,d7
	tst.b	DoImages
	beq.s	SWS320
	move.l	#ImageDataName,d1
	move.l	#MSGImageDataError,d7
SWS320
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	SWS900
	move.l	d4,d1
	move.l	WallData,d2
	move.l	WallDataPtr,d3
	sub.l	d2,d3
	move.l	d3,-(sp)
	jsr	_LVOWrite(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
	move.l	(sp)+,d3
	cmp.l	d0,d3
	bne	SWS900

;
;	>>>   SAVE WALL BANK DATA   <<<
;
SWS400
	moveq.l	#-4,d6					; D6 = Bank#
	moveq.l	#0,d5					; D5 = SourceFile FIB
	moveq.l	#0,d7					; NO ERRORS!
	bra	SWS700					; Start first SourceFile
;
;	>>>   NEXT WALL BANK   <<<
;
SWS500
	lea	WallBankSizes,a0			; D3 = Size of Bank
	move.l	(a0,d6.w),d3
	bmi	SWS700					; UnUsed Bank
	beq	SWS700					; Empty Bank
;
;	>>>   CREATE WALLIMAGE BANK FILENAME   <<<
;
	lea	WallImgDefName,a0			; Create WALLIMGDEFBANK FileName
	move.l	d6,d0
	lsr.w	#2,d0
	move.l	d0,-(sp)
	move.l	sp,a1
	jsr	VDTDebugOutC2
	add.w	#(1*4),sp
;
;	>>>   UPDATE SOURCE INCLUDE FILE   <<<
;
	move.l	OutputFIB,-(sp)
	move.l	d5,OutputFIB
;
	move.l	d6,d1				; SECTION WALLIMGDEFXX,LOAD=X,ORG=X
	lsr.w	#2,d1
	move.l	d1,d0
	swap	d0
	or.w	#$8000,d0
	move.l	d0,-(sp)
	lsr.l	#1,d0
	and.l	#$ffff8000,d0
	or.l	#$80000000,d0
	move.l	d0,-(sp)
	move.l	d1,-(sp)
	move.l	sp,a1
	lea	WallImgSectMsg,a0
	jsr	VDTDebugOutC
	add.w	#(3*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	lea	WallImgImageMsg,a0		; IMAGE <imagefilename>
	pea	MsgBuffer2
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	(sp)+,OutputFIB
;
;	>>>   SAVE NEXT WALL/IMAGE BANK DATA FILE   <<<
;
	lea	MsgBuffer2,a0
	move.l	a0,d1
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	SWS780
	move.l	WallDef,d2
	move.l	d6,d0
	move.l	#(15-2),d1
	lsl.l	d1,d0
	add.l	d0,d2
	move.l	d4,d1
	move.l	d3,-(sp)
	jsr	_LVOWrite(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
	move.l	(sp)+,d3
	cmp.l	d0,d3
	bne	SWS780
;
;	>>>   CREATE WALL/IMAGE DEF FILENAME   <<<
;
	lea	WallBankSizes,a0			; D3 = Size of Bank
	move.l	(a0,d6.w),d3
	lea	WallBankOldSizes,a0			; D7 = Offset within Bank
	move.l	(a0,d6.w),d7
	lea	WallDefName,a0				; Create WALLDEFBANK FileName
	tst.b	DoImages				; WALLS/IMAGES?
	beq.s	SWS620					; WALLS
	sub.l	d7,d3
	beq.s	SWS700
	lea	ImageDefName,a0
SWS620
	move.l	d6,d0
	lsr.w	#2,d0
	move.l	d0,-(sp)
	move.l	sp,a1
	jsr	VDTDebugOutC2
	add.w	#(1*4),sp
;
;	>>>   SAVE NEXT WALL/IMAGE BANK DATA FILE   <<<
;
SWS650
	lea	MsgBuffer2,a0
	move.l	a0,d1
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq	SWS780
	move.l	WallDef,d2
	move.l	d6,d0
	move.l	#(15-2),d1
	lsl.l	d1,d0
	add.l	d0,d2
	add.l	d7,d2
	move.l	d4,d1
	move.l	d3,-(sp)
	jsr	_LVOWrite(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
	move.l	(sp)+,d3
	cmp.l	d0,d3
	bne.s	SWS780
;
;	>>>   NEXT WALL BANK   <<<
;
SWS700
	addq.l	#4,d6					; Next Bank
;
;	>>>   NEXT SOURCE FILE?   <<<
;
	move.l	d6,d0					; Even multiple of 16 banks?
	and.l	#(($0000000f)<<2),d0
	bne.s	SWS770					; No
;
	move.l	d5,d1					; Any Open SourceFile?
	beq.s	SWS710					; No
	jsr	_LVOClose(a6)				; Yes, Close It!
SWS710
	cmp.w	#(64*4),d6				; End of Banks?
	beq	SWS900					; YES!  Don't create next SOURCE!
;
;	>>>   CREATE NEW SOURCE FILE   <<<
;
	lea	WallImgSourceName,a0			; Create SOURCE FileName
	move.l	d6,d0
	lsr.w	#(2+4),d0
	move.l	d0,-(sp)
	move.l	sp,a1
	jsr	VDTDebugOutC2
	add.w	#(1*4),sp
	lea	MsgBuffer2,a0
	move.l	a0,d1
	move.l	#1006,d2
	jsr	_LVOOpen(a6)				; Create NEW SourceFile
	move.l	d0,d5
	beq	SWS780
SWS770
	cmp.w	#(64*4),d6				; End of Banks?
	bne	SWS500					; No, Next Bank
	bra.s	SWS900
;
;	>>>   ERROR WITH BANK FILES!   <<<
;
SWS780
	move.l	d5,d1					; Any Open SourceFile?
	beq.s	SWS785					; No
	jsr	_LVOClose(a6)				; Yes, Close It!
SWS785
	lea	MSGFileError,a0
	pea	MsgBuffer2
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
;
;	>>>   FINISHED SAVING WALLS   <<<
;
SWS900
	tst.l	d7
	rts


;
;	* * * * * * *       MAKE SINGLE WALL       * * * * * * *
;
MakeAWall
	pea	WallFileName			; Send WallName and PathName
	pea	WallName
	move.l	WallTablePtr,d0			; Get Wall#
	sub.l	WallTable,d0
	lsr.w	#1,d0
	move.l	d0,-(sp)
	pea	WallMsg(pc)
	tst.b	DoImages
	beq.s	MAW120
	move.l	#ImageMsg,(sp)
MAW120
	lea	WallMsg0(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(4*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	#WallFileName,d1			; Load Wall IFF
	jsr	PictureLoadSNES
	move.l	#MSGPicError,d7
	tst.l	d0
	beq	MAW950					; Error!
	jsr	SetPicPlanes
;
;	>>>   USE IFF DIMENSIONS AS ENCLOSING RECTANGLE FOR WALL   <<<
;
	moveq.l	#0,d5					; WALL Dimensions for Width/Height
	move.w	PicY,d5
	moveq.l	#0,d6
	move.w	PicX,d6
;
;	>>>   FIND RIGHT STRIP   <<<
;
	move.l	d6,d4					; X Coordinate
MAW200
	subq.w	#1,d4
	moveq.l	#0,d5					; Y Coordinate
	move.w	PicY,d5
MAW220
	subq.w	#1,d5
	bmi.s	MAW200
	bsr	GetPixReg				; Get Colour
	tst.l	d0
	beq.s	MAW220					; Zero
	move.l	d4,d6					; Found Right-Most Strip
	addq.w	#1,d6
;
;	>>>   FIND BOTTOM STRIP   <<<
;
	moveq.l	#0,d5					; Y Coordinate
	move.w	PicY,d5
MAW250
	subq.w	#1,d5
	move.l	d6,d4					; X Coordinate
MAW270
	subq.w	#1,d4
	bmi.s	MAW250
	bsr	GetPixReg				; Get Colour
	tst.l	d0
	beq.s	MAW270					; Non-Zero
	addq.w	#1,d5					; Found Bottom-Most Strip
;
;	>>>   DISPLAY CALCULATED DIMENSIONS   <<<
;
;
	move.l	d5,d0					; GET #PIXELS IN THIS WALL/IMAGE
	mulu	d6,d0
	move.l	d0,NumPixels
	clr.l	NumPixelsUsed
;
	move.l	d5,-(sp)				; Height
	move.l	d6,-(sp)				; Width
	lea	WallMsg2(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	addq.w	#8,sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	WallDataPtr,a4				; A4 = WALLDATA Pointer
;
	move.l	WallTablePtr,a1				; Save Entry into WALLTABLE
	move.l	a4,d0
	sub.l	WallData,d0
	move.b	d0,(a1)+
	lsr.w	#8,d0
	move.b	d0,(a1)+
	move.l	a1,WallTablePtr

;	clr.w	(a4)+					; Flags
	move.b	d5,(a4)+				; Height
	move.b	d6,d0
	tst.b	DoImages				; WALLS/IMAGES?
	bne.s	MAW320					; IMAGES
	subq.b	#1,d0					; WALLS, Save *MODULO*
MAW320
	move.b	d0,(a4)+				; Modulo/Width
;
	move.l	d5,d7					; D7 = Lowest Pixel
	subq.l	#1,d7
	moveq.l	#0,d4					; Start at Left-Side
	clr.l	NumStripsNew
	clr.l	NumStripsOld
	clr.l	NumStripsPart
;
;	>>>   BUILD NEXT WALLSTRIP   <<<
;
MAW400
	move.l	Task,a0					; Quit?
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	MAW900					; Yes
;
	move.w	d4,$dff180
	ifd	egad
	move.l	d4,d0					; Print Multiples of 16
	and.l	#$f,d0
	bne.s	MAW410
	move.l	d7,-(sp)
	move.l	d4,-(sp)
	lea	WallMsg3(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#4,sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	(sp)+,d7
MAW410
	endc
;
	movem.l	d4/d6-d7,-(sp)
	bsr	BuildWallImageStrip			; Build Wall/Image Strip
	bsr	FindWallImageStrip			; Find Strip in DataBase
	or.w	#$8000,d2				; Address $8000-$FFFF
	move.b	d2,(a4)+
	lsr.w	#8,d2
	move.b	d2,(a4)+
	move.w	d6,d0					; Bank $00-$3F
	lsr.w	#2,d0
	move.b	d0,(a4)+
	movem.l	(sp)+,d4/d6-d7
;
	addq.l	#1,d4					; Next PixelStrip
	cmp.l	d4,d6
	bne	MAW400
;
	move.l	a4,WallDataPtr				; Save Updated WallData Ptr
;
	move.l	NumPixelsUsed,d1
	move.l	NumPixels,d2
	move.l	d1,d0
	mulu	#100,d0
	divu	d2,d0
	and.l	#$ffff,d0
	move.l	d0,-(sp)
	move.l	d1,-(sp)
	move.l	d2,-(sp)
	move.l	NumStripsPart,-(sp)
	move.l	NumStripsOld,-(sp)
	move.l	NumStripsNew,-(sp)
	lea	WallMsg4(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(6*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
MAW900
;	jsr	ClosePicture
	moveq.l	#0,d7
MAW950
	tst.l	d7
	rts


;
;	* * * * * * *       BUILD WALL/IMAGE STRIP       * * * * * * *
;
;	D4 = X Coordinate of Strip
;	D7 = Y Coordinate of Bottom of Strip
;
;	D7 = Length of Stored StripData-1
;
BuildWallImageStrip
	move.l	d7,d5				; Bottom-Column
	lea	WallStrip2,a2			; A2 = WallStrip2
BWIS200
	bsr	GetPixReg			; Get PixelData
	move.b	d0,(a2)+
	dbf	d5,BWIS200
;
	tst.b	DoImages			; Doing IMAGES?
	beq	BuildWallStrip			; No
;
;	* * * * * * *       BUILD IMAGE STRIP       * * * * * * *
;
;	REPEAT		DC.B	-#Repeat,Colour
;		<or>
;	UNIQUE		DC.B	#Unique,Colour,Colour,Colour...
;		<or>
;	END		DC.B	$00
;
BuildImageStrip
	tst.b	ImageComp			; NORMAL COMPRESSION?
	beq	BuildWallStrip2			; YES!
;
;	DC.B	Colour,Colour,Colour,...
;			<or>
;	DC.B	$00,$00-$7F (Skip #Pixels) ($00=End of Strip)
;			<or>
;	DC.B	$00,$80-$FF,Colour (Duplicate -#Pixels)
;
	move.l	d7,d5				; Bottom-Column
	lea	WallStrip,a2			; A2 = WallStrip
;
;	>>>   UNIQUE   <<<
;
	moveq.l	#-1,d2				; D2 = Unique Colour
BIS300
	move.l	d2,-(sp)
	bsr	GetPixReg			; Get PixelData
	move.l	(sp)+,d2
	cmp.l	d0,d2				; <REPEAT>?
	beq.s	BIS400				; Yes
BIS340
	tst.b	d0				; <SKIP>?
	beq.s	BIS600				; Yes
BIS380
	move.l	d0,d2				; <UNIQUE>
	move.b	d0,(a2)+
	dbf	d5,BIS300
;
;	>>>   UNIQUE -> END   <<<
;
	bra.s	BIS800
;
;	>>>   REPEAT   <<<
;
BIS400
	move.b	#$00,-1(a2)			; <REPEAT>
	move.b	#$fe,(a2)+			; <REPEAT COUNT>
	move.b	d2,(a2)+			; <REPEAT COLOUR>
	bra.s	BIS440
BIS420
	movem.l	d2,-(sp)
	bsr	GetPixReg			; Get PixelData
	movem.l	(sp)+,d2
	cmp.l	d0,d2				; <REPEAT>?
	bne.s	BIS480				; No
	subq.b	#1,-2(a2)			; <REPEAT COUNT>++
BIS440
	dbf	d5,BIS420
;
;	>>>   REPEAT -> END   <<<
;
	move.b	-2(a2),d1			; <REPEAT COUNT>
	cmp.b	#$fe,d1				; AT LEAST 3 REPEATS?
	bne.s	BIS800				; YES
	subq.w	#3,a2				; NO, CHANGE TO 2 UNIQUE
	move.b	d2,(a2)+
	move.b	d2,(a2)+
	bra.s	BIS800
;
;	>>>   REPEAT -> SKIP/UNIQUE   <<<
;
BIS480
	move.b	-2(a2),d1			; <REPEAT COUNT>
	cmp.b	#$fe,d1				; AT LEAST 3 REPEATS?
	bne.s	BIS340				; YES
	subq.w	#3,a2				; NO, CHANGE TO 2 UNIQUE
	move.b	d2,(a2)+
	move.b	d2,(a2)+
	bra.s	BIS340				; <SKIP/UNIQUE>
;
;	>>>   SKIP   <<<
;
BIS600
	move.b	#$00,(a2)+			; <SKIP>
	clr.b	(a2)+				; <STRIPCOUNT>
	bra.s	BIS680
BIS620
	addq.b	#1,-1(a2)			; <SKIPCOUNT>++
	bsr	GetPixReg			; Get PixelData
	tst.b	d0
	bne.s	BIS380				; <UNIQUE>
BIS680
	dbf	d5,BIS620
;
;	>>>   SKIP -> END   <<<
;
	clr.b	-1(a2)				; <END OF STRIP>
;
;	>>>   END   <<<
;
BIS800
	move.l	a2,d7				; D7 = #ImageStripBytes-1
	lea	WallStrip+1,a0
	sub.l	a0,d7
	rts

;
;	* * * * * * *       BUILD WALL STRIP       * * * * * * *
;
;	REPEAT		DC.B	-#Repeat,Colour
;		<or>
;	UNIQUE		DC.B	#Unique,Colour,Colour,Colour...
;
BuildWallStrip
;
;	>>>   SKY1/SKY2 OR REGULAR WALL?   <<<
;
	move.l	WallTablePtr,d0			; Get Wall#
	sub.l	WallTable,d0
	lsr.w	#1,d0
	cmp.l	#3,d0				; 1=SKY1,2=SKY2
	bge.s	BuildWallStrip2			; Not SKY1/SKY2!
;
;	>>>   DON'T COMPRESS SKY1/SKY2   <<<
;
	lea	WallStrip2,a0			; A0 = WallStrip2
	lea	WallStrip,a2			; A2 = WallStrip
	move.l	d7,d5				; Bottom-Column
BWS500
	move.b	(a0)+,(a2)+
	dbf	d5,BWS500
	rts
;
;	>>>   COMPRESS WALL STRIP   <<<
;
BuildWallStrip2
	movem.l	a3-a4,-(sp)
	lea	WallStrip2,a0			; A0 = Source
	lea	1(a0,d7.w),a1			; A1 = End of Source
	move.l	a0,a2				; A2 = IP (Start of LITERAL)
	move.l	a0,a3				; A3 = IQ (End+1 of LITERAL)
	lea	WallStrip,a4			; A4 = Destination
;
;	LITERAL RUN
;
CAS1
	move.l	a3,a0				; A0 = PT (Start of REPLICATES)
	move.b	(a3)+,d3			; Character
	cmp.l	a3,a1				; At End of Input?
	beq	CAS5
	move.l	a3,d1				; Check for maximum overflow
	sub.l	a2,d1
	cmp.l	#127,d1
	beq	CAS6
	cmp.b	(a3),d3				; Next character same?
	bne	CAS1				; No!
;
;	AT LEAST 2 BYTE REPEAT
;
CAS2
	move.b	(a3)+,d3			; Get next character
	cmp.l	a3,a1				; End of Input?
	beq	CAS7
	move.l	a3,d1				; Check for maximum overflow
	sub.l	a2,d1
	cmp.l	#128,d1
	beq	CAS6
	cmp.b	(a3),d3				; Next character same?
	bne	CAS1				; No!
;
;	REPLICATE RUN
;
CAS3
	move.b	(a3)+,d3			; Get next character
	cmp.l	a3,a1				; End of Input?
	beq	CAS7				; Yes
	move.l	a3,d1				; Check for maximum overflow
	sub.l	a2,d1
	cmp.l	#128,d1
	beq	CAS4
	cmp.b	(a3),d3				; Next character same?
	beq	CAS3				; Yes
;
;	DUMP LITERAL/REPEAT AND CONTINUE
;
CAS4
	move.l	a0,d2				; D2 = #Characters to output
	sub.l	a2,d2
	beq	C41				; It's a REPLICATE Run
	move.b	d2,(a4)+			; Save #Characters
	subq.l	#1,d2
C40
	move.b	(a2)+,(a4)+			; Copy LITERAL Characters
	dbf	d2,C40
C41
	move.l	a0,d2				; D2 = -#Characters to output
	sub.l	a3,d2
;	addq.l	#1,d2
	move.b	d2,(a4)+
	move.b	d3,(a4)+
	move.l	a3,a2
	bra	CAS1
;
;	LITERAL DUMP AND QUIT
;
CAS5
	move.l	a3,d2				; D2 = #Characters
	sub.l	a2,d2
	move.b	d2,(a4)+
	subq.l	#1,d2
C50
	move.b	(a2)+,(a4)+
	dbf	d2,C50
	bra	CAS8
;
;	LITERAL DUMP AND CONTINUE
;
CAS6
	move.l	a3,d2				; D2 = #Characters
	sub.l	a2,d2
	move.b	d2,(a4)+
	subq.l	#1,d2
C60
	move.b	(a2)+,(a4)+
	dbf	d2,C60
	bra	CAS1
;
;	LITERAL / REPEAT DUMP AND FINISH
;
CAS7
	move.l	a0,d2				; D2 = #Characters
	sub.l	a2,d2
	beq	C71				; REPLICATE
	move.b	d2,(a4)+
	subq.l	#1,d2
C70
	move.b	(a2)+,(a4)+
	dbf	d2,C70
C71
	move.l	a0,d2
	sub.l	a3,d2
;	addq.l	#1,d2
	move.b	d2,(a4)+
	move.b	d3,(a4)+
;
;	FINISHED
;
CAS8
	move.l	a4,d7				; D7 = #ImageStripBytes-1
	lea	WallStrip+1,a0
	sub.l	a0,d7
	movem.l	(sp)+,a3-a4
	rts


;
;	* * * * * * *       FIND WALL/IMAGE STRIP       * * * * * * *
;
;	D7		= Length of (Wall/Image Strip-1)
;	(WallStrip)	= Wall/Image Strip Data
;
;	D2		= Offset within Bank
;	D6		= (Bank*4)
;
FindWallImageStrip
	move.l	d7,d4					; D4 = BEST OFFSET.W/BEST LENGTH.W
	moveq.l	#-1,d5					; D5 = BEST BANK
	move.w	#(64*4),d6				; D6 = BANK
;
;	>>>   NEXT BANK   <<<
;
FWIS100
	subq.w	#4,d6					; Next Bank
	bmi	FWIS700
	lea	WallBankSizes,a0			; D3 = Size of BankData
	move.l	(a0,d6.w),d3
	bmi.s	FWIS100					; UnUsed/Invalid Bank
;
	move.l	WallDef,a1				; A1 = Start of BankData
	moveq.l	#0,d0
	move.w	d6,d0
	move.l	#(15-2),d1
	lsl.l	d1,d0
	add.l	d0,a1
	add.l	d3,a1					; A1 = End of BankData
;
	move.l	d3,d2					; D2 = Offset within BankData
;
;	>>>   NEXT BYTE   <<<
;
FWIS200
	subq.w	#1,d2					; Offset--
	bmi.s	FWIS100					; No more data in this bank
	subq.w	#1,a1
;
	move.w	d2,d0					; Enough room for this Strip?
	add.w	d7,d0
	bmi.s	FWIS200					; No!  It won't fit!
;
	move.w	d7,d0					; Any 4-Pixel Groups?
	addq.w	#1,d0
	lsr.w	#2,d0
	subq.w	#1,d0
	bmi.s	FWIS400					; No!
;
;	>>>   CHECK NEXT DATABASE STRIP (4-BYTE)   <<<
;
	move.l	d2,d1					; D1 = BankOffset
	move.l	a1,a0					; A0 = BankData
	lea	WallStrip,a2				; A2 = StripData
FWIS320
	addq.w	#4,d1
	cmp.w	d3,d1					; At/Past End of BankData?
	bcc.s	FWIS360					; Yes!
	cmpm.l	(a0)+,(a2)+				; Match?
	bne.s	FWIS200					; No, NextPixel
	dbf	d0,FWIS320				; Yes, keep checking
	move.w	d7,d0					; Get #Remainder Pixels
	addq.w	#1,d0
	and.w	#$3,d0
	bra.s	FWIS460
FWIS360
	subq.w	#4,d1					; Get (#Pixels Left to Match-1)
	move.w	d1,d0
	sub.w	d2,d0
	neg.w	d0
	addq.w	#1,d0
	add.w	d7,d0
	bra.s	FWIS460
;
;	>>>   CHECK NEXT DATABASE STRIP (1-BYTE)   <<<
;
FWIS400
	move.l	d7,d0					; D0 = #Pixels to Scan
	move.l	d2,d1					; D1 = BankOffset
	move.l	a1,a0					; A0 = BankData
	lea	WallStrip,a2				; A2 = StripData
FWIS420
	cmp.w	d3,d1					; At end of data?
	beq.s	FWIS500					; Yes!
	cmpm.b	(a0)+,(a2)+				; Match?
	bne.s	FWIS200					; No, NextPixel
	addq.w	#1,d1
FWIS460
	dbf	d0,FWIS420				; Yes, keep checking
	addq.l	#1,NumStripsOld
	bra	FWIS900					; Found an entire PixelStrip!
;
;	>>>   HOW MANY PIXELS MATCHED SO FAR?   <<<
;
FWIS500
	cmp.w	d0,d4					; Length < BestLength?
	ble.s	FWIS200					; No
	move.w	d2,d4					; NEW BESTOFFSET
	swap	d4
	move.w	d0,d4					; NEW BESTLENGTH
	move.w	d6,d5					; NEW BESTBANK
	bra.s	FWIS200					; NextPixel
;
;	>>>   HAVE WE FOUND A PLACE TO ADD THE UNMATCHED PIXELS?   <<<
;
FWIS700
	addq.l	#1,NumStripsPart
	move.l	d4,d2					; D2 = BEST OFFSET
	swap	d2
	and.l	#$ffff,d2
	move.w	d5,d6					; D6 = BEST BANK
	bpl.s	FWIS800					; Any BestBank?
;
;	>>>   FIND FIRST BANK THAT WILL FIT THIS PIXELSTRIP   <<<
;
	moveq.l	#-4,d6
FWIS720
	addq.w	#4,d6					; Next Bank
	lea	WallBankSizes,a0			; D2 = Size of BankData
	move.l	(a0,d6.w),d2
	bmi.s	FWIS720					; UnUsed/Invalid Bank
;
	move.w	d2,d0					; Enough room for this Strip?
	add.w	d7,d0
	bmi.s	FWIS720					; No!  It won't fit!
;
	subq.l	#1,NumStripsPart
	addq.l	#1,NumStripsNew
;
;	>>>   COPY REMAINING UNMATCHED PIXELS OF WALLSTRIP TO DATABASE   <<<
;
FWIS800
	move.l	WallDef,a0				; A0 = Start of BankData
	moveq.l	#0,d0
	move.w	d6,d0
	move.l	#(15-2),d1
	lsl.l	d1,d0
	add.l	d0,a0
	add.l	d2,a0
;
	move.l	d7,d0					; D0 = #Pixels to Copy
	lea	WallStrip,a2				; A2 = StripData
FWIS820
	move.b	(a2)+,(a0)+				; Copy Remaining Pixels
	dbf	d0,FWIS820
;
	move.l	d2,d0					; Get New Size of BankData
	add.l	d7,d0
	addq.l	#1,d0
	lea	WallBankSizes,a0
	move.l	(a0,d6.w),d1				; D1 = OldSize
	move.l	d0,(a0,d6.w)
	sub.l	d1,d0					; D0 = #Pixels Used
	add.l	d0,NumPixelsUsed
;
;	>>>   FOUND PIXELSTRIP / ADDED PIXELSTRIP   <<<
;
FWIS900
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
WallMsg		dc.b	'Wall',0
ImageMsg	dc.b	'Image',0
WallMsg0	dc.b	10
		dc.b	'Processing %5s #%03ld  <%s>',10
		dc.b	'File PathName          <%s>',10,0
WallMsg2	dc.b	'Dimensions             (%ldX,%ldY)',10,0
;WallMsg3	dc.b	'%ld',13,0
WallMsg4	dc.b	'%ld New, %ld Old, %ld Partial',10
		dc.b	'%ld Pixels, %ld PixelsUsed, %ld%%',10,0

WallTotalMsg	dc.b	10,'TOTALS: %ld Pixels, %ld Pixels Used, %ld%%',10,0

WallImgSectMsg	dc.b	9,'SECTION',9,'WALLIMGDEF%02lx,LOAD=$%08lx,ORG=$%08lx',10,0
WallImgImageMsg	dc.b	9,'IMAGE',9,'%s',10,0
		dc.w	0


;
;	* * * * * * *       VARIABLES       * * * * * * *
;
	section	__MERGED,DATA

WallBankSizes
		dc.l	-1,00,00,00,00,00,00,00		; $00-$07
		dc.l	00,00,00,00,00,00,-1,-1		; $08-$0f
		dc.l	-1,-1,-1,-1,-1,-1,-1,-1		; $10-$17
		dc.l	00,00,00,00,00,00,00,00		; $18-$1f
		dc.l	00,00,00,00,00,00,00,00		; $20-$27
		dc.l	00,00,00,00,00,00,00,00		; $28-$2f
		dc.l	00,00,00,00,00,00,00,00		; $30-$37
		dc.l	00,00,00,00,00,00,00,00		; $38-$3f

WallBankOldSizes
		ds.l	64


	end
