;***************************************************************************
;*                                                                         *
;*                           S P L I T   M U S                             *
;*                                                                         *
;*                           COMPRESSION MODULE                            *
;*                                                                         *
;***************************************************************************

	include	spmus.i


	section	RLC,CODE

	xdef	CompRLC

;
;	* * * * * * *       REDUCED LENGTH COMPRESSION (RLC)       * * * * * * *
;
;	A0 = Source
;	D2 = Size
;	A2 = Destination
;
CompRLC
	lea.l	(a0,d2.l),a1			; A1 = End
	move.l	d2,d0
	move.b	d0,(a2)+			; DC.W SIZEOFDECOMPRESSED
	lsr.w	#8,d0
	move.b	d0,(a2)+
;
	movem.l	d2-d7/a3-a6,-(a7)
	moveq.l	#0,d1
	moveq.l	#1,d2
Loop
	move.w	a2,$dff180
	bsr.s   Compress
	tst.b   d0
	beq.s   Loop100
	addq.w  #1,d1				; number of unmatched single bytes
	cmpi.w  #$108,d1			; maximum # of bytes in a single run
	bne.s   Loop100
	bsr     Rotate				; output #bytes for all unmatched data
Loop100
	cmpa.l  a0,a1
	bgt.s   Loop
	bsr     Rotate				; flush last of unmatched data
	bsr     Rotate900			; flush last word
	movem.l	(a7)+,d2-d7/a3-a6
	rts

Compress
	movea.l a0,a3				; a0=start of data
	adda.l  #$1000,a3			; largest pattern distance=$1000
	cmpa.l  a1,a3				; beyond end of data?
	ble.s   Compress100
	movea.l a1,a3				; yes, set to end
Compress100
	moveq   #1,d5				; biggest pattern
	movea.l a0,a5
	addq.w  #1,a5				; start searching at current byte + 1

Compress120
	move.b  (a0),d3				; get current byte
	move.b  1(a0),d4			; first byte to check against
Compress150
	cmp.b   (a5)+,d3			; find match of first two bytes
	bne.s   Compress200
	cmp.b   (a5),d4
	beq.s   Compress300			; found
Compress200
	cmpa.l  a5,a3				; not found
	bgt.s   Compress150			; keep looking
	bra.s   Compress750			; no match, output the byte
Compress300
	subq.w  #1,a5				; point a5 to start of match
	movea.l a0,a4				; point a4 to start of original data
Compress350
	move.b  (a4)+,d3			; do pattern check
	cmp.b   (a5)+,d3
	bne.s   Compress400
	cmpa.l  a5,a3
	bgt.s   Compress350
Compress400
	move.l  a4,d3
	sub.l   a0,d3
	subq.l  #1,d3						; length of matched data
	cmp.l   d3,d5						; best previous match
	bge.s   Compress700					; not the best match so far
	move.l  a5,d4
	sub.l   a0,d4
	sub.l   d3,d4
	subq.w  #1,d4						; d4=distance to pattern
	cmpi.l  #4,d3						; length of matched data
	ble.s   Compress500					; 4 or less bytes, choose best case
	moveq   #6,d6						; last case (#3)
	cmpi.l  #$101,d3					; can't do more than 256 bytes
	blt.s   Compress580
	move.w  #$100,d3
Compress580
	bra.s   Compress600
Compress500
	move.w  d3,d6
	subq.w  #2,d6						; case # for 2,3,4 bytes (#0-#2)
	lsl.w   #1,d6						; word index
Compress600
	lea     LookupTable,a6			; $100,$200,$400,$1000
	cmp.w   (a6,d6.w),d4				; distance to pattern
	bge.s   Compress700					; beyond maximum distance for case
	move.l  d3,d5						; save best case
	move.l	a6,-(sp)
	lea		Distance,a6
	move.l  d4,(a6)
	lea		CaseNumber,a6
	move.b  d6,(a6)
	move.l	(sp)+,a6
Compress700
	cmpa.l  a5,a3						; end of data?
	bgt.s   Compress120					; no, keep looking for pattern


Compress750
	cmpi.l  #1,d5						; length of best match
	beq.s   Compress900					; no match, write byte
	bsr.s   Rotate						; send # of previous unmatched bytes
	move.b  CaseNumber,d6			; index to 8,9,10,8 bits to output
	move.l  Distance,d3				; data to output
	move.w  $8(a6,d6.w),d0				; #bits to output

	ifd		egad
	cmp.b	#8,d0						; Case0 or Case3?
	bne		Compress770					; no
	move.w	d2,-(a7)
	and.w	#$fe00,d2					; will byte split across two words?
	bne		Compress760					; yes, don't flip it
	lea		RLCFlippedBitsTable,a6
	move.b	(a6,d3.w),d3
	lea		LookupTable,a6
Compress760
	move.w	(a7)+,d2
	endc

Compress770
	bsr     Rotate700					; output the data
	move.w  $10(a6,d6.w),d0				; Any RunLength to encode?
	beq.s   Compress800					; No, don't output any more
	move.l  d5,d3						; number of bytes to copy
	subq.w  #(1+4),d3					; output run length-5 as 8 bits

	ifd		egad
	move.w	d2,-(a7)
	and.w	#$fe00,d2					; will byte split across two longwords?
	bne		Compress780					; yes, don't flip it
	lea		RLCFlippedBitsTable,a6
	move.b	(a6,d3.w),d3
	lea		LookupTable,a6
Compress780
	move.w	(a7)+,d2
	endc

	bsr     Rotate700
