#!/usr/bin/perl
use File::Copy;
use File::Path qw(make_path remove_tree);

chomp( $guid = uc(`GuidGenConsole.exe`) );

print "GUID=$guid\n";

print "Enter project name: ";
chomp($prjname = <STDIN>);

make_path($prjname, {
      verbose => 1,
      mode => 0711,
  } );

# ********************************************************************************
# PROJECT FILE
open(VCPROJIN, "template.vcxproj") or die "could not read template.vcxproj\n";
open(VCPROJOUT, ">$prjname/$prjname.vcxproj") or die "could not write $prjname.vcxproj\n";
while( $line = <VCPROJIN> )
{
	$line =~ s/template/$prjname/g;
	$line =~ s/{guid}/{$guid}/g;
	print VCPROJOUT $line;
}

print "Wrote $prjname/$prjname.vcxproj\n";

close VCPROJIN;
close VCPROJOUT;

# ********************************************************************************
# RESOURCE FILE
copy("template.rc", "$prjname/$prjname.rc");
copy("icons/monitor.ico", "$prjname/app.ico");
print "Copied RC file\n";

# ********************************************************************************
# SOURCE .CPP FILE
open(CPPIN, "template.cpp") or die "could not read template.cpp\n";
open(CPPOUT, ">$prjname/$prjname.cpp") or die "could not write $prjname.cpp\n";
while( $line = <CPPIN> )
{
	$line =~ s/template/$prjname/g;
	print CPPOUT $line;
}
close CPPIN;
close CPPOUT;
print "Copied source file\n";
