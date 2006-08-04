#!/usr/bin/perl
###############################################################################
# Copyright 2006 -- Thomson R&D France Snc
#                   Technology - Corporate Research
###############################################################################
# File          : External.pm
# Author        : jerome.vieron@thomson.net
# Creation date : 25 January 2006
# Version       : 0.0.5
################################################################################

package External;

#-----------------------#
# System Packages       #
#-----------------------#
use strict;
use IO::File;

#-----------------------#
# Local Constants       #
#-----------------------#
my $ENCODER    = "H264AVCEncoderLibTestStatic";
my $PSNR       = "PSNRStatic";
my $EXTRACTOR  = "BitStreamExtractorStatic";
my $DECODER    = "H264AVCDecoderLibTestStatic";
my $RESAMPLER  = "DownConvertStatic";
my $QLASSIGNER = "QualityLevelAssignerStatic";
my $JMDECODER  = "ldecod";

#-----------------------#
# Functions             #
#-----------------------#

###############################################################################
# Function         : platformpath ($)
###############################################################################
#check platform and substitute "/" by "\" if needed
sub platformpath($)
{
	my $exe = shift;

	$exe =~ s|/|\\|g if ($^O =~ /^MS/);
	return $exe;
}

######################################################################################
# Function         : run ($;$$$)
######################################################################################
sub run ($;$$$)
{
  my $exe 	    = shift; #cmd line to execute
  my $log 	    = shift; #log STDOUT filename
  my $dodisplay = shift; #display stdout or not 
  my $logerr    = shift; #log STDERR filename

 #$dodisplay = 1;
 
  
  $exe = platformpath($exe);
  
  ::PrintLog("\n $exe\n", $log,$dodisplay);
 
  #Catch STDERR
  #------------ 
  if(defined $logerr) 
  {
    open(mystderr,">&STDERR");
    open(STDERR,">$logerr");
  }
 
  #Catch STDOUT
  #----------- 
  my $hexe = new IO::File "$exe |";
  (defined $hexe) or die "- Failed to run $exe : $!";
 
  ::PrintLog($hexe, $log, $dodisplay);

  $hexe->close;
	
	if(defined $logerr) 
	{open(STDERR,">&mystderr");}
	
	
  my $ret = $?;

  return $ret;
}


######################################################################################
# Function         : Encode ($;$)
######################################################################################
sub Encode($;$)
{
	my $simu=shift;
	my $param=shift;
	my $bin = $param->{path_bin};
		
	 ::PrintLog(" Encode                    .......... ");
		
	my $FGSlayer	= $simu->{nbfgslayer};
	my $FGSConf="";
	my $MotConf="";
	my $MotConfCGS="";
	my $DSConf="";
	my $cmd ;
	my $ret;
	
	my $l=-1;
	my $lp1=0;
	my $layer;
	foreach $layer (@{$simu->{layers}})
	{
		$l++;
		$lp1++;	
		if($layer->{bitrate}==0)
		{
			#cgs layer
			#motion for this layer wil have to be computed later
			$MotConf	.= " -mfile $l 2 ".$layer->{motionname};
			$MotConfCGS	.= " -mfile $l 1 ".$layer->{motionname};
		}	
		else
		{
			#Layer with FGS content
			$cmd = "$bin$ENCODER -pf ".$simu->{configname}." -bf ".$simu->{bitstreamname}." -numl $lp1  $DSConf $MotConf -mfile $l 2 ".$layer->{motionname}." $FGSConf -anafgs $l $FGSlayer ".$layer->{fgsname}." ".$simu->{singleloopflag};
    	$ret = run($cmd, $simu->{logname},0);
  	 ($ret == 0) or die "problem while executing the command:\n$cmd\n";

			if ($layer->{bitrateDS}>0)
			{
				$DSConf .=" -ds $l ".$layer->{bitrateDS};
			}

			$FGSConf .= " -encfgs $l ".$layer->{bitrate}." ".$layer->{fgsname};

			#motion can be read
			$MotConf	 = "$MotConfCGS -mfile $l 1 ".$layer->{motionname};
			$MotConfCGS	 = "$MotConfCGS -mfile $l 1 ".$layer->{motionname};
		}
	}	
	
	$cmd = "$bin$ENCODER -pf ".$simu->{configname}." -bf ".$simu->{bitstreamname}." -numl $lp1 $DSConf $MotConf $FGSConf ".$simu->{singleloopflag};
 	$ret = run($cmd,$simu->{logname},0);
  ($ret == 0) or die "problem while executing the command:\n$cmd\n";
  	
	
  	return 1;
}

######################################################################################
# Function         : QLAssigner ($;$)
######################################################################################
sub QLAssigner($$)
{
	my $simu=shift;
	my $param=shift;
	my $bin =$param->{path_bin};
	my $display=1; 

  ::PrintLog(" QualityLevelAssigner      .......... ");

  my $cmdLayer;
 	my $layer;
 	my $l=0;
	foreach $layer (@{$simu->{layers}})
  {
   $cmdLayer .= " -org $l ".$layer->{origname};
   $l++;
  }
	
	my $cmd = "$bin$QLASSIGNER -in ".$simu->{bitstreamname}." $cmdLayer -out ".$simu->{bitstreamQLname}; 
 ($cmd .= " -sei") if($simu->{qualitylayer}==2);
 ($cmd .= " -mlql") if($simu->{qualitylayer}==3);
 ($cmd .= " -mlql -sei") if($simu->{qualitylayer}==4);
  	
	my $ret = run($cmd, $simu->{logname},0);
  	($ret == 0) or die "problem while executing the command:\n$cmd\n";
}

