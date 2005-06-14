cd str\
copy City_4CIF60-2048.264 City_4CIF60-2048_o.264
cd ..\
.\bin\BitStreamExtractorStatic	str/City_4CIF60-2048_o.264	str/City_4CIF60-2048_1.264	-e 704x576@60:2048
.\bin\BitStreamExtractorStatic	str/City_4CIF60-2048_1.264	str/City_4CIF60-2048.264	-e 704x576@60:2048 -ds 0 -ds 1

@REM ============= MUNICH TEST POINTS ==========
.\bin\BitStreamExtractorStatic	str/City_4CIF60-2048_1.264	str/City_4CIF30-1024_o.264	-e 704x576@30:1024 
.\bin\BitStreamExtractorStatic	str/City_4CIF30-1024_o.264	str/City_4CIF30-1024.264	-e 704x576@30:1024 -ds 0 -ds 1

.\bin\BitStreamExtractorStatic	str/City_4CIF30-1024_o.264	str/City_CIF30-512_o.264	-e 352x288@30:512
.\bin\BitStreamExtractorStatic	str/City_CIF30-512_o.264	str/City_CIF30-512.264	-e 352x288@30:512 -ds 0

.\bin\BitStreamExtractorStatic	str/City_CIF30-512_o.264	str/City_CIF30-256_o.264	-e 352x288@30:256
.\bin\BitStreamExtractorStatic	str/City_CIF30-256_o.264	str/City_CIF30-256.264	-e 352x288@30:256 -ds 0

.\bin\BitStreamExtractorStatic	str/City_CIF30-256_o.264	str/City_QCIF15-128.264	-e 176x144@15:128
.\bin\BitStreamExtractorStatic	str/City_QCIF15-128.264	str/City_QCIF15-64.264	-e 176x144@15:64


@REM =========== OTHER TEST POINTS ===========
.\bin\BitStreamExtractorStatic	str/City_4CIF60-2048.264	str/City_4CIF60-1792.264	-e 704x576@60:1792 
.\bin\BitStreamExtractorStatic	str/City_4CIF60-2048.264	str/City_4CIF60-1536.264	-e 704x576@60:1536
.\bin\BitStreamExtractorStatic	str/City_4CIF60-2048.264	str/City_4CIF60-1280.264	-e 704x576@60:1280 
.\bin\BitStreamExtractorStatic	str/City_4CIF60-2048.264	str/City_4CIF60-1024.264	-e 704x576@60:1024 

.\bin\BitStreamExtractorStatic	str/City_4CIF60-2048.264	str/City_4CIF30-1536.264	-e 704x576@30:1536
 
.\bin\BitStreamExtractorStatic	str/City_4CIF30-1536.264	str/City_4CIF30-1280.264	-e 704x576@30:1280
.\bin\BitStreamExtractorStatic	str/City_4CIF30-1536.264	str/City_4CIF30-1024.264	-e 704x576@30:1024 
.\bin\BitStreamExtractorStatic	str/City_4CIF30-1536.264	str/City_4CIF30-896.264	-e 704x576@30:896
.\bin\BitStreamExtractorStatic	str/City_4CIF30-1536.264	str/City_4CIF30-768.264	-e 704x576@30:768

.\bin\BitStreamExtractorStatic	str/City_4CIF30-1536.264	str/City_4CIF15-1024.264	-e 704x576@15:1024
.\bin\BitStreamExtractorStatic	str/City_4CIF15-1024.264	str/City_4CIF15-896.264	-e 704x576@15:896
.\bin\BitStreamExtractorStatic	str/City_4CIF15-1024.264	str/City_4CIF15-768.264	-e 704x576@15:768
.\bin\BitStreamExtractorStatic	str/City_4CIF15-1024.264	str/City_4CIF15-640.264	-e 704x576@15:640
.\bin\BitStreamExtractorStatic	str/City_4CIF15-1024.264	str/City_4CIF15-512.264	-e 704x576@15:512

.\bin\BitStreamExtractorStatic	str/City_CIF30-512.264	str/City_CIF30-448.264	-e 352x288@30:448
.\bin\BitStreamExtractorStatic	str/City_CIF30-512.264	str/City_CIF30-384.264	-e 352x288@30:384
.\bin\BitStreamExtractorStatic	str/City_CIF30-512.264	str/City_CIF30-320.264	-e 352x288@30:320

.\bin\BitStreamExtractorStatic	str/City_CIF30-512.264	str/City_CIF15-384.264	-e 352x288@15:384

.\bin\BitStreamExtractorStatic	str/City_CIF15-384.264	str/City_CIF15-320.264	-e 352x288@15:320
.\bin\BitStreamExtractorStatic	str/City_CIF15-384.264	str/City_CIF15-256.264	-e 352x288@15:256
.\bin\BitStreamExtractorStatic	str/City_CIF15-384.264	str/City_CIF15-224.264	-e 352x288@15:224
.\bin\BitStreamExtractorStatic	str/City_CIF15-384.264	str/City_CIF15-192.264	-e 352x288@15:192

.\bin\BitStreamExtractorStatic	str/City_CIF15-384.264	str/City_CIF7.5-256.264	-e 352x288@7.5:256
.\bin\BitStreamExtractorStatic	str/City_CIF7.5-256.264	str/City_CIF7.5-224.264	-e 352x288@7.5:224
.\bin\BitStreamExtractorStatic	str/City_CIF7.5-256.264	str/City_CIF7.5-192.264	-e 352x288@7.5:192
.\bin\BitStreamExtractorStatic	str/City_CIF7.5-256.264	str/City_CIF7.5-160.264	-e 352x288@7.5:160
.\bin\BitStreamExtractorStatic	str/City_CIF7.5-256.264	str/City_CIF7.5-128.264	-e 352x288@7.5:128

.\bin\BitStreamExtractorStatic	str/City_QCIF15-128.264	str/City_QCIF15-112.264	-e 176x144@15:112
.\bin\BitStreamExtractorStatic	str/City_QCIF15-128.264	str/City_QCIF15-96.264	-e 176x144@15:96
.\bin\BitStreamExtractorStatic	str/City_QCIF15-128.264	str/City_QCIF15-80.264	-e 176x144@15:80


