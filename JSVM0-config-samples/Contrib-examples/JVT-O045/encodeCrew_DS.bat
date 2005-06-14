
@bin\H264AVCEncoderLibTestStatic -pf cfg\Crew.cfg -numl 1 -mfile 0 1 mot\Crew_layer0.mot -anafgs 0 3     tmp\Crew_FGS0.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Crew.cfg -numl 1 -encfgs 0 192.0 tmp\Crew_FGS0.dat -bf str\Crew0

@bin\H264AVCEncoderLibTestStatic -pf cfg\Crew.cfg -numl 2 -mfile 1 1 mot\Crew_layer1.mot -encfgs 0 160.0 tmp\Crew_FGS0.dat -anafgs 1 3     tmp\Crew_FGS1.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Crew.cfg -numl 2 -encfgs 0 160.0 tmp\Crew_FGS0.dat -encfgs 1 768.0 tmp\Crew_FGS1.dat -bf str\Crew1

@bin\H264AVCEncoderLibTestStatic -pf cfg\Crew.cfg -numl 3 -mfile 2 1 mot\Crew_layer2.mot -encfgs 0 160.0 tmp\Crew_FGS0.dat -encfgs 1 640.0 tmp\Crew_FGS1.dat -anafgs 2 3 tmp\Crew_FGS2.dat
@bin\H264AVCEncoderLibTestStatic -pf cfg\Crew.cfg -numl 3 -encfgs 0 160.0 tmp\Crew_FGS0.dat -encfgs 1 640.0 tmp\Crew_FGS1.dat -encfgs 2 3072.0 tmp\Crew_FGS2.dat -bf str\Crew2

@bin\EncoderBitstreamMergerStatic.exe -numl 3 -ds -infile 0 str\Crew0 -infile 1 str\Crew1 -infile 2 str\Crew2 -outfile str\Crew_4CIF60-3072.264

@CALL extractCrew_DS.bat
@CALL decodeCrew.bat
