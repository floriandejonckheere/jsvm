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
  print "\n Usage: dump [-r] : Remove streams and YUV sequences (default: Copy)
  	     [-s <name_simu1>...<name_simuN> ] : Name simus to copy/remove
  	     [-u]  			       : Usage	\n";
    
  exit 1;
}

#------------------------------------------------------------------------------#
# Main Program                                                                 #
#------------------------------------------------------------------------------#

$|=1;

my $orig="DATA";

my $DoRemove = 0;
my @ListSimus;

while (@ARGV)
{
	my $arg=shift @ARGV;	
	
	for($arg)
	{
	if   (/-r/)   {
				 $DoRemove =1;
			}
	elsif(/-s/)   {
				 ($#ARGV >= 0) or Usage;
				 undef @ListSimus;
				 @ListSimus=GetArg(\@ARGV);
			  }			
	
	 else 		{Usage;}       
	}
}

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
		  	(defined $hlogref) or die "- Failed to open $hlogref : $!";
		
			my $hlogstr = new IO::File "$logstr", "r";
		  	(defined $hlogstr) or die "- Failed to open $hlogstr : $!";
		  	
		  	while (<$hlogstr>)
		  	{
			  	#chomp;
			  	s/[\n\r]//g;
			  	unless (/^#/ or /^$/)
			  	{
			  	print "copy {$orig/$_} to {$str} \n";
			  	copy("$orig/$_","$str") or die "can not copy $_ $!";
			  	#(-f "$str/$_") and (unlink "$str/$_" or die "can not unlink $_ $!");
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


