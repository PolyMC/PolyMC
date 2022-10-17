;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                        MESSAGES AND TEXT MODULE                         *
;*                                                                         *
;***************************************************************************

	include	ripdoom.i

	section	MSGS,CODE

	xdef	MSGInit
	xdef	MSGNoMem
	xdef	MSGDoomWADError
	xdef	MSGUserBreak
	xdef	MSGNoOutputName
	xdef	MSGRLTextureListError
	xdef	MSGRLFloorListError
	xdef	MSGPicError
	xdef	MSGMaxFObjError
	xdef	MSGMaxMObjError


			dc.b	'$VER:'
MSGInit			dc.b	'RIPDOOM 1.111 (C) 1994-1995 Randy Linden/Sculptured Software',10,0
MSGNoMem		dc.b	'Not enough memory',10,0

MSGDoomWADError		dc.b	'Error with DOOM.WAD datafile!',10,0

MSGUserBreak		dc.b	'User Break',10,0

MSGNoOutputName		dc.b	'Error no OUTPUT Name!',10,0

MSGRLTextureListError	dc.b	'Error with TEXTURELIST datafile!',10,0

MSGRLFloorListError	dc.b	'Error with FLOORLIST datafile!',10,0

MSGPicError		dc.b	'Error opening Display Picture!',10,0

MSGMaxFObjError		dc.b	'***   ERROR!  TOO MANY FIXED OBJECTS!   ***',10,0
MSGMaxMObjError		dc.b	'***   ERROR!  TOO MANY MOVABLE OBJECTS!   ***',10,0

			dc.w	0

	end
