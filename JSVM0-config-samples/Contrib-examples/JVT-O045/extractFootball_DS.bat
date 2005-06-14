cd str\
copy Football_CIF30-1024.264 Football_CIF30-1024_o.264
cd ..\

.\bin\BitStreamExtractorStatic	str/Football_CIF30-1024_o.264	str/Football_CIF30-1024_1.264	-e 352x288@30:1024
.\bin\BitStreamExtractorStatic	str/Football_CIF30-1024_1.264	str/Football_CIF30-1024.264	-e 352x288@30:1024 -ds 0

@REM ============MUNICH TEST POINTS ============
.\bin\BitStreamExtractorStatic	str/Football_CIF30-1024_1.264	str/Football_CIF15-512_o.264	-e 352x288@15:512 
.\bin\BitStreamExtractorStatic	str/Football_CIF15-512_o.264	str/Football_CIF15-512.264	-e 352x288@15:512 -ds 0
.\bin\BitStreamExtractorStatic	str/Football_CIF15-512.264	str/Football_CIF15-384.264	-e 352x288@15:384 
.\bin\BitStreamExtractorStatic	str/Football_CIF15-384.264	str/Football_QCIF15-192.264	-e 176x144@15:192 
.\bin\BitStreamExtractorStatic	str/Football_QCIF15-192.264	str/Football_QCIF7.5-128.264	-e 176x144@7.5:128 

@REM =========== OTHER TEST POINTS ===========
.\bin\BitStreamExtractorStatic	str/Football_CIF30-1024.264	str/Football_CIF30-896.264	-e 352x288@30:896 
.\bin\BitStreamExtractorStatic	str/Football_CIF30-1024.264	str/Football_CIF30-768.264	-e 352x288@30:768 
.\bin\BitStreamExtractorStatic	str/Football_CIF30-1024.264	str/Football_CIF30-640.264	-e 352x288@30:640 
.\bin\BitStreamExtractorStatic	str/Football_CIF30-1024.264	str/Football_CIF30-512.264	-e 352x288@30:512 

.\bin\BitStreamExtractorStatic	str/Football_CIF30-1024_o.264	str/Football_CIF15-768_o.264	-e 352x288@15:768
.\bin\BitStreamExtractorStatic	str/Football_CIF15-768_o.264	str/Football_CIF15-768.264	-e 352x288@15:768 -ds 0

.\bin\BitStreamExtractorStatic	str/Football_CIF15-768.264	str/Football_CIF15-640.264	-e 352x288@15:640 
.\bin\BitStreamExtractorStatic	str/Football_CIF15-768.264	str/Football_CIF15-448.264	-e 352x288@15:448 

.\bin\BitStreamExtractorStatic	str/Football_CIF15-768.264	str/Football_CIF7.5-512.264	-e 352x288@7.5:512 
.\bin\BitStreamExtractorStatic	str/Football_CIF7.5-512.264	str/Football_CIF7.5-448.264	-e 352x288@7.5:448 
.\bin\BitStreamExtractorStatic	str/Football_CIF7.5-512.264	str/Football_CIF7.5-384.264	-e 352x288@7.5:384 
.\bin\BitStreamExtractorStatic	str/Football_CIF7.5-512.264	str/Football_CIF7.5-320.264	-e 352x288@7.5:320 
.\bin\BitStreamExtractorStatic	str/Football_CIF7.5-512.264	str/Football_CIF7.5-256.264	-e 352x288@7.5:256 

.\bin\BitStreamExtractorStatic	str/Football_CIF15-768_o.264	str/Football_QCIF15-384.264	-e 176x144@15:384 
.\bin\BitStreamExtractorStatic	str/Football_QCIF15-384.264	str/Football_QCIF15-320.264	-e 176x144@15:320 
.\bin\BitStreamExtractorStatic	str/Football_QCIF15-384.264	str/Football_QCIF15-256.264	-e 176x144@15:256 
.\bin\BitStreamExtractorStatic	str/Football_QCIF15-384.264	str/Football_QCIF15-224.264	-e 176x144@15:224 

.\bin\BitStreamExtractorStatic	str/Football_QCIF15-384.264	str/Football_QCIF7.5-256.264	-e 176x144@7.5:256 
.\bin\BitStreamExtractorStatic	str/Football_QCIF7.5-256.264	str/Football_QCIF7.5-224.264	-e 176x144@7.5:224 
.\bin\BitStreamExtractorStatic	str/Football_QCIF7.5-256.264	str/Football_QCIF7.5-192.264	-e 176x144@7.5:192 
.\bin\BitStreamExtractorStatic	str/Football_QCIF7.5-256.264	str/Football_QCIF7.5-160.264	-e 176x144@7.5:160 
