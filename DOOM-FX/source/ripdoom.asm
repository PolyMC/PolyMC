;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                               INIT MODULE                               *
;*                                                                         *
;***************************************************************************

	include	ripdoom.i

	xref	_LVOOpenLibrary
	xref	_LVOCloseLibrary
	xref	_LVOAllocEntry
	xref	_LVOFreeEntry
	xref	_LVOOutput
	xref	_LVOOpen
	xref	_LVOClose
	xref	_LVORead
	xref	_LVOWrite
	xref	_LVOSeek
	xref	_LVOFindTask

	xref	VDTDebugOutC
	xref	PrintMsg
	xref	GetArgs0
	xref	Quiet,Verbose

	xref	DosName,DosBase
	xref	GraphicsName,GraphicsBase
	xref	IntuitionName,IntuitionBase
	xref	MathIEEEDoubBasName,MathIEEEDoubBasBase
	xref	MathIEEEDoubTransName,MathIEEEDoubTransBase

	xref	OutputFIB
	xref	argc
	xref	argv
	xref	SystemMemory
	xref	Task

	xref	DoomWADName,DoomPaletteName,RLTextureListName,RLFloorListName
	xref	DoomReMapPaletteName
	xref	DoomWADData,DoomWADDir,DoomPalette,RLPalette
	xref	DoomColourMap,RLColourMap
	xref	TextureList,FloorList
	xref	DoomPatchList,PatchList
	xref	DoomTexture1,DoomTexture2

	xref	RipData,ConvertWAD,ConvertFloor,ConvertImage,ConvertLevel,OutputName
	xref	ConvertPlayPal,ConvertColourMap,ConvertColourReMap,ConvertRGBReMap
	xref	PicDim,DoTextures

	xref	MSGNoMem
	xref	MSGInit
	xref	MSGNoOutputName
	xref	MSGDoomWADError

	xref	DoRipDoom,DoConvertLevel,DoConvertImage,DoConvertFloor
	xref	DoConvertPlayPal,DoConvertTextures

	xref	ImageBGReMap,ImageReMap,RoundRLPalette,GammaRLPalette

	xref	NoTextures2

	xref	WADVERTEXES
	xref	WADLINEDEFS
	xref	WADSIDEDEFS
	xref	WADNODES
	xref	WADSEGS
	xref	WADSSECTORS
	xref	WADSECTORS
	xref	WADREJECT
	xref	WADBLOCKMAP
	xref	WADTHINGS,WADTHINGS2
;
	xref	RLVERTEXES
	xref	RLLINES
	xref	RLSECTORS
	xref	RLBSP
	xref	RLAREAS
	xref	RLSEGS
	xref	RLFACES
	xref	RLOBJECTS
	xref	RLBLOCKMAP
	xref	RLREJECT
	xref	RLDOORS
	xref	RLSTAIRS
	xref	RLFLOORS
	xref	RLLIFTS
	xref	RLCEILINGS


	section	INIT,CODE

	xdef	RipDoom

;
;	* * * * * * *       RIP DOOM .WAD DATA FILE       * * * * * * *
;
RipDoom
	move.l	a0,argc				; Save pointer to Argc
	sub.l	a1,a1				; Find our Task Address
	move.l	4,a6
	jsr	_LVOFindTask(a6)
	move.l	d0,Task
	lea	DosName,a1			; dos.library
	moveq.l	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,DosBase
	lea	GraphicsName,a1			; graphics.library
	moveq.l	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,GraphicsBase
	lea	IntuitionName,a1		; intuition.library
	moveq.l	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,IntuitionBase
	lea	MathIEEEDoubBasName,a1		; mathieeedoubbas.library
	moveq.l	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,MathIEEEDoubBasBase
	lea	MathIEEEDoubTransName,a1	; mathieeedoubtrans.library
	moveq.l	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,MathIEEEDoubTransBase
;
	move.l	DosBase,a6
	jsr	_LVOOutput(a6)
	move.l	d0,OutputFIB			; Save OutputFIB
;
	bsr	GetArgs0			; Fill the Arguments array
	tst.l	d2
	bne	DoScanArgs
	move.l	#MSGInit,d7			; Show Init Message
	bsr	PrintMsg
	bra	Exit				; No arguments
