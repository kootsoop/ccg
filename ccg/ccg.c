/************************************************************

WARNING: THERE IS NO GUARANTEE OR WARRANTY WITH THIS CODE.

*************************************************************
*
*  routine name : ccg - Cohen's Class Generator
*
*  routine operation :
*
*      ccg [-T[WVPcBCRP] [-whHbr] [-ln] [-in] [-N n]] [-s r] [file]
*                      Cohen's Class Generator
*      Options: T  W : Wigner
*                  V : Wigner-Ville
*                  p : Periodogram
*                  c : Correlogram
*                  B : Born-Jordan-Cohen
*                  C : Choi-Williams
*                  R : Rihaczek
*                  P : Page
*               w  h : hanning window
*                  H : Hamming window
*                  b : Blackman window
*                  r : Rectangular window
*                  B : Blackman-Harris window
*               l  n : Make window "n" points long
*               N  n : Make each TFD plot "n" points long
*               s  r : Choi-Williams parameter "r"
*
*
*  input description : one file containing a type 1 TFD file
*                   OR one file of doubleing point numbers only
*
*  output description : a type 3 TFD file is written to stdout
*
*  author : Peter Kootsookos
*           Dept. Elect. Eng., Uni. Qld., St. Lucia, 4067,
*           Australia.
*
*  date : start : 6-2-89
*
*  based on :
*
*  problems : Could use the discrete cosine transform
              instead of the Fast Fourier Transform,
              since all TFDs used are REAL.
*
*  acknowledgements :
*
*  modifications :
*
*  Other comments :
*
*************************************************************

WARNING: THERE IS NO GUARANTEE OR WARRANTY WITH THIS CODE.

*************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define  MAXHILBLENGTH  20	/* Hilbert transformer length */
#define  TIMEDEF        1	/* Time resolution default */
#define  LWINDDEF       0.25	/* Window Length default (fraction) */
#define  PI             3.141592653589796
#define  MAXLWIND       128	/* Maximum Window Length */
#define  MAXLWINDV2     65	/* Maximum Window Length /2 + 1 */
#define  CROSSTERMN     1024	/* Default Crossterm signal length */
#define  NMAX           1024	/* Maximum signal length */
#define  FORWARD        -1.0	/* Value of sw for forward FFT */
#define  REVERSE        1.0	/* Value of sw for reverse FFT */

FILE   *fp;			/* file pointer for file */
FILE   *fopen ();
int     Wigner,			/* TFD type flags */
        Ville,
        MIFFT,
	SWVD,
        PWVD,
        Period = 1,		/* DEFAULT SETTING */
        Corr,
        BJC,
        Choi,
        Riha,
        Page,
		median;
int     hann,			/* Window type flags */
        hamm,
        rect = 1,		/* DEFAULT SETTING */
        black,
        blhar;


int
fft (register double *ar, register double *ai, int n, double sw);

int
main (argc, argv)
    int     argc;
    char   *argv[];

