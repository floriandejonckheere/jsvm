Please put conformance data (bitsreams, YUV references sequences) in DATA folder or anywhere else
(available at http://ftp3.itu.ch/av-arch/jvt-site/draft_conformance/).

Before running the AVC-Conformance tests (the first time), you can (if you want) execute the "dump.pm" perl script.
It should copy (or remove) the sequences and bistreams at the right places.

USAGE: dump.pm
-------------- 
	[-c] to copy "conformance sequences and bistreams" in the corresponding simus directories.
	[-r] to remove "conformance sequences and bistreams of each simus directories.
	[-s <name_simu1>...<name_simuN> ] name of the simulations to copy/remove.
	[-data <yuv_streams_directory>] name of the directory containing the bitstreams and YUV references sequences
	[-u] Usage.
	   

