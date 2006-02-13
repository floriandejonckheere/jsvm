###############################################################################
# Copyright 2006 -- Thomson R&D France Snc
#                   Technology - Corporate Research
###############################################################################
# File          : README.TXT
# Author        : jerome.vieron@thomson.net
# Creation date : 25 January 2006
# Version       : 0.0.1
################################################################################


The current directory contains the following sub-directories:
-------------------------------------------------------------
-orig         : contains the original YUV sequences (typically the 4CIF sequences)
                NOTE : The location of this directory can be specified.

-bin          : contains the binaries of the encoder,decoder,extractor,downconvert and psnr tools; 
                NOTE : The location of this directory can be specified.

-SimuDataBase : contains the "Simulations" data base (i.e. tests set) to be run for the evaluation phase 
                
-SimuRun      : location where the "Simulations" will be run.

-Tools        : perl scripts and tools 


It contains also the file:
--------------------------

"run.pm" which is the main Perl script to be run in order to validate/evaluate JSVM perfomances (mainly after integration phase).
---------

Usage: run [-bin <bin_directory>]   :to specify the bin directory location  (default value: ./bin)
  	   [-seq <orig_directory>]  :to specify the orig directory location (default value: ./orig)
  	   [-name_simuset  [ <name_simu1>...<name_simuN>] ] 
  	   		:  to specify that the "name_simuset" simulations set must be run (default value: Short)
  	   	  	: "name_simuset" must the name of a sub-directory of the SimuDataBase directory     
  	    NOTE 0: You can specify the name of the "Simulations" you want to run inside a given simulations-set by specifying 
  	          <name-simu1>...<name-simuN>.
  	    NOTE 1: You can just specify a prefix of the "name_simuset" set.

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

- TESTS processes:
-----------------
    For each test:
    -Extracting and/or Decoding
    -Check results (if required)
    
    
A GLOBAL log file will be created at the root of the "SimuRun" directory.
A LOCAL log file (detailed) will be created at the root of the "SimuRun/SimulationName" directory.     
    
    
NOTE ABOUT CLEANING PROCESS
---------------------------
-At the beginning of a Simulation run, if the "SimuRun/SimulationName" directory exists it will be removed.
Except if the encoding process is set to OFF.

