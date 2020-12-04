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