DoScanArgs
	lea	argv,a1				; A1 points to list
ScanArgs
	move.l	(a1)+,d0			; D0 points to Argument
	beq	DoneArgs
	move.l	d0,a0				; A0 points to Argument
	move.b	(a0)+,d0
	cmp.b	#'-',d0				; Option specifier?
	bne	ScanArgs			; Nope
	move.b	(a0)+,d0
	cmp.b	#'L',d0				; -L	LEVEL NAME?
	bne	ScanArgs100
	lea	ConvertLevel,a0
SaveArg
	move.l	-4(a1),d0
	addq.l	#2,d0
	move.l	d0,(a0)
	bra	ScanArgs
ScanArgs100
	cmp.b	#'V',d0				; -V	VERBOSE
	bne	ScanArgs200
	move.b	#-1,Verbose
	bra	ScanArgs
ScanArgs200
	cmp.b	#'Q',d0				; -Q	QUIET
	bne	ScanArgs300
	move.b	#-1,Quiet
	bra	ScanArgs
ScanArgs300
	cmp.b	#'R',d0				; -R	RIPDATA
	bne	ScanArgs400
	move.b	#-1,RipData
ScanArgs400
	cmp.b	#'F',d0				; -F	FLOOR NAME?
	beq	ScanArgs450
	cmp.b	#'G',d0				; -G	FLOOR NAME (REMAP)?
	bne	ScanArgs500
	move.b	#-1,ImageBGReMap
ScanArgs450
	lea	ConvertFloor,a0
	bra	SaveArg
ScanArgs500
	cmp.b	#'I',d0				; -I	IMAGE NAME?
	beq	ScanArgs550
	cmp.b	#'J',d0				; -J	IMAGE NAME (REMAP)?
	bne	ScanArgs600
	move.b	#-1,ImageBGReMap
ScanArgs550
	lea	ConvertImage,a0
	bra	SaveArg
ScanArgs600
	cmp.b	#'O',d0				; -O	OUTPUT NAME?
	bne	ScanArgs700
	lea	OutputName,a0
	bra	SaveArg
ScanArgs700
	cmp.b	#'P',d0				; -P	PLAYPAL?
	bne	ScanArgs730
	lea	ConvertPlayPal,a0
	bra	SaveArg
ScanArgs730
	cmp.b	#'C',d0				; -C	COLOURMAP?
	bne	ScanArgs750
	lea	ConvertColourMap,a0
	bra	SaveArg
ScanArgs750
	cmp.b	#'K',d0				; -K	COLOURREMAP?
	bne	ScanArgs780
	lea	ConvertColourReMap,a0
	bra	SaveArg
ScanArgs780
	cmp.b	#'M',d0				; -M	RGBREMAP?
	bne	ScanArgs800
	lea	ConvertRGBReMap,a0
	bra	SaveArg
ScanArgs800
	cmp.b	#'D',d0				; -D	DIMENSIONS?
	bne	ScanArgs900
	lea	PicDim,a0
	bra	SaveArg
ScanArgs900
	cmp.b	#'T',d0				; -T	CONVERT TEXTURES/PATCHLIST
	bne	ScanArgs1000
	move.b	#-1,DoTextures
ScanArgs1000
	cmp.b	#'X',d0				; -X	REMAP PALETTE NAME?
	bne	ScanArgs1100
	move.b	#-1,ImageReMap
	lea	DoomReMapPaletteName,a0
	bra	SaveArg
ScanArgs1100
	cmp.b	#'U',d0				; -U	ROUND RLPALETTE
	bne	ScanArgs1150
	move.b	#-1,RoundRLPalette
ScanArgs1150
	cmp.b	#'A',d0				; -A	GAMMACORRECT RLPALETTE
	bne	ScanArgs1200
	move.b	#-1,GammaRLPalette
ScanArgs1200
	cmp.b	#'H',d0				; -H	FLOOR/CEILING TEXTURES/COLOUR?
	bne	ScanArgs1300
	move.b	#-1,NoTextures2
ScanArgs1300
	cmp.b	#'W',d0				; -W	DOOM.WAD FILENAME
	bne	ScanArgs1400
	lea	ConvertWAD,a0
	bra	SaveArg
