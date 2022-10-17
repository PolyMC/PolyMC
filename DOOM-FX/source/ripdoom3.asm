;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                         CONVERT LEVEL MODULE                            *
;*                                                                         *
;***************************************************************************

	include	ripdoom.i


	xref	DosBase
	xref	_LVOOpen,_LVOClose,_LVOSeek,_LVORead,_LVOWrite

	xref	MathIEEEDoubBasBase,MathIEEEDoubTransBase
	xref	_LVOIEEEDPFix,_LVOIEEEDPFlt,_LVOIEEEDPSqrt,_LVOIEEEDPAdd,_LVOIEEEDPDiv
	xref	_LVOIEEEDPAtan,_LVOIEEEDPMul

	xref	GraphicsBase
	xref	_LVOSetAPen,_LVOMove,_LVODraw

	xref	Task

	xref	DoomWADData

	xref	PrintMsg,VDTDebugOutC,ParseNum,ParseArg,ToUpper

	xref	ConvertLevel,OutputName,DoomFileName

	xref	OpenPictureSNES,WriteIFF,ClosePicture
	xref	SetPicPlanes
	xref	PicX,PicY,PicXBytes,PicNumPlanes,PicPalAmiga
	xref	PicPlanes,PicRPort
	xref	PicDim,PicDimWidth,PicDimHeight,PicDimPlanes

	xref	Texture1Name,Texture2Name,Texture3Name,TextureXOffset,TextureYOffset

	xref	RLTextureListName,TextureList
	xref	RLFloorListName,FloorList

	xref	NoTextures2

	xref	MSGRLTextureListError,MSGRLFloorListError
	xref	MSGPicError,MSGMaxFObjError,MSGMaxMObjError

	xref	DoomReMapPalette
	xref	WADVERTEXES,WADVERTEXESSize
	xref	WADLINEDEFS,WADLINEDEFSSize
	xref	WADSIDEDEFS,WADSIDEDEFSSize
	xref	WADNODES,WADNODESSize
	xref	WADSEGS,WADSEGSSize
	xref	WADSSECTORS,WADSSECTORSSize
	xref	WADSECTORS,WADSECTORSSize
	xref	WADREJECT,WADREJECTSize
	xref	WADBLOCKMAP,WADBLOCKMAPSize
	xref	WADTHINGS,WADTHINGS2,WADTHINGSSize,WADTHINGS2Size
;
	xref	RLVERTEXES,RLVERTEXESSize
	xref	RLLINES,RLLINESSize
	xref	RLSECTORS,RLSECTORSSize,RLSECTOROrigins
	xref	RLBSP,RLBSPSize
	xref	RLAREAS,RLAREASSize
	xref	RLSEGS,RLSEGSSize
	xref	RLFACES,RLFACESSize,RLFACESPtrs,RLFACESFlags
	xref	RLOBJECTS,RLOBJECTSSize,RLNUMOBJECTS
	xref	RLBLOCKMAP,RLBLOCKMAPSize
	xref	RLDOORS,RLDOORSSize,RLNUMDOORS
	xref	RLSTAIRS,RLSTAIRSSize,RLNUMSTAIRS
	xref	RLFLOORS,RLFLOORSSize,RLNUMFLOORS
	xref	RLLIFTS,RLLIFTSSize,RLNUMLIFTS
	xref	RLCEILINGS,RLCEILINGSSize,RLNUMCEILINGS
	xref	RLREJECT,RLREJECTSize
;
	xref	RLOBJReMapTable


	section	LEVEL,CODE

	xdef	DoConvertLevel
	xdef	GetLevelIFFDim
	xdef	ConvertVERTEXES
	xdef	ConvertLINEDEFS
	xdef	ConvertSECTORS
	xdef	ConvertNODES
	xdef	ConvertSEGS
	xdef	ConvertSIDEDEFS
	xdef	ConvertBLOCKMAP
	xdef	ConvertTHINGS
	xdef	SortTHINGS
	xdef	ConvertDOORS
	xdef	ConvertSTAIRS
	xdef	ConvertFLOORS
	xdef	ConvertLIFTS
	xdef	ConvertCEILINGS
	xdef	OptimizeLEVEL
	xdef	OptimizeFACES
	xdef	XREFTexture
	xdef	XREFTexture2
	xdef	GetSectorHEF
	xdef	GetSectorLEF
	xdef	GetSectorNHEF
	xdef	GetSectorLIC
	xdef	GetSectorLIF
	xdef	GetSectorHIF

	xdef	GetAngle


;
;	* * * * * * *       CONVERT DOOM LEVEL       * * * * * * *
;
DoConvertLevel
	bsr	ReadTextureList			; Read TEXTURELIST
	bne	DCLL900				; Error!
	bsr	ReadFloorList			; Read FLOORLIST
	bne	DCLL900				; Error!
	bsr	GetLevelIFFDim			; Get Level IFF Dimensions
	bsr	ConvertLoadLevel		; Load Level Data
	bne	DCLL900
	jsr	SetLevelIFFPalette		; Set Default PALETTE
	move.w	PicDimWidth,d0
	move.w	d0,PicX
	lsr.w	#3,d0
	move.w	d0,PicXBytes
	move.w	PicDimHeight,PicY
	move.w	PicDimPlanes,PicNumPlanes
	jsr	OpenPictureSNES			; Open the picture
	move.l	#MSGPicError,d7
	tst.l	d0
	bne	DCLL900				; Error!
	bsr	RLEngineStats			; Engine Stats Information
	bsr	ConvertVERTEXES			; Convert VERTEXES -> VERTEXES
	bsr	ConvertLINEDEFS			; Convert LINEDEFS -> LINES
	bsr	ConvertSECTORS			; Convert SECTORS -> SECTORS
	bsr	ConvertNODES			; Convert NODES -> BSP
	bsr	ConvertSIDEDEFS			; Convert SIDEDEFS -> FACES
	bsr	ConvertSEGS			; Convert SEGS -> SEGS
	bsr	ConvertSSECTORS			; Convert SSECTORS -> AREAS
	bsr	ConvertBLOCKMAP			; Convert BLOCKMAP -> BLOCKMAP
	bsr	SortTHINGS			; Sort THINGS -> THINGS2
	bsr	ConvertTHINGS			; Convert THINGS -> OBJECTS
	bne	DCLL800				; Error!
	bsr	ConvertDOORS			; Convert ? -> DOORS
	bsr	ConvertSTAIRS			; Convert ? -> STAIRS
	bsr	ConvertFLOORS			; Convert ? -> FLOORS
	bsr	ConvertLIFTS			; Convert ? -> LIFTS
	bsr	ConvertCEILINGS			; Convert ? -> CEILINGS
;	bsr	OptimizeLEVEL			; Optimize LEVEL
	bsr	ConvertSaveLevel		; Save Level Data
	move.l	#RLIFFSuffix,a2			; Save IFF
	bsr	ConvertAddRLSuffix		; Add Suffix to RL Base
	move.l	#DoomFileName,d1
	moveq.l	#0,d4
	move.w	PicX,d4
	moveq.l	#0,d5
	move.w	PicY,d5
	jsr	WriteIFF
	moveq.l	#0,d7				; NO ERRORS!
DCLL800
	move.l	d7,-(sp)
	jsr	ClosePicture
	move.l	(sp)+,d7
DCLL900
	rts


;
;	* * * * * * *       READ RL TEXTURE LIST       * * * * * * *
;
ReadTextureList
	move.l	DosBase,a6				; Open TEXTURELIST
	move.l	#RLTextureListName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGRLTextureListError,d7
	move.l	d0,d4
	beq	RWL800
	move.l	d4,d1					; Read TEXTURELIST
	move.l	TextureList,d2
	move.l	#(64*1024),d3
	jsr	_LVORead(a6)
	move.l	TextureList,a0				; TERMINATE!
	add.l	d0,a0
	clr.b	(a0)
	move.l	d4,d1					; Close TEXTURELIST
	jsr	_LVOClose(a6)
	moveq.l	#0,d7
RWL800
	tst.l	d7
	rts


;
;	* * * * * * *       READ RL FLOOR LIST       * * * * * * *
;
ReadFloorList
	move.l	DosBase,a6				; Open FLOORLIST
	move.l	#RLFloorListName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGRLFloorListError,d7
	move.l	d0,d4
	beq	RFL800
	move.l	d4,d1					; Read FLOORLIST
	move.l	FloorList,d2
	move.l	#(64*1024),d3
	jsr	_LVORead(a6)
	move.l	FloorList,a0				; TERMINATE!
	add.l	d0,a0
	clr.b	(a0)
	move.l	d4,d1					; Close FLOORLIST
	jsr	_LVOClose(a6)
	moveq.l	#0,d7
RFL800
	tst.l	d7
	rts


;
;	* * * * * * *       PARSE LEVEL IFF DIMENSIONS       * * * * * * *
;
GetLevelIFFDim
	move.l	PicDim,d0			; A0 = Level IFF Dimensions
	beq	GLID800				; No Dimensions Specified
	move.l	d0,a0
	jsr	ParseNum			; Parse WIDTH
	bne	GLID800				; Invalid Number
	cmp.w	#256,d2				; <256?
	blt	GLID800
	move.w	d2,PicDimWidth
	jsr	ParseArg
	jsr	ParseNum			; Parse HEIGHT
	bne	GLID800				; Invalid Number
	cmp.w	#256,d2				; <256?
	blt	GLID800
	move.w	d2,PicDimHeight
	jsr	ParseArg
	jsr	ParseNum			; Parse #PLANES
	bne	GLID800				; Invalid Number
	cmp.w	#1,d2				; <1?
	blt	GLID800
	cmp.w	#8,d2				; >8?
	bgt	GLID800
	move.w	d2,PicDimPlanes
	rts
GLID800
	move.w	#1024,PicDimWidth		; Defaults for PicDimensions
	move.w	#1024,PicDimHeight
	move.w	#8,PicDimPlanes
	rts


;
;	* * * * * * *       SET DEFAULT LEVEL IFF PALETTE       * * * * * * *
;
SetLevelIFFPalette
	lea	PicPalAmiga,a0
	lea	DoomReMapPalette,a1
	move.w	#$0000,(a0)+			; BLACK
	move.b	#$00,(a1)+
	move.b	#$00,(a1)+
	move.b	#$00,(a1)+
	move.w	#$0fff,(a0)+			; WHITE
	move.b	#$ff,(a1)+
	move.b	#$ff,(a1)+
	move.b	#$ff,(a1)+
	move.w	#$0888,(a0)+			; GREY
	move.b	#$80,(a1)+
	move.b	#$80,(a1)+
	move.b	#$80,(a1)+
	rts


;
;	* * * * * * *       CONVERT DOOM LEVEL - LOAD WAD DATA       * * * * * * *
;
ConvertLoadLevel
	move.l	#WADVERTEXESSuffix,a2			; Load VERTEXES
	move.l	WADVERTEXES,a3
	bsr	ConvertLoadFile
	bne	CLL800
	move.l	a4,WADVERTEXESSize
;
	move.l	#WADLINEDEFSSuffix,a2			; Load LINEDEFS
	move.l	WADLINEDEFS,a3
	bsr	ConvertLoadFile
	bne	CLL800
	move.l	a4,WADLINEDEFSSize
;
	move.l	#WADSIDEDEFSSuffix,a2			; Load SIDEDEFS
	move.l	WADSIDEDEFS,a3
	bsr	ConvertLoadFile
	bne	CLL800
	move.l	a4,WADSIDEDEFSSize
;
	move.l	#WADNODESSuffix,a2			; Load NODES
	move.l	WADNODES,a3
	bsr	ConvertLoadFile
	bne	CLL800
	move.l	a4,WADNODESSize
;
	move.l	#WADSEGSSuffix,a2			; Load SEGS
	move.l	WADSEGS,a3
	bsr	ConvertLoadFile
	bne	CLL800
	move.l	a4,WADSEGSSize
;
	move.l	#WADSSECTORSSuffix,a2			; Load SSECTORS
	move.l	WADSSECTORS,a3
	bsr	ConvertLoadFile
	bne	CLL800
	move.l	a4,WADSSECTORSSize
;
	move.l	#WADSECTORSSuffix,a2			; Load SECTORS
	move.l	WADSECTORS,a3
	bsr	ConvertLoadFile
	bne	CLL800
	move.l	a4,WADSECTORSSize
;
	move.l	#WADREJECTSuffix,a2			; Load REJECT
	move.l	WADREJECT,a3
	bsr	ConvertLoadFile
	bne	CLL800
	move.l	a4,WADREJECTSize
;
	move.l	#WADBLOCKMAPSuffix,a2			; Load BLOCKMAP
	move.l	WADBLOCKMAP,a3
	bsr	ConvertLoadFile
	bne	CLL800
	move.l	a4,WADBLOCKMAPSize
;
	move.l	#WADTHINGSSuffix,a2			; Load THINGS
	move.l	WADTHINGS,a3
	bsr	ConvertLoadFile
	bne	CLL800
	move.l	a4,WADTHINGSSize
;
CLL800
	tst.l	d7
	rts

;
;	* * * * * * *       CONVERT DOOM LEVEL - LOAD FILE       * * * * * * *
;
;	A2 = FileName Suffix
;	A3 = DataBuffer
;
;	A4 = Length
;
ConvertLoadFile
	bsr	ConvertAddWADSuffix			; Add Suffix to WAD Base
;
	move.l	#DoomFileName,d0			; Generate FileName to ErrorMsg
	move.l	d0,-(sp)
	lea	ConvertFileErrorMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
;
	move.l	DosBase,a6
	move.l	#DoomFileName,d1			; Open File
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#-1,d7
	move.l	d0,d4
	beq	CLFE800
	move.l	d4,d1					; Read File
	move.l	a3,d2
	move.l	#131072,d3
	jsr	_LVORead(a6)
	move.l	d0,a4					; Save Length
	move.l	d4,d1					; Close File
	jsr	_LVOClose(a6)
	moveq.l	#0,d7
CLFE800
	tst.l	d7
	rts

;
;	* * * * * * *       CONVERT DOOM LEVEL - SAVE RL DATA       * * * * * * *
;
ConvertSaveLevel
	move.l	#RLVERTEXESSuffix,a2			; Save VERTEXES
	move.l	RLVERTEXES,a3
	move.l	RLVERTEXESSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLLINESSuffix,a2			; Save LINES
	move.l	RLLINES,a3
	move.l	RLLINESSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLSECTORSSuffix,a2			; Save SECTORS
	move.l	RLSECTORS,a3
	move.l	RLSECTORSSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLBSPSuffix,a2				; Save BSP
	move.l	RLBSP,a3
	move.l	RLBSPSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLAREASSuffix,a2			; Save AREAS
	move.l	RLAREAS,a3
	move.l	RLAREASSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLSEGSSuffix,a2			; Save SEGS
	move.l	RLSEGS,a3
	move.l	RLSEGSSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLFACESSuffix,a2			; Save FACES
	move.l	RLFACES,a3
	move.l	RLFACESSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLOBJECTSSuffix,a2			; Save OBJECTS
	move.l	RLOBJECTS,a3
	move.l	RLOBJECTSSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLBLOCKMAPSuffix,a2			; Save BLOCKMAP
	move.l	RLBLOCKMAP,a3
	move.l	RLBLOCKMAPSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLREJECTSuffix,a2			; Save REJECT
	move.l	WADREJECT,a3
	move.l	WADREJECTSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLDOORSSuffix,a2			; Save DOORS
	move.l	RLDOORS,a3
	move.l	RLDOORSSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLSTAIRSSuffix,a2			; Save STAIRS
	move.l	RLSTAIRS,a3
	move.l	RLSTAIRSSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLFLOORSSuffix,a2			; Save FLOORS
	move.l	RLFLOORS,a3
	move.l	RLFLOORSSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLLIFTSSuffix,a2			; Save LIFTS
	move.l	RLLIFTS,a3
	move.l	RLLIFTSSize,a4
	bsr	ConvertSaveFile
	bne	CSL800
;
	move.l	#RLCEILINGSSuffix,a2			; Save CEILINGS
	move.l	RLCEILINGS,a3
	move.l	RLCEILINGSSize,a4
	bsr	ConvertSaveFile
;	bne	CSL800
CSL800
	rts

;
;	* * * * * * *       CONVERT DOOM LEVEL - SAVE FILE       * * * * * * *
;
;	A2 = FileName Suffix
;	A3 = DataBuffer
;	A4 = Length
;
ConvertSaveFile
	bsr	ConvertAddRLSuffix			; Add Suffix to RL Base
;
	move.l	#DoomFileName,d0			; Generate FileName to ErrorMsg
	move.l	d0,-(sp)
	lea	ConvertFileErrorMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
;
	move.l	DosBase,a6
	move.l	#DoomFileName,d1			; Open File
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#-1,d7
	move.l	d0,d4
	beq	CSFE800
	move.l	d4,d1					; Read File
	move.l	a3,d2
	move.l	a4,d3
	jsr	_LVOWrite(a6)
	move.l	d4,d1					; Close File
	jsr	_LVOClose(a6)
	moveq.l	#0,d7
CSFE800
	tst.l	d7
	rts


;
;	* * * * * * *       CONVERT DOOM LEVEL ADD SUFFIX       * * * * * * *
;
;	A2 = Suffix to Add
;
ConvertAddWADSuffix
	move.l	ConvertLevel,a1				; A1 = Base Name
	bra.s	CASX100
ConvertAddRLSuffix
	move.l	OutputName,a1				; A1 = Base Name
CASX100
	move.l	#DoomFileName,a0			; A0 = Destination Name
CASX200
	move.b	(a1)+,(a0)+
	bne.s	CASX200
	subq.w	#1,a0
CASX400
	move.b	(a2)+,(a0)+
	bne.s	CASX400
	rts


;
;	* * * * * * *       RL ENGINE STATS INFORMATION       * * * * * * *
;
RLEngineStats
	move.l	WADSECTORSSize,d0			; Get #SECTORS
	divu	#26,d0
	move.l	d0,-(sp)
	move.l	WADLINEDEFSSize,d0			; Get #LINEDEFS
	divu	#14,d0
	move.l	d0,-(sp)
	move.l	WADVERTEXESSize,d0			; Get #VERTEXES
	divu	#4,d0
	move.l	d0,-(sp)
	lea	EngineStatsMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(3*4),sp
	moveq.l	#-1,d7
	jmp	PrintMsg


;
;	* * * * * * *       CONVERT DOOM LEVEL VERTEXES -> RL VERTEXES       * * * * * * *
;
ConvertVERTEXES
	move.l	WADVERTEXESSize,d0			; Get Size of VERTEXES
	divu	#4,d0					; 4 Bytes per VERTEX
	move.l	d0,-(sp)
	lea	NumVERTEXESMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	WADVERTEXESSize,d0			; Get Size of VERTEXES
	move.l	d0,RLVERTEXESSize
	divu	#4,d0					; 4 Bytes per VERTEX
	subq.w	#1,d0
	move.l	WADVERTEXES,a0
	move.l	RLVERTEXES,a1
CVXS200
	move.l	(a0)+,(a1)+				; Copy VERTEXES Directly!
	dbf	d0,CVXS200
	rts


;
;	* * * * * * *       CONVERT DOOM LEVEL LINEDEFS -> RL LINES       * * * * * * *
;
ConvertLINEDEFS
	move.l	WADLINEDEFSSize,d0			; Get Size of LINEDEFS
	divu	#14,d0					; 14 Bytes per LINEDEF
	move.l	d0,-(sp)
	lea	NumLINEDEFSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	WADLINEDEFSSize,d7			; Get Size of LINEDEFS
	divu	#14,d7					; 14 Bytes per LINEDEF
	subq.w	#1,d7
	move.l	WADLINEDEFS,a2
	move.l	RLLINES,a3
	moveq.l	#0,d6					; LINE COUNTER
CLDS200
	sub.w	#(12*4),sp				; LINE#
	move.l	d6,0(sp)
;
	moveq.l	#0,d0					; VERTEX 1
	move.b	1(a2),d0
	lsl.w	#8,d0
	move.b	0(a2),d0
	move.l	d0,4(sp)
	lsl.w	#2,d0
;
	move.l	RLVERTEXES,a0
	add.l	d0,a0
	move.b	1(a0),d1				; V1X
	lsl.w	#8,d1
	move.b	0(a0),d1
	ext.l	d1
	move.l	d1,12(sp)
	move.b	3(a0),d1				; V1Y
	lsl.w	#8,d1
	move.b	2(a0),d1
	ext.l	d1
	move.l	d1,16(sp)
;
	add.l	#rlVertexData,d0
	move.b	d0,0(a3)
	lsr.w	#8,d0
	move.b	d0,1(a3)
;
	moveq.l	#0,d0					; VERTEX 2
	move.b	3(a2),d0
	lsl.w	#8,d0
	move.b	2(a2),d0
	move.l	d0,8(sp)
	lsl.w	#2,d0
;
	move.l	RLVERTEXES,a0
	add.l	d0,a0
	move.b	1(a0),d1				; V2X
	lsl.w	#8,d1
	move.b	0(a0),d1
	ext.l	d1
	move.l	d1,20(sp)
	move.b	3(a0),d1				; V2Y
	lsl.w	#8,d1
	move.b	2(a0),d1
	ext.l	d1
	move.l	d1,24(sp)
;
	add.l	#rlVertexData,d0
	move.b	d0,2(a3)
	lsr.w	#8,d0
	move.b	d0,3(a3)
