VERSION		EQU	1
REVISION	EQU	52
DATE	MACRO
		dc.b	'9.6.95'
	ENDM
VERS	MACRO
		dc.b	'rlenemies2.a 1.52'
	ENDM
VSTRING	MACRO
		dc.b	'rlenemies2.a 1.52 (9.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlenemies2.a 1.52 (9.6.95)',0
	ENDM
