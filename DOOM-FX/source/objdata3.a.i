VERSION		EQU	1
REVISION	EQU	1
DATE	MACRO
		dc.b	'12.6.95'
	ENDM
VERS	MACRO
		dc.b	'objdata3.a 1.1'
	ENDM
VSTRING	MACRO
		dc.b	'objdata3.a 1.1 (12.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: objdata3.a 1.1 (12.6.95)',0
	ENDM
