#!/usr/local/bin/perl

use TIGR::Foundation;
use TIGR::AsmLib;
use XML::Parser;
use POSIX qw(tmpnam);
$ENV{TMPDIR} = ".";

use strict;

my $VERSION = '$Revision$ ';
my $HELP = q~
    toAmos (-m mates|-x traceinfo.xml|-f frg) 
           (-c contig|-a asm|-ta tasm|-ace ace) 
           -o outfile 
           [-i insertfile | -map dstmap]
~;

my $base = new TIGR::Foundation();
if (! defined $base) {
    die("A horrible death\n");
}


$base->setVersionInfo($VERSION);
$base->setHelpInfo($HELP);

my $matesfile;
my $traceinfofile;
my $ctgfile;
my $frgfile;
my $asmfile;
my $tasmfile;
my $acefile;
my $outfile;
my $insertfile;
my $libmap;

my $err = $base->TIGR_GetOptions("m=s"   => \$matesfile,
				 "x=s"   => \$traceinfofile,
				 "c=s"   => \$ctgfile,
				 "f=s"   => \$frgfile,
				 "a=s"   => \$asmfile,
				 "ta=s"  => \$tasmfile,
				 "ace=s" => \$acefile,
				 "o=s"   => \$outfile,
				 "i=s"   => \$insertfile,
				 "map=s" => \$libmap);


my $matesDone = 0;

# this is where all my data live
my %contigs;    # contig ids to contig length map
my %seqids;     # seq_name to seq_id map
my %seqnames;   # seq id to seq name map
my %seqcontig;  # seq id to contig map
my %contigseq;  # contig id to sequence id list 
my %seq_range;  # seq id to clear range
my %asm_range;  # seq id to range within contig
my %contigcons; # contig id to consensus sequence

my %forw;       # insert to forw (rev resp) end mapping 
my %rev; 
my %libraries;  # libid to lib range (mean, stdev) mapping
my %insertlib;  # lib id to insert list
my %seenlib;    # insert id to lib id map
my %seqinsert;  # sequence id to insert id map
my %libnames;   # lib id to lib name

my $minSeqId = 1;  # where to start numbering reads

my $outprefix;

my $tmprefix = tmpnam();

open(TMPSEQ, ">$tmprefix.seq") 
    || $base->bail("Cannot open $tmprefix.seq: $!\n");
open(TMPCTG, ">$tmprefix.ctg") 
    || $base->bail("Cannot open $tmprefix.ctg: $!\n");

#first get the contig information

if (! defined $outfile){
    $base->bail("You must specify an output file with option -o\n");
}
open(OUT, ">$outfile") || $base->bail("Cannot open $outfile: $!\n");

#then figure out the mates
if (defined $frgfile){
    open(IN, $frgfile) || $base->bail("Cannot open $frgfile: $!\n");
    parseFrgFile(\*IN);
    close(IN);
    $matesDone = 1;
}

if (defined $asmfile){
    $outprefix = $asmfile;
    open(IN, $asmfile) || $base->bail("Cannot open $asmfile: $!\n");
    parseAsmFile(\*IN);
    close(IN);
}

if (defined $ctgfile){
    $outprefix = $ctgfile;
    open(IN, $ctgfile) || $base->bail("Cannot open $ctgfile: $!\n");
    parseContigFile(\*IN);
    close(IN);
}

if (defined $tasmfile) {
    $outprefix = $tasmfile;
    open(IN, $tasmfile) || $base->bail("Cannot open $tasmfile: $!\n");
    parseTAsmFile(\*IN);
    close(IN);
}

if (defined $acefile){
    $outprefix = $acefile;
    open(IN, $acefile) || $base->bail("Cannot open $acefile: $!\n");
    parseACEFile(\*IN);
    close(IN);
}

$outprefix =~ s/\.[^.]*$//;

# now it's time for library and mates information

if (defined $traceinfofile){
    $matesDone = 1;
    open(IN, $traceinfofile) || $base->bail("Cannot open $traceinfofile: $!\n");
    parseTraceInfoFile(\*IN);
    close(IN);
}


if (! $matesDone && defined $matesfile) { # the mate file contains either mates
    # or regular expressions defining them
    open(IN, $matesfile) || 
	$base->bail("Cannot open \"$matesfile\": $!\n");
    parseMatesFile(\*IN);
    close(IN);
} # if mates not done defined matesfile

