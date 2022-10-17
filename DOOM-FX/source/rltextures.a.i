VERSION		EQU	1
REVISION	EQU	52
DATE	MACRO
		dc.b	'18.5.95'
	ENDM
VERS	MACRO
		dc.b	'rltextures.a 1.52'
	ENDM
VSTRING	MACRO
		dc.b	'rltextures.a 1.52 (18.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rltextures.a 1.52 (18.5.95)',0
	ENDM
