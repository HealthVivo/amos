////////////////////////////////////////////////////////////////////////////////
//! \file
//! \author Adam M Phillippy
//! \date 10/23/2003
//!
//! \brief Source for Overlap_t
//!
////////////////////////////////////////////////////////////////////////////////

#include "Overlap_AMOS.hh"
using namespace AMOS;
using namespace std;




//================================================ Overlap_t ===================
const NCode_t Overlap_t::NCODE = M_OVERLAP;
const OverlapAdjacency_t Overlap_t::NULL_ADJACENCY = 0;
const OverlapAdjacency_t Overlap_t::NORMAL     = 'N';
const OverlapAdjacency_t Overlap_t::ANTINORMAL = 'A';
const OverlapAdjacency_t Overlap_t::INNIE      = 'I';
const OverlapAdjacency_t Overlap_t::OUTIE      = 'O';
const uint8_t Overlap_t::FIRST_BIT  = 0x1;
const uint8_t Overlap_t::SECOND_BIT = 0x2;
const uint8_t Overlap_t::FLAGC_BIT  = 0x4;
const uint8_t Overlap_t::FLAGD_BIT  = 0x8;


//----------------------------------------------------- getAdjacency -----------
OverlapAdjacency_t Overlap_t::getAdjacency ( ) const
{
  //-- first and second adjacency information is stored respectively in bits
  //   0x1 and 0x2. A 0 bit means 'B' and a 1 bit means 'E'. If 0x4 = 0, then
  //   no adjacency information exists.
  if ( flags_m . nibble & 0x4 )
    {
      if ( flags_m . nibble & 0x1 )
	{
	  if ( flags_m . nibble & 0x2 )
	    return INNIE;
	  else
	    return NORMAL;
	}
      else
	{
	  if ( flags_m . nibble & 0x2 )
	    return ANTINORMAL;
	  else
	    return OUTIE;
	}
    }
  return NULL_ADJACENCY;
}


//----------------------------------------------------- readMessage ------------
void Overlap_t::readMessage (const Message_t & msg)
{
  clear( );
  Universal_t::readMessage (msg);

  try {
    istringstream ss;

    if ( msg . exists (F_READ1) )
      {
	ss . str (msg . getField (F_READ1));
	ss >> reads_m . first;
	if ( !ss )
	  AMOS_THROW_ARGUMENT ("Invalid read1 link format");
	ss . clear( );
      }

    if ( msg . exists (F_READ2) )
      {
	ss . str (msg . getField (F_READ2));
	ss >> reads_m . second;
	if ( !ss )
	  AMOS_THROW_ARGUMENT ("Invalid read2 link format");
	ss . clear( );
      }

    if ( msg . exists (F_ADJACENCY) )
      {
        ss . str (msg . getField (F_ADJACENCY));
        setAdjacency (ss . get( ));
	ss . clear( );
      }

    if ( msg . exists (F_AHANG) )
      {
	ss . str (msg . getField (F_AHANG));
	ss >> aHang_m;
	if ( !ss )
	  AMOS_THROW_ARGUMENT ("Invalid aHang format");
	ss . clear( );
      }

    if ( msg . exists (F_BHANG) )
      {
	ss . str (msg . getField (F_BHANG));
	ss >> bHang_m;
	if ( !ss )
	  AMOS_THROW_ARGUMENT ("Invalid bHang format");
	ss . clear( );
      }

    if ( msg . exists (F_SCORE) )
      {
	ss . str (msg . getField (F_SCORE));
	ss >> score_m;
	if ( !ss )
	  AMOS_THROW_ARGUMENT ("Invalid score format");
	ss . clear( );
      }
  }
  catch (ArgumentException_t) {
    
    clear( );
    throw;
  }
}


//----------------------------------------------------- readRecord -------------
void Overlap_t::readRecord (istream & fix,
			    istream & var)
{
  //-- Read parent object data
  Universal_t::readRecord (fix, var);

  //-- Read object data
  fix . read ((char *)&aHang_m, sizeof (Size_t));
  fix . read ((char *)&bHang_m, sizeof (Size_t));
  fix . read ((char *)&reads_m, sizeof (pair<ID_t, ID_t>));
  fix . read ((char *)&score_m, sizeof (uint32_t));
}


//----------------------------------------------------- setAdjacency -----------
void Overlap_t::setAdjacency (OverlapAdjacency_t adj)
{
  //-- first and second adjacency information is stored respectively in bits
  //   0x1 and 0x2. A 0 bit means 'B' and a 1 bit means 'E'. If 0x4 = 0, then
  //   no adjacency information exists.
  switch (adj)
    {
    case NORMAL:
    case ANTINORMAL:
    case INNIE:
    case OUTIE:
      flags_m . nibble &= ~0x7;
      if ( adj == NORMAL || adj == INNIE )
	flags_m . nibble |= 0x1;
      if ( adj == INNIE || adj == ANTINORMAL )
	flags_m . nibble |= 0x2;
      flags_m . nibble |= 0x4;
      break;
    case NULL_ADJACENCY:
      flags_m . nibble &= ~0x7;
      break;
    default:
      AMOS_THROW_ARGUMENT ((string)"Invalid adjacency type " + adj);
    }
}


//----------------------------------------------------- writeMessage -----------
void Overlap_t::writeMessage (Message_t & msg) const
{
  Universal_t::writeMessage (msg);

  try {
    ostringstream ss;

    msg . setMessageCode (Overlap_t::NCODE);

    if ( reads_m . first != NULL_ID )
      {
	ss << reads_m . first;
	msg . setField (F_READ1, ss . str( ));
	ss . str (NULL_STRING);
      }

    if ( reads_m . second != NULL_ID )
      {
	ss << reads_m . second;
	msg . setField (F_READ2, ss . str( ));
	ss . str (NULL_STRING);
      }

    if ( getAdjacency( ) != NULL_ADJACENCY )
      {
        ss << getAdjacency( );
        msg . setField (F_ADJACENCY, ss . str( ));
        ss . str (NULL_STRING);
      }

    ss << aHang_m;
    msg . setField (F_AHANG, ss . str( ));
    ss . str (NULL_STRING);

    ss << bHang_m;
    msg . setField (F_BHANG, ss . str( ));
    ss . str (NULL_STRING);

    ss << score_m;
    msg . setField (F_SCORE, ss . str( ));
    ss . str (NULL_STRING);
  }
  catch (ArgumentException_t) {

    msg . clear( );
    throw;
  }
}


//----------------------------------------------------- writeRecord ------------
void Overlap_t::writeRecord (ostream & fix,
			     ostream & var) const
{
  //-- Write parent object data
  Universal_t::writeRecord (fix, var);

  //-- Write object data
  fix . write ((char *)&aHang_m, sizeof (Size_t));
  fix . write ((char *)&bHang_m, sizeof (Size_t));
  fix . write ((char *)&reads_m, sizeof (pair<ID_t, ID_t>));
  fix . write ((char *)&score_m, sizeof (uint32_t));
}
