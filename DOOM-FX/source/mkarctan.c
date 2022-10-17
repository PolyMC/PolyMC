#include <stdio.h>
#include <mffp.h>

main()
{
	double	deg;
	double	res;

	long	deg_count;
	long	tot_deg=1024;

/*
 *	CALCULATE POSITIVE ARCTAN TABLE
 */
	printf	("PArcTanTable\n");

	for (deg_count=0; deg_count<tot_deg; deg_count++) {

		deg = atan((float) ((float)deg_count/(float)tot_deg)) ;
		deg /= ((float) 0.017453293);
		res = deg*((float)65536/(float)360);

		printf	("\tdc.w\t$%04lx\t\t; (%03ld/%ld) (%03.6f deg)\n",
			(long)res,
			(long) deg_count,
			(long) tot_deg,
			(double)deg
			);
	}

}
