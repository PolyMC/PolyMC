;**********************************************************************
;*                                                                    *
;*                   P R O J E C T :   REALITY_ENGINE                 *
;*                                                                    *
;*                                    PLAYER STRUCTURE DEFINITIONS    *
;*                                                                    *
;**********************************************************************


;
;	* * * * * * *       JOYSTICK BUTTON CONFIGURATION       * * * * * * *
;
pjFORWARD		equ	joyUP		; FORWARD
pjREVERSE		equ	joyDOWN		; REVERSE
pjROTATELEFT		equ	joyLEFT		; ROTATE LEFT
pjROTATERIGHT		equ	joyRIGHT	; ROTATE RIGHT
pjSTRAFELEFT		equ	joyL		; STRAFE LEFT
pjSTRAFERIGHT		equ	joyR		; STRAFE RIGHT
 	ife	useID8
pjFIRE			equ	joyX		; FIRE
pjRUN			equ	joyA		; RUN
pjUSE			equ	joyB		; USE/OPERATE/OPEN
pjWEAPON		equ	joyY		; WEAPON CYCLE
	endif
	ifn	useID8
pjFIRE			equ	joyY		; FIRE
pjRUN			equ	joyB		; RUN
pjUSE			equ	joyA		; USE/OPERATE/OPEN
pjWEAPON		equ	joyX		; WEAPON CYCLE
	endif
pjAUTOMAP		equ	joySELECT	; AUTOMAP TOGGLE
pjPAUSE			equ	joySTART	; PAUSE
;
;	* * * * * * *       MOUSE BUTTON CONFIGURATION       * * * * * * *
;
pmFIRE			equ	joyX		; FIRE
pmUSE			equ	joyA		; USE/OPERATE/OPEN
