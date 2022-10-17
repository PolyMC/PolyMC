VERSION		EQU	1
REVISION	EQU	8
DATE	MACRO
		dc.b	'30.5.95'
	ENDM
VERS	MACRO
		dc.b	'rlenemies7.a 1.8'
	ENDM
VSTRING	MACRO
		dc.b	'rlenemies7.a 1.8 (30.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlenemies7.a 1.8 (30.5.95)',0
	ENDM
