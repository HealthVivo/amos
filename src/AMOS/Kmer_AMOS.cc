////////////////////////////////////////////////////////////////////////////////
//! \file
//! \author Adam M Phillippy
//! \date 10/29/2003
//!
//! \brief Source for Kmer_t
//!
////////////////////////////////////////////////////////////////////////////////

#include "Kmer_AMOS.hh"
using namespace AMOS;
using namespace std;




//================================================ Kmer_t ======================
const NCode_t Kmer_t::NCODE = M_KMER;
const uint8_t Kmer_t::MAX_LENGTH    = 255;
const uint8_t Kmer_t::ADENINE_BITS  = 0x0;
const uint8_t Kmer_t::CYTOSINE_BITS = 0x40;
const uint8_t Kmer_t::GUANINE_BITS  = 0x80;
const uint8_t Kmer_t::THYMINE_BITS  = 0xC0;
const uint8_t Kmer_t::SEQ_BITS      = 0xC0;


//----------------------------------------------------- compress ---------------
inline uint8_t Kmer_t::compress (char seqchar)
{
  switch ( toupper(seqchar) )
    {
    case 'A':
      return ADENINE_BITS;
    case 'C':
      return CYTOSINE_BITS;
    case 'G':
      return GUANINE_BITS;
    case 'T':
      return THYMINE_BITS;
    default:
      AMOS_THROW_ARGUMENT ((std::string)"Invalid Kmer character " + seqchar);
    }
}


//----------------------------------------------------- uncompress -------------
inline char Kmer_t::uncompress (uint8_t byte)
{
  switch ( byte & SEQ_BITS )
    {
    case ADENINE_BITS:
      return 'A';
    case CYTOSINE_BITS:
      return 'C';
    case GUANINE_BITS:
      return 'G';
    case THYMINE_BITS:
      return 'T';
    default:
      AMOS_THROW ("Unknown logic error");
    }
}


//----------------------------------------------------- getSeqString -----------
string Kmer_t::getSeqString ( ) const
{
  string retval;
  retval . reserve (length_m);

  //-- See comments for getBase regarding the compressed sequence
  Pos_t ci = -1;
  uint8_t byte = 0;
  for ( Pos_t ui = 0; ui < length_m; ui ++ )
    {
      if ( ui % 4 == 0 )
	byte = seq_m [++ ci];

      retval += uncompress (byte);
      byte <<= 2;
    }

  return retval;
}


//----------------------------------------------------- readMessage-------------
void Kmer_t::readMessage (const Message_t & msg)
{
  clear( );
  Universal_t::readMessage (msg);

  try {

    istringstream ss;

    if ( msg . exists (F_COUNT) )
      {
	ss . str (msg . getField (F_COUNT));
	ss >> count_m;
	if ( !ss )
	  AMOS_THROW_ARGUMENT ("Invalid count format");
	ss . clear( );
      }

    if ( msg . exists (F_SEQUENCE) )
      setSeqString (msg . getField (F_SEQUENCE));

    if ( msg . exists (F_READS) )
      {
	ID_t iid;

	ss . str (msg . getField (F_READS));

	while ( ss )
	  {
	    ss >> iid;
	    if ( ! ss . fail( ) )
	      reads_m . push_back (iid);
	  }

	if ( !ss . eof( ) )
	  AMOS_THROW_ARGUMENT ("Invalid read link list format");
	ss . clear( );
      }
  }
  catch (ArgumentException_t) {
    
    clear( );
    throw;
  }
}


//----------------------------------------------------- readRecord -------------
void Kmer_t::readRecord (istream & fix,
			 istream & var)
{
  Size_t size;

  //-- Read parent object data
  Universal_t::readRecord (fix, var);

  //-- Read object data
  fix . read ((char *)&count_m, sizeof (uint32_t));
  fix . read ((char *)&size, sizeof (Size_t));
  fix . read ((char *)&length_m, sizeof (uint8_t));

  reads_m . resize (size, NULL_ID);
  for ( Pos_t i = 0; i < size; i ++ )
    var . read ((char *)&reads_m [i], sizeof (ID_t));

  size = length_m / 4 + (length_m % 4 ? 1 : 0);
  seq_m = (uint8_t *) SafeRealloc (seq_m, size);
  var . read ((char *)seq_m, size);
}


//----------------------------------------------------- setSeqString -----------
void Kmer_t::setSeqString (const string & seq)
{
  Size_t osize = seq . size( );
  Size_t size = osize;
  if ( size > Kmer_t::MAX_LENGTH )
    AMOS_THROW_ARGUMENT ("Invalid kmer sequence is too long");

  size = size / 4 + (size % 4 ? 1 : 0);
  seq_m = (uint8_t *) SafeRealloc (seq_m, size);

  //-- See comments for setBase regarding the compressed sequence
  Pos_t ci = -1;
  int offset = 8;
  length_m = 0;
  for ( Size_t ui = 0; ui < osize; ui ++ )
    {
      if ( seq [ui] == NL_CHAR )
	continue;

      length_m ++;
      if ( offset >= 8 )
	{
	  offset = 0;
	  seq_m [++ ci] = 0;
	}
      seq_m [ci] |= compress (seq [ui]) >> offset;
      offset += 2;
    }

  if ( length_m != osize )
    {
      size = length_m / 4 + (length_m % 4 ? 1 : 0);
      seq_m = (uint8_t *) SafeRealloc (seq_m, size);
    }
}


//----------------------------------------------------- writeMessage -----------
void Kmer_t::writeMessage (Message_t & msg) const
{
  Universal_t::writeMessage (msg);

  try {

    ostringstream ss;

    msg . setMessageCode (Kmer_t::NCODE);

    ss << count_m;
    msg . setField (F_COUNT, ss . str( ));
    ss . str (NULL_STRING);

    if ( length_m != 0 )
      msg . setField (F_SEQUENCE, getSeqString( ));

    if ( !reads_m . empty( ) )
      {
	vector<ID_t>::const_iterator vi;

	for ( vi = reads_m . begin( ); vi != reads_m . end( ); vi ++ )
	  ss << *vi << endl;
	msg . setField (F_READS, ss . str( ));
	ss . str (NULL_STRING);
      }
  }
  catch (ArgumentException_t) {

    msg . clear( );
    throw;
  }
}


//----------------------------------------------------- writeRecord ------------
void Kmer_t::writeRecord (ostream & fix,
			  ostream & var) const
{
  Size_t size = reads_m . size( );

  //-- Write parent object data
  Universal_t::writeRecord (fix, var);

  //-- Write the object data
  fix . write ((char *)&count_m, sizeof (uint32_t));
  fix . write ((char *)&size, sizeof (Size_t));
  fix . write ((char *)&length_m, sizeof (uint8_t));

  for ( Pos_t i = 0; i < size; i ++ )
    var . write ((char *)&reads_m [i], sizeof (ID_t));

  size = length_m / 4 + (length_m % 4 ? 1 : 0);
  var . write ((char *)seq_m, size);
}


//----------------------------------------------------- operator= --------------
Kmer_t & Kmer_t::operator= (const Kmer_t & source)
{
  if ( this != &source )
    {
      //-- Make sure parent data is copied
      Universal_t::operator= (source);

      Size_t size = source . length_m / 4 + (source . length_m % 4 ? 1 : 0);

      //-- Copy object data
      seq_m = (uint8_t *) SafeRealloc (seq_m, size);
      memcpy (seq_m, source . seq_m, size);

      count_m = source . count_m;
      length_m = source . length_m;
      reads_m = source . reads_m;
    }

  return *this;
}
