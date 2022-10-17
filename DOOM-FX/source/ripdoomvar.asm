;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                           VARIABLES MODULE                              *
;*                                                                         *
;***************************************************************************

	include	ripdoom.i


	section	__MERGED,DATA

	xdef	DosName,DosBase
	xdef	IntuitionName,IntuitionBase
	xdef	GraphicsName,GraphicsBase
	xdef	MathIEEEDoubBasName,MathIEEEDoubBasBase
	xdef	MathIEEEDoubTransName,MathIEEEDoubTransBase

	xdef	IFFMode,IFFMask,IFFComp,IFFCompLen,IFFCompData

	xdef	Task,OutputFIB,MsgBuffer
	xdef	argc,argv
	xdef	SystemMemory
	xdef	Quiet,Verbose

	xdef	DoomWADFIB
	xdef	DoomPaletteName,DoomReMapPaletteName,DoomColourMapName
	xdef	RLFloorListName,RLPatchListName,RLTextureListName
	xdef	DoomPatchListName,DoomTexture1Name,DoomTexture2Name

	xdef	DoomWADData,DoomWADDir
	xdef	NumDirEntries
	xdef	DoomEntryName,DoomPrefixName,DoomFileName
	xdef	DoomPalette,RLPalette,DoomReMapPalette
	xdef	DoomReMapTable,DoomUnMapTable
	xdef	DoomColourMap,RLColourMap
	xdef	DoomPatchList,DoomTexture1,DoomTexture2

	xdef	RipData,ConvertWAD,ConvertFloor,ConvertImage,ConvertLevel
	xdef	OutputName,DoTextures
	xdef	ConvertPlayPal,ConvertColourMap,ConvertColourReMap,ConvertRGBReMap
	xdef	PicDim,PicDimWidth,PicDimHeight,PicDimPlanes,PicPlanesOffset

	xdef	FloorList,PatchList,TextureList

	xdef	Texture1Name,Texture2Name,Texture3Name,TextureXOffset,TextureYOffset

	xdef	ImageBGReMap,ImageReMap,RoundRLPalette,GammaRLPalette
	xdef	NoTextures2

	xdef	WADVERTEXES,WADVERTEXESSize
	xdef	WADLINEDEFS,WADLINEDEFSSize
	xdef	WADSIDEDEFS,WADSIDEDEFSSize
	xdef	WADNODES,WADNODESSize
	xdef	WADSEGS,WADSEGSSize
	xdef	WADSSECTORS,WADSSECTORSSize
	xdef	WADSECTORS,WADSECTORSSize
	xdef	WADREJECT,WADREJECTSize
	xdef	WADBLOCKMAP,WADBLOCKMAPSize
	xdef	WADTHINGS,WADTHINGS2,WADTHINGSSize,WADTHINGS2Size
;
	xdef	RLVERTEXES,RLVERTEXESSize
	xdef	RLLINES,RLLINESSize
	xdef	RLSECTORS,RLSECTORSSize,RLSECTOROrigins
	xdef	RLBSP,RLBSPSize
	xdef	RLAREAS,RLAREASSize
	xdef	RLSEGS,RLSEGSSize
	xdef	RLFACES,RLFACESSize,RLFACESPtrs,RLFACESFlags
	xdef	RLOBJECTS,RLOBJECTSSize,RLNUMOBJECTS
	xdef	RLBLOCKMAP,RLBLOCKMAPSize
	xdef	RLREJECT,RLREJECTSize
	xdef	RLDOORS,RLDOORSSize,RLNUMDOORS
	xdef	RLSTAIRS,RLSTAIRSSize,RLNUMSTAIRS
	xdef	RLFLOORS,RLFLOORSSize,RLNUMFLOORS
	xdef	RLLIFTS,RLLIFTSSize,RLNUMLIFTS
	xdef	RLCEILINGS,RLCEILINGSSize,RLNUMCEILINGS
;
	xdef	RLOBJReMapTable


DosName			dc.b	'dos.library',0
GraphicsName		dc.b	'graphics.library',0
IntuitionName		dc.b	'intuition.library',0
MathIEEEDoubBasName	dc.b	'mathieeedoubbas.library',0
MathIEEEDoubTransName	dc.b	'mathieeedoubtrans.library',0
;
DoomPaletteName		dc.b	'DOOMDATA:PALETTES/PLAYPAL',0
DoomColourMapName	dc.b	'DOOMDATA:PALETTES/COLORMAP',0
DoomPatchListName	dc.b	'DOOMDATA:MISC/PNAMES',0
DoomTexture1Name	dc.b	'DOOMDATA:MISC/TEXTURE1',0
DoomTexture2Name	dc.b	'DOOMDATA:MISC/TEXTURE2',0
RLFloorListName		dc.b	'FLOORLIST',0
RLPatchListName		dc.b	'PATCHLIST',0
RLTextureListName	dc.b	'TEXTURELIST',0
			dc.w	0

