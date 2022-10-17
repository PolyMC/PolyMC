;***************************************************************************
;*                                                                         *
;*                           S P L I T   M U S                             *
;*                                                                         *
;*                              MAIN MODULE                                *
;*                                                                         *
;***************************************************************************

	include	spmus.i

	xref	DosName,DosBase
	xref	OutputFIB
	xref	Task

	xref	_LVOOpen,_LVOClose,_LVOWrite

	xref	VDTDebugOutC,PrintMsg,GetArgs0

	xref	MUSData,MUSDataEnd
	xref	OutBaseName,OutFIB
	xref	OutName
	xref	AsmOutName,AsmFIB
	xref	SectBaseName
	xref	BankData
	xref	Blocks,BlockLists
	xref	NumModules,NumEffects,TurboModule
	xref	ModuleNumBlocks

	xref	Chunks,ChunksE,CChunks,CChunksE
	xref	ChunkTable,ChunkTableE,ChunkLists,ChunkListsE
	xref	ChunkIndex
	xref	TurboTable

	xref	MSGUserBreak
	xref	MSGNewLine
	xref	MSGMUSTable
	xref	MSGBadMUS
	xref	MSGOUTError

	xref	NoComp

	xref	CompRLC


	section	MAIN,CODE

	xdef	DoSplitMUS
	xdef	ReadMUS

	xdef	BuildBlockLists
	xdef	DumpBlockLists

	xdef	GroupBlocks
	xdef	MergeBlocks
	xdef	BuildBlocks

	xdef	DumpBlocks

	xdef	BuildChunks
	xdef	CompressChunks
	xdef	DumpChunks

	xdef	DumpModules

	xdef	ReadAPUByte
	xdef	BuildTurboTable


;
;	* * * * * * *       SPLIT MUSIC FILES       * * * * * * *
;
DoSplitMUS
	bsr	ReadMUS				; Read MUS File into Emulation ROM
	bne.s	DoSplitMUS9

	bsr	BuildBlockLists			; Build Module BlockLists
;
	tst.b	NoComp
	bne.s	DoSplitMUS2
	bsr	GroupBlocks			; Group Unique Blocks into Common
	bne.s	DoSplitMUS9
	bsr	MergeBlocks			; Merge Contiguous Blocks
	bne.s	DoSplitMUS9
DoSplitMUS2
	bsr	DumpBlockLists			; Display Module BlockLists
	bne.s	DoSplitMUS9
;
	bsr	BuildBlocks			; Build Blocks
	bne.s	DoSplitMUS9
	bsr	DumpBlocks			; Display Blocks
	bne.s	DoSplitMUS9
;
	bsr	BuildChunks			; Build Chunks
	bne.s	DoSplitMUS9
;
	bsr	CompressChunks			; Compress Chunks
	bne.s	DoSplitMUS9
	bsr	DumpChunks			; Display Chunks
	bne.s	DoSplitMUS9
;
	bsr	BuildTurboTable			; Build TurboTable
;
	bsr	DumpModules			; Display Modules
	bne.s	DoSplitMUS9
;
	bsr	SaveBoot			; Save BootData
	bne.s	DoSplitMUS9
	bsr	SaveDriver			; Save DriverData
	bne.s	DoSplitMUS9
	bsr	SaveChunks			; Save ChunkData
	bne.s	DoSplitMUS9
	bsr	SaveChunkTable			; Save ChunkTable
	bne.s	DoSplitMUS9
	bsr	SaveChunkLists			; Save ChunkLists
	bne.s	DoSplitMUS9
	bsr	SaveChunkIndex			; Save ChunkIndex
	bne.s	DoSplitMUS9
	bsr	SaveTurboTable			; Save TurboTable
DoSplitMUS9
	rts


;
;	* * * * * * *       BUILD BLOCKLISTS       * * * * * * *
;
;	FOR EACH MODULE:
;		CREATE A BLOCKLIST OF:	ROM.L,APU.W,SIZE.W (DISCARD SIZE=0)
;		SORT BLOCKLIST BY APU.W
;
BuildBlockLists
	move.l	BankData,a5			; Get #MODULES
	add.l	#MusicTableOffset,a5
	moveq.l	#0,d6				; D6 = MAX MODULES
	move.b	(a5),d6
	move.l	d6,NumModules
	moveq.l	#0,d5				; D5 = MODULE#
	lea	ModuleNumBlocks,a2		; A2 = POINTER TO #BLOCKS
;
;	>>>   NEXT MODULE   <<<
;
BBL200
	move.l	BankData,a5			; Move to MODULE TABLE
	add.l	#ModuleTableOffset,a5
	move.l	d5,d0
	mulu	#3,d0
	add.l	d0,a5
	moveq.l	#0,d0				; Get ORIGIN ADDRESS
	move.b	2(a5),d0
	lsl.w	#8,d0
	move.b	1(a5),d0
	lsl.l	#8,d0
	move.b	0(a5),d0
	and.l	#$007fffff,d0			; SLOWROM
	move.l	d0,d1				; Get LOAD ADDRESS
	and.l	#$7fff,d1
	and.l	#$00ff0000,d0
	lsr.l	#1,d0
	add.l	d0,d1
	move.l	BankData,a5			; A5 = MUS Data Pointer
	add.l	d1,a5
	sub.l	#MusicLoadOffset,a5
;
	move.l	d5,d0				; A4 = BLOCKLIST POINTER
	mulu.l	#(MaxModuleBlocks*16),d0
	move.l	BlockLists,a4
	add.l	d0,a4
	move.l	a4,a3				; A3 = BLOCKLIST POINTER END
;
	clr.l	(a2)				; #BLOCKS ADDED=0
;
	moveq.l	#0,d7				; D7 = #BLOCKS
	move.b	1(a5),d7
	lsl.w	#8,d7
	move.b	0(a5),d7
	addq.w	#2,a5
	bra.s	BBL700
;
;	>>>   NEXT BLOCK   <<<
;
BBL300
	move.w	#$0fff,$dff180
	moveq.l	#0,d3				; APU ADDRESS.W
	move.b	6(a5),d3
	lsl.w	#8,d3
	move.b	5(a5),d3
;
;	>>>   FIND INSERTION POINT   <<<
;
	move.l	a4,a1
BBL400
	cmp.l	a1,a3				; AT END?
	beq.s	BBL600
	cmp.l	4(a1),d3			; FOUND INSERTION POINT?
	ble.s	BBL500				; YES!
	add.w	#16,a1
	bra.s	BBL400
;
;	>>>   INSERT NEW BLOCKLIST ENTRY   <<<
;
BBL500
	move.l	a3,a0				; Insert Empty BlockList Entry!
BBL520
	move.l	-(a0),16(a0)
	cmp.l	a0,a1
	bne.s	BBL520
;
;	>>>   STORE NEW BLOCKLIST ENTRY   <<<
;
BBL600
	move.l	d3,4(a1)			; APU ADDRESS.W
;
	moveq.l	#0,d0				; BLOCK SIZE.W
	move.b	1(a5),d0
	lsl.w	#8,d0
	move.b	0(a5),d0
	move.l	d0,8(a1)
