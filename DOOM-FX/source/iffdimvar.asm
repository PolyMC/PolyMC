;***************************************************************************
;*                                                                         *
;*                             I F F   D I M                               *
;*                                                                         *
;*                            VARIABLES MODULE                             *
;*                                                                         *
;***************************************************************************

	include	iffdim.i

	section	__MERGED,DATA

	xdef	DosName,DosBase

	xdef	ListName,ListData
	xdef	IFFData,LevelData

	xdef	Task,OutputFIB,MsgBuffer
	xdef	argc,argv
	xdef	SystemMemory
	xdef	Quiet,Verbose
	xdef	LevelList,IFFList


DosName			dc.b	'dos.library',0
			dc.w	0

DosBase			dc.l	0		; dos.library

Task			dc.l	0		; Address of Amiga_Task_Structure
OutputFIB		dc.l	0		; Output FIB
argc			dc.l	0
argv			ds.l	256
SystemMemory		dc.l	0		; Pointer to System Memory Block
MsgBuffer		ds.b	1024		; Message Text Buffer
Quiet			dc.b	0
Verbose			dc.b	0

ListName		dc.l	0		; Pointer to ListName
ListData		dc.l	0		; Pointer to ListData

IFFData			dc.l	0		; Pointer to IFFData
LevelData		dc.l	0		; Pointer to LevelData

LevelList		dc.b	0		; -1 = LEVEL LIST
IFFList			dc.b	0		; -1 = IFF LIST

	end