{
    extern int
            Wigner,		/* TFD type flags */
            Ville,
            MIFFT,
	    SWVD,
            PWVD,
            Period,
            Corr,
            BJC,
            Choi,
            Riha,
            Page,
            median;

    extern int
            hann,		/* Window type flags */
            hamm,
            rect,
            black,
            blhar;

    int     nplot,		/* points per TFD plot */
            timeres = 0,	/* time resolution between plots */
            current_time,	/* current position in signal */
            lwind = 0,		/* window length */
            half_lwind,		/* (window length - 1)/2 */
            N,			/* signal length */
            NSpec,		/* No. of points is spectrum */
            nplots,		/* No. of TFD plots */
            wind,		/* window type */
            num_stars,		/* Number of stars output */
            num_added,		/* Number of added points */
            CROSS = 0,
            QUIET = 0,
            i,
            j,			/* counter variables */
			jmax, jmin;	/* ditto */
    double  sig_real[NMAX],	/* real part of signal */
            sig_imag[NMAX],	/* imaginary part of signal */
            sig_real2[NMAX],	/* real part of signal 2 */
            sig_imag2[NMAX],	/* imaginary part of signal 2 */
            xr[NMAX],		/* real part of dummy */
            xi[NMAX],		/* imaginary part of dummy */
            window[MAXLWIND],	/* window function to be used */
            bil_prod_r[MAXLWIND][MAXLWINDV2],
    /* real part of bilinear product */
            bil_prod_i[MAXLWIND][MAXLWINDV2],
    /* imag part of bilinear product */
            fsam,		/* Sampling frequency */
            freqdiff,		/* Difference frequency */
            freq_1,		/* Frequency 1 for crossterms */
            freq_2,		/* Frequency 2 for crossterms */
            wt,			/* weighting for current time CW */
            sigma,		/* parameter for Choi-Williams */
			min,		/* minimum value */
			max,		/* maximum value */
			next;
    char   *s,			/* Working char */
            tfdtype,		/* TFD Type from flag */
            windtype;		/* Window type from flag */

/* Now declare functions */
    void    Zero_TFD_Flags (),
            Zero_Window_Flags (),
            form_product (),
            get_signal (),
            PrintUsage();
    int     Rad2 ();


    if (argc==1)
      PrintUsage();
/********************************************************************
                    DO ARGUMENT PROCESSING
********************************************************************/

    while (--argc > 0 && (*++argv)[0] == '-') {
      fprintf(stderr,"argc = %d\n", argc);
	s = argv[0] + 1;
	switch (*s) {
	case 'Q':		/* No percentage done output to stderr */
	    QUIET = 1;
	    break;
	case 'X':		/* Select crossterm output */
	    CROSS = 1;
	    if (*++s != '\0')
		freqdiff = atof (s);
	    else {
		++argv;
		freqdiff = 10.0;
		if ((((--argc > 0) && (*argv[0] >= '0') && (*argv[0] <= '9')) || *argv[0] == '.'))
		    freqdiff = atof (argv[0]);
	    }
	    if (fabs (freqdiff) > 0.5) {
		--argv;
		++argc;
	    }
	    break;
	case 'T':		/* Select Type of TFD */
	    if (*++s != '\0')
		tfdtype = *s;
	    else {
		++argv;
		tfdtype = *s;
	    }
	    Zero_TFD_Flags ();
	    switch (tfdtype) {
	    case 'W':		/* Wigner */
		Wigner = 1;
		break;
	    case 'V':		/* Wigner-Ville */
		Ville = 1;
		break;
	    case 'p':		/* Periodogram */
		Period = 1;
		break;
	    case 'c':		/* Correlogram */
		Corr = 1;
		break;
	    case 'B':		/* Born-Jordan-Cohen */
		BJC = 1;
		break;
	    case 'C':		/* Choi-Williams */
		Choi = 1;
		break;
	    case 'R':		/* Rihaczek */
		Riha = 1;
		break;
	    case 'P':		/* Page */
		Page = 1;
		break;
	    case 'I':		/* MIFFT */
		MIFFT = 1;
		break;
	    case 'S':		/* SWVD */
		SWVD = 1;
		break;
	    case 's':		/* Parameterized WVD */
		PWVD = 1;
		break;
	    case 'M':		/* Median Filtered WVD */
		median = 1;
		break;
	    default:
		fprintf (stderr, "ccg: TFD type invalid.\n");
		exit (1);
		break;
	    }
	    break;
	case 'w':		/* Select WINDOW TYPE */
	    if (*++s != '\0')
		windtype = *s;
	    else {
		++argv;
		windtype = *s;
	    }
	    Zero_Window_Flags ();
	    switch (windtype) {
	    case 'h':		/* hanning window */
		hann = 1;
		break;
	    case 'H':		/* Hamming analysis window */
		hamm = 1;
		break;
	    case 'r':		/* Rectangular window (DEFAULT) */
		rect = 1;
		break;
	    case 'b':		/* Blackman window */
		black = 1;
		break;
	    case 'B':		/* Blackman-Harris window */
		blhar = 1;
		break;
	    default:
		fprintf (stderr, "ccg: window invalid.\n");
		exit (2);
		break;
	    }
	    break;
	case 'N':		/* Points per plot */
	    if (*++s != '\0')
		nplot = atoi (s);
	    else {
		++argv;
		nplot = -1;
		if ((--argc > 0) && (*argv[0] >= '0') && (*argv[0] <= '9'))
		    nplot = atoi (argv[0]);
	    }
	    if (nplot < 0) {
		--argv;
		++argc;
	    }
	    break;
	case 'i':		/* Time resolution */
	    if (*++s != '\0')
		timeres = atoi (s);
	    else {
		++argv;
		timeres = -1;
		if ((--argc > 0) && (*argv[0] >= '0') && (*argv[0] <= '9'))
		    timeres = atoi (argv[0]);
	    }
	    if (timeres < 0) {
		timeres = TIMEDEF;
		--argv;
		++argc;
	    }
	    break;
	case 'l':		/* Window Length */
	    if (*++s != '\0')
		lwind = atoi (s);
	    else {
		++argv;
		lwind = -1;
		if ((--argc > 0) && (*argv[0] >= '0') && (*argv[0] <= '9'))
		    lwind = atoi (argv[0]);
	    }
	    if (lwind < 0) {
		--argv;
		++argc;
	    }
	    if ((lwind % 2 == 0) || (lwind > MAXLWIND)) {
		fprintf (stderr, "Window wrong !\n");
		exit (3);
	    }
	    break;
	case 's':		/* Set Choi-Williams parameter */
	    if (*++s != '\0')
		sigma = atof (s);
	    else {
		++argv;
		sigma = -1.0;
		if ( (((--argc > 0) && (*argv[0] >= '0') && (*argv[0] <= '9')) || *argv[0] == '.'))
		    sigma = atof (argv[0]);
	    }
	    if (sigma < 0) {
		--argv;
		++argc;
	    }
	    break;
	default:		/* Give Error Message */
	    argc = 0;
	    break;
	}
    }

    if (argc <= -1) {		/* Give usage message */
      PrintUsage();
    }



    if (PWVD && (sigma <= 0.0)) {
	fprintf (stderr,
		 "ccg: -s switch not set when using MIFFT\n");
	return (6);
    }
    if (SWVD && (sigma <= 0.0)) {
	fprintf (stderr,
		 "ccg: -s switch not set when using Smoothed WVD\n");
	return (6);
    }
    if (Choi && (sigma <= 0.0)) {
	fprintf (stderr,
		 "ccg: -s switch not set when using Choi-Williams\n");
	return (6);
    }

/********************************************************************
                 ARGUMENT PROCESSING FINISHED
            NOW SET UP FOR COHEN'S CLASS CALCULATION
********************************************************************/

/* Read signal in, after working out where from, and whether it is
                              possible.                           */
    if (!CROSS) {
	if (argc < 1)
	    fp = stdin;
	else if ((fp = fopen (*argv, "r")) == NULL) {
	    fprintf (stderr, "ccg: can't open %s\n", *argv);
	    exit (5);
	}
	get_signal (fp, sig_real, sig_imag, &fsam, &N);
	for (i = 0; i < N; i++) {
	    sig_real2[i] = sig_real[i];
	    sig_imag2[i] = sig_imag[i];
	}
    }
    else {			/* if want crossterm output only */
	N = CROSSTERMN;
	fsam = 1.0;
	freq_1 = 0.25 - freqdiff / 2.0;
	freq_2 = 0.25 + freqdiff / 2.0;
	for (i = 0; i < N; i++) {
	    sig_real[i] = cos (2.0 * PI * freq_1 * (double) i);
	    sig_imag[i] = sin (2.0 * PI * freq_1 * (double) i);
	    sig_real2[i] = cos (2.0 * PI * freq_2 * (double) i);
	    sig_imag2[i] = sin (2.0 * PI * freq_2 * (double) i);
	}
    }
/* select only real signal if Wigner Distribution required */
    if (Wigner)
	for (i = 0; i < N; i++) {
	    sig_imag[i] = 0.0;
	    sig_imag2[i] = 0.0;
	}
/* NOW DO SOME ERROR CHECKING AND SET DEFAULTS */
/* Default window length */
    if (lwind <= 0)
	lwind = 2 * (double) (N / 2.0) - 1;
/* Limit window length to ODD(N) */
    if (lwind > N)
	lwind = 2 * (double) (N / 2.0) - 1;

/* select default time resolution */
    if (timeres <= 0)
	timeres = TIMEDEF;
/* select points per plot */
    if (nplot < lwind)
	nplot = Rad2 (lwind);	/* make nplot at least lwind radix 2 */
    else
	nplot = Rad2 (nplot);	/* ensure selected nplot is radix 2 */

/* Output type 3 file header */
    NSpec = Rad2 (N) / 2;
    nplots = N / timeres;
/* find out which window type is being used,
   and calculate window function */
    if (rect) {
	wind = 1;
	for (i = 0; i < nplot; i++)
	    window[i] = 1.0;
    }
    if (hann) {			/* Hanning windows are type 2 */
	wind = 2;
	for (i = 0; i < nplot; i++)
	    window[i] = 0.5 - 0.5 * cos (2.0 * PI * (double) i / nplot);
    }
    if (hamm) {			/* Hamming windows are type 3 */
	wind = 3;
	for (i = 0; i < nplot; i++)
	    window[i] = 0.54 - 0.46 * cos (2.0 * PI * (double) i / nplot);
    }
    if (black) {		/* Blackman windows are type 4 */
	/* UNIMPLEMENTED */
	wind = 4;
	for (i = 0; i < nplot; i++)
	    window[i] = 1.0;
    }
    if (blhar) {		/* Blackman-Harris windows are type 5 */
	/* UNIMPLEMENTED */
	wind = 5;
	for (i = 0; i < nplot; i++)
	    window[i] = 1.0;
    }

/*    printf ("3\n%d\n%d\n%d\n%d\n%lg\n%d\n%d\n%d\n",
	    N, NSpec, nplot, nplots, fsam, timeres, wind, lwind); */

fprintf(stderr,"Only printing distribution data.\n");

/* Output signal 
    for (i = 0; i < N; i++) {
	/* copy signal into an array for ffting 
	xr[i] = sig_real[i];
	xi[i] = sig_imag[i];
	if (CROSS) {
	    xr[i] = 0.0;
	    xi[i] = 0.0;
	}
	printf ("%lg\n", xr[i]);
    } 
*/
/* take fft 
    fft (xr, xi, Rad2 (N), FORWARD);
/* Output spectrum 
    for (i = 0; i < NSpec; i++)
	printf ("%lg\n", (double) (xr[i] * xr[i] + xi[i] * xi[i]));
*/

/********************************************************************
    MAIN LOOP INITIALIZATION
********************************************************************/
    current_time = 0;
    half_lwind = (lwind - 1) / 2;
/* form bilinear product */
    form_product (sig_real,
		  sig_imag,
		  sig_real2,
		  sig_imag2,
		  current_time,
		  timeres,
		  half_lwind,
		  N,
		  bil_prod_r,
		  bil_prod_i);

    if (!QUIET) {
	fprintf (stderr,
		 "\n____________________________________________________\n");
	fprintf (stderr,
		 "                  PERCENTAGE DONE                   \n");
	fprintf (stderr,
		 "____________________________________________________\n");
	fprintf (stderr,
		 "0...10...20...30...40...50...60...70...80...90...100\n");
	fprintf (stderr,
		 "____________________________________________________\n");
    }
/********************************************************************
    MAIN LOOP FOR CALCULATION OF COHEN'S CLASS MEMBER
********************************************************************/
    for (current_time = 0, num_stars = 0; current_time < N; current_time += timeres) {

	if (!QUIET) {
	    j = (int) (53.0 * (double) current_time / (double) N) - num_stars;
	    for (i = 1; i <= j; i++) {
/* This unreadable piece of code outputs an reverse video space */
/*		fprintf (stderr, "\033[7m \033[m"); */
		fprintf (stderr, "*");
		num_stars++;
	    }
	}
/* initialization of TFD array */

	for (i = 0; i < nplot; i++) {
	    xr[i] = 0.0;
	    xi[i] = 0.0;
	}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Perform time-lag convolution of bilinear product array with
   time-lag kernel function.

   Data is contained in the bilinear array :

      bil_prod_r[i][j] = Re(z[current_time+i+j]z*[current_time+i-j])
      where i,j > 0

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

	if (Wigner || Ville) {
	    for (i = 0; i <= half_lwind; i++) {
		xr[i] = bil_prod_r[half_lwind][i];
		xi[i] = bil_prod_i[half_lwind][i];
	    }
	}
	if (SWVD) {
	    for (i = 0; i <= half_lwind; i++) {
		for (j=(int)-(sigma*half_lwind); j<=(int)(sigma*half_lwind); j++){
		    xr[i] = xr[i] + bil_prod_r[half_lwind+j][i];
		    xi[i] = xi[i] + bil_prod_i[half_lwind+j][i];
                }
	    }
	}
	if (median) {
	    for (i = 0; i <= half_lwind; i++) {
			min = sqrt(bil_prod_r[0][i]*bil_prod_r[0][i] 
				+ bil_prod_i[0][i]*bil_prod_i[0][i]);
			max = min;
			jmax = -half_lwind;
			jmin = -half_lwind;
			for (j=(int)-half_lwind; j<=(int)half_lwind; j++){
				next = sqrt(
				bil_prod_r[half_lwind-j][i]*bil_prod_r[half_lwind-j][i] 
				+ bil_prod_i[half_lwind-j][i]*bil_prod_i[half_lwind-j][i] );
				if (min>next) {min = next; jmin = j;}
				if (max<next) {max = next; jmax = j;}
			}
			xr[i] = 0.5*(bil_prod_r[half_lwind-jmax][i] +
					bil_prod_r[half_lwind-jmin][i]);
			xi[i] = 0.5*(bil_prod_i[half_lwind-jmax][i] +
					bil_prod_i[half_lwind-jmin][i]);
	    }
	}
	if (Period) {
	    for (i = 0; i <= half_lwind; i++) {
		for (j = -(half_lwind - i); j <= half_lwind - i; j++) {
		    xr[i] = xr[i] + bil_prod_r[j + half_lwind][i] /
			(double) lwind;
		    xi[i] = xi[i] + bil_prod_i[j + half_lwind][i] /
			(double) lwind;
		}
	    }
	}
	if (Corr) {
	    for (i = 0; i <= half_lwind; i++) {
		for (num_added = 0, j = -(half_lwind - i); j <= half_lwind - i; j++) {
		    xr[i] = xr[i] + bil_prod_r[j + half_lwind][i];
		    xi[i] = xi[i] + bil_prod_i[j + half_lwind][i];
		    num_added++;
		}
		xr[i] /= (double) num_added;
		xi[i] /= (double) num_added;
	    }
	}
	if (BJC) {
	    for (i = 0; i <= half_lwind; i++) {
		for (num_added = 0, j = -i; j <= i; j++) {
		    xr[i] = xr[i] + bil_prod_r[j + half_lwind][i];
		    xi[i] = xi[i] + bil_prod_i[j + half_lwind][i];
		    num_added++;
		}
		xr[i] /= (double) num_added;
		xi[i] /= (double) num_added;
	    }
	}
	if (Choi) {
	    for (i = 0; i <= half_lwind; i++) {
		for (j = -half_lwind; j <= half_lwind; j++) {
		    if (i == 0) {
			if (j == 0) {
			    wt = 1.0;
			}
			else {
			    wt = 0.0;
			}
		    }
		    else {
			wt = (1.0 / sqrt (4.0 * PI * (double) i * (double) i / sigma)) *
			    exp (-sigma * (double) j * (double) j /
				 (4.0 * (double) i * (double) i));
		    }
		    xr[i] = xr[i] + wt * bil_prod_r[j + half_lwind][i];
		    xi[i] = xi[i] + wt * bil_prod_i[j + half_lwind][i];
		}
	    }
	}
	if (Riha) {
	    for (i = 0; i <= half_lwind; i++) {
		xr[i] = 0.5 * (bil_prod_r[half_lwind + i][i]
			       + bil_prod_r[half_lwind - i][i]);
		xi[i] = 0.5 * (bil_prod_i[half_lwind + i][i]
			       + bil_prod_i[half_lwind - i][i]);
	    }
	}
	if (Page) {
	    for (i = 0; i <= half_lwind; i++) {
		xr[i] = bil_prod_r[i + half_lwind][i];
		xi[i] = bil_prod_i[i + half_lwind][i];
	    }
	}
	if (MIFFT) {
	    for (i = 2; i <= half_lwind; i++) {
		for (num_added = 0, j = -(half_lwind - i); j <= half_lwind - i; j++) {
		    xr[i] = xr[i] + bil_prod_r[j + half_lwind][i];
		    xi[i] = xi[i] + bil_prod_i[j + half_lwind][i];
		    num_added++;
		}
		xr[i] /= (double) num_added;
		xi[i] /= (double) num_added;
	    }
	    xr[0] = bil_prod_r[half_lwind][0];
	    xi[0] = bil_prod_i[half_lwind][0];
	    xr[1] = bil_prod_r[half_lwind][1];
	    xi[1] = bil_prod_i[half_lwind][1];
	}
	if (PWVD) {
	    for (i = 0; i <= half_lwind; i++) {
		for (num_added=0, j = -(int) (sigma * (half_lwind - i));
		     j <= (int) (sigma * (half_lwind - i)); j++) {
		    xr[i] = xr[i] + bil_prod_r[j + half_lwind][i];
		    xi[i] = xi[i] + bil_prod_i[j + half_lwind][i];
		    num_added++;
		}
		xr[i] /= (double) num_added;
		xi[i] /= (double) num_added;
	    }
	}

/* Fill in negative lags */

	for (i = 1; i <= half_lwind; i++) {
	    xr[nplot - i] = xr[i];
	    xi[nplot - i] = -xi[i];
	}

/* Do windowing and take Fast Fourier Transform */

	for (i = 0; i < nplot; i++) {
	    xr[i] = xr[i] * window[i];
	    xi[i] = xi[i] * window[i];
	}
	if (CROSS)
	    for (i = 0; i < nplot; i++) {
		xi[i] = 0.0;
		xr[i] = 2 * xr[i];
	    }
	fft (xr, xi, nplot, FORWARD);

/* Write out that TFD plot */

	for (i = 0; i < nplot; i++)
	    printf ("%lg\n", xr[i]);

/*       update bilinear product */

	form_product (sig_real,
		      sig_imag,
		      sig_real2,
		      sig_imag2,
		      (current_time + timeres),
		      timeres,
		      half_lwind,
		      N,
		      bil_prod_r,
		      bil_prod_i);

    }				/* END OF MAIN LOOP */
    if (!QUIET) {
	fprintf (stderr,
		 "\n____________________________________________________\n");

/* Now tidy up Percentage Done trace */
	fprintf (stderr, "\n\n");
    }
/* TRACES
    fprintf(stderr,"\nTFD FLAGS\n");
    fprintf(stderr,"Ville  = %d  ",Ville);
    fprintf(stderr,"Wigner = %d\n",Wigner);
    fprintf(stderr,"Period = %d  ",Period);
    fprintf(stderr,"Corr   = %d\n",Corr);
    fprintf(stderr,"Riha   = %d  ",Riha);
    fprintf(stderr,"BJC    = %d\n",BJC);
    fprintf(stderr,"Choi   = %d  ",Choi);
    fprintf(stderr,"Page   = %d\n",Page);

    fprintf(stderr,"\nWINDOW FLAGS\n");
    fprintf(stderr,"Hann = %d  ",hann);
    fprintf(stderr,"Hamm = %d\n",hamm);
    fprintf(stderr,"black= %d  ",black);
    fprintf(stderr,"blhar= %d\n",blhar);
    fprintf(stderr,"rect = %d\n",rect);

    fprintf(stderr,"\nTFD PARAMETERS\n");
    fprintf(stderr,"Time resolution = %5d  ",timeres);
    fprintf(stderr,"Window Length   = %5d\n",lwind);
    fprintf(stderr,"Nplot           = %5d  ",nplot);
    fprintf(stderr,"N               = %5d\n",N);
    fprintf(stderr,"Sampling Rate   = %lg\n",fsam);
END OF TRACES */

}				/* End of main() */


