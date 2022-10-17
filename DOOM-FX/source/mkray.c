#include <stdio.h>
#include <mffp.h>

main()
{
	double	deg;
	double	res,res2;
	double	LeftX,LeftY,RightX,RightY;

	long	deg_count;
	long	tot_deg=1024;
	double	plane=108.0;
	long	quiescent_count=128;

/*
 *	CALCULATE LINE-OF-SIGHT RAY TABLE
 */
	deg=0;
	printf	("SightRayTable\n");

	for (deg_count=0; deg_count<tot_deg; deg_count++) {

		res = (deg*(float) 0.017453293);

		res2=cos(res);
		res=sin(res);

/*
 *	ROTATED_X = 	 cos(90deg.-PlayerAngle) * (VertexX)
 *			-sin(90deg.-PlayerAngle) * (VertexY)
 *
 *	ROTATED_Y = 	 sin(90deg.-PlayerAngle) * (VertexX)
 *			+cos(90deg.-PlayerAngle) * (VertexY)
 */

	LeftX = ( (res2*plane)
		 -(res*( (108.0*320.0/256.0)/(quiescent_count/plane) )) )
		 *(16384.0/(plane*2));

	LeftY = ( (res*plane)
		 +(res2*( (108.0*320.0/256.0)/(quiescent_count/plane) )) )
		 *(16384.0/(plane*2));

		printf	("\tdc.w\t$%04lx,$%04lx\t\t; (%03ld/%ld) (%03.6f deg)\n",
			((long)LeftX&0xffffL),
			((long)LeftY&0xffffL),
			(long) deg_count,
			(long) tot_deg,
			(double)deg
			);

	RightX = ( (res2*plane)
		  -(res*-( (108.0*2.0*320.0/256.0)/(quiescent_count/plane) )) )
		  *(16384.0/(plane*2));

	RightY = ( (res*plane)
		  +(res2*-( (108.0*2.0*320.0/256.0)/(quiescent_count/plane) )) )
		  *(16384.0/(plane*2));

		printf	("\tdc.w\t$%04lx,$%04lx\n",
			((long)RightX&0xffffL),
			((long)RightY&0xffffL),
			(long) deg_count,
			(long) tot_deg,
			(double)deg
			);

	deg += (double) ((double) 360/(double) tot_deg);
	}


/*
 *	CALCULATE SCREEN-Y-INV-SLOPE TABLE
 */
	printf	("\nSlopeYInvTable\n");

	for (deg_count=72; deg_count>=1; deg_count--) {
		res = (((double)quiescent_count/(double)deg_count)*512.0);
		if (res>32767) res=32767.0;
		printf	("\tdc.w\t$%04lx\t\t\t; %ld\n",
			(long)( ((long)res &0xffffL)),
			(long) (72-deg_count)
			);
	}
	for (deg_count=1; deg_count<=72; deg_count++) {
		res = (((double)quiescent_count/(double)deg_count)*512.0);
		if (res>32767) res=32767.0;
		printf	("\tdc.w\t$%04lx\t\t\t; %ld\n",
			(long)( ((long)res &0xffffL)),
			(long) (deg_count+71)
			);
	}


/*
 *	CALCULATE SCREEN-X-ANGLE ADJUSTMENT TABLE
 */
	printf	("\nScreenXAngleTable\n");

	for (deg_count=108; deg_count>-108; deg_count-=2) {

		deg = ((atan(deg_count/(float)108.0))/(float) 0.017453293);
		res = deg*((float)65536.0/(float)360.0);

		if (res>32767) res=32767;
		if (res<-32767) res=-32767;
		if (res<0) {
			res=65536+res;
		}
		printf	("\tdc.w\t$%04lx\t\t\t; (%03ld) (%03.6f deg)\n",
			((long)res&0xffffL),
			(long) 108-deg_count,
			(double)(deg)
			);
	}

}
