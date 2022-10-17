VERSION		EQU	1
REVISION	EQU	83
DATE	MACRO
		dc.b	'9.6.95'
	ENDM
VERS	MACRO
		dc.b	'rlcolour.a 1.83'
	ENDM
VSTRING	MACRO
		dc.b	'rlcolour.a 1.83 (9.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlcolour.a 1.83 (9.6.95)',0
	ENDM
