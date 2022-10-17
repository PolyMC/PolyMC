#include <stdio.h>
#include <mffp.h>

main()
{
	double	deg;
	double	res;

	long	deg_count;
	long	tot_deg=1024;
	long	max_deg;
	max_deg = tot_deg + (tot_deg/4);

/*
 *	CALCULATE SIN/COS TABLE
 */
	deg=0;
	printf	("SinTable\n");

	for (deg_count=0; deg_count<max_deg; deg_count++) {

		if (deg_count==(tot_deg/4)) printf("CosTable\n");

		res = (deg*(float) 0.017453293);
		res = sin(res);
		res *= (double) 32768;

		if (res==32768) res=32767;
		if (res==-32768) res=-32767;
		if (res<0) {
			res=65536+res;
		}

		printf	("\tdc.w\t$%04lx\t\t; (%03ld/%ld) (%03.6f deg)\n",
			(long)res,
			(long) deg_count,
			(long) tot_deg,
			(double)deg
			);

	deg += (double) ((double) 360/(double) tot_deg);
	}

}
