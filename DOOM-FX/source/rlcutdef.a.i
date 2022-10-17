VERSION		EQU	1
REVISION	EQU	43
DATE	MACRO
		dc.b	'10.5.95'
	ENDM
VERS	MACRO
		dc.b	'rlcutdef.a 1.43'
	ENDM
VSTRING	MACRO
		dc.b	'rlcutdef.a 1.43 (10.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlcutdef.a 1.43 (10.5.95)',0
	ENDM
