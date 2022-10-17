;***************************************************************************
;*                                                                         *
;*                            S P L I T   M U S                            *
;*                                                                         *
;*                        MESSAGES AND TEXT MODULE                         *
;*                                                                         *
;***************************************************************************

	include	spmus.i

	section	MSGS,CODE

	xdef	MSGInit
	xdef	MSGNewLine
	xdef	MSGNoMem
	xdef	MSGUserBreak

	xdef	MSGNoMUS
	xdef	MSGNoOut
	xdef	MSGNoAsm
	xdef	MSGNoSect

	xdef	MSGMUSError
	xdef	MSGASMError
	xdef	MSGOUTError

	xdef	MSGMUSTable
	xdef	MSGBadMUS

			dc.b	'$VER:'
MSGInit			dc.b	'SPMUS 1.2 ',$a9,' 1994-1995 Randy Linden/Sculptured Software',10,0
MSGNewLine		dc.b	10,0
MSGNoMem		dc.b	'Not enough memory',10,0
MSGUserBreak		dc.b	'User Break',10,0

MSGNoMUS		dc.b	'No MUS Object file specified!',10,0
MSGNoOut		dc.b	'No OUTPUT BASE filename specified!',10,0
MSGNoAsm		dc.b	'No OUTPUT ASM filename specified!',10,0
MSGNoSect		dc.b	'No SECTION BASE name specified!',10,0

MSGMUSError		dc.b	'Error with MUS file!',10,0
MSGASMError		dc.b	'Error with ASM file!',10,0
MSGOUTError		dc.b	'Error with OUTPUT file!',10,0

MSGMUSTable		dc.b	10,'MUS NOB CHUNKS',10
			dc.b	'Origin  Load      Size',10
			dc.b	'------- --------- ---------',10,0

MSGBadMUS		dc.b	'Error in MUS data!',10,0

			dc.w	0
	end