/************************************************************
* Analytic: This procedure converts real data
* into analytic data using the discrete definition of
* the Hilbert transform.
************************************************************/

void
analytic (xr, xi, nsam)
    double  xr[],
            xi[];		/* real and imag. arrays */
int     nsam;			/* length of arrays */

{
    int     i,
            j;			/* indices */

    for (i = 0; i < nsam; i++) {
	xi[i] = 0.0;
	for (j = 0; j < nsam; j++) {
	    /* j-i odd and less than length */
	    if (((j - i) & 1) && (abs (j - i) < MAXHILBLENGTH))
		xi[i] = xi[i] + 2.0 * xr[j] / PI / (double) (i - j);
	}
    }
}				/* END OF FUNCITON analytic */

/************************************************************
* FFT routine based on the FORTRAN routine presented
* in Rabiner and Gold:"Theory and Application of DSP"
* section 6.5. This routine is different in that either a
* forward or reverse transform may be performed depending
* on the value of sw.
* FORWARD: sw = -1.0;
* REVERSE: sw = +1.0;
* This is simply the sign of the exponent of the
* primitive root of unity in the definition of the DFT.
* Other input variables:
*
* ar: pointer to array of doubles
* ai: pointer to array of doubles
* n: integer (must be a power of 2) size of transform
*         to be performed.
*
* This routine was originally "lifted" from Rabiner & Gold
* by Peter Garrone, and was modified by Robert Williamson.
***********************************************************/

