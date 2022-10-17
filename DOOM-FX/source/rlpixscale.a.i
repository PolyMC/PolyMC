VERSION		EQU	1
REVISION	EQU	13
DATE	MACRO
		dc.b	'21.5.95'
	ENDM
VERS	MACRO
		dc.b	'rlpixscale.a 1.13'
	ENDM
VSTRING	MACRO
		dc.b	'rlpixscale.a 1.13 (21.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlpixscale.a 1.13 (21.5.95)',0
	ENDM
