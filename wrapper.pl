# Exiled.Net IRC Services.
# $Id: wrapper.pl,v 1.1.1.1 2002/08/20 15:20:02 mr Exp $
#
# Simple wrapper script to automatically restart
# services after a crash, and copy the executable
# and the core-file to a seperate subdirectory
# so it can be debugged later.

#!/usr/bin/perl


use File::Copy;
use strict;

my $execfile = "./services";
my $corefile = "./services.core";

my $err = -1;

my $lastexit = 0;

while ($err)
{
  $err = system($execfile);
  printf("Returned> $err\n");
  
  if ($err)
  {
    my $ctime = time();
    printf("$ctime\n");
    copy($execfile, "cores/$ctime");
    copy($corefile, "cores/$ctime.core");
    system("rm -f $corefile");
  }
  if ((time() - $lastexit) < (60*5))
  {
    $err = 0;
  }
  $lastexit = time();
  sleep 60;
}

