VERSION		EQU	1
REVISION	EQU	30
DATE	MACRO
		dc.b	'28.4.95'
	ENDM
VERS	MACRO
		dc.b	'inverse.a 1.30'
	ENDM
VSTRING	MACRO
		dc.b	'inverse.a 1.30 (28.4.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: inverse.a 1.30 (28.4.95)',0
	ENDM
