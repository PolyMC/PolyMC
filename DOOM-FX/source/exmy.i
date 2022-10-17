;**********************************************************************
;*                                                                    *
;*                       P R O J E C T :   R A G E                    *
;*                                                                    *
;*                   EPISODE x, MISSION y MACRO MODULE                *
;*                                                                    *
;**********************************************************************



EXMYMAC	MACRO

;
;	* * * * * * *       BSP, AREAS, VERTEXES       * * * * * * *
;	* * * * * * *       LINES, SEGS, REJECT        * * * * * * *
;	* * * * * * *       FACES                      * * * * * * *
;
	SECTION	@0_A,RELOC,BASE=$80000000,RORG=$400000
;
;	>>>   BSP TREE   <<<
;
@0BSP
	ifn	use@0
	image	RLLEV:@0/BSP
	endif
@0BSPE
;
;	>>>   AREAS   <<<
;
@0AREAS
	ifn	use@0
	image	RLLEV:@0/AREAS
	endif
;
;	>>>   VERTEXES   <<<
;
@0VERTEXES
	ifn	use@0
	image	RLLEV:@0/VERTEXES
	endif
@0VERTEXESE
;
;	>>>   LINES   <<<
;
@0LINES
	ifn	use@0
	image	RLLEV:@0/LINES
	endif
@0LINESE
;
;	>>>   SEGMENTS   <<<
;
@0SEGS
	ifn	use@0
	image	RLLEV:@0/SEGS
	endif
@0SEGSE
;
;	>>>   REJECT   <<<
;
@0REJECT
	ifn	use@0
	image	RLLEV:@0/REJECT
	endif
;
;	>>>   FACES   <<<
;
@0FACES
	ifn	use@0
	image	RLLEV:@0/FACES
	endif


;
;	* * * * * * *       SECTORS, OBJECTS, BLOCKMAP       * * * * * * *
;	* * * * * * *       DOORS, FLOORS, CEILINGS          * * * * * * *
;	* * * * * * *       STAIRS, LIFTS                    * * * * * * *
;
	SECTION	@0_B,RELOC,BASE=$80000000,RORG=$400000
;
;	>>>   SECTORS   <<<
;
@0SECTORS
	ifn	use@0
	image	RLLEV:@0/SECTORS
	endif
@0SECTORSE
	ifn	use@0
	dc.w	$524C
	endif
;
;	>>>   OBJECTS   <<<
;
@0OBJECTS
	ifn	use@0

	ifn	useTESTTARGET
	dc.b	$c7,$00				; PLAYER1START
	dc.b	$20,$04,$e0,$f1,$00,$40
	dc.b	$c7,$06				; CACODEMON
	dc.b	$20,$04,$e0,$f2,$00,$c0
	endif

	ifn	useTESTLEVELOBJ
;	dc.b	$c7,$00				; PLAYER1START
;	dc.b	$0e,$03,$99,$f2,$00,$80		; E1M1 (Geometry)
	dc.b	$c7,$00				; PLAYER1START
	dc.b	$00,$f8,$30,$04,$00,$40		; E1M2 (ChainSaw)
	image	RLLEV:@0/OBJECTS,8
	endif

	ife	(useTESTLEVELOBJ|useTESTTARGET)
	image	RLLEV:@0/OBJECTS
	endif

	dc.w	0
	endif
;
;	>>>   BLOCKMAP   <<<
;
@0BLOCKMAP
	ifn	use@0
	image	RLLEV:@0/BLOCKMAP
	endif
;
;	>>>   DOORS   <<<
;
@0DOORS
	ifn	use@0
	image	RLLEV:@0/DOORS
	dc.b	$FF
	endif
;
;	>>>   STAIRS   <<<
;
@0STAIRS
	ifn	use@0
	image	RLLEV:@0/STAIRS
	endif
;
;	>>>   FLOORS   <<<
;
@0FLOORS
	ifn	use@0
	image	RLLEV:@0/FLOORS
	endif
;
;	>>>   CEILINGS   <<<
;
@0CEILINGS
	ifn	use@0
	image	RLLEV:@0/CEILINGS
	endif
;
;	>>>   LIFTS   <<<
;
@0LIFTS
	ifn	use@0
	image	RLLEV:@0/LIFTS
	endif


	ENDMAC
