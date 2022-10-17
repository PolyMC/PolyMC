;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                           DATA RIPPER MODULE                            *
;*                                                                         *
;***************************************************************************

	include	ripdoom.i


	xref	DosBase

	xref	_LVOCreateDir
	xref	_LVOUnLock
	xref	_LVOOpen
	xref	_LVOClose
	xref	_LVOSeek
	xref	_LVORead
	xref	_LVOWrite

	xref	Task

	xref	DoomWADFIB
	xref	ConvertWAD
	xref	DoomWADData,DoomWADDir
	xref	DoomEntryName,DoomPrefixName,DoomFileName
	xref	NumDirEntries

	xref	PrintMsg,VDTDebugOutC

	xref	MSGUserBreak
	xref	MSGDoomWADError


	section	RIPDATA,CODE

	xdef	DoRipDoom


;
;	* * * * * * *       RIP DOOM .WAD DATA FILE       * * * * * * *
;
DoRipDoom
	move.l	DosBase,a6				; Open DOOM.WAD file
	move.l	ConvertWAD,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGDoomWADError,d7
	move.l	d0,d4
	beq	DRD950					; Error!
	move.l	d0,DoomWADFIB
	move.l	d4,d1					; Read File Header
	move.l	DoomWADData,d2
	move.l	#12,d3
	jsr	_LVORead(a6)
	move.l	DoomWADData,a0				; Get #Directory Entries
	move.b	7(a0),d0
	lsl.l	#8,d0
	move.b	6(a0),d0
	lsl.l	#8,d0
	move.b	5(a0),d0
	lsl.l	#8,d0
	move.b	4(a0),d0
	move.l	d0,NumDirEntries
	move.l	d4,d1					; Move to start of directory
	move.b	11(a0),d2				; Get Offset to DirectoryStart
	lsl.l	#8,d2
	move.b	10(a0),d2
	lsl.l	#8,d2
	move.b	9(a0),d2
	lsl.l	#8,d2
	move.b	8(a0),d2
	move.l	#-1,d3
	jsr	_LVOSeek(a6)
	move.l	d4,d1					; Read entire directory structure
	move.l	DoomWADDir,d2
	move.l	NumDirEntries,d3
	lsl.l	#4,d3
	jsr	_LVORead(a6)
	jsr	DoRipDoom2				; RIP DOOM
	move.l	DoomWADFIB,d1				; Close DOOM.WAD file
	jmp	_LVOClose(a6)
;

DoRipDoom2
	move.l	DoomWADDir,a5				; A5 = Directory Data
	move.l	NumDirEntries,d6			; D6 = #Directory Entries
	lea	DoomPrefixes,a3				; A3 = PrefixList
	bra.s	DRD800
;
;	>>>   RIP A SINGLE ENTRY   <<<
;
DRD200
	move.l	#MSGUserBreak,d7			; Quit?
	move.l	Task,a0
	move.l	$1a(a0),d0
	and.l	#$00001000,d0
	bne	DRD950					; Yes!
	bsr	RipDoomEntry				; Rip a single entry
	add.w	#16,a5					; Move to next directory entry
DRD800
	dbf	d6,DRD200				; Next directory entry
;
;	>>>   COMPLETED RIPPING ENTRIES   <<<
;
DRD900
	moveq.l	#0,d7
DRD950
	rts


;
;	* * * * * * *       RIP A SINGLE DOOM ENTRY       * * * * * * *
;
;	A5 = DOOM.WAD Directory Entry Pointer
;
RipDoomEntry
	move.b	3(a5),d4				; d4 = Offset to resource
	lsl.l	#8,d4
	move.b	2(a5),d4
	lsl.l	#8,d4
	move.b	1(a5),d4
	lsl.l	#8,d4
	move.b	0(a5),d4
;
	move.b	7(a5),d5				; d5 = Size of resource
	lsl.l	#8,d5
	move.b	6(a5),d5
	lsl.l	#8,d5
	move.b	5(a5),d5
	lsl.l	#8,d5
	move.b	4(a5),d5
;
	lea	DoomEntryName,a1			; Copy Entry Name to FileName
	lea	8(a5),a0
	move.l	(a0)+,d0
	move.l	d0,(a1)+
	move.l	(a0)+,(a1)+
	clr.b	(a1)+
