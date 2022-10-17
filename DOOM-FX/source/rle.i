;**********************************************************************
;*                                                                    *
;*                   P R O J E C T :   REALITY_ENGINE                 *
;*                                                                    *
;*                                   ENGINE STRUCTURE DEFINITIONS     *
;*                                                                    *
;**********************************************************************


;
;	* * * * * * *       DEFINITIONS       * * * * * * *
;
RLCharX			equ	27			; #Characters Wide
RLCharY			equ	22			; #Characters Tall
RLPixX			equ	RLCharX*8		; #Pixels Wide
RLPixY			equ	RLCharY*8		; #Pixels Tall
;
RLStatStartPixX		equ	((256-RLPixX)/2)
RLStatStartPixY		equ	RLVIntTop+(RLPixY-RLStatPixY)
RLStatCharX		equ	27			; #Characters Wide
RLStatCharY		equ	4			; #Characters Tall
RLStatPixX		equ	RLStatCharX*8
RLStatPixY		equ	RLStatCharY*8
RLStatEndPixY		equ	RLStatStartPixY+RLStatPixY
;
RLViewCharX		equ	27			; #Characters Wide
RLViewCharY		equ	18			; #Characters Tall
RLViewPixX		equ	RLViewCharX*8		; #Pixels Wide
RLViewPixY		equ	RLViewCharY*8		; #Pixels Tall
;
RLVIntTop		equ	((224-RLPixY)/2)-1	; Top Interrupt ScanLine
RLVIntBottom		equ	224-((224-RLPixY)/2)-1	; Bottom Interrupt ScanLine
;
;	HInt = ( BorderRightStart - (TotalCycles To On/Off Display*2) - (IRQTime*2)+ Fudge )
;	       ( (256-((256-(RLPixX*8))/2)) - (50*2) - (8*2) + 10)
;
RLHIntL			equ	96			; Left Interrupt HPos
RLHIntR			equ	248 ; 242		; Right Interrupt HPos


;
;	* * * * * * *       ENGINE MAXIMUMS       * * * * * * *
;
MaxRLVertexes		equ	1056	; E2M4		; *Maximum VERTEXES per Level
MaxRLSectors		equ	205	; E2M4		; *Maximum SECTORS per Level
;MaxRLAreas		equ	512			; Maximum AREA Nodes per Level
;MaxRLSegs		equ	2438			; Maximum SEGS per Level
MaxRLLines		equ	1118	; E2M4 (1117)	; *Maximum LINES per Level
;MaxRLFaces		equ	1440			; Maximum FACES per Level
;
MaxRLTextures		equ	256			; Maximum TEXTURES
;
MaxRLFObjects		equ	180 ; 160		; *Maximum FIXED OBJECTS per Level
MaxRLMObjects		equ	100			; *Maximum MOVABLE OBJECTS per Level
MaxRLObjectTypes	equ	256			; Maximum OBJECT Types
MaxRLImages		equ	256			; Maximum IMAGES
;
RLScreenPlane		equ	4			; Distance from Eye to ScreenPlane
RLAspectRatio		equ	$00014000		; AspectRatio (%Increase in Y)
RLMaxViewDistance	equ	7168			; Maximum Viewing Distance
	ife	useHIGHDETAIL
MaxRLClipZones		equ	(RLViewPixX/2/(2+1))	; Maximum CLIPZONES (2SOLID+1BLANK)
	endif
	ifn	useHIGHDETAIL
MaxRLClipZones		equ	(RLViewPixX/(2+1))	; Maximum CLIPZONES (2SOLID+1BLANK)
	endif
;
MaxRLVSegs		equ	168 ; 160		; Maximum Visible Segments
MaxRLVFloors		equ	40 ; 32			; Maximum Visible Floors/Ceilings
;MaxRLVAreas		equ	256			; Maximum Visible Areas
MaxRLVSectors		equ	(64-1)			; Maximum Visible Sectors
MaxRLVObjs		equ	28 ; 32			; Maximum Visible Objects
;
MaxRLTasks		equ	48 ; 64			; Maximum TASKS
;
;MaxRLMoveLines		equ	(64-1)			; Maximum Lines Checked when Moving
MaxRLUseLines		equ	(64-1)			; Maximum Lines Checked to Use/Operate
;
MaxRLMPlats		equ	48 ; 64			; Maximum Movable Platforms
MaxRLToggles		equ	12			; Maximum Toggles
;
MaxRLSounds		equ	8			; Maximum Sound Effects
;
;MaxRLEnemies		equ	MaxRLMObjects		; Maximum Enemies
MaxRLTargetEnemies	equ	24			; Maximum Targetting Enemies
MaxRLMoveEnemies	equ	24			; Maximum Moving Enemies
MaxRLAttackEnemies	equ	20			; Maximum Attacking Enemies
MaxRLBirthEnemies	equ	MaxRLAttackEnemies	; Maximum Birthing Enemies
MaxRLMoveProjectiles	equ	24			; Maximum Moving Projectiles
MaxRLRadiusDamages	equ	14			; Maximum Radius Damages

RLMaxCutSprOAMs		equ	(128-(3+8+12+3+3+3))		; Max CutSpriteOAMs (98)
;RLMaxCutSprCHRs	equ	(512-((vmWEAPONDEF&$1fff)>>4))	; Max CutSpriteCHRs (168)
RLMaxCutSprCHRs		equ	RLMaxCutSprOAMs
;
RLMaxCharDefXFer	equ	1024			; Maximum .CHARDEF Bytes per XFer