int
fft (ar, ai, n, sw)
    register double *ar,
           *ai;
    int     n;
    double  sw;
{
    register int i,
            ip,
            j,
            le1;
    int     m,
            nv2,
            nm1,
            k,
            l,
            le;
    double  ur,
            ui,
            tr,
            ti,
            wr,
            wi;

    m = 0;
/*
Calculate Log2(n)
*/
    for (nv2 = n; nv2 > 1; nv2 /= 2)
	m++;
/*
Bit reverse ordering of input data
*/
    nv2 = n / 2;
    nm1 = n - 1;
    j = 0;
    for (i = 0; i < nm1; i++) {
	if (i < j) {
	    tr = ar[j];
	    ti = ai[j];
	    ar[j] = ar[i];
	    ai[j] = ai[i];
	    ar[i] = tr;
	    ai[i] = ti;
	}
	k = nv2;
	while (k <= j) {
	    j -= k;
	    k /= 2;
	}
	j += k;
    }
    for (l = 1; l <= m; l++) {
	le = 1 << l;
	le1 = le >> 1;
	ur = 1.;
	ui = 0.0;
	wr = cos ((PI / le1));
	wi = sw * sin ((PI / le1));
	for (j = 0; j < le1; j++) {
	    for (i = j; i < n; i += le) {
/*
Butterfly calculation; note le1 is the "butterfly wingspan"
*/
		ip = i + le1;
		tr = ar[ip] * ur - ai[ip] * ui;
		ti = ar[ip] * ui + ur * ai[ip];
		ar[ip] = ar[i] - tr;
		ai[ip] = ai[i] - ti;
		ar[i] += tr;
		ai[i] += ti;
/*
End of Butterfly calculation
*/
	    }
	    tr = ur * wr - ui * wi;
	    ui = ur * wi + ui * wr;
	    ur = tr;
	}
    }
/*
If we are doing a reverse transform we multiply by 1/n
for normalisation of the results
*/
    if (sw == 1.) {
	for (i = 0; i < n;) {
	    ar[i] /= n;
	    ai[i++] /= n;
	}
    }
    return (n);
}				/* END OF FUNCTION fft */

