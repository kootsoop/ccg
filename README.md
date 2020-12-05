# ccg
## Cohen's Class Generator

This repository contains code that generates time-frequency representations of [Cohen's Class.](https://en.wikipedia.org/wiki/Bilinear_time%E2%80%93frequency_distribution)

If the `ccg.c` file is compiled to a `ccg` executable, then the usage is:

```
Usage : 
ccg -T[WVpcBCRP] [-w[hHbBr]] [-l#] [-i#] [-N#] [-s #] file
Options :
 TW : Wigner
  V : Wigner-Ville
  p : Periodogram
  c : Correlogram
  B : Born-Jordan-Cohen
  C : Choi-Williams
  R : Rihaczek
  P : Page
  I : MIFFT
  S : SWVD
  s : Parameterized WVD
  M : Median Filtered WVD
 wh : hanning window
  H : Hamming window
  b : Blackman window
  r : Rectangular window
  B : Blackman-Harris window
 l# : Window '#' long
 i# : Time increment = '#' 
 N# : TFD plot '#' long
 s# : Choi-Williams parameter '#'
 X# : Cross terms, difference frequency '#'
 Q  : Quiet - no percentage done
```

For example, to generate the Wigner distribution of the file `signal`:

```
./ccg -TW signal > signal.wd
```

The file `signal` needs three lines at the beginning:
```
1
N
fs
```
followed by `N` signal samples.  Here, `1` indicates the file type, `N` is the number of samples following the header (one per line), and `fs` is the sampling frequency.

![Example output](./ccg/example_plot_three_wvds.png?raw=true "Example output")
