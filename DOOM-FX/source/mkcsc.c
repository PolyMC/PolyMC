#include <stdio.h>
#include <mffp.h>

main()
{
	double	deg;
	double	res,res2;

	long	deg_count;
	long	tot_deg=1024;
	long	max_deg;
	max_deg = tot_deg + (tot_deg/4);

/*
 *	CALCULATE CSC/SEC TABLE
 */
	deg=0;
	printf	("CscTable\n");

	for (deg_count=0; deg_count<max_deg; deg_count++) {

		if (deg_count==(tot_deg/4)) printf("SecTable\n");

		res = (deg*(float) 0.017453293);

		res=sin(res);
		if (res==0) res2=0x524c;
		else {
			res = 1/res;

			if (res<0) res2=0-res;
			else res2=res;

			if (res2>=128) res2=0x7fff;
			else res2 *= 256;

			if (res<0) res2=0-res2;
		}

		printf	("\tdc.w\t$%04lx\t\t; (%03ld/%ld) (%03.6f deg)\n",
			((long)res2&0xffffL),
			(long) deg_count,
			(long) tot_deg,
			(double)deg
			);

	deg += (double) ((double) 360/(double) tot_deg);
	}

}