void    Zero_TFD_Flags ()
/* Initialize TFD type flags */
{
    extern int
            Wigner,
            Ville,
            Period,
            Corr,
            BJC,
            Choi,
            Riha,
            Page,
            PWVD,
            MIFFT;

    Wigner = Ville = Period = Corr = BJC =
	Choi = Riha = Page = PWVD = MIFFT = 0;
}				/* END OF FUNCTION Zero_TFD_Flags */

void    Zero_Window_Flags ()
/* Initialize window type flags */
{
    extern int
            hann,
            hamm,
            rect,
            black,
            blhar;

    hann = hamm = rect = black = blhar = 0;
}				/* END OF FUNCTION Zero_Window_Flags */

int     Rad2 (N)
/* Return the Radix 2 value greater than or equal to N */
    int     N;

{
    int     dummy;

    dummy = 1;
    while (dummy < N)
	dummy *= 2;
    return dummy;
}				/* END OF FUNCTION Rad2 */

/*******************************************************
* function get_signal reads signal data into sig_re from
* the file pointed to by filepointer. If the file is a
* type 2 TFD file then the imaginary part is set too.
* If the signal is type 1, its hilbert transform is
* returned in the imaginary part (sig_im).
********************************************************/


void    get_signal (filepointer, sig_re, sig_im, fsam, sig_length)
    FILE   *filepointer;
    double  sig_re[],
            sig_im[],
           *fsam;
    int    *sig_length;

