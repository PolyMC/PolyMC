;***************************************************************************
;*                                                                         *
;*                          M A K E   S P R I T E                          *
;*                                                                         *
;*                            VARIABLES MODULE                             *
;*                                                                         *
;***************************************************************************

	include	mkspr.i

	section	__MERGED,DATA

	xdef	DosName,DosBase
	xdef	IntuitionName,IntuitionBase
	xdef	GraphicsName,GraphicsBase

	xdef	DoomPaletteName,SpriteListName

	xdef	IFFMode,IFFMask,IFFComp,IFFCompLen,IFFCompData

	xdef	Task,OutputFIB,MsgBuffer
	xdef	argc,argv
	xdef	SystemMemory
	xdef	Quiet,Verbose

	xdef	DoomPalette
	xdef	SpriteList
	xdef	SpriteName,SpriteFileName,SpriteDefFileName,SpriteDefFileName0

	xdef	SpriteDef,SpriteDefPtr
	xdef	SpriteMinX,SpriteMaxX


DosName			dc.b	'dos.library',0
GraphicsName		dc.b	'graphics.library',0
IntuitionName		dc.b	'intuition.library',0
DoomPaletteName		dc.b	'DOOMDATA:PALETTES/PLAYPAL',0
			dc.w	0

DosBase			dc.l	0		; dos.library
GraphicsBase		dc.l	0		; graphics.library
IntuitionBase		dc.l	0		; intuition.library

SpriteListName		dc.l	0

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
SpriteList		dc.l	0		; Pointer to SPRITELIST DataFile
SpriteName		ds.b	80		; SpriteName Text
SpriteFileName		ds.b	80		; SpriteFileName Text

SpriteDefFileName0	dc.b	'RLDATA:SPRITES/'
SpriteDefFileName	ds.b	80		; SpriteDefFileName Text

SpriteDef		dc.l	0		; Pointer to SPRITEDEF Data
SpriteDefPtr		dc.l	0		; Pointer to SPRITEDEF Data Current

SpriteMinX		dc.w	0		; Minimum X Coordinate
SpriteMaxX		dc.w	0		; Maximum X Coordinate

	end
