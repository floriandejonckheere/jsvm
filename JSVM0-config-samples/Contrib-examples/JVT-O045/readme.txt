Modification of Scaling factor and variable length nal unit header:
no modification is made in the batch files used for encoding, extracting and decoding. 

Quality level estimation:
for quality level estimation, you should set the quality level option to 1 in the xxx.cfg file of the sequence (in general parameters).
In the encoder batch file, you must specify an output bitstream file by using the "-bf fileName.264" option of the encode command line.  
The output bitstream will then be saved in the file fileName.264

To insert quality level information in the bitstream, after the encoding you should call the EncoderBitstreamMergerStatic.exe with the following command line:
for example for the sequence Bus:
-numl 2 -ql tmp\Bus -infile 0 str\Bus0 -outfile str\Bus_CIF30-512.264

-numl indicates the number of layer of the input bitstream. -ql indicates the root filename of the rate and distorsion files (always tmp\name of sequence), -infile 0 specifies the name of the input file  (same as the one used with the -bf option of the encoder), -outfile specifies the output bitstream filename.

The encoding and merging stage can be performed by using the EncodeXXX_QL.bat batch file.

Dead substream:
for the encoding, use the batch files provided: EncodeXXX_DS.bat
then call the EncoderBitstreamMergerStatic with the following command line:
for example for the sequence Bus:
-numl 2 -ds -infile 0 str\Bus0 -infile 1 str\Bus1 -outfile str\Bus_CIF30-512.264

The EncoderBitstreamMergerStatic will merge all the layered bitstreams encoded and add DS information such as maxRate in SEI NAL and set the discardable flag for some NAL.

-numl indicates the number of layer for the output bitstream, -ds indicates that we want to perform merging of layered bitstream, -infile specifies for the corresponding layer the name of the input bitstream (must correspond to the name used at the encoding of the layer), -infile must be present for all the layers we want to merge, if the sequence has 2 layers, we must have 2 -infile options. -outfile specifies the name of the output bitstream. 

For the extraction, the dead substreams from the input bitstream are kept by default. To remove the dead substream, use the -ds layer option in the extractor command line. -ds specifies the layer of the dead substream you want to remove. 
For example to remove the dead substream of the layer 0, use -ds 0 in the extractor command line. 
The batch file extractXXX_DS.bat performs an inter-layer extraction path, same as the Palma extraction path. (dead substreams are kept only at the high rate point of a spatio-temporal resolution). 