ScanArgs1400
	bra	ScanArgs

DoneArgs
	tst.b	Quiet
	bne	DoneArgs100
	move.l	#MSGInit,d7			; Show Init Message
	bsr	PrintMsg
DoneArgs100
	move.l	4,a6
	lea	MemoryList,a0			; A0 = MemList
	jsr	_LVOAllocEntry(a6)
	move.l	#MSGNoMem,d7
	bclr	#31,d0				; Bit31 = 0 = OK, = 1 = BAD!
	bne	Error	
	move.l	d0,SystemMemory
	move.l	d0,a0
	lea	$10(a0),a0			; A0 points to MemList
	move.l	(a0),DoomWADData
	addq.w	#8,a0
	move.l	(a0),DoomWADDir
	addq.w	#8,a0
	move.l	(a0),DoomPalette
	addq.w	#8,a0
	move.l	(a0),DoomPatchList
	addq.w	#8,a0
	move.l	(a0),DoomTexture1
	addq.w	#8,a0
	move.l	(a0),DoomTexture2
	addq.w	#8,a0
	move.l	(a0),FloorList
	addq.w	#8,a0
	move.l	(a0),PatchList
	addq.w	#8,a0
	move.l	(a0),TextureList
;
	addq.w	#8,a0
	move.l	(a0),WADVERTEXES
	addq.w	#8,a0
	move.l	(a0),WADLINEDEFS
	addq.w	#8,a0
	move.l	(a0),WADSIDEDEFS
	addq.w	#8,a0
	move.l	(a0),WADNODES
	addq.w	#8,a0
	move.l	(a0),WADSEGS
	addq.w	#8,a0
	move.l	(a0),WADSSECTORS
	addq.w	#8,a0
	move.l	(a0),WADSECTORS
	addq.w	#8,a0
	move.l	(a0),WADREJECT
	addq.w	#8,a0
	move.l	(a0),WADBLOCKMAP
	addq.w	#8,a0
	move.l	(a0),WADTHINGS
	addq.w	#8,a0
	move.l	(a0),WADTHINGS2
;
	addq.w	#8,a0
	move.l	(a0),RLVERTEXES
	addq.w	#8,a0
	move.l	(a0),RLLINES
	addq.w	#8,a0
	move.l	(a0),RLSECTORS
	addq.w	#8,a0
	move.l	(a0),RLBSP
	addq.w	#8,a0
	move.l	(a0),RLAREAS
	addq.w	#8,a0
	move.l	(a0),RLSEGS
	addq.w	#8,a0
	move.l	(a0),RLFACES
	addq.w	#8,a0
	move.l	(a0),RLOBJECTS
	addq.w	#8,a0
	move.l	(a0),RLBLOCKMAP
	addq.w	#8,a0
	move.l	(a0),RLREJECT
	addq.w	#8,a0
	move.l	(a0),RLDOORS
	addq.w	#8,a0
	move.l	(a0),RLSTAIRS
	addq.w	#8,a0
	move.l	(a0),RLFLOORS
	addq.w	#8,a0
	move.l	(a0),RLLIFTS
	addq.w	#8,a0
	move.l	(a0),RLCEILINGS
;
	addq.w	#8,a0
	move.l	(a0),RLPalette
	addq.w	#8,a0
	move.l	(a0),DoomColourMap
	addq.w	#8,a0
	move.l	(a0),RLColourMap
;
	tst.b	RipData				; Rip Data?
	beq	DoneArgs200
	move.l	#MSGDoomWADError,d7
	tst.l	ConvertWAD			; WAD FileName?
	beq	Error
	jsr	DoRipDoom			; Yes
	tst.l	d7				; Any error msg?
	bne	Error				; Yes!
	bra	DoneArgs700
DoneArgs200
	tst.l	ConvertFloor			; Convert Floor?
	beq	DoneArgs300
	move.l	#MSGNoOutputName,d7
	tst.l	OutputName
	beq	Error
	jsr	DoConvertFloor
	tst.l	d7				; Any error msg?
	bne	Error				; Yes!
