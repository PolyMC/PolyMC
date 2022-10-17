VERSION		EQU	1
REVISION	EQU	92
DATE	MACRO
		dc.b	'13.6.95'
	ENDM
VERS	MACRO
		dc.b	'rlweapons3.a 1.92'
	ENDM
VSTRING	MACRO
		dc.b	'rlweapons3.a 1.92 (13.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlweapons3.a 1.92 (13.6.95)',0
	ENDM