;
;	* * * * * * *       VRAM MEMORY MAP       * * * * * * *
;
vm			var	0
vmCHR1			equ	vm
vmCHRA			equ	vm				; CHAR Set "A"
vm			var	vm+(RLCharX/3)*(RLViewCharY)*(64/2)
vmCHRB			equ	vm				; CHAR Set "B"
vm			var	vm+(RLCharX/3)*(RLViewCharY)*(64/2)
vmCHRC			equ	vm				; CHAR Set "C" (COMMON)
vm			var	vm+(RLCharX/3)*(RLViewCharY)*(64/2)
vmCHRD			equ	vm				; CHAR Set "D" (SWAP A)
vm			var	vm+(RLCharX/3)*(RLViewCharY)*(64/2)
vmCHRE			equ	vm				; CHAR Set "E" (SWAP B)
vm			var	vm+(RLCharX/3)*(RLViewCharY)*(64/2)
;
vmCHR2			equ	((vm>>12)<<12)			; CHR2 Base ($6000)
vmSPR			equ	((vm>>13)<<13)			; SPRITES Base ($6000)
;
vmSTATDEF		equ	vm				; CHAR Set "STATUS" ($6540)
vm			var	vm+(RLCharX*RLStatCharY)*(32/2)
;
vmFACEDEF		equ	vm+((00+00)*32/2)		; CHAR Set "FACE" ($6c00)
;
vmNUMDEF		equ	vm+((00+12)*32/2)		; CHAR Set "NUMBERS" ($6cc0)
;
vmARMSDEF		equ	vm+((16+00)*32/2)		; CHAR Set "ARMS" ($6d00)
;
vmKEYSDEF		equ	vm+((16+08)*32/2)		; CHAR Set "KEYS" ($6d80)
;
vmCHR2B			equ	vm+((16+11)*32/2)		; CHAR Set "BLANK" ($6db0)
;
vm			var	vm+((16*4)*32/2)
;
vmMAP1			equ	vm				; MAP for VIEW0/1 ($7000)
vm			var	vm+(RLCharY*32*2/2)
vmMAP2			equ	vm-(4*32*2/2)			; MAP for STATUS ($7240)
vm			var	vm+((RLCharY-4)*32*2/2)
;
vmWEAPONDEF		equ	vm				; CHAR Set "WEAPON" ($7500)
vm			var	vm+(RLMaxCutSprCHRs*(32/2))
;
vmCHR2C			equ	vm				; CHAR Set "DEBUG" ($7b30)


;
;	* * * * * * *       REALITY_ENGINE FLAGS       * * * * * * *
;
rlfPhasePending		equ	$8000			; Phase Ended, Next Phase Pending
rlfHalt			equ	$4000			; RLEngine Halted
;rlfPause		equ	$2000			; RLEngine Pause
rlfDraw			equ	$0001			; Draw Built
rlfStatus		equ	$0002			; Status Built
rlfWeaponOAM		equ	$0004			; Weapon OAM Built
rlfWeaponDEF		equ	$0008			; Weapon DEF+OAM Built
rlfDebugSCR		equ	$0100			; Debugging Screen Built
rlfDebug		equ	$0200			; Debugging Screen Update
rlfExchange		equ	$1000			; Packets Exchanged


;
;	* * * * * * *       RGB PALETTE ANIMATION TAGS       * * * * * * *
;
rgbNone			equ	-1			; No RGB Change
rgbNormal		equ	0			; Normal RGB
rgbRad			equ	2			; Radiation Suit
rgbBerserk		equ	4			; Berserk PowerUp
rgbPick			equ	6			; PickUp
rgbHit			equ	8			; Player Hit


;
;	* * * * * * *       BSP TREE NODE STRUCTURE       * * * * * * *
;
rlb			var	0
rlbLineY		equ	rlb			; Partition Line Y
rlb			var	rlb+2
rlbDeltaX		equ	rlb			; Partition Line Delta X
rlb			var	rlb+2
rlbLineX		equ	rlb			; Partition Line X
rlb			var	rlb+2
rlbDeltaY		equ	rlb			; Partition Line Delta Y
rlb			var	rlb+2
rlbLeftYMax		equ	rlb			; Left BoundaryBox Y Maximum
rlb			var	rlb+2
rlbLeftYMin		equ	rlb			; Left BoundaryBox Y Minimum
rlb			var	rlb+2
rlbLeftXMin		equ	rlb			; Left BoundaryBox X Minimum
rlb			var	rlb+2
rlbLeftXMax		equ	rlb			; Left BoundaryBox X Maximum
rlb			var	rlb+2
rlbLeftChild		equ	rlb			; Left Child, >=$8000 = AREA
rlb			var	rlb+2
rlbRightYMax		equ	rlb			; Right BoundaryBox Y Maximum
rlb			var	rlb+2
rlbRightYMin		equ	rlb			; Right BoundaryBox Y Minimum
rlb			var	rlb+2
rlbRightXMin		equ	rlb			; Right BoundaryBox X Minimum
rlb			var	rlb+2
rlbRightXMax		equ	rlb			; Right BoundaryBox X Maximum
rlb			var	rlb+2
rlbRightChild		equ	rlb			; Right Child,>=$8000 = AREA
rlb			var	rlb+2
rlbSize			equ	rlb


;
;	* * * * * * *       AREA STRUCTURE       * * * * * * *
;
rla			var	0
rlaNumSegs		equ	rla			; Number of Segments
rla			var	rla+1
rlaSegOffset		equ	rla			; Offset to Starting Segment
rla			var	rla+2
rlaSector		equ	rla			; Sector this Area belongs to
rla			var	rla+1
rlaSize			equ	rla


;
;	* * * * * * *       VERTEX STRUCTURE       * * * * * * *
;
rlx			var	0
rlxVertexX		equ	rlx			; Vertex X
rlx			var	rlx+2
rlxVertexY		equ	rlx			; Vertex Y
rlx			var	rlx+2
rlxSize			equ	rlx


;
;	* * * * * * *       SEGMENT STRUCTURE       * * * * * * *
;
rlg			var	0
rlgVertex1		equ	rlg			; Vertex1
rlg			var	rlg+2
rlgVertex2		equ	rlg			; Vertex2
rlg			var	rlg+2
rlgFlags		equ	rlg			; Flags
rlg			var	rlg+1
rlgOffsetX		equ	rlg			; FaceOffsetX
rlg			var	rlg+1
rlgOffsetY		equ	rlg			; FaceOffsetY
rlg			var	rlg+1
rlgFace			equ	rlg			; Face
rlg			var	rlg+2
rlgLine			equ	rlg			; Line
rlg			var	rlg+2
rlgSize			equ	rlg
;
;	>>>   SEGMENT FLAGS   <<<
;
rlgfSolid		equ	$01			; SEGMENT is SOLID
rlgfSky			equ	$02			; UPPER TEXTURE is SKY
rlgfDoor		equ	$04			; SEGMENT is DOOR
rlgfClear		equ	$08			; SEGMENT is CLEAR
rlgfALTTEXTURE		equ	$10			; ALTERNATE TEXTURE
rlgfNORMALPEGGED	equ	$40			; NORMAL WALL TEXTURE IS PEGGED
rlgfLOWERPEGGED		equ	$40			; LOWER WALL TEXTURE IS PEGGED
rlgfUPPERPEGGED		equ	$20			; UPPER WALL TEXTURE IS PEGGED


