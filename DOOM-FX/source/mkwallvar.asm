;***************************************************************************
;*                                                                         *
;*                            M A K E   W A L L                            *
;*                                                                         *
;*                            VARIABLES MODULE                             *
;*                                                                         *
;***************************************************************************

	include	mkwall.i

	section	__MERGED,DATA

	xdef	DosName,DosBase
	xdef	IntuitionName,IntuitionBase
	xdef	GraphicsName,GraphicsBase

	xdef	DoomPaletteName,WallListName,ImageListName
	xdef	WallTableName,WallDataName,WallDefName
	xdef	ImageTableName,ImageDataName,ImageDefName
	xdef	WallImgDefName,WallImgSourceName
	xdef	DoImages,ImageComp

	xdef	IFFMode,IFFMask,IFFComp,IFFCompLen,IFFCompData

	xdef	Task,OutputFIB,MsgBuffer,MsgBuffer2
	xdef	argc,argv
	xdef	SystemMemory
	xdef	Quiet,Verbose

	xdef	DoomPalette
	xdef	WallList
	xdef	WallName,WallFileName

	xdef	WallTable,WallTablePtr
	xdef	WallData,WallDataPtr
	xdef	WallDef

	xdef	WallStrip,WallStrip2
	xdef	NumStripsNew,NumStripsOld,NumStripsPart
	xdef	NumPixels,NumPixelsTotal,NumPixelsUsed,NumPixelsUsedTotal


DosName			dc.b	'dos.library',0
GraphicsName		dc.b	'graphics.library',0
IntuitionName		dc.b	'intuition.library',0
DoomPaletteName		dc.b	'DOOMDATA:PALETTES/PLAYPAL',0
WallListName		dc.b	'WallList',0
WallTableName		dc.b	'RLDATA:WALLS/WALLS.TBL',0
WallDataName		dc.b	'RLDATA:WALLS/WALLS.DAT',0
WallDefName		dc.b	'RLDATA:WALLS/WALLS.DEF%02lx',0
;
ImageListName		dc.b	'ImageList',0
ImageTableName		dc.b	'RLDATA:IMAGES/IMAGES.TBL',0
ImageDataName		dc.b	'RLDATA:IMAGES/IMAGES.DAT',0
ImageDefName		dc.b	'RLDATA:IMAGES/IMAGES.DEF%02lx',0
;
WallImgDefName		dc.b	'RLDATA:WALLIMG/BANK%02lx.DEF',0
WallImgSourceName	dc.b	'RL:rlwallimgdef%ld.a',0
			dc.w	0

DosBase			dc.l	0		; dos.library
GraphicsBase		dc.l	0		; graphics.library
IntuitionBase		dc.l	0		; intuition.library

IFFMode			dc.b	0		; 0=ILBM,1=PBM
IFFMask			dc.b	0		; 0=No Mask/Stencil,1=Stencil/Mask ON
IFFComp			dc.b	0		; Compression Format
IFFCompLen		dc.b	0		; Compression Length
IFFCompData		dc.b	0		; Compression DataByte

Task			dc.l	0		; Address of Amiga_Task_Structure
OutputFIB		dc.l	0		; Output FIB
argc			dc.l	0
argv			ds.l	256
SystemMemory		dc.l	0		; Pointer to System Memory Block
MsgBuffer		ds.b	256		; Message Text Buffer
MsgBuffer2		ds.b	256		; Message Text Buffer2
Quiet			dc.b	0
Verbose			dc.b	0

DoomPalette		dc.l	0		; Pointer to DOOM.WAD Palette
WallList		dc.l	0		; Pointer to WALLLIST DataFile
WallName		ds.b	80		; WallName Text
WallFileName		ds.b	80		; WallFileName Text

WallTable		dc.l	0		; Pointer to WALLTABLE Data
WallTablePtr		dc.l	0		; Pointer to WALLTABLE Data Current

WallData		dc.l	0		; Pointer to WALLDATA Data
WallDataPtr		dc.l	0		; Pointer to WALLDATA Data Current

WallDef			dc.l	0		; Pointer to WALLDEF Data

WallStrip		ds.b	256		; 256 Pixel Tall Wall Strip (Compressed)
WallStrip2		ds.b	256		; 256 Pixel Tall Wall Strip (UnCompressed)
NumStripsNew		dc.l	0		; #PixelStrips Added
NumStripsOld		dc.l	0		; #PixelStrips ReUsed
NumStripsPart		dc.l	0		; #PixelStrips Partially Added/ReUsed

NumPixels		dc.l	0		; #Pixels
NumPixelsTotal		dc.l	0		; #Pixels Total
NumPixelsUsed		dc.l	0		; #Pixels Used
NumPixelsUsedTotal	dc.l	0		; #Pixels Used Total

DoImages		dc.b	0		; Process IMAGES instead of WALLS
ImageComp		dc.b	0		; Compress IMAGES SMALLER vs. FASTER

	end
