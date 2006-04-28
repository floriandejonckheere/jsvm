###############################################################################
# Copyright 2006 -- Thomson R&D France Snc
#                   Technology - Corporate Research
###############################################################################
# File          : README.TXT
# Author        : jerome.vieron@thomson.net
# Creation date : 25 January 2006
# Version       : 0.0.5
################################################################################


The current directory contains the following sub-directories:
=============================================================

SimuDataBase : contains the "Simulations" data base (i.e. tests set) to be run for the evaluation phase 
                
Tools        : perl scripts and tools.

SimuRun      : location where the "Simulations" will be run. (if not present, will be automatically created)

[orig]       : contains the original YUV sequences (typically the 4CIF sequences) [OPTIONAL]
                NOTE: The location of this directory (including its name) can be specifiede.
               
[bin]        : contains the binaries of the encoder,decoder,extractor,downconvert and psnr tools;[OPTIONAL] 
                NOTE : The location of this directory (including its name) can be specified.


IMPORTANT NOTES:
================-

1) The directory "orig"  must at least contain the following YUV sequences:
- BUS_352x288_30.yuv, MOBILE_352x288_30.yuv, FOOTBALL_352x288_30.yuv  to run the Short-term simulations set.
- CREW_704x576_30.yuv to run the Long-term simulations set.

2) The directory "bin" must contain the following executables
- DownConvertStatic
- H264AVCDecoderLibTestStatic
- H264AVCEncoderLibTestStatic
- BitStreamExtractorStatic
- PSNRStatic
- QualityLevelAssignerStatic

3) Before running AVC_Conformance simulation set, please read the README.TXT files located in the SimuDataBase/AVC_Conformance/ directory.
You will have to download the conformance bistreams and YUV sequences available at "http://ftp3.itu.ch/av-arch/jvt-site/draft_conformance/".
The numbering of the conformance tests corresponds to the classification of the conformance bit-streams in the ITU-T Recommendation H.264.1 "Conformance specification for H.264 advanced video coding".
So far, only tests "6.6.1, 6.6.11 and, Hierachical" must be run. Please again, read the instructions...


It contains also the file:
==========================

"run.pm" which is the main Perl script to be run in order to validate/evaluate JSVM perfomances (mainly after integration phase).
---------

Usage:  
------
[-SimusetName  [ <SimuName1>...<SimuNameN>] ] 
  	   		:  to specify that the  simulations set called ""SimusetName" must be run (default value: Short_term)
  	   	  : "SimusetName" must be the name of a sub-directory of the SimuDataBase directory     
  	    NOTE 0: You can specify the name of the "Simulations" you want to run inside a given simulations-set by specifying <SimuName1>...<SimuNameN>.
  	    NOTE 1: You can just specify a prefix of the "name_simuset" set.

[-bin <bin_directory>]   :to specify the bin directory location  (default value: ./bin)

[-seq <orig_directory>]  :to specify the orig directory location (default value: ./orig)

[-v]  : Version number; 
[-u ] : Usage;

        




The run.pm script works as following:
====================================

- Building of the work directory:
--------------------------------
 A dedicated directory (with the Name of the considered simulation) will be created in the SimuRun directory.
 All the data related to this simulation (located in the DataBase directory) will be copy in these new directory.

- Reference sequences generation: 
---------------------------------
Using ONE original "High Dimension" YUV sequence as input, the DownConvertStatic tools is used to generate the reference sequences of each layer.
NOTE: if the sequences are already built, this creation will not be done.

- Encoding process:
-------------------
This part can be set to OFF depending on the kind of Simulation you want to run.
If required, a step of so-called "quality layer assignment" will be applied.
 
- TESTS processes:
-----------------
    For each test:
    -Extracting and/or Decoding
    -Check results (if required)
    
    
A GLOBAL log file will be created at the root of the "SimuRun" directory.
A LOCAL log file (detailed) will be created at the root of the "SimuRun" directory.
    
NOTE CLEANING PROCESS
===========================
-At the beginning of a Simulation run, if the "SimuRun/SimulationName" directory exists it will be removed.
Except if the encoding process is set to OFF.