DosBase			dc.l	0		; dos.library
GraphicsBase		dc.l	0		; graphics.library
IntuitionBase		dc.l	0		; intuition.library
MathIEEEDoubBasBase	dc.l	0		; mathieeedoubbas.library
MathIEEEDoubTransBase	dc.l	0		; mathieeedoubtrans.library

IFFMode			dc.b	0		; 0=ILBM,1=PBM
IFFMask			dc.b	0		; 0=No Mask/Stencil,1=Stencil/Mask ON
IFFComp			dc.b	0		; Compression Format
IFFCompLen		dc.b	0		; Compression Length
IFFCompData		dc.b	0		; Compression DataByte
			dc.b	0

Task			dc.l	0		; Address of Amiga_Task_Structure
OutputFIB		dc.l	0		; Output FIB
argc			dc.l	0
argv			ds.l	256
SystemMemory		dc.l	0		; Pointer to System Memory Block
MsgBuffer		ds.b	256		; Message Text Buffer
Quiet			dc.b	0
Verbose			dc.b	0

DoomWADFIB		dc.l	0		; FIB for DOOM.WAD File
DoomWADData		dc.l	0		; Pointer to DOOM.WAD DataBlock
DoomWADDir		dc.l	0		; Pointer to DOOM.WAD Directory
DoomPalette		dc.l	0		; Pointer to DOOM.WAD Palette
RLPalette		dc.l	0		; Pointer to RL ReMapped Palette
DoomColourMap		dc.l	0		; Pointer to DOOM.WAD ColourMap
RLColourMap		dc.l	0		; Pointer to RL ReMapped ColourMap
DoomPatchList		dc.l	0		; Pointer to DOOM.WAD PNAMES
DoomTexture1		dc.l	0		; Pointer to DOOM.WAD TEXTURE1
DoomTexture2		dc.l	0		; Pointer to DOOM.WAD TEXTURE2
NumDirEntries		dc.l	0		; Number of directory entries

DoomEntryName		ds.b	16		; Doom Entry Name
DoomPrefixName		ds.b	64		; Doom FileName Prefix
DoomFileName		ds.b	256		; Doom FileName


RipData			dc.b	0		; -1 = RipData
DoTextures		dc.b	0		; -1 = Convert Textures/PatchList
ConvertWAD		dc.l	0		; APTR Name of WAD to Convert
ConvertFloor		dc.l	0		; APTR Name of Floor to Convert
ConvertImage		dc.l	0		; APTR Name of Image to Convert
ConvertLevel		dc.l	0		; APTR Name of Level to Convert
ConvertPlayPal		dc.l	0		; APTR Name of PlayPal to Convert to
ConvertColourMap	dc.l	0		; APTR Name of ColourMap to Convert to
ConvertColourReMap	dc.l	0		; APTR Name of ColourReMap Table to SAVE
ConvertRGBReMap		dc.l	0		; APTR Name of RGBReMap Table to SAVE
OutputName		dc.l	0		; APTR Name of Output File
DoomReMapPaletteName	dc.l	0		; APTR Name of ReMap Palette Image

PicDim			dc.l	0		; APTR Dimensions of LEVEL IFF File
PicDimWidth		dc.w	0		; Width of LEVEL IFF File
PicDimHeight		dc.w	0		; Height of LEVEL IFF File
PicDimPlanes		dc.w	0		; #Planes of LEVEL IFF File
PicPlanesOffset		dc.l	0		; Offset to add to PICPLANES

FloorList		dc.l	0		; APTR RL FLOOR LIST
PatchList		dc.l	0		; APTR RL PATCH LIST
TextureList		dc.l	0		; APTR RL TEXTURE LIST

