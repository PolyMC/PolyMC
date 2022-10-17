;***************************************************************************
;*                                                                         *
;*                           S P L I T   M U S                             *
;*                                                                         *
;*                              INIT MODULE                                *
;*                                                                         *
;***************************************************************************

	include	spmus.i

	xref	_LVOOpenLibrary
	xref	_LVOCloseLibrary
	xref	_LVOAllocEntry
	xref	_LVOFreeEntry
	xref	_LVOOutput
	xref	_LVOOpen
	xref	_LVOClose
	xref	_LVORead
	xref	_LVOFindTask

	xref	VDTDebugOutC
	xref	PrintMsg
	xref	GetArgs0
	xref	Quiet,Verbose
	xref	NoComp

	xref	MUSName,MUSData,MUSDataEnd
	xref	OutBaseName,OutFIB
	xref	AsmOutName,AsmFIB
	xref	SectBaseName
	xref	BankData
	xref	BlockLists,Blocks
	xref	Chunks,ChunksE,ChunkTable,ChunkTableE,ChunkLists,ChunkListsE
	xref	TurboTable

	xref	DosName,DosBase
	xref	OutputFIB
	xref	argc
	xref	argv
	xref	SystemMemory
	xref	Task

	xref	MSGNoMem
	xref	MSGInit
	xref	MSGNoMUS
	xref	MSGNoOut
	xref	MSGNoAsm
	xref	MSGNoSect
	xref	MSGMUSError

	xref	DoSplitMUS


	section	INIT,CODE

	xdef	SplitMUS

;
;	* * * * * * *       SPLIT MUSIC FILES       * * * * * * *
;
SplitMUS
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
	cmp.b	#'M',d0				; -M	MUS NAME?
	bne	ScanArgs50
	lea	MUSName,a0
SaveArg
	move.l	-4(a1),d0
	addq.l	#2,d0
	move.l	d0,(a0)
	bra	ScanArgs
ScanArgs50
	cmp.b	#'O',d0				; -O	OUTPUT BASE NAME?
	bne	ScanArgs100
	lea	OutBaseName,a0
	bra	SaveArg
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
	cmp.b	#'A',d0				; -A	OUTPUT ASM NAME?
	bne	ScanArgs400
	lea	AsmOutName,a0
	bra	SaveArg
ScanArgs400
	cmp.b	#'S',d0				; -S	SECTION BASE NAME?
	bne	ScanArgs500
	lea	SectBaseName,a0
	bra	SaveArg
ScanArgs500
	cmp.b	#'N',d0				; -N	NO COMPRESSION?
	bne	ScanArgs600
	move.b	#-1,NoComp
	bra	ScanArgs
ScanArgs600
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
	move.l	(a0),MUSData
	addq.w	#8,a0
	move.l	(a0),BankData
	addq.w	#8,a0
	move.l	(a0),Chunks
	move.l	(a0),ChunksE
	addq.w	#8,a0
	move.l	(a0),BlockLists
	addq.w	#8,a0
	move.l	(a0),Blocks
	addq.w	#8,a0
	move.l	(a0),ChunkTable
	move.l	(a0),ChunkTableE
	addq.w	#8,a0
	move.l	(a0),ChunkLists
	move.l	(a0),ChunkListsE
	addq.w	#8,a0
	move.l	(a0),TurboTable
;
	move.l	#MSGNoMUS,d7
	move.l	MUSName,d1			; MUS specified?
	beq	Error				; No
	move.l	#MSGNoOut,d7
	move.l	OutBaseName,d1			; OUTPUT BASE specified?
	beq	Error				; No
	move.l	#MSGNoAsm,d7
	move.l	AsmOutName,d1			; OUTPUT ASM specified?
	beq	Error				; No
	move.l	#MSGNoSect,d7
	move.l	SectBaseName,d1			; SECTION BASE specified?
	beq	Error				; No
;
	move.l	DosBase,a6			; Read the MUS File
	move.l	MUSName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGMUSError,d7
	move.l	d0,d4
	beq	Error
	move.l	d4,d1
	move.l	MUSData,d2
	move.l	#(512*1024),d3
	jsr	_LVORead(a6)
	add.l	MUSData,d0
	move.l	d0,MUSDataEnd
	move.l	d4,d1
	jsr	_LVOClose(a6)
;
	jsr	DoSplitMUS			; Split the MUS File
;
	tst.l	d7
	bne	Error
	bra	Exit
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
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; MUS DATA FILE
	dc.l	(512*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; MEMORY BANK EMULATION
	dc.l	(512*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; CHUNK DATA
	dc.l	(512*1024)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; MODULE BLOCKLISTS
	dc.l	(MaxModules*MaxModuleBlocks*16)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; BLOCKS
	dc.l	(MaxBlocks*8)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; CHUNKTABLE
	dc.l	(MaxChunks*16)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; MODULE CHUNKLISTS
	dc.l	(MaxModules*MaxModuleChunks*16)
	dc.l	(MEMF_PUBLIC!MEMF_CLEAR)		; TURBO TABLE
	dc.l	(MaxEffects*6)
MemoryListEnd


	end
