#!/user/local/bin/perl
# -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
#
# The contents of this file are subject to the Netscape Public
# License Version 1.1 (the "License"); you may not use this file
# except in compliance with the License. You may obtain a copy of
# the License at http://www.mozilla.org/NPL/
# 
# Software distributed under the License is distributed on an "AS
# IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
# implied. See the License for the specific language governing
# rights and limitations under the License.
#
# The Original Code is Mozilla Communicator client code.
#
# The Initial Developer of the Original Code is Netscape Communications
# Corporation.  Portions created by Netscape are
# Copyright (C) 1998 Netscape Communications Corporation. All
# Rights Reserved.
#
# Contributor(s): 
#
$rowwidth = ((0xff - 0x80)+(0x7f - 0x40));
sub cp936tonum()
{
   my($cp936) = (@_);
   my($first,$second,$jnum);
   $first = hex(substr($cp936,2,2));
   $second = hex(substr($cp936,4,2));
   $jnum = ($first - 0x81 ) * $rowwidth;
   if($second >= 0x80)
   {
       $jnum += $second - 0x80 + (0x7f-0x40);
   }
   else
   {
       $jnum += $second - 0x40;
   }
   return $jnum;
}

@map = {};
sub readtable()
{
open(CP936, "<CP936.TXT") || die "cannot open CP936.TXT";
while(<CP936>)
{
   if(! /^#/) {
        chop();
        ($j, $u, $r) = split(/\t/,$_);
        if(length($j) > 4)
        {
        $n = &cp936tonum($j);
        $map{$n} = $u;
        }
   } 
}
}


#
# According to 
# HKEY_LOCAL_MACHINE:SYSTEM:CurrentControlSet:Control:Nls:CodePage:EUDCCodeRange
# 936=AAA1-AFFE,F8A1-FEFE,A140-A7A0
#
sub addeudc()
{
  print "/* \n";
  print "   The following range are User Defined Characters \n";
  print "       [CP936]              [Unicode] \n";
  my($l,$h,$hl,$us);
  $u = 0xE000;
  $us = sprintf "0x%04X", $u;
  # For AAA1-AFFE
  print "     0xAAA1-0xAFFE Map to " . $us , "-" ;
  for($h=0xAA; $h <=0xAF;$h++)
  {
    for($l=0xA1; $l <=0xFE;$l++,$u++)
    {
        $us = sprintf "0x%04X", $u;
        $hl = sprintf "0x%02X%02X", $h, $l;
        if($map{&cp936tonum($hl)} == "") 
        {
           $map{&cp936tonum($hl)}= $us ;
        }
    }
  }
  print $us . "\n";

  # For F8A1-FEFE
  $us = sprintf "0x%04X", $u;
  print "     0xF8A1-0xFEFE Map to " . $us , "-" ;
  for($h=0xF8; $h <=0xFE;$h++)
  {
    for($l=0xA1; $l <=0xFE;$l++,$u++)
    {
        $us = sprintf "0x%04X", $u;
        $hl = sprintf "0x%02X%02X", $h, $l;
        if($map{&cp936tonum($hl)} == "") 
        {
           $map{&cp936tonum($hl)}= $us ;
        }
    }
  }
  print $us . "\n";

  # For A140-A7A0
  $us = sprintf "0x%04X", $u;
  print "     0xA140-0xA7A0 Map to " . $us , "-" ;
  for($h=0xA1; $h <=0xA7;$h++)
  {
    for($l=0x40; $l <=0x7E;$l++,$u++)
    {
        $us = sprintf "0x%04X", $u;
        $hl = sprintf "0x%02X%02X", $h, $l;
        if($map{&cp936tonum($hl)} == "") 
        {
           $map{&cp936tonum($hl)}= $us ;
        }
    }
    # We need to skip 7F
    for($l=0x80; $l <=0xA0;$l++,$u++)
    {
        $us = sprintf "0x%04X", $u;
        $hl = sprintf "0x%02X%02X", $h, $l;
        if($map{&cp936tonum($hl)} == "") 
        {
           $map{&cp936tonum($hl)}= $us ;
        }
    }
  }
  print $us . "\n";
  print "\n";
  print " */\n";
}

sub printtable()
{
for($i=0;$i<126;$i++)
{
     printf ( "/* 0x%2XXX */\n", ( $i + 0x81));
     for($j=0;$j<(0x7f-0x40);$j++)
     {
         if("" eq ($map{($i * $rowwidth + $j)}))
         {
            printf "0xFFFD,"
         } 
         else 
         {   
            printf $map{($i * $rowwidth + $j)} . ",";
         }
         if( 0 == (($j + 1) % 8))
         {
            printf "/* 0x%2X%1X%1X*/\n", $i+0x81, 4+($j/16), (7==($j%16))?0:8;
         }
     }
     
	 print "0xFFFF,";	#let 0xXX7F map to 0xFFFF

     printf "/* 0x%2X%1X%1X*/\n", $i+0x81, 4+($j/16),(7==($j%16))?0:8;
     for($j=0;$j < (0xff-0x80);$j++)
     {
         if("" eq ($map{($i * $rowwidth + $j + 0x3f)}))		# user defined chars map to 0xFFFD
         {

			if ( ( $i == 125 ) and ( $j == (0xff - 0x80 - 1 )))
			{
				printf "0xFFFD";							#has no ',' followed last item
			}
			else
			{
				printf "0xFFFD,";
			}
         } 
		 else
		 {
			if ( ( $i == 125 ) and ( $j == (0xff - 0x80 - 1 )))
			{
				printf $map{($i * $rowwidth + $j + 0x3f)};	#has no ',' followed last item
			}
			else
			{
				printf $map{($i * $rowwidth + $j + 0x3f)} . ",";
			}
		 }
		  	
         if( 0 == (($j + 1) % 8))
         {
            printf "/* 0x%2X%1X%1X*/\n", $i+0x81, 8+($j/16), (7==($j%16))?0:8;
         }
     }
     printf "       /* 0x%2X%1X%1X*/\n", $i+0x81, 8+($j/16),(7==($j%16))?0:8;
}
}
sub printnpl()
{
$npl = <<END_OF_NPL;
/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */
END_OF_NPL
print $npl;
}
sub printdontmodify()
{
$dont_modify = <<END_OF_DONT_MODIFY;
/*
  This file is generated by mozilla/intl/uconv/tools/cp936tocdx.pl
  Please do not modify this file by hand
  Instead, you should download CP936.TXT from
  http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/
  and put under mozilla/intl/uconv/toools
  and run perl cp936tocdx.pl > ../ucvcn/cp936map.h
  If you have question, mailto:ftan\@netscape.com
 */
END_OF_DONT_MODIFY
print $dont_modify;
}

&readtable();
&printnpl();
&printdontmodify();
&addeudc();
&printtable();

