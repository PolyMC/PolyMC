;***************************************************************************
;*                                                                         *
;*                             C O N V   G U S                             *
;*                                                                         *
;*                              CONVERT MODULE                             *
;*                                                                         *
;***************************************************************************

	include	convgus.i

	xref	_LVOOpen
	xref	_LVOClose
	xref	_LVORead
	xref	_LVOWrite
	xref	_LVOSeek

	xref	VDTDebugOutC
	xref	PrintMsg

	xref	DosBase

	xref	MSGBadPatch
	xref	MSGBadOutput

	xref	PatchName,PatchData
	xref	OutputName,OutputFileName

	xref	MsgBuffer
	xref	WaveCount


	section	CONVERT,CODE

	xdef	DoConvGUS

;
;	* * * * * * *       CONVERT GUS PATCH FILES       * * * * * * *
;
DoConvGUS
;
;	>>>   READ PATCH   <<<
;
	move.l	DosBase,a6				; Open PATCH
	move.l	PatchName,d1
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGBadPatch,d7
	move.l	d0,d4
	beq	DCGS9000
	move.l	d4,d1					; Read PATCH
	move.l	PatchData,d2
	move.l	#(256*1024),d3
	jsr	_LVORead(a6)
	move.l	d4,d1					; Close PATCH
	jsr	_LVOClose(a6)
