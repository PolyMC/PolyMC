;**********************************************************************
;*                                                                    *
;*                   P R O J E C T :   REALITY_ENGINE                 *
;*                                                                    *
;*                                             GLOBAL DEFINITIONS     *
;*                                                                    *
;**********************************************************************


;
;	* * * * * * *       TARGET EXECUTABLE       * * * * * * *
;
ROM			equ	1		; 1=ROM Version
;
DEBUG			var	1*(1-ROM)	; 1=DEBUGGING Version
DEBUGSCR		var	1*(1-ROM)	; 1=DEBUGGING SCREEN Version
PROFILE			var	0*(1-ROM)	; 1=Enable DEVELOPMENT PROFILER
MEMTEST			var	0*(1-ROM)	; 1=Enable DEVELOPMENT MEMORY TEST
RAMBPINTEST		var	0*(1-ROM)	; 1=Enable DEVELOPMENT RAM BANK PIN TEST
RECORDDEMO		equ	0*(1-ROM)	; 1=Enable RECORDING DEMOS
IDLERESET		equ	0*(1-ROM)	; 1=Enable IDLE TIMEOUT RESET (Show Demos)


;
;	* * * * * * *       DEVELOPMENT HARDWARE       * * * * * * *
;
SFX2			equ	0		; SFX2 Hardware Interface
WIRE			equ	0		; WIRE Hardware Interface
NOAICESE		equ	1		; NOA ICE SE Hardware


;
;	* * * * * * *       DEVELOPMENT SOFTWARE       * * * * * * *
;
ACCESS			equ	1		; 1=ACCESS Development System
SASM			equ	0		; 1=SASM Development Assembler


;
;	* * * * * * *       GSU HARDWARE       * * * * * * *
;
GSUREV1			equ	0		; 1=GSU Revision 1 (StarFox 8MBit 10MHz $01)
GSUREV1A		equ	0		; 1=GSU Revision 1A (StuntRaceFX 8MBit $04)
GSUREV2			equ	1		; 1=GSU Revision 2 (StarFox2 16MBit $04)
GSUFAST			equ	1		; 1=HighSpeed GSU Operations (21MHz)


;
;	* * * * * * *       ENGINE FEATURES       * * * * * * *
;
useSKY			equ	1		; 1=Enable SKY Texture Mapping
useFLOORS		equ	0		; 1=Enable FLOOR/CEILING Texture Mapping
useHIGHDETAIL		equ	0		; 1=Enable HIGH DETAIL
useTEXTURES		equ	0		; 1=Enable WALL Texturing


;
;	* * * * * * *       ENGINE TESTING/DEBUGGING       * * * * * * *
;
useTESTMOVEXY		var	0*(DEBUG)	; 1=Enable MoveXY Testing
useTESTLEVELOBJ		var	0*(DEBUG)	; 1=Enable Level Object Testing
useTESTTARGET		var	0*(DEBUG)	; 1=Enable Targetting Testing
;
useCHECKBSPDATA		var	0*(DEBUG)	; 1=Enable BSP AreaSegList Overflow Checking
useCHECKVSEGS		var	0*(DEBUG)	; 1=Enable VisibleSegmentList Overflow Checking
useCHECKWALLPLOTDATA	var	0*(DEBUG)	; 1=Enable WallPlotData Pixel Range Checking
useCHECKFLOORPLOTDATA	var	0*(DEBUG)	; 1=Enable FloorPlotData Pixel Range Checking
useCHECKOBJPLOTDATA	var	0*(DEBUG)	; 1=Enable ObjPlotData Pixel Range Checking
;
useCHECKSECTOROBJECTS	var	0*(DEBUG)	; 1=Enable Checking Sector Object Lists
;
useCHECKSOUND		var	0*(DEBUG)	; 1=Enable Checking SoundCommands
