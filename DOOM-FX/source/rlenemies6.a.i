VERSION		EQU	1
REVISION	EQU	44
DATE	MACRO
		dc.b	'9.6.95'
	ENDM
VERS	MACRO
		dc.b	'rlenemies6.a 1.44'
	ENDM
VSTRING	MACRO
		dc.b	'rlenemies6.a 1.44 (9.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlenemies6.a 1.44 (9.6.95)',0
	ENDM
