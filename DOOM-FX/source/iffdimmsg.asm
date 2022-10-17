;***************************************************************************
;*                                                                         *
;*                              I F F   D I M                              *
;*                                                                         *
;*                         MESSAGES AND TEXT MODULE                        *
;*                                                                         *
;***************************************************************************

	include	iffdim.i

	section	MSGS,CODE

	xdef	MSGInit
	xdef	MSGNoMem
	xdef	MSGUserBreak
	xdef	MSGNoListName
	xdef	MSGBadList
	xdef	MSGBadIFF
	xdef	MSGBadLevel


			dc.b	'$VER:'
MSGInit			dc.b	'IFFDIM 1.0 IFF Dimensions',10,0
MSGNoMem		dc.b	'Not enough memory',10,0

MSGUserBreak		dc.b	'User Break',10,0

MSGNoListName		dc.b	'Error no LIST File!',10,0
MSGBadList		dc.b	'Error with LIST File!',10,0
MSGBadIFF		dc.b	'Error with IFF File!',10,0
MSGBadLevel		dc.b	'Error with LEVEL File!',10,0

			dc.w	0

	end
