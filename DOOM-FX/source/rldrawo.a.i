VERSION		EQU	1
REVISION	EQU	73
DATE	MACRO
		dc.b	'23.5.95'
	ENDM
VERS	MACRO
		dc.b	'rldrawo.a 1.73'
	ENDM
VSTRING	MACRO
		dc.b	'rldrawo.a 1.73 (23.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rldrawo.a 1.73 (23.5.95)',0
	ENDM