if (defined $insertfile){
    open(IN, $insertfile) || $base->bail("Cannot open $insertfile: $!\n");
    parseInsertFile(\*IN);
    close(IN);
}

if (defined $libmap){
    open(IN, $libmap) || $base->bail("Cannot open $libmap: $!\n");
    parseLibMapFile(\*IN);
    close(IN);
}
close(TMPSEQ);
close(TMPCTG);

## here's where we output all the stuff

# first print out a pretty header message
my $date = localtime();

print OUT "{UNV\n";
print OUT "act:A\n";
print OUT "eid:1\n";
print OUT "com:\n";
print OUT "$date\n";
print OUT ".\n";
print OUT "}\n";


# then print out one library at a time
while (my ($lib, $range) = each %libraries){
    my ($mean, $sd) = split(' ', $range);
    print OUT "{LIB\n";
    print OUT "act:A\n";
    print OUT "eid:$lib\n";
    if (exists $libnames{$lib}){
	print OUT "com:\n$libnames{$lib}\n.\n";
    }
    print OUT "{DST\n";
    print OUT "mea:$mean\n";
    print OUT "std:$sd\n";
    print OUT "skw:0\n";
    print OUT "}\n";
    print OUT "}\n";
}

# then all the inserts
while (my ($ins, $lib) = each %seenlib){
    print OUT "{FRG\n";
    print OUT "act:A\n";
    print OUT "eid:$ins\n";
#    print OUT "com:\n";
    print OUT "lib:$lib\n";
    print OUT "typ:I\n";
#   print OUT "src:0\n";
    print OUT "}\n";
}

# then all the reads
open(TMPSEQ, "$tmprefix.seq") 
    || $base->bail("Cannot open $tmprefix.seq: $!\n");

while (<TMPSEQ>){
    if (/^\#(\d+)/){
	my $rid = $1;
	print OUT "{RED\n";
	print OUT "act:A\n";
	print OUT "eid:$rid\n";
	print OUT "com:\n";
	print OUT "$seqnames{$rid}\n";
	print OUT ".\n";
	print OUT "seq:\n";
	$_ = <TMPSEQ>;
	while ($_ !~ /^\#/){
	    print OUT;
	    $_ = <TMPSEQ>;
	}
	print OUT ".\n";
	print OUT "qlt:\n";
	$_ = <TMPSEQ>;
	while ($_ !~ /^\#/){
	    print OUT;
	    $_ = <TMPSEQ>;
	}
	print OUT ".\n";
	if  (! exists $seqinsert{$rid}){
	    die("Cannot find insert for $rid ($seqnames{$rid})\n");
	}
	print OUT "frg:$seqinsert{$rid}\n";
	my ($cll, $clr) = split(' ', $seq_range{$rid});
	print OUT "clr:$cll,$clr\n";
	print OUT "vcr:$cll,$clr\n";
	print OUT "qcr:$cll,$clr\n";
	print OUT "}\n";
    } else {
	$base->bail("Weird error at line $. in $tmprefix.seq");
    }
}
close(TMPSEQ);

unlink("$tmprefix.seq") || $base->bail("Cannot remove $tmprefix.seq: $!\n");

# then all the links

my $linkId = 0;
while (my ($ins, $rd) = each %forw){
    if (exists $rev{$ins}){
	$linkId++;
	print OUT "{MTP\n";
#	print OUT "act:A\n";
	print OUT "eid:$linkId\n";
	print OUT "rd1:$forw{$ins}\n";
	print OUT "rd2:$rev{$ins}\n";
	print OUT "typ:E\n";
	print OUT "}\n";
    }
}

# then all the contigs

open(TMPCTG, "$tmprefix.ctg") 
    || $base->bail("Cannot open $tmprefix.ctg: $!\n");

