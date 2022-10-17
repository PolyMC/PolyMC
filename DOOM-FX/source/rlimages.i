;**********************************************************************
;*                                                                    *
;*                   P R O J E C T :   REALITY_ENGINE                 *
;*                                                                    *
;*                                    IMAGES STRUCTURE DEFINITIONS    *
;*                                                                    *
;**********************************************************************


;
;	* * * * * * *       IMAGE NUMBER TAGS MACRO       * * * * * * *
;
inMAC	MACRO
@0	equ	in
;	TEXT	"@0=%ld",in
in	var	in+@1
	TEXT	((MaxRLImages-in)>>31),"***  WARNING!  MAXRLIMAGES INVALID!   ***"
	ENDMAC

;
;	* * * * * * *       IMAGE NUMBER TAGS       * * * * * * *
;
in	var	0				; IMAGE# 0
;
;	>>>   PLAYER   <<<
;
	inMAC	inPlayerWalk,(4*3)
	inMAC	inPlayerShoot,(2*3)
	inMAC	inPlayerHit,(1*3)
	inMAC	inPlayerDie,(5*1)
;
;	>>>   SOLDIER   <<<
;
	inMAC	inSoldierWalk,4
	inMAC	inSoldierShoot,2
	inMAC	inSoldierHit,1
	inMAC	inSoldierDie,5
	inMAC	inBloodyMess,1
;
;	>>>   SERGEANT   <<<
;
	inMAC	inSergeantWalk,4
	inMAC	inSergeantShoot,2
	inMAC	inSergeantHit,1
	inMAC	inSergeantDie,5
;
;	>>>   TROOPER   <<<
;
	inMAC	inTrooperWalk,4
	inMAC	inTrooperShoot,3
	inMAC	inTrooperHit,1
	inMAC	inTrooperDie,5
	inMAC	inBloodyMess2,1
;
;	>>>   DEMON   <<<
;
	inMAC	inDemonWalk,4
	inMAC	inDemonBite,3
	inMAC	inDemonHit,1
	inMAC	inDemonDie,4
;
;	>>>   CACODEMON   <<<
;
	inMAC	inCacoDemonFloat,1
	inMAC	inCacoDemonBite,3
	inMAC	inCacoDemonHit,2
	inMAC	inCacoDemonDie,4
;
;	>>>   LOST SOUL   <<<
;
	inMAC	inLostSoulFloat,2
	inMAC	inLostSoulBite,2
	inMAC	inLostSoulHit,1
	inMAC	inLostSoulDie,3
;
;	>>>   BARON OF HELL   <<<
;
	inMAC	inBaronOfHellWalk,4
	inMAC	inBaronOfHellShoot,3
	inMAC	inBaronOfHellHit,1
	inMAC	inBaronOfHellDie,5
;
;	>>>   CYBERDEMON   <<<
;
	inMAC	inCyberDemonWalk,4
	inMAC	inCyberDemonShoot,2
	inMAC	inCyberDemonHit,1
	inMAC	inCyberDemonDie,5
;
;	>>>   SPIDERDEMON   <<<
;
	inMAC	inSpiderDemonWalk,3
	inMAC	inSpiderDemonShoot,2
	inMAC	inSpiderDemonHit,1
	inMAC	inSpiderDemonDie,4

;
;	>>>   WEAPONS   <<<
;
	inMAC	inShotGun,1
	inMAC	inChainSaw,1
	inMAC	inChainGun,1
	inMAC	inRocketLauncher,1
	inMAC	inPlasmaGun,1
	inMAC	inBFG9000,1
;
;	>>>   KEYCARDS   <<<
;
	inMAC	inRedKeyCard,1
	inMAC	inBlueKeyCard,1
	inMAC	inYellowKeyCard,1
	inMAC	inRedSkullKey,1
	inMAC	inBlueSkullKey,1
	inMAC	inYellowSkullKey,1
;
;	>>>   WEAPON AMMO   <<<
;
	inMAC	inBackPack,1
	inMAC	inClip,1
	inMAC	inAmmoBox,1
	inMAC	inShells,1
	inMAC	inShellsBox,1
	inMAC	inRocket,1
	inMAC	inRocketBox,1
	inMAC	inCell,1
	inMAC	inCellPack,1
;
;	>>>   HEALTH/ARMOR PICKUPS   <<<
;
	inMAC	inStimPak,1
	inMAC	inMedikit,1
	inMAC	inHealthBonus,4
	inMAC	inArmorBonus,4
	inMAC	inArmorGreen,1
	inMAC	inArmorBlue,1
;
;	>>>   POWERUPS   <<<
;
	inMAC	inSoulSphere,4
	inMAC	inInvulnerable,4
	inMAC	inBerserk,1
	inMAC	inInvisible,4
	inMAC	inRadiationSuit,1
	inMAC	inComputerMap,4
	inMAC	inLightGoggles,2
;
;	>>>   MISCELLANEOUS   <<<
;
	inMAC	inBarrel,2
	inMAC	inBarrelExp,3
	inMAC	inFloorLamp,1
	inMAC	inCandelabra,1
	inMAC	inColumnTechTall,1
	inMAC	inFlamingSkullRock,4
	inMAC	inTreeGray,1
	inMAC	inCandle,1
	inMAC	inFireStickTallRed,4
	inMAC	inPillarShortGreen,1
	inMAC	inShrubBrown,1
	inMAC	inSkullOnPole,1
	inMAC	inFireStickShortBlue,4
;
;	>>>   MISCELLANEOUS 2   <<<
;
	inMAC	inFireBall1,5
	inMAC	inFireBall2,5
	inMAC	inFireBall7,5
	inMAC	inMissile,4
	inMAC	inPlasma,5
;
inMax	equ	in				; MAXIMUM IMAGE NUMBER
;	TEXT	"inMAX=%ld",in
