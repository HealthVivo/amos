////////////////////////////////////////////////////////////////////////////////
//! \file
//! \author Adam M Phillippy
//! \date 10/23/2003
//!
//! \brief Source for Contig_t
//!
////////////////////////////////////////////////////////////////////////////////

#include "Contig_AMOS.hh"
using namespace AMOS;
using namespace std;




//================================================ Contig_t ====================
//----------------------------------------------------- getUngappedQualString --
string Contig_t::getUngappedQualString (Range_t range) const
{
  //-- Check preconditions
  if ( range . begin > range . end ||
       range . begin < 0 ||
       range . end > getLength( ) )
    AMOS_THROW_ARGUMENT ("range does not represent a valid substring");

  pair<char, char> seqqualc;
  string retval;
  retval . reserve (range . end - range . begin);

  //-- Skip the gaps in the sequence and populate the retval
  for ( Pos_t i = range . begin; i < range . end; i ++ )
    {
      seqqualc = getBase (i);
      if ( isalpha (seqqualc . first) )
	retval += seqqualc . second;
    }

  return retval;
}


//----------------------------------------------------- getUngappedSeqString ---
string Contig_t::getUngappedSeqString (Range_t range) const
{
  //-- Check preconditions
  if ( range . begin > range . end ||
       range . begin < 0 ||
       range . end > getLength( ) )
    AMOS_THROW_ARGUMENT ("range does not represent a valid substring");

  char seqc;
  string retval;
  retval . reserve (range . end - range . begin);

  //-- Skip the gaps in the sequence and populate the retval
  for ( Pos_t i = range . begin; i < range . end; i ++ )
    {
      seqc = getBase (i) . first;
      if ( isalpha (seqc) )
	retval += seqc;
    }

  return retval;
}


//----------------------------------------------------- readRecord -------------
Size_t Contig_t::readRecord (istream & fix,
			     istream & var)
{
  Size_t streamsize = Sequence_t::readRecord (fix, var);
  Size_t size, tsize;

  //-- Read FIX data
  fix . read ((char *)&size, sizeof (Size_t));
  streamsize += sizeof (Size_t);
  fix . read ((char *)&poly_m, sizeof (null_t));
  streamsize += sizeof (null_t);

  //-- Read VAR data
  reads_m . resize (size);
  for ( Pos_t i = 0; i < size; i ++ )
    {
      var . read ((char *)&tsize, sizeof (Size_t));
      streamsize += sizeof (Size_t);
      reads_m [i] . gaps . resize (tsize);
      for ( Pos_t j = 0; j < tsize; j ++ )
	var . read ((char *)&(reads_m [i] . gaps [j]), sizeof (int32_t));
      streamsize += tsize * sizeof (int32_t);

      var . read ((char *)&reads_m [i] . id, sizeof (ID_t));
      streamsize += sizeof (ID_t);
      var . read ((char *)&reads_m [i] . offset, sizeof (Pos_t));
      streamsize += sizeof (Pos_t);
      var . read ((char *)&reads_m [i] . range, sizeof (Range_t));
      streamsize += sizeof (Range_t);
    }

  return streamsize;
}


//----------------------------------------------------- writeRecord ------------
Size_t Contig_t::writeRecord (ostream & fix,
			      ostream & var) const
{
  Size_t streamsize = Sequence_t::writeRecord (fix, var);
  Size_t size, tsize;

  //-- Write FIX data
  size = reads_m . size( );
  fix . write ((char *)&size, sizeof (Size_t));
  streamsize += sizeof (Size_t);
  fix . write ((char *)&poly_m, sizeof (null_t));
  streamsize += sizeof (null_t);

  //-- Write VAR data
  for ( Pos_t i = 0; i < size; i ++ )
    {
      tsize = reads_m [i] . gaps . size( );
      var . write ((char *)&tsize, sizeof (Size_t));
      streamsize += sizeof (Size_t);
      for ( Pos_t j = 0; j < tsize; j ++ )
	var . write ((char *)&(reads_m [i] . gaps [j]), sizeof (int32_t));
      streamsize += tsize * sizeof (int32_t);

      var . write ((char *)&reads_m [i] . id, sizeof (ID_t));
      streamsize += sizeof (ID_t);
      var . write ((char *)&reads_m [i] . offset, sizeof (Pos_t));
      streamsize += sizeof (Pos_t);
      var . write ((char *)&reads_m [i] . range, sizeof (Range_t));
      streamsize += sizeof (Range_t);
    }

  return streamsize;
}