{
    register int i;		/* counter variable */
    int     sigtype;		/* data file type */
    double  dummy1,
            dummy2;		/* dummy temporary variables */


    fscanf (filepointer, "%d\n", &sigtype);
    if (sigtype == 1) {		/* Type one TFD file */
	fscanf (filepointer, "%d\n%lg\n", sig_length, fsam);
	for (i = 0; i < *sig_length; i++) {
	    fscanf (filepointer, "%lg\n", &sig_re[i]);
	}
	analytic (sig_re, sig_im, *sig_length);
    }
    else {
	if (sigtype == 2) {	/* Type 2 TFD file */
	    fprintf(stderr,"Complex signal.\n");	   
	    fscanf (filepointer, "%d\n%lg\n", sig_length, fsam);
	    for (i = 0; i < *sig_length; i++) {
		fscanf (filepointer, "(%lg,%lg)\n", &sig_re[i], &sig_im[i]);
		printf("%lg\n",sig_re[i]);	    
	    }
	}
	else {
	    fprintf (stderr, "ccg : incorrect input format.\n");
	    exit (7);
	}
	fclose (filepointer);
    }
}				/* END OF FUNCTION get_signal */

/*******************************************************
* function form_product take the signal as input
* and gives as output the bilinear product array.
*
* The maximum array size of the output array is MAXLWIND
* by MAXLWIND/2+1.
********************************************************/

