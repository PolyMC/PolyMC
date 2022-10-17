;***************************************************************************
;*                                                                         *
;*                            S P L I T   M U S                            *
;*                                                                         *
;*                            VARIABLES MODULE                             *
;*                                                                         *
;***************************************************************************

	include	spmus.i


	section	__MERGED,DATA

	xdef	DosName,DosBase

	xdef	Task,OutputFIB,MsgBuffer
	xdef	argc,argv
	xdef	SystemMemory
	xdef	Quiet,Verbose
	xdef	NoComp

	xdef	MUSName,OutBaseName,AsmOutName,OutName,SectBaseName
	xdef	MUSData,MUSDataEnd
	xdef	AsmFIB
	xdef	BankData
	xdef	Blocks,BlockLists
	xdef	NumModules,NumEffects,TurboModule
	xdef	ModuleNumBlocks
	xdef	Chunks,ChunksE,CChunks,CChunksE
	xdef	ChunkTable,ChunkTableE,ChunkLists,ChunkListsE
	xdef	ChunkIndex
	xdef	TurboTable


DosName			dc.b	'dos.library',0
			dc.w	0

DosBase			dc.l	0		; dos.library

Task			dc.l	0		; Address of Amiga_Task_Structure
OutputFIB		dc.l	0		; Output FIB
argc			dc.l	0
argv			ds.l	256
SystemMemory		dc.l	0		; Pointer to System Memory Block
MsgBuffer		ds.b	256		; Message Text Buffer
Quiet			dc.b	0
Verbose			dc.b	0
NoComp			dc.b	0		; No Compression
			dc.b	0

MUSData			dc.l	0		; MUS Object FileData
MUSDataEnd		dc.l	0		; MUS Object FileDataEnd

AsmFIB			dc.l	0

MUSName			dc.l	0		; Pointer to MUS Name
OutBaseName		dc.l	0		; Pointer to Output Base Name
SectBaseName		dc.l	0		; Pointer to Section Base Name
AsmOutName		dc.l	0		; Pointer to ASM Output Name

OutName			ds.b	256		; Name of OUTPUT OBJECT File

BankData		dc.l	0		; Pointer to Memory Bank Emulation
BlockLists		dc.l	0		; Base of Module BlockLists

NumModules		dc.l	0		; #Modules
NumEffects		dc.l	0		; #Effects
TurboModule		dc.l	0		; First Turbo Module#

ModuleNumBlocks		ds.l	MaxModules	; #Blocks in Each Module
Blocks			dc.l	0		; List of Unique Blocks

Chunks			dc.l	0		; Pointer to ChunkData
ChunksE			dc.l	0		; Pointer to ChunkData End

CChunks			dc.l	0		; Pointer to Compressed ChunkData
CChunksE		dc.l	0		; Pointer to Compressed ChunkData End

ChunkTable		dc.l	0		; Pointer to ChunkTable
ChunkTableE		dc.l	0		; Pointer to ChunkTable End

ChunkLists		dc.l	0		; Pointer to ChunkLists
ChunkListsE		dc.l	0		; Pointer to ChunkLists End

ChunkIndex		ds.w	MaxModules	; Offset to ChunkLists for Modules

TurboTable		dc.l	0		; Pointer to TurboTable


	end