;
	moveq.l	#0,d0				; ROM ADDRESS.L
	move.b	4(a5),d0
	lsl.w	#8,d0
	move.b	3(a5),d0
	lsl.l	#8,d0
	move.b	2(a5),d0
;
	move.l	d0,d1				; LOAD ADDRESS.L
	and.l	#$7fff,d1
	and.l	#$00ff0000,d0
	lsr.l	#1,d0
	add.l	d0,d1
	move.l	d1,0(a1)
;
	addq.l	#1,(a2)				; #BLOCKS++
	add.w	#16,a3
	addq.w	#7,a5				; Next BLOCKHEADER
BBL700
	dbf	d7,BBL300			; Next Block
;
;	>>>   NEXT MODULE   <<<
;
	addq.w	#1,d5				; Next Module
	addq.w	#4,a2
	dbf	d6,BBL200
	rts


;
;	* * * * * * *       MERGE BLOCKLIST BLOCKS       * * * * * * *
;
MergeBlocks
	move.l	NumModules,d6			; D6 = #MODULES
	moveq.l	#0,d5				; D5 = MODULE#
	lea	ModuleNumBlocks,a3		; A3 = POINTER TO #BLOCKS
;
;	>>>   NEXT MODULE   <<<
;
MBL200
	move.l	Task,a0				; Quit?
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	beq	MBL220				; No
	move.l	#MSGUserBreak,d7
	bra	MBL900				; Yes!
MBL220
;
	move.l	d5,d0				; A4 = BLOCKLIST POINTER
	mulu.l	#(MaxModuleBlocks*16),d0
	move.l	BlockLists,a4
	add.l	d0,a4
;
	move.l	(a3),d7				; D7 = #BLOCKS
	bra.s	MBL800
;
;	>>>   NEXT BLOCK   <<<
;
MBL300
	move.w	#$000f,$dff180
	move.l	8(a4),d0			; D0 = BLOCKSIZE
	beq.s	MBL700				; *EMPTY BLOCK*
	move.l	0(a4),d4			; D4 = ROM ADDRESS END
	add.l	d0,d4
	move.l	4(a4),d3			; D3 = APU ADDRESS END
	add.l	d0,d3
;
;	>>>   SCAN FOR CONTIGUOUS BLOCK   <<<
;
	move.l	a4,a0
	move.l	d7,d1
	bra.s	MBL600
MBL400
	cmp.l	0(a0),d4			; ROM ADDRESS START = CURRENT BLOCK END?
	bne.s	MBL500
	cmp.l	4(a0),d3			; APU ADDRESS START = CURRENT BLOCK END?
	bne.s	MBL500
	move.l	8(a0),d0			; CURRENT BLOCK SIZE += MERGED BLOCK SIZE
	add.l	d0,8(a4)
	add.l	d0,d3
	add.l	d0,d4
	clr.l	8(a0)				; MERGED BLOCK SIZE = 0
MBL500
	add.w	#16,a0				; Next Block
MBL600
	dbf	d1,MBL400			; Keep Scanning
;
;	>>>   NEXT BLOCK   <<<
;
MBL700
	add.w	#16,a4				; Next BLOCKLIST ENTRY
MBL800
	dbf	d7,MBL300			; Next Block
;
;	>>>   NEXT MODULE   <<<
;
	addq.w	#1,d5				; Next Module
	addq.w	#4,a3
	dbf	d6,MBL200
	moveq.l	#0,d7
MBL900
	rts


;
;	* * * * * * *       GROUP BLOCKS       * * * * * * *
;
GroupBlocks
	move.l	NumModules,d6			; D6 = #MODULES
	moveq.l	#0,d5				; D5 = MODULE#
	lea	ModuleNumBlocks,a3		; A3 = POINTER TO #BLOCKS
	bra	GBL950				; Don't Group Module#0!
;
;	>>>   NEXT MODULE   <<<
;
GBL200
	move.l	Task,a0				; Quit?
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	beq	GBL220				; No
	move.l	#MSGUserBreak,d7
	bra	GBL990				; Yes!
GBL220
	move.l	d5,d0				; A4 = BLOCKLIST POINTER
	mulu.l	#(MaxModuleBlocks*16),d0
	move.l	BlockLists,a4
	add.l	d0,a4
	move.l	(a3),d7				; D7 = #BLOCKS
	bra	GBL940
;
;	>>>   NEXT BLOCK   <<<
;
GBL300
	move.w	#$0f00,$dff180
	move.l	8(a4),d4			; D4 = BLOCKSIZE
	beq	GBL920				; *EMPTY BLOCK*
	move.l	4(a4),d3			; D3 = APU ADDRESS
	add.l	d3,d4				; D4 = APU ADDRESS END
;
;	>>>   SCAN FOR APU MEMORY OVERLAP   <<<
;
	move.l	d6,-(sp)
	move.l	NumModules,d6			; D6 = #MODULES
	moveq.l	#0,d2				; D2 = MODULE#
	lea	ModuleNumBlocks,a1		; A1 = POINTER TO #BLOCKS
GBL400
	move.l	(a1)+,d1			; D1 = #BLOCKS
	cmp.w	d2,d5				; Same Module?
	beq.s	GBL480				; Yes!
	move.l	d2,d0				; A2 = BLOCKLIST POINTER
	mulu.l	#(MaxModuleBlocks*16),d0
	move.l	BlockLists,a2
	add.l	d0,a2
	bra.s	GBL460
GBL420
	tst.l	8(a2)				; *EMPTY BLOCK*
	beq.s	GBL450
	move.l	4(a2),d0			; D0 = APU ADDRESS
	cmp.l	d4,d0				; START AFTER BLOCK ENDS?
	bge.s	GBL450
	add.l	8(a2),d0			; ENDS BEFORE BLOCK STARTS?
	cmp.l	d3,d0
	bgt.s	GBL900				; NO!  BLOCK OVERLAPS!
GBL450
	add.w	#16,a2				; Next BlockList Entry
GBL460
	dbf	d1,GBL420			; Keep Scanning
GBL480
	addq.w	#1,d2				; Next Module
	dbf	d6,GBL400
;
;	>>>   MOVE BLOCK TO COMMON   <<<
;
	move.l	BlockLists,a1			; A1 = BLOCKLIST FOR MODULE#0
	move.l	ModuleNumBlocks,d1		; D1 = #BLOCKS FOR MODULE#0
	bra.s	GBL740
GBL700
	cmp.l	4(a1),d3			; FOUND INSERTION POINT?
	ble.s	GBL750				; YES!
	add.w	#16,a1
GBL740
	dbf	d1,GBL700			; Next Block
	bra.s	GBL800				; APPEND!
;
;	>>>   INSERT NEW BLOCKLIST ENTRY   <<<
;
GBL750
	move.l	BlockLists,a0			; A0 = BLOCKLIST END FOR MODULE#0
	move.l	ModuleNumBlocks,d0
	mulu.l	#16,d0
	add.l	d0,a0
GBL760
	move.l	-(a0),16(a0)
	cmp.l	a0,a1
	bne.s	GBL760
