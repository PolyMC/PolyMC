VERSION		EQU	1
REVISION	EQU	62
DATE	MACRO
		dc.b	'27.4.95'
	ENDM
VERS	MACRO
		dc.b	'rldraws.a 1.62'
	ENDM
VSTRING	MACRO
		dc.b	'rldraws.a 1.62 (27.4.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rldraws.a 1.62 (27.4.95)',0
	ENDM
