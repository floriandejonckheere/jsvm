
@bin\H264AVCEncoderLibTestStatic -pf cfg\City.cfg -numl 1 -mfile 0 1 mot\City_layer0.mot -anafgs 0 3     tmp\City_FGS0.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\City.cfg -numl 1 -encfgs 0 128.0 tmp\City_FGS0.dat -bf str\City0

@bin\H264AVCEncoderLibTestStatic -pf cfg\City.cfg -numl 2 -mfile 1 1 mot\City_layer1.mot -encfgs 0 112.0 tmp\City_FGS0.dat -anafgs 1 3     tmp\City_FGS1.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\City.cfg -numl 2 -encfgs 0 112.0 tmp\City_FGS0.dat -encfgs 1 512.0 tmp\City_FGS1.dat -bf str\City1

@bin\H264AVCEncoderLibTestStatic -pf cfg\City.cfg -numl 3 -mfile 2 1 mot\City_layer2.mot -encfgs 0 112.0 tmp\City_FGS0.dat -encfgs 1 448.0 tmp\City_FGS1.dat -anafgs 2 3 tmp\City_FGS2.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\City.cfg -numl 3 -encfgs 0 112.0 tmp\City_FGS0.dat -encfgs 1 448.0 tmp\City_FGS1.dat -encfgs 2 2048.0 tmp\City_FGS2.dat -bf str\City2

@bin\EncoderBitstreamMergerStatic.exe -numl 3 -ds -infile 0 str\City0 -infile 1 str\City1 -infile 2 str\City2 -outfile str\City_4CIF60-2048.264

@CALL extractCity_DS.bat
@CALL decodeCity.bat
