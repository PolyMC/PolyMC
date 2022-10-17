#include <stdio.h>
#include <stdlib.h>
#include <mffp.h>

main()
{
	double	res;

	long	count;
	long	tot_count=7168*1.25;	/*8960*/
	long	quiescent_count=128;

/*
 *	CALCULATE SCREEN PIXEL -> RL WORLD PIXEL SCALING TABLE
 */
	printf	("SCNRLPixScale\n");

	for (count=0; count<tot_count; count++) {
		res = ((double) count/(double) quiescent_count);
		res *= 256.0;
		if (res==0) res=2;
		printf	("\tdc.w\t$%04lx\t\t; %ld\n",
			(long)res,
			(long)count
			);
	};


/*
 *	CALCULATE RL WORLD PIXEL -> SCREEN PIXEL SCALING TABLE
 */
	printf	("RLSCNPixScale\n");

	for (count=0; count<tot_count; count++) {
		if (count==0) {
			res=128.0*65536.0;
		}
		else {
			res = (quiescent_count/(double) (count));
			res *= 65536.0;
		}
		printf	("\tdc.l\t$%06lx\t\t; %ld\n",
			(long)( (((long)res>>16)<<16)|(((long)res&0xffffL)>>1) ),
			(long) count
			);
	};
}