;
;	>>>   ANALYZE PATCH HEADER   <<<
;
	move.l	PatchData,a0
	moveq.l	#0,d0
	move.b	88(a0),d0				; MasterVolume
	lsl.w	#8,d0
	move.b	87(a0),d0
	move.l	d0,-(sp)
	move.b	86(a0),d0				; Number of Waveforms
	lsl.w	#8,d0
	move.b	85(a0),d0
	move.l	d0,-(sp)
	moveq.l	#0,d0
	move.b	84(a0),d0				; Number of Channels
	move.l	d0,-(sp)
	move.b	83(a0),d0				; Number of Voices
	move.l	d0,-(sp)
	move.b	82(a0),d0				; Number of Instruments
	move.l	d0,-(sp)
	pea	12(a0)					; ID
	pea	0(a0)					; Version
	lea	PatchHdrMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(7*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
;	>>>   ANALYZE INSTRUMENT HEADER   <<<
;
	move.l	PatchData,a2
	add.l	#129,a2
	move.b	21(a2),d0				; SkipSize
	lsl.w	#8,d0
	move.b	20(a2),d0
	lsl.l	#8,d0
	move.b	19(a2),d0
	lsl.l	#8,d0
	move.b	18(a2),d0
	move.l	d0,-(sp)
	moveq.l	#0,d0
	move.b	22(a2),d0				; #Layers
	move.l	d0,-(sp)
	pea	2(a2)					; Name
	moveq.l	#0,d0
	move.w	(a2),d0					; ID
	move.l	d0,-(sp)
	lea	PatchInstMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(4*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
;	>>>   ANALYZE LAYER HEADER   <<<
;
	move.l	PatchData,a2
	add.l	#129+63,a2
	moveq.l	#0,d6					; #WaveSamples
	move.b	6(a2),d6
	move.l	d6,-(sp)
	move.b	5(a2),d0				; SkipSize
	lsl.w	#8,d0
	move.b	4(a2),d0
	lsl.l	#8,d0
	move.b	3(a2),d0
	lsl.l	#8,d0
	move.b	2(a2),d0
	move.l	d0,-(sp)
	moveq.l	#0,d0
	move.b	(a2),d0					; Previous
	move.l	d0,-(sp)
	moveq.l	#0,d0
	move.b	1(a2),d0				; ID
	move.l	d0,-(sp)
	lea	PatchLayerMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(4*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
	move.l	PatchData,a2
	add.l	#129+63+47,a2
	move.l	d6,d5					; D5 = #WaveSamples
	subq.w	#1,d6
	clr.l	WaveCount				; WaveCounter
;
;	>>>   ANALYZE WAVE HEADER   <<<
;
DCGS2000
	move.b	59(a2),d1				; ScaleFactor
	lsl.w	#8,d1
	move.b	58(a2),d1
	move.l	d1,-(sp)
;
	move.b	57(a2),d1				; ScaleFreq
	lsl.w	#8,d1
	move.b	56(a2),d1
	move.l	d1,-(sp)
;
	move.b	7(a2),d0				; LoopEnd Fraction
	lsr.b	#4,d0
	and.l	#$f,d0
	move.l	d0,-(sp)
	move.b	19(a2),d1				; LoopEnd
	lsl.w	#8,d1
	move.b	18(a2),d1
	lsl.l	#8,d1
	move.b	17(a2),d1
	lsl.l	#8,d1
	move.b	16(a2),d1
	move.l	d1,-(sp)
;
	move.b	7(a2),d0				; LoopStart Fraction
	and.l	#$f,d0
	move.l	d0,-(sp)
	move.b	15(a2),d1				; LoopStart
	lsl.w	#8,d1
	move.b	14(a2),d1
	lsl.l	#8,d1
	move.b	13(a2),d1
	lsl.l	#8,d1
	move.b	12(a2),d1
	move.l	d1,-(sp)
;
	moveq.l	#0,d0
	move.b	36(a2),d0				; Balance
	move.l	d0,-(sp)
;
	moveq.l	#0,d0
	move.b	35(a2),d0				; Tune
	lsl.w	#8,d0
	move.b	34(a2),d0
	move.l	d0,-(sp)
;
	move.b	33(a2),d3				; RootFreq
	lsl.w	#8,d3
	move.b	32(a2),d3
	lsl.l	#8,d3
	move.b	31(a2),d3
	lsl.l	#8,d3
	move.b	30(a2),d3
	jsr	FindFreqName
	pea	0(a1)
	move.l	d3,-(sp)
;
	move.b	29(a2),d3				; HighFreq
	lsl.w	#8,d3
	move.b	28(a2),d3
	lsl.l	#8,d3
	move.b	27(a2),d3
	lsl.l	#8,d3
	move.b	26(a2),d3
	jsr	FindFreqName
	pea	0(a1)
	move.l	d3,-(sp)
;
	move.b	25(a2),d3				; LowFreq
	lsl.w	#8,d3
	move.b	24(a2),d3
	lsl.l	#8,d3
	move.b	23(a2),d3
	lsl.l	#8,d3
	move.b	22(a2),d3
	jsr	FindFreqName
	pea	0(a1)
	move.l	d3,-(sp)
;
	move.b	11(a2),d4				; #Samples
	lsl.w	#8,d4
	move.b	10(a2),d4
	lsl.l	#8,d4
	move.b	9(a2),d4
	lsl.l	#8,d4
	move.b	8(a2),d4
	move.l	d4,-(sp)
;
	lea	PatchWaveSignedMsg(pc),a0		; Signed
	move.b	55(a2),d0
	and.b	#%00000010,d0
	beq.s	DCGS2100
	lea	PatchWaveUnSignedMsg(pc),a0		; UnSigned
DCGS2100
	pea	0(a0)
;
	moveq.l	#8,d1					; 8bit
	move.b	55(a2),d0
	and.b	#%00000001,d0
	beq.s	DCGS2200
	moveq.l	#16,d1					; 16bit
DCGS2200
	move.l	d1,-(sp)
;
	moveq.l	#0,d0
	move.b	21(a2),d0				; SampleRate
	lsl.w	#8,d0
	move.b	20(a2),d0
	move.l	d0,-(sp)
;
	pea	0(a2)					; Name
;
	lea	PatchWaveMsg(pc),a0
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(19*4),sp
	moveq.l	#-1,d7
	jsr	PrintMsg
;
;	>>>   CONVERT TO WAVETABLE DATA TO SIGNED FORMAT   <<<
;
	move.b	55(a2),d0				; Already Signed?
	and.b	#%00000010,d0
	beq.s	DCGS7200				; Yes
	lea	96(a2),a0				; A0 points to WaveData
	move.b	55(a2),d0				; 8-bit or 16-bit?
	and.b	#%00000001,d0
	beq.s	DCGS6200				; 8-bit
	addq.w	#1,a0					; Move to UpperByte
DCGS6200
	move.l	d4,d1					; D1 = #Bytes
DCGS6300
	move.b	(a0),d0
	eor.b	#$80,d0
	move.b	d0,(a0)
	move.b	55(a2),d0				; 8-bit or 16-bit?
	and.b	#%00000001,d0
	beq.s	DCGS6400				; 8-bit
	subq.l	#1,d1
	addq.w	#1,a0
DCGS6400
	addq.w	#1,a0
	subq.l	#1,d1
	bne.s	DCGS6300
;
;	>>>   WRITE OUT THE WAVETABLE DATA   <<<
;
DCGS7000
	add.l	#96,a2					; Skip WaveHeader
	move.l	WaveCount,d0
	lea	PatchWavenName(pc),a0
	cmp.l	#1,d5					; Only 1 WaveTable?
	bne.s	DCGS7200				; No
	lea	PatchWave1Name(pc),a0			; Yes, don't need #Suffix
	move.l	a0,d0
DCGS7200
	move.l	d0,-(sp)
	move.l	OutputName,-(sp)
	move.l	sp,a1
	jsr	VDTDebugOutC
	add.w	#(2*4),sp
	move.l	#MsgBuffer,d1
	move.l	#1006,d2
	jsr	_LVOOpen(a6)
	move.l	#MSGBadOutput,d7
	tst.l	d0
	beq.s	DCGS9000
	move.l	d0,d7
	move.l	d7,d1
	move.l	a2,d2
	move.l	d4,d3
	jsr	_LVOWrite(a6)
	move.l	d7,d1
	jsr	_LVOClose(a6)
;
;	>>>   MOVE TO NEXT WAVEFORM   <<<
;
	add.l	d4,a2					; Skip WaveData
	addq.l	#1,WaveCount				; Next WaveForm
	dbf	d6,DCGS2000				; Next Wave
;
;	>>>   CONVERSION COMPLETE!   <<<
;
	moveq.l	#0,d7					; No Error!
DCGS9000
	rts

;
;	* * * * * * *       FIND FREQUENCY       * * * * * * *
;
;	D3 = Frequency
;	A0 = Name
;
FindFreqName
	lea	FreqTable(pc),a0				; FrequenyTable
	lea	FreqNames(pc),a1				; FrequencyNames
FFN200
	addq.w	#4,a1
	move.l	(a0)+,d0					; Get Next Frequency
	beq.s	FFN800						; Invalid!
	cmp.l	d0,d3
	bgt.s	FFN200
	subq.w	#4,a1
	subq.w	#4,a0
FFN800
	rts


FreqTable
	dc.l	16351,17323,18354,19445,20601,21826		; Octave0
	dc.l	23124,24499,25956,27500,29135,30867
	dc.l	32703,34647,36708,38890,41203,43653		; Octave1
	dc.l	46249,48999,51913,54999,58270,61735
	dc.l	65406,69295,73416,77781,82406,87306		; Octave2
	dc.l	92498,97998,103826,109999,116540,123470
	dc.l	130812,138591,146832,155563,164813,174614	; Octave3
	dc.l	184997,195997,207652,219999,233081,246941
	dc.l	261625,277182,293664,311126,329627,349228	; Octave4
	dc.l	369994,391995,415304,440000,466163,493883
	dc.l	523251,554365,587329,622254,659255,698456	; Octave5
	dc.l	739989,783991,830609,880000,932328,987767
	dc.l	1046503,1108731,1174660,1244509,1318511,1396914	; Octave6
	dc.l	1479979,1567983,1661220,1760002,1864657,1975536
	dc.l	2093007,2217464,2349321,2489019,2637024,2793830	; Octave7
	dc.l	2959960,3135968,3322443,3520006,3729316,3951073
	dc.l	4186073,4434930,4698645,4978041,5274051,5587663	; Octave8
	dc.l	5919922,6271939,6644889,7040015,7458636,7902150
	dc.l	0
FreqNames
	dc.b	'C0',0,0,'C#0',0,'D0',0,0,'D#0',0,'E0',0,0,'F0',0,0
	dc.b	'F#0',0,'G0',0,0,'G#0',0,'A0',0,0,'A#0',0,'B0',0,0
	dc.b	'C1',0,0,'C#1',0,'D1',0,0,'D#1',0,'E1',0,0,'F1',0,0
	dc.b	'F#1',0,'G1',0,0,'G#1',0,'A1',0,0,'A#1',0,'B1',0,0
	dc.b	'C2',0,0,'C#2',0,'D2',0,0,'D#2',0,'E2',0,0,'F2',0,0
	dc.b	'F#2',0,'G2',0,0,'G#2',0,'A2',0,0,'A#2',0,'B2',0,0
	dc.b	'C3',0,0,'C#3',0,'D3',0,0,'D#3',0,'E3',0,0,'F3',0,0
	dc.b	'F#3',0,'G3',0,0,'G#3',0,'A3',0,0,'A#3',0,'B3',0,0
	dc.b	'C4',0,0,'C#4',0,'D4',0,0,'D#4',0,'E4',0,0,'F4',0,0
	dc.b	'F#4',0,'G4',0,0,'G#4',0,'A4',0,0,'A#4',0,'B4',0,0
	dc.b	'C5',0,0,'C#5',0,'D5',0,0,'D#5',0,'E5',0,0,'F5',0,0
	dc.b	'F#5',0,'G5',0,0,'G#5',0,'A5',0,0,'A#5',0,'B5',0,0
	dc.b	'C6',0,0,'C#6',0,'D6',0,0,'D#6',0,'E6',0,0,'F6',0,0
	dc.b	'F#6',0,'G6',0,0,'G#6',0,'A6',0,0,'A#6',0,'B6',0,0
	dc.b	'C7',0,0,'C#7',0,'D7',0,0,'D#7',0,'E7',0,0,'F7',0,0
	dc.b	'F#7',0,'G7',0,0,'G#7',0,'A7',0,0,'A#7',0,'B7',0,0
	dc.b	'C8',0,0,'C#8',0,'D8',0,0,'D#8',0,'E8',0,0,'F8',0,0
	dc.b	'F#8',0,'G8',0,0,'G#8',0,'A8',0,0,'A#8',0,'B8',0,0
	dc.b	'???',0


PatchHdrMsg		dc.b	'%s, %s',10
			dc.b	'%ld Instruments, %ld Voices',10
			dc.b	'%ld Channels, %ld Waveforms',10
			dc.b	'%ld MasterVolume',10
			dc.b	10,0

PatchInstMsg		dc.b	'INSTRUMENT ID %ld, "%s"',10
			dc.b	'%ld Layers, %ld SkipSize',10
			dc.b	10,0

PatchLayerMsg		dc.b	'LAYER ID %ld',10
			dc.b	'%ld Previous, %ld SkipSize',10
			dc.b	'%ld WaveSamples',10
			dc.b	10,0

PatchWaveMsg		dc.b	'WAVE <%s>',10
			dc.b	'%ld SampleRate, %ld-bit %s',10
			dc.b	'%ld Bytes',10
			dc.b	'%ld (%s) LowFreq, %ld (%s) HighFreq, %ld (%s) RootFreq',10
			dc.b	'%ld Tune, %ld Balance',10
			dc.b	'LoopByteStart %ld.%lx, LoopByteEnd %ld.%lx',10
			dc.b	'ScaleFreq %ld, ScaleFactor %ld',10
			dc.b	10,0

PatchWaveSignedMsg	dc.b	'Signed',0
PatchWaveUnSignedMsg	dc.b	'UnSigned',0

PatchWave1Name		dc.b	'%s.16',0
;PatchWavenName		dc.b	'%s%ld.16',0
PatchWavenName		dc.b	'%s.%ld',0

			dc.w	0


	end
