#include "foundation_AMOS.hh"
#include "amp.hh"
#include "fasta.hh"

using namespace AMOS;
using namespace std;

bool OPT_LayoutOnly = 0;
bool OPT_SimpleLayout = false;
bool OPT_UseEIDs = 0;
bool OPT_UseIIDs = 0;
bool OPT_Trapper = 0;
string OPT_BankName;

string OPT_EIDFile;
string OPT_IIDFile;

void PrintUsage (const char * s)
{
  cerr << "\nUSAGE: " << s << "  [options]  <bank path>\n\n";
}

//------------------------------------------------------------- PrintHelp ----//
void PrintHelp (const char * s)
{
  PrintUsage(s);
  cerr << "-h            Display help information\n"
       << "-v            Display the compatible bank version\n"
       << "-e            Use EIDs for names (DEFAULT)\n"
       << "-i            Use IIDs for names\n"
       << "-E file       Dump just the contig eids listed in file\n"
       << "-I file       Dump just the contig iids listed in file\n"
       << "-L            Just create a layout file (no sequence)\n"
       << "-S            Simple Layout style\n"
       << "-T            XML Format suitable for DNPTrapper\n"
       << endl;
  
  cerr << "Takes an AMOS bank directory and dumps the contigs to stdout\n\n";
}

void ParseArgs (int argc, char ** argv)
{
  int ch, errflg = 0;
  optarg = NULL;

  while ( !errflg && ((ch = getopt (argc, argv, "hveiTLSE:I:")) != EOF) )
  {
    switch (ch)
    {
      case 'h':
        PrintHelp (argv[0]);
        exit (EXIT_SUCCESS);
        break;

      case 'v':
        PrintBankVersion (argv[0]);
        exit (EXIT_SUCCESS);
      break;

      case 'e': OPT_UseEIDs = true;      break;
      case 'i': OPT_UseIIDs = true;      break;
      case 'L': OPT_LayoutOnly = true;   break;
      case 'E': OPT_EIDFile = optarg;    break;
      case 'I': OPT_IIDFile = optarg;    break;
      case 'S': OPT_SimpleLayout = true; break;
      case 'T': OPT_Trapper = true;      break;

      default: errflg ++;
      }
  }

  if ( errflg > 0 || optind != argc - 1 )
  {
      PrintUsage (argv[0]);
      cerr << "Try '" << argv[0] << " -h' for more information.\n";
      exit (EXIT_FAILURE);
  }

  OPT_BankName = argv [optind];

  if (!OPT_UseEIDs && !OPT_UseIIDs) { OPT_UseEIDs = true; }
}

bool firstContig = true;


