VERSION		EQU	1
REVISION	EQU	75
DATE	MACRO
		dc.b	'17.10.95'
	ENDM
VERS	MACRO
		dc.b	'rlmobjects.a 1.75'
	ENDM
VSTRING	MACRO
		dc.b	'rlmobjects.a 1.75 (17.10.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlmobjects.a 1.75 (17.10.95)',0
	ENDM
