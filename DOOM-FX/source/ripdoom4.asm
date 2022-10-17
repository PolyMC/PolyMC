;***************************************************************************
;*                                                                         *
;*                         D O O M   R I P P E R                           *
;*                                                                         *
;*                         CONVERT FLOOR MODULE                            *
;*                                                                         *
;***************************************************************************

	include	ripdoom.i


	xref	OpenPictureSNES,WriteIFF,ClosePicture
	xref	SetPicPlanes,ReadDoomPalette,SetPixReg
	xref	ConvertImageReMapBG

	xref	DosBase
	xref	_LVOOpen
	xref	_LVOClose
	xref	_LVORead

	xref	Task
	xref	DoomWADData
	xref	OutputName
	xref	ConvertFloor
	xref	DoomReMapTable

	xref	PicX,PicY,PicXBytes,PicNumPlanes

	xref	ImageBGReMap

	xref	MSGUserBreak


	section	FLOOR,CODE

	xdef	DoConvertFloor


;
;	* * * * * * *       CONVERT DOOM FLOOR       * * * * * * *
;
DoConvertFloor
	move.w	#320,PicX
	move.w	#(320/16*2),PicXBytes
	move.w	#200,PicY
	move.w	#8,PicNumPlanes
	jsr	ReadDoomPalette				; Read PLAYPAL
	bne	DCF900					; Error!
	jsr	OpenPictureSNES				; Open the picture
	bne	DCF900					; Error!
;
	move.l	DosBase,a6
	move.l	ConvertFloor,d1				; Open Floor Texture
	move.l	#1005,d2
	jsr	_LVOOpen(a6)
	move.l	d0,d4
	beq.s	DCF900
	move.l	d4,d1					; Read Floor Texture
	move.l	DoomWADData,d2
	move.l	#4096,d3
	jsr	_LVORead(a6)
	move.l	d4,d1					; Close Floor Texture
	jsr	_LVOClose(a6)
;
	jsr	SetPicPlanes				; Set Picture Planes
	tst.b	ImageBGReMap				; ReMap BG from 0 to 255?
	beq.s	DCF200					; No
	jsr	ConvertImageReMapBG			; Yes!
DCF200

	move.l	DoomWADData,a4
	moveq.l	#0,d5
DCF300
	moveq.l	#0,d4
DCF400
	moveq.l	#0,d0
	move.b	(a4)+,d0
	lea	DoomReMapTable,a0			; REMAP Table
	move.b	(a0,d0.w),d0
	jsr	SetPixReg
	addq.w	#1,d4
	cmp.w	#64,d4
	bne.s	DCF400
	addq.w	#1,d5
	cmp.w	#64,d5
	bne.s	DCF300
;
	move.l	OutputName,d1				; Save Output File
	jsr	WriteIFF
	jsr	ClosePicture
DCF900
	moveq.l	#0,d7
	rts


	end
