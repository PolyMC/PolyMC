#include <stdio.h>
#include <mffp.h>

main()
{
	double	deg;
	double	res;

	long	deg_count;
	long	tot_deg=1024;

/*
 *	CALCULATE TANGENT TABLE
 */
	deg=0;
	printf	("TanTable\n");

	for (deg_count=0; deg_count<tot_deg; deg_count++) {

		res = (deg*(float) 0.017453293);
		res = tan(res);
		res *= (double) 128;

		if (res>32767) res=32767;
		if (res<-32768) res=-32767;

		printf	("\tdc.w\t$%04lx\t\t; (%03ld/%ld) (%03.6f deg)\n",
			((long)res&0xffffL),
			(long) deg_count,
			(long) tot_deg,
			(double)deg
			);

	deg += (double) ((double) 360/(double) tot_deg);
	}

}