WADVERTEXES		dc.l	0		; APTR WAD VERTEXES
WADVERTEXESSize		dc.l	0		; Size of WAD VERTEXES
WADLINEDEFS		dc.l	0		; APTR WAD LINEDEFS
WADLINEDEFSSize		dc.l	0		; Size of WAD LINEDEFS
WADSIDEDEFS		dc.l	0		; APTR WAD SIDEDEFS
WADSIDEDEFSSize		dc.l	0		; Size of WAD SIDEDEFS
WADNODES		dc.l	0		; APTR WAD NODES
WADNODESSize		dc.l	0		; Size of WAD NODES
WADSEGS			dc.l	0		; APTR WAD SEGS
WADSEGSSize		dc.l	0		; Size of WAD SEGS
WADSSECTORS		dc.l	0		; APTR WAD SSECTORS
WADSSECTORSSize		dc.l	0		; Size of WAD SSECTORS
WADSECTORS		dc.l	0		; APTR WAD SECTORS
WADSECTORSSize		dc.l	0		; Size of WAD SECTORS
WADREJECT		dc.l	0		; APTR WAD REJECT
WADREJECTSize		dc.l	0		; Size of WAD REJECT
WADBLOCKMAP		dc.l	0		; APTR WAD BLOCKMAP
WADBLOCKMAPSize		dc.l	0		; Size of WAD BLOCKMAP
WADTHINGS		dc.l	0		; APTR WAD THINGS
WADTHINGS2		dc.l	0		; APTR WAD THINGS2 (PRIORITY SORTED)
WADTHINGSSize		dc.l	0		; Size of WAD THINGS
WADTHINGS2Size		dc.l	0		; Size of WAD THINGS (PRIORITY SORTED)
;
RLVERTEXES		dc.l	0		; APTR RL VERTEXES
RLVERTEXESSize		dc.l	0		; Size of RL VERTEXES
RLLINES			dc.l	0		; APTR RL LINES
RLLINESSize		dc.l	0		; Size of RL LINES
RLSECTORS		dc.l	0		; APTR RL SECTORS
RLSECTORSSize		dc.l	0		; Size of RL SECTORS
RLBSP			dc.l	0		; APTR RL BSP
RLBSPSize		dc.l	0		; Size of RL BSP
RLAREAS			dc.l	0		; APTR RL AREAS
RLAREASSize		dc.l	0		; Size of RL AREAS
RLSEGS			dc.l	0		; APTR RL SEGS
RLSEGSSize		dc.l	0		; Size of RL SEGS
RLFACES			dc.l	0		; APTR RL FACES
RLFACESSize		dc.l	0		; Size of RL FACES
RLOBJECTS		dc.l	0		; APTR RL OBJECTS
RLOBJECTSSize		dc.l	0		; Size of RL OBJECTS
RLBLOCKMAP		dc.l	0		; APTR RL BLOCKMAP
RLBLOCKMAPSize		dc.l	0		; Size of RL BLOCKMAP
RLREJECT		dc.l	0		; APTR RL REJECT
RLREJECTSize		dc.l	0		; Size of RL REJECT
RLDOORS			dc.l	0		; APTR RL DOORS
RLDOORSSize		dc.l	0		; Size of RL DOORS
RLSTAIRS		dc.l	0		; APTR RL STAIRS
RLSTAIRSSize		dc.l	0		; Size of RL STAIRS
RLFLOORS		dc.l	0		; APTR RL FLOORS
RLFLOORSSize		dc.l	0		; Size of RL FLOORS
RLLIFTS			dc.l	0		; APTR RL LIFTS
RLLIFTSSize		dc.l	0		; Size of RL LIFTS
RLCEILINGS		dc.l	0		; APTR RL CEILINGS
RLCEILINGSSize		dc.l	0		; Size of RL CEILINGS

Texture1Name		ds.b	10		; 8+NULL
Texture2Name		ds.b	10		; 8+NULL
Texture3Name		ds.b	10		; 8+NULL
TextureXOffset		ds.w	0		; XOffset for Texture Translation
TextureYOffset		ds.w	0		; YOffset for Texture Translation

ImageBGReMap		dc.b	0		; -1 = ReMap Image BackGround to 255
ImageReMap		dc.b	0		; -1 = ReMap Palette
RoundRLPalette		dc.b	0		; -1 = Round RL Palette
GammaRLPalette		dc.b	0		; -1 = GammaCorrect RL Palette
NoTextures2		dc.b	0		; -1 = Don't Use Floor/Ceiling Textures2!
			dc.b	0

DoomReMapPalette	ds.b	(3*256)		; ReMapping RGB Triplets
DoomReMapTable		ds.b	256		; Palette ReMapping Table (WAD->RL)
DoomUnMapTable		ds.b	256		; Palette ReMapping Table (RL->WAD)

RLFACESPtrs		ds.l	MaxWADFaces	; Pointer to Each FACE
RLFACESFlags		ds.b	MaxWADFaces	; Flags for Each FACE
RLSECTOROrigins		ds.l	MaxWADSectors	; Sound OriginX,Y for Each SECTOR

RLOBJReMapTable		ds.w	MaxRLObjectTypes ; Maximum RLObjectTypes

RLNUMDOORS		ds.l	1		; #DOORS
RLNUMSTAIRS		ds.l	1		; #STAIRS
RLNUMFLOORS		ds.l	1		; #FLOORS
RLNUMLIFTS		ds.l	1		; #LIFTS
RLNUMCEILINGS		ds.l	1		; #CEILINGS
RLNUMOBJECTS		ds.l	1		; #LEVEL OBJECTS

	end
