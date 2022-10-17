#include <stdio.h>
#include <mffp.h>

main()
{
	double	deg;
	double	res,res2;

	long	deg_count;
	long	tot_deg=1024;
	long	max_deg;
	max_deg = (tot_deg/8)+1;

/*
 *	CALCULATE SEC TABLE
 */
	deg=0;
	printf	("SecTable\n");

	for (deg_count=0; deg_count<max_deg; deg_count++) {

		res = (deg*(float) 0.017453293);

		res=(((double)1.0)/cos(res));
		res -= (double)1.0;
		res2 = res*(double)65536.0;

		printf	("\tdc.w\t$%04lx\t\t; (%03ld/%ld) (%03.6f deg)\n",
			((long)res2&0xffffL),
			(long) deg_count,
			(long) tot_deg,
			(double)deg
			);

	deg += (double) ((double) 360/(double) tot_deg);
	}

}
