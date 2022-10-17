;**********************************************************************
;*                                                                    *
;*                   P R O J E C T :   REALITY_ENGINE                 *
;*                                                                    *
;*                    CUTTER DEFINITIONS AND EQUATES                  *
;*                                                                    *
;**********************************************************************


;
;	* * * * * * *       DEFINITIONS       * * * * * * *
;
MaxCutImages		equ	6		; Maximum RESIDENT CUTTER Images
MaxCutCharData		equ	128		; Maximum #BYTES for _CharData
MaxCutMiscData		equ	704		; Maximum #BYTES for _MiscData
MaxCutCharDef		equ	7168		; Maximum #BYTES for _CharDef


;
;	* * * * * * *       IMAGE TABLE       * * * * * * *
;
;	n *	/	DC.W	CHARDATA INDEX
;		\	DC.W	MISCDATA INDEX
;
its		var	0
itsCharData	equ	its			; Character Data Index
its		var	its+2
itsMiscData	equ	its			; Miscellaneous Data Index
its		var	its+2
itsSize		equ	its


;
;	* * * * * * *       MISCELLANEOUS IMAGE DATA TABLES       * * * * * * *
;
;			DC.B	#OAMs_Required
;
;	DataListTable:
;			DC.B	%vh00ccc1
;				v	(Vertical Flipping)
;				h	(Horizontal Flipping)
;				ccc	(Colour Palette)
;			DC.B	OAM_X_Pixel_Offset
;			DC.B	OAM_Y_Pixel_Offset
;
imd		var	0
imdOAMNum	equ	imd			; Number of OAMs Required Total
imd		var	imd+1
imdDataList	equ	imd			; DataList Table Starting
;
imdl		var	0
imdlFlags	equ	imdl			; Flags
imdl		var	imdl+1
imdlOAMXOfs	equ	imdl			; OAM X_Offset
imdl		var	imdl+1
imdlOAMYOfs	equ	imdl			; OAM Y_Offset
imdl		var	imdl+1
imdlSize	equ	imdl
;
;	Image_MiscData_Flags:
;
;			DC.B	%vhppccc1
;				v	(Vertical Flipping)
;				h	(Horizontal Flipping)
;				p	(Priority)
;				ccc	(Colour Palette)
;
imdfVFlip	equ	$80			; Vertical Flipping
imdfHFlip	equ	$40			; Horizontal Flipping
imdfPri		equ	$30			; Priority
imdfColour	equ	$0e			; Colour Palette
