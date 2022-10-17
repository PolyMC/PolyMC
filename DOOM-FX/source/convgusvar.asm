;***************************************************************************
;*                                                                         *
;*                             C O N V   G U S                             *
;*                                                                         *
;*                            VARIABLES MODULE                             *
;*                                                                         *
;***************************************************************************

	include	convgus.i

	section	__MERGED,DATA

	xdef	DosName,DosBase

	xdef	PatchName
	xdef	PatchData
	xdef	OutputName
	xdef	OutputFileName

	xdef	Task,OutputFIB,MsgBuffer
	xdef	argc,argv
	xdef	SystemMemory
	xdef	Quiet,Verbose

	xdef	WaveCount


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

PatchName		dc.l	0		; Pointer to PatchName
PatchData		dc.l	0		; Pointer to PATCHDATA
OutputName		dc.l	0		; Pointer to OutputName Base
OutputFileName		ds.b	256		; OutputName Text

WaveCount		dc.l	0		; WaveCounter

	end
