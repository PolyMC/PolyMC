;**********************************************************************
;*                                                                    *
;*                   P R O J E C T :   REALITY_ENGINE                 *
;*                                                                    *
;*                                   WEAPONS STRUCTURE DEFINITIONS    *
;*                                                                    *
;**********************************************************************


;
;	* * * * * * *       DEFINITIONS       * * * * * * *
;
WeaponSwayVBlanks	equ	64		; WeaponSway Cycle #VBlanks (Power of 2!)


;
;	* * * * * * *       WEAPONS TYPE TAGS       * * * * * * *
;
wtFist			equ	0		; Fist
wtPistol		equ	2		; Pistol
wtShotGun		equ	4		; ShotGun
wtChainSaw		equ	6		; ChainSaw
wtChainGun		equ	8		; ChainGun
wtRocket		equ	10		; Rocket Launcher
wtPlasma		equ	12		; Plasma Rifle
wtBFG			equ	14		; BFG9000
;
;	* * * * * * *       WEAPONS TYPE BITS       * * * * * * *
;
wtbFist			equ	$01		; Fist
wtbPistol		equ	$02		; Pistol
wtbShotGun		equ	$04		; ShotGun
wtbChainSaw		equ	$08		; ChainSaw
wtbChainGun		equ	$10		; ChainGun
wtbRocket		equ	$20		; Rocket Launcher
wtbPlasma		equ	$40		; Plasma Rifle
wtbBFG			equ	$80		; BFG9000


;
;	* * * * * * *       WEAPONS DATA STRUCTURE       * * * * * * *
;
wd			var	0
wdRaise			equ	wd		; Raise Weapon Address
wd			var	wd+2
wdLower			equ	wd		; Lower Weapon Address
wd			var	wd+2
wdReady			equ	wd		; Ready Weapon Address
wd			var	wd+2
wdUse			equ	wd		; Use Weapon Address
wd			var	wd+2
wdAmmo			equ	wd		; Ammunition Address
wd			var	wd+2
wdSize			equ	wd


;
;	* * * * * * *       WEAPONS STATE STRUCTURE       * * * * * * *
;
ws			var	0
wsCode			equ	ws		; CodeAddress in this State
ws			var	ws+2
wsCount			equ	ws		; #Cycles in this State
ws			var	ws+1
wsFrame			equ	ws		; Frame#
ws			var	ws+1
wsNextState		equ	ws		; Address of Next State
ws			var	ws+2
wsSize			equ	ws


;
;	* * * * * * *       WEAPON FRAME NUMBERS       * * * * * * *
;
;	>>>   FIST   <<<
;
wfPUNGA0		equ	0
wfPUNFA0		equ	4
wfPUNFB0		equ	8
wfPUNFC0		equ	12

;
;	>>>   PISTOL   <<<
;
wfPISGA0		equ	0
wfPISFA0		equ	4
wfPISFB0		equ	8

;
;	>>>   SHOTGUN   <<<
;
wfSHTGA0		equ	0
wfSHTFA0		equ	4
wfSHTFB0		equ	8
wfSHTFC0		equ	12
wfSHTFD0		equ	16
wfSHTFE0		equ	20

;
;	>>>   CHAINSAW   <<<
;
wfSAWGA0		equ	0
wfSAWGB0		equ	4
wfSAWFA0		equ	8
wfSAWFB0		equ	12

;
;	>>>   CHAINGUN   <<<
;
wfCHGGA0		equ	0
wfCHGFA0		equ	4
wfCHGFB0		equ	8

;
;	>>>   ROCKET LAUNCHER   <<<
;
wfMISGA0		equ	0
wfMISFA0		equ	4
wfMISFB0		equ	8
wfMISFC0		equ	12
wfMISFD0		equ	16
wfMISFE0		equ	20

;
;	>>>   PLASMA RIFLE   <<<
;
wfPLSGA0		equ	0
wfPLSFA0		equ	4
wfPLSFB0		equ	8
wfPLSFC0		equ	12

;
;	>>>   BFG9000   <<<
;
wfBFGGA0		equ	0
wfBFGFA0		equ	4
wfBFGFB0		equ	8
