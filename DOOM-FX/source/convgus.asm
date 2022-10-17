;***************************************************************************
;*                                                                         *
;*                             C O N V   G U S                             *
;*                                                                         *
;*                               INIT MODULE                               *
;*                                                                         *
;***************************************************************************

	include	convgus.i

	xref	_LVOOpenLibrary
	xref	_LVOCloseLibrary
	xref	_LVOAllocEntry
	xref	_LVOFreeEntry
	xref	_LVOOutput
	xref	_LVOOpen
	xref	_LVOClose
	xref	_LVORead
	xref	_LVOWrite
	xref	_LVOSeek
	xref	_LVOFindTask

	xref	VDTDebugOutC
	xref	PrintMsg
	xref	GetArgs0
	xref	Quiet,Verbose

	xref	DosName,DosBase
	xref	OutputFIB
	xref	argc
	xref	argv
	xref	SystemMemory
	xref	Task

	xref	MSGNoMem
	xref	MSGInit
	xref	MSGNoPatchName
	xref	MSGNoOutputName

	xref	DoConvGUS

	xref	PatchName,PatchData
	xref	OutputName


	section	INIT,CODE

	xdef	ConvGUS

;
;	* * * * * * *       CONVERT GUS PATCH FILES       * * * * * * *
;
ConvGUS
	move.l	a0,argc				; Save pointer to Argc
	sub.l	a1,a1				; Find our Task Address
	move.l	4,a6
	jsr	_LVOFindTask(a6)
	move.l	d0,Task
	lea	DosName,a1			; dos.library
	moveq.l	#0,d0
	jsr	_LVOOpenLibrary(a6)
	move.l	d0,DosBase
;
	move.l	DosBase,a6
	jsr	_LVOOutput(a6)
	move.l	d0,OutputFIB			; Save OutputFIB
;
	bsr	GetArgs0			; Fill the Arguments array
	tst.l	d2
	bne	DoScanArgs
	move.l	#MSGInit,d7			; Show Init Message
	bsr	PrintMsg
	bra	Exit				; No arguments
DoScanArgs
	lea	argv,a1				; A1 points to list
ScanArgs
	move.l	(a1)+,d0			; D0 points to Argument
	beq	DoneArgs
	move.l	d0,a0				; A0 points to Argument
	move.b	(a0)+,d0
	cmp.b	#'-',d0				; Option specifier?
	bne	ScanArgs			; Nope
	move.b	(a0)+,d0
	cmp.b	#'P',d0				; -P	PATCH NAME?
	bne	ScanArgs100
	lea	PatchName,a0
SaveArg
	move.l	-4(a1),d0
	addq.l	#2,d0
	move.l	d0,(a0)
	bra	ScanArgs
ScanArgs100
	cmp.b	#'V',d0				; -V	VERBOSE
	bne	ScanArgs200
	move.b	#-1,Verbose
	bra	ScanArgs
ScanArgs200
	cmp.b	#'Q',d0				; -Q	QUIET
	bne	ScanArgs300
	move.b	#-1,Quiet
	bra	ScanArgs
ScanArgs300
	cmp.b	#'O',d0				; -O	OUTPUT NAME?
	bne	ScanArgs400
	lea	OutputName,a0
	bra	SaveArg
ScanArgs400
	bra	ScanArgs

DoneArgs
	tst.b	Quiet
	bne	DoneArgs100
	move.l	#MSGInit,d7			; Show Init Message
	bsr	PrintMsg
DoneArgs100
	move.l	4,a6
	lea	MemoryList,a0			; A0 = MemList
	jsr	_LVOAllocEntry(a6)
	move.l	#MSGNoMem,d7
	bclr	#31,d0				; Bit31 = 0 = OK, = 1 = BAD!
	bne	Error	
	move.l	d0,SystemMemory
	move.l	d0,a0
	lea	$10(a0),a0			; A0 points to MemList
	move.l	(a0),PatchData
;	addq.w	#8,a0
;
	move.l	#MSGNoPatchName,d7
	tst.l	PatchName
	beq	Error
;
	move.l	#MSGNoOutputName,d7
	tst.l	OutputName
	beq	Error
;
	jsr	DoConvGUS			; Convert GUS Patch
	tst.l	d7				; Any error msg?
	beq	Exit				; No
;
;	ERROR/EXIT	--	Close all, DeAllocate all, PrintMsg if any, and Exit!
;
Error
	bsr	PrintMsg
	bsr	Exit
Error900
	moveq.l	#1,d0
	rts
Exit
	move.l	4,a6
Exit300
	lea	SystemMemory,a0			; Free memory allocated
	tst.l	(a0)
	beq	Exit400
	move.l	(a0),a0
	jsr	_LVOFreeEntry(a6)
Exit400
	move.l	DosBase,a1			; dos.library
	jsr	_LVOCloseLibrary(a6)
	moveq.l	#0,d0
	rts


;
;	LIST OF MEMORY REQUIREMENTS
;
MemoryList
	ds.b	$e					; ListNode at top
	dc.w	(MemoryListEnd-MemoryListStart)/8	; # of Memory Entries
MemoryListStart
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; GUS PATCH FILE
	dc.l	(256*1024)
MemoryListEnd


	end
