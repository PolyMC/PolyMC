VERSION		EQU	1
REVISION	EQU	60
DATE	MACRO
		dc.b	'7.5.95'
	ENDM
VERS	MACRO
		dc.b	'rltracew4.a 1.60'
	ENDM
VSTRING	MACRO
		dc.b	'rltracew4.a 1.60 (7.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rltracew4.a 1.60 (7.5.95)',0
	ENDM