void    form_product (s_r, s_i, s_r2, s_i2, time, timeres, hlflw, N, b_r, b_i)
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                         INPUTS
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    double  s_r[],		/* real part of signal                  */
            s_i[],		/* imaginary part of signal             */
            s_r2[],		/* real part of second signal           */
            s_i2[];		/* imaginary part of second signal      */
int     time,			/* current position in signal           */
        timeres,		/* time increment                       */
        hlflw,			/* half the window length (maximum lag) */
        N;			/* Total signal length                  */

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                         OUTPUTS
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
double  b_r[MAXLWIND][MAXLWINDV2],
 /* real part of bilinear product        */
        b_i[MAXLWIND][MAXLWINDV2];

 /* imaginary part of bilinear product   */
{
    int     i,
            j,			/* counter variables                    */
            t1,
            t2,			/* index varibles                       */
            upperlimit;		/* lets intialization take place        */

/* initialize */
    if (time == 0) {
	for (i = 0; i < 2 * hlflw + 1; i++)
	    for (j = 0; j < 2 * hlflw + 1; j++)
		b_i[i][j] = b_r[i][j] = 0.0;
	upperlimit = 2 * hlflw;
    }
    else {
	/* move array */
	upperlimit = timeres;
	for (i = timeres; i < 2 * hlflw + 1; i++) {
	    for (j = 0; j <= hlflw; j++) {	/* Lag variable  */
		b_r[i - timeres][j] = b_r[i][j];
		b_i[i - timeres][j] = b_i[i][j];
		b_r[i][j] = 0.0;
		b_i[i][j] = 0.0;
	    }
	}
    }
/* main assignment loop */
    for (i = hlflw - upperlimit; i <= hlflw; i++) {	/* Time variable */
	for (j = 0; j <= hlflw; j++) {	/* Lag variable  */
	    t1 = time + i + j;
	    t2 = time + i - j;
	    if (t1 >= 0 && t1 <= N - 1 && t2 >= 0 && t2 <= N - 1) {
		/* not outside signal range */
		b_r[i + hlflw][j] = s_r[t1] * s_r2[t2] + s_i[t1] * s_i2[t2];
		b_i[i + hlflw][j] = s_i[t1] * s_r2[t2] - s_r[t1] * s_i2[t2];
	    }
	    else {
		/* outside signal range */
		b_r[i + hlflw][j] = 0.0;
		b_i[i + hlflw][j] = 0.0;
	    }
	}
    }
}				/* END OF FUNCTION form_product */


