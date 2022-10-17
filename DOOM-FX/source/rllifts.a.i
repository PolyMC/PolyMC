VERSION		EQU	1
REVISION	EQU	17
DATE	MACRO
		dc.b	'19.5.95'
	ENDM
VERS	MACRO
		dc.b	'rllifts.a 1.17'
	ENDM
VSTRING	MACRO
		dc.b	'rllifts.a 1.17 (19.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rllifts.a 1.17 (19.5.95)',0
	ENDM