######################################################################################
# Function         : Extract ($;$;$)
######################################################################################
sub Extract($$;$)
{
	my $simu=shift;
	my $test=shift;
	my $param=shift;
	my $bin =$param->{path_bin};
	my $display=1; 

  my $cmd = "$bin$EXTRACTOR ".$test->{bitstreamname}." ".$test->{extractedname}." -e ".$test->{extractoption}; 
 ($cmd .= " -ql") if($test->{useql}==1);
 ($cmd .= " -qlord") if($test->{useql}==2);
 ($cmd .= " -sip") if($test->{usesip}==1);
 ($cmd .= " -sip -suf") if($test->{usesip}>1); 
 
	my $ret = run($cmd, $simu->{logname},0);
  	($ret == 0) or die "problem while executing the command:\n$cmd\n";
}

###############################################################################
# Function         : Decode ($;$;$)
###############################################################################
sub Decode($$;$)
{
	my $simu=shift;
	my $test=shift;
	my $param=shift;
	
	my $bin =$param->{path_bin};
	my $tmp =$param->{path_tmp};
	my $display=1; 

	my $cmd ="$bin$DECODER ". $test->{extractedname}." ".$test->{decodedname};
	(defined $test->{errorconcealment}) and $cmd .= " -ec ".$test->{errorconcealment};
	my $ret = run($cmd, $simu->{logname},0);
  	($ret == 0) or die "problem while executing the command:\n$cmd\n $!";
    	
  	return ComputePSNR($bin,$simu->{logname},$test->{width},$test->{height},$test->{origname},$test->{decodedname},$test->{extractedname},$test->{framerate},"${tmp}psnr.dat");	
 }

###############################################################################
# Function         : JMDecode ($;$;$)
###############################################################################
sub JMDecode($$;$)
{
	my $simu=shift;
	my $test=shift;
	my $param=shift;
	
	my $bin =$param->{path_bin};
	my $tmp =$param->{path_tmp};
	my $display=1; 

	my $cmd ="$bin$JMDECODER ". $test->{extractedname}." ".$test->{jmdecodedname};
	my $ret = run($cmd, $simu->{logname},0);
  	($ret == 0) or die "problem while executing the command:\n$cmd\n $!";
}

###############################################################################
# Function         : ComputePSNR ($$$$$)
###############################################################################
sub ComputePSNR($$$$$$$$$)
{
   my $bin=shift;
   my $log=shift;
   my $width= shift;
   my $height=shift;
   my $refname=shift;
   my $decname=shift;
   my $extname=shift;
   my $framerate = shift;
   my $errname=shift;
   
  my $cmd = "${bin}$PSNR $width $height $refname $decname 0 0 $extname $framerate";
   
   my $ret = run($cmd, $log,0,$errname);
  ($ret == 0) or die "problem while executing the command:\n$cmd \n $!";

   (-f $errname) or die "Problem the file $errname has not been created $!";
    my $PSNR = new IO::File $errname, "r";
    my $result=<$PSNR>;
    #chomp $result;
    $result =~ s/\s*[\n\r]+//g;
    $result =~ s/,/./g;
    $PSNR->close();
   
    my ($res_rate, $res_psnrY, $res_psnrCb, $res_psnrCr) = split ( /	/, $result );
	
    unlink $errname or die "Can not delete $errname $!";
   
   return ($res_rate, $res_psnrY, $res_psnrCb, $res_psnrCr);
}

##############################################################################
# Function         : Resize ($;$;@)
##############################################################################
sub Resize($$;@)
{
	my $param=shift;
	my $log=shift;
	my ($namein,$win,$hin,$frin,$nameout,$wout,$hout,$frout,$phasemode,$essopt,$nbfr,$cropfile)=@_;
	
	my $display=1;
	my $bin =$param->{path_bin};
	my $tmp =$param->{path_tmp};
	
	my $cmd;
	my $ret;
	my $temporatio=GetPowerof2($frin/$frout);
	($win>=$wout) or die "The input width must be greater or equal to the output width $!";
	($hin>=$hout) or die "The input height must be greater or equal to the output height $!";
		
	if(defined $cropfile)
	{
	  if($phasemode==1)
	  {
	    $cmd ="$bin$RESAMPLER  $win $hin $namein  $wout $hout $nameout 0 $temporatio 0 $nbfr -crop 1 $cropfile -phase 0 0 0 0";
	  }
	  else
	  {
	    $cmd ="$bin$RESAMPLER  $win $hin $namein  $wout $hout $nameout 0 $temporatio 0 $nbfr -crop 1 $cropfile";
	  }
		$ret = run($cmd, $log,0);
		($ret == 0) or die "problem while executing the command:\n$cmd\n";  
	}
	else
	{
	  if($phasemode==1)
	  {
	    $cmd ="$bin$RESAMPLER  $win $hin $namein  $wout $hout $nameout 0 $temporatio 0 $nbfr -phase 0 0 0 0";
	  }
	  else
	  {
      $cmd ="$bin$RESAMPLER  $win $hin $namein  $wout $hout $nameout 0 $temporatio 0 $nbfr";
    }
		$ret = run($cmd, $log,0);
		($ret == 0) or die "problem while executing the command:\n$cmd\n";  
	}  	   					         
} 


##############################################################################
# Function         : GetPowerof2 ($)
##############################################################################
sub GetPowerof2
{
	my $val=shift;

        ($val <1) and return -1;
	($val == 1) and return 0;
    
        my $exp = 0 ;
    
        while (!($val & 1))
        {
            $val>>=1; 
            $exp++ ;
        }

        (($val == 1) && ($exp != 0)) and return $exp ;

        return -1 ;
}


1;
__END__
