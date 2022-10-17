;***************************************************************************
;*                                                                         *
;*                         M A K E   T E X T U R E                         *
;*                                                                         *
;*                        MESSAGES AND TEXT MODULE                         *
;*                                                                         *
;***************************************************************************

	include	mktex.i

	section	MSGS,CODE

	xdef	MSGInit
	xdef	MSGNoMem
	xdef	MSGUserBreak
	xdef	MSGNoOutputName
	xdef	MSGNoTextureList

	xdef	MSGWallListError
	xdef	MSGTextureList2Error
	xdef	MSGTextureDataError
	xdef	MSGTextureTableError
	xdef	MSGTextureTable2Error
	xdef	MSGBadTexDim
	xdef	MSGBadPatchDim


			dc.b	'$VER:'
MSGInit			dc.b	'MKTEX 1.5 Make TEXTURE Data',10,0
MSGNoMem		dc.b	'Not enough memory',10,0

MSGUserBreak		dc.b	'User Break',10,0

MSGNoOutputName		dc.b	'Error no OUTPUT Name!',10,0
MSGNoTextureList	dc.b	'Error no TEXTURELIST File!',10,0

MSGWallListError	dc.b	'Error with WALLLIST datafile!',10,0
MSGTextureList2Error	dc.b	'Error with TEXTURELIST2 datafile!',10,0
MSGTextureDataError	dc.b	'Error with TEXTUREDATA datafile!',10,0
MSGTextureTableError	dc.b	'Error with TEXTURETABLE datafile!',10,0
MSGTextureTable2Error	dc.b	'Error with TEXTURETABLE2 datafile!',10,0

MSGBadTexDim		dc.b	'Error with TEXTURE Dimensions!',10,0
MSGBadPatchDim		dc.b	'Error with PATCH Dimensions!',10,0

			dc.w	0

	end