;
;	* * * * * * *       FACE STRUCTURE       * * * * * * *
;
;	>>>   SOLID/TRANSPARENT FACE   <<<
;
rlf			var	0
rlfSectorNear		equ	rlf			; Sector Near
rlf			var	rlf+1
rlfSizeST		equ	rlf			; SIZE FOR UNION SOLID/TRANSPARENT
;
;	>>>   SOLID FACE   <<<
;
rlf			var	rlfSizeST
rlfWallTexture		equ	rlf			; Texture of Normal Wall
rlf			var	rlf+1
rlfSizeS		equ	rlf			; SIZE FOR SOLID FACE
;
;	>>>   TRANSPARENT FACE   <<<
;
rlf			var	rlfSizeST
rlfSectorFar		equ	rlf			; Sector Far
rlf			var	rlf+1
rlfUpperTexture		equ	rlf			; Texture of Upper Wall
rlf			var	rlf+1
rlfLowerTexture		equ	rlf			; Texture of Lower Wall
rlf			var	rlf+1
rlfSizeT		equ	rlf			; SIZE FOR TRANSPARENT FACE


;
;	* * * * * * *       SECTOR STRUCTURE       * * * * * * *
;
rls			var	0
rlsType			equ	rls			; Sector Type
rls			var	rls+1
rlsLightLevel		equ	rls			; Light Level (Near)
rls			var	rls+1
rlsLightLevel2		equ	rls			; Light Level (Far/Neighbor)
rls			var	rls+1
rlsFloorHeight		equ	rls			; Floor Height
rls			var	rls+2
rlsCeilingHeight	equ	rls			; Ceiling Height
rls			var	rls+2
rlsFloorTexture		equ	rls			; Floor Texture
rls			var	rls+1
rlsCeilingTexture	equ	rls			; Ceiling Texture
rls			var	rls+1
rlsTag			equ	rls			; Tag
rls			var	rls+1
rlsSize			equ	rls


;
;	* * * * * * *       SECTORDATA STRUCTURE       * * * * * * *
;
rlsd			var	0
rlsdFlags		equ	rlsd			; Flags
rlsd			var	rlsd+1
rlsdLightLevel		equ	rlsd			; LightLevel ($00=Bright,$ff=Dark)
rlsd			var	rlsd+1
rlsdFloorHeight		equ	rlsd			; Floor Height
rlsd			var	rlsd+2
rlsdCeilingHeight	equ	rlsd			; Ceiling Height
rlsd			var	rlsd+2
rlsdData		equ	rlsd			; DataBlock
rlsd			var	rlsd+1
rlsd			var	rlsd+1			; UNUSED!
rlsdFloorTexture	equ	rlsd			; Floor Texture
rlsd			var	rlsd+1
rlsdCeilingTexture	equ	rlsd			; Ceiling Texture
rlsd			var	rlsd+1
rlsdRLCount		equ	rlsd			; RLEngine Count
rlsd			var	rlsd+2
rlsdObjects		equ	rlsd			; Pointer to First RLObject
rlsd			var	rlsd+2
rlsdSize		equ	rlsd
;
;	>>>   SECTORDATA FLAGS   <<<
;
rlsdfLightDir		equ	$01			; LightLevel is Incrementing
rlsdfSecret		equ	$02			; SECRET Found
rlsdfTypeNormal		equ	$04			; TYPE Now NORMAL
rlsdfTypeNukage		equ	$08			; TYPE Now NUKAGE
rlsdfCeilingSky		equ	$80			; Ceiling Texture is SKY!


;
;	* * * * * * *       LINE STRUCTURE       * * * * * * *
;
rll			var	0
rllVertex1		equ	rll			; Vertex 1
rll			var	rll+2
rllVertex2		equ	rll			; Vertex 2
rll			var	rll+2
rllFlags		equ	rll			; Flags
rll			var	rll+2
rllAngle		equ	rll			; Angle
rll			var	rll+2
rllFace			equ	rll			; Face
rll			var	rll+2
rllType			equ	rll			; Type
rll			var	rll+1
rllTag			equ	rll			; Tag
rll			var	rll+1
rllSize			equ	rll			; !!!DANGER!!!  _RLE2T3300  !!!
;
;	>>>   LINE FLAGS (* indicates which flags are in RAM)   <<<
;
rllfSolid		equ	$0001			; *LINE is SOLID
rllfMapped		equ	$0002			; *LINE is MAPPED
rllfUsed		equ	$0004			; *LINE is USED (Operates Once)
rllfSecret		equ	$0004			;  LINE is SECRET
rllfImpassible		equ	$0008			;  LINE is IMPASSIBLE
rllfDoor		equ	$0010			;  LINE is DOOR
rllfDontDraw		equ	$0020			;  LINE is HIDDEN
;
rllfFloorH		equ	$0100			; FloorHeight Changes
rllfCeilingH		equ	$0200			; CeilingHeight Changes
rllfTrigger		equ	$0400			; LINE is TRIGGER
rllfUsable		equ	$0800			; LINE is USABLE
rllfTriggerE		equ	$1000			; LINE is TRIGGER BY ENEMIES
rllfUsableE		equ	$2000			; LINE is USABLE BY ENEMIES
;
rllfItem		equ	$8000			; LINE is ITEM


;
;	* * * * * * *       BLOCKMAP STRUCTURE       * * * * * * *
;
rlk			var	0
rlkX			equ	rlk			; X Origin
rlk			var	rlk+2
rlkY			equ	rlk			; Y Origin
rlk			var	rlk+2
rlkDim			equ	rlk			; Dimensions
rlkXBlocks		equ	rlk			; #Blocks in X
rlk			var	rlk+1
rlkYBlocks		equ	rlk			; #Blocks in Y
rlk			var	rlk+1
;
;	>>>   LIST OF BLOCKMAP OFFSETS FOLLOWS   <<<
;
rlkBlockOffsets		equ	rlk			; Start of BlockOffsets
;
;	>>>   LISTS OF LINES FOLLOW   <<<
;
;rlkLines		equ	rlk			; Start of LineLists
;rlkSize		equ	rlk


