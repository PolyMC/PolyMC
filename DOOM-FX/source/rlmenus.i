;**********************************************************************
;*                                                                    *
;*                   P R O J E C T :   REALITY_ENGINE                 *
;*                                                                    *
;*                                     MENUS STRUCTURE DEFINITIONS    *
;*                                                                    *
;**********************************************************************


;
;	* * * * * * *       MENU DATA STRUCTURE       * * * * * * *
;
rlmd			var	0
rlmdMenuMap		equ	rlmd			; Menu VRAM Map
rlmd			var	rlmd+3
rlmdMenuDef		equ	rlmd			; Menu VRAM Def
rlmd			var	rlmd+3
rlmdMenuHDMATM		equ	rlmd			; Menu HDMATM Table
rlmd			var	rlmd+3
rlmdInitCode		equ	rlmd			; InitCode-1
rlmd			var	rlmd+2
rlmdBackCode		equ	rlmd			; BackCode-1
rlmd			var	rlmd+2
rlmdItems		equ	rlmd			; Number of Menu Items
rlmd			var	rlmd+1
rlmdItemData		equ	rlmd			; ItemData
;rlmdSize		equ	rlmd
;
;	>>>   MENU ITEM DATA SUBSTRUCTURE   <<<
;
rlmid			var	0
rlmidCode		equ	rlmid			; ItemCode-1
rlmid			var	rlmid+2
rlmid0			equ	rlmid
rlmidSize		equ	rlmid0+6
;
;	>>>   MENU ITEM "NORMAL" SUBSTRUCTURE   <<<
;
rlmid			var	rlmid0
;
;	>>>   MENU ITEM "TOGGLE" SUBSTRUCTURE   <<<
;
rlmid			var	rlmid0
rlmidtVar		equ	rlmid			; Address of Toggle Variable
rlmid			var	rlmid+2
;
;	>>>   MENU KEY DATA SUBSTRUCTURE   <<<
;
rlmkd			var	0
rlmkdKeys		equ	rlmkd			; KeyBits (Terminated with $0000)
rlmkd			var	rlmkd+2
rlmkdCode		equ	rlmkd			; KeyCode-1
rlmkd			var	rlmkd+2