DoneArgs300
	tst.l	ConvertImage			; Convert Image?
	beq	DoneArgs400
	move.l	#MSGNoOutputName,d7
	tst.l	OutputName
	beq	Error
	jsr	DoConvertImage
	tst.l	d7				; Any error msg?
	bne	Error				; Yes!
DoneArgs400
	tst.l	ConvertLevel			; Convert Level?
	beq	DoneArgs500
	move.l	#MSGNoOutputName,d7
	tst.l	OutputName
	beq	Error
	jsr	DoConvertLevel
	tst.l	d7				; Any error msg?
	bne	Error				; Yes!
DoneArgs500
	tst.l	ConvertPlayPal			; Convert PlayPalette?
	beq	DoneArgs600
	move.l	#MSGNoOutputName,d7
	tst.l	ConvertColourMap		; ColourMap Name Specified?
	beq	Error
	tst.l	ConvertColourReMap		; ColourReMap Name Specified?
	beq	Error
	tst.l	ConvertRGBReMap			; RGBReMap Name Specified?
	beq	Error
	jsr	DoConvertPlayPal
	tst.l	d7				; Any error msg?
	bne	Error				; Yes!
DoneArgs600
	tst.b	DoTextures			; Convert Textures/PatchList?
	beq	DoneArgs700
	move.l	#MSGNoOutputName,d7
	tst.l	OutputName
	beq	Error
	jsr	DoConvertTextures
	tst.l	d7				; Any error msg?
	bne	Error				; Yes!
DoneArgs700
	bra	Exit
;
;	ERROR/EXIT	--	Close all, DeAllocate all, PrintMsg if any, and Exit!
;
Error
	bsr	PrintMsg
	bsr	Exit
Error900
	moveq.l	#10,d0
	rts
Exit
	move.l	4,a6
Exit300
	lea	SystemMemory,a0			; Free memory allocated
	tst.l	(a0)
	beq	Exit400
	move.l	(a0),a0
	jsr	_LVOFreeEntry(a6)
Exit400
	move.l	MathIEEEDoubTransBase,a1	; mathieeedoubtrans.library
	jsr	_LVOCloseLibrary(a6)
	move.l	MathIEEEDoubBasBase,a1		; mathieeedoubbas.library
	jsr	_LVOCloseLibrary(a6)
	move.l	IntuitionBase,a1		; intuition.library
	jsr	_LVOCloseLibrary(a6)
	move.l	GraphicsBase,a1			; graphics.library
	jsr	_LVOCloseLibrary(a6)
	move.l	DosBase,a1			; dos.library
	jsr	_LVOCloseLibrary(a6)
	moveq.l	#0,d0
	rts


;
;	LIST OF MEMORY REQUIREMENTS
;
MemoryList
	ds.b	$e					; ListNode at top
	dc.w	(MemoryListEnd-MemoryListStart)/8	; # of Memory Entries
MemoryListStart
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; DOOM.WAD DATA
	dc.l	(1024*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; DOOM.WAD DIRECTORY DATA
	dc.l	(4096*16)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; DOOM PALETTE DATA
	dc.l	(256*3*14)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; DOOM PNAMES
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; DOOM TEXTURE1
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; DOOM TEXTURE2
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL FLOOR LIST
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL PATCH LIST
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL TEXTURES LIST
	dc.l	(64*1024)
;
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; WAD VERTEXES
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; WAD LINEDEFS
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; WAD SIDEDEFS
	dc.l	(128*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; WAD NODES
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; WAD SEGS
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; WAD SSECTORS
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; WAD SECTORS
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; WAD REJECT
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; WAD BLOCKMAP
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; WAD THINGS
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; WAD THINGS2
	dc.l	(32*1024)
;
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL VERTEXES
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL LINES
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL SECTORS
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL BSP
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL AREAS
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL SEGS
	dc.l	(64*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL FACES
	dc.l	(128*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL OBJECTS
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL BLOCKMAP
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL REJECT
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL DOORS
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL STAIRS
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL FLOORS
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL LIFTS
	dc.l	(32*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL CEILINGS
	dc.l	(32*1024)
;
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL PALETTE DATA
	dc.l	(256*2*14)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; DOOM COLOURMAP DATA
	dc.l	(256*34)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; RL COLOURMAP DATA
	dc.l	(256*34)
MemoryListEnd


	end
