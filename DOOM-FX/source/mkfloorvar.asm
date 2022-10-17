;***************************************************************************
;*                                                                         *
;*                           M A K E   F L O O R                           *
;*                                                                         *
;*                            VARIABLES MODULE                             *
;*                                                                         *
;***************************************************************************

	include	mkfloor.i

	section	__MERGED,DATA

	xdef	DosName,DosBase
	xdef	IntuitionName,IntuitionBase
	xdef	GraphicsName,GraphicsBase

	xdef	DoomPaletteName,FloorListName,FloorList2Name
	xdef	FloorDefName

	xdef	IFFMode,IFFMask,IFFComp,IFFCompLen,IFFCompData

	xdef	Task,OutputFIB,MsgBuffer
	xdef	argc,argv
	xdef	SystemMemory
	xdef	Quiet,Verbose

	xdef	FloorColour

	xdef	DoomPalette
	xdef	FloorList,FloorList2
	xdef	FloorName,FloorFileName

	xdef	FloorDef,FloorDefPtr

	xdef	FloorColourTally


DosName			dc.b	'dos.library',0
GraphicsName		dc.b	'graphics.library',0
IntuitionName		dc.b	'intuition.library',0
DoomPaletteName		dc.b	'HD2:DOOMDATA/PALETTES/PLAYPAL',0
FloorListName		dc.b	'FLOORLIST',0
FloorList2Name		dc.b	'FLOORLIST2',0
FloorDefName		dc.b	'RLDATA:FLOORS/FLOORS.DEF',0
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
MsgBuffer		ds.b	160		; Message Text Buffer
Quiet			dc.b	0
Verbose			dc.b	0

DoomPalette		dc.l	0		; Pointer to DOOM.WAD Palette
FloorList		dc.l	0		; Pointer to FLOORLIST DataFile
FloorList2		dc.l	0		; Pointer to FLOORLIST2 DataFile
FloorName		ds.b	80		; FloorName Text
FloorFileName		ds.b	80		; FloorFileName Text

FloorDef		dc.l	0		; Pointer to FLOORDEF Data
FloorDefPtr		dc.l	0		; Pointer to FLOORDEF Data Current

FloorColour		dc.b	0		; -1 = Use COLOUR for TEXTURE2
			dc.b	0

FloorColourTally	ds.w	256		; Tally of Colours Used in a Single Floor


	end
