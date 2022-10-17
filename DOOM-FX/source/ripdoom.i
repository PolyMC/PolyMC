;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                            INCLUDES MODULE                              *
;*                                                                         *
;***************************************************************************

rllSize			equ	12
rlbSize			equ	28
rlaSize			equ	4
rlgSize			equ	11
rlfSizeS		equ	2
rlfSizeT		equ	4
rlsSize			equ	10
rlsdSize		equ	14
rlpSize			equ	8

;MaxRLVertexes		equ	1626			; Maximum VERTEXES per Level
MaxRLSectors		equ	205			; *Maximum SECTORS per Level
;MaxRLAreas		equ	512			; Maximum AREA Nodes per Level
;MaxRLSegs		equ	2438			; Maximum SEGS per Level
;MaxRLLines		equ	1764			; Maximum LINES per Level
;MaxRLFaces		equ	2252			; Maximum FACES per Level
MaxRLFObjects		equ	250			; *Maximum FIXED OBJECTS per Level
MaxRLMObjects		equ	180			; *Maximum MOVABLE OBJECTS per Level
MaxRLObjectTypes	equ	256			; Maximum OBJECT Types

MaxWADSectors		equ	512			; Maximum SECTORS per WAD Level
MaxWADFaces		equ	3072			; Maximum FACES per WAD Level

rlSectorData		equ	$00703080
rlVertexData		equ	rlSectorData+(MaxRLSectors*rlsdSize)


MEMF_PUBLIC		equ	$00001
MEMF_CHIP		equ	$00002
MEMF_FAST		equ	$00004
MEMF_CLEAR		equ	$10000
