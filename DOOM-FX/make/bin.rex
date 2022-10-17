/*  MAKE BINARY ROM IMAGE  */

OPTIONS RESULTS
ADDRESS COMMAND

'VERSION >T:1 RLOBJ:RL.'
dummy1 = open('1','T:1','read')
RLVersion = RIGHT(readln('1'),4)
dummy1 = close('1')


MakeBinCmd = "xr -v -eRLOBJ:RL. -s$80000000 -z$200000 -k -y -xRLOBJ:RL" || RLVersion || ".BIN" || '0A'X
'ECHO ' || MakeBinCmd
MakeBinCmd
EXIT
