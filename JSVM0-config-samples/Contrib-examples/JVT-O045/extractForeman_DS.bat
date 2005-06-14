cd str\
copy Foreman_CIF30-256.264 Foreman_CIF30-256_o.264
cd ..\

.\bin\BitStreamExtractorStatic	str/Foreman_CIF30-256_o.264	str/Foreman_CIF30-256_1.264	-e 352x288@30:256
.\bin\BitStreamExtractorStatic	str/Foreman_CIF30-256_1.264	str/Foreman_CIF30-256.264	-e 352x288@30:256 -ds 0

@REM ============MUNICH TEST POINTS ============
.\bin\BitStreamExtractorStatic	str/Foreman_CIF30-256_1.264	str/Foreman_CIF15-192_o.264	-e 352x288@15:192 
.\bin\BitStreamExtractorStatic	str/Foreman_CIF15-192_o.264	str/Foreman_CIF15-192.264	-e 352x288@15:192 -ds 0  
.\bin\BitStreamExtractorStatic	str/Foreman_CIF15-192.264	str/Foreman_CIF15-96.264	-e 352x288@15:96 
.\bin\BitStreamExtractorStatic	str/Foreman_CIF15-96.264	str/Foreman_QCIF15-48.264	-e 176x144@15:48 
.\bin\BitStreamExtractorStatic	str/Foreman_QCIF15-48.264	str/Foreman_QCIF7.5-32.264	-e 176x144@7.5:32 

@REM =========== OTHER TEST POINTS ===========
.\bin\BitStreamExtractorStatic	str/Foreman_CIF30-256.264	str/Foreman_CIF30-224.264	-e 352x288@30:224 
.\bin\BitStreamExtractorStatic	str/Foreman_CIF30-256.264	str/Foreman_CIF30-192.264	-e 352x288@30:192 
.\bin\BitStreamExtractorStatic	str/Foreman_CIF30-256.264	str/Foreman_CIF30-160.264	-e 352x288@30:160 
.\bin\BitStreamExtractorStatic	str/Foreman_CIF30-256.264	str/Foreman_CIF30-128.264	-e 352x288@30:128 

.\bin\BitStreamExtractorStatic	str/Foreman_CIF15-192.264	str/Foreman_CIF15-160.264	-e 352x288@15:160 
.\bin\BitStreamExtractorStatic	str/Foreman_CIF15-192.264	str/Foreman_CIF15-128.264	-e 352x288@15:128 
.\bin\BitStreamExtractorStatic	str/Foreman_CIF15-192.264	str/Foreman_CIF15-112.264	-e 352x288@15:112

.\bin\BitStreamExtractorStatic	str/Foreman_CIF15-192.264	str/Foreman_CIF7.5-128.264	-e 352x288@7.5:128 
.\bin\BitStreamExtractorStatic	str/Foreman_CIF7.5-128.264	str/Foreman_CIF7.5-112.264	-e 352x288@7.5:112 
.\bin\BitStreamExtractorStatic	str/Foreman_CIF7.5-128.264	str/Foreman_CIF7.5-96.264	-e 352x288@7.5:96 
.\bin\BitStreamExtractorStatic	str/Foreman_CIF7.5-128.264	str/Foreman_CIF7.5-80.264	-e 352x288@7.5:80 
.\bin\BitStreamExtractorStatic	str/Foreman_CIF7.5-128.264	str/Foreman_CIF7.5-64.264	-e 352x288@7.5:64 

.\bin\BitStreamExtractorStatic	str/Foreman_CIF15-192_o.264	str/Foreman_QCIF15-96.264	-e 176x144@15:96 
.\bin\BitStreamExtractorStatic	str/Foreman_QCIF15-96.264	str/Foreman_QCIF15-80.264	-e 176x144@15:80 
.\bin\BitStreamExtractorStatic	str/Foreman_QCIF15-96.264	str/Foreman_QCIF15-64.264	-e 176x144@15:64 
.\bin\BitStreamExtractorStatic	str/Foreman_QCIF15-96.264	str/Foreman_QCIF15-56.264	-e 176x144@15:56 

.\bin\BitStreamExtractorStatic	str/Foreman_QCIF15-96.264	str/Foreman_QCIF7.5-64.264	-e 176x144@7.5:64 
.\bin\BitStreamExtractorStatic	str/Foreman_QCIF7.5-64.264	str/Foreman_QCIF7.5-56.264	-e 176x144@7.5:56 
.\bin\BitStreamExtractorStatic	str/Foreman_QCIF7.5-64.264	str/Foreman_QCIF7.5-48.264	-e 176x144@7.5:48 
.\bin\BitStreamExtractorStatic	str/Foreman_QCIF7.5-64.264	str/Foreman_QCIF7.5-40.264	-e 176x144@7.5:40 
