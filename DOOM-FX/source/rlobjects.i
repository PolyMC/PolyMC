;**********************************************************************
;*                                                                    *
;*                   P R O J E C T :   REALITY_ENGINE                 *
;*                                                                    *
;*                                   OBJECTS STRUCTURE DEFINITIONS    *
;*                                                                    *
;**********************************************************************


;
;	* * * * * * *       OBJECT DATA STRUCTURE       * * * * * * *
;
rlod			var	0
rlodFlags		equ	rlod		; Flags
rlod			var	rlod+2
rlodDiameter		equ	rlod		; Diameter
rlod			var	rlod+1
rlodHeight		equ	rlod		; Height
rlod			var	rlod+1
rlodImage		equ	rlod		; Image#
rlod			var	rlod+1
rlodSize		equ	rlod
;
;	>>>   OBJECT DATA FLAGS   <<<
;
rlodfKill		equ	$0002		; Object counts as "KILL"
rlodfFloat		equ	$0004		; Object FLOATS
rlodfProjectile		equ	$0008		; Object is PROJECTILE
rlodfItem		equ	$0010		; Object can be PICKED UP
rlodfItemCount		equ	$0020		; Object counts in ITEM TOTALS
rlodfAnim2		equ	$0100		; ANIMATED 2-Frame
rlodfAnim4		equ	$0200		; ANIMATED 4-Frame
rlodfImage		equ	$0400		; RLObject has IMAGE#
rlodfGlow		equ	$0800		; Object GLOWS
rlodfShine		equ	$1000		; Object SHINES


;
;	* * * * * * *       MOVABLE OBJECT DATA STRUCTURE       * * * * * * *
;
rlmod			var	0
rlmodHealth		equ	rlmod		; Health
rlmod			var	rlmod+2
rlmodMoveVelocities	equ	rlmod		; MoveVelocities Table
rlmod			var	rlmod+2
rlmodSleepS		equ	rlmod		; Sleep State
rlmodBirthS		equ	rlmod		; Birth State
rlmod			var	rlmod+2
rlmodSleepA		equ	rlmod		; Sleep Animation
rlmodBirthA		equ	rlmod		; Birth Animation
rlmod			var	rlmod+2
rlmodDamageChance	equ	rlmod		; Chance of Damage interrupting State
rlmod			var	rlmod+1
rlmodDamageS		equ	rlmod		; Damage State
rlmod			var	rlmod+2
rlmodAttackCloseDamage	equ	rlmod		; Attack Close Damage/Multiplier
rlmod			var	rlmod+2
rlmodAttackFarDamage	equ	rlmod		; Attack Far Damage/Multiplier
rlmod			var	rlmod+2
rlmodAwakeSound		equ	rlmod		; Awake Sound Effect
rlmodBirthSound		equ	rlmod		; Birth Sound Effect
rlmod			var	rlmod+2
rlmodAttackCloseSound	equ	rlmod		; Attack Close Sound Effect
rlmod			var	rlmod+2
rlmodAttackFarSound	equ	rlmod		; Attack Far Sound Effect
rlmod			var	rlmod+2
rlmodDamageSound	equ	rlmod		; Damage Sound Effect
rlmod			var	rlmod+2
rlmodDieSound		equ	rlmod		; Die Sound Effect
rlmod			var	rlmod+2
rlmodBirthFlags		equ	rlmod		; Birth Flags
rlmod			var	rlmod+1
rlmodBirthType		equ	rlmod		; Birth Type
rlmod			var	rlmod+1
rlmodSize		equ	rlmod


;
;	* * * * * * *       OBJECT TYPE TAGS MACRO       * * * * * * *
;
otMAC	MACRO
@0	equ	ot
;	TEXT	"@0=%ld",ot
ot	var	ot+1
	ENDMAC

;
;	* * * * * * *       OBJECT TYPE TAGS       * * * * * * *
;
ot	var	0				; OBJECT# 0
	otMAC	otPlayer1Start
	otMAC	otPlayer2Start
	otMAC	otPlayer3Start
	otMAC	otPlayer4Start
	otMAC	otDeathMatchStart
	otMAC	otTeleportSpot
otAngular	equ	ot-1			; LAST ANGULAR OBJECT#
;
	otMAC	otSoldier
	otMAC	otSergeant
	otMAC	otTrooper
	otMAC	otDemon
	otMAC	otCacoDemon
	otMAC	otLostSoul
	otMAC	otBaronOfHell
	otMAC	otCyberDemon
	otMAC	otSpiderDemon