;
;	>>>   COPY BLOCKLIST ENTRY   <<<
;
GBL800
	move.l	0(a4),(a1)+			; COPY BLOCK TO COMMON
	move.l	4(a4),(a1)+
	move.l	8(a4),(a1)+
	move.l	12(a4),(a1)+
	clr.l	8(a4)				; MOVED BLOCK SIZE = 0
	lea	ModuleNumBlocks,a0		; #MODULE0 BLOCKS++
	addq.l	#1,(a0)
;
;	>>>   NEXT BLOCK   <<<
;
GBL900
	move.l	(sp)+,d6
GBL920
	add.w	#16,a4				; Next BLOCKLIST ENTRY
GBL940
	dbf	d7,GBL300			; Next Block
;
;	>>>   NEXT MODULE   <<<
;
GBL950
	addq.w	#1,d5				; Next Module
	addq.w	#4,a3
	dbf	d6,GBL200
	moveq.l	#0,d7
GBL990
	rts


;
;	* * * * * * *       BUILD BLOCKS       * * * * * * *
;
BuildBlocks
	move.l	NumModules,d6			; D6 = #MODULES
	moveq.l	#0,d5				; D5 = MODULE#
	lea	ModuleNumBlocks,a3		; A3 = POINTER TO #BLOCKS
	move.l	Blocks,a5			; A5 = POINTER TO BLOCKS
;
;	>>>   NEXT MODULE   <<<
;
BLK200
	move.l	Task,a0				; Quit?
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	beq	BLK220				; No
	move.l	#MSGUserBreak,d7
	bra	BLK900				; Yes!
BLK220
	move.l	d5,d0				; A4 = BLOCKLIST POINTER
	mulu.l	#(MaxModuleBlocks*16),d0
	move.l	BlockLists,a4
	add.l	d0,a4
	move.l	(a3),d7				; D7 = #BLOCKS
	bra.s	BLK800
;
;	>>>   NEXT BLOCK   <<<
;
BLK300
	move.w	#$00f0,$dff180
	move.l	8(a4),d0			; D0 = BLOCKSIZE
	beq.s	BLK700				; *EMPTY BLOCK*
	move.l	0(a4),d1			; D1 = ROM ADDRESS
;
;	>>>   SCAN FOR DUPLICATE BLOCK   <<<
;
	move.l	Blocks,a0			; A0 = POINTER TO BLOCKS
BLK400
	cmp.l	a0,a5				; End of Blocks?
	beq.s	BLK600				; YES!  NEW BLOCK!
	cmp.l	0(a0),d1			; Same LOAD?
	bne.s	BLK500				; No
	cmp.l	4(a0),d0			; Same SIZE?
	beq.s	BLK700				; YES!  SAME BLOCK!
BLK500
	add.w	#8,a0
	bra.s	BLK400
;
;	>>>   FIND INSERTION POINT   <<<
;
BLK600
	move.l	Blocks,a1			; A1 = POINTER TO BLOCKS
BLK620
	cmp.l	a1,a5				; AT END?
	beq.s	BLK680
	cmp.l	0(a1),d1			; FOUND INSERTION POINT?
	ble.s	BLK650				; YES!
	add.w	#8,a1
	bra.s	BLK620
;
;	>>>   INSERT NEW BLOCK ENTRY   <<<
;
BLK650
	move.l	a5,a0				; Insert Empty BlockList Entry!
BLK660
	move.l	-(a0),8(a0)
	cmp.l	a0,a1
	bne.s	BLK660
;
;	>>>   NEW BLOCK   <<<
;
BLK680
	move.l	d1,(a1)				; LOAD.L
	move.l	d0,4(a1)			; SIZE.L
	add.w	#8,a5
;
;	>>>   NEXT BLOCK   <<<
;
BLK700
	add.w	#16,a4				; Next BLOCKLIST ENTRY
BLK800
	dbf	d7,BLK300			; Next Block
;
;	>>>   NEXT MODULE   <<<
;
	addq.w	#1,d5				; Next Module
	addq.w	#4,a3
	dbf	d6,BLK200
;
	clr.l	(a5)+				; Terminate BLOCKS
	moveq.l	#0,d7
BLK900
	rts


;
;	* * * * * * *       DUMP BLOCKLISTS       * * * * * * *
;
DumpBlockLists
	move.l	NumModules,d6			; Get #MODULES
	moveq.l	#0,d5				; D5 = MODULE#
;
;	>>>   NEXT MODULE   <<<
;
DBL200
	move.l	d5,d0				; A4 = BLOCKLIST POINTER
	mulu.l	#(MaxModuleBlocks*16),d0
	move.l	BlockLists,a4
	add.l	d0,a4
	move.l	d5,d0				; D4 = #BLOCKS
	lsl.w	#2,d0
	lea	ModuleNumBlocks,a0
	move.l	(a0,d0.w),d4
;
	moveq.l	#0,d1				; D1 = #BLOCKS USED
	move.l	d4,d2
	move.l	a4,a0
	bra.s	DBL380
DBL300
	tst.l	8(a0)				; *EMPTY BLOCK*?
	beq.s	DBL360
	addq.l	#1,d1				; NO!
DBL360
	add.w	#16,a0
DBL380
	dbf	d2,DBL300