void printContig(Contig_t & contig, Bank_t & read_bank)
{

  Read_t read; 
  std::vector<Tile_t> & tiling = contig.getReadTiling();
  sort(tiling.begin(), tiling.end(), TileOrderCmp());

  if (OPT_Trapper)
  {
    if (firstContig) { cout << "<TRAPPER>" << endl; firstContig = false;}

    printf("<contig name=\"%s\">\n", contig.getEID().c_str());

    int row = 0;

    // do a quick first pass to figure out how much the reads need to be shifted to ensure no one goes negative
    int leftmost = 0;
    vector<Tile_t>::const_iterator ti;
    for (ti = tiling.begin(); ti != tiling.end(); ti++)
    {
      Range_t clr = ti->range;
      int lefttrim = clr.getLo();

      if (clr.isReverse())
      {
        Read_t read;
        read_bank.fetch(ti->source, read);

        lefttrim = read.getLength() - clr.getHi();
      }

      if (ti->offset-lefttrim < leftmost)
      {
        leftmost = ti->offset - lefttrim;
      }
    }

    for (ti = tiling.begin(); ti != tiling.end(); ti++)
    {
      row++;
      bool rc = 0;
      Range_t range = ti->range;
      Range_t clr = range;
      Pos_t gappedLen = ti->getGappedLength();

      if (ti->range.begin > ti->range.end) { rc = 1; } 

      Read_t read;
      read_bank.fetch(ti->source, read);
      string sequence = read.getSeqString();
      string qual     = read.getQualString();

      if (rc) { range.swap(), Reverse_Complement(sequence);  reverse(qual.begin(), qual.end()); }

      Pos_t gapcount = 0;

      int origlen = sequence.length();

      int lefttrim = clr.getLo();
      int righttrim = origlen - clr.getHi();

      if (rc)
      {
        int t = lefttrim;
        lefttrim = righttrim;
        righttrim = t;
      }

      int beginGood = lefttrim;
      int endGood = lefttrim + ti->getGappedLength()-1;
      int gappedseqlen = qual.length();

      vector<Pos_t>::const_iterator g;
      for (g  = ti->gaps.begin();
           g != ti->gaps.end();
           g++)
      {
        sequence.insert(lefttrim+*g+gapcount, "*", 1);
        qual.insert(lefttrim+*g+gapcount, "0", 1);
        gapcount++;
        gappedseqlen++;
      }

      int OPT_TrapperFull = 0;
      if (OPT_TrapperFull)
      {
        if (rc) { beginGood = 0; }
        else    { endGood = gappedseqlen; }
      }

      printf("<ReadData row=\"%d\" name=\"%s\" startPos=\"%d\" endPos=\"%d\" strand=\"%c\" beginGood=\"%d\" endGood=\"%d\">\n",
             row, read_bank.lookupEID(ti->source).c_str(), ti->offset-lefttrim-leftmost, ti->getRightOffset()+righttrim-leftmost, (rc ? 'C' : 'U'), beginGood, endGood);

      printf("<DnaStrData startPos=\"%d\" endPos=\"%d\" trappervector=\"%s\"/>\n",
             0, gappedseqlen, sequence.c_str());

      printf("<QualityData startPos=\"%d\" endPos=\"%d\" trappervector=\"", 
             0, gappedseqlen);
 
      for (int i = 0; i < gappedseqlen; i++)
      {
        printf(" %d", qual[i]-'0');
      }
      printf("\"/>\n");

      printf("<ChromatData startPos=\"%d\" endPos=\"%d\"/>\n",  
             0, gappedseqlen);

      printf("</ReadData>\n");
    }

    printf("</contig>\n");
  }
  else if (OPT_SimpleLayout)
  {
    string contigeid = contig.getEID();

    vector<Tile_t>::const_iterator ti;
    for (ti = tiling.begin(); ti != tiling.end(); ti++)
    {
      cout << contigeid << "\t"
           << read_bank.lookupEID(ti->source) << "\t"
           << (ti->range.isReverse() ? 1 : 0) << "\t"
           << ti->offset
           << endl;
    }
  }
  else
  {
    cout << "##";

    if (OPT_UseEIDs) 
    { 
      string s(contig.getEID());
      int i = s.find(' ');
      if (i != s.npos) { s = s.substr(0,i); }
      if (s.empty()) { cout << contig.getIID(); }
      else           { cout << s; }
    }
    else 
    { 
      cout << contig.getIID(); 
    }

    const string cons = contig.getSeqString();

    cout << " "  << tiling.size()
         << " "  << cons.length()
         << " bases, 00000000 checksum." << endl;

    if (!OPT_LayoutOnly)
    {
      Fasta_Print(stdout, cons.c_str(), NULL, 60);
    }

    vector<Tile_t>::const_iterator i;
    for (i =  tiling.begin();
         i != tiling.end();
         i++)
    {
      bool rc = 0;
      Range_t range = i->range;
      Range_t clr = range;
      Pos_t gappedLen = i->getGappedLength();

      if (i->range.begin > i->range.end) { clr.end++; rc = 1; } 
      else                               { clr.begin++; }

      cout << "#";

      if (OPT_UseEIDs) 
      { 
        string s = read_bank.lookupEID(i->source);
        int i = s.find(' ');
        if (i != s.npos) { s = s.substr(0,i); }
        cout << s;
      }
      else 
      { 
        cout << i->source;
      }

      cout << "(" << i->offset 
           << ((rc) ? ") [RC] " : ") [] ") << gappedLen
           << " bases, 00000000 checksum."
           << " {" << clr.begin << " " << clr.end << "}"
           << " <" << contig.gap2ungap(i->offset)
           << " "  << contig.gap2ungap(i->getRightOffset())
           << ">"  << endl;

      if (!OPT_LayoutOnly)
      {
        read_bank.fetch(i->source, read);
        if (rc) { range.swap(); }
        string sequence = read.getSeqString(range);
        if (rc) { Reverse_Complement(sequence); }

        Pos_t gapcount = 0;

        vector<Pos_t>::const_iterator g;
        for (g  = i->gaps.begin();
             g != i->gaps.end();
             g++)
        {
          sequence.insert(*g+gapcount, "-", 1);
          gapcount++;
        }


        Fasta_Print(stdout, sequence.c_str(), NULL, 60);
      }
    }
  }
}

int main (int argc, char ** argv)
{
  ParseArgs (argc, argv);

  BankStream_t contig_bank(Contig_t::NCODE);
  Bank_t read_bank(Read_t::NCODE);

  cerr << "Processing " << OPT_BankName << " at " << Date() << endl;

  try
  {
    read_bank.open(OPT_BankName, B_READ);
    contig_bank.open(OPT_BankName, B_READ);

    Contig_t contig;
    ifstream file;
    string id;

    if (!OPT_EIDFile.empty())
    {
      file.open(OPT_EIDFile.c_str());
      
      if (!file)
      {
        throw Exception_t("Couldn't open EID File", __LINE__, __FILE__);
      }

      while (file >> id)
      {
        contig_bank.seekg(contig_bank.getIDMap().lookupBID(id));
        contig_bank >> contig;
        printContig(contig, read_bank);
      }
    }
    else if (!OPT_IIDFile.empty())
    {
      file.open(OPT_IIDFile.c_str());

      if (!file)
      {
        throw Exception_t("Couldn't open IID File", __LINE__, __FILE__);
      }

      while (file >> id)
      {
        contig_bank.seekg(contig_bank.getIDMap().lookupBID(atoi(id.c_str())));
        contig_bank >> contig;
        printContig(contig, read_bank);
      }
    }
    else
    {
      while (contig_bank >> contig)
      {
        printContig(contig, read_bank);
      }
    }

    read_bank.close();
    contig_bank.close();

    if (OPT_Trapper && !firstContig)
    {
      printf("</TRAPPER>\n");
    }
  }
  catch (Exception_t & e)
  {
    cerr << "ERROR: -- Fatal AMOS Exception --\n" << e;
    return EXIT_FAILURE;
  }

  cerr << "End: " << Date() << endl;
  return EXIT_SUCCESS;
}
