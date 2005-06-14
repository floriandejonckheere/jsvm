
@bin\H264AVCEncoderLibTestStatic -pf cfg\Mobile.cfg -numl 1 -mfile 0 2 mot\MOBILE_layer0.mot -anafgs 0 3    tmp\MOBILE_FGS0.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Mobile.cfg -numl 2 -mfile 1 2 mot\MOBILE_layer1.mot -encfgs 0 96.0 tmp\MOBILE_FGS0.dat -anafgs 1 3     tmp\MOBILE_FGS1.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Mobile.cfg -numl 2                                  -encfgs 0 96.0 tmp\MOBILE_FGS0.dat -encfgs 1 384.0 tmp\MOBILE_FGS1.dat -bf str\Mobile01

@bin\EncoderBitstreamMergerStatic.exe -numl 2 -ql tmp\Mobile -infile 0 str\Mobile01 -outfile str\Mobile_CIF30-384.264

@CALL extractMobile.bat
@CALL decodeMobile.bat
