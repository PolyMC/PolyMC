;***************************************************************************
;*                                                                         *
;*                            M A K E   F L O O R                          *
;*                                                                         *
;*                              WINDOWS MODULE                             *
;*                                                                         *
;***************************************************************************

	include	mkfloor.i

	section	WINDOWS,CODE

	xdef	PicScreen,PicWindow,PicVPort
	xdef	NewSNESPicScreen
	xdef	NewSNESPicScreenTags
	xdef	NewSNESPicWindow,NewSNESPicWindowScreen


;
;	* * * * * * *       TEXT TITLES       * * * * * * *
;
NewSNESPicScreenTitle
	dc.b	'MKFLOOR Picture Screen',0

	dc.w	0


;
;	* * * * * * *       SNES PICTURE SCREEN/WINDOWS       * * * * * * *
;
NewSNESPicScreen
	dc.w	0,100,-1,-1,5
	dc.b	0,1
	dc.w	$0000
	dc.w	$000f
	dc.l	0
	dc.l	NewSNESPicScreenTitle
	dc.l	0
	dc.l	0

NewSNESPicScreenTags
	dc.l	$8000003a,NewSNESPicScreenCols
	dc.l	$80000034,1
	dc.l	$80000039,-1
	dc.l	0,0
NewSNESPicScreenCols
	dc.w	-1

NewSNESPicWindow
	dc.w	0,11,-1,-1
	dc.b	0,1
	dc.l	$00000000
	dc.l	$00001900
	dc.l	0	;SNESPicWindowGadgets
	dc.l	0
	dc.l	0
NewSNESPicWindowScreen
	dc.l	0
	dc.l	0
	dc.w	-1,-1,-1,-1
	dc.w	$000f

;
;	* * * * * * *       SCREENS/WINDOWS POINTERS       * * * * * * *
;
PicScreen		dc.l	0				; Picture Screen Pointer
PicWindow		dc.l	0				; Picture Window Pointer
PicVPort		dc.l	0				; Picture ViewPort


	end
