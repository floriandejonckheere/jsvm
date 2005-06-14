
@bin\H264AVCEncoderLibTestStatic -pf cfg\Football.cfg -numl 1 -mfile 0 1 mot\Football_layer0.mot -anafgs 0 3     tmp\Football_FGS0.dat 
@bin\H264AVCEncoderLibTestStatic -pf cfg\Football.cfg -numl 1 -encfgs 0 384.0 tmp\Football_FGS0.dat -bf str\Football0
@bin\H264AVCEncoderLibTestStatic -pf cfg\Football.cfg -numl 2 -mfile 1 1 mot\Football_layer1.mot -encfgs 0 256.0 tmp\Football_FGS0.dat -anafgs 1 3     tmp\Football_FGS1.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Football.cfg -numl 2 -encfgs 0 256.0 tmp\Football_FGS0.dat -encfgs 1 1024.0 tmp\Football_FGS1.dat -bf str\Football1

@bin\EncoderBitstreamMergerStatic.exe -numl 2 -ds -infile 0 str\Football0 -infile 1 str\Football1 -outfile str\Football_CIF30-1024.264

@call extractFootball_DS.bat
@call decodeFootball.bat