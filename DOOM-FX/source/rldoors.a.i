VERSION		EQU	1
REVISION	EQU	47
DATE	MACRO
		dc.b	'18.5.95'
	ENDM
VERS	MACRO
		dc.b	'rldoors.a 1.47'
	ENDM
VSTRING	MACRO
		dc.b	'rldoors.a 1.47 (18.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rldoors.a 1.47 (18.5.95)',0
	ENDM
