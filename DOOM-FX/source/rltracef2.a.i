VERSION		EQU	1
REVISION	EQU	45
DATE	MACRO
		dc.b	'23.5.95'
	ENDM
VERS	MACRO
		dc.b	'rltracef2.a 1.45'
	ENDM
VSTRING	MACRO
		dc.b	'rltracef2.a 1.45 (23.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rltracef2.a 1.45 (23.5.95)',0
	ENDM
