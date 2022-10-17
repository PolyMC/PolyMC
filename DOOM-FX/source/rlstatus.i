;**********************************************************************
;*                                                                    *
;*                   P R O J E C T :   REALITY_ENGINE                 *
;*                                                                    *
;*                                STATUS BAR STRUCTURE DEFINITIONS    *
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
StatFacePosX		equ	RLStatStartPixX+103	; X Coordinate of Face
StatFacePosY		equ	RLStatStartPixY+0	; Y Coordinate of Face
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
StatAmmoPosX		equ	RLStatStartPixX+2	; X Coordinate of Ammo
StatAmmoPosY		equ	RLStatStartPixY+3	; Y Offset of Ammo
;
;	>>>   HEALTH STATNUM DEFINITIONS   <<<
;
StatHealthPosX		equ	RLStatStartPixX+52	; X Coordinate of Health
StatHealthPosY		equ	RLStatStartPixY+3	; Y Offset of Health
;
;	>>>   ARMOR STATNUM DEFINITIONS   <<<
;
StatArmorPosX		equ	RLStatStartPixX+168	; X Coordinate of Armor
StatArmorPosY		equ	RLStatStartPixY+3	; Y Offset of Armor
;
;	>>>   KEYS DEFINITIONS   <<<
;
StatKeyRPosX		equ	RLStatStartPixX+133	; X Coordinate of Red Key
StatKeyRPosY		equ	RLStatStartPixY+2	; Y Coordinate of Red Key
StatKeyYPosX		equ	RLStatStartPixX+145	; X Coordinate of Yellow Key
StatKeyYPosY		equ	RLStatStartPixY+2	; Y Coordinate of Yellow Key
StatKeyBPosX		equ	RLStatStartPixX+156	; X Coordinate of Blue Key
StatKeyBPosY		equ	RLStatStartPixY+2	; Y Coordinate of Blue Key
;
;	>>>   ARMS DEFINITIONS   <<<
;
StatArmsPosX		equ	RLStatStartPixX+133	; X Coordinate of Arms
StatArmsPosY		equ	RLStatStartPixY+14	; Y Coordinate of Arms
