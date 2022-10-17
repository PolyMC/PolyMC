VERSION		EQU	1
REVISION	EQU	48
DATE	MACRO
		dc.b	'31.5.95'
	ENDM
VERS	MACRO
		dc.b	'rlenemies3.a 1.48'
	ENDM
VSTRING	MACRO
		dc.b	'rlenemies3.a 1.48 (31.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlenemies3.a 1.48 (31.5.95)',0
	ENDM
