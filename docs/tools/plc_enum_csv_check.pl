#!/usr/bin/perl
# PLCMAdress.csv (Bin/Config, runtime copy) <-> plc_MBIT_t enum (Manager = compiled, Include = used by dSACC DLL) positional check.
# Run from repo root: perl docs/tools/plc_enum_csv_check.pl   (LYS 2026-09-17)
use strict; use warnings;
sub rd { my $f=shift; open(my $i,"<:raw",$f) or die "open $f: $!"; local $/; my $d=<$i>; close $i; $d }
sub parse_enum { my $d=shift; my $p=rindex($d,"enum plc_MBIT_t"); die "enum not found\n" if $p<0; my $s=index($d,"{",$p); my $e=index($d,"};",$s); my $body=substr($d,$s+1,$e-$s-1);
  my @names; my $aliases=0; my $count;
  for my $line (split /\r?\n/, $body) { $line =~ s{//.*$}{}; $line =~ s{/\*.*?\*/}{}g; $line =~ s/^\s+|\s+$//g; next unless length $line;
    for my $tok (split /,/, $line) { $tok =~ s/^\s+|\s+$//g; next unless length $tok;
      if ($tok =~ /^(\w+)\s*=\s*(\w+)$/) { my ($n,$v)=($1,$2); if ($v =~ /^\d+$/) { push @names,$n } else { $aliases++ } }
      elsif ($tok =~ /^(\w+)$/) { if ($1 eq 'PLC_MBIT_COUNT') { $count=$1 } else { push @names,$1 } } } }
  return (\@names,$aliases,defined $count) }
my ($mgr,$al1,$c1)=parse_enum(rd("Source/ALPHA/Manager/IOMapAddressDefine.h"));
my ($inc,$al2,$c2)=parse_enum(rd("Source/Include/IOMapAddressDefine.h"));
my @csv; my $csv=rd("Source/Bin/Config/CoreFunction/IOMapConfig/PLCMAdress.csv"); my @lines=split /\r?\n/,$csv; shift @lines;
for my $l (@lines) { next unless length $l; my ($n,$a,$rest)=split /,/,$l,3; push @csv,[$n//'',$a//'',$rest//''] }
printf "Manager enum slots=%d (aliases %d, COUNT sentinel %s)  Include enum slots=%d  CSV data rows=%d\n", scalar @$mgr,$al1,($c1?"yes":"no"),scalar @$inc,scalar @csv;
my $inc_same = (@$mgr==@$inc); if ($inc_same) { for (0..$#$mgr) { if ($mgr->[$_] ne $inc->[$_]) { $inc_same=0; print "Manager/Include name differs at slot $_: $mgr->[$_] vs $inc->[$_]\n"; last } } }
print "Manager vs Include order: ",($inc_same?"identical":"DIFFERENT"),"\n";
my $n = @$mgr < @csv ? @$mgr : @csv; my $mis=0; my $shift=0; my @mislist;
for my $i (0..$n-1) { my $e=$mgr->[$i]; my $c=$csv[$i][0]; next if $e eq $c; $mis++; push @mislist,[$i,$e,$c,$csv[$i][1]];
  # shift signature: csv name equals neighbouring enum name
  $shift++ if ($i+1<@$mgr && $c eq $mgr->[$i+1]) || ($i>0 && $c eq $mgr->[$i-1]) }
print "name mismatches: $mis (of $n)   shift-signature mismatches (csv==neighbour enum): $shift\n";
print "--- first 12 mismatches (slot, enum, csv, addr)\n"; printf "%5d %-45s %-45s %s\n",@$_ for @mislist[0..($#mislist<11?$#mislist:11)];
print "--- last 6 mismatches\n"; printf "%5d %-45s %-45s %s\n",@$_ for @mislist[($#mislist>5?$#mislist-5:0)..$#mislist];
# longest run of consecutive mismatches (a shift shows as a long run)
my ($run,$best,$bs)=(0,0,0); for my $i (0..$n-1) { if ($mgr->[$i] ne $csv[$i][0]) { $run++; if ($run>$best){$best=$run;$bs=$i-$run+1} } else { $run=0 } }
print "longest consecutive mismatch run: $best (starting slot $bs)\n";
# spot checks for known addresses
for my $want (qw(PLC_AUTOCHANGE_DOWN_01 PLC_AUTOCHANGE_PROBE_CHECK1 PLC_AUTO_CHANGE_X0_POSITION_03 PLC_AUTO_CHANGE_X0_POSITION_04 PLC_R_PROBE_UNIT05_CARD_CHK_01 PLC_CASSETTE_PROBE_CHK_06_1 PLC_R_PACC_BUFFER_PROBE_CARD_CHK_02_2 PLC_BIT_W_AC_X_SAFETY_POS PLC_BIT_W_EQ_BUSY_ON PLC_BIT_UNLOAD_SENSOR03)) {
  my ($i)=grep{$mgr->[$_] eq $want}0..$#$mgr; if (defined $i) { printf "%-40s enum slot %4d -> csv row %4d '%s' addr %s\n",$want,$i,$i,$csv[$i][0],$csv[$i][1] } else { print "$want: not in enum\n" } }
print "\n=== slots 190-252 (enum | csv | addr)\n"; for my $i (190..252) { printf "%4d %-40s %-42s %s%s\n",$i,$mgr->[$i],$csv[$i][0],$csv[$i][1],($mgr->[$i] eq $csv[$i][0]?"":"   <-- diff") }
print "\n=== Manager vs Include divergence\n"; my $m=@$mgr<@$inc?@$mgr:@$inc; my $first; for (0..$m-1) { if ($mgr->[$_] ne $inc->[$_]) { $first=$_; last } }
if (defined $first) { print "first differing slot: $first\n"; for my $i ($first-2..$first+8) { printf "%4d M:%-40s I:%s\n",$i,$mgr->[$i]//'-',$inc->[$i]//'-' } } else { print "prefix identical; Manager has extra tail: ",join(", ",@$mgr[$m..$#$mgr]),"\n" }
my %inc=map{$_=>1}@$inc; print "in Manager but not Include: ",join(", ",grep{!$inc{$_}}@$mgr),"\n"; my %mg=map{$_=>1}@$mgr; print "in Include but not Manager: ",join(", ",grep{!$mg{$_}}@$inc),"\n";
print "\n=== shift-signature slot(s)\n"; for my $i (0..$n-1) { my $c=$csv[$i][0]; next if $c eq $mgr->[$i]; if (($i+1<@$mgr && $c eq $mgr->[$i+1]) || ($i>0 && $c eq $mgr->[$i-1])) { printf "%4d enum=%s csv=%s addr=%s (enum[i-1]=%s enum[i+1]=%s)\n",$i,$mgr->[$i],$c,$csv[$i][1],$mgr->[$i-1]//'-',$mgr->[$i+1]//'-' } }
print "\n=== Include vs Manager index delta (Manager idx - Include idx), by range\n"; my %mi; $mi{$mgr->[$_]}=$_ for 0..$#$mgr; my ($prev,$start); for my $j (0..$#$inc) { my $nm=$inc->[$j]; my $d = exists $mi{$nm} ? $mi{$nm}-$j : 'NA'; if (!defined $prev || $d ne $prev) { printf "  Include slot %4d.. delta %s (starts at %s)\n",$j,$d,$nm; $prev=$d } }
print "\n=== Manager-only names and their Manager slots\n"; printf "  %4d %s\n",$mi{$_},$_ for grep{!$inc{$_}}@$mgr;
print "\n=== SACC-relevant bits: Manager slot vs Include slot\n"; my %ii; $ii{$inc->[$_]}=$_ for 0..$#$inc;
for my $nm (qw(PLC_PACC_AUTO_MODE PLC_PACC_PACC_ALL_SEQ_DONE PLC_BIT_UNLOAD_SENSOR03 PLC_AUTOCHANGE_PROBE_CHECK1 PLC_AUTOCHANGE_PROBE_CHECK2 PLC_CASSETTE_PROBE_CHK_01_1 PLC_CASSETTE_PROBE_CHK_06_1 PLC_R_PACC_BUFFER_PROBE_CARD_CHK_01_1 PLC_R_PROBE_UNIT05_CARD_CHK_01 PLC_R_PACC_BUFFER_01_UP_01)) { printf "  %-40s Manager %4s  Include %4s  %s\n",$nm,$mi{$nm}//'-',$ii{$nm}//'-',(defined $mi{$nm} && defined $ii{$nm} && $mi{$nm}==$ii{$nm})?"same":"DIFF" }
