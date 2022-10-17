VERSION		EQU	1
REVISION	EQU	39
DATE	MACRO
		dc.b	'28.4.95'
	ENDM
VERS	MACRO
		dc.b	'rlfloorsdef.a 1.39'
	ENDM
VSTRING	MACRO
		dc.b	'rlfloorsdef.a 1.39 (28.4.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlfloorsdef.a 1.39 (28.4.95)',0
	ENDM