;
;	>>>   CALCULATE ANGLE   <<<
;
GetAngle
	move.l	24(sp),d1				; DeltaY
	sub.l	16(sp),d1
	move.l	20(sp),d0				; DeltaX
	sub.l	12(sp),d0
	move.l	d0,-(sp)
	move.l	d1,-(sp)
	move.l	MathIEEEDoubBasBase,a6
	move.l	4(sp),d0				; Convert DeltaX to Double
	bne.s	GetAngle200				; DeltaX = 0?
	move.l	#$4000,d0				; 90Degrees
	tst.l	0(sp)					; DeltaY >= 0?
	bpl	GetAngle800
	move.l	#$c000,d0				; 270Degrees
	bra	GetAngle800
GetAngle200
	lsl.l	#8,d0
	bpl.s	GetAngle220
	neg.l	d0
GetAngle220
	jsr	_LVOIEEEDPFlt(a6)
	move.l	d0,d2
	move.l	d1,d3
	move.l	(sp),d0					; Convert DeltaY to Double
	bne.s	GetAngle250
	move.l	#$0000,d0				; 0Degrees
	tst.l	4(sp)					; DeltaX >= 0?
	bpl	GetAngle800
	move.l	#$8000,d0				; 180Degrees
	bra	GetAngle800
GetAngle250
	lsl.l	#8,d0
	bpl.s	GetAngle260
	neg.l	d0
GetAngle260
	jsr	_LVOIEEEDPFlt(a6)
	move.l	0(sp),d5				; D5 = |DeltaY|
	bpl.s	GetAngle310
	neg.l	d5
GetAngle310
	move.l	4(sp),d4				; D4 = |DeltaX|
	bpl.s	GetAngle320
	neg.l	d4
GetAngle320
	cmp.l	d4,d5					; |DeltaY| < |DeltaX|?
	bgt.s	GetAngle350				; No!
	moveq.l	#0,d5
	bra.s	GetAngle400
GetAngle350
	move.l	d0,d5					; Swap DeltaX/DeltaY
	move.l	d2,d0
	move.l	d5,d2
	move.l	d1,d5
	move.l	d3,d1
	move.l	d5,d3
	moveq.l	#1,d5
GetAngle400
	jsr	_LVOIEEEDPDiv(a6)			; Divide DeltaY/DeltaX
	move.l	MathIEEEDoubTransBase,a6		; Get ArcTan(DeltaY/DeltaX)
	jsr	_LVOIEEEDPAtan(a6)
	movem.l	d0-d1,-(sp)
	move.l	MathIEEEDoubBasBase,a6
	move.l	#0031415926,d0				; *(180/PI)
	jsr	_LVOIEEEDPFlt(a6)
	move.l	d0,d2
	move.l	d1,d3
	move.l	#1800000000,d0
	jsr	_LVOIEEEDPFlt(a6)
	jsr	_LVOIEEEDPDiv(a6)
	movem.l	(sp)+,d2-d3
	jsr	_LVOIEEEDPMul(a6)
	movem.l	d0-d1,-(sp)
;
	move.l	#00360,d0				; *(65536/360)
	jsr	_LVOIEEEDPFlt(a6)
	move.l	d0,d2
	move.l	d1,d3
	move.l	#65536,d0
	jsr	_LVOIEEEDPFlt(a6)
	jsr	_LVOIEEEDPDiv(a6)
	movem.l	(sp)+,d2-d3
	jsr	_LVOIEEEDPMul(a6)
;
	jsr	_LVOIEEEDPFix(a6)			; Get Integer Result
	and.l	#$ffff,d0
	tst.l	d5					; DeltaX/DeltaY Swapped?
	beq.s	GetAngle500				; No
	move.l	#$4000,d1				; Yes, (90-Angle)
	sub.l	d0,d1
	move.l	d1,d0
GetAngle500
	tst.l	4(sp)					; Quad0/3?
	bmi.s	GetAngle600				; Quad1/2
	tst.l	0(sp)					; Quad0 (Angle)
	bpl.s	GetAngle800
	neg.l	d0					; Quad3 (360-Angle)
	bra.s	GetAngle800
GetAngle600
	tst.l	0(sp)					; Quad1/2?
	bpl.s	GetAngle700				; Quad1
	add.l	#$8000,d0				; Quad2 (180+Angle)
	bra.s	GetAngle800
GetAngle700
	move.l	#$8000,d1				; Quad1 (180-Angle)
	sub.l	d0,d1
	move.l	d1,d0
GetAngle800
	addq.w	#8,sp					; Pop Stack
	and.l	#$ffff,d0
	move.l	d0,28(sp)
	add.w	#32,d0					; Adjust for RoundOff Errors
	lsr.w	#6,d0
	lsl.w	#6,d0
	move.l	d0,32(sp)
	move.b	d0,6(a3)				; LINE.ANGLE
	lsr.w	#8,d0
	move.b	d0,7(a3)
;
	move.b	5(a2),d0				; D0 = LINEDEF Flags
	lsl.w	#8,d0
	move.b	4(a2),d0
	moveq.l	#0,d1					; D1 = LINE Flags
	btst	#2,d0					; ~"TWO-SIDED" -> "SOLID"
	bne.s	CLDS2100
	bset	#0,d1
CLDS2100
	btst	#5,d0					; "SECRET" -> "SECRET"
	beq.s	CLDS2200
	bset	#2,d1
CLDS2200
	btst	#7,d0					; "DONTDRAW" -> "DONTDRAW"
	beq.s	CLDS2300
	bset	#5,d1
CLDS2300
	btst	#0,d0					; "IMPASSIBLE" -> "IMPASSIBLE"
	beq.s	CLDS2350
	bset	#3,d1
CLDS2350
	move.b	d1,4(a3)
	moveq.l	#0,d1					; FLAGS2
;
	move.b	13(a2),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a2),d0
	cmp.w	#-1,d0					; No LEFT SIDEDEF?
	beq	CLDS2500				; No!
;
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	move.b	29(a0),d0				; D0 = LEFT SECTOR#
	lsl.w	#8,d0
	move.b	28(a0),d0
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = LEFT SECTOR
	add.l	d0,a0
	move.b	1(a0),d2				; D2 = LEFT SECTOR FLOOR HEIGHT
	lsl.w	#8,d2
	move.b	0(a0),d2
	move.b	3(a0),d3				; D3 = LEFT SECTOR CEILING HEIGHT
	lsl.w	#8,d3
	move.b	2(a0),d3
;
	move.b	11(a2),d0				; RIGHT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a2),d0
	move.l	WADSIDEDEFS,a0				; A0 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	move.b	29(a0),d0				; D0 = RIGHT SECTOR#
	lsl.w	#8,d0
	move.b	28(a0),d0
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = RIGHT SECTOR
	add.l	d0,a0
	move.b	1(a0),d0				; D0 = RIGHT SECTOR FLOOR HEIGHT
	lsl.w	#8,d0
	move.b	0(a0),d0
	cmp.w	d0,d2
	beq.s	CLDS2400
	bset	#0,d1
CLDS2400
	move.b	3(a0),d0				; D0 = RIGHT SECTOR CEILING HEIGHT
	lsl.w	#8,d0
	move.b	2(a0),d0
	cmp.w	d0,d3
	beq.s	CLDS2500
	bset	#1,d1
CLDS2500
	moveq.l	#0,d0					; LINE_TYPE -> "SPECIAL"
	move.b	7(a2),d0
	lsl.w	#8,d0
	move.b	6(a2),d0
	tst.w	d0					; LineType = 0 (NORMAL)?
	beq.s	CLDS2600				; Yes
	cmp.w	#-1,d0					; E2M7 Invalid Line?
	beq.s	CLDS2600				; Yes!
	lea	WADLINETYPEFLAGS(pc),a0			; Get TRIGGER/USABLE Flag
	or.b	(a0,d0.w),d1
CLDS2600
	move.b	d1,5(a3)				; FLAGS2
;
	moveq.l	#0,d0					; TYPE
	move.b	6(a2),d0
	move.b	d0,10(a3)
	move.l	d0,40(sp)
;
	move.b	8(a2),d0				; TAG
	move.b	d0,11(a3)
	move.l	d0,44(sp)
;
	move.b	5(a3),d0				; FLAGS/FLAGS2
	lsl.w	#8,d0
	move.b	4(a3),d0
	move.l	d0,36(sp)
