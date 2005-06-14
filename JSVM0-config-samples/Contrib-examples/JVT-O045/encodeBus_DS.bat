
@bin\H264AVCEncoderLibTestStatic -pf cfg\Bus.cfg -numl 1 -mfile 0 1 mot\Bus_layer0.mot -anafgs 0 3     tmp\Bus_FGS0.dat 
@bin\H264AVCEncoderLibTestStatic -pf cfg\Bus.cfg -numl 1 -encfgs 0 192.0 tmp\Bus_FGS0.dat -bf str\Bus0
@bin\H264AVCEncoderLibTestStatic -pf cfg\Bus.cfg -numl 2 -mfile 1 1 mot\Bus_layer1.mot -encfgs 0 128.0 tmp\Bus_FGS0.dat -anafgs 1 3     tmp\Bus_FGS1.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Bus.cfg -numl 2 -encfgs 0 128.0 tmp\Bus_FGS0.dat -encfgs 1 512.0 tmp\Bus_FGS1.dat -bf str\Bus1

@bin\EncoderBitstreamMergerStatic.exe -numl 2 -ds -infile 0 str\Bus0 -infile 1 str\Bus1 -outfile str\Bus_CIF30-512.264

@call extractBus_DS.bat
@call decodeBus.bat