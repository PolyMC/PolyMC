VERSION		EQU	1
REVISION	EQU	79
DATE	MACRO
		dc.b	'31.5.95'
	ENDM
VERS	MACRO
		dc.b	'rlobjects2.a 1.79'
	ENDM
VSTRING	MACRO
		dc.b	'rlobjects2.a 1.79 (31.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlobjects2.a 1.79 (31.5.95)',0
	ENDM
