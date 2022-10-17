VERSION		EQU	1
REVISION	EQU	68
DATE	MACRO
		dc.b	'17.10.95'
	ENDM
VERS	MACRO
		dc.b	'rlmobjects2.a 1.68'
	ENDM
VSTRING	MACRO
		dc.b	'rlmobjects2.a 1.68 (17.10.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlmobjects2.a 1.68 (17.10.95)',0
	ENDM
