;***************************************************************************
;*                                                                         *
;*                             C O N V   G U S                             *
;*                                                                         *
;*                         MESSAGES AND TEXT MODULE                        *
;*                                                                         *
;***************************************************************************

	include	convgus.i

	section	MSGS,CODE

	xdef	MSGInit
	xdef	MSGNoMem
	xdef	MSGUserBreak
	xdef	MSGNoPatchName
	xdef	MSGBadPatch
	xdef	MSGNoOutputName
	xdef	MSGBadOutput


			dc.b	'$VER:'
MSGInit			dc.b	'CONVGUS 1.1 Convert GUS PatchData',10,0
MSGNoMem		dc.b	'Not enough memory',10,0

MSGUserBreak		dc.b	'User Break',10,0

MSGNoPatchName		dc.b	'Error no PATCH File!',10,0
MSGBadPatch		dc.b	'Error with PATCH File!',10,0
MSGNoOutputName		dc.b	'Error no OUTPUT Name!',10,0
MSGBadOutput		dc.b	'Error with OUTPUT File!',10,0

			dc.w	0

	end