;
	tst.l	d5					; Length = 0?
	bne.s	RDE300					; No
;
;	>>>   SEPERATOR   <<<
;
	lea	BasePrefix,a0				; Copy Base Prefix
	lea	DoomPrefixName,a1
RDE220
	move.b	(a0)+,(a1)+
	bne.s	RDE220
	subq.w	#1,a1
	lea	DoomEntryName,a0			; Add name of seperator
RDE240
	move.b	(a0)+,(a1)+
	bne.s	RDE240
	bra.s	RDE500
;
;	>>>   CHECK FOR PREFIXES   <<<
;
RDE300
	move.l	a3,a0					; List of Prefix Changes
RDE320
	move.l	(a0)+,d1				; End of Prefixes?
	bmi.s	RDE600					; Yes
	cmp.l	d0,d1					; Matching Prefix?
	beq.s	RDE400					; Yes
	addq.w	#4,a0
	bra.s	RDE320					; No
;
;	>>>   FOUND NEW PREFIX   <<<
;
RDE400
	move.l	(a0)+,a1				; Copy new Prefix Over!
	move.l	a0,a3					; New Prefix Start Address
	lea	DoomPrefixName,a0
RDE420
	move.b	(a1)+,(a0)+
	bne.s	RDE420
RDE500
	move.l	#DoomPrefixName,d1			; Create new directory
	jsr	_LVOCreateDir(a6)
	move.l	d0,d1
	jsr	_LVOUnLock(a6)
;
RDE600
	lea	DoomEntryName,a0			; Build Output FileName
	lea	DoomPrefixName,a1
	lea	DoomFileName,a2
RDE620
	move.b	(a1)+,(a2)+				; Copy Prefix
	bne.s	RDE620
	subq.w	#1,a2
	move.b	#'/',(a2)+
RDE640
	move.b	(a0)+,(a2)+				; Copy Entry Name
	bne.s	RDE640
;
	pea	DoomFileName				; CPTR FileName
	move.l	d5,-(sp)				; Size
	move.l	d4,-(sp)				; Offset
	pea	DoomEntryName				; CPTR EntryName
;
	lea	DoomEntryMsg,a0
	move.l	a7,a1
	bsr	VDTDebugOutC
	lea	16(a7),a7
	moveq.l	#-1,d7					; Print calculated string
	jsr	PrintMsg
;
	tst.l	d5					; Any Size?
	beq.s	RDE900					; No, no resource
;
	move.l	DoomWADFIB,d1				; Seek to Resource
	move.l	d4,d2
	move.l	#-1,d3
	jsr	_LVOSeek(a6)
	move.l	DoomWADFIB,d1				; Read Resource Data
	move.l	DoomWADData,d2
	move.l	d5,d3
	jsr	_LVORead(a6)
;
	move.l	#DoomFileName,d1			; Create File
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	move.l	d4,d1					; Write Resource Data
	move.l	DoomWADData,d2
	move.l	d5,d3
	jsr	_LVOWrite(a6)
	move.l	d4,d1					; Close File
	jsr	_LVOClose(a6)
;
RDE900
	rts


;
;	* * * * * * *       TEXT STRINGS/MESSAGES       * * * * * * *
;
DoomPrefixes
	dc.l	'PLAY'
	dc.l	PalettePrefix
	dc.l	'ENDO'
	dc.l	MiscPrefix
	dc.l	'TEXT'
	dc.l	MiscPrefix
	dc.l	'GENM'
	dc.l	MusicPrefix
	dc.l	'HELP'
	dc.l	PicPrefix
	dc.l	-1

BasePrefix
	dc.b	':DOOMDATA/',0
PalettePrefix
	dc.b	':DOOMDATA/PALETTES',0
MiscPrefix
	dc.b	':DOOMDATA/MISC',0
MusicPrefix
	dc.b	':DOOMDATA/MUSIC',0
PicPrefix
	dc.b	':DOOMDATA/PICS',0

DoomEntryMsg
	dc.b	'Entry "%s" $%08lx $%08lx into "%s"',10,0
	dc.w	0

	end
