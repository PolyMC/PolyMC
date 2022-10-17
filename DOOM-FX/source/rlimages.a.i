VERSION		EQU	1
REVISION	EQU	68
DATE	MACRO
		dc.b	'23.5.95'
	ENDM
VERS	MACRO
		dc.b	'rlimages.a 1.68'
	ENDM
VSTRING	MACRO
		dc.b	'rlimages.a 1.68 (23.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlimages.a 1.68 (23.5.95)',0
	ENDM
