;**********************************************************************
;*                                                                    *
;*              S U P E R   N I N T E N D O   S Y S T E M             *
;*                                                                    *
;*                        MACRO DEFINITIONS FILE                      *
;*                                                                    *
;*                        Written by Randy Linden                     *
;*                                                                    *
;**********************************************************************


;
;	* * * * * * *       HALT THE CPU       * * * * * * *
;
HALT	MACRO
	ife	NOAICESE
	brk	@0
	endif
	ifn	NOAICESE
;	cop	@0
	jsl	>HALTERROR
	dc.b	@0
	endif
	ENDMAC


;
;	* * * * * * *       COLOURFLASH       * * * * * * *
;
COLOURFLASH	MACRO
	PUSHMODE
	php
	mode	'ax!'
	lda	#$0f
	sta	INIDISP
	lda	#0
	ldx	#32
.T
	stz	CGADD
	sta	CGDATA
	sta	CGDATA
	inc
	bne	.T
	dex
	bne	.T
	lda	#$80
	sta	INIDISP
	plp
	POPMODE
	ENDMAC
