cd str\
copy Mobile_CIF30-384.264 Mobile_CIF30-384_o.264
cd ..\

.\bin\BitStreamExtractorStatic	str/Mobile_CIF30-384_o.264	str/Mobile_CIF30-384_1.264	-e 352x288@30:384
.\bin\BitStreamExtractorStatic	str/Mobile_CIF30-384_1.264	str/Mobile_CIF30-384.264	-e 352x288@30:384 -ds 0

@REM ============MUNICH TEST POINTS ============
.\bin\BitStreamExtractorStatic	str/Mobile_CIF30-384_1.264	str/Mobile_CIF15-256_o.264	-e 352x288@15:256 
.\bin\BitStreamExtractorStatic	str/Mobile_CIF15-256_o.264	str/Mobile_CIF15-256.264	-e 352x288@15:256 -ds 0  
.\bin\BitStreamExtractorStatic	str/Mobile_CIF15-256.264	str/Mobile_CIF15-128.264	-e 352x288@15:128 
.\bin\BitStreamExtractorStatic	str/Mobile_CIF15-128.264	str/Mobile_QCIF15-64.264	-e 176x144@15:64 
.\bin\BitStreamExtractorStatic	str/Mobile_QCIF15-64.264	str/Mobile_QCIF7.5-48.264	-e 176x144@7.5:48 

@REM =========== OTHER TEST POINTS ===========
.\bin\BitStreamExtractorStatic	str/Mobile_CIF30-384.264	str/Mobile_CIF30-320.264	-e 352x288@30:320 
.\bin\BitStreamExtractorStatic	str/Mobile_CIF30-384.264	str/Mobile_CIF30-256.264	-e 352x288@30:256 
.\bin\BitStreamExtractorStatic	str/Mobile_CIF30-384.264	str/Mobile_CIF30-224.264	-e 352x288@30:224 
.\bin\BitStreamExtractorStatic	str/Mobile_CIF30-384.264	str/Mobile_CIF30-192.264	-e 352x288@30:192 

.\bin\BitStreamExtractorStatic	str/Mobile_CIF15-256.264	str/Mobile_CIF15-224.264	-e 352x288@15:224 
.\bin\BitStreamExtractorStatic	str/Mobile_CIF15-256.264	str/Mobile_CIF15-192.264	-e 352x288@15:192 
.\bin\BitStreamExtractorStatic	str/Mobile_CIF15-256.264	str/Mobile_CIF15-160.264	-e 352x288@15:160

.\bin\BitStreamExtractorStatic	str/Mobile_CIF15-256.264	str/Mobile_CIF7.5-192.264	-e 352x288@7.5:192 
.\bin\BitStreamExtractorStatic	str/Mobile_CIF7.5-192.264	str/Mobile_CIF7.5-160.264	-e 352x288@7.5:160 
.\bin\BitStreamExtractorStatic	str/Mobile_CIF7.5-192.264	str/Mobile_CIF7.5-128.264	-e 352x288@7.5:128 
.\bin\BitStreamExtractorStatic	str/Mobile_CIF7.5-192.264	str/Mobile_CIF7.5-112.264	-e 352x288@7.5:112 
.\bin\BitStreamExtractorStatic	str/Mobile_CIF7.5-192.264	str/Mobile_CIF7.5-96.264	-e 352x288@7.5:96 

.\bin\BitStreamExtractorStatic	str/Mobile_CIF15-256_o.264	str/Mobile_QCIF15-128.264	-e 176x144@15:128 
.\bin\BitStreamExtractorStatic	str/Mobile_QCIF15-128.264	str/Mobile_QCIF15-112.264	-e 176x144@15:112 
.\bin\BitStreamExtractorStatic	str/Mobile_QCIF15-128.264	str/Mobile_QCIF15-96.264	-e 176x144@15:96 
.\bin\BitStreamExtractorStatic	str/Mobile_QCIF15-128.264	str/Mobile_QCIF15-80.264	-e 176x144@15:80 

.\bin\BitStreamExtractorStatic	str/Mobile_QCIF15-128.264	str/Mobile_QCIF7.5-96.264	-e 176x144@7.5:96 
.\bin\BitStreamExtractorStatic	str/Mobile_QCIF7.5-96.264	str/Mobile_QCIF7.5-80.264	-e 176x144@7.5:80 
.\bin\BitStreamExtractorStatic	str/Mobile_QCIF7.5-96.264	str/Mobile_QCIF7.5-64.264	-e 176x144@7.5:64 
.\bin\BitStreamExtractorStatic	str/Mobile_QCIF7.5-96.264	str/Mobile_QCIF7.5-56.264	-e 176x144@7.5:56 
