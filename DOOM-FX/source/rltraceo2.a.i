VERSION		EQU	1
REVISION	EQU	49
DATE	MACRO
		dc.b	'12.6.95'
	ENDM
VERS	MACRO
		dc.b	'rltraceo2.a 1.49'
	ENDM
VSTRING	MACRO
		dc.b	'rltraceo2.a 1.49 (12.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rltraceo2.a 1.49 (12.6.95)',0
	ENDM
