#!/usr/bin/perl

## this program retrieves a collection of reads from the bank
## identifies those that belong to the same fragment, then builds
## layouts (using nucmer).

use lib "/usr/local/lib";
use AMOS::AmosLib;
use AMOS::Foundation;

my $END = 10; # end wobble allowed

my $base = new TIGR::Foundation;
if (! defined $base){
    die ("Foundation cannot be created.  Walk, do not run, to the nearest exit!\n");
}

my $VERSION = '$Revision$ ';
$base->setVersionInfo($VERSION);

my $HELPTEXT = q~
Usage: preassembleFrgs.pl [-poly] <bank>
    ~;

$base->setHelpInfo($HELPTEXT);

my $dopoly = undef;

my $err = $base->TIGR_GetOptions(
				 "poly=s" => \$dopoly
				 );

if (! $err){
    $base->bail("Error processing command line!");
}

my $bank = $ARGV[0];
my $temp = "TEMP";

my $amospath = $ENV{AMOSBIN};

my $report = (defined $amospath) ? "$amospath/bank-report" : "bank-report";
my $overlap = (defined $amospath) ? "$amospath/hash-overlap" : "hash-overlap";
my $tigger = (defined $amospath) ? "$amospath/tigger" : "tigger";
my $consensus;
if (defined $dopoly) {
    $consensus = (defined $amospath) ? "$amospath/make-consensus_poly" : "make-consensus_poly";
} else {
    $consensus = (defined $amospath) ? "$amospath/make-consensus" : "make-consensus";
}

my %frgrds;
my %rdidx;
my %clears;

open(TMP, "$report -b $bank RED |") || die ("Cannot report bank: $!\n");

while (my $record = getRecord(\*TMP)){
    my ($rec, $fields, $recs) = parseRecord($record);
    if ($rec eq "RED"){
	my $id = $$fields{iid};
	my $frg = $$fields{frg};
	if (! defined $frg) {next;} # only want reads in fragments
	$frgrds{$frg} .= "$id ";
    } # RED records
} # for each input record

close(TMP);

while (my ($frg, $seqs) = each %frgrds){
    my @seqs = split(" ", $seqs);
    if ($#seqs < 1){ next;} # fragments with at least two reads
    
    open(TMPSEQ, ">$temp.list") || die ("Cannot open $temp.list: $!\n");

    for ($i = 0; $i <= $#seqs; $i++){
	print TMPSEQ $seqs[$i], "\n";
    }
    
    close(TMPSEQ);
    
    system ("$overlap -B $bank -I $temp.list");

} # for each fragment

system("$tigger -b $bank");
system("$consensus -B -b $bank");
