VERSION		EQU	1
REVISION	EQU	50
DATE	MACRO
		dc.b	'9.6.95'
	ENDM
VERS	MACRO
		dc.b	'rlfloors.a 1.50'
	ENDM
VSTRING	MACRO
		dc.b	'rlfloors.a 1.50 (9.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlfloors.a 1.50 (9.6.95)',0
	ENDM
