;***************************************************************************
;*                                                                         *
;*                          M A K E   T E X T U R E                        *
;*                                                                         *
;*                        TEXTURE DATA CREATION MODULE                     *
;*                                                                         *
;***************************************************************************

	include	mktex.i

	xref	DosBase
	xref	_LVOOpen,_LVOClose,_LVORead,_LVOWrite

	xref	Task
	xref	PrintMsg,VDTDebugOutC,ParseNum,ParseArg,ToUpper
	xref	MSGUserBreak,MSGNoTextureList
	xref	MSGWallListError,MSGTextureList2Error
	xref	MSGTextureDataError,MSGTextureTableError,MSGTextureTable2Error
	xref	MSGBadTexDim,MSGBadPatchDim

	xref	TextureListName,TextureList
	xref	TextureListName2,TextureList2
	xref	WallListName,WallList,WallListPtr
	xref	TextureDataName,TextureData,TextureDataPtr,TextureDataPtr0
	xref	TextureTableName,TextureTable,TextureTablePtr
	xref	TextureTable2,TextureTable2Name
	xref	TextureName,WallName,WallFileName
	xref	WallTranslation,TextureCount

	xref	MsgBuffer

	xref	TextureWidth,TextureHeight
	xref	WallWidth,WallHeight
	xref	PatchXOffset,PatchYOffset
	xref	WallPrefixName,WallPrefix2Name
	xref	SinglePatch


	section	MKTEX,CODE

	xdef	DoMakeTexture
	xdef	MakeATexture
	xdef	MakeAPatch


;
;	* * * * * * *       MAKE TEXTURE DATA FILES       * * * * * * *
;
DoMakeTexture
	move.l	DosBase,a6			; Open TEXTURELIST
	move.l	#TextureListName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGNoTextureList,d7
	move.l	d0,d4
	beq	DMT950
	move.l	d4,d1				; Read TEXTURELIST
	move.l	TextureList,d2
	move.l	#(64*1024),d3
	jsr	_LVORead(a6)
	move.l	d0,-(sp)
	move.l	d4,d1				; Close TEXTURELIST
	jsr	_LVOClose(a6)
	move.l	TextureList,a0			; Terminate with $00
	add.l	(sp)+,a0
	clr.b	(a0)
;
	move.l	TextureList,a5			; A5 = TEXTURE LIST
	move.l	TextureList2,a4			; A4 = TEXTURE LIST 2
	clr.w	TextureCount			; NO TEXTURES PROCESSED YET
DMT200
	move.l	#MSGUserBreak,d7		; Quit?
	move.l	Task,a0
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	DMT950				; Yes
	moveq.l	#0,d7
DMT300
	move.b	(a5)+,d0			; Any more TEXTURES?
	beq	DMT800				; No!
	move.b	d0,(a4)+
	cmp.b	#10,d0
	beq	DMT300
	cmp.b	#'<',d0				; Skip?
	bne.s	DMT380
DMT320
	move.b	(a5)+,d0			; Yes, Scan for EndSkip
	beq	DMT900
	move.b	d0,(a4)+
	cmp.b	#'>',d0
	bne.s	DMT320
	bra.s	DMT300
;
;	>>>   PARSE TEXTURE NAME   <<<
;
DMT380
	addq.w	#1,TextureCount			; ONE MORE TEXTURE PROCESSED
	subq.w	#1,a5
	subq.w	#1,a4
	lea	TextureName,a0			; A0 = Texture Name
DMT400
	move.b	(a5)+,d0
	move.b	d0,(a4)+
	move.b	d0,(a0)+
	beq	DMT900
	cmp.b	#',',d0
	beq.s	DMT420
	cmp.b	#10,d0
	bne.s	DMT400
DMT420
	clr.b	-1(a0)
;
;	>>>   PARSE TEXTURE DIMENSIONS/ALTERNATE IMAGERY   <<<
;
	move.l	a5,a0
