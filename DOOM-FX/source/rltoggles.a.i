VERSION		EQU	1
REVISION	EQU	3
DATE	MACRO
		dc.b	'17.5.95'
	ENDM
VERS	MACRO
		dc.b	'rltoggles.a 1.3'
	ENDM
VSTRING	MACRO
		dc.b	'rltoggles.a 1.3 (17.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rltoggles.a 1.3 (17.5.95)',0
	ENDM
