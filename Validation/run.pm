#!/usr/bin/perl
###############################################################################
# Copyright 2006 -- Thomson R&D France Snc
#                   Technology - Corporate Research
###############################################################################
# File          : run.pm
# Author        : jerome.vieron@thomson.net
# Creation date : 25 January 2006
# Version       : 0.0.1
################################################################################


#-----------------------#
# System Packages       #
#-----------------------#
use strict;
use IO::File;
use File::Path;
use Cwd;
use Data::Dumper;

#-----------------------#
# Project Packages      #
#-----------------------#
use Tools::External;
use Tools::Tools;
use Tools::DirTree;

#-----------------------#    
# Local variables       #    
#-----------------------#    
my $GLOBAL_LOG = "../Global.log"; #=== Global Log file
my $DO_DISPLAY = 1;            #=== Display on stdout or not	

#=== Hash table containing Paths
my $Param={
	       path_globalorig 	=> "../../orig/",#=== original YUV sequences directory
	       path_bin 	=> "../../bin/", #=== binaries directory 
	     
	       path_orig 	=> "orig/",
	       path_cfg 	=> "cfg/",
	       path_tmp 	=> "tmp/",
	       path_str 	=> "str/",
               path_mot 	=> "mot/",
               path_fgs 	=> "fgs/",
               path_rec		=> "rec/",
               path_crop	=> "crop/",
               path_log		=> "../",               
               path_database	=> "SimuDataBase/Short_term/",  #=== directory containing Simulations DataBase
               path_simu	=> "SimuRun/",		   #=== directory where running simulations	
 	      };


#-----------------------#
# Functions             #
#-----------------------#
################################################################################
# Function         : PrintLog ($;[$];[$])
##############################################################################
sub PrintLog 
{
	my $string  =shift;
	my $log     =shift;
	my $display =shift;
	
	#$display=1;
	
	(defined $log) or $log = $GLOBAL_LOG;
	(defined $display) or $display = $DO_DISPLAY;
	
	my $is_append = (-f $log);
	my $hlog = new IO::File $log, ($is_append ? "a" : "w");
        (defined $hlog) or die "- Failed to open the logfile $log : $!";
         	
  	unless (ref $string eq "IO::File")
  	{
		print $hlog $string;
		($display) and print $string;
	}
	else
	{
		while (<$string>)
		{
			print $hlog $_;
			($display) and print $_;
		}
	}
	 $hlog->close;
}


###############################################################################
# Function         : RunSimus (@)
###############################################################################
sub RunSimus
{
	my @listsimus = @_;
	
	my $currentdir=getcwd();
	
	foreach my $simuname (@listsimus)
	{
		my ($simu,$simudir)=Tools::LoadSimu($simuname,$Param) ;
		chdir $simudir or die "Can not change dir to $simudir $!";
		
		$GLOBAL_LOG="../".$simu->{name}."_Global.log";
		(-f $GLOBAL_LOG) and unlink $GLOBAL_LOG;
		
		PrintLog "\n==================================================\n";
		PrintLog " Run simu $simuname:\n";
		PrintLog "-------------------\n";
		
		PrintLog(" Load Simu\t\t.......... ok\n");
			
		Tools::CreateSequences($simu,$Param) and PrintLog("ok\n");
		
		($simu->{runencode}) and External::Encode($simu,$Param) and PrintLog("ok\n");
			
		($simu->{qualitylayer}) and External::QLAssigner($simu,$Param) and PrintLog("ok\n");	
			
		my $ret=Tools::ApplyTests ($simu,$Param) and PrintLog("ok\n");
	
		#print Dumper($simu);
		chdir $currentdir or die "Can not change dir to $currentdir $!";
		
		DirTree::CleanSimuDir ($simu->{name},$Param,$simu->{verbosemode}-$ret);
	}
}

###############################################################################
# Function         : GetArg (\@)
###############################################################################
sub GetArg($)
{
	my $argv =shift;
	my @list ;
	while (@$argv)
	{
	if ($argv->[0]=~ /^[^-]/ ) { push (@list,$argv->[0]) and shift @$argv;}
	else {last;}
	}
	
	return @list;
}

###############################################################################
# Function         : Usage ([$])
###############################################################################
sub Usage (;$)
{
  my $str = shift;
  (defined $str) and print "$str\n";
  print "\n Usage: run [-bin <bin_directory>]  
  	    [-seq <orig_directory>] 
  	    [-name_simuset  [ <name_simu1>...<name_simuN>] ]  
  	    [-u]  				              : Usage	\n";
    
  exit 1;
}


#------------------------------------------------------------------------------#
# Main Program                                                                 #
#------------------------------------------------------------------------------#
$|=1;

if( $#ARGV >=0){$Param->{path_database}="SimuDataBase";}
my @ListSimus;
my @SimusDB=DirTree::GetDir($Param->{path_database});

while (@ARGV)
{
	my $arg=shift @ARGV;	
		
	for($arg)
	{
	if   (/-seq/)   {
				 ($arg=shift @ARGV) or Usage;
				 $arg=~ s|\\|/|g;
				 $Param->{path_globalorig} = DirTree::CheckDir($arg);
			}
	elsif( /-bin/)   {
				 ($arg=shift @ARGV) or Usage;;
				 $arg=~ s|\\|/|g;
				 $Param->{path_bin}= DirTree::CheckDir($arg);
			 }			
  	 elsif( /^-/)   {
			 	$arg =~ s/^-//;
				my @simudir=grep (/^$arg/,@SimusDB);	 
			       ($#simudir == 0) or (print "\n Several simulations sets (or no simulations set) beginning by $arg exist(s) ! \n" and Usage); 
			       	$Param->{path_database} .= "/$simudir[0]/";
				undef @ListSimus;
				@ListSimus=GetArg(\@ARGV);
			}
	 else 	      {Usage;}       
	}
}

(defined @ListSimus) or @ListSimus=grep ( $_ ne "DATA", DirTree::GetDir($Param->{path_database}));

# ===== Simulations ==================================================
RunSimus(@ListSimus);

1;
__END__