DMT520
	move.b	(a0)+,d0
	beq	DMT950
	move.b	d0,(a4)+
	cmp.b	#10,d0
	bne.s	DMT520
;
	move.l	a5,a0
	move.l	#MSGBadTexDim,d7
	jsr	ParseNum			; Parse WIDTH
	bne	DMT950				; Invalid Number
	cmp.l	#8,d2				; <8?
	blt	DMT950
	move.l	d2,TextureWidth
;
	jsr	ParseArg
	jsr	ParseNum			; Parse HEIGHT
	bne	DMT950				; Invalid Number
	cmp.l	#8,d2				; <8?
	blt	DMT950
	move.l	d2,TextureHeight
;
	cmp.b	#',',d0				; ALTERNATE IMAGE?
	bne.s	DMT560				; No
DMT540
	move.b	(a0)+,d0			; Parse ALTERNATE IMAGE
	beq	DMT950
	cmp.b	#10,d0
	bne.s	DMT540
DMT560
	move.l	a0,a5
;
;	>>>   PARSE A SINGLE TEXTURE'S PATCHES   <<<
;
	move.l	TextureHeight,-(sp)
	move.l	TextureWidth,-(sp)
	pea	TextureName
	lea	TexMsg0,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(3*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	bsr	MakeATexture			; Make a Single Texture
;
;	>>>   COPY MISCELLANEOUS TEXT   <<<
;
DMT720
	move.b	(a5)+,d0
	beq	DMT900
	move.b	d0,(a4)+
	cmp.b	#10,d0
	bne.s	DMT720
;
	tst.l	d7
	beq	DMT200				; Do Next Texture
	bra	DMT900
;
;	>>>   MAKE TEXTURE ALTERNATE IMAGERY TRANSLATION TABLE   <<<
;
DMT800
	moveq.l	#0,d7
	move.w	WallTranslation,d7
	lea	TextureTable2,a3
	move.l	TextureList2,a5
DMT820
	move.l	a5,a0
	moveq.l	#0,d1				; WALL TRANSLATION
DMT830
	move.b	(a5)+,d0			; Skip TEXTURENAME and DIMENSIONS
	cmp.b	#10,d0
	bne.s	DMT830
	move.b	(a5),d0				; PATCH/TEXTURE TRANSLATION?
	cmp.b	#' ',d0
	beq.s	DMT850				; PATCH
DMT840
	move.b	(a5)+,d0			; TEXTURE TRANSLATION
	cmp.b	#10,d0
	bne.s	DMT840
	bra.s	DMT870
DMT850
	moveq.l	#1,d1				; WALL TEXTURE
	move.b	(a5),d0				; Skip ALL PATCHES
	cmp.b	#' ',d0
	bne.s	DMT870				; No more Patches!
DMT860
	move.b	(a5)+,d0			; Skip PATCH DIMENSIONS
	cmp.b	#10,d0
	bne.s	DMT860
	bra.s	DMT850
DMT870
	move.b	(a5)+,d0			; Skip WALL TRANSLATION
	cmp.b	#10,d0
	bne.s	DMT870
DMT880
	move.b	(a5)+,d0			; Skip MISCELLANEOUS TEXT
	cmp.b	#10,d0
	bne.s	DMT880
	tst.l	d1				; WALL TEXTURE OR TRANSLATION?
	beq.s	DMT820				; TRANSLATION
;
	move.l	a0,a1				; WALL TEXTURE!  FIND IT'S ALTERNATE!
DMT892
	move.b	(a1)+,d0			; Find Dimensions
	cmp.b	#',',d0
	bne.s	DMT892
DMT894
	move.b	(a1)+,d0			; Skip X Dimension
	cmp.b	#',',d0
	bne.s	DMT894
DMT896
	move.b	(a1)+,d0			; Skip Y Dimension
	cmp.b	#10,d0
	beq.s	DMT898
	cmp.b	#',',d0
	bne.s	DMT896
	move.l	a1,a0				; Found Alternate Texture!
DMT898
;
	moveq.l	#8-1,d1				; 8 Characters Maximum
	lea	TextureName,a1
XTE200
	move.b	(a0)+,d0			; Copy Texture Name
	cmp.b	#10,d0
	beq.s	XTE210
	cmp.b	#',',d0
	beq.s	XTE210
	jsr	ToUpper
	move.b	d0,(a1)+
	dbf	d1,XTE200
XTE210
	clr.b	(a1)				; Terminate Texture Name
;
;	>>>   SCAN TEXTURE LIST   <<<
;
XTE300
	lea	TextureName,a1
	move.l	TextureList2,a2
;
;	>>>   CHECK NEXT TEXTURE   <<<
;
XTE400
	tst.b	(a2)				; End of TEXTURELIST?
	beq	XTE900				; Yes!
	move.l	a1,a0				; A0 = TEXTURENAME
XTE500
	move.b	(a2)+,d0			; Does NAME Match?
	move.b	(a0)+,d1
	cmp.b	d0,d1
	beq.s	XTE500				; Yes, keep looking
	cmp.b	#',',d0				; At end of TEXTURELIST Name?
	bne.s	XTE600				; No, names don't match
	tst.b	d1				; At end of TEXTURE Name?
	beq.s	XTE700
;
XTE600
	move.b	(a2)+,d0			; Skip TEXTURENAME and DIMENSIONS
	cmp.b	#10,d0
	bne.s	XTE600
XTE610
	move.b	(a2)+,d0			; Skip FIRST PATCH/TEXTURE TRANSLATION
	cmp.b	#10,d0
	bne.s	XTE610
XTE620
	move.b	(a2),d0				; Skip ALL PATCHES
	cmp.b	#' ',d0
	bne.s	XTE660				; No more Patches!
XTE640
	move.b	(a2)+,d0			; Skip PATCH DIMENSIONS
	cmp.b	#10,d0
	bne.s	XTE640
	bra.s	XTE620
XTE660
	move.b	(a2)+,d0			; Skip WALL TRANSLATION
	cmp.b	#10,d0
	bne.s	XTE660
XTE680
	move.b	(a2)+,d0			; Skip MISCELLANEOUS TEXT
	cmp.b	#10,d0
	bne.s	XTE680
	bra.s	XTE400
;
;	>>>   FOUND A MATCH   <<<
;
XTE700
	move.b	(a2)+,d0			; Skip TEXTURE DIMENSIONS
	cmp.b	#10,d0
	bne.s	XTE700
	move.b	(a2),d0				; PATCHLIST OR TRANSLATION?
	cmp.b	#' ',d0
	beq.s	XTE720				; PATCHLIST
;
;	>>>   TEXTURE TRANSLATION   <<<
;
	move.l	a1,a0				; Copy TRANSLATION TEXTURE
XTE710
	move.b	(a2)+,d0
	cmp.b	#',',d0
	beq.s	XTE715
	cmp.b	#10,d0
	beq.s	XTE715
	move.b	d0,(a0)+
	bra.s	XTE710
XTE715
	clr.b	(a0)
	bra	XTE300				; RESCAN LIST!
;
;	>>>   PATCH LIST   <<<
;
XTE720
	move.b	(a2),d0				; Skip PATCH
	cmp.b	#' ',d0
	bne.s	XTE760				; No more Patches!
XTE740
	move.b	(a2)+,d0			; Skip PATCH DIMENSIONS
	cmp.b	#10,d0
	bne.s	XTE740
	bra.s	XTE720
XTE760
	move.l	a2,a0				; Get WALL TRANSLATION
	jsr	ParseNum
	bra.s	XTE980
;
;	>>>   DIDN'T FIND A MATCH   <<<
;
XTE900
	moveq.l	#-1,d2				; INVALID TEXTURE?!
;
XTE980
	lsl.w	#1,d2
	move.b	d2,(a3)+			; Save Translation
	lsr.w	#8,d2
	move.b	d2,(a3)+
	dbf	d7,DMT820
	moveq.l	#0,d7
;
;	>>>   COMPLETED MAKING TEXTURES   <<<
;
DMT900
	tst.l	d7				; Error?
	bne	DMT950				; Yes, don't save!
;
	move.l	DosBase,a6			; Save WALLLIST
	move.l	#WallListName,d1
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGWallListError,d7
	move.l	d0,d4
	beq	DMT950
	move.l	d4,d1
	move.l	WallList,d2
	move.l	WallListPtr,d3
	sub.l	d2,d3
	move.l	d3,-(sp)
	jsr	_LVOWrite(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
	move.l	(sp)+,d3
	cmp.l	d0,d3
	bne	DMT950
;
	move.l	#TextureDataName,d1		; Save TEXTUREDATA
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGTextureDataError,d7
	move.l	d0,d4
	beq	DMT950
	move.l	d4,d1
	move.l	TextureData,d2
	move.l	TextureDataPtr,d3
	sub.l	d2,d3
	move.l	d3,-(sp)
	jsr	_LVOWrite(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
	move.l	(sp)+,d3
	cmp.l	d0,d3
	bne	DMT950
;
	move.l	#TextureTableName,d1		; Save TEXTURETABLE
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGTextureTableError,d7
	move.l	d0,d4
	beq	DMT950
	move.l	d4,d1
	move.l	TextureTable,d2
	move.l	TextureTablePtr,d3
	sub.l	d2,d3
	move.l	d3,-(sp)
	jsr	_LVOWrite(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
	move.l	(sp)+,d3
	cmp.l	d0,d3
	bne	DMT950
;
	move.l	#TextureListName2,d1		; Save TEXTURELIST2
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGTextureList2Error,d7
	move.l	d0,d4
	beq	DMT950
	move.l	d4,d1
	move.l	TextureList2,d2
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
	bne	DMT950
;
	move.l	#TextureTable2Name,d1		; Save TEXTURETABLE2
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGTextureTable2Error,d7
	move.l	d0,d4
	beq	DMT950
	move.l	d4,d1
	lea	TextureTable2,a0
	move.l	a0,d2
	moveq.l	#0,d3
	move.w	WallTranslation,d3
	addq.l	#1,d3
	lsl.l	#1,d3
	move.l	d3,-(sp)
	jsr	_LVOWrite(a6)
	move.l	d0,-(sp)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	move.l	(sp)+,d0
	move.l	(sp)+,d3
	cmp.l	d0,d3
	bne	DMT950
;
	moveq.l	#0,d7
DMT950
	rts


;
;	* * * * * * *       MAKE SINGLE TEXTURE       * * * * * * *
;
MakeATexture
;
;	>>>   TRANSLATION OR PATCHLIST?   <<<
;
	move.b	(a5),d0				; Get FIRST Character
	cmp.b	#' ',d0				; PATCHLIST?
	beq.s	MAT100				; YES
	lea	WallName,a0			; A0 = WallName
MAT50
	move.b	(a5)+,d0			; Skip TRANSLATION TEXTURE
	beq	MAT70
	move.b	d0,(a4)+
	move.b	d0,(a0)+
	cmp.b	#10,d0
	bne.s	MAT50
MAT70
	clr.b	-1(a0)
	pea	WallName
	lea	TexMsg3,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
MAT80
	move.b	(a5)+,d0			; Skip Existing WallTranslation Text
	beq	MAT90
	cmp.b	#10,d0
	bne.s	MAT80
	move.b	#10,(a4)+
MAT90
	moveq.l	#0,d7
	rts
;
;	>>>   ADD ENTRY IN TEXTURETABLE   <<<
;
MAT100
	move.l	TextureTablePtr,a1		; Save Entry into TEXTURETABLE
	move.l	TextureDataPtr,d0
	sub.l	TextureData,d0
	move.b	d0,(a1)+
	lsr.w	#8,d0
	move.b	d0,(a1)+
	move.l	a1,TextureTablePtr
;
;	>>>   ADD ENTRY IN TEXTUREDATA   <<<
;
	move.l	TextureDataPtr,a0
	move.l	TextureWidth,d0			; TEXTURE WIDTH
	move.b	d0,(a0)+
	lsr.w	#8,d0
	move.b	d0,(a0)+
	move.l	TextureHeight,d0		; TEXTURE HEIGHT
	move.b	d0,(a0)+
	lsr.w	#8,d0
	move.b	d0,(a0)+
	clr.b	(a0)+				; #PATCHES
	move.l	a0,TextureDataPtr0
;
;	>>>   PARSE NEXT PATCH NAME   <<<
;
MAT200
	moveq.l	#0,d7
	move.b	(a5)+,d0			; Get next Character
	move.b	d0,(a4)+
	cmp.b	#' ',d0
	bne	MAT900				; No more Patches!
	lea	WallName,a0			; A0 = WallName
MAT400
	move.b	(a5)+,d0
	move.b	d0,(a4)+
	move.b	d0,(a0)+
	beq	MAT500
	cmp.b	#',',d0
	beq.s	MAT420
	cmp.b	#10,d0
	bne.s	MAT400
MAT420
	clr.b	-1(a0)
;
;	>>>   PARSE PATCH DIMENSIONS   <<<
;
MAT500
	move.l	a5,a0
MAT520
	move.b	(a0)+,d0
	beq	MAT900
	move.b	d0,(a4)+
	cmp.b	#10,d0
	bne.s	MAT520
;
	move.l	a5,a0
	move.l	#MSGBadPatchDim,d7
	jsr	ParseNum			; Parse X OFFSET
	bne	MAT900				; Invalid Number
	move.l	d2,PatchXOffset
;
	jsr	ParseArg
	jsr	ParseNum			; Parse Y Offset
	bne	MAT900				; Invalid Number
	move.l	d2,PatchYOffset
	move.l	a0,a5
;
;	>>>   PROCESS THIS PATCH   <<<
;
	move.l	PatchYOffset,-(sp)
	move.l	PatchXOffset,-(sp)
	pea	WallName
	lea	TexMsg1,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(3*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	bsr	MakeAPatch			; Process this Patch
	bra	MAT200				; Do Next Patch
;
;	>>>   CREATE NEW WALL TRANSLATION TEXT   <<<
;
MAT900
	subq.w	#1,a5
	subq.w	#1,a4
MAT920
	move.b	(a5)+,d0			; Skip Existing WallTranslation Text
	beq	MAT990
	cmp.b	#10,d0
	bne.s	MAT920
;
	moveq.l	#0,d0
	move.w	WallTranslation,d0		; Get Texture->Wall Translation
	move.l	d0,-(sp)			; Create WallTranslation Text
	lea	WallTranslationMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
;	addq.w	#4,sp
;
	lea	MsgBuffer,a0			; Copy WallTranslation Text
MAT940
	move.b	(a0)+,(a4)+
	bne.s	MAT940
	subq.w	#1,a4				; Move back overtop $00
;
	lea	TexMsg2,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	move.l	d7,-(sp)
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	(sp)+,d7
;
;	>>>   NO MORE PATCHES   <<<
;
MAT990
	move.l	TextureDataPtr0,d0		; Update TextureData Pointer
	move.l	d0,TextureDataPtr
	rts


;
;	* * * * * * *       MAKE SINGLE PATCH       * * * * * * *
;
MakeAPatch
	tst.b	SinglePatch			; SINGLE PATCH Mode?
	beq.s	MAP200				; No
	move.l	TextureDataPtr,a0		; One more Patch
	tst.b	4(a0)				; ANY Patches Yet?
	bne	MAP800				; Yes!  DONE!
MAP200
	move.l	TextureDataPtr0,a0
	move.l	PatchXOffset,d0			; PATCH X OFFSET
	move.b	d0,(a0)+
	lsr.w	#8,d0
	move.b	d0,(a0)+
	move.l	PatchYOffset,d0			; PATCH Y OFFSET
	move.b	d0,(a0)+
	lsr.w	#8,d0
	move.b	d0,(a0)+
	move.l	a0,TextureDataPtr0
	move.l	TextureDataPtr,a0		; One more Patch
	addq.b	#1,4(a0)
	bsr	AddWallName			; Add Wall Name to WallList
	move.w	d2,WallTranslation
	move.l	TextureDataPtr0,a0		; WALL#
	move.b	d2,(a0)+
	lsr.w	#8,d2
	move.b	d2,(a0)+
	move.l	a0,TextureDataPtr0
MAP800
	rts


;
;	* * * * * * *       ADD WALL PATCH NAME       * * * * * * *
;
AddWallName
	move.l	WallList,a0
	move.l	WallListPtr,a1
	moveq.l	#0,d2				; D2 = WALL NUMBER
AWN200
	lea	WallName,a2			; A2 = WallName
AWN300
	cmp.l	a1,a0				; At end of List?
	bge.s	AWN2000				; Yes!  New WallName!
	move.b	(a0)+,d0
	move.b	(a2)+,d1
	cmp.b	d0,d1				; Do Characters Match?
	beq.s	AWN300				; Yes, keep checking
	cmp.b	#10,d0				; End of WallListName?
	bne.s	AWN800				; No
	tst.b	d1				; End of WallName?
	bne.s	AWN900				; No
	bra.s	AWN1000				; Yes!  Found It!
AWN800
	move.b	(a0)+,d0			; Scan to end of WallName
	cmp.b	#10,d0
	bne.s	AWN800
AWN900
	move.b	(a0)+,d0			; Scan to end of WallFileName
	cmp.b	#10,d0
	bne.s	AWN900
	addq.w	#1,a0				; Skip BlankLine
	addq.w	#1,d2				; Next Wall#
	bra.s	AWN200
;
;	>>>   FOUND OLD WALLNAME   <<<
;
AWN1000
	rts
;
;	>>>   ADD NEW WALLNAME   <<<
;
AWN2000
	lea	WallName,a2			; A2 = WallName
AWN2100
	move.b	(a2)+,(a1)+
	bne.s	AWN2100
	move.b	#10,-1(a1)
;
	lea	WallName,a2			; A2 = WallName
	lea	WallPrefixName,a0		; DOOMIFF:WALLS/
	move.b	(a2),d0				; WallName starts with "_"?
	cmp.b	#'_',d0
	bne.s	AWN2200				; No, Normal Wall
	lea	WallPrefix2Name,a0		; RLART:WALLS/
AWN2200
	move.b	(a0)+,(a1)+
	bne.s	AWN2200
	subq.w	#1,a1
AWN2300
	move.b	(a2)+,(a1)+
	bne.s	AWN2300
	move.b	#10,-1(a1)
	move.b	#10,(a1)+			; Blank Line
	move.l	a1,WallListPtr
	rts


;
;	* * * * * * *       TEXT MESSAGES       * * * * * * *
;
TexMsg0			dc.b	10,'TEXTURE <%16s> (%ld,%ld)',10,0
TexMsg1			dc.b	'  PATCH <%16s> (%ld,%ld)',10,0
TexMsg2			dc.b	'   WALL (%ld)',10,0
TexMsg3			dc.b	'   XLAT <%16s>',10,0

WallTranslationMsg	dc.b	'%ld',10,0
 

		dc.w	0


	end