;
	move.l	d7,d5
	lea	LINEDEFDataMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(12*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	d5,d7
;
	add.w	#14,a2					; Next WAD LINEDEF
	add.w	#rllSize,a3				; Next RL LINE
	addq.w	#1,d6					; Next LINE
	dbf	d7,CLDS200
;
	move.l	RLLINES,a0				; Determine Size of LINES
	sub.l	a0,a3
	move.l	a3,RLLINESSize
	rts

;
;	* * * * * * *       WAD LINE_TYPE FLAGS TABLE       * * * * * * *
;
WADLINETYPEFLAGS
	dc.b	$00,$28,$04,$04,$00,$04,$00,$08,$04,$08	; 000-009
	dc.b	$04,$08,$00,$04,$08,$00,$04,$00,$08,$04	; 010-019
	dc.b	$08,$08,$04,$08,$00,$00,$08,$08,$08,$08	; 020-029
	dc.b	$04,$08,$08,$08,$08,$04,$04,$04,$04,$14	; 030-039
	dc.b	$00,$00,$08,$00,$00,$00,$00,$00,$00,$00	; 040-049
	dc.b	$00,$08,$04,$00,$00,$00,$04,$00,$04,$04	; 050-059
	dc.b	$00,$08,$08,$08,$00,$00,$00,$00,$00,$00	; 060-069
	dc.b	$08,$00,$00,$04,$04,$04,$00,$04,$00,$00	; 070-079
	dc.b	$00,$00,$04,$00,$00,$00,$04,$04,$14,$04	; 080-089
	dc.b	$04,$04,$00,$00,$00,$00,$00,$14,$04,$00	; 090-099
	dc.b	$00,$00,$08,$08,$00,$00,$00,$00,$00,$00	; 100-109


;
;	* * * * * * *       CONVERT DOOM LEVEL SECTORS -> RL SECTORS       * * * * * * *
;
ConvertSECTORS
	move.l	WADSECTORSSize,d0			; Get Size of SECTORS
	divu	#26,d0					; 26 Bytes per SECTOR
	move.l	d0,-(sp)
	lea	NumSECTORSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	WADSECTORSSize,d7			; Get Size of SECTORS
	divu	#26,d7					; 26 Bytes per SECTOR
	subq.w	#1,d7
	move.l	WADSECTORS,a2
	move.l	RLSECTORS,a3
	move.l	#rlSectorData,d5			; SectorData Pointer
	moveq.l	#0,d6
CSCS200
	sub.w	#(11*4),sp
	move.l	d6,0(sp)
;
	move.b	22(a2),d0				; Type
	lsl.b	#2,d0
	move.b	d0,0(a3)
;
	move.b	24(a2),d0				; Tag
	move.b	d0,9(a3)
;
	move.w	0(a2),d0				; Floor Height
	move.w	d0,3(a3)
	moveq.l	#0,d0
	move.b	1(a2),d0
	lsl.w	#8,d0
	move.b	0(a2),d0
	ext.l	d0
	move.l	d0,4(sp)
;
	move.w	2(a2),d0				; Ceiling Height
	move.w	d0,5(a3)
	moveq.l	#0,d0
	move.b	3(a2),d0
	lsl.w	#8,d0
	move.b	2(a2),d0
	ext.l	d0
	move.l	d0,8(sp)
;
	move.l	#$ff,d0					; Brightness Level
	sub.b	20(a2),d0
	cmp.l	#$f7,d0
	ble.s	CSCS1800
	move.l	#$f7,d0
CSCS1800
	move.b	d0,1(a3)
	move.l	d0,28(sp)
;
;	>>>   SCAN LINEDEFS FOR SEGMENT FOR FAR SECTOR   <<<
;
CSCS2000
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
	move.l	WADLINEDEFS,a0
	moveq.l	#0,d3					; Brightness Level FAR
CSCS2200
	moveq.l	#0,d1
	move.b	13(a0),d1				; D1 = RIGHT SIDEDEF#
	lsl.w	#8,d1
	move.b	12(a0),d1
	cmp.w	#-1,d1					; No RIGHT SIDEDEF?
	beq.s	CSCS2300				; Yes, can't get opposite!
	moveq.l	#0,d0
	move.b	11(a0),d0				; D0 = LEFT SIDEDEF#
	lsl.w	#8,d0
	move.b	10(a0),d0
;	cmp.w	#-1,d0					; No LEFT SIDEDEF?
;	beq.s	CSCS2300				; Yes, can't get opposite!
	move.l	WADSIDEDEFS,a1				; A1 = RIGHT SIDEDEF
	mulu	#30,d1
	add.l	d1,a1
	moveq.l	#0,d1
	move.b	29(a1),d1				; D1 = RIGHT SECTOR#
	lsl.w	#8,d1
	move.b	28(a1),d1
	move.l	WADSIDEDEFS,a1				; A1 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a1
	moveq.l	#0,d0
	move.b	29(a1),d0				; D0 = LEFT SECTOR#
	lsl.w	#8,d0
	move.b	28(a1),d0
;
	cmp.w	d1,d6					; RIGHT SIDEDEF'S SECTOR Matches?
	beq.s	CSCS2400				; Yes, use LEFT
	cmp.w	d0,d6					; LEFT SIDEDEF'S SECTOR Matches?
	bne.s	CSCS2300				; No
	move.l	d1,d0					; Yes, use RIGHT
	bra.s	CSCS2400
CSCS2300
	add.w	#14,a0					; Next LINEDEF
	dbf	d2,CSCS2200
	cmp.l	#1,d3					; Any DARKNESS?
	bgt.s	CSCS2700				; Yes
	move.b	#$f7,d3					; NO!  ABSOLUTE DARKNESS!
	bra.s	CSCS2700
CSCS2400
	cmp.w	d0,d6					; FAR SIDEDEF same as NEAR SIDEDEF?
	beq.s	CSCS2300				; Yes!
	mulu	#26,d0					; A0 = SECTOR
	move.l	WADSECTORS,a1
	add.l	d0,a1
	move.l	#$ff,d0					; Brightness Level
	sub.b	20(a1),d0
	cmp.l	#$f7,d0
	ble.s	CSCS2600
	move.l	#$f7,d0
CSCS2600
	cmp.l	d0,d3					; DARKER than Brightness Level FAR?
	bge.s	CSCS2300				; NO!
	move.l	d0,d3					; YES!  NEW LOWEST BRIGHTNESS LEVEL!
	bra.s	CSCS2300
CSCS2700
	move.b	d3,2(a3)				; Brightness Level FAR
	move.l	d3,32(sp)
;
	moveq.l	#0,d0					; SPECIAL
	move.b	23(a2),d0
	lsl.w	#8,d0
	move.b	22(a2),d0
	move.l	d0,36(sp)
;
	moveq.l	#0,d0					; TRIGGER
	move.b	25(a2),d0
	lsl.w	#8,d0
	move.b	24(a2),d0
	move.l	d0,40(sp)
;
	lea	4(a2),a1				; FLOOR TEXTURE
	lea	Texture1Name,a0
	bsr	XREFTexture2
	move.l	#Texture1Name,12(sp)
	move.l	d0,20(sp)
	move.b	d0,7(a3)
;
	lea	12(a2),a1				; CEILING TEXTURE
	lea	Texture2Name,a0
	bsr	XREFTexture2
	move.l	#Texture2Name,16(sp)
	move.l	d0,24(sp)
	move.b	d0,8(a3)

;
;	>>>   SCAN LINEDEFS TO DETERMINE SOUND ORIGIN OF SECTOR   <<<
;
CSCS3000
	movem.l	d5/d7,-(sp)
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
	move.l	WADLINEDEFS,a0
	move.l	#$7fffffff,d3				; Left X
	move.l	#$7fffffff,d4				; Top Y
	move.l	#$80000000,d5				; Right X
	move.l	#$80000000,d7				; Bottom Y
CSCS3200
	moveq.l	#0,d0
	move.b	11(a0),d0				; D0 = RIGHT SIDEDEF#
	lsl.w	#8,d0
	move.b	10(a0),d0
	move.l	WADSIDEDEFS,a1				; A1 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a1
	moveq.l	#0,d0
	move.b	29(a1),d0				; D0 = RIGHT SECTOR#
	lsl.w	#8,d0
	move.b	28(a1),d0
	cmp.w	d0,d6					; RIGHT SIDEDEF'S SECTOR Matches?
	beq	CSCS3300				; Yes
;
	moveq.l	#0,d0
	move.b	13(a0),d0				; D0 = LEFT SIDEDEF#
	lsl.w	#8,d0
	move.b	12(a0),d0
	cmp.w	#-1,d0
	beq	CSCS3500				; No LEFT SIDEDEF!
	move.l	WADSIDEDEFS,a1				; A1 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a1
	moveq.l	#0,d0
	move.b	29(a1),d0				; D0 = LEFT SECTOR#
	lsl.w	#8,d0
	move.b	28(a1),d0
	cmp.w	d0,d6					; LEFT SIDEDEF'S SECTOR Matches?
	bne	CSCS3500				; No
;
;	>>>   VERTEX 0   <<<
;
CSCS3300
	moveq.l	#0,d0
	move.b	1(a0),d0				; D0 = VERTEX 0
	lsl.w	#8,d0
	move.b	0(a0),d0
	move.l	WADVERTEXES,a1
	lsl.l	#2,d0
	add.l	d0,a1
	move.b	1(a1),d0				; V0X
	lsl.w	#8,d0
	move.b	0(a1),d0
	ext.l	d0
	cmp.l	d0,d3					; Vx < LeftX?
	ble.s	CSCS3320
	move.l	d0,d3
CSCS3320
	cmp.l	d0,d5					; Vx > RightX?
	bge.s	CSCS3340
	move.l	d0,d5
CSCS3340
	move.b	3(a1),d0				; V0Y
	lsl.w	#8,d0
	move.b	2(a1),d0
	ext.l	d0
	cmp.l	d0,d4					; Vy < TopY?
	ble.s	CSCS3360
	move.l	d0,d4
CSCS3360
	cmp.l	d0,d7					; Vy > BotY?
	bge.s	CSCS3380
	move.l	d0,d7
CSCS3380
;
;	>>>   VERTEX 1   <<<
;
	moveq.l	#0,d0
	move.b	3(a0),d0				; D0 = VERTEX 1
	lsl.w	#8,d0
	move.b	2(a0),d0
	move.l	WADVERTEXES,a1
	lsl.l	#2,d0
	add.l	d0,a1
	move.b	1(a1),d0				; V1X
	lsl.w	#8,d0
	move.b	0(a1),d0
	ext.l	d0
	cmp.l	d0,d3					; Vx < LeftX?
	ble.s	CSCS3420
	move.l	d0,d3
CSCS3420
	cmp.l	d0,d5					; Vx > RightX?
	bge.s	CSCS3440
	move.l	d0,d5
CSCS3440
	move.b	3(a1),d0				; V1Y
	lsl.w	#8,d0
	move.b	2(a1),d0
	ext.l	d0
	cmp.l	d0,d4					; Vy < TopY?
	ble.s	CSCS3460
	move.l	d0,d4
CSCS3460
	cmp.l	d0,d7					; Vy > BotY?
	bge.s	CSCS3480
	move.l	d0,d7
CSCS3480

CSCS3500
	add.w	#14,a0					; Next LINEDEF
	dbf	d2,CSCS3200
;
	sub.l	d3,d5					; Get (RightX-LeftX)/2
	lsr.l	#1,d5
	add.l	d3,d5					; Add LeftX
;
	sub.l	d4,d7					; Get (BottomY-TopY)/2
	lsr.l	#1,d7
	add.l	d4,d7					; Add TopY
;
	move.l	d6,d0					; D0 = Sector#
	lsl.l	#2,d0					; 4 Bytes per Sector Origin
	lea	RLSECTOROrigins,a0
	add.l	d0,a0
	move.w	d5,(a0)+				; Sector Origin X
	move.w	d7,(a0)+				; Sector Origin Y
	movem.l	(sp)+,d5/d7
;
;	>>>   FINISHED PROCESSING SECTOR   <<<
;
	move.l	d7,d4
	lea	SECTORDataMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(11*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	d4,d7
;
	add.w	#26,a2					; Next WAD SECTOR
	add.w	#rlsSize,a3				; Next RL SECTOR
	add.w	#rlsdSize,d5				; Next RL SECTORDATA
	addq.w	#1,d6					; Next NODE
	dbf	d7,CSCS200
;
	move.l	RLSECTORS,a0				; Determine Size of SECTORS
	sub.l	a0,a3
	move.l	a3,RLSECTORSSize
	rts


;
;	* * * * * * *       CONVERT DOOM LEVEL NODES -> RL BSP       * * * * * * *
;
ConvertNODES
	move.l	WADNODESSize,d0				; Get Size of NODES
	divu	#28,d0					; 28 Bytes per NODE
	move.l	d0,-(sp)
	lea	NumNODESMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	WADNODESSize,d7				; Get Size of NODES
	divu	#28,d7					; 4 Bytes per NODES
	subq.w	#1,d7
	move.l	WADNODES,a2
	move.l	RLBSP,a3
	moveq.l	#0,d6
CNDS200
	sub.w	#(9*4),sp
	move.l	d6,0(sp)
;
	move.w	16(a2),8(a3)				; Left BoundaryBox YMax
	move.w	18(a2),10(a3)				; Left BoundaryBox YMin
	move.w	20(a2),12(a3)				; Left BoundaryBox XMin
	move.w	22(a2),14(a3)				; Left BoundaryBox XMax
	move.w	8(a2),18(a3)				; Right BoundaryBox YMax
	move.w	10(a2),20(a3)				; Right BoundaryBox YMin
	move.w	12(a2),22(a3)				; Right BoundaryBox XMin
	move.w	14(a2),24(a3)				; Right BoundaryBox XMax
;
	move.w	2(a2),0(a3)				; Y Coordinate
	move.w	4(a2),2(a3)				; X Delta
	move.w	0(a2),4(a3)				; X Coordinate
	move.w	6(a2),6(a3)				; Y Delta
;
	moveq.l	#0,d0
	move.b	1(a2),d0
	lsl.w	#8,d0
	move.b	0(a2),d0
	ext.l	d0
	move.l	d0,4(sp)
;
	moveq.l	#0,d0
	move.b	3(a2),d0
	lsl.w	#8,d0
	move.b	2(a2),d0
	ext.l	d0
	move.l	d0,8(sp)
;
	moveq.l	#0,d0
	move.b	5(a2),d0
	lsl.w	#8,d0
	move.b	4(a2),d0
	ext.l	d0
	move.l	d0,12(sp)
;
	moveq.l	#0,d0
	move.b	7(a2),d0
	lsl.w	#8,d0
	move.b	6(a2),d0
	ext.l	d0
	move.l	d0,16(sp)
;
	moveq.l	#0,d0
	move.b	25(a2),d0				; D0 = RIGHT NODE
	lsl.w	#8,d0
	move.b	24(a2),d0
	tst.w	d0
	bpl.s	CNDS300
	and.l	#$7fff,d0
	move.l	d0,32(sp)
	mulu	#rlaSize,d0
	or.w	#$8000,d0
	move.l	#NODEDataAREAMsg,28(sp)
	bra.s	CNDS400
CNDS300
	move.l	d0,32(sp)
	mulu	#rlbSize,d0
	move.l	#NODEDataNODEMsg,28(sp)
CNDS400
	moveq.l	#0,d1
	move.b	27(a2),d1				; D1 = LEFT NODE
	lsl.w	#8,d1
	move.b	26(a2),d1
	tst.w	d1
	bpl.s	CNDS600
	and.l	#$7fff,d1
	move.l	d1,24(sp)
	mulu	#rlaSize,d1
	or.w	#$8000,d1
	move.l	#NODEDataAREAMsg,20(sp)
	bra.s	CNDS700
CNDS600
	move.l	d1,24(sp)
	mulu	#rlbSize,d1
	move.l	#NODEDataNODEMsg,20(sp)
CNDS700
	move.b	d1,16(a3)				; LEFT NODE
	lsr.w	#8,d1
	move.b	d1,17(a3)
	move.b	d0,26(a3)				; RIGHT NODE
	lsr.w	#8,d0
	move.b	d0,27(a3)
;
	move.l	d7,d5
	lea	NODEDataMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(9*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	d5,d7
;
	add.w	#28,a2					; Next WAD NODE
	add.w	#rlbSize,a3				; Next RL BSP
	addq.w	#1,d6					; Next NODE
	dbf	d7,CNDS200
;
	move.l	RLBSP,a0				; Determine Size of BSP
	sub.l	a0,a3
	move.l	a3,RLBSPSize
	rts


;
;	* * * * * * *       CONVERT DOOM LEVEL SSECTORS -> RL AREAS       * * * * * * *
;
ConvertSSECTORS
	move.l	WADSSECTORSSize,d0			; Get Size of SSECTORS
	divu	#4,d0					; 4 Bytes per SSECTOR
	move.l	d0,-(sp)
	lea	NumSSECTORSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	WADSSECTORSSize,d7			; Get Size of SSECTORS
	divu	#4,d7					; 4 Bytes per SSECTOR
	subq.w	#1,d7
	move.l	WADSSECTORS,a2
	move.l	RLAREAS,a3
CSSS200
	move.b	(a2)+,(a3)+				; Get #SEGS
	addq.w	#1,a2					; Skip HighByte
	moveq.l	#0,d0
	move.b	1(a2),d0				; Get STARTING SEGMENT#
	lsl.w	#8,d0
	move.b	0(a2),d0
	addq.w	#2,a2
	mulu	#rlgSize,d0				; *rlgSize
	move.l	d0,d1
	move.b	d0,(a3)+
	lsr.w	#8,d0
	move.b	d0,(a3)+
;
	move.l	RLSEGS,a0				; Move to SEGMENT
	add.l	d1,a0
	move.b	8(a0),d0				; Get FACE
	lsl.w	#8,d0
	move.b	7(a0),d0
	move.l	RLFACES,a0				; Move to FACE
	add.l	d0,a0
	move.b	(a0),(a3)+				; Save SECTOR
;
	dbf	d7,CSSS200
	move.l	RLAREAS,a0				; Determine Size of AREAS
	sub.l	a0,a3
	move.l	a3,RLAREASSize
	rts


;
;	* * * * * * *       CONVERT DOOM LEVEL SEGS -> RL SEGS       * * * * * * *
;
ConvertSEGS
	move.l	WADSEGSSize,d0				; Get Size of SEGS
	divu	#12,d0					; 12 Bytes per SEG
	move.l	d0,-(sp)
	lea	NumSEGSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	WADSEGSSize,d7				; Get Size of SEGS
	divu	#12,d7					; 12 Bytes per SEGS
	subq.w	#1,d7
	move.l	WADSEGS,a2
	move.l	RLSEGS,a3
	moveq.l	#0,d6					; D6 = SEGMENT NUMBER
;
;	>>>   PROCESS THE NEXT SEGMENT   <<<
;
CSGS200
	sub.w	#(14*4),sp
	move.l	d6,0(sp)
;
	moveq.l	#0,d0					; VERTEX1
	move.b	1(a2),d0
	lsl.w	#8,d0
	move.b	0(a2),d0
	move.l	d0,8(sp)
	lsl.w	#2,d0
	move.l	RLVERTEXES,a0
	add.l	d0,a0
	add.l	#rlVertexData,d0
	move.b	d0,(a3)
	lsr.w	#8,d0
	move.b	d0,1(a3)
	moveq.l	#0,d0					; VERTEX2
	move.b	3(a2),d0
	lsl.w	#8,d0
	move.b	2(a2),d0
	move.l	d0,12(sp)
	lsl.w	#2,d0
	move.l	RLVERTEXES,a1
	add.l	d0,a1
	add.l	#rlVertexData,d0
	move.b	d0,2(a3)
	lsr.w	#8,d0
	move.b	d0,3(a3)
;
	move.b	1(a0),d2				; V1X
	lsl.w	#8,d2
	move.b	0(a0),d2
	ext.l	d2
	move.l	d2,16(sp)
;
	move.b	1(a1),d0				; V2X
	lsl.w	#8,d0
	move.b	0(a1),d0
	ext.l	d0
	move.l	d0,24(sp)
;
	sub.l	d2,d0
	bpl.s	CSGS1200
	neg.l	d0
CSGS1200
	mulu	d0,d0
;
	move.b	3(a0),d2				; V1Y
	lsl.w	#8,d2
	move.b	2(a0),d2
	ext.l	d2
	move.l	d2,20(sp)
;
	move.b	3(a1),d1				; V2Y
	lsl.w	#8,d1
	move.b	2(a1),d1
	ext.l	d1
	move.l	d1,28(sp)
;
	sub.l	d2,d1
	bpl.s	CSGS1400
	neg.l	d1
CSGS1400
	mulu	d1,d1
;
	add.l	d1,d0					; D2 = A2+B2
	move.l	MathIEEEDoubBasBase,a6			; Convert A2+B2 to Double
	jsr	_LVOIEEEDPFlt(a6)
	move.l	MathIEEEDoubTransBase,a6		; Get SquareRoot(A2+B2)
	jsr	_LVOIEEEDPSqrt(a6)
	move.l	d0,d4					; Save Result in D4/D5
	move.l	d1,d5
	move.l	MathIEEEDoubBasBase,a6			; Convert 2 to Double 2.0
	move.l	#2,d0
	jsr	_LVOIEEEDPFlt(a6)
	move.l	d0,d2
	move.l	d1,d3
	move.l	#1,d0					; Convert 1 to Double 1.0
	jsr	_LVOIEEEDPFlt(a6)
	jsr	_LVOIEEEDPDiv(a6)			; Divide 1.0/2.0 = 0.5
	move.l	d4,d2					; Add 0.5 to Sqrt(A2+B2) Result
	move.l	d5,d3
	jsr	_LVOIEEEDPAdd(a6)
	jsr	_LVOIEEEDPFix(a6)			; Get Integer Result
;
	move.l	d0,32(sp)
;	move.b	d0,6(a3)				; LENGTH
;	lsr.w	#8,d0
;	move.b	d0,7(a3)
;
;	>>>   FLAGS   <<<
;
	moveq.l	#0,d0
	move.b	7(a2),d0				; Get LINEDEF
	lsl.w	#8,d0
	move.b	6(a2),d0
	mulu	#14,d0
	move.l	WADLINEDEFS,a0
	add.l	d0,a0
	move.b	5(a0),d0				; D0 = LINEDEF Flags
	lsl.w	#8,d0
	move.b	4(a0),d0
	moveq.l	#0,d1					; D1 = SEGMENT Flags
	btst	#2,d0					; ~"TWO-SIDED" -> "SOLID"
	bne.s	CSGS2200
;
;	>>>   SOLID   <<<
;
	bset	#0,d1
	btst	#4,d0					; NORMAL UNPEGGED?
	bne.s	CSGS2400
	bset	#6,d1
	bra.s	CSGS2400
;
;	>>>   TRANSPARENT   <<<
;
CSGS2200
	btst	#3,d0					; UPPER UNPEGGED?
	beq.s	CSGS2300
	bset	#5,d1
CSGS2300
	btst	#4,d0					; LOWER UNPEGGED?
	beq.s	CSGS2400
	bset	#6,d1
;
;	>>>   FLAGS 2   <<<
;
CSGS2400
	moveq.l	#0,d0					; Get OFFSET to SIDEDEFS
	move.b	8(a2),d0
	lsl.w	#1,d0
	add.l	d0,a0
	add.w	#10,a0
	moveq.l	#0,d0					; Get FACE (SIDEDEF)
	move.b	1(a0),d0
	lsl.w	#8,d0
	move.b	0(a0),d0
	lea	RLFACESFlags,a0				; Get FACE Flags
	move.b	(a0,d0.w),d0
;
;	>>>   CLEAR   <<<
;
	btst	#0,d0					; ALL TEXTURES TRANSPARENT?
	beq.s	CSGS2450				; No, SEGMENT is NOT "CLEAR"
	bset	#3,d1
;
;	>>>   SKY   <<<
;
CSGS2450
	btst	#1,d0					; SKY?
	beq.s	CSGS2500
	bset	#1,d1					; YES!

;
;	>>>   NORMAL/LOWER SWITCH   <<<
;
CSGS2500
	ifd	egad
	btst	#6,d0					; NORMAL/LOWER SWITCH?
	beq.s	CSGS2550
	bclr	#6,d1					; YES!
;
;	>>>   UPPER SWITCH   <<<
;
CSGS2550
	btst	#5,d0					; UPPER SWITCH?
	beq.s	CSGS2700
	bclr	#5,d1					; YES!
	endc

;
CSGS2700
	move.l	d1,52(sp)
	move.b	d1,4(a3)
;	lsr.w	#8,d1
;	move.b	d1,5(a3)
;
	ifd	egad
	move.w	4(a2),8(a3)				; ANGLE
	endc

	moveq.l	#0,d0
	move.b	5(a2),d0
	lsl.w	#8,d0
	move.b	4(a2),d0
	move.l	d0,36(sp)
;
	moveq.l	#0,d0
	move.b	7(a2),d0				; Get LINEDEF
	lsl.w	#8,d0
	move.b	6(a2),d0
	move.l	d0,d1
	mulu	#14,d0
	move.l	WADLINEDEFS,a0
	add.l	d0,a0
	moveq.l	#0,d0					; Get OFFSET to SIDEDEFS
	move.b	8(a2),d0
	lsl.w	#1,d0
	add.l	d0,a0
	add.w	#10,a0
	moveq.l	#0,d0					; Get SIDEDEF
	move.b	1(a0),d0
	lsl.w	#8,d0
	move.b	0(a0),d0
	move.l	d0,48(sp)
	lsl.w	#2,d0					; GET OFFSET TO FACES
	lea	RLFACESPtrs,a0
	move.l	(a0,d0.w),d0
	sub.l	RLFACES,d0
;
	mulu	#rllSize,d1				; POINT TO RLLINE
	move.l	RLLINES,a0
	add.l	d1,a0
;
	move.w	d0,d1
	move.b	d0,7(a3)				; SEGMENT.FACE
	lsr.w	#8,d0
	move.b	d0,8(a3)
;
	tst.w	8(a2)					; LEFT or RIGHT side?
	bne.s	CSGS2800				; RIGHT, DON'T USE BACKWARDS FACE!
	move.b	d1,8(a0)				; LINE.FACE
	lsr.w	#8,d1
	move.b	d1,9(a0)
;	move.w	4(a2),6(a0)				; LINE.ANGLE
CSGS2800
;
	move.l	#' ',4(sp)
	tst.w	8(a2)					; LEFT or RIGHT side?
	beq.s	CSGS3200				; RIGHT
	move.l	#'*',4(sp)				; LEFT
	ifd	egad
	move.w	0(a3),d0				; Swap VERTEXES
	move.w	2(a3),0(a3)
	move.w	d0,2(a3)
	eor.b	#$80,9(a3)				; Reverse ANGLE
	endc
CSGS3200
	move.l	48(sp),d0				; Get SIDEDEF.XOFFSET
	mulu	#30,d0
	move.l	WADSIDEDEFS,a0
	add.l	d0,a0
	moveq.l	#0,d1
	move.b	1(a0),d1
	lsl.w	#8,d1
	move.b	0(a0),d1
	ext.l	d1
	moveq.l	#0,d0					; Get SEGMENT.OFFSET
	move.b	11(a2),d0
	lsl.w	#8,d0
	move.b	10(a2),d0
	add.l	d1,d0
	and.l	#$000000ff,d0
	move.l	d0,40(sp)				; FACEOFFSET X
	move.b	d0,5(a3)
;
	move.l	48(sp),d0				; Get SIDEDEF.YOFFSET
	mulu	#30,d0
	move.l	WADSIDEDEFS,a0
	add.l	d0,a0
	moveq.l	#0,d1
	move.b	3(a0),d1
	lsl.w	#8,d1
	move.b	2(a0),d1
	and.l	#$000000ff,d1
	move.l	d1,44(sp)				; FACEOFFSET Y
	move.b	d1,6(a3)
;
	move.w	6(a2),9(a3)				; LINEDEF
;
	move.l	d7,d5
	lea	SEGDataMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(14*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	d5,d7
;
;	>>>   DRAW THE SEGMENT   <<<
;
	move.l	d6,-(sp)
;
	move.l	#2,d0					; SOLID WALL is GREY
;	move.b	5(a3),d1
;	lsl.w	#8,d1
	move.b	4(a3),d1
	btst	#0,d1
	bne.s	CSGS8200
	move.l	#1,d0					; TRANSPARENT WALL is WHITE
CSGS8200
	move.l	GraphicsBase,a6
	move.l	PicRPort,a1
	jsr	_LVOSetAPen(a6)
;
	move.l	WADBLOCKMAP,a0
	moveq.l	#0,d2					; D2 = X ORIGIN
	move.b	1(a0),d2
	lsl.w	#8,d2
	move.b	0(a0),d2
	ext.l	d2
	moveq.l	#0,d0					; D2 = XORIGIN+(XWIDTH/2)
	move.b	5(a0),d0
	lsl.w	#8,d0
	move.b	4(a0),d0
	lsl.l	#6,d0
	add.l	d0,d2
	moveq.l	#0,d3					; D3 = Y ORIGIN
	move.b	3(a0),d3
	lsl.w	#8,d3
	move.b	2(a0),d3
	ext.l	d3
	moveq.l	#0,d0					; D3 = YORIGIN+(YWIDTH/2)
	move.b	7(a0),d0
	lsl.w	#8,d0
	move.b	6(a0),d0
	lsl.l	#6,d0
	add.l	d0,d3
;
	move.l	WADBLOCKMAP,a0
	moveq.l	#0,d1					; D5 = XWIDTH
	move.b	5(a0),d1
	lsl.w	#8,d1
	move.b	4(a0),d1
	lsl.l	#7,d1
	moveq.l	#0,d5					; D1 = IFF WIDTH
	move.w	PicX,d5
	sub.w	#(32*2),d5				; 32 pixel border
	swap	d5
	divu.l	d1,d5					; D5 = X_SCALE
;
	moveq.l	#0,d1					; D6 = YWIDTH
	move.b	7(a0),d1
	lsl.w	#8,d1
	move.b	6(a0),d1
	lsl.l	#7,d1
	moveq.l	#0,d6					; D1 = IFF WIDTH
	move.w	PicY,d6
	sub.w	#(32*2),d6				; 32 pixel border
	swap	d6
	divu.l	d1,d6					; D6 = Y_SCALE
;
	cmp.l	d5,d6					; Y_SCALE > X_SCALE?
	bgt.s	CSGS8400				; Yes, Use X_SCALE
	move.l	d6,d5					; No, Use Y_SCALE
CSGS8400
	move.l	d5,d6
;
	moveq.l	#0,d0					; VERTEX1
	move.b	1(a2),d0
	lsl.w	#8,d0
	move.b	0(a2),d0
	lsl.w	#2,d0
	move.l	RLVERTEXES,a0
	add.l	d0,a0
	move.b	1(a0),d0				; V1X
	lsl.w	#8,d0
	move.b	0(a0),d0
	ext.l	d0
	move.b	3(a0),d1				; V1Y
	lsl.w	#8,d1
	move.b	2(a0),d1
	ext.l	d1
	sub.l	d2,d0					; X-(X0+(XSize/2))
	sub.l	d3,d1					; Y-(Y0+(YSize/2))
	muls.l	d5,d0
	muls.l	d6,d1
	swap	d0
	ext.l	d0
	swap	d1
	ext.l	d1
	neg.l	d1
	moveq.l	#0,d4					; + (PicX/2)
	move.w	PicX,d4
	lsr.w	#1,d4
	add.l	d4,d0
	move.w	PicY,d4					; + (PicY/2)
	lsr.w	#1,d4
	add.l	d4,d1
	move.l	PicRPort,a1
	jsr	_LVOMove(a6)
;
	moveq.l	#0,d0					; VERTEX2
	move.b	3(a2),d0
	lsl.w	#8,d0
	move.b	2(a2),d0
	lsl.w	#2,d0
	move.l	RLVERTEXES,a0
	add.l	d0,a0
	move.b	1(a0),d0				; V2X
	lsl.w	#8,d0
	move.b	0(a0),d0
	ext.l	d0
	move.b	3(a0),d1				; V2Y
	lsl.w	#8,d1
	move.b	2(a0),d1
	ext.l	d1
	sub.l	d2,d0					; X-(X0+(XSize/2))
	sub.l	d3,d1					; Y-(Y0+(YSize/2))
	muls.l	d5,d0
	muls.l	d6,d1
	swap	d0
	ext.l	d0
	swap	d1
	ext.l	d1
	neg.l	d1
	moveq.l	#0,d4					; + (PicX/2)
	move.w	PicX,d4
	lsr.w	#1,d4
	add.l	d4,d0
	move.w	PicY,d4					; + (PicY/2)
	lsr.w	#1,d4
	add.l	d4,d1
	move.l	PicRPort,a1
	jsr	_LVODraw(a6)
;
	move.l	(sp)+,d6
	add.w	#12,a2					; Next WAD SEG
	add.w	#rlgSize,a3				; Next RL SEG
;
	addq.w	#1,d6
	dbf	d7,CSGS200
	move.l	RLSEGS,a0				; Determine Size of SEGS
	sub.l	a0,a3
	move.l	a3,RLSEGSSize
	rts


;
;	* * * * * * *       CONVERT DOOM LEVEL DOORS -> RL DOORS       * * * * * * *
;
ConvertDOORS
	clr.l	RLNUMDOORS				; NO Doors Defined Yet!
	move.l	WADLINEDEFSSize,d7			; Get Size of LINEDEFS
	divu	#14,d7					; 14 Bytes per LINEDEF
	subq.w	#1,d7
	move.l	WADLINEDEFS,a2
	move.l	RLDOORS,a3
CDRS200
	moveq.l	#0,d0					; D0 = LINE_TYPE
	move.b	7(a2),d0
	lsl.w	#8,d0
	move.b	6(a2),d0
	tst.w	d0					; Invalid?
	bmi	CDRS800					; Yes!
	lea	WADLINETYPEDOOR,a1
	add.l	d0,a1
	move.b	(a1),d0					; Is it a "DOOR" ?
	beq	CDRS800					; No
	cmp.b	#2,d0					; No, it AFFECTS a DOOR
	beq	CDRS500
;
;	* * * * * * *       NORMAL DOOR       * * * * * * *
;
;	>>>   GET LINEDEF'S SECTOR   <<<
;
	move.b	13(a2),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a2),d0
	cmp.w	#-1,d0					; No LEFT SIDEDEF?
	beq	CDRS800					; No!
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d5
	move.b	29(a0),d5				; D5 = LEFT SECTOR#
	lsl.w	#8,d5
	move.b	28(a0),d5
	move.l	d5,d1
	bsr	DoConvertDOOR				; Add Door
	bra.s	CDRS800
;
;	* * * * * * *       TAG-BASED DOOR       * * * * * * *
;
;	>>>   GET LINEDEF'S TAG   <<<
;
CDRS500
	move.b	9(a2),d6				; LINEDEF TAG
	lsl.w	#8,d6
	move.b	8(a2),d6
	tst.w	d6					; Invalid Tag?
	beq.s	CDRS800					; YES!
;
;	>>>   SCAN ALL SECTORS FOR MATCHING TAG VALUE   <<<
;
	movem.l	d7/a2,-(sp)
	move.l	WADSECTORSSize,d7			; Get Size of SECTORS
	divu	#26,d7					; 26 Bytes per SECTOR
	subq.w	#1,d7
	move.l	WADSECTORS,a2
	moveq.l	#0,d5					; D5 = SECTOR#
CDRS600
	moveq.l	#0,d0					; SECTOR TAG
	move.b	25(a2),d0
	lsl.w	#8,d0
	move.b	24(a2),d0
	cmp.w	d0,d6					; MATCHES LINEDEF TAG?
	bne.s	CDRS700					; No
	move.l	d5,d1					; D1 = RLSECTOR
	movem.l	d5-d7/a2,-(sp)
	bsr	DoConvertDOOR				; Add Door
	movem.l	(sp)+,d5-d7/a2
CDRS700
	add.w	#26,a2					; Next WAD SECTOR
	addq.w	#1,d5					; Next SECTOR
	dbf	d7,CDRS600
	movem.l	(sp)+,d7/a2
;
;	* * * * * * *       NEXT LINEDEF       * * * * * * *
;
CDRS800
	add.w	#14,a2					; Next LINEDEF
	dbf	d7,CDRS200
;
	move.l	RLDOORS,a0				; Determine Size of DOORS
	sub.l	a0,a3
	move.l	a3,RLDOORSSize
;
	move.l	RLNUMDOORS,-(sp)			; Print# DOORS
	lea	NumDOORSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jmp	PrintMsg

;
;	* * * * * * *       WAD LINE_TYPE DOOR FLAG TABLE       * * * * * * *
;
;	0=NOT a door, 1=DOOR, 2=AFFECTS DOOR IN TAG
;
WADLINETYPEDOOR
	dc.b	0,1,2,2,0,0,0,0,0,0			; 000-009
	dc.b	0,0,0,0,0,0,2,0,0,0			; 010-019
	dc.b	0,0,0,0,0,0,1,1,1,2			; 020-029
	dc.b	0,1,1,1,1,0,0,0,0,0			; 030-039
	dc.b	0,0,2,0,0,0,2,0,0,0			; 040-049
	dc.b	0,0,0,0,0,0,0,0,0,0			; 050-059
	dc.b	0,2,0,2,0,0,0,0,0,0			; 060-069
	dc.b	0,0,0,0,0,2,0,0,0,0			; 070-079
	dc.b	0,0,0,0,0,0,2,0,0,0			; 080-089
	dc.b	2,0,0,0,0,0,0,0,0,0			; 090-099
	dc.b	0,0,0,2,0,0,0,0,0,0			; 100-109

;
;	* * * * * * *       CONVERT A SINGLE DOOR       * * * * * * *
;
;	D1 = SECTOR# OF DOOR
;
DoConvertDOOR
;
;	>>>   DOOR ALREADY HANDLED?   <<<
;
	move.l	RLDOORS,a0
DCDR300
	cmp.l	a0,a3					; No, add it
	beq	DCDR400
	cmp.b	(a0),d1					; Get DOOR SECTOR
	beq	DCDR900					; Yes!
	moveq.l	#0,d0
	move.b	7(a0),d0				; Skip DOOR'S LINES
	add.w	#8,a0
	lsl.w	#1,d0
	add.w	d0,a0
	bra	DCDR300					; Next DOOR
;
;	>>>   ADD A NEW DOOR   <<<
;
DCDR400
	addq.l	#1,RLNUMDOORS				; One More Door Defined!
	move.b	d1,0(a3)				; SECTOR
;
	move.l	d5,d0					; D0 = Sector#
	lsl.l	#2,d0					; 4 Bytes per Sector Origin
	lea	RLSECTOROrigins,a0
	add.l	d0,a0
	move.w	(a0)+,d0				; SECTOR ORIGIN X
	move.b	d0,3(a3)
	lsr.w	#8,d0
	move.b	d0,4(a3)
	move.w	(a0)+,d0				; SECTOR ORIGIN Y
	move.b	d0,5(a3)
	lsr.w	#8,d0
	move.b	d0,6(a3)
;
	clr.b	7(a3)					; NO LINES ADDED
;
	move.l	#$7fffffff,d6				; D6 = MAXIMUM CEILING HEIGHT
	lea	8(a3),a4				; LINELIST
	move.l	WADLINEDEFS,a1
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
DCDR500
	moveq.l	#0,d0
	move.b	13(a1),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a1),d0
	cmp.w	#-1,d0					; LEFT SIDEDEF?
	beq	DCDR520					; NO!
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = LEFT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	beq	DCDR550
DCDR520
	moveq.l	#0,d0
	move.b	11(a1),d0				; RIGHT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a1),d0
	move.l	WADSIDEDEFS,a0				; A0 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = RIGHT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	bne	DCDR680
;
DCDR550
	move.l	a1,d4					; D4 = LINE#
	sub.l	WADLINEDEFS,d4
	divu	#14,d4					; 14 Bytes per LINEDEF
	move.w	d4,d0
	move.b	d0,(a4)+				; ADD LINE TO LINELIST
	lsr.w	#8,d0
	move.b	d0,(a4)+
	add.b	#1,7(a3)				; LINECOUNT++
;
	move.l	d4,d0
	mulu	#rllSize,d0
	add.l	RLLINES,d0
	move.l	d0,a0
	move.b	4(a0),d0
	bset	#4,d0					; LINE IS "DOOR"
	move.b	d0,4(a0)
;
;	>>>   FAR SECTOR?   <<<
;
	moveq.l	#0,d0
	move.b	13(a1),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a1),d0
	cmp.w	#-1,d0					; No LEFT SIDEDEF?
	beq	DCDR680					; No!
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = LEFT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	bne.s	DCDR570
;
	moveq.l	#0,d0
	move.b	11(a1),d0				; RIGHT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a1),d0
	move.l	WADSIDEDEFS,a0				; A0 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = RIGHT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
