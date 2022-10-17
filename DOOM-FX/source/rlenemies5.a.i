VERSION		EQU	1
REVISION	EQU	55
DATE	MACRO
		dc.b	'17.10.95'
	ENDM
VERS	MACRO
		dc.b	'rlenemies5.a 1.55'
	ENDM
VSTRING	MACRO
		dc.b	'rlenemies5.a 1.55 (17.10.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlenemies5.a 1.55 (17.10.95)',0
	ENDM
