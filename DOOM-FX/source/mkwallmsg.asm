;***************************************************************************
;*                                                                         *
;*                            M A K E   W A L L                            *
;*                                                                         *
;*                        MESSAGES AND TEXT MODULE                         *
;*                                                                         *
;***************************************************************************

	include	mkwall.i

	section	MSGS,CODE

	xdef	MSGInit
	xdef	MSGNoMem
	xdef	MSGUserBreak
	xdef	MSGNoOutputName
	xdef	MSGNoWallList
	xdef	MSGNoImageList

	xdef	MSGWallTblError
	xdef	MSGWallDataError

	xdef	MSGImageTblError
	xdef	MSGImageDataError

	xdef	MSGFileError

	xdef	MSGPicError


			dc.b	'$VER:'
MSGInit			dc.b	'MKWALL 3.6 Make WALL/IMAGE Data',10,0
MSGNoMem		dc.b	'Not enough memory',10,0

MSGUserBreak		dc.b	'User Break',10,0

MSGNoOutputName		dc.b	'Error no OUTPUT Name!',10,0
MSGNoWallList		dc.b	'Error no WALLLIST File!',10,0
MSGNoImageList		dc.b	'Error no IMAGELIST File!',10,0

MSGWallTblError		dc.b	'Error with WALLS.TBL datafile!',10,0
MSGWallDataError	dc.b	'Error with WALLS.DAT datafile!',10,0

MSGImageTblError	dc.b	'Error with IMAGES.TBL datafile!',10,0
MSGImageDataError	dc.b	'Error with IMAGES.DAT datafile!',10,0

MSGFileError		dc.b	'Error with <%s> datafile!',10,0

MSGPicError		dc.b	'Error opening Display Picture!',10,0

			dc.w	0

	end
