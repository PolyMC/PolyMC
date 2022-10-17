;**********************************************************************
;*                                                                    *
;*                   P R O J E C T :   REALITY_ENGINE                 *
;*                                                                    *
;*                                 TRANSFER STRUCTURE DEFINITIONS     *
;*                                                                    *
;**********************************************************************



;
;	* * * * * * *       GENERAL FLAGS AND EQUATES       * * * * * * *
;
xferNastyMax		equ	(5*1024)	; 5K Maximum per NastyTransfer

;
;	* * * * * * *       TRANSFER STRUCTURE       * * * * * * *
;
xfs		var	0
xfsType		equ	xfs			; Type of Transfer
xfs		var	xfs+2
xfsSource	equ	xfs			; Source Address
xfs		var	xfs+4
xfsDest		equ	xfs			; Destination Address
xfs		var	xfs+4
xfsLen		equ	xfs			; Length (used for fill)
xfsData0	equ	xfs			; Data0  (AND Mask)
xfs		var	xfs+2
xfsData1	equ	xfs			; Data1  (ORA Mask)
xfs		var	xfs+2
xfsData2	equ	xfs			; Data2  (Pointer to ADD Value)
xfs		var	xfs+2
xfsSize		equ	xfs
;
;	>>>   TRANSFER STRUCTURE TYPES   <<<
;
xftEND		equ	$8000			; End
xftRAW		equ	$4000			; Data is NOT Compressed
xftNODATA	equ	$2000			; NO DATA associated with this XFER
xftCODE		equ	(00*2)			; Code Execution
xftRAM		equ	(01*2)			; XFer Source->Dest RAM
xftVRAM8L	equ	(02*2)			; XFer Source->Dest VRAM 8L
xftVRAM8H	equ	(03*2)			; XFer Source->Dest VRAM 8H
xftVRAM16	equ	(04*2)			; XFer Source->Dest VRAM 16
xftCGRAM	equ	(05*2)			; XFer Source->Dest CGRAM
xftVRAMF8L	equ	(06*2)			; Fill Source->Dest VRAM 8L
xftVRAMF8H	equ	(07*2)			; Fill Source->Dest VRAM 8H
xftVRAMF16	equ	(08*2)			; Fill Source->Dest VRAM 16
xftVRAMM8L	equ	(09*2)			; Mask Source->Dest VRAM 8L
xftVRAMM8H	equ	(10*2)			; Mask Source->Dest VRAM 8H
xftVRAMM16	equ	(11*2)			; Mask Source->Dest VRAM 16
xftVRAMD16	equ	(12*2)			; XFer Source->Dest VRAM 16 (Add Base to Dest)
xftVRAM8HC	equ	(13*2)			; XFer Source->Dest VRAM 8H (Continuation)
