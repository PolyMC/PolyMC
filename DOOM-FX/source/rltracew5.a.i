VERSION		EQU	1
REVISION	EQU	35
DATE	MACRO
		dc.b	'7.5.95'
	ENDM
VERS	MACRO
		dc.b	'rltracew5.a 1.35'
	ENDM
VSTRING	MACRO
		dc.b	'rltracew5.a 1.35 (7.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rltracew5.a 1.35 (7.5.95)',0
	ENDM