while (<TMPCTG>){
    if (/^\#(\d+)/){
	my $cid = $1;
	print OUT "{CTG\n";
	print OUT "act:A\n";
	print OUT "eid:$cid\n";
	print OUT "seq:\n";
	$_ = <TMPCTG>;
	while ($_ !~ /^\#/){
	    print OUT;
	    $_ = <TMPCTG>;
	}
	print OUT ".\n";
	print OUT "qlt:\n";
	$_ = <TMPCTG>;
	while ($_ !~ /^\#/){
	    print OUT;
	    $_ = <TMPCTG>;
	}
	print OUT ".\n";
#	print OUT "ply:\n";
#	print OUT ".\n";
	while (/^\#(\d+)/){
	    my $rid = $1;
	    my ($len, $ren) = split(' ', $asm_range{$rid});
	    my ($cl, $cr) = split(' ', $seq_range{$rid});
	    if ($len > $ren){
		$len = $ren;
		$ren = $cr;
		$cr = $cl;
		$cl = $ren;
	    }
	    print OUT "{TLE\n";
	    print OUT "src:$rid\n";
	    print OUT "off:$len\n";
	    print OUT "clr:$cl,$cr\n";
	    my $deltastring;
	    $_ = <TMPCTG>;
	    while ($_ !~ /^\#/){
		$deltastring .= $_;
		$_ = <TMPCTG>;
	    }
	    if ($deltastring !~ /^\s*$/){
		print OUT "del:\n";
		print OUT $deltastring;
		print OUT ".\n";
	    }
	    print OUT "}\n";
	}
	print OUT "}\n";
    } else {
	$base->bail("Weird error at line $. in $tmprefix.seq");
    }
}

close(TMPCTG);

unlink("$tmprefix.ctg") || $base->bail("Cannot remove $tmprefix.ctg: $!\n");

# all the contig links

# all the contig edges

# and all the scaffolds

close(OUT);

exit(0);




###############################################################################


# LIBRARY NAME PARSING
sub parseInsertFile {
    my $IN = shift;

    while (<IN>){
	if (/GenomicLibrary Id=\"(\S+)\" acc=\"(\d+)\"/){
	    $libnames{$2} = $1;
	    print STDERR "lib-id = $2; lib-name = $1\n";
	}
    }
} # parseInsertFile

sub parseLibMapFile {
    my $IN = shift;

    while (<IN>){
	my ($id, $name) = split(' ', $_);
	$libnames{$id} = $name;
    }
}
# MATES PARSING FUNCTIONS

# parse Trace Archive style XML files
my $tag;
my $library;
my $template;
my $clipl;
my $clipr;
my $mean;
my $stdev;
my $end;
my $seqId;

sub parseTraceInfoFile {
    my $IN = shift;

    my $xml = new XML::Parser(Style => 'Stream');

    if (! defined $xml){
	$base->bail("Cannot create an XML parser");
    }

    # start parsing away.  The hashes will magically fill up

    $xml->parse($IN);

} # parseTraceInfoFile


# Celera .frg
# populates %seqids, %seqnames, and %seq_range, %libraries, %insertlib,
# %seenlib, %seqinsert
sub parseFrgFile {
    my $IN = shift;

    while (my $record = getCARecord($IN)){
	my ($type, $fields, $recs) = parseCARecord($record);
	if ($type eq "FRG") {
	    my $id = getCAId($$fields{acc});
	    my $nm = $$fields{src};
	    my @lines = split('\n', $nm);
	    $nm = join('', @lines);
	    if ($nm ne "" && $nm !~ /^\s*$/){
		$seqnames{$id} = $nm;
		$seqids{$nm} = $id;
	    }
	    my ($seql, $seqr) = split(',', $$fields{clr});
	    $seq_range{$id} = "$seql $seqr";
	    print TMPSEQ "#$id\n";
	    print TMPSEQ "$$fields{seq}";
	    print TMPSEQ "#\n";
	    print TMPSEQ "$$fields{qlt}";
	    print TMPSEQ "#\n";
	    next;
	}
	
	if ($type eq "DST"){
	    my $id = getCAId($$fields{acc});
	    $libraries{$id} = "$$fields{mea} $$fields{std}";
	    next;
	}
	
	if ($type eq "LKG"){
	    my $id = $minSeqId++;
	    $insertlib{$$fields{dst}} .= "$id ";
	    $seenlib{$id} = $$fields{dst};
	    $seqinsert{$$fields{fg1}} = $id;
	    $seqinsert{$$fields{fg2}} = $id;
	    $forw{$id} = $$fields{fg1};
	    $rev{$id} = $$fields{fg2};
	    next;
	}
    }

# make sure all reads have an insert
    my $ll = $minSeqId++;
    $libraries{$ll} = "0 0"; # dummy library for unmated guys
    while (my ($sid, $sname) = each %seqnames){
	if (! exists $seqinsert{$sid}){
	    my $id = $minSeqId++;
	    $seqinsert{$sid} = $id;
	    $seenlib{$id} = $ll;
	    $insertlib{$ll} .= "$id ";
	    $forw{$id} = $sid;
	}
    }
} #parseFrgFile


