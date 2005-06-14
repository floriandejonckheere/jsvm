
@bin\H264AVCEncoderLibTestStatic -pf cfg\Foreman.cfg -numl 1 -mfile 0 1 mot\Foreman_layer0.mot -anafgs 0 3     tmp\Foreman_FGS0.dat 
@bin\H264AVCEncoderLibTestStatic -pf cfg\Foreman.cfg -numl 1 -encfgs 0 96.0 tmp\Foreman_FGS0.dat -bf str\Foreman0

@bin\H264AVCEncoderLibTestStatic -pf cfg\Foreman.cfg -numl 2 -mfile 1 1 mot\Foreman_layer1.mot -encfgs 0 80.0 tmp\Foreman_FGS0.dat -anafgs 1 3     tmp\Foreman_FGS1.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Foreman.cfg -numl 2 -encfgs 0 80.0 tmp\Foreman_FGS0.dat -encfgs 1 256.0 tmp\Foreman_FGS1.dat -bf str\Foreman1

@bin\EncoderBitstreamMergerStatic.exe -numl 2 -ds -infile 0 str\Foreman0 -infile 1 str\Foreman1 -outfile str\Foreman_CIF30-256.264

@call extractForeman_DS.bat
@call decodeForeman.bat