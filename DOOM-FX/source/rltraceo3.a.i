VERSION		EQU	1
REVISION	EQU	66
DATE	MACRO
		dc.b	'12.6.95'
	ENDM
VERS	MACRO
		dc.b	'rltraceo3.a 1.66'
	ENDM
VSTRING	MACRO
		dc.b	'rltraceo3.a 1.66 (12.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rltraceo3.a 1.66 (12.6.95)',0
	ENDM