;
;	* * * * * * *       LEVEL OBJECT STRUCTURE       * * * * * * *
;
rlp			var	0
rlpFlags		equ	rlp			; Flags
rlp			var	rlp+1
rlpType			equ	rlp			; Type
rlp			var	rlp+1
rlpX			equ	rlp			; X Coordinate
rlp			var	rlp+2
rlpY			equ	rlp			; Y Coordinate
rlp			var	rlp+2
rlpSize0		equ	rlp
rlpAngle		equ	rlp			; Angle
rlp			var	rlp+2
rlpSize			equ	rlp
;
;	>>>   LEVEL OBJECT FLAGS   <<<
;
rlpfSkill12		equ	$01			; Present at skill 1 and 2
rlpfSkill3		equ	$02			; Present at skill 3 (hurt me plenty)
rlpfSkill45		equ	$04			; Present at skill 4 and 5 (ultra-violence, nightmare)
;rlpfDeafGuard		equ	$08			; Deaf Guard
rlpfMultiPlayer		equ	$10			; Only appears in MultiPlayer Mode
rlpfMovable		equ	$40			; Object can MOVE throughout Level
rlpfActive		equ	$80			; Object is ACTIVE (Visible)
;
;	>>>   LEVEL OBJECT FLAGS   <<<
;
rlpfEnemyBlocked	equ	$07			; Enemy Movement was just Blocked
rlpfProjectileFast	equ	$01			; Projectile is FAST
rlpfSolid		equ	$08			; Object is SOLID
;rlpfProjectile		equ	$10			; Object is PROJECTILE
rlpfItem		equ	$10			; Object is ITEM
rlpfTarget		equ	$20			; Object is TARGET
rlpfMovable		equ	$40			; Object is MOVABLE
rlpfActive		equ	$80			; Object is ACTIVE


;
;	* * * * * * *       PLATFORM DOOR STRUCTURE       * * * * * * *
;
rlpd			var	0
rlpdSector		equ	rlpd			; Sector#
rlpd			var	rlpd+1
rlpdMaxHeight		equ	rlpd			; Maximum Ceiling Height
rlpd			var	rlpd+2
rlpdOriginX		equ	rlpd			; Sound Origin X
rlpd			var	rlpd+2
rlpdOriginY		equ	rlpd			; Sound Origin Y
rlpd			var	rlpd+2
rlpdNumLines		equ	rlpd			; #Lines
rlpd			var	rlpd+1
rlpdLines		equ	rlpd			; Line List
;rlpdSize		equ	rlpd

;
;	* * * * * * *       PLATFORM FLOOR STRUCTURE       * * * * * * *
;
rlpf			var	0
rlpfTag			equ	rlpf			; Tag
rlpf			var	rlpf+1
rlpfType		equ	rlpf			; Type
rlpf			var	rlpf+1
rlpfNumSectors		equ	rlpf			; #Sectors
rlpf			var	rlpf+1
rlpfSize		equ	rlpf
;
rlpfs			var	0			; REPEATED FOR EACH FLOOR SECTOR
rlpfsSector		equ	rlpfs			; Sector#
rlpfs			var	rlpfs+1
rlpfsHeight		equ	rlpfs			; Destination Height
rlpfs			var	rlpfs+2
rlpfsOriginX		equ	rlpfs			; Sound Origin X
rlpfs			var	rlpfs+2
rlpfsOriginY		equ	rlpfs			; Sound Origin Y
rlpfs			var	rlpfs+2
rlpfsModelSector	equ	rlpfs			; Model Sector#
rlpfs			var	rlpfs+1
rlpfsSize		equ	rlpfs

;
;	* * * * * * *       PLATFORM STAIR STRUCTURE       * * * * * * *
;
rlps			var	0
rlpsTag			equ	rlps			; Tag
rlps			var	rlps+1
rlpsOriginX		equ	rlps			; Sound Origin X
rlps			var	rlps+2
rlpsOriginY		equ	rlps			; Sound Origin Y
rlps			var	rlps+2
rlpsHeight		equ	rlps			; Starting Height
rlps			var	rlps+2
rlpsNumSectors		equ	rlps			; #Sectors
rlps			var	rlps+1
rlpsSectors		equ	rlps			; Sector List
;rlpsSize		equ	rlps

;
;	* * * * * * *       PLATFORM CEILING STRUCTURE       * * * * * * *
;
rlpc			var	0
rlpcTag			equ	rlpc			; Tag
rlpc			var	rlpc+1
rlpcType		equ	rlpc			; Type
rlpc			var	rlpc+1
rlpcNumSectors		equ	rlpc			; #Sectors
rlpc			var	rlpc+1
;
rlpcs			var	0			; REPEATED FOR EACH CEILING SECTOR
rlpcsSector		equ	rlpcs			; Sector#
rlpcs			var	rlpcs+1
rlpcsMaxHeight		equ	rlpcs			; Maximum Ceiling Height
rlpcs			var	rlpcs+2
rlpcsOriginX		equ	rlpcs			; Sound Origin X
rlpcs			var	rlpcs+2
rlpcsOriginY		equ	rlpcs			; Sound Origin Y
rlpcs			var	rlpcs+2
rlpcsNumLines		equ	rlpcs			; #Lines
rlpcs			var	rlpcs+1
rlpcsLines		equ	rlpcs			; Line List
;rlpcsSize		equ	rlpcs

;
;	* * * * * * *       PLATFORM LIFT STRUCTURE       * * * * * * *
;
rlpl			var	0
rlplTag			equ	rlpl			; Tag
rlpl			var	rlpl+1
rlplType		equ	rlpl			; Type
rlpl			var	rlpl+1
rlplNumSectors		equ	rlpl			; #Sectors
rlpl			var	rlpl+1
;
rlpls			var	0			; REPEATED FOR EACH LIFT SECTOR
rlplsSector		equ	rlpls			; Sector#
rlpls			var	rlpls+1
rlplsMinHeight		equ	rlpls			; Minimum Floor Height
rlpls			var	rlpls+2
rlplsMaxHeight		equ	rlpls			; Maximum Floor Height
rlpls			var	rlpls+2
rlplsOriginX		equ	rlpls			; Sound Origin X
rlpls			var	rlpls+2
rlplsOriginY		equ	rlpls			; Sound Origin Y
rlpls			var	rlpls+2
rlplsSize		equ	rlpls


;
;	* * * * * * *       TEXTURE STRUCTURE       * * * * * * *
;
rlt			var	0
rltWidth		equ	rlt			; #Pixels Wide
rlt			var	rlt+2
rltHeight		equ	rlt			; #Pixels Tall
rlt			var	rlt+2
rltNumPatches		equ	rlt			; #Patches
rlt			var	rlt+1
rltPatches		equ	rlt			; Patch Definitions
;
;	* * * * * * *       TEXTURE PATCH STRUCTURE       * * * * * * *
;
rlh			var	0
rlhXOffset		equ	rlh			; X Offset
rlh			var	rlh+2
rlhYOffset		equ	rlh			; Y Offset
rlh			var	rlh+2
rlhWall			equ	rlh			; Wall#
rlh			var	rlh+2
rlhSize			equ	rlh


