;**********************************************************************
;*                                                                    *
;*                       P R O J E C T :   R A G E                    *
;*                                                                    *
;*                                          STATUS BAR DEFINITIONS    *
;*                                                                    *
;**********************************************************************

;
;	* * * * * * *       STATUS BAR DEFINITIONS       * * * * * * *
;
;	>>>   FACE DEFINITIONS   <<<
;
StatFacesX		equ	8			; #Faces per Line in Imagery
StatFacePixX		equ	24			; X Pixels Wide
StatFacePixY		equ	32			; Y Pixels Tall
StatFaceCharX		equ	StatFacePixX/8		; X Characters Wide
StatFaceCharY		equ	StatFacePixY/8		; Y Characters Tall
StatFacePosX		equ	104			; X Coordinate of Face
StatFacePosY		equ	0			; Y Coordinate of Face
;
;	>>>   STATNUM DEFINITIONS   <<<
;
StatNumsX		equ	10			; #Digits per Line in Imagery
StatNumPixX		equ	16			; X Pixels Wide
StatNumPixY		equ	16			; Y Pixels Tall
StatNumCharX		equ	StatNumPixX/8		; X Characters Wide
StatNumCharY		equ	StatNumPixY/8		; Y Characters Tall
;
;	>>>   AMMO STATNUM DEFINITIONS   <<<
;
StatAmmoPosX		equ	0			; X Coordinate of Ammo
StatAmmoOfsY		equ	2			; Y Offset of Ammo
;
;	>>>   HEALTH STATNUM DEFINITIONS   <<<
;
StatHealthPosX		equ	40			; X Coordinate of Health
StatHealthOfsY		equ	2			; Y Offset of Health
;
;	>>>   ARMOR STATNUM DEFINITIONS   <<<
;
StatArmorPosX		equ	168			; X Coordinate of Armor
StatArmorOfsY		equ	2			; Y Offset of Armor
