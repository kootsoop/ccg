
#include <stdio.h>
#include <math.h>
#define PI 3.1415926535

main(argc,argv)

int	argc;
char	*argv[];

{
  int	i, N, datatype, firstnonchar;
  double dummy, SRate, FStart, FStop, Phase, phi;

	if ((argc<5) || (argc>6))
	{fprintf(stderr,"Usage: lin [-c] N START_FREQ STOP_FREQ PHASE(DEGS)\n");
	  exit(1);
	}

	firstnonchar = argc-4;

	SRate = 1.0;
	N     = atoi(argv[firstnonchar++]);
	FStart = atof(argv[firstnonchar++]);
	FStop = atof(argv[firstnonchar++]);
	Phase = atof(argv[firstnonchar]);
/*        fprintf(stderr,"FStart%lg\n",FStart);
        fprintf(stderr,"FStop%lg\n",FStop);
        fprintf(stderr,"Phase%lg\n",Phase); */
	if (FStart>0.5 || FStart < 0.0) 
	{
          fprintf(stderr,"ERROR : Must have 0 < FStart < 0.5\n");
          exit(2);
	}
	if (FStop>0.5 || FStop < 0.0) 
	{
            fprintf(stderr,"ERROR : Must have 0 < FStop < 0.5\n");
            exit(2);
	}
	
	if (argc==5) 
            printf("1\n");
        else
            printf("2\n");
	printf("%d\n", N);
	printf("1.0\n");
	for (i=0; i<=N; i++) 
	{
	     phi= 2.0*PI*i*(FStart+0.5*(FStop-FStart)*i/(N-1.0))+PI*Phase/180.0;
	if (argc==5)
	    {printf("%lg\n",
	     cos(phi));}
	else
		{printf("(%lg,%lg)\n",cos(phi),sin(phi));}
	}
}