void PrintUsage(void)
{
  	fprintf (stderr, "Usage : \n");
	fprintf (stderr,
	     "ccg -T[WVpcBCRP] [-w[hHbBr]] [-l#] [-i#] [-N#] [-s #] file\n"
	    );
	fprintf (stderr, "Options :\n");
	fprintf (stderr, " TW : Wigner\n");
	fprintf (stderr, "  V : Wigner-Ville\n");
	fprintf (stderr, "  p : Periodogram\n");
	fprintf (stderr, "  c : Correlogram\n");
	fprintf (stderr, "  B : Born-Jordan-Cohen\n");
	fprintf (stderr, "  C : Choi-Williams\n");
	fprintf (stderr, "  R : Rihaczek\n");
	fprintf (stderr, "  P : Page\n");
	fprintf (stderr, "  I : MIFFT\n");
	fprintf (stderr, "  S : SWVD\n");
	fprintf (stderr, "  s : Parameterized WVD\n");
	fprintf (stderr, "  M : Median Filtered WVD\n");
	fprintf (stderr, " wh : hanning window\n");
	fprintf (stderr, "  H : Hamming window\n");
	fprintf (stderr, "  b : Blackman window\n");
	fprintf (stderr, "  r : Rectangular window\n");
	fprintf (stderr, "  B : Blackman-Harris window\n");
	fprintf (stderr, " l# : Window '#' long\n");
	fprintf (stderr, " i# : Time increment = '#' \n");
	fprintf (stderr, " N# : TFD plot '#' long\n");
	fprintf (stderr, " s# : Choi-Williams parameter '#'\n");
	fprintf (stderr, " X# : Cross terms, difference frequency '#'\n");
	fprintf (stderr, " Q  : Quiet - no percentage done\n");
	exit (4);

}
/* THIS IS THE END OF THE FILE */
