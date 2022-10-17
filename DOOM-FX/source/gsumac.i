;**********************************************************************
;*                                                                    *
;*              S U P E R   N I N T E N D O   S Y S T E M             *
;*                                                                    *
;*                      GSU MACRO DEFINITIONS FILE                    *
;*                                                                    *
;*                        Written by Randy Linden                     *
;*                                                                    *
;**********************************************************************


	public	CACHES:
	public	CACHED:

;
;	* * * * * * *       HALT THE GSU       * * * * * * *
;
HALTGSU	MACRO
	nop
	iwt	r0,@0
	move	(<GSUERROR),r0
	stop
	nop
	ENDMAC


;
;	* * * * * * *       RESET CACHE BLOCK TRACKING       * * * * * * *
;
CACHER	MACRO
CACHED:	equ	0			; CACHE BLOCK DEBUGGING FLAG
CACHES:	equ	1			; CACHE BLOCK CLOSED
	ENDMAC

;
;	* * * * * * *       START A CACHE BLOCK       * * * * * * *
;
CACHEB	MACRO
	cache
	LTEXT	(1-CACHES:),"Cache already open <@0>"
CACHES:	equ	0
CACHEB:
	LTEXT	(CACHEB:&$0f),"Cache MisAligned in <@0>"
	ENDMAC

;
;	* * * * * * *       END A CACHE BLOCK       * * * * * * *
;
CACHEE	MACRO
	LTEXT	(CACHES:),"Cache not open <@0>"
CACHES:	equ	1
CACHEE:
CACHET:	equ	(1-(((CACHEE:-CACHEB:)-512)>>31))
	LTEXT	(CACHET:),"Cache OverFlow in <@0> is $%08lx",(CACHEE:-CACHEB:)
	LTEXT	((1-CACHET:)*CACHED:),"Cache at <@0> is $%08lx",(*-CACHEB:)
	ENDMAC

;
;	* * * * * * *       SHOW THE POSITION OF A CACHE BLOCK       * * * * * * *
;
CACHEP	MACRO
	LTEXT	(CACHES:),"Cache not open <@0>"
	LTEXT	(1-CACHES:),"Cache at <@0> is $%08lx",(*-CACHEB:)
	ENDMAC
