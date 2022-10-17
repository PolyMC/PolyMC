VERSION		EQU	1
REVISION	EQU	43
DATE	MACRO
		dc.b	'9.6.95'
	ENDM
VERS	MACRO
		dc.b	'rltext.a 1.43'
	ENDM
VSTRING	MACRO
		dc.b	'rltext.a 1.43 (9.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rltext.a 1.43 (9.6.95)',0
	ENDM