DCDR570
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	3(a0),d0				; D0 = SECTOR CEILING HEIGHT
	lsl.w	#8,d0
	move.b	2(a0),d0
	ext.l	d0
	cmp.l	d0,d6					; Opposite Ceiling < Max Ceiling?
	ble	DCDR600					; No
	move.l	d0,d6					; Yes, New Max Ceiling!
;
;	>>>   SCAN SEGS THAT BELONG TO THIS LINE   <<<
;
DCDR600
	move.l	WADSEGSSize,d1				; Get Size of SEGS
	divu	#12,d1					; 12 Bytes per SEGS
	subq.w	#1,d1
	move.l	WADSEGS,a0
DCDR620
	move.b	7(a0),d0				; Get LINEDEF
	lsl.w	#8,d0
	move.b	6(a0),d0
	cmp.w	d0,d4					; Same LINE?
	bne	DCDR660
	move.l	a0,-(sp)
	move.l	a0,d0
	sub.l	WADSEGS,d0
	divu	#12,d0
	mulu	#rlgSize,d0
	add.l	RLSEGS,d0
	move.l	d0,a0
	move.b	4(a0),d0
	bset	#2,d0					; SEGMENT IS "DOOR"
	move.b	d0,4(a0)
	move.l	(sp)+,a0
DCDR660
	add.w	#12,a0					; Next SEG
	dbf	d1,DCDR620
DCDR680
	add.w	#14,a1					; Next LINEDEF
	dbf	d2,DCDR500
;
	sub.w	#4,d6					; MAX. CEILING -4
	move.b	d6,1(a3)
	lsr.w	#8,d6
	move.b	d6,2(a3)
	tst.b	7(a3)					; ANY LINES ADDED?
	beq.s	DCDR800					; NO!
	move.l	a4,a3
	rts
DCDR800
	subq.l	#1,RLNUMDOORS				; One LESS Door Defined!
DCDR900
	rts


;
;	* * * * * * *       CONVERT DOOM LEVEL STAIRS -> RL STAIRS       * * * * * * *
;
ConvertSTAIRS
	clr.l	RLNUMSTAIRS				; NO Stairs Defined Yet!
	move.l	WADLINEDEFSSize,d7			; Get Size of LINEDEFS
	divu	#14,d7					; 14 Bytes per LINEDEF
	subq.w	#1,d7
	move.l	WADLINEDEFS,a2
	move.l	RLSTAIRS,a3
CSTS200
	moveq.l	#0,d0					; D0 = LINE_TYPE
	move.b	7(a2),d0
	lsl.w	#8,d0
	move.b	6(a2),d0
	cmp.w	#7,d0					; STAIRS?
	beq.s	CSTS300					; Yes!
	cmp.w	#8,d0					; STAIRS?
	bne	CSTS800					; No
;
;	>>>   GET LINEDEF'S TAG   <<<
;
CSTS300
	move.b	9(a2),d6				; LINEDEF TAG
	lsl.w	#8,d6
	move.b	8(a2),d6
	tst.w	d6					; Invalid Tag?
	beq	CSTS800					; YES!
;
;	>>>   SCAN ALL SECTORS FOR MATCHING TAG VALUE   <<<
;
	move.l	WADSECTORSSize,d1			; Get Size of SECTORS
	divu	#26,d1					; 26 Bytes per SECTOR
	subq.w	#1,d1
	move.l	WADSECTORS,a1
	moveq.l	#0,d5					; D5 = SECTOR#
CSTS320
	moveq.l	#0,d0					; SECTOR TAG
	move.b	25(a1),d0
	lsl.w	#8,d0
	move.b	24(a1),d0
	cmp.w	d0,d6					; MATCHES LINEDEF TAG?
	beq.s	CSTS400					; Yes!
	add.w	#26,a1					; Next WAD SECTOR
	addq.w	#1,d5					; Next SECTOR
	dbf	d1,CSTS320
	bra	CSTS800
;
;	>>>   ADD A NEW STAIR   <<<
;
CSTS400
	addq.l	#1,RLNUMSTAIRS				; One More Stair Defined!
	move.b	d6,(a3)+				; TAG VALUE
;
	move.l	d5,d0					; D0 = Sector#
	lsl.l	#2,d0					; 4 Bytes per Sector Origin
	lea	RLSECTOROrigins,a0
	add.l	d0,a0
	move.w	(a0)+,d0				; SECTOR ORIGIN X
	move.b	d0,(a3)+
	lsr.w	#8,d0
	move.b	d0,(a3)+
	move.w	(a0)+,d0				; SECTOR ORIGIN Y
	move.b	d0,(a3)+
	lsr.w	#8,d0
	move.b	d0,(a3)+
	move.b	(a1),(a3)+				; STARTING HEIGHT
	move.b	1(a1),(a3)+
	clr.b	(a3)					; NO SECTORS ADDED
	lea	1(a3),a4				; SECTORLIST
	move.l	a1,a5					; A5 = BASE SECTOR
;
;	>>>   ADD A NEW STAIR SECTOR   <<<
;
CSTS500
	move.b	d5,(a4)+				; SECTOR
	add.b	#1,(a3)					; SECTORCOUNT++
;
;	>>>   FIND NEXT STAIR SECTOR   <<<
;
	move.l	WADLINEDEFS,a1
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
CSTS520
	moveq.l	#0,d0
	move.b	11(a1),d0				; RIGHT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a1),d0
	move.l	WADSIDEDEFS,a0				; A0 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = RIGHT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	bne	CSTS580
	move.b	13(a1),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a1),d0
	cmp.w	#-1,d0					; LEFT SIDEDEF?
	bne	CSTS600					; YES!  FOUND NEXT SECTOR!
CSTS580
	add.w	#14,a1					; Next LINEDEF
	dbf	d2,CSTS520
	bra.s	CSTS700
;
;	>>>   FOUND NEXT STAIR SECTOR   <<<
;
CSTS600
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d1					; D1 = LEFT SECTOR# (NEW SECTOR!)
	move.b	29(a0),d1
	lsl.w	#8,d1
	move.b	28(a0),d1
	move.l	WADSECTORS,a0
	move.l	d1,d0
	mulu	#26,d0
	add.l	d0,a0
	move.l	4(a0),d0				; SAME FLOOR TEXTURE?
	cmp.l	4(a5),d0
	bne.s	CSTS580
	move.l	8(a0),d0
	cmp.l	8(a5),d0
	bne.s	CSTS580					; NO!  KEEP SCANNING!
	move.l	d1,d5					; D5 = RLSECTOR
	bra	CSTS500					; YES!  ADD NEXT SECTOR!
CSTS700
	move.l	a4,a3					; Update RLSTAIRS
;
;	* * * * * * *       NEXT LINEDEF       * * * * * * *
;
CSTS800
	add.w	#14,a2					; Next LINEDEF
	dbf	d7,CSTS200
	move.l	RLSTAIRS,a0				; Determine Size of STAIRS
	sub.l	a0,a3
	move.l	a3,RLSTAIRSSize
	move.l	RLNUMSTAIRS,-(sp)			; Print# STAIRS
	lea	NumSTAIRSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jmp	PrintMsg


;
;	* * * * * * *       CONVERT DOOM LEVEL FLOORS -> RL FLOORS       * * * * * * *
;
ConvertFLOORS
	clr.l	RLNUMFLOORS				; NO FLOORS Defined Yet!
	move.l	WADLINEDEFSSize,d7			; Get Size of LINEDEFS
	divu	#14,d7					; 14 Bytes per LINEDEF
	subq.w	#1,d7
	move.l	WADLINEDEFS,a2
	move.l	RLFLOORS,a3
CFLS200
	moveq.l	#0,d0					; D0 = LINE_TYPE
	move.b	7(a2),d0
	lsl.w	#8,d0
	move.b	6(a2),d0
	tst.w	d0					; Invalid?
	bmi	CFLS800					; Yes!
	move.l	d0,d5
	lsl.w	#2,d0
	lea	WADLINETYPEFLOORCODE,a0
	move.l	(a0,d0.w),d0				; Is it a FLOOR?
	beq	CFLS800					; No
	move.l	d0,a5
;
;	>>>   GET LINEDEF'S TAG   <<<
;
CFLS300
	move.b	9(a2),d6				; LINEDEF TAG
	lsl.w	#8,d6
	move.b	8(a2),d6
	tst.w	d6					; Invalid Tag?
	beq	CFLS800					; YES!
;
;	>>>   SCAN FOR EXISTING FLOORS   <<<
;
	move.l	RLFLOORS,a0
CFLS320
	cmp.l	a0,a3					; No, add it
	beq	CFLS350
	move.b	(a0)+,d0
	move.b	(a0)+,d1
	cmp.b	d0,d6
	bne.s	CFLS340
	cmp.b	d1,d5
	beq	CFLS800					; Already Added!
CFLS340
	moveq.l	#0,d0
	move.b	(a0)+,d0				; Get #Sectors
	mulu	#8,d0
	add.l	d0,a0
	bra	CFLS320					; Next FLOOR
;
;	>>>   ADD A NEW FLOOR   <<<
;
CFLS350
	movem.l	d7/a2,-(sp)
	addq.l	#1,RLNUMFLOORS				; One More FLOOR Defined!
	move.b	d6,(a3)+				; TAG VALUE
	move.b	d5,(a3)+				; TYPE VALUE
	move.l	a3,a4					; A4 = SECTOR COUNT
	clr.b	(a3)+					; NO SECTORS ADDED
;
;	>>>   SCAN ALL SECTORS FOR MATCHING TAG VALUE   <<<
;
	move.l	WADSECTORSSize,d7			; Get Size of SECTORS
	divu	#26,d7					; 26 Bytes per SECTOR
	subq.w	#1,d7
	move.l	WADSECTORS,a2
	moveq.l	#0,d5					; D5 = SECTOR#
CFLS400
	moveq.l	#0,d0					; SECTOR TAG
	move.b	25(a2),d0
	lsl.w	#8,d0
	move.b	24(a2),d0
	cmp.w	d0,d6					; MATCHES LINEDEF TAG?
	bne.s	CFLS700					; No
;
;	>>>   ADD A NEW FLOOR SECTOR   <<<
;
	add.b	#1,(a4)					; SECTORCOUNT++
;
	move.l	d5,d1					; D1 = RLSECTOR
	move.b	d1,(a3)+				; SECTOR
;
	jsr	(a5)					; CALCULATE DESTINATION HEIGHT
;
	move.b	d0,(a3)+				; DESTINATION HEIGHT
	lsr.w	#8,d0
	move.b	d0,(a3)+
;
	move.l	d5,d0					; D0 = Sector#
	lsl.l	#2,d0					; 4 Bytes per Sector Origin
	lea	RLSECTOROrigins,a0
	add.l	d0,a0
	move.w	(a0)+,d0				; SECTOR ORIGIN X
	move.b	d0,(a3)+
	lsr.w	#8,d0
	move.b	d0,(a3)+
	move.w	(a0)+,d0				; SECTOR ORIGIN Y
	move.b	d0,(a3)+
	lsr.w	#8,d0
	move.b	d0,(a3)+
;
	moveq.l	#0,d0					; MODEL SECTOR
	swap	d3
	move.w	d3,d0
	move.b	d0,(a3)+
;
;	>>>   NEXT SECTOR   <<<
;
CFLS700
	add.w	#26,a2					; Next WAD SECTOR
	addq.w	#1,d5					; Next SECTOR
	dbf	d7,CFLS400
	movem.l	(sp)+,d7/a2
;
;	>>>   DID WE JUST PROCESS "DONUT"?   <<<
;
	move.b	-1(a4),d0				; D0 = TYPE
	cmp.b	#9,d0					; DONUT?
	bne.s	CFLS800					; No!
;
;	>>>   ADD THE DONUT SECTOR TO THE FLOOR   <<<
;
	add.b	#1,(a4)+				; SECTORCOUNT++
	move.b	(a4)+,d0				; Get Hole Sector
	sub.w	#1,d0					; Get Donut Sector
	move.b	d0,(a3)+
	move.w	(a4)+,(a3)+				; Destination Height
	move.w	(a4)+,(a3)+				; Origin X
	move.w	(a4)+,(a3)+				; Origin Y
	move.b	(a4)+,(a3)+				; ModelSector
;
;	* * * * * * *       NEXT LINEDEF       * * * * * * *
;
CFLS800
	add.w	#14,a2					; Next LINEDEF
	dbf	d7,CFLS200
	move.l	RLFLOORS,a0				; Determine Size of FLOORS
	sub.l	a0,a3
	move.l	a3,RLFLOORSSize
	move.l	RLNUMFLOORS,-(sp)			; Print# FLOORS
	lea	NumFLOORSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jmp	PrintMsg

;
;	* * * * * * *       WAD LINE_TYPE FLOOR CODE TABLE       * * * * * * *
;
WADLINETYPEFLOORCODE
	dc.l	0,0,0,0,0				; 000-004
	dc.l	GetSectorLIC				; 005
	dc.l	0,0,0					; 006-008
	dc.l	GetSectorDonut				; 009
	dc.l	0,0,0,0					; 010-013
	dc.l	GetSectorF32				; 014
	dc.l	0,0,0					; 015-017
	dc.l	GetSectorNHEF				; 018
	dc.l	GetSectorHEF				; 019
	dc.l	GetSectorNHEF				; 020
	dc.l	0					; 021
	dc.l	GetSectorNHEF				; 022
	dc.l	GetSectorLEF				; 023
	dc.l	0,0,0,0,0,0				; 024-029
	dc.l	GetSectorF64				; 030
	dc.l	0,0,0,0,0				; 031-035
	dc.l	GetSectorHEF8				; 036
	dc.l	GetSectorLEF				; 037
	dc.l	GetSectorLEF				; 038
	dc.l	0					; 039
	dc.l	0,0,0,0,0,0,0,0,0,0			; 040-049
	dc.l	0,0,0,0,0,0				; 050-055
	dc.l	GetSectorLIC8				; 056
	dc.l	0					; 057
	dc.l	GetSectorF24				; 058
	dc.l	GetSectorF24				; 059
	dc.l	0,0,0,0,0,0,0,0,0,0			; 060-069
	dc.l	GetSectorHEF8				; 070
	dc.l	0,0,0,0,0,0,0,0,0			; 071-079
	dc.l	0,0					; 080-081
	dc.l	GetSectorLEF				; 082
	dc.l	0,0,0,0,0,0,0				; 083-089
	dc.l	0					; 090
	dc.l	GetSectorLIC				; 091
	dc.l	0,0,0,0,0,0				; 092-097
	dc.l	GetSectorHEF8				; 098
	dc.l	0					; 099
	dc.l	0,0					; 100-101
	dc.l	GetSectorHEF				; 102
	dc.l	0,0,0,0,0,0,0				; 103-109

;
;	* * * * * * *       GET CURRENT FLOOR       * * * * * * *
;
GetSectorF
	move.l	d5,d3					; D3 = MODEL SECTOR
	swap	d3
	move.l	d5,d0					; D0 = CURRENT SECTOR#
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	1(a0),d0				; D0 = CURRENT SECTOR FLOOR HEIGHT
	lsl.w	#8,d0
	move.b	0(a0),d0
	ext.l	d0
	rts
