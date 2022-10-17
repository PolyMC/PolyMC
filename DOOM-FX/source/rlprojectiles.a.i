VERSION		EQU	1
REVISION	EQU	25
DATE	MACRO
		dc.b	'5.6.95'
	ENDM
VERS	MACRO
		dc.b	'rlprojectiles.a 1.25'
	ENDM
VSTRING	MACRO
		dc.b	'rlprojectiles.a 1.25 (5.6.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlprojectiles.a 1.25 (5.6.95)',0
	ENDM
