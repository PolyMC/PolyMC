;**********************************************************************
;*                                                                    *
;*                   P R O J E C T :   REALITY_ENGINE                 *
;*                                                                    *
;*                                       SOUND EFFECTS DEFINITIONS    *
;*                                                                    *
;**********************************************************************


;
;	* * * * * * *       DEFINITIONS       * * * * * * *
;
MaxRLRSounds		equ	16		; Maximum RegularSound Effects
MaxRLTSounds		equ	4		; Maximum TurboSound Effects


;
;	* * * * * * *       MUSIC DRIVER SEMAPHORE LOCKS       * * * * * * *
;
mdlLock			equ	$8000		; Lock MusicDriver


;
;	* * * * * * *       SOUND EFFECTS DATA STRUCTURE       * * * * * * *
;
rlsx			var	0
rlsxVol			equ	rlsx		; Volume Threshold
rlsx			var	rlsx+1
rlsxEffect		equ	rlsx		; Effect# to play when <= Volume
rlsx			var	rlsx+1
rlsxSize		equ	rlsx


;
;	* * * * * * *       SOUND EFFECT STRUCTURE       * * * * * * *
;
rlse			var	0
rlsePriority		equ	rlse			; Priority (Volume)
rlse			var	rlse+1
rlseEffect		equ	rlse			; Sound Effect#
rlse			var	rlse+1
rlseSize		equ	rlse


;
;	* * * * * * *       REGULAR SOUND EFFECT STRUCTURE       * * * * * * *
;
rlser			var	0
rlserPriority		equ	rlser			; Priority (Volume)
rlser			var	rlser+1
rlserEffect		equ	rlser			; Sound Effect#
rlser			var	rlser+1
rlserSize		equ	rlser


;
;	* * * * * * *       TURBO SOUND EFFECT STRUCTURE       * * * * * * *
;
rlset			var	0
rlsetPriority		equ	rlset			; Priority (Volume)
rlset			var	rlset+1
rlsetEffect		equ	rlset			; Effect#
rlset			var	rlset+1
rlsetFlags		equ	rlset			; Flags
rlset			var	rlset+1
rlsetBWave		equ	rlset			; BWave#
rlset			var	rlset+1
rlsetData		equ	rlset			; Data Address
rlset			var	rlset+3
rlsetAPU		equ	rlset			; APU Address
rlset			var	rlset+2
rlsetBytes		equ	rlset			; #Bytes Remaining
rlset			var	rlset+2
rlsetAPUBase		equ	rlset			; APU BaseAddress
rlset			var	rlset+2
rlsetSize		equ	rlset
;
;	>>>   TURBO SOUND EFFECT FLAGS   <<<
;
rlsetfIdle		equ	$80			; Idle
rlsetfPlaying		equ	$40			; Playing
rlsetfCycle		equ	$3F	; MASK		; Cycle Counter ($00-$3F)


;
;	* * * * * * *       TURBO SOUND EFFECT TABLE STRUCTURE       * * * * * * *
;
rlsett			var	0
rlsettBWave		equ	rlsett			; BWave#
rlsett			var	rlsett+1
rlsettData		equ	rlsett			; Data Address
rlsett			var	rlsett+3
rlsettBytes		equ	rlsett			; #Bytes
rlsett			var	rlsett+2
rlsettSize		equ	rlsett
