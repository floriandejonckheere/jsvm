
@bin\H264AVCEncoderLibTestStatic -pf cfg\Bus.cfg -numl 1 -mfile 0 2 mot\Bus_layer0.mot -anafgs 0 1     tmp\Bus_FGS0.dat > log\BUS_enc_1.txt
@bin\H264AVCEncoderLibTestStatic -pf cfg\Bus.cfg -numl 2 -mfile 1 2 mot\Bus_layer1.mot -encfgs 0 192.0 tmp\Bus_FGS0.dat -anafgs 1 1     tmp\Bus_FGS1.dat > log\BUS_enc_2.txt
@bin\H264AVCEncoderLibTestStatic -pf cfg\Bus.cfg -numl 2                               -encfgs 0 192.0 tmp\Bus_FGS0.dat -encfgs 1 768.0 tmp\Bus_FGS1.dat > log\BUS_enc_3.txt

@CALL extractBus.bat > log\BUS_ext.txt
@CALL decodeBus.bat > log\BUS_dec.txt