;
;	* * * * * * *       GET CURRENT FLOOR +24       * * * * * * *
;
GetSectorF24
	bsr	GetSectorF
	add.l	#24,d0
	rts
;
;	* * * * * * *       GET CURRENT FLOOR +32       * * * * * * *
;
GetSectorF32
	bsr	GetSectorF
	add.l	#32,d0
	rts
;
;	* * * * * * *       GET CURRENT FLOOR +64       * * * * * * *
;
GetSectorF64
	bsr	GetSectorF
	add.l	#64,d0
	rts

;
;	* * * * * * *       FIND HIGHEST FLOOR EXCLUDING CURRENT       * * * * * * *
;
GetSectorHEF8
	bsr	GetSectorHEF
	addq.l	#8,d0
	rts
GetSectorHEF
	move.l	d6,-(sp)
	move.l	d5,d3					; D3 = MODEL SECTOR
	swap	d3
	move.l	#$80000000,d6				; D6 = HIGHEST FLOOR HEIGHT
	move.l	WADLINEDEFS,a1
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
GHEF200
	moveq.l	#0,d0
	move.b	13(a1),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a1),d0
	cmp.w	#-1,d0					; LEFT SIDEDEF?
	beq	GHEF800					; NO!
	move.l	#10,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = LEFT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	beq	GHEF400
;
	moveq.l	#0,d0
	move.b	11(a1),d0				; RIGHT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a1),d0
	move.l	#12,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = RIGHT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	bne	GHEF800
GHEF400
	moveq.l	#0,d0
	move.b	1(a1,d1.w),d0				; GET OPPOSITE SIDEDEF
	lsl.w	#8,d0
	move.b	0(a1,d1.w),d0
	move.l	WADSIDEDEFS,a0				; A0 = OPPOSITE SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = OPPOSITE SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	move.w	d0,d3					; D3 = MODEL SECTOR
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	1(a0),d0				; D0 = SECTOR FLOOR HEIGHT
	lsl.w	#8,d0
	move.b	0(a0),d0
	ext.l	d0
	cmp.l	d0,d6					; Opposite Floor > Max Floor?
	bge	GHEF800					; No
	move.l	d0,d6					; Yes, New Max Floor!
	swap	d3					; New Model Sector
GHEF800
	add.w	#14,a1					; Next LINEDEF
	dbf	d2,GHEF200
	move.l	d6,d0					; D0 = HEIGHT
	move.l	(sp)+,d6
	rts

;
;	* * * * * * *       FIND LOWEST FLOOR EXCLUDING CURRENT       * * * * * * *
;
GetSectorLEF
	move.l	d6,-(sp)
	move.l	d5,d3					; D3 = MODEL SECTOR
	swap	d3
	move.l	#$7fffffff,d6				; D6 = LOWEST FLOOR HEIGHT
	move.l	WADLINEDEFS,a1
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
GLEF200
	moveq.l	#0,d0
	move.b	13(a1),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a1),d0
	cmp.w	#-1,d0					; LEFT SIDEDEF?
	beq	GLEF800					; NO!
	move.l	#10,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = LEFT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	beq	GLEF400
;
	moveq.l	#0,d0
	move.b	11(a1),d0				; RIGHT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a1),d0
	move.l	#12,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = RIGHT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	bne	GLEF800
GLEF400
	moveq.l	#0,d0
	move.b	1(a1,d1.w),d0				; GET OPPOSITE SIDEDEF
	lsl.w	#8,d0
	move.b	0(a1,d1.w),d0
	move.l	WADSIDEDEFS,a0				; A0 = OPPOSITE SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = OPPOSITE SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	move.w	d0,d3					; D3 = MODEL SECTOR
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	1(a0),d0				; D0 = SECTOR FLOOR HEIGHT
	lsl.w	#8,d0
	move.b	0(a0),d0
	ext.l	d0
	cmp.l	d0,d6					; Opposite Floor < Min Floor?
	blt	GLEF800					; No
	move.l	d0,d6					; Yes, New Min Floor!
	swap	d3
GLEF800
	add.w	#14,a1					; Next LINEDEF
	dbf	d2,GLEF200
	move.l	d6,d0					; D0 = HEIGHT
	move.l	(sp)+,d6
	rts

;
;	* * * * * * *       FIND NEXT-HIGHEST FLOOR EXCLUDING CURRENT       * * * * * * *
;
GetSectorNHEF
	move.l	d6,-(sp)
	move.l	d5,d3					; D3 = MODEL SECTOR
	swap	d3
	move.l	d5,d0					; D0 = CURRENT SECTOR#
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d4
	move.b	1(a0),d4				; D4 = CURRENT SECTOR FLOOR HEIGHT
	lsl.w	#8,d4
	move.b	0(a0),d4
	ext.l	d4
	move.l	#$7fffffff,d6				; D6 = NEXT-HIGHEST FLOOR HEIGHT
	move.l	WADLINEDEFS,a1
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
GNHEF200
	moveq.l	#0,d0
	move.b	13(a1),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a1),d0
	cmp.w	#-1,d0					; LEFT SIDEDEF?
	beq	GNHEF800				; NO!
	move.l	#10,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = LEFT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	beq	GNHEF400
;
	moveq.l	#0,d0
	move.b	11(a1),d0				; RIGHT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a1),d0
	move.l	#12,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = RIGHT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	bne	GNHEF800
GNHEF400
	moveq.l	#0,d0
	move.b	1(a1,d1.w),d0				; GET OPPOSITE SIDEDEF
	lsl.w	#8,d0
	move.b	0(a1,d1.w),d0
	move.l	WADSIDEDEFS,a0				; A0 = OPPOSITE SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = OPPOSITE SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	move.w	d0,d3					; MODEL SECTOR
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	1(a0),d0				; D0 = SECTOR FLOOR HEIGHT
	lsl.w	#8,d0
	move.b	0(a0),d0
	ext.l	d0
	cmp.l	d0,d6					; Opposite Floor > Max Floor?
	blt	GNHEF800				; No
	cmp.l	d0,d4
	bge	GNHEF800
	move.l	d0,d6					; Yes, New Max Floor!
	swap	d3					; New Model Sector
GNHEF800
	add.w	#14,a1					; Next LINEDEF
	dbf	d2,GNHEF200
	move.l	d6,d0					; D0 = HEIGHT
	move.l	(sp)+,d6
	rts

;
;	* * * * * * *       FIND LOWEST CEILING INCLUDING CURRENT       * * * * * * *
;
GetSectorLIC8
	bsr	GetSectorLIC
	subq.l	#8,d0
	rts
GetSectorLIC
	move.l	d6,-(sp)
	move.l	d5,d3					; D3 = MODEL SECTOR
	swap	d3
	move.l	d5,d0					; D0 = CURRENT SECTOR#
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d6
	move.b	3(a0),d6				; D6 = CURRENT SECTOR CEILING HEIGHT
	lsl.w	#8,d6
	move.b	2(a0),d6
	ext.l	d6
	move.l	WADLINEDEFS,a1
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
GLIC200
	moveq.l	#0,d0
	move.b	13(a1),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a1),d0
	cmp.w	#-1,d0					; LEFT SIDEDEF?
	beq	GLIC800					; NO!
	move.l	#10,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = LEFT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	beq	GLIC400
;
	moveq.l	#0,d0
	move.b	11(a1),d0				; RIGHT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a1),d0
	move.l	#12,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = RIGHT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	bne	GLIC800
GLIC400
	moveq.l	#0,d0
	move.b	1(a1,d1.w),d0				; GET OPPOSITE SIDEDEF
	lsl.w	#8,d0
	move.b	0(a1,d1.w),d0
	move.l	WADSIDEDEFS,a0				; A0 = OPPOSITE SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = OPPOSITE SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	move.w	d0,d3					; MODEL SECTOR
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	3(a0),d0				; D0 = SECTOR CEILING HEIGHT
	lsl.w	#8,d0
	move.b	2(a0),d0
	ext.l	d0
	cmp.l	d0,d6					; Opposite Ceiling < Min Ceiling?
	blt	GLIC800					; No
	move.l	d0,d6					; Yes, New Min Ceiling!
	swap	d3					; New Model Sector
GLIC800
	add.w	#14,a1					; Next LINEDEF
	dbf	d2,GLIC200
	move.l	d6,d0					; D0 = HEIGHT
	move.l	(sp)+,d6
	rts

;
;	* * * * * * *       FIND LOWEST FLOOR INCLUDING CURRENT       * * * * * * *
;
GetSectorLIF
	move.l	d6,-(sp)
	move.l	d5,d3					; D3 = MODEL SECTOR
	swap	d3
	move.l	d5,d0					; D0 = CURRENT SECTOR#
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d6
	move.b	1(a0),d6				; D6 = CURRENT SECTOR FLOOR HEIGHT
	lsl.w	#8,d6
	move.b	0(a0),d6
	ext.l	d6
	move.l	WADLINEDEFS,a1
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
GLIF200
	moveq.l	#0,d0
	move.b	13(a1),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a1),d0
	cmp.w	#-1,d0					; LEFT SIDEDEF?
	beq	GLIF800					; NO!
	move.l	#10,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = LEFT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	beq	GLIF400
;
	moveq.l	#0,d0
	move.b	11(a1),d0				; RIGHT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a1),d0
	move.l	#12,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = RIGHT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	bne	GLIF800
GLIF400
	moveq.l	#0,d0
	move.b	1(a1,d1.w),d0				; GET OPPOSITE SIDEDEF
	lsl.w	#8,d0
	move.b	0(a1,d1.w),d0
	move.l	WADSIDEDEFS,a0				; A0 = OPPOSITE SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = OPPOSITE SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	move.w	d0,d3					; MODEL SECTOR
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	1(a0),d0				; D0 = SECTOR FLOOR HEIGHT
	lsl.w	#8,d0
	move.b	0(a0),d0
	ext.l	d0
	cmp.l	d0,d6					; Opposite Floor < Min Floor?
	blt	GLIF800					; No
	move.l	d0,d6					; Yes, New Min Floor!
	swap	d3					; New Model Sector
GLIF800
	add.w	#14,a1					; Next LINEDEF
	dbf	d2,GLIF200
	move.l	d6,d0					; D0 = HEIGHT
	move.l	(sp)+,d6
	rts

;
;	* * * * * * *       FIND HIGHEST FLOOR INCLUDING CURRENT       * * * * * * *
;
GetSectorHIF
	move.l	d6,-(sp)
	move.l	d5,d3					; D3 = MODEL SECTOR
	swap	d3
	move.l	d5,d0					; D0 = CURRENT SECTOR#
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d6
	move.b	1(a0),d6				; D6 = CURRENT SECTOR FLOOR HEIGHT
	lsl.w	#8,d6
	move.b	0(a0),d6
	ext.l	d6
	move.l	WADLINEDEFS,a1
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
GHIF200
	moveq.l	#0,d0
	move.b	13(a1),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a1),d0
	cmp.w	#-1,d0					; LEFT SIDEDEF?
	beq	GHIF800					; NO!
	move.l	#10,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = LEFT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	beq	GHIF400
;
	moveq.l	#0,d0
	move.b	11(a1),d0				; RIGHT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a1),d0
	move.l	#12,d1					; D1 = SIDEDEF
	move.l	WADSIDEDEFS,a0				; A0 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = RIGHT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	bne	GHIF800
GHIF400
	moveq.l	#0,d0
	move.b	1(a1,d1.w),d0				; GET OPPOSITE SIDEDEF
	lsl.w	#8,d0
	move.b	0(a1,d1.w),d0
	move.l	WADSIDEDEFS,a0				; A0 = OPPOSITE SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = OPPOSITE SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	move.w	d0,d3					; MODEL SECTOR
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	1(a0),d0				; D0 = SECTOR FLOOR HEIGHT
	lsl.w	#8,d0
	move.b	0(a0),d0
	ext.l	d0
	cmp.l	d0,d6					; Opposite Floor > Max Floor?
	bge	GHIF800					; No
	move.l	d0,d6					; Yes, New Max Floor!
	swap	d3					; New Model Sector
GHIF800
	add.w	#14,a1					; Next LINEDEF
	dbf	d2,GHIF200
	move.l	d6,d0					; D0 = HEIGHT
	move.l	(sp)+,d6
	rts

;
;	* * * * * * *       E1M2 DONUT SPECIAL       * * * * * * *
;
GetSectorDonut
	subq.l	#1,d5					; D5 = DONUT SECTOR
	bsr	GetSectorNHEF				; Get MODEL Sector Height
	addq.l	#1,d5
	rts


;
;	* * * * * * *       CONVERT DOOM LEVEL LIFTS -> RL LIFTS       * * * * * * *
;
ConvertLIFTS
	clr.l	RLNUMLIFTS				; NO LIFTS Defined Yet!
	move.l	WADLINEDEFSSize,d7			; Get Size of LINEDEFS
	divu	#14,d7					; 14 Bytes per LINEDEF
	subq.w	#1,d7
	move.l	WADLINEDEFS,a2
	move.l	RLLIFTS,a3
CLFS200
	moveq.l	#0,d0					; D0 = LINE_TYPE
	move.b	7(a2),d0
	lsl.w	#8,d0
	move.b	6(a2),d0
	tst.w	d0					; Invalid?
	bmi	CLFS800					; Yes!
	move.l	d0,d5
	lsl.w	#2,d0
	lea	WADLINETYPELIFTCODE,a0
	move.l	(a0,d0.w),d0				; Is it a LIFT?
	beq	CLFS800					; No
	move.l	d0,a5
;
;	>>>   GET LINEDEF'S TAG   <<<
;
CLFS300
	move.b	9(a2),d6				; LINEDEF TAG
	lsl.w	#8,d6
	move.b	8(a2),d6
	tst.w	d6					; Invalid Tag?
	beq	CLFS800					; YES!
;
;	>>>   SCAN FOR EXISTING LIFTS   <<<
;
	move.l	RLLIFTS,a0
CLFS320
	cmp.l	a0,a3					; No, add it
	beq	CLFS350
	move.b	(a0)+,d0
	move.b	(a0)+,d1
	cmp.b	d0,d6
	bne.s	CLFS340
	cmp.b	d1,d5
	beq	CLFS800					; Already Added!
CLFS340
	moveq.l	#0,d0
	move.b	(a0)+,d0				; Get #Sectors
	mulu	#9,d0
	add.l	d0,a0
	bra	CLFS320					; Next LIFT
;
;	>>>   ADD A NEW LIFT   <<<
;
CLFS350
	movem.l	d7/a2,-(sp)
	addq.l	#1,RLNUMLIFTS				; One More LIFT Defined!
	move.b	d6,(a3)+				; TAG VALUE
	move.b	d5,(a3)+				; TYPE VALUE
	move.l	a3,a4					; A4 = SECTOR COUNT
	clr.b	(a3)+					; NO SECTORS ADDED
;
;	>>>   SCAN ALL SECTORS FOR MATCHING TAG VALUE   <<<
;
	move.l	WADSECTORSSize,d7			; Get Size of SECTORS
	divu	#26,d7					; 26 Bytes per SECTOR
	subq.w	#1,d7
	move.l	WADSECTORS,a2
	moveq.l	#0,d5					; D5 = SECTOR#
CLFS400
	moveq.l	#0,d0					; SECTOR TAG
	move.b	25(a2),d0
	lsl.w	#8,d0
	move.b	24(a2),d0
	cmp.w	d0,d6					; MATCHES LINEDEF TAG?
	bne.s	CLFS700					; No
;
;	>>>   ADD A NEW LIFT SECTOR   <<<
;
	add.b	#1,(a4)					; SECTORCOUNT++
;
	move.l	d5,d1					; D1 = RLSECTOR
	move.b	d1,(a3)+				; SECTOR
;
	moveq.l	#0,d0					; CALCULATE MINIMUM HEIGHT
	jsr	(a5)
	move.b	d0,(a3)+				; MINIMUM HEIGHT
	lsr.w	#8,d0
	move.b	d0,(a3)+
;
	moveq.l	#1,d0					; CALCULATE MAXIMUM HEIGHT
	jsr	(a5)
	move.b	d0,(a3)+				; MAXIMUM HEIGHT
	lsr.w	#8,d0
	move.b	d0,(a3)+
;
	move.l	d5,d0					; D0 = Sector#
	lsl.l	#2,d0					; 4 Bytes per Sector Origin
	lea	RLSECTOROrigins,a0
	add.l	d0,a0
	move.w	(a0)+,d0				; SECTOR ORIGIN X
	move.b	d0,(a3)+
	lsr.w	#8,d0
	move.b	d0,(a3)+
	move.w	(a0)+,d0				; SECTOR ORIGIN Y
	move.b	d0,(a3)+
	lsr.w	#8,d0
	move.b	d0,(a3)+
;
;	>>>   NEXT SECTOR   <<<
;
CLFS700
	add.w	#26,a2					; Next WAD SECTOR
	addq.w	#1,d5					; Next SECTOR
	dbf	d7,CLFS400
	movem.l	(sp)+,d7/a2
;
;	* * * * * * *       NEXT LINEDEF       * * * * * * *
;
CLFS800
	add.w	#14,a2					; Next LINEDEF
	dbf	d7,CLFS200
	move.l	RLLIFTS,a0				; Determine Size of LIFTS
	sub.l	a0,a3
	move.l	a3,RLLIFTSSize
	move.l	RLNUMLIFTS,-(sp)			; Print# LIFTS
	lea	NumLIFTSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jmp	PrintMsg

;
;	* * * * * * *       GET LIFT HEIGHTS       * * * * * * *
;
;	D0 = 0 = Lowest, = 1 = Highest
;
GetLiftHeights
	tst.l	d0					; Lowest/Highest?
	beq.s	GLH500					; Lowest
	move.l	d5,d0					; D0 = CURRENT SECTOR#
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	1(a0),d0
	lsl.w	#8,d0
	move.b	0(a0),d0
	ext.l	d0
	rts
GLH500
	jmp	GetSectorLIF
;
;	* * * * * * *       GET MOVING FLOOR SECTOR HEIGHTS       * * * * * * *
;
;	D0 = 0 = Lowest, = 1 = Highest
;
GetMovingFloorHeights
	tst.l	d0					; Lowest/Highest?
	beq.s	GMVH500					; Lowest
	jmp	GetSectorHIF
GMVH500
	jmp	GetSectorLIF


;
;	* * * * * * *       WAD LINE_TYPE LIFT CODE TABLE       * * * * * * *
;
WADLINETYPELIFTCODE
	dc.l	0,0,0,0,0,0,0,0,0,0			; 000-009
	dc.l	GetLiftHeights				; 010
	dc.l	0,0,0,0,0,0,0,0,0			; 011-019
	dc.l	0					; 020
	dc.l	GetLiftHeights				; 021
	dc.l	0,0,0,0,0,0,0,0				; 022-029
	dc.l	0,0,0,0,0,0,0,0,0,0			; 030-039
	dc.l	0,0,0,0,0,0,0,0,0,0			; 040-049
	dc.l	0,0,0,0,0,0,0,0,0,0			; 050-059
	dc.l	0,0					; 060-061
	dc.l	GetLiftHeights				; 062
	dc.l	0,0,0,0,0,0,0				; 063-069
	dc.l	0,0,0,0,0,0,0,0,0,0			; 070-079
	dc.l	0,0,0,0,0,0,0				; 080-086
	dc.l	GetMovingFloorHeights			; 087
	dc.l	GetLiftHeights				; 088
	dc.l	GetMovingFloorHeights			; 089
	dc.l	0,0,0,0,0,0,0,0,0,0			; 090-099
	dc.l	0,0,0,0,0,0,0,0,0,0			; 100-109


;
;	* * * * * * *       CONVERT DOOM LEVEL CEILINGS -> RL CEILINGS       * * * * * * *
;
ConvertCEILINGS
	clr.l	RLNUMCEILINGS				; NO CEILINGS Defined Yet!
	move.l	WADLINEDEFSSize,d7			; Get Size of LINEDEFS
	divu	#14,d7					; 14 Bytes per LINEDEF
	subq.w	#1,d7
	move.l	WADLINEDEFS,a2
	move.l	RLCEILINGS,a3
CCLS200
	moveq.l	#0,d0					; D0 = LINE_TYPE
	move.b	7(a2),d0
	lsl.w	#8,d0
	move.b	6(a2),d0
	tst.w	d0					; Invalid?
	bmi	CCLS800					; Yes!
	move.l	d0,d5
	lsl.w	#2,d0
	lea	WADLINETYPECEILINGCODE,a0
	move.l	(a0,d0.w),d0				; Is it a CEILING?
	beq	CCLS800					; No
;
;	>>>   GET LINEDEF'S TAG   <<<
;
CCLS300
	move.b	9(a2),d6				; LINEDEF TAG
	lsl.w	#8,d6
	move.b	8(a2),d6
	tst.w	d6					; Invalid Tag?
	beq	CCLS800					; YES!
;
;	>>>   SCAN FOR EXISTING CEILINGS   <<<
;
	move.l	RLCEILINGS,a0
CCLS320
	cmp.l	a0,a3					; No, add it
	beq	CCLS380
	move.b	(a0)+,d0
	move.b	(a0)+,d1
	cmp.b	d0,d6
	bne.s	CCLS340
	cmp.b	d1,d5
	beq	CCLS800					; Already Added!
CCLS340
	moveq.l	#0,d1
	move.b	(a0)+,d1				; Get #Sectors
	subq.w	#1,d1
