cd str\
copy Harbour_4CIF60-3072.264 Harbour_4CIF60-3072_o.264
cd ..\
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF60-3072_o.264	str/Harbour_4CIF60-3072_1.264	-e 704x576@60:3072
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF60-3072_1.264	str/Harbour_4CIF60-3072.264	-e 704x576@60:3072 -ds 0 -ds 1

@REM =================== MUNICH TEST POINTS ============
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF60-3072_1.264	str/Harbour_4CIF30-1536_o.264	-e 704x576@30:1536
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF30-1536_o.264	str/Harbour_4CIF30-1536.264	-e 704x576@30:1536 -ds 01 -ds 1

.\bin\BitStreamExtractorStatic	str/Harbour_4CIF30-1536_o.264	str/Harbour_CIF30-768_o.264	-e 352x288@30:768
.\bin\BitStreamExtractorStatic	str/Harbour_CIF30-768_o.264	str/Harbour_CIF30-768.264	-e 352x288@30:768 -ds 0

.\bin\BitStreamExtractorStatic	str/Harbour_CIF30-768_o.264	str/Harbour_CIF30-384_o.264	-e 352x288@30:384
.\bin\BitStreamExtractorStatic	str/Harbour_CIF30-384_o.264	str/Harbour_CIF30-384.264	-e 352x288@30:384 -ds 0


@REM =========== OTHER TEST POINTS ===========
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF60-3072.264	str/Harbour_4CIF60-2560.264	-e 704x576@60:2560 
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF60-3072.264	str/Harbour_4CIF60-2048.264	-e 704x576@60:2048
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF60-3072.264	str/Harbour_4CIF60-1780.264	-e 704x576@60:1780 
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF60-3072.264	str/Harbour_4CIF60-1536.264	-e 704x576@60:1536 

.\bin\BitStreamExtractorStatic	str/Harbour_4CIF60-3072_o.264	str/Harbour_4CIF30-2048_o.264	-e 704x576@30:2048
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF30-2048_o.264	str/Harbour_4CIF30-2048.264	-e 704x576@30:2048 -ds 0 -ds 1
 
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF30-2048.264	str/Harbour_4CIF30-1792.264	-e 704x576@30:1792
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF30-2048.264	str/Harbour_4CIF30-1280.264	-e 704x576@30:1280
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF30-2048.264	str/Harbour_4CIF30-1024.264	-e 704x576@30:1024

.\bin\BitStreamExtractorStatic	str/Harbour_4CIF30-2048.264	str/Harbour_4CIF15-1536.264	-e 704x576@15:1536
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF15-1536.264	str/Harbour_4CIF15-1280.264	-e 704x576@15:1280
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF15-1536.264	str/Harbour_4CIF15-1024.264	-e 704x576@15:1024
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF15-1536.264	str/Harbour_4CIF15-896.264	-e 704x576@15:896
.\bin\BitStreamExtractorStatic	str/Harbour_4CIF15-1536.264	str/Harbour_4CIF15-768.264	-e 704x576@15:768

.\bin\BitStreamExtractorStatic	str/Harbour_CIF30-768.264	str/Harbour_CIF30-640.264	-e 352x288@30:640
.\bin\BitStreamExtractorStatic	str/Harbour_CIF30-768.264	str/Harbour_CIF30-512.264	-e 352x288@30:512
.\bin\BitStreamExtractorStatic	str/Harbour_CIF30-768.264	str/Harbour_CIF30-448.264	-e 352x288@30:448

.\bin\BitStreamExtractorStatic	str/Harbour_CIF30-768_o.264	str/Harbour_CIF15-512_o.264	-e 352x288@15:512
.\bin\BitStreamExtractorStatic	str/Harbour_CIF15-512_o.264	str/Harbour_CIF15-512.264	-e 352x288@15:512 -ds 0

.\bin\BitStreamExtractorStatic	str/Harbour_CIF15-512.264	str/Harbour_CIF15-320.264	-e 352x288@15:320
.\bin\BitStreamExtractorStatic	str/Harbour_CIF15-512.264	str/Harbour_CIF15-384.264	-e 352x288@15:384
.\bin\BitStreamExtractorStatic	str/Harbour_CIF15-512.264	str/Harbour_CIF15-448.264	-e 352x288@15:448
.\bin\BitStreamExtractorStatic	str/Harbour_CIF15-512.264	str/Harbour_CIF15-256.264	-e 352x288@15:256

.\bin\BitStreamExtractorStatic	str/Harbour_CIF15-512.264	str/Harbour_CIF7.5-384.264	-e 352x288@7.5:384
.\bin\BitStreamExtractorStatic	str/Harbour_CIF7.5-384.264	str/Harbour_CIF7.5-224.264	-e 352x288@7.5:224
.\bin\BitStreamExtractorStatic	str/Harbour_CIF7.5-384.264	str/Harbour_CIF7.5-192.264	-e 352x288@7.5:192
.\bin\BitStreamExtractorStatic	str/Harbour_CIF7.5-384.264	str/Harbour_CIF7.5-256.264	-e 352x288@7.5:256
.\bin\BitStreamExtractorStatic	str/Harbour_CIF7.5-384.264	str/Harbour_CIF7.5-320.264	-e 352x288@7.5:320

.\bin\BitStreamExtractorStatic	str/Harbour_CIF30-384_o.264	str/Harbour_QCIF15-192.264	-e 176x144@15:192

.\bin\BitStreamExtractorStatic	str/Harbour_QCIF15-192.264	str/Harbour_QCIF15-112.264	-e 176x144@15:112
.\bin\BitStreamExtractorStatic	str/Harbour_QCIF15-192.264	str/Harbour_QCIF15-96.264	-e 176x144@15:96
.\bin\BitStreamExtractorStatic	str/Harbour_QCIF15-192.264	str/Harbour_QCIF15-128.264	-e 176x144@15:128
.\bin\BitStreamExtractorStatic	str/Harbour_QCIF15-192.264	str/Harbour_QCIF15-160.264	-e 176x144@15:160


