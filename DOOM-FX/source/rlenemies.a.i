VERSION		EQU	1
REVISION	EQU	106
DATE	MACRO
		dc.b	'9.6.95'
	ENDM
VERS	MACRO
		dc.b	'rlenemies.a 1.106'
	ENDM
VSTRING	MACRO
		dc.b	'rlenemies.a 1.106 (9.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlenemies.a 1.106 (9.6.95)',0
	ENDM
