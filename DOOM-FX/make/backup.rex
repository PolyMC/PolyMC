/*  BACKUP ENTIRE RL DIRECTORY STRUCTURE TO :RLBACKUPS/RL_YYMMDD  */

/*  Get Current Date in YYMMDD Format  */
FileDate=RIGHT(DATE(SORTED),6)

/*  Determine the last letter used  */
DO FileCount=1 TO 26 BY 1
	FileBase=":RLBACKUPS/RL_" || FileDate || SUBSTR('ABCDEFGHIJKLMNOPQRSTUVWXYZ',FileCount,1)
	IF ~(EXISTS(FileBase)) THEN DO
		CopyCommand="COPY CLONE RL: " || FileBase || " ALL QUIET"
		ADDRESS command
		'ECHO NOLINE >>TODO "@Backup (' || FileBase || ')  "'
		'VERSION >>TODO RLOBJ:RL.'
		'ECHO '||CopyCommand
		CopyCommand
		EXIT
	END
END
SAY 'ERROR!  All FileDirectories for the current date are taken!'
EXIT 20
