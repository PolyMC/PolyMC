VERSION		EQU	1
REVISION	EQU	11
DATE	MACRO
		dc.b	'7.5.95'
	ENDM
VERS	MACRO
		dc.b	'rllights.a 1.11'
	ENDM
VSTRING	MACRO
		dc.b	'rllights.a 1.11 (7.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rllights.a 1.11 (7.5.95)',0
	ENDM
