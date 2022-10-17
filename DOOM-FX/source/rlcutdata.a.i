VERSION		EQU	1
REVISION	EQU	44
DATE	MACRO
		dc.b	'10.5.95'
	ENDM
VERS	MACRO
		dc.b	'rlcutdata.a 1.44'
	ENDM
VSTRING	MACRO
		dc.b	'rlcutdata.a 1.44 (10.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlcutdata.a 1.44 (10.5.95)',0
	ENDM
