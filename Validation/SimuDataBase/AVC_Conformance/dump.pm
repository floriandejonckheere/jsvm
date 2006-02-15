#!/usr/bin/perl
###############################################################################
# Copyright 2006 -- Thomson R&D France Snc
#                   Technology - Corporate Research
###############################################################################
# File          : run.pm
# Author        : jerome.vieron@thomson.net
# Creation date : 25 January 2006
# Version       : 0.0.2
################################################################################


#-----------------------#
# System Packages       #
#-----------------------#
use strict;
use IO::File;
use File::Copy;
use File::Path;
use Cwd;
use Data::Dumper;


###############################################################################
# Function         : GetDir ($)                                                                         
############################################################################### 
sub GetDir ($)
{
     my $srcdir = shift;
         	
     opendir(DIR,$srcdir) or die "GetDir : $srcdir doesn't exist $!";
     my @dirlist = grep( $_ ne '.' && $_ ne '..' && (-d "$srcdir/$_"), readdir(DIR));
     closedir DIR;
      
     return @dirlist;
 }    
 
###############################################################################
# Function         : GetFile ($)                                                                         
############################################################################### 
 sub GetFile ($)
{
     my $srcdir = shift;
         	
     opendir(DIR,$srcdir) or die "GetFile : $srcdir doesn't exist $!";
     my @filelist = grep((-f "$srcdir/$_"), readdir(DIR));
     closedir DIR;
      
     return @filelist;
 }    
###############################################################################
# Function         : CheckDir($)                                                                             
###############################################################################
sub CheckDir($)
{
	my $simudir=shift;
	my $currentdir=getcwd();
	 
 	chdir $simudir or die "The directory $simudir doesn't exist! $!";	
 	my $currsimudir=getcwd();
	
	#for cygwin
	$currsimudir=~ s|^/cygdrive/(.)/|$1:/|;
	
	chdir $currentdir or die "Can not go back to root! $!";
		
	return "$currsimudir/"; 	
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
  print "\n Usage: dump 
             [-c] : Copy streams and YUV sequences to appropriate simulations
                    directories
             [-r] : Remove streams and YUV sequences
             [-simu <name_simu1>...<name_simuN> ] : Name simus to copy/remove
             [-data <yuv_streams_directory>]    : Name of directory containing 
                                                 conformance streams/sequences 
                                                 (default: DATA/)
             [-u]                              : Usage	\n";
    
  exit 1;
}

#------------------------------------------------------------------------------#
# Main Program                                                                 #
#------------------------------------------------------------------------------#

$|=1;

my $orig;

my $DoRemove;
my @ListSimus;

while (@ARGV)
{
	my $arg=shift @ARGV;	
	
	for($arg)
	{
	if     (/-c/) {
				 $DoRemove =0;
		      }
	elsif   (/-r/){
				 $DoRemove =1;
		      }
	elsif(/-simu/)  
	     {
				 ($arg=shift @ARGV) or Usage;
				 undef @ListSimus;
				 @ListSimus=GetArg(\@ARGV);
			  }
	elsif(/-data/)
	    {
	       ($#ARGV >= 0) or Usage;
	       $arg=shift @ARGV;
	       $arg=~ s|\\|/|g;
	       $orig= CheckDir($arg);
	    } 		  			
		 else 		{Usage;}       
	}
}

(defined $orig)      or $orig="DATA";
(defined $DoRemove)  or Usage;
(defined @ListSimus) or @ListSimus=grep ( $_ ne $orig,GetDir("./"));

foreach my $simuname (@ListSimus)
{
		my $ref="$simuname/orig";
		my $str="$simuname/str";
		
		if($DoRemove)
		{
			my @ListRef=GetFile($ref);
			my @ListStr=GetFile($str);
			
			#print join("\n", @ListRef);
			#print join("\n", @ListStr);
			
			foreach (@ListRef)
			{ ($_ ne "Readme.txt") or next;
			  print "remove $ref/$_ \n";
			  unlink "$ref/$_" or die "- Can not remove $ref/$_ : $!";
			  }
			
			foreach (@ListStr)
			{ 
			  ($_ ne "Readme.txt") or next;
			  print "remove $str/$_ \n";
			  unlink "$str/$_" or die "- Can not remove $str/$_ : $!";
			}	
		}
		else
		{
			my $logref="$ref/Readme.txt";
			my $logstr="$str/Readme.txt";
			
			my $hlogref = new IO::File "$logref", "r";
		  	(defined $hlogref) or die "- Failed to open $logref : $!";
		
			my $hlogstr = new IO::File "$logstr", "r";
		  	(defined $hlogstr) or die "- Failed to open $logstr : $!";
		  	
		  	while (<$hlogstr>)
		  	{
			  	#chomp;
			  	s/\s*[\n\r]+//g;
			  	unless (/^#/ or /^$/)
			  	{
			  	(-f "$str/$_") and next;
			  	print "copy {$orig/$_} to {$str} \n";
			  	copy("$orig/$_","$str") or die "can not copy $_ $!";
			    }	
		    }
		    	
		    while (<$hlogref>)
		  	{
		  	#chomp;
		  	s/[\n\r]//g;	
		  	unless (/^#/ or /^$/)
		  	{
		  	print "copy {$orig/$_} to {$ref} \n";
		  	copy("$orig/$_","$ref") or die "can not copy $_ $!";
		  	#(-f "$ref/$_") and (unlink "$ref/$_" or die "can not unlink $_ $!");
		  	}	
		    	}
		     
		    	$hlogref->close();
		  	$hlogstr->close();
		}
	
}

1;
__END__


