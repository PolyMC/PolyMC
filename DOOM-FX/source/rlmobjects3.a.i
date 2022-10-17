VERSION		EQU	1
REVISION	EQU	64
DATE	MACRO
		dc.b	'9.6.95'
	ENDM
VERS	MACRO
		dc.b	'rlmobjects3.a 1.64'
	ENDM
VSTRING	MACRO
		dc.b	'rlmobjects3.a 1.64 (9.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlmobjects3.a 1.64 (9.6.95)',0
	ENDM