;
	move.l	d4,-(sp)
	move.l	d1,-(sp)			; D1 = #BLOCKS USED
	move.l	d5,-(sp)
	lea	BlockListMsg,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(3*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	bra.s	DBL700
;
;	>>>   NEXT BLOCK   <<<
;
DBL400
	move.l	Task,a0				; Quit?
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	beq	DBL500				; No
	move.l	#MSGUserBreak,d7
	bra	DBL900				; Yes!
DBL500
	move.l	8(a4),d0			; BLOCK SIZE.W
	beq.s	DBL600				; *EMPTY BLOCK*
	move.l	d0,-(sp)
	move.l	4(a4),-(sp)			; APU ADDRESS.W
	move.l	0(a4),-(sp)			; ROM ADDRESS.L
	lea	BlockListMsg2,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(3*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
DBL600
	add.w	#16,a4				; Next BlockList Entry
DBL700
	dbf	d4,DBL400			; Next Block
;
;	>>>   NEXT MODULE   <<<
;
	addq.w	#1,d5				; Next Module
	dbf	d6,DBL200
	moveq.l	#0,d7
DBL900
	rts


;
;	* * * * * * *       DUMP BLOCKS       * * * * * * *
;
DumpBlocks
	move.l	Blocks,a5			; A5 = BLOCKS POINTER
	move.l	a5,a0
DBK200
	tst.l	(a0)				; END?
	beq.s	DBK300
	add.w	#8,a0
	bra.s	DBK200
DBK300
	sub.l	a5,a0
	move.l	a0,d0
	lsr.l	#2,d0	
	move.l	d0,-(sp)			; #BLOCKS
	lea	BlockMsg,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(1*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	(a5),d6				; D6 = LOAD ADDRESS
	moveq.l	#0,d5				; D5 = USED SIZE
	moveq.l	#0,d4				; D4 = UNUSED SIZE
	move.l	#MSGUserBreak,d7
DBK400
	move.l	Task,a0				; Quit?
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	DBK900				; Yes!
	move.l	(a5)+,d3			; D3 = LOAD.L
	beq.s	DBK800				; *END*
	move.l	(a5)+,d2			; D2 = SIZE.L
;
;	>>>   TALLY USED/UNUSED SIZE   <<<
;
	cmp.l	d6,d3				; LOAD ADDRESS CONTIGUOUS?
	beq.s	DBK600				; YES!
;
;	>>>   UNUSED   <<<
;
DBK500
	move.l	d3,d0				; D0 = #UNUSED BYTES
	sub.l	d6,d0
	add.l	d0,d4				; UNUSED SIZE += BLOCK BREAK SIZE
	move.l	d3,d6				; LOAD ADDRESS = BLOCK ADDRESS
	add.l	d2,d6				; LOAD ADDRESS += BLOCK SIZE
	lea	BlockMsg2U,a0
	bra.s	DBK700
;
;	>>>   USED   <<<
;
DBK600
	add.l	d2,d6				; LOAD ADDRESS += BLOCK SIZE
	add.l	d2,d5				; USED SIZE += BLOCK SIZE
	lea	BlockMsg2,a0
;
;	>>>   NEXT BLOCK   <<<
;
DBK700
	move.l	d2,-(sp)
	move.l	d3,-(sp)
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(2*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	bra.s	DBK400
DBK800
	move.l	d4,-(sp)			; UNUSED SIZE
	move.l	d5,-(sp)			; USED SIZE
	add.l	d4,d5				; TOTAL SIZE
	move.l	d5,-(sp)
	lea	BlockMsg3,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(3*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	moveq.l	#0,d7
DBK900
	rts


;
;	* * * * * * *       BUILD CHUNKS       * * * * * * *
;
BuildChunks
	move.l	Chunks,a6			; A6 = ChunkData
	move.l	ChunkTable,a5			; A5 = ChunkTable
	move.l	ChunkLists,a4			; A4 = ChunkLists
BCKS1000
	moveq.l	#0,d7				; D7 = MODULE#
;
;	>>>   NEXT MODULE   <<<
;
BCKS2000
	move.l	d7,d0				; CREATE CHUNKINDEX
	lsl.w	#1,d0
	lea	ChunkIndex,a0
	add.w	d0,a0
	move.l	a4,d0
	sub.l	ChunkLists,d0
	move.b	d0,(a0)+
	lsr.w	#8,d0
	move.b	d0,(a0)+
;
	move.l	d7,d0				; A3 = BLOCKLIST POINTER
	mulu.l	#(MaxModuleBlocks*16),d0
	move.l	BlockLists,a3
	add.l	d0,a3
	move.l	d7,d0				; D4 = #BLOCKS
	lsl.w	#2,d0
	lea	ModuleNumBlocks,a0
	move.l	(a0,d0.w),d4
	moveq.l	#0,d6				; D6 = APUADDRESS
	moveq.l	#1,d5				; D5 = *NOT* UNIQUE
	bra	BCKS7700
;
;	>>>   NEXT BLOCK   <<<
;
BCKS3000
	move.w	a3,$dff180
	tst.l	8(a3)				; *EMPTY BLOCK*?
	beq	BCKS7600			; Yes
;
;	>>>   CONTIGUOUS TRANSFER?   <<<
;
	move.l	4(a3),d0			; Get Next Block's APU Address
	cmp.l	d0,d6
	beq	BCKS4000			; Contiguous
	tst.l	d6				; NON-CONTIGUOUS!
	beq.s	BCKS3500			; Any ChunkList Already?
	move.w	#$0080,(a4)+			; Yes!  Terminate ChunkList
BCKS3500
	move.l	d0,d6
	move.b	d0,(a4)+			; Start New ChunkList
	lsr.w	#8,d0
	move.b	d0,(a4)+
	moveq.l	#1,d5				; D5 = *NOT* UNIQUE
;
;	>>>   ALREADY ADDED THIS CHUNK?   <<<
;
BCKS4000
	moveq.l	#1,d3				; D3 = NEW *NOT* UNIQUE
	move.l	0(a3),d1			; D1 = OLD LOAD
	move.l	8(a3),d2			; D2 = NEW SIZE
	move.l	ChunkTable,a2			; A2 = ChunkTable
BCKS4200
	cmp.l	a5,a2				; End of ChunkTable?
	beq.s	BCKS5000			; Yes!  Chunk Not Added!
	cmp.l	8(a2),d1			; Found Correct Chunk?
	beq	BCKS7000			; Yes!  Chunk Added!
	add.w	#16,a2
	bra.s	BCKS4200
;
;	>>>   TALLY CHUNK REFERENCES IN BLOCKLISTS   <<<
;
BCKS5000
	moveq.l	#0,d3				; D3 = NEW UNIQUE
	movem.l	d2/d6,-(sp)
	moveq.l	#0,d6				; D6 = MODULE#
	lea	ModuleNumBlocks,a1		; A1 = POINTER TO #BLOCKS
BCKS5200
	move.l	(a1)+,d2			; D2 = #BLOCKS
	cmp.w	d6,d7				; Same Module?
	beq.s	BCKS5800			; Yes!
	move.l	d6,d0				; A0 = BLOCKLIST POINTER
	mulu.l	#(MaxModuleBlocks*16),d0
	move.l	BlockLists,a0
	add.l	d0,a0
	bra.s	BCKS5600
BCKS5300
	tst.l	8(a0)				; *EMPTY BLOCK*
	beq.s	BCKS5500
	cmp.l	0(a0),d1			; SAME LOAD?
	bne.s	BCKS5500			; No
	addq.l	#1,d3
BCKS5500
	add.w	#16,a0				; Next BlockList Entry
BCKS5600
	dbf	d2,BCKS5300			; Keep Scanning
BCKS5800
	addq.w	#1,d6				; Next Module
	cmp.l	NumModules,d6
	ble	BCKS5200
	movem.l	(sp)+,d2/d6

;
;	>>>   ADD CHUNK TO CHUNKTABLE   <<<
;
;BCKS6000
	move.l	a6,d0				; LOAD ADDRESS.L
	sub.l	Chunks,d0
	move.l	d0,(a5)+
	move.l	d2,(a5)+			; SIZE.L
	move.l	0(a3),(a5)+			; OLD LOAD.L
	addq.w	#4,a5
;
;	>>>   ADD CHUNK TO CHUNKDATA   <<<
;
	sub.l	#MusicLoadOffset,d1		; Get Offset within Emulation ROM
	move.l	BankData,a0
	add.l	d1,a0
	move.l	d2,d0
	bra.s	BCKS6300
BCKS6200
	move.b	(a0)+,(a6)+			; Copy ChunkData
BCKS6300
	dbf	d0,BCKS6200
;
;	>>>   APPENDING TO A UNIQUE CHUNK?   <<<
;
	tst.l	d5				; Old Chunk Unique?
	bne	BCKS7000			; No
;
;	>>>   NEW CHUNK UNIQUE?   <<<
;
	tst.l	d3				; New Chunk Unique?
	bne	BCKS7000			; No, Can't Merge It!
	sub.w	#16,a5				; YES!  Discard Added ChunkTable Entry!
	add.l	d2,-12(a5)			; Merge OLD and NEW Unique Chunks!
	bra	BCKS7500
;
;	>>>   ADD CHUNK TO CHUNKLIST   <<<
;
BCKS7000
	move.l	a2,d0				; Get Chunk#
	sub.l	ChunkTable,d0
	lsr.l	#4,d0
	move.b	d0,(a4)+
	lsr.w	#8,d0
	move.b	d0,(a4)+
;
;	>>>   FINISHED THIS BLOCK!   <<<
;
BCKS7500
	move.l	d3,d5				; Unique = NewUnique
	add.l	d2,d6				; APUADDRESS += SIZE
BCKS7600
	add.w	#16,a3				; Next Block
BCKS7700
	dbf	d4,BCKS3000
;
;	>>>   FINISHED THIS MODULE!   <<<
;
BCKS8000
	tst.l	d6				; ANY ChunkList?
	beq.s	BCKS8200			; NO!
	move.w	#$0080,(a4)+			; Terminate ChunkList
BCKS8200
	move.w	#$0000,(a4)+			; Terminate Module
;
	addq.l	#1,d7				; Next Module#
	cmp.l	NumModules,d7
	ble	BCKS2000
;
	move.l	a6,ChunksE			; Ending Address of ChunkData
	move.l	a5,ChunkTableE			; Ending Address of ChunkTable
	move.l	a4,ChunkListsE			; Ending Address of ChunkLists
	moveq.l	#0,d7
	rts


;
;	* * * * * * *       COMPRESS ALL CHUNKS       * * * * * * *
;
CompressChunks
	move.l	MUSData,a2			; A2 = Compressed Chunks Pointer
	move.l	a2,CChunks
;
	move.l	ChunkTable,a6			; A6 = ChunkTable
	moveq.l	#0,d5				; D5 = TOTAL SAVINGS
	moveq.l	#0,d6				; D6 = TOTAL ORIGINAL SIZE
;
;	>>>   NEXT CHUNK   <<<
;
CCKS2000
	move.l	#MSGUserBreak,d7
	move.l	Task,a0				; Quit?
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	CCKS9000			; Yes!
;
	cmp.l	ChunkTableE,a6			; At End of Chunks?
	beq	CCKS8000			; YES!
;
;	>>>   IS THIS CHUNK A TURBO SOUNDEFFECT?   <<<
;
	move.l	BankData,a5			; A5 = EFFECTS MODULE TABLE
	add.l	#(MusicTableOffset+3),a5
	move.l	NumModules,d0			; Skip Module Address Table
	addq.l	#1,d0
	mulu.l	#3,d0
	add.l	d0,a5
	move.l	BankData,a0			; Skip Song Module Table
	add.l	#MusicTableOffset,a0
	moveq.l	#0,d0
	move.b	1(a0),d0
	add.l	d0,a5
	move.b	2(a0),d0			; Skip Effect Module Table
	addq.l	#4,d0				; Skip BlastAddress.W/BlastSize.W
	lea	(a5,d0.l),a4			; A4 = EFFECTS BLAST TABLE
	move.l	BankData,a0			; D4 = #EFFECTS
	add.l	#MusicTableOffset,a0
	moveq.l	#0,d4
	move.b	2(a0),d4
	moveq.l	#0,d2				; D2 = EFFECT#
	bra	CCKS3800
;
;	>>>   CHECK NEXT EFFECT'S MODULE   <<<
;
CCKS3000
	tst.w	(a4)				; Regular/Turbo Effect?
	beq	CCKS3600			; Regular
	moveq.l	#0,d0				; Get Effect Module
	move.b	(a5),d0
	lsl.l	#1,d0				; Get Effect Module ChunkIndex
	lea	ChunkIndex,a0
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	1(a0),d0
	lsl.w	#8,d0
	move.b	0(a0),d0
	move.l	ChunkLists,a0			; A0 = ChunkLists
	add.l	d0,a0
	addq.w	#2,a0				; Skip APU Address
	moveq.l	#0,d0				; Get Chunk#
	move.b	1(a0),d0
	lsl.w	#8,d0
	move.b	0(a0),d0
	move.l	ChunkTable,a0			; A0 = ChunkTable
	lsl.l	#4,d0
	add.l	d0,a0
	cmp.l	a0,a6				; Is this Chunk a TurboChunk?
	beq.s	CCKS4000			; Yes!  Can't Compress!
CCKS3600
	addq.l	#1,d2				; Next Effect
	addq.w	#2,a4
	addq.w	#1,a5
CCKS3800
	dbf	d4,CCKS3000
CCKS4000
;
;	>>>   UPDATE LOAD ADDRESS   <<<
;
	move.l	0(a6),a0			; A0 = ChunkData Offset
	add.l	Chunks,a0
;
	move.l	a2,d0				; LOAD ADDRESS.L
	sub.l	CChunks,d0
	move.l	d0,0(a6)
;
;	>>>   COMPRESS THIS CHUNK   <<<
;
	move.l	4(a6),-(sp)			; ORIGINAL SIZE.L
	move.l	4(a6),d2			; D2 = SIZE.L
	tst.w	d4				; COMPRESS THIS CHUNK?
	bpl.s	CCKS6300			; NO!
;	cmp.l	#8,d2				; At least 8 bytes?
;	blt.s	CCKS6300			; No!  Don't Compress!
	movem.l	a0/a2,-(sp)
	jsr	CompRLC				; Compress Chunk
	move.l	a2,a1				; A1 = Ending
	movem.l	(sp)+,a0/a2
	move.l	a1,d0				; D0 = Size of Compressed
	sub.l	a2,d0
	cmp.l	d2,d0
	bge.s	CCKS6300			; >= UnCompressed!
;
;	>>>   COMPRESSED CHUNK IS SMALLER!   <<<
;
	move.l	a1,a2				; Use Compressed Chunk
	add.w	#($8000-2),d0			; Mark Chunk Compressed
	move.w	d0,6(a6)			; Update Size of Chunk (Compressed-2)
	bra.s	CCKS7000
;
;	>>>   NO COMPRESSION FOR THIS CHUNK   <<<
;
CCKS6200
	move.b	(a0)+,(a2)+			; Copy ChunkData
CCKS6300
	dbf	d2,CCKS6200
;
;	>>>   FINISHED THIS CHUNK!   <<<
;
CCKS7000
	move.l	(sp)+,d1			; ORIGINAL SIZE.L
	add.l	d1,d6				; TOTAL SIZE += ORIGINAL
	move.l	4(a6),d0			; COMPRESSED SIZE.L
	and.l	#$7fff,d0
	move.l	d0,-(sp)
	move.l	d1,-(sp)
	sub.l	d0,d1				; D1 = (ORIGINAL-COMPRESSED)
	move.l	a6,d0
	sub.l	ChunkTable,d0
	lsr.l	#4,d0
	move.l	d0,-(sp)
	lea	CompressMsgU,a0
	tst.w	6(a6)
	bpl.s	CCKS7200
	lea	CompressMsg,a0
	add.l	d1,d5				; TOTAL SAVINGS += (ORIGINAL-COMPRESSED)
CCKS7200
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(3*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	add.w	#16,a6				; Next Chunk
	bra	CCKS2000
;
;	>>>   FINISHED COMPRESSING CHUNKS!   <<<
;
CCKS8000
	move.l	d5,-(sp)			; TOTAL SAVINGS
	move.l	d6,-(sp)
	lea	CompressMsg2,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(2*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	a2,CChunksE
	moveq.l	#0,d7
CCKS9000
	rts


;
;	* * * * * * *       DUMP CHUNKS       * * * * * * *
;
DumpChunks
	move.l	ChunkTableE,d0			; Get #CHUNKS
	sub.l	ChunkTable,d0
	lsr.l	#4,d0
	move.l	d0,-(sp)			; #CHUNKS
	lea	ChunkMsg,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(1*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	move.l	ChunkTable,a2
	moveq.l	#0,d6				; D6 = Total Size
	moveq.l	#0,d5				; D5 = Chunk Count
DCKS200
	move.l	#MSGUserBreak,d7
	move.l	Task,a0				; Quit?
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	DCKS900				; Yes!
;
	cmp.l	ChunkTableE,a2			; At End?
	beq	DCKS800
	move.l	8(a2),-(sp)
	move.l	4(a2),d0
	move.l	d0,-(sp)
	and.l	#$7fff,d0
	add.l	d0,d6
	move.l	0(a2),-(sp)
	move.l	d5,-(sp)
	lea	ChunkMsg2,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(4*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	addq.l	#1,d5
	add.w	#16,a2
	bra.s	DCKS200
DCKS800
	move.l	d6,-(sp)			; TOTAL SIZE
	lea	ChunkMsg3,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(1*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	moveq.l	#0,d7
DCKS900
	rts


;
;	* * * * * * *       DUMP MODULES       * * * * * * *
;
DumpModules
	moveq.l	#0,d6				; D6 = Module Count
	move.l	ChunkLists,a2			; A2 = ChunkList Pointer
DMOD200
	move.l	#MSGUserBreak,d7
	move.l	Task,a0				; Quit?
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	DMOD900				; Yes!
;
	move.l	d6,d0
	lsl.l	#1,d0
	lea	ChunkIndex,a0
	add.l	d0,a0
	moveq.l	#0,d1
	move.b	1(a0),d1
	lsl.w	#8,d1
	move.b	0(a0),d1
	move.l	a2,d0
	sub.l	ChunkLists,d0
	cmp.l	d0,d1
	beq.s	DMOD220
	move.l	#$52414e44,d1
DMOD220
	move.l	d1,-(sp)
	move.l	d6,-(sp)			; MODULE#
	lea	ModuleMsg,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(2*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
DMOD400
	moveq.l	#0,d0				; APU Address
	move.b	1(a2),d0
	lsl.w	#8,d0
	move.b	0(a2),d0
	addq.w	#2,a2
	tst.w	d0
	beq	DMOD700				; Next Module
;
	move.l	d0,-(sp)			; APU ADDRESS
	lea	ModuleMsg2,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(1*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
DMOD500
	moveq.l	#0,d1				; Chunk#
	move.b	1(a2),d1
	lsl.w	#8,d1
	move.b	0(a2),d1
	addq.w	#2,a2
	tst.w	d1
	bmi	DMOD400				; Next ChunkGroup
	move.l	d1,d0
	lsl.l	#4,d0
	move.l	ChunkTable,a0
	add.l	d0,a0
	move.l	4(a0),-(sp)
	move.l	0(a0),-(sp)
	move.l	d1,-(sp)
	lea	ModuleMsg3,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(3*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
	bra.s	DMOD500
DMOD700
	addq.w	#1,d6				; Next Module
	cmp.l	NumModules,d6
	ble	DMOD200
	moveq.l	#0,d7
DMOD900
	rts


;
;	* * * * * * *       BUILD TURBOTABLE       * * * * * * *
;
BuildTurboTable
	move.l	BankData,a5			; Get #EFFECTS
	add.l	#MusicTableOffset,a5
	moveq.l	#0,d0
	move.b	2(a5),d0
	move.l	d0,NumEffects
	move.l	BankData,a5			; A5 = EFFECTS MODULE TABLE
	add.l	#(MusicTableOffset+3),a5
	move.l	NumModules,d0			; Skip Module Address Table
	addq.l	#1,d0
	mulu.l	#3,d0
	add.l	d0,a5
	move.l	BankData,a0			; Skip Song Module Table
	add.l	#MusicTableOffset,a0
	moveq.l	#0,d0
	move.b	1(a0),d0
	add.l	d0,a5
	move.b	2(a0),d0			; Skip Effect Module Table
	addq.l	#4,d0				; Skip BlastAddress.W/BlastSize.W
	lea	(a5,d0.l),a4			; A4 = EFFECTS BLAST TABLE
;
	move.l	d0,-(sp)			; #EFFECTS
	lea	TurboTableMsg,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(1*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	#($1f00-$20+14+1),d5		; A3 = BFX DIRECTORY BASE
	moveq.l	#0,d2				; D2 = Module#
	bsr	ReadAPUByte
	move.l	d0,d4
	lsl.w	#8,d4
	subq.l	#1,d5
	bsr	ReadAPUByte
	move.b	d0,d4
	move.l	d4,a3
	addq.w	#1,a3
;
	move.l	#($1f00-$20+2+1),d5		; A2 = BSN DIRECTORY BASE
	bsr	ReadAPUByte
	move.l	d0,d4
	lsl.w	#8,d4
	subq.l	#1,d5
	bsr	ReadAPUByte
	move.b	d0,d4
	move.l	d4,a2
	addq.w	#1,a2
;
	move.l	TurboTable,a6			; A6 = TurboTable
	moveq.l	#0,d6				; D6 = Effect Count
BTTE200
	move.l	#MSGUserBreak,d7
	move.l	Task,a0				; Quit?
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	BTTE900				; Yes!
;
	sub.w	#(5*4),sp
	move.l	d6,(sp)				; EFFECT#
	moveq.l	#0,d2				; EFFECT MODULE#
	move.b	(a5),d2
	move.l	d2,8(sp)
	lea	TurboTableMsg2,a0
	tst.w	(a4)				; Regular/Turbo Effect?
	beq	BTTE600				; Regular
;
	move.l	d2,d0				; Get Module ChunkList Index
	lsl.l	#1,d0
	lea	ChunkIndex,a0
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	1(a0),d0
	lsl.w	#8,d0
	move.b	0(a0),d0
	move.l	ChunkLists,a0			; A0 = ChunkLists
	add.l	d0,a0
	addq.w	#2,a0				; Skip APU Address
	moveq.l	#0,d0				; Get Chunk#
	move.b	1(a0),d0
	lsl.w	#8,d0
	move.b	0(a0),d0
	move.l	ChunkTable,a0			; A0 = ChunkTable
	lsl.l	#4,d0
	add.l	d0,a0
	move.l	0(a0),d0			; LOAD.L
	add.l	#$470000,d0
	move.l	d0,12(sp)
	move.b	d0,1(a6)
	lsr.l	#8,d0
	move.b	d0,2(a6)
	lsr.l	#8,d0
	move.b	d0,3(a6)
	move.l	4(a0),d0			; SIZE.L
	move.l	d0,16(sp)
	move.b	d0,4(a6)
	lsr.w	#8,d0
	move.b	d0,5(a6)
;
	move.l	a3,d5				; BFX DIRECTORY ENTRY
	bsr	ReadAPUByte
	move.l	d0,d4
	lsl.w	#8,d4
	subq.l	#1,d5
	bsr	ReadAPUByte
	move.b	d0,d4
;
	move.l	d4,d5				; BFX STRUCTURE.COUNT
	addq.l	#1,d5				; BFX STRUCTURE.BSOUND#
	bsr	ReadAPUByte
;
	lsl.l	#1,d0				; BSN DIRECTORY ENTRY
	move.l	d0,d5
	add.l	a2,d5
	bsr	ReadAPUByte
	move.l	d0,d4
	lsl.w	#8,d4
	subq.l	#1,d5
	bsr	ReadAPUByte
	move.b	d0,d4
;
	move.l	d4,d5				; BSN STRUCTURE.BWAVE#
	addq.l	#2,d5
	bsr	ReadAPUByte
;
	move.l	d0,4(sp)			; BWAVE#
	move.b	d0,0(a6)
	lea	TurboTableMsg2T,a0
BTTE600
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(5*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
BTTE700
	addq.w	#6,a6				; Next Effect
	addq.w	#1,a5				; EffectModuleTable++
	addq.w	#2,a4				; RingTable++
	addq.w	#2,a3				; BFXBase++
	addq.w	#1,d6
	cmp.l	NumEffects,d6
	bne	BTTE200
	moveq.l	#0,d7
BTTE900
	rts


;
;	* * * * * * *       READ A BYTE FROM APU MEMORY       * * * * * * *
;
;	D5 = APUAddress
;	D2 = Module#
;
ReadAPUByte
RAB200
	move.l	d2,d0				; A1 = BLOCKLIST POINTER
	mulu.l	#(MaxModuleBlocks*16),d0
	move.l	BlockLists,a1
	add.l	d0,a1
	move.l	d2,d0				; D1 = #BLOCKS
	lsl.w	#2,d0
	lea	ModuleNumBlocks,a0
	move.l	(a0,d0.w),d1
	bra.s	RAB800
RAB300
	tst.l	8(a1)				; *EMPTY BLOCK*?
	beq.s	RAB700
	move.l	4(a1),d0			; D0 = APU ADDRESS.W
	cmp.l	d0,d5				; Starts AFTER Byte?
	blt.s	RAB700
	add.l	8(a1),d0			; Ends BEFORE Byte?
	cmp.l	d0,d5
	bge.s	RAB700
;
	move.l	d5,d0				; Get Offset within Block
	sub.l	4(a1),d0
	add.l	0(a1),d0			; LOAD.L
	sub.l	#MusicLoadOffset,d0		; Get Offset within Emulation ROM
	move.l	BankData,a0
	add.l	d0,a0
	moveq.l	#0,d0
	move.b	(a0),d0
RAB500
	rts
RAB700
	add.w	#16,a1				; Next Block
RAB800
	dbf	d1,RAB300
	tst.l	d2				; Already Tried COMMON Module?
	beq.s	RAB500				; Yes!  Error!
	moveq.l	#0,d2				; Scan Again Through COMMON!
	bra	ReadAPUByte


;
;	* * * * * * *       SAVE BOOT       * * * * * * *
;
SaveBoot
	move.l	OutBaseName,a0			; Create BootData Name
	lea	OutName,a1
	move.l	a1,d1
SBOT200
	move.b	(a0)+,(a1)+
	bne.s	SBOT200
	subq.w	#1,a1
	move.b	#'B',(a1)+
	clr.b	(a1)
;
	move.l	DosBase,a6			; Save the DriverData
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGOUTError,d7
	move.l	d0,d4
	beq	SBOT800
	move.l	d4,d1
;
	move.l	BankData,a0
	move.l	a0,d2
	addq.l	#4,d2
	moveq.l	#0,d3
	move.b	1(a0),d3
	lsl.w	#8,d3
	move.b	0(a0),d3
;
	jsr	_LVOWrite(a6)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	moveq.l	#0,d7
SBOT800
	rts


;
;	* * * * * * *       SAVE DRIVER       * * * * * * *
;
SaveDriver
	move.l	OutBaseName,a0			; Create DriverData Name
	lea	OutName,a1
	move.l	a1,d1
SDRV200
	move.b	(a0)+,(a1)+
	bne.s	SDRV200
	subq.w	#1,a1
	move.b	#'D',(a1)+
	clr.b	(a1)
;
	move.l	DosBase,a6			; Save the DriverData
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGOUTError,d7
	move.l	d0,d4
	beq	SDRV800
	move.l	d4,d1
;
	move.l	BankData,a0
	moveq.l	#0,d0
	move.b	1(a0),d0
	lsl.w	#8,d0
	move.b	0(a0),d0
	addq.l	#4+4,d0				; LEN.W/DEST.W,DATA,$0000.W,$0700.W
	add.l	d0,a0
;
	move.l	a0,d2
	addq.l	#4,d2
	moveq.l	#0,d3
	move.b	1(a0),d3
	lsl.w	#8,d3
	move.b	0(a0),d3
;
	jsr	_LVOWrite(a6)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	moveq.l	#0,d7
SDRV800
	rts


;
;	* * * * * * *       SAVE CHUNKS       * * * * * * *
;
SaveChunks
	move.l	OutBaseName,a0			; Create ChunkData Name
	lea	OutName,a1
	move.l	a1,d1
SCKS200
	move.b	(a0)+,(a1)+
	bne.s	SCKS200
	subq.w	#1,a1
	move.b	#'C',(a1)+
	clr.b	(a1)
;
	move.l	DosBase,a6			; Save the ChunkData
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGOUTError,d7
	move.l	d0,d4
	beq	SCKS800
	move.l	d4,d1
	move.l	CChunks,d2
	move.l	CChunksE,d3
	sub.l	d2,d3
	jsr	_LVOWrite(a6)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	moveq.l	#0,d7
SCKS800
	rts


;
;	* * * * * * *       SAVE CHUNKTABLE       * * * * * * *
;
SaveChunkTable
	move.l	OutBaseName,a0			; Create ChunkTable Name
	lea	OutName,a1
	move.l	a1,d1
SCTE200
	move.b	(a0)+,(a1)+
	bne.s	SCTE200
	subq.w	#1,a1
	move.b	#'T',(a1)+
	clr.b	(a1)
;
	move.l	MUSData,a2			; A0 = MUS Data Pointer
	move.l	ChunkTable,a0
	move.l	ChunkTableE,a1
SCTE400
	cmp.l	a0,a1
	beq.s	SCTE500
	move.l	0(a0),d0			; LOAD.L
	add.l	#$470000,d0
	move.b	d0,(a2)+
	lsr.l	#8,d0
	move.b	d0,(a2)+
	lsr.l	#8,d0
	move.b	d0,(a2)+
	move.b	7(a0),(a2)+			; SIZE.W
	move.b	6(a0),(a2)+
	add.w	#16,a0
	bra.s	SCTE400
;
SCTE500
	move.l	DosBase,a6			; Save the ChunkTable
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGOUTError,d7
	move.l	d0,d4
	beq	SCTE800
	move.l	d4,d1
	move.l	MUSData,d2
	move.l	a2,d3
	sub.l	d2,d3
	jsr	_LVOWrite(a6)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	moveq.l	#0,d7
SCTE800
	rts


;
;	* * * * * * *       SAVE CHUNKLISTS       * * * * * * *
;
SaveChunkLists
	move.l	OutBaseName,a0			; Create ChunkLists Name
	lea	OutName,a1
	move.l	a1,d1
SCLS200
	move.b	(a0)+,(a1)+
	bne.s	SCLS200
	subq.w	#1,a1
	move.b	#'L',(a1)+
	clr.b	(a1)
;
	move.l	DosBase,a6			; Save the ChunkLists
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGOUTError,d7
	move.l	d0,d4
	beq	SCLS800
	move.l	d4,d1
	move.l	ChunkLists,d2
	move.l	ChunkListsE,d3
	sub.l	d2,d3
	jsr	_LVOWrite(a6)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	moveq.l	#0,d7
SCLS800
	rts


;
;	* * * * * * *       SAVE CHUNKINDEX       * * * * * * *
;
SaveChunkIndex
	move.l	OutBaseName,a0			; Create ChunkIndex Name
	lea	OutName,a1
	move.l	a1,d1
SCIX200
	move.b	(a0)+,(a1)+
	bne.s	SCIX200
	subq.w	#1,a1
	move.b	#'I',(a1)+
	clr.b	(a1)
;
	move.l	DosBase,a6			; Save the ChunkIndex
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGOUTError,d7
	move.l	d0,d4
	beq	SCIX800
	move.l	d4,d1
	lea	ChunkIndex,a0
	move.l	a0,d2
	move.l	NumModules,d3
	addq.l	#1,d3
	lsl.w	#1,d3
	jsr	_LVOWrite(a6)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	moveq.l	#0,d7
SCIX800
	rts


;
;	* * * * * * *       SAVE TURBOTABLE       * * * * * * *
;
SaveTurboTable
	move.l	OutBaseName,a0			; Create TurboTable Name
	lea	OutName,a1
	move.l	a1,d1
STTE200
	move.b	(a0)+,(a1)+
	bne.s	STTE200
	subq.w	#1,a1
	move.b	#'O',(a1)+
	clr.b	(a1)
;
	move.l	DosBase,a6			; Save the TurboTable
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGOUTError,d7
	move.l	d0,d4
	beq	STTE800
	move.l	d4,d1
	move.l	TurboTable,d2
	move.l	NumEffects,d3
	mulu.l	#6,d3
	jsr	_LVOWrite(a6)
	move.l	d4,d1
	jsr	_LVOClose(a6)
	moveq.l	#0,d7
STTE800
	rts


;
;	* * * * * * *       READ MUS FILE       * * * * * * *
;
ReadMUS
	move.l	#MSGMUSTable,d7			; Print Table
	jsr	PrintMsg
	move.l	MUSData,a5			; A5 = MUS Data Pointer
;
;	>>>   NEXT CHUNK   <<<
;
RMS200
	cmp.l	MUSDataEnd,a5			; At end?
	beq	RMS900				; Yes
	move.l	#MSGBadMUS,d7
	move.l	(a5)+,d0			; Check NOB Header
	lsr.l	#8,d0
	cmp.l	#'SYM',d0
	beq	RMS900
	cmp.l	#'NOB',d0
	bne	RMS950
	move.w	1(a5),d4			; D4 = Origin
	move.b	(a5),d4
	addq.w	#2,a5
	moveq.l	#0,d6
	move.w	1(a5),d6			; D6 = Length
	move.b	(a5),d6
	addq.w	#2,a5
	swap	d4
	move.b	(a5),d4				; Bank
	swap	d4
	and.l	#$007fffff,d4
	addq.w	#4,a5
	move.l	d4,d5
	and.l	#$7fff,d5
	move.l	d4,d0
	and.l	#$00ff0000,d0
	lsr.l	#1,d0
	add.l	d0,d5
;
	move.l	d6,-(sp)			; Send Message
	move.l	d5,-(sp)
	move.l	d4,-(sp)
	lea	MUSChunkMsg,a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	lea	(3*4)(sp),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
;	>>>   COPY CHUNK TO EMULATION ROM   <<<
;
	move.l	d5,d0				; Get Offset within Emulation ROM
	sub.l	#MusicLoadOffset,d0
	move.l	BankData,a0
	add.l	d0,a0
	subq.l	#1,d6
RMS400
	move.b	(a5)+,(a0)+			; Copy NOB Chunk to Emulation ROM
	dbf	d6,RMS400
	bra	RMS200				; Process Next NOB Chunk
RMS900
	move.l	#MSGNewLine,d7
	jsr	PrintMsg
	moveq.l	#0,d7
RMS950
	tst.l	d7
	rts


;
;	* * * * * * *       TEXT MESSAGES       * * * * * * *
;
MUSChunkMsg	dc.b	'$%06lx $%08lx $%08lx',10,0

BlockListMsg	dc.b	10,'Module %ld',10
		dc.b	'  [%ld Blocks of %ld]',10,0
BlockListMsg2	dc.b	'  LOAD=$%06lx  APU=$%04lx  Size=$%04lx',10,0

BlockMsg	dc.b	10,10,'%ld BLOCKS',10,0
BlockMsg2	dc.b	'  $%06lx $%04lx',10,0
BlockMsg2U	dc.b	'* $%06lx $%04lx',10,0
BlockMsg3	dc.b	'$%06lx TOTAL SIZE, $%06lx USED, $%06lx UNUSED',10,10,0

CompressMsg	dc.b	'Chunk *%03ld:  $%04lx -> $%04lx',10,0
CompressMsgU	dc.b	'Chunk  %03ld:  $%04lx',10,0
CompressMsg2	dc.b	'TOTALS     $%06lx  $%06lx',10,0

ChunkMsg	dc.b	10,10,'%ld CHUNKS',10,0
ChunkMsg2	dc.b	'%03ld:  $%06lx $%04lx  ($%06lx)',10,0
ChunkMsg3	dc.b	'$%06lx TOTAL SIZE',10,0

ModuleMsg	dc.b	10,'Module %ld  (_MCI=$%04lx)',10,0
ModuleMsg2	dc.b	'APU $%04lx',10,0
ModuleMsg3	dc.b	'    %03ld: $%06lx $%04lx',10,0

TurboTableMsg	dc.b	10,10,'%ld EFFECTS',10,0
TurboTableMsg2	dc.b	'Effect %ld',10,0
TurboTableMsg2T	dc.b	'Effect %ld: BWave %03ld, Module %02ld, Load $%06lx, Size $%04lx',10,0


	end
