VERSION		EQU	1
REVISION	EQU	41
DATE	MACRO
		dc.b	'19.5.95'
	ENDM
VERS	MACRO
		dc.b	'rlplats.a 1.41'
	ENDM
VSTRING	MACRO
		dc.b	'rlplats.a 1.41 (19.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlplats.a 1.41 (19.5.95)',0
	ENDM