CCLS350
	moveq.l	#0,d0					; Get #Lines
	move.b	7(a0),d0
	add.w	#8,a0
	lsl.w	#1,d0
	add.l	d0,a0
	dbf	d1,CCLS350
	bra	CCLS320					; Next CEILING
;
;	>>>   ADD A NEW CEILING   <<<
;
CCLS380
	movem.l	d7/a2,-(sp)
	addq.l	#1,RLNUMCEILINGS			; One More CEILING Defined!
	move.b	d6,(a3)+				; TAG VALUE
	move.b	d5,(a3)+				; TYPE VALUE
	move.l	a3,a4					; A4 = SECTOR COUNT
	clr.b	(a3)+					; NO SECTORS ADDED
;
;	>>>   SCAN ALL SECTORS FOR MATCHING TAG VALUE   <<<
;
	move.l	WADSECTORSSize,d7			; Get Size of SECTORS
	divu	#26,d7					; 26 Bytes per SECTOR
	subq.w	#1,d7
	move.l	WADSECTORS,a2
	moveq.l	#0,d5					; D5 = SECTOR#
CCLS400
	moveq.l	#0,d0					; SECTOR TAG
	move.b	25(a2),d0
	lsl.w	#8,d0
	move.b	24(a2),d0
	cmp.w	d0,d6					; MATCHES LINEDEF TAG?
	bne	CCLS700					; No
;
;	>>>   ADD A NEW CEILING SECTOR   <<<
;
	add.b	#1,(a4)					; SECTORCOUNT++
;
	move.l	d5,d1					; D1 = RLSECTOR
	move.b	d1,(a3)+				; SECTOR
;
	move.l	d5,d0					; D0 = CURRENT SECTOR#
	mulu	#26,d0
	move.l	WADSECTORS,a0				; A0 = SECTOR
	add.l	d0,a0
	move.w	2(a0),(a3)+				; CURRENT SECTOR CEILING HEIGHT
;
	move.l	d5,d0					; D0 = Sector#
	lsl.l	#2,d0					; 4 Bytes per Sector Origin
	lea	RLSECTOROrigins,a0
	add.l	d0,a0
	move.w	(a0)+,d0				; SECTOR ORIGIN X
	move.b	d0,(a3)+
	lsr.w	#8,d0
	move.b	d0,(a3)+
	move.w	(a0)+,d0				; SECTOR ORIGIN Y
	move.b	d0,(a3)+
	lsr.w	#8,d0
	move.b	d0,(a3)+
;
	clr.b	(a3)					; NO LINES ADDED
	lea	1(a3),a5				; LINELIST
	move.l	WADLINEDEFS,a1
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
CCLS500
	moveq.l	#0,d0
	move.b	13(a1),d0				; LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	12(a1),d0
	cmp.w	#-1,d0					; LEFT SIDEDEF?
	beq	CCLS520					; NO!
	move.l	WADSIDEDEFS,a0				; A0 = LEFT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = LEFT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	beq	CCLS550
CCLS520
	moveq.l	#0,d0
	move.b	11(a1),d0				; RIGHT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a1),d0
	move.l	WADSIDEDEFS,a0				; A0 = RIGHT SIDEDEF
	mulu	#30,d0
	add.l	d0,a0
	moveq.l	#0,d0					; D0 = RIGHT SECTOR#
	move.b	29(a0),d0
	lsl.w	#8,d0
	move.b	28(a0),d0
	cmp.w	d0,d5
	bne	CCLS580
CCLS550
	move.l	a1,d4					; D4 = LINE#
	sub.l	WADLINEDEFS,d4
	divu	#14,d4					; 14 Bytes per LINEDEF
	move.w	d4,d0
	move.b	d0,(a5)+				; ADD LINE TO LINELIST
	lsr.w	#8,d0
	move.b	d0,(a5)+
	add.b	#1,(a3)					; LINECOUNT++
CCLS580
	add.w	#14,a1					; Next LINEDEF
	dbf	d2,CCLS500
	move.l	a5,a3
;
;	>>>   NEXT SECTOR   <<<
;
CCLS700
	add.w	#26,a2					; Next WAD SECTOR
	addq.w	#1,d5					; Next SECTOR
	dbf	d7,CCLS400
	movem.l	(sp)+,d7/a2
;
;	* * * * * * *       NEXT LINEDEF       * * * * * * *
;
CCLS800
	add.w	#14,a2					; Next LINEDEF
	dbf	d7,CCLS200
	move.l	RLCEILINGS,a0				; Determine Size of CEILINGS
	sub.l	a0,a3
	move.l	a3,RLCEILINGSSize
	move.l	RLNUMCEILINGS,-(sp)			; Print# CEILINGS
	lea	NumCEILINGSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jmp	PrintMsg

;
;	* * * * * * *       WAD LINE_TYPE CEILING CODE TABLE       * * * * * * *
;
WADLINETYPECEILINGCODE
	dc.l	0,0,0,0,0,0,0,0,0,0			; 000-009
	dc.l	0,0,0,0,0,0,0,0,0,0			; 010-019
	dc.l	0,0,0,0,0,0,0,0,0,0			; 020-029
	dc.l	0,0,0,0,0,0,0,0,0,0			; 030-039
	dc.l	0,0,0,0,0,0,0,0,0,0			; 040-049
	dc.l	0,0,0,0,0,0,0,0,0,0			; 050-059
	dc.l	0,0,0,0,0,0,0,0,0,0			; 060-069
	dc.l	0,0,0,1,1,0,0,1,0,0			; 070-079
	dc.l	0,0,0,0,0,0,0,0,0,0			; 080-089
	dc.l	0,0,0,0,0,0,0,0,0,0			; 090-099
	dc.l	0,0,0,0,0,0,0,0,0,0			; 100-109


