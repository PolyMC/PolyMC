VERSION		EQU	1
REVISION	EQU	16
DATE	MACRO
		dc.b	'31.5.95'
	ENDM
VERS	MACRO
		dc.b	'rlradius.a 1.16'
	ENDM
VSTRING	MACRO
		dc.b	'rlradius.a 1.16 (31.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlradius.a 1.16 (31.5.95)',0
	ENDM
