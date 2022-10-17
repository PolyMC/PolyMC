VERSION		EQU	1
REVISION	EQU	27
DATE	MACRO
		dc.b	'9.6.95'
	ENDM
VERS	MACRO
		dc.b	'sightray.a 1.27'
	ENDM
VSTRING	MACRO
		dc.b	'sightray.a 1.27 (9.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: sightray.a 1.27 (9.6.95)',0
	ENDM
