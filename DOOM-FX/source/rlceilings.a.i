VERSION		EQU	1
REVISION	EQU	55
DATE	MACRO
		dc.b	'9.6.95'
	ENDM
VERS	MACRO
		dc.b	'rlceilings.a 1.55'
	ENDM
VSTRING	MACRO
		dc.b	'rlceilings.a 1.55 (9.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlceilings.a 1.55 (9.6.95)',0
	ENDM