Compress800
	move.w  $18(a6,d6.w),d0				; index to 2,3,3,3
	move.w  $20(a6,d6.w),d3				; %01,%100,%101,%110
	bsr.s   Rotate700
	addi.w  #1,$28(a6,d6.w)
	adda.l  d5,a0
	clr.b   d0
	rts
Compress900
	moveq.l	#0,d0
	move.b  (a0)+,d0					; output byte

	ifd		egad
	move.w	d2,-(a7)
	and.w	#$fe00,d2					; will byte split across two words?
	bne		Compress950					; yes, don't flip it
	move.b	RLCFlippedBitsTable(pc,d0.w),d0
Compress950
	move.w	(a7)+,d2
	endc

	move.b	d0,d3
	moveq   #8,d0						; 8 bits
	bsr.s   Rotate700					; ****** output backwards
	moveq   #1,d0
	rts

;
;	d1=number of bits to output
;
Rotate
	tst.w	d1
	beq.s   Rotate100				; exit if 0
	move.w  d1,d3
	clr.w   d1
	cmpi.w  #9,d3
	bge.s   Rotate500
	subq.w  #1,d3					; output d3-1
	moveq   #5,d0					; output 5 bits
	bra.s   Rotate700
Rotate100
	rts
Rotate500
	subi.w  #9,d3					; output d3-9 (#bytes to copy-9)

	ifd		egad
	move.w	d2,-(a7)
	and.w	#$fe00,d2				; will byte split across two words?
	bne		Rotate550				; yes, don't flip it
	and.w	#$ff,d3
	move.b	RLCFlippedBitsTable(pc,d3.w),d3
Rotate550
	move.w	(a7)+,d2
	endc

	ori.w   #$700,d3				; %111 case
	moveq   #11,d0					; output 11 bits
;
;	d0=# of bits
;	d3=data to shift out
;
Rotate700
	subq.w  #1,d0
Rotate800
	lsr.l   #1,d3					; shift the bits
	roxl.w  #1,d2
	bcs.s   Rotate950				; done word
	dbf     d0,Rotate800
	rts
Rotate900
	clr.w   d0
Rotate950
	move.b	d2,(a2)+				; Write out next WORD (LOW.B / HIGH.B)
	lsr.w	#8,d2
	move.b	d2,(a2)+
	moveq   #1,d2					; reset output word
	dbf     d0,Rotate800
	rts


RLCFlippedBitsTable
	dc.b	$00,$80,$40,$c0,$20,$a0,$60,$e0,$10,$90,$50,$d0,$30,$b0,$70,$f0
	dc.b	$08,$88,$48,$c8,$28,$a8,$68,$e8,$18,$98,$58,$d8,$38,$b8,$78,$f8
	dc.b	$04,$84,$44,$c4,$24,$a4,$64,$e4,$14,$94,$54,$d4,$34,$b4,$74,$f4
	dc.b	$0c,$8c,$4c,$cc,$2c,$ac,$6c,$ec,$1c,$9c,$5c,$dc,$3c,$bc,$7c,$fc
	dc.b	$02,$82,$42,$c2,$22,$a2,$62,$e2,$12,$92,$52,$d2,$32,$b2,$72,$f2
	dc.b	$0a,$8a,$4a,$ca,$2a,$aa,$6a,$ea,$1a,$9a,$5a,$da,$3a,$ba,$7a,$fa
	dc.b	$06,$86,$46,$c6,$26,$a6,$66,$e6,$16,$96,$56,$d6,$36,$b6,$76,$f6
	dc.b	$0e,$8e,$4e,$ce,$2e,$ae,$6e,$ee,$1e,$9e,$5e,$de,$3e,$be,$7e,$fe
	dc.b	$01,$81,$41,$c1,$21,$a1,$61,$e1,$11,$91,$51,$d1,$31,$b1,$71,$f1
	dc.b	$09,$89,$49,$c9,$29,$a9,$69,$e9,$19,$99,$59,$d9,$39,$b9,$79,$f9
	dc.b	$05,$85,$45,$c5,$25,$a5,$65,$e5,$15,$95,$55,$d5,$35,$b5,$75,$f5
	dc.b	$0d,$8d,$4d,$cd,$2d,$ad,$6d,$ed,$1d,$9d,$5d,$dd,$3d,$bd,$7d,$fd
	dc.b	$03,$83,$43,$c3,$23,$a3,$63,$e3,$13,$93,$53,$d3,$33,$b3,$73,$f3
	dc.b	$0b,$8b,$4b,$cb,$2b,$ab,$6b,$eb,$1b,$9b,$5b,$db,$3b,$bb,$7b,$fb
	dc.b	$07,$87,$47,$c7,$27,$a7,$67,$e7,$17,$97,$57,$d7,$37,$b7,$77,$f7
	dc.b	$0f,$8f,$4f,$cf,$2f,$af,$6f,$ef,$1f,$9f,$5f,$df,$3f,$bf,$7f,$ff

LookupTable
		dc.w	$0100,$0200,$0400,$1000
		dc.w	$0008,$0009,$000a,$000c		; Size of Offset in Bits
		dc.w	$0000,$0000,$0000,$0008		; Size of RunLength
		dc.w	$0002,$0003,$0003,$0003		; Size of Encoded Patterns Below
		dc.w	$0001,$0004,$0005,$0006		; %01 %100 %101 %110
		dc.w	$0000,$0000,$0000,$0000


Distance	dc.l	0
CaseNumber	dc.b	0
			dc.b	0			; Pad byte


	end
