;**********************************************************************
;*                                                                    *
;*                      P R O J E C T :   R A G E                     *
;*                                                                    *
;*                              XBAND HARDWARE EQUATES/DEFINITIONS    *
;*                                                                    *
;**********************************************************************


;
;	* * * * * * *       GAME FEATURES       * * * * * * *
;
usePOCKY		equ	0		; 1=Enable POCKY Board Version

;
;	* * * * * * *       XBAND HARDWARE       * * * * * * *
;
XBANDROM		equ	$D00000
XBANDRAM		equ	$E00000

;
;	* * * * * * *       "FRED" HARDWARE       * * * * * * *
;
FRED			equ	$FBC001
KILL			equ	FRED+$3C00
CONTROL			equ	FRED+$3C02
RANDY			equ	$524E4C

translation_address0	equ	FRED+($00<<1)
translation_address1	equ	FRED+($04<<1)
translation_address2	equ	FRED+($08<<1)
translation_address3	equ	FRED+($0c<<1)
translation_address4	equ	FRED+($10<<1)
translation_address5	equ	FRED+($14<<1)
translation_address6	equ	FRED+($18<<1)
translation_address7	equ	FRED+($1c<<1)
translation_address8	equ	FRED+($20<<1)
translation_address9	equ	FRED+($24<<1)
translation_address10	equ	FRED+($28<<1)
range0start		equ	FRED+($2c<<1)
range1start		equ	FRED+($30<<1)

magic_address		equ	FRED+($38<<1)

range0end		equ	FRED+($40<<1)
range1end		equ	FRED+($44<<1)

range0destination	equ	FRED+($50<<1)
range0mask		equ	FRED+($52<<1)

range1destination	equ	FRED+($54<<1)
range1mask		equ	FRED+($56<<1)

ram_base		equ	FRED+($60<<1)
ram_bound		equ	FRED+($64<<1)
vector_table_base	equ	FRED+($68<<1)
hit_enables		equ	FRED+($6c<<1)
rom_bound		equ	FRED+($70<<1)
rom_base		equ	FRED+($74<<1)
snes_control		equ	FRED+($78<<1)
sram_protect		equ	FRED+($79<<1)
address_status		equ	FRED+($7c<<1)

smart_control		equ	FRED+($80<<1)
smart_status		equ	FRED+($84<<1)
vsync_count		equ	FRED+($88<<1)
modem_control_1		equ	FRED+($8c<<1)
modem_transmit_buffer	equ	FRED+($90<<1)
modem_receive_buffer	equ	FRED+($94<<1)
modem_status_2		equ	FRED+($98<<1)
serial_vertical_count	equ	FRED+($9c<<1)
modem_status_1		equ	FRED+($a0<<1)
guard_count		equ	FRED+($a4<<1)
baud_count_divisor	equ	FRED+($a8<<1)
modem_control_2		equ	FRED+($ac<<1)
write_vsync		equ	FRED+($b0<<1)
led_data		equ	FRED+($b4<<1)
enable_led		equ	FRED+($b5<<1)
;
ring_check		equ	FRED+($cf<<1)

mdTop_LED_Mask		equ	$02
mdMiddle_LED_Mask	equ	$08
mdBottom_LED_Mask	equ	$20


;
;	* * * * * * *       ROCKWELL MODEM HARDWARE       * * * * * * *
;
;	MODEM CONTROL REGISTER 1 BITS
;
mc1b_BREAK		equ $80
mc1b_ODDPARITY		equ $40
mc1b_ENABLEPARITY	equ $20
mc1b_ONESTOP		equ $10
mc1b_ENABLESTOP		equ $08
mc1b_BIT8		equ $04
mc1b_RESETSERIAL	equ $02
mc1b_ENABLESERIAL	equ $01


