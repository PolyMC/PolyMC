;***************************************************************************
;*                                                                         *
;*                         M A K E   T E X T U R E                         *
;*                                                                         *
;*                            VARIABLES MODULE                             *
;*                                                                         *
;***************************************************************************

	include	mktex.i

	section	__MERGED,DATA

	xdef	DosName,DosBase
	xdef	IntuitionName,IntuitionBase
	xdef	GraphicsName,GraphicsBase

	xdef	TextureListName,TextureListName2,WallListName,TextureDataName

	xdef	Task,OutputFIB,MsgBuffer
	xdef	argc,argv
	xdef	SystemMemory
	xdef	Quiet,Verbose

	xdef	TextureList,TextureList2,WallList,WallListPtr
	xdef	TextureData,TextureDataPtr,TextureDataPtr0
	xdef	TextureName,WallName,WallFileName
	xdef	TextureTableName,TextureTable,TextureTablePtr
	xdef	TextureTable2,TextureTable2Name
	xdef	TextureWidth,TextureHeight
	xdef	WallWidth,WallHeight
	xdef	PatchXOffset,PatchYOffset
	xdef	WallPrefixName,WallPrefix2Name
	xdef	WallTranslation,TextureCount

	xdef	SinglePatch


DosName			dc.b	'dos.library',0
GraphicsName		dc.b	'graphics.library',0
IntuitionName		dc.b	'intuition.library',0
TextureListName		dc.b	'TextureList',0
TextureListName2	dc.b	'TextureList2',0
WallListName		dc.b	'WallList',0
WallPrefixName		dc.b	'DOOMIFF:WALLS/',0
WallPrefix2Name		dc.b	'RLART:WALLS/',0
TextureDataName		dc.b	'RLDATA:WALLS/TEXTURE.DAT',0
TextureTableName	dc.b	'RLDATA:WALLS/TEXTURE.TBL',0
TextureTable2Name	dc.b	'RLDATA:WALLS/TEXTURE2.TBL',0
			dc.w	0

DosBase			dc.l	0		; dos.library
GraphicsBase		dc.l	0		; graphics.library
IntuitionBase		dc.l	0		; intuition.library

Task			dc.l	0		; Address of Amiga_Task_Structure
OutputFIB		dc.l	0		; Output FIB
argc			dc.l	0
argv			ds.l	256
SystemMemory		dc.l	0		; Pointer to System Memory Block
MsgBuffer		ds.b	256		; Message Text Buffer
Quiet			dc.b	0
Verbose			dc.b	0

TextureList		dc.l	0		; APTR TEXTURELIST DataFile
TextureList2		dc.l	0		; APTR TEXTURELIST2 DataFile

WallList		dc.l	0		; APTR WALLLIST DataFile
WallListPtr		dc.l	0		; APTR WALLLIST DataFile Current

TextureData		dc.l	0		; APTR TEXTUREDATA DataFile
TextureDataPtr		dc.l	0		; APTR TEXTUREDATA DataFile Current
TextureDataPtr0		dc.l	0

TextureTable		dc.l	0		; APTR TEXTURETABLE DataFile
TextureTablePtr		dc.l	0		; APTR TEXTURETABLE DataFile Current

TextureName		ds.b	80		; TextureName Text
WallName		ds.b	80		; Wall Text
WallFileName		ds.b	80		; WallFileName Text

TextureWidth		dc.l	0		; Width of Texture
TextureHeight		dc.l	0		; Height of Texture
WallWidth		dc.l	0		; Width of Wall
WallHeight		dc.l	0		; Height of Wall
PatchXOffset		dc.l	0		; X Offset of Patch
PatchYOffset		dc.l	0		; Y Offset of Patch

SinglePatch		dc.b	0		; -1 = SINGLE PATCH Mode
			dc.b	0

WallTranslation		dc.w	0		; Wall Translation Number

TextureCount		dc.w	0		; #Textures Processed
TextureTable2		ds.b	1024		; Texture Alternate Translation Table


	end
