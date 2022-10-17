;***************************************************************************
;*                                                                         *
;*                           S P L I T   M U S                             *
;*                                                                         *
;*                            INCLUDES MODULE                              *
;*                                                                         *
;***************************************************************************

MEMF_PUBLIC		equ	$00001
MEMF_CHIP		equ	$00002
MEMF_FAST		equ	$00004
MEMF_CLEAR		equ	$10000

MaxModules		equ	128		; Maximum #Modules

MaxBlocks		equ	3172		; Maximum Blocks
MaxModuleBlocks		equ	1024		; Maximum Blocks per Module

MaxChunks		equ	MaxBlocks	; Maximum Chunks
MaxModuleChunks		equ	512		; Maximum Chunks per Module

MaxEffects		equ	256		; Maximum #Effects

MusicBootAddress	equ	$0E8000
MusicLoadOffset		equ	(MusicBootAddress&$7FFF)+((MusicBootAddress>>16)*$8000)
MusicTableAddress	equ	$0E9900
MusicTableOffset	equ	(MusicTableAddress-MusicBootAddress)
ModuleTableOffset	equ	(MusicTableOffset+3)