# parses BAMBUS style .mates file
# * expects %seqids to be populated
# * populates %libraries, %forw, %rev, %insertlib, %seenlib, %seqinsert
sub parseMatesFile {
    my $IN = shift;

    my @libregexp;
    my @libids;
    my @pairregexp;
    my $insname = 1;
    while (<$IN>){
	chomp;
	if (/^library/){
	    my @recs = split('\t', $_);
	    if ($#recs < 3 || $#recs > 4){
		print STDERR "Only ", $#recs + 1, " fields\n";
		$base->logError("Improperly formated line $. in \"$matesfile\".\nMaybe you didn't use TABs to separate fields\n", 1);
		next;
	    }
	    
	    if ($#recs == 4){
		$libregexp[++$#libregexp] = $recs[4];
		$libids[++$#libids] = $recs[1];
	    }
	    my $mean = ($recs[2] + $recs[3]) / 2;
	    my $stdev = ($recs[3] - $recs[2]) / 6;
	    $libraries{$recs[1]} = "$mean $stdev";
	    next;
	} # if library
	if (/^pair/){
	    my @recs = split('\t', $_);
	    if ($#recs != 2){
		$base->logError("Improperly formated line $. in \"$matesfile\".\nMaybe you didn't use TABs to separate fields\n");
		next;
	    }
	    @pairregexp[++$#pairregexp] = "$recs[1] $recs[2]";
	    next;
	}
	if (/^\#/) { # comment
	    next;
	}
	if (/^\s*$/) { # empty line
	    next;
	}
	
	# now we just deal with the pair lines
	my @recs = split('\t', $_);
	if ($#recs < 1 || $#recs > 2){
	    $base->logError("Improperly formated line $. in \"$matesfile\".\nMaybe you didn't use TABs to separate fields\n");
	    next;
	}
	
# make sure we've seen these sequences
	if (! defined $seqids{$recs[0]}){
	    $base->logError("No contig contains sequence $recs[0] at line $. in \"$matesfile\"");
	    next;
	}
	if (! defined $seqids{$recs[1]} ){
	    $base->logError("No contig contains sequence $recs[1] at line $. in \"$matesfile\"");
	    next;
	}
	
	if (defined $recs[2]){
	    $insertlib{$recs[2]} .= "$insname ";
	    $seenlib{$insname} = $recs[2];
	} else {
	    $base->logError("$insname has no library\n");
	}
	
	$forw{$insname} = $seqids{$recs[0]};
	$rev{$insname} = $seqids{$recs[1]};
	
	$seqinsert{$seqids{$recs[0]}} = $insname;
	$seqinsert{$seqids{$recs[1]}} = $insname;
	
	$insname++;
    } # while <IN>

    # now we have to go through all the sequences and assign them to
    # inserts
    while (my ($nm, $sid) = each %seqids){
	for (my $r = 0; $r <= $#pairregexp; $r++){
	    my ($freg, $revreg) = split(' ', $pairregexp[$r]);
	    $base->logLocal("trying $freg and $revreg on $nm\n", 2);
	    if ($nm =~ /$freg/){
		$base->logLocal("got forw $1\n", 2);
		if (! exists $forw{$1}){
		    $forw{$1} = $sid;
		    $seqinsert{$sid} = $1;
		}
		last;
	    }
	    if ($nm =~ /$revreg/){
		$base->logLocal("got rev $1\n", 2);
		if (! exists $rev{$1}){
		    $rev{$1} = $sid;
		    $seqinsert{$sid} = $1;
		}
		last;
	    }
	} # for each pairreg
    } # while each %seqids
    
    while (my ($ins, $nm) = each %forw) {
	if (! exists $seenlib{$ins}){
	    my $found = 0;
	    
	    $nm = $seqnames{$nm};

	    for (my $l = 0; $l <= $#libregexp; $l++){
		$base->logLocal("Trying $libregexp[$l] on $nm\n", 2);
		if ($nm =~ /$libregexp[$l]/){
		    $base->logLocal("found $libids[$l]\n", 2);
		    $insertlib{$libids[$l]} .= "$ins ";
		    $seenlib{$ins} = $libids[$l];
		    $found = 1;
		    last;
		}
	    }
	    if ($found == 0){
		$base->logError("Cannot find library for \"$nm\"");
		next;
	    }
	}
    }
} # parseMateFile;


# CONTIG PARSING FUNCTIONS
#
# Each function parses either a file or a database table and
# fills in the following hashes:
# 
# %contigs - contig_ids and sizes
# %seqids - seq_name to seq_id
# %seqnames - seq_id to seq_name
# %seq_range - seq_id to seq_range 
# %asm_range - seq_id to asm_range as blank delimited string
# %seqcontig - seq_id to contig
# %contigcons - contig consensus for each contig



# Celera .asm
# populates %contigs, %asm_range, %seqcontig, %contigcons
# expects %seq_range to be populated
sub parseAsmFile {
    my $IN = shift;

    while (my $record = getCARecord($IN)){
	my ($type, $fields, $recs) = parseCARecord($record);
	if ($type eq "CCO"){
	    my $id = getCAId($$fields{acc});
	    my $contiglen = $$fields{len};

	    my @offsets; my $coord;

	    my $consensus = $$fields{cns};
	    my @consensus = split('\n', $consensus);
	    $consensus = join('', @consensus);

	    print TMPCTG "#$id\n";
	    print TMPCTG $$fields{cns};
	    print TMPCTG "#\n";
	    print TMPCTG $$fields{qlt};
	    
	    
	    $#offsets = length($consensus) - 1;

	    for (my $i = 0; $i < length($consensus); $i++){
		if (substr($consensus, $i, 1) ne "-"){
		    $coord++;
		} else {
		    $contiglen--;
		}
		$offsets[$i] = $coord;
	    }

	    $contigs{$id} = $contiglen;

	    for (my $i = 0; $i <= $#$recs; $i++){
		my ($sid, $sfs, $srecs) = parseCARecord($$recs[$i]);
		if ($sid eq "MPS"){
		    my $fid = getCAId($$sfs{mid});
		    my ($cll, $clr) = split(' ', $seq_range{$fid});

		    print TMPCTG "#$fid\n";
		    print TMPCTG $$sfs{del};
		    $seqcontig{$fid} = $id;
		    $contigseq{$id} .= "$fid ";
		    
		    my ($asml, $asmr) = split(',', $$sfs{pos});
		    $asm_range{$fid} = "$asml $asmr";
		}
	    }
	    print TMPCTG "#\n";
	} # if type eq CCO
    }
} # parseAsmFile

# TIGR .asm
sub parseTAsmFile {
    my $IN = shift;

    my $ctg; 
    my $len;
    my $sname;
    my $alend;
    my $arend;
    my $slend;
    my $srend;
    my $sid;
    my $consensus;
    while (<$IN>){
	if (/^sequence\s+(\w+)/){
	    $len = length($1);
	    $consensus = $1;
	    next;
	}
	if (/^asmbl_id\s+(\w+)/){
	    $ctg = $1;
	    $contigs{$ctg} = $len;  # here we assume that length 
                                    # was already computed
	    next;
	}
	if (/^seq_name\s+(\S+)/){
	    $sname = $1;
	    if (! exists $seqids{$sname}){
		$sid = $minSeqId++;
		$seqids{$sname} = $sid;
		$seqnames{$sid} = $sname;
	    } else {
		$sid = $seqids{$sname};
	    }

	    $seqcontig{$sid} = $ctg;
	    $contigseq{$ctg} .= "$sid ";
	    next;
	}
	if (/^asm_lend\s+(\d+)/){
	    $alend = $1 - 1; # 0 based
	    next;
	}
	if (/^asm_rend\s+(\d+)/){
	    $arend = $1;
	    next;
	}
	if (/^seq_lend\s+(\d+)/){
	    $slend = $1 - 1;
	    next;
	}
	if (/^seq_rend\s+(\d+)/){
	    $srend = $1;
	    next;
	}
	if (/^offset/){
	    $seq_range{$sid} = "$slend $srend";
	    $asm_range{$sid} = "$alend $arend";
	    next;
	}
    }
} # parseTasmFile


# New .ACE format
sub parseACEFile {
    my $IN = shift;
    
    my $ctg; 
    my $len;
    my $sname;
    my $alend;
    my $arend;
    my $slend;
    my $srend;
    my $sid;

    my $inContig = 0;
    my $inSequence = 0;

    my $contigName;
    my $contigLen;
    my $contigSeqs;
    my $seqName;
    my %offset;
    my %rc;
    my $seq;
    my @gaps;
    while (<$IN>){
	if (/^CO (\S+) (\d+) (\d+)/){
	    $contigName = $1;
	    $contigLen = $2;
	    $contigSeqs = $3;
	    $inContig = 1;
	    $seq = "";
	    %offset = ();
	    next;
	}
	if ($inContig && /^\s*$/){
	    $inContig = 0;
	    $seq =~ s/\*/-/g;
	    @gaps = (); 
	    my $gap  = index($seq, "-");
	    while ($gap != -1){
		push(@gaps, $gap + 1);
		$gap = index($seq, "-", $gap + 1);
	    }
	    $contigs{$contigName} = $contigLen;
	    
	    next;
	}
	if ($inSequence && $_ =~ /^\s*$/){
	    $inSequence = 0;
	    next;
	}

	if ($inContig || $inSequence) {
	    chomp;
	    $seq .= $_;
	    next;
	}

	
	if (/^AF (\S+) (\w) (-?\d+)/){
	    $offset{$1} = $3;
	    $rc{$1} = $2;
	    next;
	}
	
	if (/^RD (\S+)/){
	    $inSequence = 1;
	    $seqName = $1;
	    $seq = "";
	    next;
	}

	if (/^QA -?(\d+) -?(\d+) (\d+) (\d+)/){
	    my $offset = $offset{$seqName};
	    my $cll = $3;
	    my $clr = $4;
	    my $end5 = $1;
	    my $end3 = $2;
	    $seq =~ s/\*/-/g;
	    my $len = length($seq);
	    $offset += $cll - 2;
	    $seq = substr($seq, $cll - 1, $clr - $cll + 1);
	    
	    my $i = 0;
	    my $asml = $offset;
	    my $asmr = $asml + $clr - $cll + 1;
	    while ($i <= $#gaps && $offset > $gaps[$i]){
		$asml--; $asmr--; $i++;
	    } # get rid of gaps from offset here
	    while ($i <= $#gaps && $offset + $clr - $cll + 1 > $gaps[$i]){
		$asmr--; $i++;
	    }

	    if ($rc{$seqName} eq "C"){ # make coordinates with respect to forw strand
		$cll = $len - $cll + 1;
		$clr = $len - $clr + 1;
		my $tmp = $cll;
		$cll = $clr;
		$clr = $tmp;
	    }

	    while ($seq =~ /-/g){ #make $clr ungapped
		$clr--;
	    }

	    if ($rc{$seqName} eq "C"){
		my $tmp = $cll;
		$cll = $clr;
		$clr = $tmp;
	    }
        
	    my $seqId;
	    if (! exists $seqids{$seqName}){
		$seqId = $minSeqId++;
		$seqids{$seqName} = $seqId;
		$seqnames{$seqId} = $seqName;
	    } else {
		$seqId = $seqids{$seqName};
	    }
	    $seqcontig{$seqId} = $contigName;
	    $contigseq{$contigName} .= "$seqId ";
	    $seq_range{$seqId} = "$cll $clr";
	    $asm_range{$seqId} = "$asml $asmr";
	    next;
	}
    } # while <$IN>
} #parseAceFile



# TIGR .contig file
sub parseContigFile {
    my $IN = shift;

    my $ctg; 
    my $len;
    my $sname;
    my $alend;
    my $arend;
    my $slend;
    my $srend;
    my $sid;
    my $incontig = 0;
    my $consensus = "";
    while (<$IN>){
	if (/^\#\#(\S+) \d+ (\d+)/ ){
	    if (defined $consensus){
		$consensus =~ s/-//g;
	    }
	    $consensus = "";
	    $ctg = $1;
	    $contigs{$ctg} = $2;
	    $incontig = 1;
	    next;
	}

	if (/^\#(\S+)\(\d+\) .*\{(\d+) (\d+)\} <(\d+) (\d+)>/){
	    $incontig = 0;
	    $sname = $1;
	    if (! exists $seqids{$sname}){
		$sid = $minSeqId++;
		$seqids{$sname} = $sid;
		$seqnames{$sid} = $sname;
	    } else {
		$sid = $seqids{$sname};
	    }
	    $seqcontig{$sid} = $ctg;
	    $contigseq{$ctg} .= "$sid ";
#	    print STDERR "adding $sname to $ctg\n";
	    $alend = $4 - 1;
	    $arend = $5;
	    $slend = $2 - 1;
	    $srend = $3;
	    $seq_range{$sid} = "$slend $srend";
	    $asm_range{$sid} = "$alend $arend";
	    next;
	}

	if ($incontig){
	    # here I try to get rid of dashes when computing contig sizes
	    my $ind = -1;
	    while (($ind = index($_ ,"-", $ind + 1)) != -1){
		$contigs{$ctg}--;
	    }
	    chomp;
	    $consensus .= $_;
	}
    }
    if (defined $consensus){
	$consensus =~ s/-//g;
    }

} # parseContigFile


###############################################################
# XML parser functions
###############################################################
sub StartDocument
{
#    print "starting\n";
}

sub EndDocument
{
#    print "done\n";
}

sub StartTag
{
    $tag = lc($_[1]);
    
    if ($tag eq "trace"){
        $library = undef;
        $template = undef;
        $clipl = undef;
        $clipr = undef;
        $mean = undef;
        $stdev = undef;
        $end = undef;
        $seqId = undef;
    }
}


sub EndTag
{
    $tag = lc($_[1]);
    if ($tag eq "trace"){
        if (! defined $seqId){
            $base->logError("trace has no name???\n");
        }
        if (! defined $library){
            $base->logError("trace $seqId has no library\n");
        }
        if (! defined $mean){
            $base->logError("library $library has no mean\n");
        } 
        
        if (! defined $stdev){
            $base->logError("library $library has no stdev\n");
        }

	if (defined $mean and defined $stdev){
	    $libraries{$library} = "$mean $stdev";
        }

        if (! defined $template){
            $base->logError("trace $seqId has no template\n");
        } 
        
        if (! defined $end) {
            $base->logError("trace $seqId has no end\n");
        }
        
        if ($end eq "R"){
            if (! exists $rev{$template} ||
                $seqnames{$seqId} gt $seqnames{$rev{$template}}){
                $rev{$template} = $seqId;
            }
        }
	 
        if ($end eq "F"){
            if (! exists $forw{$template} ||
                $seqnames{$seqId} gt $seqnames{$forw{$template}}){
                $forw{$template} = $seqId;
            }
        }
	    
	$seqinsert{$seqId} = $template;
	$insertlib{$library} .= "$template ";
	$seenlib{$template} = $library;
	
    
        if (defined $clipl && defined $clipr){
	    $seq_range{$seqId} = "$clipl $clipr";
        }
    }

    $tag = undef;
}


sub Text 
{
    if (defined $tag){
        if ($tag eq "insert_size"){
            $mean = $_;
        } elsif ($tag eq "insert_stdev"){
            $stdev = $_;
        } elsif ($tag eq "trace_name"){
            my $seqName = $_;
	    $seqId = $minSeqId++;
	    $seqids{$seqName} = $seqId;
	    $seqnames{$seqId} = $seqName;
        } elsif ($tag eq "library_id"){
            $library = $_;
        } elsif ($tag eq "seq_lib_id") {
            if (! defined $library) {
                $library = $_;
            }
        } elsif ($tag eq "template_id"){
            $template = $_;
        } elsif ($tag eq "trace_end"){
            $end = $_;
        } elsif ($tag eq "clip_quality_left" ||
                 $tag eq "clip_vector_left"){
            if (! defined $clipl || $_ > $clipl){
                $clipl = $_;
            }
        } elsif ($tag eq "clip_quality_right" ||
                 $tag eq "clip_vector_right"){
            if (! defined $clipr || $_ < $clipr){
                $clipr = $_;
            }
        }
    }
}

sub pi
{

}