otMObj	equ	ot-1				; LAST MOVABLE OBJECT#
;
;	>>>   WEAPONS   <<<
;
otIObj	equ	ot				; FIRST ITEM OBJECT#
	otMAC	otShotGun
	otMAC	otChainSaw
	otMAC	otChainGun
	otMAC	otRocketLauncher
	otMAC	otPlasmaGun
	otMAC	otBFG9000
;
;	>>>   KEYCARDS   <<<
;
	otMAC	otRedKeyCard
	otMAC	otBlueKeyCard
	otMAC	otYellowKeyCard
	otMAC	otRedSkullKey
	otMAC	otBlueSkullKey
	otMAC	otYellowSkullKey
;
;	>>>   WEAPON AMMO   <<<
;
	otMAC	otBackPack
	otMAC	otClip
	otMAC	otAmmoBox
	otMAC	otShells
	otMAC	otShellsBox
	otMAC	otRocket
	otMAC	otRocketBox
	otMAC	otCell
	otMAC	otCellPack
;
;	>>>   HEALTH/ARMOR PICKUPS   <<<
;
	otMAC	otStimPak
	otMAC	otMedikit
	otMAC	otHealthBonus
	otMAC	otArmorBonus
	otMAC	otArmorGreen
	otMAC	otArmorBlue
;
;	>>>   POWERUPS   <<<
;
	otMAC	otSoulSphere
	otMAC	otInvulnerable
	otMAC	otBerserk
	otMAC	otInvisible
	otMAC	otRadiationSuit
	otMAC	otComputerMap
	otMAC	otLightGoggles
;
;	>>>   MISCELLANEOUS   <<<
;
	otMAC	otBarrel
	otMAC	otFloorLamp
	otMAC	otBloodyMess
	otMAC	otDeadTrooper
	otMAC	otDeadDemon
	otMAC	otSkullOnPole
	otMAC	otPillarShortGreen
	otMAC	otCandle
	otMAC	otCandelabra
	otMAC	otFlamingSkullRock
	otMAC	otTreeGray
	otMAC	otFireStickTallRed
	otMAC	otShrubBrown
	otMAC	otColumnTechTall
	otMAC	otFireStickShortBlue
;
;	>>>   PROJECTILES   <<<
;
otPObj	equ	ot				; FIRST PROJECTILE OBJECT#
	otMAC	otFireBall1
	otMAC	otFireBall2
	otMAC	otFireBall7
	otMAC	otMissile
	otMAC	otPlasma
;
;	>>>   MISCELLANEOUS 2   <<<
;
	otMAC	otBarrelExp
;
otMax	equ	ot				; MAXIMUM OBJECT TYPE
;	TEXT	"otMAX=%ld",ot


;
;	* * * * * * *       OBJECT DIMENSIONS       * * * * * * *
;
;	>>>   HEIGHT   <<<
;
odhPlayer		equ	56		; Player
odhPlayerEye		equ	42		; View Eye
;
odhSoldier		equ	56		; Enemies
odhSergeant		equ	56
odhTrooper		equ	56
odhDemon		equ	56
odhCacoDemon		equ	56
odhLostSoul		equ	56
odhBaronOfHell		equ	56
odhCyberDemon		equ	96
odhSpiderDemon		equ	88
;
odhPickUp		equ	16		; Pickup Items
odhObstacle		equ	16		; Obstacles
;
odhFireBall1		equ	16		; FireBall1
odhFireBall2		equ	16		; FireBall2
odhFireBall7		equ	16		; FireBall7
odhMissile		equ	14		; Missile
odhPlasma		equ	24		; Plasma
;
odhTarget		equ	56		; Height of Objects for Targetting


;
;	>>>   DIAMETER   <<<
;
oddPlayer		equ	32		; Player
;
oddSoldier		equ	40		; Enemies
oddSergeant		equ	40
oddTrooper		equ	40
oddDemon		equ	60
oddCacoDemon		equ	62
oddLostSoul		equ	32
oddBaronOfHell		equ	48
oddCyberDemon		equ	70
oddSpiderDemon		equ	192
;
oddPickUp		equ	64 ; 40		; Pickup Items
oddObstacle		equ	48 ; 32		; Obstacles
;
oddFireBall1		equ	16		; FireBall1 (Red)
oddFireBall2		equ	16		; FireBall2 (Purple)
oddFireBall7		equ	18		; FireBall7 (Green)
oddMissile		equ	14		; Missile
oddPlasma		equ	24		; Plasma


;
;	>>>   RADIUS   <<<
;
odrPlayer		equ	(oddPlayer/2)	; Player Radius
odrMoveXY		equ	odrPlayer	; Player<->Line Movement Collision Radius
odrMoveLineXY		equ	odrPlayer	; Line<->Player Movement Collision Radius
odrUseXY		equ	(odrPlayer*3)	; Use/Operate Intersection Radius