;
;	* * * * * * *       CONVERT DOOM LEVEL SIDEDEFS -> RL FACES       * * * * * * *
;
ConvertSIDEDEFS
	move.l	WADSIDEDEFSSize,d0			; Get Size of SIDEDEFS
	divu	#30,d0					; 30 Bytes per SIDEDEF
	move.l	d0,-(sp)
	lea	NumSIDEDEFSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(1*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	WADSIDEDEFSSize,d7			; Get Size of SIDEDEFS
	divu	#30,d7					; 30 Bytes per SIDEDEF
	subq.w	#1,d7
	move.l	WADSIDEDEFS,a2
	move.l	RLFACES,a3
	moveq.l	#0,d6					; D6 = SEGMENT NUMBER
;
;	>>>   PROCESS THE NEXT SIDEDEF   <<<
;
CSDS200
	sub.w	#(11*4),sp
	move.l	d6,0(sp)				; SIDEDEF NUMBER
;
	ifd	egad
	moveq.l	#0,d0					; X OFFSET
	move.b	1(a2),d0
	lsl.w	#8,d0
	move.b	0(a2),d0
	move.l	d0,4(sp)
	moveq.l	#0,d0					; Y OFFSET
	move.b	3(a2),d0
	lsl.w	#8,d0
	move.b	2(a2),d0
	move.l	d0,8(sp)
	endc
;
	moveq.l	#0,d0					; SECTOR NEAR
	move.b	29(a2),d0
	lsl.w	#8,d0
	move.b	28(a2),d0
	move.l	d0,36(sp)
	move.l	d0,d3
	move.b	d0,0(a3)
;
;	>>>   SCAN LINEDEFS FOR FAR SECTOR   <<<
;
CSDS2000
	move.l	WADLINEDEFSSize,d2			; Get Size of LINEDEFS
	divu	#14,d2					; 14 Bytes per LINEDEF
	subq.w	#1,d2
	move.l	WADLINEDEFS,a0
CSDS2200
	moveq.l	#0,d1
	move.b	13(a0),d1				; D1 = RIGHT SIDEDEF
	lsl.w	#8,d1
	move.b	12(a0),d1
	moveq.l	#0,d0
	move.b	11(a0),d0				; D0 = LEFT SIDEDEF
	lsl.w	#8,d0
	move.b	10(a0),d0
	cmp.w	d1,d6					; RIGHT SIDEDEF Matches?
	beq.s	CSDS2400				; Yes, use LEFT
	cmp.w	d0,d6					; LEFT SIDEDEF Matches?
	bne.s	CSDS2300				; No
	move.l	d1,d0					; Yes, use RIGHT
	bra.s	CSDS2400
CSDS2300
	add.w	#14,a0					; Next LINEDEF
	dbf	d2,CSDS2200
CSDS2380
	moveq.l	#-1,d0					; NO FAR SECTOR
	bra.s	CSDS2700
CSDS2400
	cmp.w	#-1,d0					; No Opposite SIDEDEF?
	beq.s	CSDS2300				; Yes!  Keep Scanning!
	cmp.w	d0,d6					; OPPOSITE SIDEDEF same as NEAR?
	beq.s	CSDS2300				; Yes!
	mulu	#30,d0					; A0 = SIDEDEF
	move.l	WADSIDEDEFS,a0
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	29(a0),d0				; Get SECTOR
	lsl.w	#8,d0
	move.b	28(a0),d0
CSDS2700
	move.l	d0,40(sp)
	move.b	d0,1(a3)
;
	move.l	d0,d5					; D5 = FAR SECTOR (or -1)
	moveq.l	#0,d4					; D4 = FLAGS
	clr.w	TextureXOffset				; NO TEXTURE X OFFSET
	clr.w	TextureYOffset				; NO TEXTURE Y OFFSET
;
;	>>>   NORMAL   <<<
;
	lea	20(a2),a1				; NORMAL TEXTURE
	lea	Texture1Name,a0
	bsr	XREFTexture
	move.l	#Texture1Name,12(sp)
	move.l	d0,24(sp)
	move.l	#1,d1
	tst.w	d5					; FAR SECTOR?
	bmi.s	CSDS3200				; NO
	move.l	#2,d1					; YES!
	bset	#0,d4					; DEFAULT TO CLEAR!
CSDS3200
	move.b	d0,0(a3,d1.w)
	lea	Texture1Name,a0				; NORMAL SWITCH?
	move.w	(a0),d0
	cmp.w	#'SW',d0
	bne.s	CSDS3300				; No
	bset	#6,d4					; YES!
;
;	>>>   LOWER   <<<
;
CSDS3300
	lea	12(a2),a1				; LOWER TEXTURE
	lea	Texture2Name,a0
	bsr	XREFTexture
	move.l	#Texture2Name,16(sp)
	move.l	d0,28(sp)
	tst.w	d5					; ONE-SIDED?
	bmi.s	CSDS3400				; Yes!
	move.b	d0,3(a3)
	tst.w	d0
	bmi.s	CSDS3400
	bclr	#0,d4					; NOT CLEAR
	lea	Texture2Name,a0				; LOWER SWITCH?
	move.w	(a0),d0
	cmp.w	#'SW',d0
	bne.s	CSDS3400				; No
	bset	#6,d4					; YES!
;
;	>>>   UPPER   <<<
;
CSDS3400
	lea	4(a2),a1				; UPPER TEXTURE
	lea	Texture3Name,a0
	bsr	XREFTexture
	move.l	#Texture3Name,20(sp)
	move.l	d0,32(sp)
	tst.w	d5					; ONE-SIDED?
	bmi.s	CSDS3500				; Yes!
	move.b	d0,2(a3)
	tst.w	d0
	bmi.s	CSDS3500
	bclr	#0,d4					; NOT CLEAR
	lea	Texture3Name,a0				; UPPER SWITCH?
	move.w	(a0),d0
	cmp.w	#'SW',d0
	bne.s	CSDS3500				; No
	bset	#5,d4					; YES!
;
;	>>>   HANDLE SKY   <<<
;
CSDS3500
	move.l	RLSECTORS,a0				; SKY IN NEAR SECTOR?
	mulu	#rlsSize,d3
	add.l	d3,a0
	move.b	8(a0),d0				; SKY?
	cmp.b	#$ff,d0
	bne.s	CSDS4000				; No!
	tst.w	d5					; FAR SECTOR?
	bmi.s	CSDS3600				; No!
	mulu	#rlsSize,d5
	move.l	RLSECTORS,a0
	add.l	d5,a0
	move.b	8(a0),d0				; SKY IN FAR SECTOR?
	cmp.b	#$ff,d0
	bne.s	CSDS4000				; No!
CSDS3600
	move.l	#TextureSKYMsg,20(sp)
	bset	#1,d4					; SKY!
;
;	>>>   HANDLE TEXTURE TRANSLATION X OFFSET   <<<
;
CSDS4000
	moveq.l	#0,d0					; ADD TRANSLATION TEXTURE X OFFSET
	move.b	1(a2),d0
	lsl.w	#8,d0
	move.b	0(a2),d0
	add.w	TextureXOffset,d0
	ext.l	d0
	move.l	d0,4(sp)
	move.b	d0,0(a2)
	lsr.w	#8,d0
	move.b	d0,1(a2)
;
;	>>>   HANDLE TEXTURE TRANSLATION Y OFFSET   <<<
;
	moveq.l	#0,d0					; ADD TRANSLATION TEXTURE Y OFFSET
	move.b	3(a2),d0
	lsl.w	#8,d0
	move.b	2(a2),d0
	add.w	TextureYOffset,d0
	ext.l	d0
	move.l	d0,8(sp)
	move.b	d0,2(a2)
	lsr.w	#8,d0
	move.b	d0,3(a2)
;
;	>>>   ADD FACE DATA TO RLFACES   <<<
;
	lea	RLFACESFlags,a0				; Save FACES Flags
	move.b	d4,(a0,d6.w)
	move.l	d6,d0
	lsl.w	#2,d0
	lea	RLFACESPtrs,a1				; Save FACES Pointer
	add.l	d0,a1
	move.l	a3,(a1)
;
;	>>>   SCAN FACES DATA TO FIND DUPLICATED FACE INFORMATION   <<<
;
	move.l	RLFACES,a4				; A4 = OLD FACE DATA
	subq.w	#1,a4
	moveq.l	#(rlfSizeS-1),d1			; D1 = SIZE OF SOLID FACE-1
	tst.w	d5
	bmi.s	CSDS8100
	moveq.l	#(rlfSizeT-1),d1			; D1 = SIZE OF TRANSPARENT FACE-1
CSDS8100
	move.l	a3,a5					; A5 = NEW FACE DATA
	addq.w	#1,a4
	move.l	a4,a0					; A0 = OLD FACE DATA
	move.l	d1,d0					; D0 = #BYTES IN NEW FACE
CSDS8200
	cmpm.b	(a5)+,(a0)+				; FACE DATA MATCHES?
	bne.s	CSDS8100				; No, Try next byte
	dbf	d0,CSDS8200
	move.l	(a1),d0					; D0 = APPENDED FACE
	cmp.l	a4,d0					; Matched the Newly Added FACE?
	beq.s	CSDS8500				; Yes, Add the NEW FACE!
	move.l	a4,(a1)					; NO!  THIS FACE IS DUPLICATED!
	addq.w	#1,a4
	add.w	d1,a4
	cmp.l	a4,a3					; Using Partially Added Data?
	bcc.s	CSDS8600				; No
	move.l	a4,a3					; YES!  ADD PARTIAL DATA!
	bra.s	CSDS8600
CSDS8500
	addq.w	#1,d1					; ADD THIS NEW FACE!
	add.w	d1,a3
;
CSDS8600
	move.l	d7,d5
	lea	SIDEDEFDataMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(11*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	d5,d7
;
	add.w	#30,a2					; Next WAD SIDEDEF
	addq.w	#1,d6
	dbf	d7,CSDS200
;
	move.l	RLFACES,a0				; Determine Size of FACES
	sub.l	a0,a3
	move.l	a3,RLFACESSize
	rts


;
;	* * * * * * *       CONVERT DOOM LEVEL BLOCKMAP -> RL BLOCKMAP       * * * * * * *
;
ConvertBLOCKMAP
	sub.w	#(5*4),sp
	move.l	WADBLOCKMAP,a2
	move.l	RLBLOCKMAP,a3
;
	moveq.l	#0,d2
	move.w	(a2)+,d2				; X Origin
	move.w	d2,(a3)+
	move.l	d2,d0
	lsl.w	#8,d2
	lsr.w	#8,d0
	or.w	d0,d2
	move.l	d2,0(sp)
;
	moveq.l	#0,d3
	move.w	(a2)+,d3				; Y Origin
	move.w	d3,(a3)+
	move.l	d3,d0
	lsl.w	#8,d3
	lsr.w	#8,d0
	or.w	d0,d3
	move.l	d3,4(sp)
;
	moveq.l	#0,d0
	move.b	(a2)+,d0				; X Blocks
	move.b	d0,(a3)+
	addq.w	#1,a2
	moveq.l	#0,d1
	move.b	(a2)+,d1				; Y Blocks
	move.b	d1,(a3)+
	addq.w	#1,a2
	move.l	d0,d4					; D4 = #Blocks to Process
	mulu	d1,d4
	move.l	d4,16(sp)
;
	lsl.l	#7,d0
	add.w	d0,d2
	move.l	d2,8(sp)
	lsl.l	#7,d1
	add.w	d1,d3
	move.l	d3,12(sp)
;
	move.l	a3,a1					; A1 points to RL BLOCKMAP OFFSETS
	move.l	d4,d0					; D0 = (#BLOCKS*2)
	lsl.w	#1,d0
	add.l	d0,a3					; A3 points to RL LINELISTS
	add.l	d0,a2					; A2 points to WAD LINELISTS
	subq.w	#1,d4					; D4 = #BLOCKS-1
	move.l	a3,a6					; A6 points to START of RL LINELISTS
CBMP2000
	move.l	a3,d0					; Calculate next Offset
	sub.l	a1,d0
	subq.l	#(2-1),d0
	move.b	d0,(a1)+
	lsr.w	#8,d0
	move.b	d0,(a1)+
;
	addq.w	#2,a2					; Skip WAD LINELIST $0000 Entry
	moveq.l	#0,d0
	move.l	a3,a5					; A5 points to NEW RL LINELIST
CBMP2100
	move.w	(a2)+,d0				; Copy Line#s
	move.w	d0,(a3)+
	tst.b	d0
	bpl.s	CBMP2100				; Not Finished!
;
;	>>>   SCAN BLOCKMAP LINELISTS TO FIND DUPLICATED BLOCKMAP LINELISTS   <<<
;
	move.l	a6,a4					; A4 = OLD BLOCKMAP DATA
	subq.w	#1,a4
	move.l	a3,d1					; D1 = SIZE OF NEW LINELIST-1
	sub.l	a5,d1
	subq.l	#1,d1
CBMP2200
	move.l	a5,a3					; A3 = NEW RL LINELIST
	addq.w	#1,a4
	move.l	a4,a0					; A0 = OLD LINELIST DATA
	move.l	d1,d0					; D0 = #BYTES IN NEW LINELIST
CBMP2300
	cmpm.b	(a3)+,(a0)+				; BLOCKMAP DATA MATCHES?
	bne.s	CBMP2200				; No, Try next byte
	dbf	d0,CBMP2300
;
	move.l	a4,d0					; Calculate NEW Offset
	sub.l	a1,d0
	sub.l	#(2-1-2),d0
	move.b	-1(a1),d1
	lsl.w	#8,d1
	move.b	-2(a1),d1
	cmp.w	d0,d1					; Mathced the Newly Added LINELIST?
	beq.s	CBMP2500				; YES, ADD THE NEW LINELIST!
	move.b	d0,-2(a1)				; NO!  THIS LINELIST IS DUPLICATED!
	lsr.w	#8,d0
	move.b	d0,-1(a1)
	move.l	a5,a3					; DON'T ADD THIS NEW LINELIST!
;
CBMP2500
	dbf	d4,CBMP2000				; Next BLOCKMAP BLOCK
;
	move.l	RLBLOCKMAP,a0				; Determine Size of BLOCKMAP
	sub.l	a0,a3
	move.l	a3,RLBLOCKMAPSize
;
	lea	NumBLOCKMAPMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(5*4),sp
	moveq.l	#-1,d7
	jmp	PrintMsg


;
;	* * * * * * *       CROSS-REFERENCE SIDEDEF WALL TEXTURE       * * * * * * *
;
;	A1 = Pointer to 8-Character Texture Name
;	A0 = Pointer to Destination Texture Name
;
;	D0 = RL Texture Number
;
XREFTexture
	move.l	a2,-(sp)
	movem.l	a0/a1,-(sp)
	moveq.l	#8-1,d1					; 8 Characters Maximum
XTE200
	move.b	(a1)+,d0
	beq.s	XTE210
	jsr	ToUpper
	move.b	d0,(a0)+
	dbf	d1,XTE200
XTE210
	clr.b	(a0)					; Terminate Texture Name
	movem.l	(sp)+,a0/a1
	move.l	a0,a1
	move.b	(a0)+,d0				; TRANSPARENT TEXTURE?
	cmp.b	#'-',d0
	beq	XTE950					; Yes!
;
;	>>>   SCAN TEXTURE LIST   <<<
;
XTE300
	move.l	TextureList,a2
	moveq.l	#0,d2					; Texture#
;
;	>>>   CHECK NEXT TEXTURE   <<<
;
XTE400
	tst.b	(a2)					; End of TEXTURELIST?
	beq	XTE900					; Yes!
	move.l	a1,a0					; A0 = TEXTURENAME
XTE500
	move.b	(a2)+,d0				; Does NAME Match?
	move.b	(a0)+,d1
	cmp.b	d0,d1
	beq.s	XTE500					; Yes, keep looking
	cmp.b	#',',d0					; At end of TEXTURELIST Name?
	bne.s	XTE600					; No, names don't match
	tst.b	d1					; At end of TEXTURE Name?
	bne.s	XTE600					; No, names don't match
;	move.l	d2,d0					; YES!  TEXTURE NAMES MATCH!
	bra.s	XTE700
;
XTE600
	move.b	(a2)+,d0				; Skip TEXTURENAME and DIMENSIONS
	cmp.b	#10,d0
	bne.s	XTE600
XTE610
	move.b	(a2)+,d0				; Skip FIRST PATCH/TEXTURE TRANSLATION
	cmp.b	#10,d0
	bne.s	XTE610
;
XTE620
	move.b	(a2),d0					; Skip ALL PATCHES
	cmp.b	#' ',d0
	bne.s	XTE660					; No more Patches!
XTE640
	move.b	(a2)+,d0				; Skip PATCH DIMENSIONS
	cmp.b	#10,d0
	bne.s	XTE640
	bra.s	XTE620
;
XTE660
	move.b	(a2)+,d0				; Skip WALL TRANSLATION
	cmp.b	#10,d0
	bne.s	XTE660
XTE680
	move.b	(a2)+,d0				; Skip MISCELLANEOUS TEXT
	cmp.b	#10,d0
	bne.s	XTE680
	addq.l	#1,d2					; Next TEXTURE
	bra.s	XTE400
;
;	>>>   FOUND A MATCH   <<<
;
XTE700
	move.b	(a2)+,d0				; Skip TEXTURE DIMENSIONS
	cmp.b	#10,d0
	bne.s	XTE700
	move.b	(a2),d0					; PATCHLIST OR TRANSLATION?
	cmp.b	#' ',d0
	beq.s	XTE720					; PATCHLIST
;
;	>>>   TEXTURE TRANSLATION   <<<
;
	move.l	a1,a0					; Copy TRANSLATION TEXTURE
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
	cmp.b	#',',d0					; TEXTURE X OFFSET?
	bne	XTE300					; NO, RESCAN LIST!
	move.l	a2,a0					; GET TEXTURE X OFFSET
	jsr	ParseNum
	move.w	d2,TextureXOffset
	bra	XTE300					; RESCAN LIST!
;
;	>>>   PATCH LIST   <<<
;
XTE720
	move.b	(a2),d0					; Skip PATCH
	cmp.b	#' ',d0
	bne.s	XTE760					; No more Patches!
XTE740
	move.b	(a2)+,d0				; Skip PATCH DIMENSIONS
	cmp.b	#10,d0
	bne.s	XTE740
	bra.s	XTE720
XTE760
	move.l	a2,a0					; Get WALL TRANSLATION
	jsr	ParseNum
	move.l	d2,d0					; WALL TRANSLATION
	lsl.l	#1,d0
	bra.s	XTE980
;
;	>>>   DIDN'T FIND A MATCH   <<<
;
XTE900
;	moveq.l	#1,d0					; INVALID TEXTURE?!  (USE DEFAULT)
	moveq.l	#2,d0
	bra.s	XTE980
;
;	>>>   TRANSPARENT TEXTURE   <<<
;
XTE950
;	move.l	#$8000,d0				; TRANSPARENT TEXTURE
	moveq.l	#2,d0
XTE980
	move.l	(sp)+,a2
	rts


;
;	* * * * * * *       CROSS-REFERENCE FLOOR/CEILING TEXTURE       * * * * * * *
;
;	A1 = Pointer to 8-Character Texture Name
;	A0 = Pointer to Destination Texture Name
;
;	D0 = RL Texture Number
;
XREFTexture2
	move.l	a2,-(sp)
	movem.l	a0/a1,-(sp)
	moveq.l	#8-1,d1					; 8 Characters Maximum
XRTE200
	move.b	(a1)+,d0
	beq.s	XRTE210
	jsr	ToUpper
	move.b	d0,(a0)+
	dbf	d1,XRTE200
XRTE210
	clr.b	(a0)					; Terminate Texture Name
	movem.l	(sp)+,a0/a1
	move.l	a0,a1
	move.l	(a0)+,d0				; SKY TEXTURE?
	cmp.b	#'F_SK',d0
	bne.s	XRTE300					; No
	move.w	(a0)+,d0
	cmp.w	#'Y1',d0
	beq.s	XRTE800					; Yes!
XRTE300
	move.l	FloorList,a2
	moveq.l	#0,d2					; Texture#
XRTE400
	tst.b	(a2)					; End of TEXTURELIST?
	beq.s	XRTE900					; Yes!
	move.l	a1,a0					; A0 = TEXTURENAME
XRTE500
	move.b	(a2)+,d0				; Does NAME Match?
	move.b	(a0)+,d1
	cmp.b	d0,d1
	beq.s	XRTE500					; Yes, keep looking
	cmp.b	#10,d0					; At end of FLOORLIST Name?
	bne.s	XRTE600					; No, names don't match
	tst.b	d1					; At end of TEXTURE Name?
	bne.s	XRTE620					; No, names don't match
	bra.s	XRTE700
XRTE600
	move.b	(a2)+,d0				; Skip TEXTURENAME
	cmp.b	#10,d0
	bne.s	XRTE600
XRTE620
	move.b	(a2)+,d0				; Skip FILENAME
	cmp.b	#10,d0
	bne.s	XRTE620
XRTE640
	move.b	(a2)+,d0				; Skip TEXTURE SOLID COLOUR
	cmp.b	#10,d0
	bne.s	XRTE640
XRTE660
	move.b	(a2)+,d0				; Skip MISCELLANEOUS TEXT
	cmp.b	#10,d0
	bne.s	XRTE660
	addq.l	#2,d2					; Next TEXTURE
	bra.s	XRTE400
XRTE700
	move.b	(a2)+,d0				; Skip FILENAME
	cmp.b	#10,d0
	bne.s	XRTE700
	move.l	d2,-(sp)				; Save TEXTURE2
	move.l	a2,a0					; Get COLOUR
	jsr	ParseNum
	move.l	(sp)+,d0				; Restore TEXTURE2
	tst.b	NoTextures2				; TEXTURE2/COLOR?
	beq.s	XRTE950					; TEXTURE2
	move.l	d2,d0					; COLOUR
	bra.s	XRTE950
XRTE800
	move.l	#$00FF,d0				; SKY TEXTURE
	bra.s	XRTE950
XRTE900
	moveq.l	#0,d0					; INVALID TEXTURE?!
XRTE950
	move.l	(sp)+,a2
	rts


;
;	* * * * * * *       PRIORITY SORT THINGS -> THINGS2       * * * * * * *
;
SortTHINGS
	move.l	WADTHINGS2,a3			; A3 = PRIORITY SORTED THINGS2
	lea	WADTHINGSPRIORITY,a2		; A2 = PRIORITY GROUP POINTER
;
;	>>>   NEXT PRIORITY GROUP   <<<
;
STGS1200
	move.l	a2,a1				; A1 = GROUP POINTER
	moveq.l	#0,d2				; D2 = #GROUP THINGS ADDED
;
;	>>>   NEXT GROUP ELEMENT   <<<
;
STGS2200
	move.w	(a1)+,d1			; D1 = THING TYPE
	beq	STGS8800			; No More THINGS!
	bpl.s	STGS3200			; Next Item
;
;	>>>   END OF GROUP   <<<
;
	tst.l	d2				; Any GROUP THINGS ADDED?
	bne.s	STGS1200			; Yes, Process GROUP Again
	move.l	a1,a2				; A2 = NEW GROUP POINTER
	bra.s	STGS1200
;
;	>>>   ADD A SINGLE THING FROM GROUP   <<<
;
STGS3200
	move.l	WADTHINGSSize,d7		; Get Size of THINGS
	divu	#10,d7				; 10 Bytes per THING
	subq.w	#1,d7
	move.l	WADTHINGS,a0
STGS3400
	moveq.l	#0,d0				; Type
	move.b	7(a0),d0
	lsl.w	#8,d0
	move.b	6(a0),d0
	cmp.w	d0,d1
	bne.s	STGS3800			; Wrong Type
	move.b	8(a0),d0			; INVALID THING?
	and.b	#%00010111,d0
	beq.s	STGS3800			; YES!
	move.l	0(a0),(a3)+			; ADD THING!
	move.l	4(a0),(a3)+
	move.w	8(a0),(a3)+
	clr.w	6(a0)				; REMOVE THING
	addq.l	#1,d2				; ONE MORE GROUP THING ADDED!
	bra.s	STGS2200
STGS3800
	add.w	#10,a0				; Next THING
	dbf	d7,STGS3400
	bra.s	STGS2200			; Next GROUP ELEMENT
;
;	>>>   FINISHED SORTING THINGS   <<<
;
STGS8800
	move.l	WADTHINGS2,a0			; Determine Size of THINGS2
	sub.l	a0,a3
	move.l	a3,WADTHINGS2Size
	rts


;
;	* * * * * * *       TABLE OF WAD TYPE#s IN PRIORITY ORDER       * * * * * * *
;
WADTHINGSPRIORITY
;
;	PLAYERS/DEATHMATCH STARTS/TELEPORTS 0s
;
	dc.w	1			; Player1Start
	dc.w	2			; Player2Start
	dc.w	3			; Player3Start
	dc.w	4			; Player4Start
	dc.w	11			; DeathMatchStart
	dc.w	14			; TeleportSpot
	dc.w	-1			; GROUPEND
;
;	ENEMIES	10s
;
	dc.w	3004			; Soldier
	dc.w	9			; Sergeant
	dc.w	3001			; Trooper
	dc.w	3002			; Demon
	dc.w	58			; Spectre
	dc.w	3005			; CacoDemon
	dc.w	3006			; LostSoul
	dc.w	3003			; BaronOfHell
	dc.w	16			; CyberDemon
	dc.w	7			; SpiderDemon
	dc.w	-1			; GROUPEND
;
;	WEAPONS	20s
;
	dc.w	2001			; ShotGun
	dc.w	2005			; ChainSaw
	dc.w	2002			; ChainGun
	dc.w	2003			; RocketLauncher
	dc.w	2004			; PlasmaGun
	dc.w	2006			; BFG9000
	dc.w	-1			; GROUPEND
;
;	KEY CARDS	30s
;
	dc.w	13			; KeyCardRed
	dc.w	5			; KeyCardBlue
	dc.w	6			; KeyCardYellow
	dc.w	38			; RedSkullKey
	dc.w	39			; YellowSkullKey
	dc.w	40			; BlueSkullKey
	dc.w	-1			; GROUPEND
;
;	POWERUPS	60s
;
	dc.w	2013			; SoulSphere
	dc.w	2022			; Invulnerability
	dc.w	2023			; BerserkStrength
	dc.w	2024			; Invisibility
	dc.w	2025			; RadiationSuit
	dc.w	2026			; ComputerMap
	dc.w	2045			; LiteGoggles
	dc.w	-1			; GROUPEND
;
;	HEALTH/ARMOR PICKUPS	50s
;	WEAPON AMMO	40s
;
	dc.w	2011			; Stimpak
	dc.w	2012			; Medikit
	dc.w	2014			; HealthBonus
	dc.w	2015			; ArmorBonus
	dc.w	2018			; ArmorGreen
	dc.w	2019			; ArmorBlue
;
	dc.w	8			; BackPack
	dc.w	2007			; AmmoClip
	dc.w	2048			; BoxAmmo
	dc.w	2008			; ShellsFour
	dc.w	2049			; BoxShells
	dc.w	2010			; Rocket
	dc.w	2046			; BoxRockets
	dc.w	2047			; CellCharge
	dc.w	17			; CellChargePack
	dc.w	-1			; GROUPEND
;
;	MISC.	70s
;
	dc.w	2028			; FloorLamp
	dc.w	34			; Candle
	dc.w	35			; Candelabra
	dc.w	48			; ColumnTechTall
	dc.w	43			; TreeGray
	dc.w	54			; TreeBrown
	dc.w	47			; ShrubBrown
	dc.w	41			; EyeInSymbol
	dc.w	42			; FlamingSkullRock
	dc.w	55			; FireStickShortBlue
	dc.w	56			; FireStickShortGreen
	dc.w	57			; FireStickShortRed
	dc.w	44			; FireStickTallBlue
	dc.w	45			; FireStickTallGreen
	dc.w	46			; FireStickTallRed
	dc.w	-1			; GROUPEND
;
;	MISC.2	80s
;
	dc.w	2035			; Barrel
	dc.w	10			; BloodyMess
	dc.w	12			; ?BloodyMess
	dc.w	15			; DeadPlayer
	dc.w	18			; DeadSoldier
	dc.w	19			; DeadSergeant
	dc.w	20			; DeadTrooper
	dc.w	21			; DeadDemon
	dc.w	22			; DeadCacoDemon
	dc.w	23			; DeadLostSoul
	dc.w	24			; PoolOfBlood
	dc.w	25			; ImpaledHuman
	dc.w	26			; TwitchingImpaledHuman
	dc.w	27			; SkullOnPole
	dc.w	28			; SkullsOnPole
	dc.w	29			; PileSkullsCandles
	dc.w	30			; PillarTallGreen
	dc.w	31			; PillarShortGreen
	dc.w	32			; PillarTallRed
	dc.w	33			; PillarShortRed
	dc.w	36			; PillarShortHeartGreen
	dc.w	37			; PillarShortSkullRed
	dc.w	49			; HangingVictimSwaying
	dc.w	63			; ?HangingVictimSwaying
	dc.w	50			; HangingVictimArmsOut
	dc.w	59			; ?HangingVictimArmsOut
	dc.w	51			; HangingVictimOneLeg
	dc.w	61			; ?HangingVictimOneLeg
	dc.w	52			; HangingVictimUpsideDown
	dc.w	60			; ?HangingVictimUpsideDown
	dc.w	53			; HangingSeveredLeg
	dc.w	62			; ?HangingSeveredLeg
	dc.w	-1			; GROUPEND
;
	dc.w	0			; *END*


;
;	* * * * * * *       CONVERT DOOM LEVEL THINGS -> RL ?       * * * * * * *
;
ConvertTHINGS
	clr.l	RLNUMOBJECTS				; NO Objects Defined Yet!
;
;	>>>   CALCULATE REDUCED RLOBJ TYPE REMAPPING TABLE   <<<
;
	lea	RLOBJReMapTable,a0			; Clear ReMapping Table
	move.w	#MaxRLObjectTypes-1,d0
CTGS200
	clr.w	(a0)+
	dbf	d0,CTGS200
;
	lea	WADtoRLOBJECTNUMS,a0			; Find Replacement Type#
	lea	RLOBJReMapTable,a1			; Pointer to ReMapping Table
	move.w	#-1,(a1)				; RLObj#0 is DISABLED!
	moveq.l	#0,d7					; Current Reduced RLOBJ Type#
CTGS400
	addq.w	#2,a0					; Skip WAD Type#
	move.w	(a0)+,d1				; Get RL Type#
	addq.w	#2,a0					; Skip RLObj Flags
	bmi.s	CTGS1000				; End of Table!
CTGS420
	tst.b	(a0)+					; Skip Name
	bne.s	CTGS420
	move.l	a0,d0					; Ensure Word Alignment
	and.l	#1,d0
	beq.s	CTGS500
	addq.w	#1,a0
CTGS500
	lsl.w	#1,d1
	tst.w	(a1,d1.w)				; ReMapped already?
	bne.s	CTGS400					; Yes
	move.w	d7,(a1,d1.w)				; No, Create NEW ReMapping!
	addq.w	#1,d7					; One More UNIQUE RLObject!
	bra.s	CTGS400

;
;	>>>   PROCESS THINGS -> RLOBJECTS   <<<
;
CTGS1000
	move.l	d7,-(sp)				; #UNIQUE TYPES
	move.l	WADTHINGS2Size,d0			; Get Size of THINGS2
	divu	#10,d0
	move.l	d0,-(sp)
	move.l	WADTHINGSSize,d0			; Get Size of THINGS
	divu	#10,d0
	move.l	d0,-(sp)
	lea	NumTHINGSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(3*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	WADTHINGS2Size,d7			; Get Size of THINGS2
	divu	#10,d7					; 10 Bytes per THING
	subq.w	#1,d7
	move.l	WADTHINGS2,a2
	move.l	RLOBJECTS,a3
	moveq.l	#0,d6					; THING Number
	moveq.l	#0,d4					; D4 = MOVABLE OBJECT COUNTER
;
;	>>>   PROCESS NEXT THING   <<<
;
CTGS1200
	sub.w	#(11*4),sp
	move.l	d6,0(sp)				; THING NUMBER
;
	moveq.l	#0,d0					; Type
	move.b	7(a2),d0
	lsl.w	#8,d0
	move.b	6(a2),d0
	ext.l	d0
	move.l	d0,32(sp)
;
	lea	WADtoRLOBJECTNUMS,a0			; Find Replacement Type#
	moveq.l	#-1,d2					; Default to DISCARD THING
CTGS1400
	move.w	(a0)+,d1				; D1 = WAD Type#
	addq.w	#4,a0
	bmi.s	CTGS1480
	cmp.w	d0,d1					; Matches?
	beq.s	CTGS1450				; Yes!
CTGS1430
	tst.b	(a0)+					; Skip Name
	bne.s	CTGS1430
	move.l	a0,d1					; Ensure Word Alignment
	and.l	#1,d1
	beq.s	CTGS1400
	addq.w	#1,a0
	bra.s	CTGS1400
CTGS1450
	move.w	-4(a0),d2				; Get Replacement Type#
	lea	RLOBJReMapTable,a1			; Get Reduced RLObj Type#
	lsl.w	#1,d2
	move.w	(a1,d2.w),d2
	move.w	-2(a0),d1				; Get Replacement Flags
CTGS1480
	move.l	a0,40(sp)				; Save Name
	ext.l	d2
	move.l	d2,28(sp)				; Save Replacement Type#
	move.l	d2,d0
	move.b	d0,1(a3)				; Type
;
	moveq.l	#0,d0					; Attributes
	move.b	8(a2),d0
	and.b	#%00010111,d0
	or.b	d1,d0					; Add RL Attribute Bits
	move.b	d0,(a3)
	move.l	d0,36(sp)
	addq.w	#2,a3
;
	moveq.l	#0,d0					; X Coordinate
	move.b	1(a2),d0
	lsl.w	#8,d0
	move.b	0(a2),d0
	ext.l	d0
	move.l	d0,4(sp)
	and.l	#$ffff,d0
	move.l	d0,16(sp)
	move.w	(a2),(a3)+				; X Coordinate

	moveq.l	#0,d0					; Y Coordinate
	move.b	3(a2),d0
	lsl.w	#8,d0
	move.b	2(a2),d0
	ext.l	d0
	move.l	d0,8(sp)
	and.l	#$ffff,d0
	move.l	d0,20(sp)
	move.w	2(a2),(a3)+				; Y Coordinate
;
	moveq.l	#0,d0					; ANGLE
	move.b	5(a2),d0
	lsl.w	#8,d0
	move.b	4(a2),d0
	mulu	#182,d0
	add.w	#$1000,d0				; Add 22.5 Degrees
	and.l	#$e000,d0				; Multiple of 45 Degrees
	move.l	d0,24(sp)
	move.l	d0,d5
	move.b	d0,(a3)+
	lsr.w	#8,d0
	move.b	d0,(a3)+
	divu	#(65536/360),d5
	and.l	#$ffff,d5
	move.l	d5,12(sp)
;
	tst.w	d2					; Replacement RLObject Found?
	bpl.s	CTGS1800				; Yes
	sub.w	#rlpSize,a3				; No, Discard THING!
	bra.s	CTGS1850
CTGS1800
	addq.l	#1,RLNUMOBJECTS				; One more Object
	move.w	d1,d0					; ANGULAR OBJECT?
	and.w	#$0100,d0
	bne.s	CTGS1820				; YES!
	subq.w	#2,a3					; NO!  DON'T SAVE ANGLE!
CTGS1820
	and.w	#$40,d1					; Movable Object?
	beq.s	CTGS1850				; No!
	addq.w	#1,d4					; #MOBJ++
CTGS1850
	move.l	d7,d5
	lea	THINGDataMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(11*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	d5,d7
;
	addq.w	#1,d6					; Next THING
	add.w	#10,a2
	dbf	d7,CTGS1200
	move.l	RLOBJECTS,a0				; Determine Size of OBJECTS
	sub.l	a0,a3
	move.l	a3,RLOBJECTSSize
;
	move.l	d4,-(sp)				; #MOVABLE OBJECTS
	move.l	RLNUMOBJECTS,d5				; #LEVEL OBJECTS
	move.l	d5,d0
	sub.l	d4,d0
	move.l	d0,-(sp)				; #FIXED OBJECTS
	move.l	d5,-(sp)
	lea	NumOBJECTSMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(3*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	moveq.l	#0,d7					; D7 = ErrorCode
	cmp.w	#MaxRLMObjects,d4			; Too many Movable Objects?
	ble.s	CTGS1900
	move.l	#MSGMaxMObjError,d7			; YES!  ERROR!
CTGS1900
	sub.w	d4,d5					; D5 = #Fixed Objects
	cmp.w	#MaxRLFObjects,d5			; Too many Fixed Objects?
	blt.s	CTGS1920
	move.l	#MSGMaxFObjError,d7			; YES!  ERROR!
CTGS1920
	tst.l	d7
	rts

;
;	* * * * * * *       TABLE OF WAD TYPE#s TO RL TYPE#s       * * * * * * *
;
WADtoRLOBJECTNUMS
;
;	PLAYERS/DEATHMATCH STARTS/TELEPORTS 0s
;
	dc.w	1,1,$01e8
	dc.b	"Player1Start",0
	dc.w	2,2,$01f8
	dc.b	"Player2Start",0
	dc.w	3,3,$01f8
	dc.b	"Player3Start",0
	dc.w	4,4,$01f8
	dc.b	"Player4Start",0
	dc.w	11,5,$01f8
	dc.b	"DeathMatchStart",0
	dc.w	14,7,$0100
	dc.b	"TeleportSpot",0
;
;	ENEMIES	10s
;
	dc.w	3004,10,$00e8
	dc.b	"Soldier",0
	dc.w	9,11,$00e8
	dc.b	"Sergeant",0
	dc.w	3001,12,$00e8
	dc.b	"Trooper",0
	dc.w	3002,13,$00e8
	dc.b	"Demon",0
	dc.w	58,13,$00e8
	dc.b	"Spectre",0
	dc.w	3005,14,$00e8
	dc.b	"CacoDemon",0
	dc.w	3006,15,$00e8
	dc.b	"LostSoul",0
	dc.w	3003,16,$00e8
	dc.b	"BaronOfHell",0
	dc.w	16,17,$00e8
	dc.b	"CyberDemon",0
	dc.w	7,18,$00e8
	dc.b	"SpiderDemon",0
;
;	WEAPONS	20s
;
	dc.w	2001,20,$0080
	dc.b	"ShotGun",0
	dc.w	2005,21,$0080
	dc.b	"ChainSaw",0
	dc.w	2002,22,$0080
	dc.b	"ChainGun",0
	dc.w	2003,23,$0080
	dc.b	"RocketLauncher",0
	dc.w	2004,24,$0080
	dc.b	"PlasmaGun",0
	dc.w	2006,25,$0080
	dc.b	"BFG9000",0
;
;	KEY CARDS	30s
;
	dc.w	13,30,$0080
	dc.b	"KeyCardRed",0
	dc.w	5,31,$0080
	dc.b	"KeyCardBlue",0
	dc.w	6,32,$0080
	dc.b	"KeyCardYellow",0
	dc.w	38,33,$0080
	dc.b	"RedSkullKey",0
	dc.w	39,34,$0080
	dc.b	"YellowSkullKey",0
	dc.w	40,35,$0080
	dc.b	"BlueSkullKey",0
;
;	WEAPON AMMO	40s
;
	dc.w	8,40,$0080
	dc.b	"BackPack",0
	dc.w	2007,41,$0080
	dc.b	"AmmoClip",0
	dc.w	2048,42,$0080
	dc.b	"BoxAmmo",0
	dc.w	2008,43,$0080
	dc.b	"ShellsFour",0
	dc.w	2049,44,$0080
	dc.b	"BoxShells",0
	dc.w	2010,45,$0080
	dc.b	"Rocket",0
	dc.w	2046,46,$0080
	dc.b	"BoxRockets",0
	dc.w	2047,47,$0080
	dc.b	"CellCharge",0
	dc.w	17,48,$0080
	dc.b	"CellChargePack",0
;
;	HEALTH/ARMOR PICKUPS	50s
;
	dc.w	2011,50,$0080
	dc.b	"Stimpak",0
	dc.w	2012,51,$0080
	dc.b	"Medikit",0
	dc.w	2014,52,$0080
	dc.b	"HealthBonus",0
	dc.w	2015,53,$0080
	dc.b	"ArmorBonus",0
	dc.w	2018,54,$0080
	dc.b	"ArmorGreen",0
	dc.w	2019,55,$0080
	dc.b	"ArmorBlue",0
;
;	POWERUPS	60s
;
	dc.w	2013,60,$0080
	dc.b	"SoulSphere",0
	dc.w	2022,61,$0080
	dc.b	"Invulnerability",0
	dc.w	2023,62,$0080
	dc.b	"BerserkStrength",0
	dc.w	2024,63,$0080
	dc.b	"Invisibility",0
	dc.w	2025,64,$0080
	dc.b	"RadiationSuit",0
	dc.w	2026,65,$0080
	dc.b	"ComputerMap",0
	dc.w	2045,66,$0080
	dc.b	"LiteGoggles",0
;
;	MISC.	70s
;
	dc.w	2035,70,$00a8
	dc.b	"Barrel",0
	dc.w	2028,71,$0088
	dc.b	"FloorLamp",0
	dc.w	10,72,$0080
	dc.b	"BloodyMess",0
	dc.w	12,72,$0080
	dc.b	"?BloodyMess",0		; SAME AS PREVIOUS?
	dc.w	15,72,$0080
	dc.b	"DeadPlayer",0
	dc.w	18,72,$0080
	dc.b	"DeadSoldier",0
	dc.w	19,72,$0080
	dc.b	"DeadSergeant",0
	dc.w	20,73,$0080
	dc.b	"DeadTrooper",0
	dc.w	21,74,$0080
	dc.b	"DeadDemon",0
	dc.w	22,0,0
	dc.b	"DeadCacoDemon",0
	dc.w	23,0,0
	dc.b	"DeadLostSoul",0
	dc.w	24,0,0
	dc.b	"PoolOfBlood",0
	dc.w	25,0,0
	dc.b	"ImpaledHuman",0
	dc.w	26,0,0
	dc.b	"TwitchingImpaledHuman",0
	dc.w	27,75,$0088
	dc.b	"SkullOnPole",0
	dc.w	28,75,$0088
	dc.b	"SkullsOnPole",0
	dc.w	29,0,0
	dc.b	"PileSkullsCandles",0
	dc.w	30,76,$0088
	dc.b	"PillarTallGreen",0
	dc.w	31,76,$0088
	dc.b	"PillarShortGreen",0
	dc.w	32,76,$0088
	dc.b	"PillarTallRed",0
	dc.w	33,76,$0088
	dc.b	"PillarShortRed",0
	dc.w	34,77,$0080
	dc.b	"Candle",0
	dc.w	35,78,$0088
	dc.b	"Candelabra",0
	dc.w	36,76,$0088
	dc.b	"PillarShortHeartGreen",0
	dc.w	37,76,$0088
	dc.b	"PillarShortSkullRed",0
	dc.w	41,0,0
	dc.b	"EyeInSymbol",0
	dc.w	42,79,$0088
	dc.b	"FlamingSkullRock",0
	dc.w	43,80,$0088
	dc.b	"TreeGray",0
	dc.w	44,81,$0088
	dc.b	"FireStickTallBlue",0
	dc.w	45,81,$0088
	dc.b	"FireStickTallGreen",0
	dc.w	46,81,$0088
	dc.b	"FireStickTallRed",0
	dc.w	47,82,$0088
	dc.b	"ShrubBrown",0
	dc.w	48,83,$0088
	dc.b	"ColumnTechTall",0
	dc.w	49,0,0
	dc.b	"HangingVictimSwaying",0
	dc.w	63,0,0
	dc.b	"?HangingVictimSwaying",0	; SAME AS PREVIOUS?
	dc.w	50,0,0
	dc.b	"HangingVictimArmsOut",0
	dc.w	59,0,0
	dc.b	"?HangingVictimArmsOut",0	; SAME AS PREVIOUS?
	dc.w	51,0,0
	dc.b	"HangingVictimOneLeg",0
	dc.w	61,0,0
	dc.b	"?HangingVictimOneLeg",0	; SAME AS PREVIOUS?
	dc.w	52,0,0
	dc.b	"HangingVictimUpsideDown",0
	dc.w	60,0,0
	dc.b	"?HangingVictimUpsideDown",0	; SAME AS PREVIOUS?
	dc.w	53,0,0
	dc.b	"HangingSeveredLeg",0
	dc.w	62,0,0
	dc.b	"?HangingSeveredLeg",0		; SAME AS PREVIOUS?
	dc.w	54,80,$0088
	dc.b	"TreeBrown",0
	dc.w	55,84,$0088
	dc.b	"FireStickShortBlue",0
	dc.w	56,84,$0088
	dc.b	"FireStickShortGreen",0
	dc.w	57,84,$0088
	dc.b	"FireStickShortRed",0

	dc.w	-1,-1,0
	dc.b	"?!",0				; End of Table

	dc.w	0


;
;	* * * * * * *       OPTIMIZE THE LEVEL       * * * * * * *
;
OptimizeLEVEL
	bsr	OptimizeFACES				; Optimize FACES
	rts


;
;	* * * * * * *       OPTIMIZE FACES       * * * * * * *
;
;	REMOVE ALL TRANSPARENT FACES
;
OptimizeFACES
	lea	OptFACESMsg(pc),a0			; Optimizing FACES!
	jsr	VDTDebugOutC
	moveq.l	#-1,d7
	jsr	PrintMsg
;
;	>>>   INITIALIZE FACE SCAN   <<<
;
	sub.w	#(6*4),sp
	move.l	RLFACESSize,d7				; Get Size of FACES
	divu	#rlfSizeT,d7				; Bytes per FACE
	move.l	d7,4(sp)
	clr.l	0(sp)					; #FACES REMOVED
	clr.l	8(sp)					; #SEGS REMOVED
	clr.l	16(sp)					; #AREAS REMOVED
	move.l	RLSEGSSize,d0				; Get Size of SEGS
	divu	#rlgSize,d0				; Bytes per SEG
	move.l	d0,12(sp)
	move.l	RLAREASSize,d0				; Get Size of AREAS
	divu	#rlaSize,d0				; Bytes per AREA
	move.l	d0,20(sp)
	subq.w	#1,d7
	moveq.l	#0,d6					; FACE#
	move.l	RLFACES,a4
;
;	>>>   SCAN FOR ALL TRANSPARENT FACES   <<<
;
OFS1200
	move.b	5(a4),d0				; ALL Textures TRANSPARENT?
	and.b	7(a4),d0
	and.b	9(a4),d0
	bpl	OFS8000					; No!
;
;	>>>   DELETE THIS FACE!   <<<
;
	move.l	0(sp),d0				; Get #DELETED FACES
	add.l	d6,d0
	move.l	d7,-(sp)
	move.l	d0,-(sp)
	lea	OptFACESDataMsg(pc),a0			; DELETED FACE Message
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#4,sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	(sp)+,d7
;
;	>>>   SCAN SEGS FOR REFERENCE TO DELETED FACE   <<<
;
	move.l	RLSEGSSize,d2				; Get Size of SEGS
	divu	#rlgSize,d2				; Bytes per SEG
	subq.w	#1,d2
	move.l	RLSEGS,a2
	moveq.l	#0,d5					; D5 = SEG#
OFS2100
	moveq.l	#0,d0
	move.b	11(a2),d0				; D0 = FACE OFFSET
	lsl.w	#8,d0
	move.b	10(a2),d0
	divu	#rlfSizeT,d0
	and.l	#$ffff,d0				; D0 = FACE#
	cmp.w	d0,d6					; REFERENCING DELETED FACE?
	bne	OFS3800					; No
;
;	>>>   FOUND A SEGMENT THAT REFERENCES THE DELETED FACE!   <<<
;
	move.l	d7,-(sp)
	move.l	d5,-(sp)
	lea	OptFACESData2Msg(pc),a0			; DELETED SEG Message
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#4,sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	(sp)+,d7
;
	move.l	a2,a0					; A0 = Start of Current SEG
	move.l	RLSEGS,a1				; A1 = End of SEGS
	add.l	RLSEGSSize,a1
OFS2400
	move.b	rlgSize(a0),(a0)+			; Copy SEGS Down
	cmp.l	a0,a1
	bne.s	OFS2400
	sub.l	#rlgSize,RLSEGSSize			; Adjust Size of SEGS
	add.l	#1,8(sp)				; ONE MORE SEG REMOVED!
;
;	>>>   SCAN AREAS FOR REFERENCES TO DELETED SEG   <<<
;
OFS3000
	move.l	RLAREASSize,d3				; Get Size of AREAS
	divu	#rlaSize,d3				; Bytes per AREA
	subq.w	#1,d3
	move.l	RLAREAS,a3
	moveq.l	#0,d4					; D4 = AREA#
OFS3100
	tst.b	(a3)					; ALREADY EMPTY?
	beq	OFS3700					; Yes!
	moveq.l	#0,d0
	move.b	2(a3),d0				; D0 = STARTING SEGMENT
	lsl.w	#8,d0
	move.b	1(a3),d0
	divu	#rlgSize,d0
	and.l	#$ffff,d0
	cmp.w	d0,d5					; DELETED SEGMENT >= STARTING SEGMENT
	bge	OFS3500					; Yes!
;
;	>>>   AREA'S SEGMENTS AFTER DELETED SEGMENT   <<<
;
	subq.w	#1,d0					; Adjust STARTING SEGMENT Back 1
	mulu	#rlgSize,d0
	move.b	d0,1(a3)
	lsr.w	#8,d0
	move.b	d0,2(a3)
	bra	OFS3700
;
;	>>>   AREA'S SEGMENTS WITHIN RANGE OF DELETED SEGMENT?   <<<
;
OFS3500
	moveq.l	#0,d1					; D1 = #SEGMENTS REFERENCED
	move.b	0(a3),d1
	add.l	d0,d1					; D1 = ENDING SEGMENT
	subq.l	#1,d1
	cmp.l	d1,d5					; DELETED SEGMENT > ENDING SEGMENT?
	bgt	OFS3700					; Yes, AREA Not Within Range
;
;	>>>   AREA'S SEGMENTS WITHIN RANGE   <<<
;
	subq.b	#1,0(a3)				; One Less SEGMENT In This AREA!
	bne.s	OFS3700
;
;	>>>   AREA HAS NO REMAINING SEGMENTS!   <<<
;
	add.l	#1,16(sp)				; ONE MORE AREA REMOVED!
	move.l	d7,-(sp)
	move.l	d4,-(sp)
	lea	OptFACESData3Msg(pc),a0			; EMPTY AREA Message!
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#4,sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	(sp)+,d7
;
;	>>>   SET FIRST SEGMENT IN EMPTY AREA TO <SECTOR NEAR>   <<<
;
	move.w	(a4),1(a3)				; SEGMENT = SECTOR NEAR
;
;	>>>   NEXT AREA   <<<
;
OFS3700
	add.w	#rlaSize,a3				; Next AREA
	addq.l	#1,d4
	dbf	d3,OFS3100
	bra	OFS3950
;
;	>>>   FACE AFTER DELETED FACE?   <<<
;
OFS3800
	bgt	OFS3900					; DELETED FACE > FACE?
	subq.w	#1,d0					; Yes, Adjust FACE Reference!
	mulu	#rlfSizeT,d0
	move.b	d0,10(a2)
	lsr.w	#8,d0
	move.b	d0,11(a2)
;
;	>>>   NEXT SEGMENT   <<<
;
OFS3900
	add.w	#rlgSize,a2				; Next SEG
	addq.l	#1,d5
OFS3950
	dbf	d2,OFS2100
;
;	>>>   REMOVE THIS FACE FROM MEMORY   <<<
;
	move.l	a4,a0					; A0 = Start of Current Face
	move.l	RLFACES,a1				; A1 = End of Faces
	add.l	RLFACESSize,a1
OFS4200
	move.b	rlfSizeT(a0),(a0)+			; Copy FACES Down
	cmp.l	a0,a1
	bne.s	OFS4200
	sub.l	#rlfSizeT,RLFACESSize			; Adjust Size of FACES
	add.l	#1,0(sp)				; ONE MORE FACE REMOVED!
	bra.s	OFS8200					; Don't need to Move Forwards!
;
;	>>>   MOVE TO NEXT FACE   <<<
;
OFS8000
	add.w	#rlfSizeT,a4				; Move to Next FACE
	addq.l	#1,d6
OFS8200
	dbf	d7,OFS1200
;
	lea	OptFACESDoneMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(6*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	rts


;
;	* * * * * * *       TEXT MESSAGES       * * * * * * *
;
ConvertFileErrorMsg
	dc.b	'Error with File %s!',10,0

EngineStatsMsg
	dc.b	'RLENGINE STATS: %ld VERTEXES, %ld LINEDEFS, %ld SECTORS',10,0

NumVERTEXESMsg
	dc.b	'%ld VERTEXES',10,0

NumLINEDEFSMsg
	dc.b	'%ld LINEDEFS',10,0
LINEDEFDataMsg
	dc.b	'LINE %4ld  [%4ld,%4ld] (%5ld,%5ld)->(%5ld,%5ld)'
	dc.b	'  A=$%04lx/$%04lx  RF=$%04lx  T=%3ld  G=$%04lx',10,0

NumSECTORSMsg
	dc.b	12,'%ld SECTORS',10,0
SECTORDataMsg
	dc.b	'SECTOR %3ld  F=%5ld  C=%5ld  <%8s> <%8s>'
	dc.b	'  <%04lx> <%04lx>  B=%02lx,%02lx  T=%2ld  G=$%04lx',10,0

NumSSECTORSMsg
	dc.b	'%ld SSECTORS',10,0

NumSEGSMsg
	dc.b	12,'%ld SEGS',10,0
SEGDataMsg
	dc.b	'SEG %4ld%lc [%4ld,%4ld] (%5ld,%5ld)->(%5ld,%5ld)'
	dc.b	'  L=%4ld  A=$%04lx  OX=%02lx,OY=%02lx  F=%4ld  RF=$%04lx',10,0

NumSIDEDEFSMsg
	dc.b	12,'%ld SIDEDEFS',10,0
SIDEDEFDataMsg
	dc.b	'SIDE %4ld  (%4ld,%4ld)  <%8s> <%8s> <%8s>'
	dc.b	'  <%04lx> <%04lx> <%04lx>  [%3ld][%3ld]',10,0

NumNODESMsg
	dc.b	12,'%ld NODES',10,0
NODEDataMsg
	dc.b	'NODE %4ld  (%5ld,%5ld) (%5ld,%5ld)'
	dc.b	'   Left %s %3ld,  Right %s %3ld',10,0
NODEDataAREAMsg
	dc.b	'AREA',0
NODEDataNODEMsg
	dc.b	'NODE',0

NumTHINGSMsg
	dc.b	'%ld THINGS, %ld THINGS2  (%ld TYPES)',10,0
THINGDataMsg
	dc.b	'THING %3ld  (%5ld,%5ld  %3ld)  ($%04lx, $%04lx  $%04lx)'
	dc.b	'  [%3ld] (%4ld)  F=$%02lx  %s',10,0

NumOBJECTSMsg
	dc.b	'%ld OBJECTS (%ld FIXED, %ld MOVABLE)',10,0

NumBLOCKMAPMsg
	dc.b	'BLOCKMAP ($%04lx,$%04lx)->($%04lx,$%04lx)  %ld Blocks',10,0

NumDOORSMsg
	dc.b	'%ld DOORS',10,0

NumSTAIRSMsg
	dc.b	'%ld STAIRS',10,0

NumFLOORSMsg
	dc.b	'%ld FLOORS',10,0

NumLIFTSMsg
	dc.b	'%ld LIFTS',10,0

NumCEILINGSMsg
	dc.b	'%ld CEILINGS',10,0

OptFACESMsg
	dc.b	12,'OPTIMIZING FACES...',10,0
OptFACESDataMsg
	dc.b	'REMOVED FACE %4ld',10,0
OptFACESData2Msg
	dc.b	' REMOVED SEG  %4ld',10,0
OptFACESData3Msg
	dc.b	'  REMOVED AREA %4ld',10,0
OptFACESDoneMsg
	dc.b	'%ld FACES Removed (of %ld), %ld SEGS Removed (of %ld),'
	dc.b	' %ld AREAS Removed (of %ld)',10,0

TextureSKYMsg
	dc.b	'SKY',0


;
;	* * * * * * *       LIST OF DIRECTORY NAME SUFFIXES       * * * * * * *
;
WADVERTEXESSuffix
	dc.b	'/VERTEXES',0
WADNODESSuffix
	dc.b	'/NODES',0
WADSEGSSuffix
	dc.b	'/SEGS',0
WADSSECTORSSuffix
	dc.b	'/SSECTORS',0
WADSECTORSSuffix
	dc.b	'/SECTORS',0
WADLINEDEFSSuffix
	dc.b	'/LINEDEFS',0
WADSIDEDEFSSuffix
	dc.b	'/SIDEDEFS',0
WADREJECTSuffix
	dc.b	'/REJECT',0
WADBLOCKMAPSuffix
	dc.b	'/BLOCKMAP',0
WADTHINGSSuffix
	dc.b	'/THINGS',0

RLVERTEXESSuffix
	dc.b	'/VERTEXES',0
RLLINESSuffix
	dc.b	'/LINES',0
RLSECTORSSuffix
	dc.b	'/SECTORS',0
RLBSPSuffix
	dc.b	'/BSP',0
RLAREASSuffix
	dc.b	'/AREAS',0
RLSEGSSuffix
	dc.b	'/SEGS',0
RLFACESSuffix
	dc.b	'/FACES',0
RLIFFSuffix
	dc.b	'/IFF',0
RLOBJECTSSuffix
	dc.b	'/OBJECTS',0
RLBLOCKMAPSuffix
	dc.b	'/BLOCKMAP',0
RLREJECTSuffix
	dc.b	'/REJECT',0
RLDOORSSuffix
	dc.b	'/DOORS',0
RLSTAIRSSuffix
	dc.b	'/STAIRS',0
RLFLOORSSuffix
	dc.b	'/FLOORS',0
RLLIFTSSuffix
	dc.b	'/LIFTS',0
RLCEILINGSSuffix
	dc.b	'/CEILINGS',0

	dc.w	0


	end
