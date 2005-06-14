
@bin\H264AVCEncoderLibTestStatic -pf cfg\Harbour.cfg -numl 1 -mfile 0 1 mot\Harbour_layer0.mot -anafgs 0 3     tmp\Harbour_FGS0.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Harbour.cfg -numl 2 -mfile 1 1 mot\Harbour_layer1.mot -encfgs 0 192.0 tmp\Harbour_FGS0.dat -anafgs 1 3     tmp\Harbour_FGS1.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Harbour.cfg -numl 3 -mfile 2 1 mot\Harbour_layer2.mot -encfgs 0 192.0 tmp\Harbour_FGS0.dat -encfgs 1 768.0 tmp\Harbour_FGS1.dat -anafgs 2 3      tmp\Harbour_FGS2.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Harbour.cfg -numl 3                                   -encfgs 0 192.0 tmp\Harbour_FGS0.dat -encfgs 1 768.0 tmp\Harbour_FGS1.dat -encfgs 2 3072.0 tmp\Harbour_FGS2.dat -bf str\Harbour01

@bin\EncoderBitstreamMergerStatic.exe -numl 3 -ql tmp\Harbour -infile 0 str\Harbour01 -outfile str\Harbour_4CIF60-3072.264

@CALL extractHarbour.bat
@CALL decodeHarbour.bat
