;***************************************************************************
;*                                                                         *
;*                           M A K E   F L O O R                           *
;*                                                                         *
;*                        MESSAGES AND TEXT MODULE                         *
;*                                                                         *
;***************************************************************************

	include	mkfloor.i

	section	MSGS,CODE

	xdef	MSGInit
	xdef	MSGNoMem
	xdef	MSGUserBreak
	xdef	MSGNoOutputName
	xdef	MSGNoFloorList

	xdef	MSGPicError
	xdef	MSGFloorDefError
	xdef	MSGFloorList2Error


			dc.b	'$VER:'
MSGInit			dc.b	'MKFLOOR 1.3 Make FLOOR Data',10,0
MSGNoMem		dc.b	'Not enough memory',10,0

MSGUserBreak		dc.b	'User Break',10,0

MSGNoOutputName		dc.b	'Error no OUTPUT Name!',10,0
MSGNoFloorList		dc.b	'Error no FLOORLIST File!',10,0

MSGPicError		dc.b	'Error opening Display Picture!',10,0
MSGFloorDefError	dc.b	'Error with FLOORS.DEF datafile!',10,0
MSGFloorList2Error	dc.b	'Error with FLOORLIST2 File!',10,0

			dc.w	0

	end
