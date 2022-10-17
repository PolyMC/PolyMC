;***************************************************************************
;*                                                                         *
;*                          M A K E   S P R I T E                          *
;*                                                                         *
;*                         MESSAGES AND TEXT MODULE                        *
;*                                                                         *
;***************************************************************************

	include	mkspr.i

	section	MSGS,CODE

	xdef	MSGInit
	xdef	MSGNoMem
	xdef	MSGUserBreak
	xdef	MSGNoOutputName
	xdef	MSGNoSpriteList
	xdef	MSGSpriteImgErr


			dc.b	'$VER:'
MSGInit			dc.b	'MKSPRITE 1.1 Make SPRITE Data',10,0
MSGNoMem		dc.b	'Not enough memory',10,0

MSGUserBreak		dc.b	'User Break',10,0

MSGNoOutputName		dc.b	'Error no OUTPUT Name!',10,0
MSGNoSpriteList		dc.b	'Error no SPRITELIST File!',10,0

MSGSpriteImgErr		dc.b	'Error with SPRITE Imagery!',10,0

			dc.w	0

	end