;
;	* * * * * * *       "POCKY" BOARD HARDWARE       * * * * * * *
;
rxd_A	    equ	    $21C0
txd_A	    equ	    $21C0
ier_A	    equ	    $21C1	; int enable reg
dll_A	    equ	    $21C0	; divisor latch lo
dlh_A	    equ	    $21C1	; divisor latch hi
iir_A	    equ	    $21C2	; int id reg
fcr_A	    equ	    $21C2	; fifo ctl reg
lcr_A	    equ	    $21C3	; line ctl reg
mcr_A	    equ	    $21C4	; modem ctl reg
lsr_A	    equ	    $21C5	; line status reg
msr_A	    equ	    $21C6	; modem status reg
scr_A	    equ	    $21C7	; scratch pad reg

rxd_B	    equ	    $21D0
txd_B	    equ	    $21D0
ier_B	    equ	    $21D1	; int enable reg
dll_B	    equ	    $21D0	; divisor latch lo
dlh_B	    equ	    $21D1	; divisor latch hi
iir_B	    equ	    $21D2	; int id reg
fcr_B	    equ	    $21D2	; fifo ctl reg
lcr_B	    equ	    $21D3	; line ctl reg
mcr_B	    equ	    $21D4	; modem ctl reg
lsr_B	    equ	    $21D5	; line status reg
msr_B	    equ	    $21D6	; modem status reg
scr_B	    equ	    $21D7	; scratch pad reg

port_A_baud		equ	57600
port_B_baud		equ	4800
port_B_baud_SGP		equ	57600

port_A_baud_val		equ	(115200/port_A_baud)
port_B_baud_val		equ	(115200/port_B_baud)
port_B_baud_SGP_val	equ	(115200/port_B_baud_SGP)


;
;	* * * * * * *       OS AND HARDWARE DEFINES       * * * * * * *
;
kGPInitGamePatch	equ	0
kGPStartGamePatch	equ	1
kGPDisplayMessage	equ	2
kGPStopGame		equ	3
kGPKillGame		equ	4
kGPReportError		equ	5
kGPPlaySinglePlayerGame	equ	6

kGameTimeout		equ	-426
kCallWaitingErr		equ	-427
kRemoteCallWaitingErr	equ	-428
kSinglePlayerGameOver	equ	-709
kNetRegisterTimeout	equ	-710

kOverrunError		equ	-425	; $fe57
kTimeout		equ	-426	; $fe56
kGibbledPacket		equ	-601	; $fda7
kNoData			equ	-602	; $fda6
kOutOfSync		equ	-901	; $fc76

gRegisterBase		equ	$c0	; where Fred is (required if you move Fred)

kOSHandleGameError	equ	0
kOSGameOver		equ	1
kOSCheckLine		equ	24

kResetCPU		equ	14


;
;	* * * * * * *       XBAND GLOBAL VARIABLES       * * * * * * *
;
gTicks			equ	$e00020		; TimerTicks
gSessionIsMaster	equ	$e02c44		; 0 = Slave, !=0 = Master
;
_XBOSDispatcher		equ	$e000cc
;
kHWKeyboardBitTimeConst		equ	7	; BitDelay between BitPairs
kHardwareKeyboardIDConst	equ	$78	; Hardware ID Code


;
;	* * * * * * *       RICHARD KISS'S JML MACRO       * * * * * * *
;
;	Call with:	Address of Code to Call that ends in RTS
;			Address of RTL in same Bank as Code
;
RICHJML	MACRO
	phk
	pea	#<(*+10-1)
	pea	#<(@1-1)
	jml	@0
	ENDMAC


;
;	* * * * * * *       COMMUNICATIONS       * * * * * * *
;
XBFrameLatency		equ	1		; Frame Latency
;Packet_Prefill		equ	2		; 2 frames Pre Latency (Local->Global Data)
Packet_Size		equ	8		; #Bytes per Packet
XBModemBufferSize	equ	256
XBModemBufferMask	equ	XBModemBufferSize-1
