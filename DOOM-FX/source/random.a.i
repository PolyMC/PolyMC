VERSION		EQU	1
REVISION	EQU	3
DATE	MACRO
		dc.b	'11.5.95'
	ENDM
VERS	MACRO
		dc.b	'random.a 1.3'
	ENDM
VSTRING	MACRO
		dc.b	'random.a 1.3 (11.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: random.a 1.3 (11.5.95)',0
	ENDM
