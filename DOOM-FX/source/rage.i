;**********************************************************************
;*                                                                    *
;*                       P R O J E C T :   R A G E                    *
;*                                                                    *
;*                                             GLOBAL DEFINITIONS     *
;*                                                                    *
;**********************************************************************

	globals	on

	include	rlinc.i


;
;	* * * * * * *       GAME FEATURES       * * * * * * *
;
useSOUND		equ	1		; 1=Enable SOUND/MUSIC
useAUTOMAP		equ	1		; 1=Enable AUTOMAP
useAUTOMAPROT		equ	1		; 1=Enable AUTOMAP Rotation
useXBAND		equ	0		; 1=Enable XBAND Hardware
;
usePATCH		equ	1		; 1=Enable PATCHES
;
useWILLIAMS		equ	1		; 1=Enable WILLIAMS Changes
useID			equ	1		; 1=Enable ID Changes
useID2			equ	1		; 1=Enable ID Changes2
useID3			equ	1		; 1=Enable ID Changes3
useID4			equ	1		; 1=Enable ID Changes4
useID7			equ	1		; 1=Enable ID Changes7
useID8			equ	1		; 1=Enable ID Changes8
;
useMULTIPLAYER		equ	1		; 1=Enable MULTIPLAYER
useSCOPE		equ	0		; 1=Enable SUPERSCOPE
useCHEATS		equ	0		; 1=Enable CHEAT Codes
;
usePAL			equ	0		; 1=Enable PAL Version
useOCEAN		equ	0		; 1=Enable OCEAN Version
useIMAGINEER		equ	0		; 1=Enable IMAGINEER Version


;
;	* * * * * * *       FLUFF SCREENS       * * * * * * *
;
useSYSINFO		equ	0		; 1=Enable SystemInfo Sequence
useLOGO			equ	0		; 1=Enable Logo Sequence
useLOGO2		equ	1 ; 1		; 1=Enable Logo2 Sequence
useSCULPT		equ	0		; 1=Enable Sculptured Sequence
useTITLE		equ	1 ; 1		; 1=Enable Title Sequence
useSCORE		equ	1 ; 1		; 1=Enable Score Sequence
useLEGAL		equ	1 ; 1		; 1=Enable Legal Sequence
;
useDEMO1		equ	0		; 1=Enable Demo1 Sequence
useDEMO			equ	useDEMO1


;
;	* * * * * * *       LEVELS       * * * * * * *
;
useE1M1ONLY		var	0		; 1=Enable E1M1 ONLY
useE1M1			var	1		; 1=Enable E1M1 "Hangar"
useE1M2			var	1		; 1=Enable E1M2 "Nuclear Plant"
useE1M3			var	1		; 1=Enable E1M3 "Toxin Refinery"
useE1M4			var	1		; 1=Enable E1M4 "Command Control"
useE1M5			var	1		; 1=Enable E1M5 "Phobos Lab"
useE1M6			var	0		; 1=Enable E1M6 "Central Processing"
useE1M7			var	1		; 1=Enable E1M7 "Computer Station"
useE1M8			var	1		; 1=Enable E1M8 "Phobos Anomaly"
useE1M9			var	1		; 1=Enable E1M9 "Military Base"
;
useE2M1			var	1		; 1=Enable E2M1 "Deimos Anomaly"
useE2M2			var	0		; 1=Enable E2M2 "Containment Area"
useE2M3			var	1		; 1=Enable E2M3 "Refinery"
useE2M4			var	1		; 1=Enable E2M4 "Deimos Lab"
useE2M5			var	0		; 1=Enable E2M5 "Command Center"
useE2M6			var	1		; 1=Enable E2M6 "Halls of the Damned"
useE2M7			var	0		; 1=Enable E2M7 "Spawning Vats"
useE2M8			var	1		; 1=Enable E2M8 "Tower of Babel"
useE2M9			var	1		; 1=Enable E2M9 "Fortress of Mystery"
;
useE3M1			var	1		; 1=Enable E3M1 "Hell Keep"
useE3M2			var	1		; 1=Enable E3M2 "Slough of Despair"
useE3M3			var	1		; 1=Enable E3M3 "Pandemonium"
useE3M4			var	1		; 1=Enable E3M4 "House of Pain"
useE3M5			var	0		; 1=Enable E3M5 "Unholy Cathedral"
useE3M6			var	1		; 1=Enable E3M6 "Mt. Erebus"
useE3M7			var	1 ; 1		; 1=Enable E3M7 "Limbo"
useE3M8			var	1 ; 1		; 1=Enable E3M8 "Dis"
useE3M9			var	1 ; 1		; 1=Enable E3M9 "Warrens"
;
useE1M2			var	useE1M2*(1-useE1M1ONLY))
useE1M3			var	useE1M3*(1-useE1M1ONLY))
useE1M4			var	useE1M4*(1-useE1M1ONLY))
useE1M5			var	useE1M5*(1-useE1M1ONLY))
useE1M6			var	useE1M6*(1-useE1M1ONLY))
useE1M7			var	useE1M7*(1-useE1M1ONLY))
useE1M8			var	useE1M8*(1-useE1M1ONLY))
useE1M9			var	useE1M9*(1-useE1M1ONLY))
;
useE2M1			var	useE2M1*(1-useE1M1ONLY))
useE2M2			var	useE2M2*(1-useE1M1ONLY))
useE2M3			var	useE2M3*(1-useE1M1ONLY))
useE2M4			var	useE2M4*(1-useE1M1ONLY))
useE2M5			var	useE2M5*(1-useE1M1ONLY))
useE2M6			var	useE2M6*(1-useE1M1ONLY))
useE2M7			var	useE2M7*(1-useE1M1ONLY))
useE2M8			var	useE2M8*(1-useE1M1ONLY))
useE2M9			var	useE2M9*(1-useE1M1ONLY))
;
useE3M1			var	useE3M1*(1-useE1M1ONLY))
useE3M2			var	useE3M2*(1-useE1M1ONLY))
useE3M3			var	useE3M3*(1-useE1M1ONLY))
useE3M4			var	useE3M4*(1-useE1M1ONLY))
useE3M5			var	useE3M5*(1-useE1M1ONLY))
useE3M6			var	useE3M6*(1-useE1M1ONLY))
useE3M7			var	useE3M7*(1-useE1M1ONLY))
useE3M8			var	useE3M8*(1-useE1M1ONLY))
useE3M9			var	useE3M9*(1-useE1M1ONLY))
