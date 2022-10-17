#include <stdio.h>
#include <stdlib.h>
#include <mffp.h>
#include <fcntl.h>

short   InverseTable[0x8000];
char    InverseFileName[256] = "InverseTable.bin";
long    InverseFile;


long    FlipWord (long data)
{
	return( ( (data & 0xff) << 8) + ( (data & 0xff00) >> 8) );
}


main()
{
	double	res;

	long	count;
	long	tot_count=16384;

/*
 *	CALCULATE INVERSE TABLE
 */
	printf	("InvTable\n");

	for (count=1; count<tot_count; count++) {
		if (count == 1) {
			res = 32767.0/32768.0;
		}
		else {
			res = (1.0/(double) count);
		}
		if (count <= 128) {
			res *= 32768;
		}
		else {
			res *= 32768*128;
		}
/*		printf	("\tdc.w\t$%04lx\t\t; 1/%ld\n",
			(long)((long)res &0xffffL),
			(long) count
			);
*/
		InverseTable[count] = FlipWord(((long) res&0xffffL));
	};

	InverseFile = open(InverseFileName, O_WRONLY | O_TRUNC | O_CREAT, 0);
	write(InverseFile, &InverseTable[0], 0x8000);
}
