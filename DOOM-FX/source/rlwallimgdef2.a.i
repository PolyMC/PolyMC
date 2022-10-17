VERSION		EQU	1
REVISION	EQU	50
DATE	MACRO
		dc.b	'23.5.95'
	ENDM
VERS	MACRO
		dc.b	'rlwallimgdef2.a 1.50'
	ENDM
VSTRING	MACRO
		dc.b	'rlwallimgdef2.a 1.50 (23.5.95)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: rlwallimgdef2.a 1.50 (23.5.95)',0
	ENDM
