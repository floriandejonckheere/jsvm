
@bin\H264AVCEncoderLibTestStatic -pf cfg\Mobile.cfg -numl 1 -mfile 0 1 mot\Mobile_layer0.mot -anafgs 0 3     tmp\Mobile_FGS0.dat 
@bin\H264AVCEncoderLibTestStatic -pf cfg\Mobile.cfg -numl 1 -encfgs 0 128.0 tmp\Mobile_FGS0.dat -bf str\Mobile0

@bin\H264AVCEncoderLibTestStatic -pf cfg\Mobile.cfg -numl 2 -mfile 1 1 mot\Mobile_layer1.mot -encfgs 0 96.0 tmp\Mobile_FGS0.dat -anafgs 1 3     tmp\Mobile_FGS1.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Mobile.cfg -numl 2 -encfgs 0 96.0 tmp\Mobile_FGS0.dat -encfgs 1 384.0 tmp\Mobile_FGS1.dat -bf str\Mobile1

@bin\EncoderBitstreamMergerStatic.exe -numl 2 -ds -infile 0 str\Mobile0 -infile 1 str\Mobile1 -outfile str\Mobile_CIF30-384.264

@call extractMobile_DS.bat
@call decodeMobile.bat