;
;	* * * * * * *       IMAGERY STRUCTURE       * * * * * * *
;
rli			var	0
;rliFlags		equ	rli			; Flags
;rli			var	rli+2
rliHeight		equ	rli			; #Pixels Tall
rli			var	rli+1
rliWidth		equ	rli			; #Pixels Wide
rli			var	rli+1
rliData			equ	rli			; Pointers to PixelStrips
;
rlid			var	0
rlidStrip		equ	rlid			; Pointer to PixelStrip
rlid			var	rlid+3


;
;	* * * * * * *       SPRITE STRUCTURE       * * * * * * *
;
rlr			var	0
rlrWidth		equ	rlr			; #Pixels Wide (Whole Sprite)
rlr			var	rlr+1
rlrHeight		equ	rlr			; #Pixels Tall (Whole Sprite)
;
;	>>>   SPRITESTRIP SUBSTRUCTURE (FOR EACH LINE)   <<<
;
rlrs			var	0
rlrsOffset		equ	rlrs			; #Pixels Offset (This Line)
rlrs			var	rlrs+1
rlrsWidth		equ	rlrs			; #Pixels Wide (This Line)
rlrs			var	rlrs+1
rlrsLineOffset		equ	rlrs			; LineOffset (This Line)
rlrs			var	rlrs+2			; (from HighByte to LineData)
rlrsSize		equ	rlrs
;
;	>>>   SPRITEPIXELBLOCK SUBSUBSTRUCTURE (FOR EACH PIXELBLOCK ON A LINE)   <<<
;
rlrb			var	0
rlrbLength		equ	rlrb			; #Pixels Wide ($00=End of Line)
rlrb			var	rlrb+1
rlrbOffset		equ	rlrb			; #Pixels Offset
rlrb			var	rlrb+1
rlrbData		equ	rlrb			; DataBlock (COLOUR# per Pixel)
;							; $FF,LEN,COLOUR for RUNS >=3


;
;	* * * * * * *       GSU AREA/SEGMENT LIST STRUCTURE       * * * * * * *
;
rlas			var	0
rlasPrev		equ	rlas		; Previous Segment Block
rlas			var	rlas+2
rlasArea		equ	rlas		; Area
rlas			var	rlas+2
rlasSegs		equ	rlas		; List of Segments and Segments' Data
;rlas			var	rlas+(#Segs*rluSize)
rlasEnd			equ	rlas		; Ending Segment Terminator ($7FXX)
rlas			var	rlas+2
rlasSize		equ	rlas


;
;	* * * * * * *       GSU BSP SEGMENT LIST STRUCTURE       * * * * * * *
;
rlu			var	0
rluSeg			equ	rlu		; Segment
rlu			var	rlu+2
rluV1			equ	rlu		; Vertex 1
rlu			var	rlu+2
rluV2			equ	rlu		; Vertex 2
rlu			var	rlu+2
rluXs1			equ	rlu		; Screen X1
rlu			var	rlu+2
rluXs2			equ	rlu		; Screen X2
rlu			var	rlu+2
rluY1			equ	rlu		; World Y1
rlu			var	rlu+2
rluY2			equ	rlu		; World Y2
rlu			var	rlu+2
rluSize			equ	rlu


;
;	* * * * * * *       GSU BSP SEGMENT CLIPZONE STRUCTURE       * * * * * * *
;
rlcz			var	0
rlczX2			equ	rlcz		; Screen X2
rlcz			var	rlcz+1
rlczX1			equ	rlcz		; Screen X1
rlcz			var	rlcz+1
rlczNext		equ	rlcz		; Next
rlcz			var	rlcz+2
rlczSize		equ	rlcz


;
;	* * * * * * *       GSU BSP VISIBLE SEGMENT CLIPZONE STRUCTURE       * * * * * * *
;
rlvcz			var	0
rlvczClipRange		equ	rlvcz		; Screen ClipRange MinY/MaxY PreVSeg
rlvcz			var	rlvcz+2
rlvczY			equ	rlvcz		; World Y Distance
rlvcz			var	rlvcz+2
rlvczCeilingY		equ	rlvcz		; Screen Ceiling Y
rlvcz			var	rlvcz+2
rlvczFloorY		equ	rlvcz		; Screen Floor Y
rlvcz			var	rlvcz+2
rlvczClipRange2		equ	rlvcz		; Screen ClipRange MinY/MaxY PostVSeg
rlvcz			var	rlvcz+2
rlvczSize		equ	rlvcz


;
;	* * * * * * *       GSU VISIBLE SEGMENT LIST STRUCTURE       * * * * * * *
;
rlv			var	0
rlvSeg			equ	rlv		; Segment Address
rlvXsT			equ	rlv		; Screen X1/XsCount (#Pixels to Trace)
rlv			var	rlv+2
rlvXs0			equ	rlv		; Screen X0 (rXs0)
rlv			var	rlv+2
rlvFlags		equ	rlv		; Flags
rlv			var	rlv+2
rlvXs1			equ	rlv		; Screen X1 (rXs1)
rlv			var	rlv+1
rlvXs2			equ	rlv		; Screen X2 (rXs2)
rlv			var	rlv+1
rlvXsRatioI		equ	rlv		; Screen Xs RatioIndex (ScreenX3-ScreenX0+1)
rlvFace			equ	rlv		; Face Address
rlv			var	rlv+2
;
rlvSegFlags		equ	rlv		; SegmentFlags
rlvClipRangeP		equ	rlv		; ClipRange Address
rlv			var	rlv+2
;
rlvClipZoneP		equ	rlv		; ClipZone Address
rlv			var	rlv+2
;
rlvY1			equ	rlv		; World Y1
rlv			var	rlv+2
rlvY2			equ	rlv		; World Y2
rlv			var	rlv+2
rlvYInvF		equ	rlv		; YInv Fraction
rlv			var	rlv+2
rlvYInv			equ	rlv		; YInv
rlv			var	rlv+2
;
rlvFaceOffsetX		equ	rlv		; FaceOffsetX
rlv			var	rlv+1
rlvFaceOffsetY		equ	rlv		; FaceOffsetY
rlv			var	rlv+1
;
rlvYInvStepF		equ	rlv		; YInv StepDelta Fraction
rlv			var	rlv+2
rlvYInvStep		equ	rlv		; YInv StepDelta
rlv			var	rlv+2
;
rlvSectorNearData	equ	rlv		; SectorNearData Address
rlv			var	rlv+2
rlvSectorFarData	equ	rlv		; SectorFarData Address
rlv			var	rlv+2
;
rlvFloorHeight		equ	rlv		; Height of Floor
rlvNormalWallBotHeight	equ	rlv		; Height of Normal Wall Bottom
rlvLowerWallBotHeight	equ	rlv		; Height of Lower  Wall Bottom
rlv			var	rlv+2
rlvCeilingHeight	equ	rlv		; Height of Ceiling
rlvNormalWallTopHeight	equ	rlv		; Height of Normal Wall Top
rlvUpperWallTopHeight	equ	rlv		; Height of Upper  Wall Top
rlv			var	rlv+2
;
rlvUpperWallBotHeight	equ	rlv		; Height of Upper  Wall Bottom
rlv			var	rlv+2
rlvLowerWallTopHeight	equ	rlv		; Height of Lower  Wall Top
rlv			var	rlv+2
;
rlvWallPlotDataStart	equ	rlv		; Start of WallPlotData for this VSeg
rlv			var	rlv+2
rlvWallPlotDataEnd	equ	rlv		; End of WallPlotData for this VSeg
rlv			var	rlv+2
;
rlvNormalWallTextureH	equ	rlv		; TextureHeight  for Normal Wall
rlvUpperWallTextureH	equ	rlv		; TextureHeight  for Upper  Wall
rlv			var	rlv+1
rlvNormalWallTextureW	equ	rlv		; TextureWidth   for Normal Wall
rlvUpperWallTextureW	equ	rlv		; TextureWidth   for Upper  Wall
rlv			var	rlv+1
rlvNormalWallTexture	equ	rlv		; TextureAddress for Normal Wall
rlvUpperWallTexture	equ	rlv		; TextureAddress for Upper  Wall
rlv			var	rlv+2
rlvLowerWallTextureH	equ	rlv		; TextureHeight  for Lower  Wall
rlv			var	rlv+1
rlvLowerWallTextureW	equ	rlv		; TextureWidth   for Lower  Wall
rlv			var	rlv+1
rlvLowerWallTexture	equ	rlv		; TextureAddress for Lower  Wall
rlv			var	rlv+2
;
rlvVertex1		equ	rlv		; Vertex 1
rlv			var	rlv+2
rlvVertex2		equ	rlv		; Vertex 2
rlv			var	rlv+2
;
rlvRSAngle		equ	rlv		; Rotated Segment Angle
rlv			var	rlv+2
rlvRSPDistance		equ	rlv		; Rotated Segment Perpendicular Distance
rlv			var	rlv+2
rlvVTOffset		equ	rlv		; Vertex TextureOffset
rlv			var	rlv+2
;
rlvSize			equ	rlv
;
;	>>>   VISIBLE SEGMENT FLAGS   <<<
;
rlvfWALL		equ	$8000		; WALL *ANY WALL*
;
rlvfNORMALWALL		equ	$0001		; NORMAL WALL
rlvfUPPERWALL		equ	$0002		; UPPER WALL
rlvfLOWERWALL		equ	$0004		; LOWER WALL
;
;rlvfNORMALCLIP		equ	$0010		; NORMAL CLIP
rlvfUPPERCLIP		equ	$0100		; UPPER CLIP
rlvfLOWERCLIP		equ	$0080		; LOWER CLIP
;
rlvfADDCEILING		equ	$0020		; CEILING *ONLY*
rlvfADDFLOOR		equ	$0040		; FLOOR *ONLY*
;
rlvfNORMALSPRCLIP	equ	$0200		; NORMAL SPRITE CLIP
rlvfUPPERSPRCLIP	equ	$0400		; UPPER SPRITE CLIP
rlvfLOWERSPRCLIP	equ	$0008		; LOWER SPRITE CLIP
;
rlvfNORMALPEGGED	equ	$4000		; NORMAL WALL TEXTURE IS PEGGED
rlvfLOWERPEGGED		equ	$4000		; LOWER WALL TEXTURE IS PEGGED
rlvfUPPERPEGGED		equ	$2000		; UPPER WALL TEXTURE IS PEGGED
;
rlvfALTTEXTURE		equ	$1000		; ALTERNATE TEXTURE


;
;	* * * * * * *       GSU VISIBLE OBJECT LIST STRUCTURE       * * * * * * *
;
rlq			var	0
rlqY			equ	rlq		; World Y
rlq			var	rlq+2
rlqNext			equ	rlq		; Pointer to Next VObj
rlq			var	rlq+2
rlqObj			equ	rlq		; RLObject Address
rlq			var	rlq+2
rlqWX			equ	rlq		; World X
rlq			var	rlq+2
rlqWZ			equ	rlq		; World Z
rlq			var	rlq+2
rlqXs1			equ	rlq		; Screen X1 (rXs1)
rlq			var	rlq+2
rlqXs2			equ	rlq		; Screen X2 (rXs2)
rlq			var	rlq+2
rlqHeight		equ	rlq		; World Height
rlq			var	rlq+1
rlqWidth		equ	rlq		; World Width
rlq			var	rlq+1
rlqSectorData		equ	rlq		; Pointer to SectorData
rlq			var	rlq+2
rlqImage		equ	rlq		; Imagery Pointer
rlq			var	rlq+2
rlqDrawn		equ	rlq		; Drawn Flag
rlq			var	rlq+1
rlqFlipWidth		equ	rlq		; World Width-1
rlq			var	rlq+1
rlqObjPlotDataStart	equ	rlq		; Start of ObjPlotData for this VObj
rlq			var	rlq+2
rlqObjPlotDataEnd	equ	rlq		; End of ObjPlotData for this VObj
rlq			var	rlq+2
rlqSize			equ	rlq


;
;	* * * * * * *       GSU FLOOR/CEILING DATA STRUCTURE       * * * * * * *
;
rlfd			var	0
rlfdKey			equ	rlfd		; Search Key
rlfdSectorData		equ	rlfd		; SectorDataNear (Bit0:Floor=0,Ceiling=1)
rlfd			var	rlfd+2
rlfdNext		equ	rlfd		; Pointer to Next Floor/Ceiling
rlfd			var	rlfd+2
rlfdMinX		equ	rlfd		; Minimum X Coordinate
rlfd			var	rlfd+1
rlfdMaxX		equ	rlfd		; Maximum X Coordinate
rlfd			var	rlfd+1
rlfdHeight		equ	rlfd		; Height of this Floor/Ceiling
rlfd			var	rlfd+2
rlfdFloorPlotDataStart	equ	rlfd		; Start of FloorPlotData
rlfd			var	rlfd+2
rlfdFloorPlotDataEnd	equ	rlfd		; End of FloorPlotData
rlfd			var	rlfd+2
rlfdRange		equ	rlfd		; Range of Floor/Ceiling Data
	ife	useHIGHDETAIL
rlfd			var	rlfd+(((RLViewPixX/3/2)+1)*2)
	endif
	ifn	useHIGHDETAIL
rlfd			var	rlfd+(((RLViewPixX/3)+1)*2)
	endif
rlfdSize		equ	rlfd


;
;	* * * * * * *       GSU PRE WALL PLOT LIST STRUCTURE       * * * * * * *
;
plv			var	0
plvZ2			equ	plv		; World Z Adjust
plv			var	plv+2
plvY			equ	plv		; World Y
plv			var	plv+2
plvClipRange		equ	plv		; Clipping Range
plv			var	plv+2
plvXs			equ	plv		; Screen X Coordinate
plv			var	plv+1
plv			var	plv+1
plvScaleF		equ	plv		; RL->SCN Pixel Scaling Fraction
plv			var	plv+2
plvVSegTextureH		equ	plv		; Offset to VisibleSegment Texture Height
plv			var	plv+1
plvScale		equ	plv		; RL->SCN Pixel Scaling Integer
plv			var	plv+1
plvZ			equ	plv		; World Z at Bottom
plv			var	plv+2
plvSize			equ	plv


;
;	* * * * * * *       GSU WALL PLOT LIST STRUCTURE       * * * * * * *
;
plw			var	0
plwNumPix		equ	plw		; Number of ScreenPixels to Plot
plw			var	plw+1
plwColourMap		equ	plw		; HighByte of ColourMap
plw			var	plw+1
plwPixScale		equ	plw		; SCN->RL Pixel Scaling Value
plw			var	plw+2
plwPixY			equ	plw		; Starting Y Coordinate
plw			var	plw+1
plwScale		equ	plw		; RL->SCN Pixel Scaling Integer
plwBank			equ	plw		; Bank of Data to Plot
plw			var	plw+1
plwScaleF		equ	plw		; RL->SCN Pixel Scaling Fraction
plwData			equ	plw		; Address of Data to Plot
plw			var	plw+2
plwWallF		equ	plw		; World Z Fraction
plw			var	plw+1
plwTextureOffset	equ	plw		; TextureOffset in Pixels
plw			var	plw+1
plwVSegTextureH		equ	plw		; Offset to VisibleSegment TextureHeight
plw			var	plw+1
plwXs			equ	plw		; Screen X Coordinate
plw			var	plw+1
plwNext			equ	plw		; Pointer to Next WallPlot
plw			var	plw+2
plwSize			equ	plw


;
;	* * * * * * *       GSU FLOOR PLOT LIST STRUCTURE       * * * * * * *
;
	ife	useFLOORS
plf			var	0
plfNumPix		equ	plf		; Number of ScreenPixels to Plot +1
plf			var	plf+1
plfColourMap		equ	plf		; HighByte of ColourMap (0=SKY)
plf			var	plf+1
plfPixX			equ	plf		; Starting X Coordinate
plf			var	plf+1
plfPixY			equ	plf		; Starting Y Coordinate
plf			var	plf+1
plfSector		equ	plf		; Pointer to SectorData
plf			var	plf+2
plfSize			equ	plf
	endif
	ifn	useFLOORS
plf			var	0
plfNumPix		equ	plf		; Number of ScreenPixels to Plot +1
plf			var	plf+1
plfColourMap		equ	plf		; HighByte of ColourMap (0=SKY)
plf			var	plf+1
plfPixScaleX		equ	plf		; SCN->RL X Pixel Scaling Value
plf			var	plf+2
plfPixScaleY		equ	plf		; SCN->RL Y Pixel Scaling Value
plf			var	plf+2
plfSector		equ	plf		; Pointer to SectorData
plfFloorXY		equ	plf		; Texture X/Y Offsets
plf			var	plf+2
plfFloorXYF		equ	plf		; World X/World Y Fractions
plf			var	plf+2
plfPixX			equ	plf		; Starting X Coordinate
plf			var	plf+1
plfPixY			equ	plf		; Starting Y Coordinate
plf			var	plf+1
plfSize			equ	plf
	endif


;
;	* * * * * * *       GSU OBJ PLOT LIST STRUCTURE       * * * * * * *
;
plo			var	0

ploImageOffset		equ	plo		; ImageOffset in Pixels
plo			var	plo+1
plo			var	plo+1		; UNUSED!

ploXs			equ	plo		; Screen X Coordinate
plo			var	plo+1
ploYs			equ	plo		; Screen Y Coordinate
plo			var	plo+1

ploData			equ	plo		; Address of Data to Plot
plo			var	plo+2

ploBank			equ	plo		; Bank of Data to Plot
plo			var	plo+1
ploColourMap		equ	plo		; HighByte of ColourMap
plo			var	plo+1

ploObjF			equ	plo		; World Z Fraction
plo			var	plo+1
ploNumPix		equ	plo		; Number of ScreenPixels to Plot
plo			var	plo+1

ploSize			equ	plo


;
;	* * * * * * *       GSU OBJECT STRUCTURE       * * * * * * *
;
rlo			var	0
rloNext			equ	rlo		; Next Object (-1=FREE)
rlo			var	rlo+2
rloPrev			equ	rlo		; Previous Object
rlo			var	rlo+2
rloFlags		equ	rlo		; rlpFlags (from LevelObject)
rlo			var	rlo+1
rloType			equ	rlo		; Type
rlo			var	rlo+1
;
rloLObj			equ	rlo		; Level Object (UNMOVABLE ONLY!)
rloSize0		equ	rlo+2		; SIZE FOR UNMOVABLE OBJECTS
;
rloX			equ	rlo		; X Coordinate
rlo			var	rlo+4
rloY			equ	rlo		; Y Coordinate
rlo			var	rlo+4
rloZ			equ	rlo		; Z Coordinate
rlo			var	rlo+2
rloVX			equ	rlo		; X Velocity
rlo			var	rlo+2
rloVY			equ	rlo		; Y Velocity
rlo			var	rlo+2
rloAngle		equ	rlo		; Angle
rlo			var	rlo+2
rloSector		equ	rlo		; Sector
rlo			var	rlo+1
;
;	>>>   ANIMATION/IMAGERY SUBSTRUCTURE   <<<
;
rloImage		equ	rlo		; Image#
rlo			var	rlo+1
rloAnim			equ	rlo		; Animation
rlo			var	rlo+2
rloAnimCount		equ	rlo		; #Cycles Left in Current AnimImage
rlo			var	rlo+1
;
;	>>>   STATE SUBSTRUCTURE   <<<
;
rloStateCount		equ	rlo		; #Cycles Left in Current State
rlo			var	rlo+1
rloState		equ	rlo		; Pointer to MObjState
rlo			var	rlo+2
rloMData		equ	rlo		; Pointer to MObjData
rlo			var	rlo+2
rloSize1		equ	rlo		; BASE SIZE FOR MOVABLE OBJECTS
;
;	>>>   MOVING OBJECTS SUBSTRUCTURE UNION   <<<
;
rlo			var	rloSize1
rloHealth		equ	rlo		; #Health Points
rlo			var	rlo+2
rloSize2		equ	rlo		; BASE SIZE FOR PLAYER/ENEMIES
;
;	>>>   PLAYER DATA SUBSTRUCTURE UNION   <<<
;
;rloPlayerData		equ	rlo		; PlayerData SubStructure
rloZ2			equ	rlo		; Z2 Coordinate
rlo			var	rlo+2
rloC2			equ	rlo		; C2 Coordinate
rlo			var	rlo+2
;
;	>>>   ENEMY DATA SUBSTRUCTURE UNION   <<<
;
rlo			var	rloSize2
;rloEnemyData		equ	rlo		; EnemyData SubStructure
rloTObj			equ	rlo		; Pointer to Target RLObject
rlo			var	rlo+2
rloTDistance		equ	rlo		; Distance to Target
rlo			var	rlo+2
;rloTAngle		equ	rlo		; Angle to Target
;rlo			var	rlo+2
rloSize			equ	rlo		; SIZE FOR MOVABLE OBJECTS
;
;	>>>   PROJECTILE SUBSTRUCTURE UNION   <<<
;
rlo			var	rloSize1
rloVZ			equ	rlo		; Z Velocity
rlo			var	rlo+2
rloVXY			equ	rlo		; XY Velocity (+$8000>>1)
rlo			var	rlo+2		; (OVERLAYS rloTObj!)
rloZF			equ	rlo		; Z Coordinate Fraction (Projectiles)
rlo			var	rlo+2


;
;	* * * * * * *       GSU MOVE OBJECT LINE STRUCTURE       * * * * * * *
;
rlml			var	0
rlmlLine		equ	rlml		; Line#
rlml			var	rlml+2
rlmlLX1			equ	rlml		; Line Vertex X1
rlml			var	rlml+2
rlmlLY1			equ	rlml		; Line Vertex Y1
rlml			var	rlml+2
rlmlPUX			equ	rlml		; Line Perpendicular UnitVector X
rlml			var	rlml+2
rlmlPUY			equ	rlml		; Line Perpendicular UnitVector Y
rlml			var	rlml+2
rlmlFlags		equ	rlml		; Line Flags
rlml			var	rlml+2
rlmlLX2			equ	rlml		; Line Vertex X2
rlml			var	rlml+2
rlmlLY2			equ	rlml		; Line Vertex Y2
rlml			var	rlml+2
rlmlHighFloorHeight	equ	rlml		; Highest Floor Height
rlmlSectorData		equ	rlml		; SectorData for Items
rlml			var	rlml+2
rlmlLowCeilingHeight	equ	rlml		; Lowest Ceiling Height
rlml			var	rlml+2
rlmlLowFloorHeight	equ	rlml		; Lowest Floor Height
rlml			var	rlml+2
rlmlSize		equ	rlml


;
;	* * * * * * *       GSU TASK STRUCTURE       * * * * * * *
;
rlm			var	0
rlmCode			equ	rlm		; Pointer to TaskCode
rlm			var	rlm+2
rlmData			equ	rlm		; Pointer to TaskData
rlm			var	rlm+2
rlmSize			equ	rlm


;
;	* * * * * * *       TOGGLE TASK STRUCTURE       * * * * * * *
;
rltt			var	0
rlttCount		equ	rltt			; Toggle TimeCount
rltt			var	rltt+2
rlttLineFlag		equ	rltt			; Pointer to LineFlag
rltt			var	rltt+2
rlttSize		equ	rltt
rlttCountTime		equ	60			; Toggle in 60 Ticks (1 Second)


;
;	* * * * * * *       PLATFORM TASK STRUCTURE       * * * * * * *
;
rlpt			var	0
rlptState		equ	rlpt			; Platform State
rlpt			var	rlpt+2
rlptData1		equ	rlpt			; DataVariable
rlpt			var	rlpt+2
rlptPlatform		equ	rlpt			; Pointer to PlatformData
rlpt			var	rlpt+2
rlptSectorData		equ	rlpt			; Pointer to SectorData
rlpt			var	rlpt+2
rlptSize		equ	rlpt

;
;	* * * * * * *       PLATFORM DOOR TASK UNION STRUCTURE       * * * * * * *
;
rlpdtHoldCount		equ	rlptData1		; Hold Counter
rlpdtDoor		equ	rlptPlatform		; DoorData
rlpdtSectorData		equ	rlptSectorData		; SectorData
rlpdMovement		equ	6			; Doors Move 6 Pixels

;
;	* * * * * * *       PLATFORM FLOOR TASK UNION STRUCTURE       * * * * * * *
;
rlpftHeight		equ	rlptData1		; Destination Height
rlpftFloor		equ	rlptPlatform		; FloorData
rlpftSectorData		equ	rlptSectorData		; SectorData
rlpfMovement		equ	3			; Floors Move 3 Pixels
rlpsMovement		equ	2			; Stairs Move 2 Pixels

;
;	* * * * * * *       PLATFORM CEILING TASK UNION STRUCTURE       * * * * * * *
;
rlpctHeight		equ	rlptData1		; Destination Height
rlpctCeiling		equ	rlptPlatform		; CeilingData
rlpctSectorData		equ	rlptSectorData		; SectorData
rlpcMovement		equ	2			; Ceilings Move 2 Pixels

;
;	* * * * * * *       PLATFORM LIFT TASK UNION STRUCTURE       * * * * * * *
;
rlpltHoldCount		equ	rlptData1		; Hold Counter
rlpltLift		equ	rlptPlatform		; LiftData
rlpltSectorData		equ	rlptSectorData		; SectorData
rlplMovement		equ	6			; Lifts Move 6 Pixels
rlpmMovement		equ	2			; MovingFloors Move 2 Pixels


;
;	* * * * * * *       SOUND EFFECT STRUCTURE       * * * * * * *
;
rlse			var	0
rlsePriority		equ	rlse			; Priority (Volume)
rlse			var	rlse+1
rlseEffect		equ	rlse			; Sound Effect#
rlse			var	rlse+1
rlseSize		equ	rlse
