////////////////////////////////////////////////////////////////////////////////
//! \file
//! \author Adam M Phillippy
//! \date 10/23/2003
//!
//! \brief Source for Fragment_t
//!
////////////////////////////////////////////////////////////////////////////////

#include "Fragment_AMOS.hh"
using namespace AMOS;
using namespace Message_k;
using namespace std;




//================================================ Fragment_t ==================
//----------------------------------------------------- readMessage-------------
void Fragment_t::readMessage (const Message_t & msg)
{
  clear( );
  Universal_t::readMessage (msg);

  try {
    ID_t iid;
    stringstream ss;

    if ( msg . exists (F_LIBRARY) )
      {
	ss . str (msg . getField (F_LIBRARY));
	ss >> library_m;
	if ( !ss )
	  AMOS_THROW_ARGUMENT ("Invalid lib format");
      }

    if ( msg . exists (F_5PRIME) )
      {
	ss . str (msg . getField (F_5PRIME));
	ss >> ends_m . first;
	if ( !ss )
	  AMOS_THROW_ARGUMENT ("Invalid 5pr format");
      }

    if ( msg . exists (F_3PRIME) )
      {
	ss . str (msg . getField (F_3PRIME));
	ss >> ends_m . second;
	if ( !ss )
	  AMOS_THROW_ARGUMENT ("Invalid 3pr format");
      }

    if ( msg . exists (F_TYPE) )
      {
        ss . str (msg . getField (F_TYPE));
        setType (ss . get( ));
      }

    if ( msg . exists (F_READS) )
      {
	ss . str (msg . getField (F_READS));

	while ( ss )
	  {
	    ss >> iid;
	    if ( ! ss . fail( ) )
	      reads_m . push_back (iid);
	  }

	if ( !ss . eof( ) )
	  AMOS_THROW_ARGUMENT ("Invalid rds format");
      }
  }
  catch (ArgumentException_t) {
    
    clear( );
    throw;
  }
}


//----------------------------------------------------- readRecord -------------
Size_t Fragment_t::readRecord (istream & fix,
			       istream & var)
{
  Size_t streamsize = Universal_t::readRecord (fix, var);
  Size_t size;

  //-- Read FIX data
  fix . read ((char *)&size, sizeof (Size_t));
  streamsize += sizeof (Size_t);
  fix . read ((char *)&ends_m, sizeof (pair<ID_t, ID_t>));
  streamsize += sizeof (pair<ID_t, ID_t>);
  fix . read ((char *)&library_m, sizeof (ID_t));
  streamsize += sizeof (ID_t);
  fix . read ((char *)&type_m, sizeof (FragmentType_t));
  streamsize += sizeof (FragmentType_t);

  //-- Read VAR data
  reads_m . resize (size, NULL_ID);
  for ( Pos_t i = 0; i < size; i ++ )
    var . read ((char *)&reads_m [i], sizeof (ID_t));
  streamsize += size * sizeof (ID_t);

  return streamsize;
}


//----------------------------------------------------- writeMessage -----------
void Fragment_t::writeMessage (Message_t & msg) const
{
  Universal_t::writeMessage (msg);

  try {
    vector<ID_t>::const_iterator vi;
    stringstream ss;

    msg . setMessageCode (NCode( ));

    if ( library_m != NULL_ID )
      {
        ss << library_m;
        msg . setField (F_LIBRARY, ss . str( ));
        ss . str("");
      }

    if ( ends_m . first != NULL_ID )
      {
	ss << ends_m . first;
	msg . setField (F_5PRIME, ss . str( ));
	ss . str("");
      }

    if ( ends_m . second != NULL_ID )
      {
	ss << ends_m . second;
	msg . setField (F_3PRIME, ss . str( ));
	ss . str("");
      }

    if ( type_m != NULL_FRAGMENT )
      {
        ss << type_m;
        msg . setField (F_TYPE, ss . str( ));
        ss . str("");
      }

    if ( reads_m . size( ) != 0 )
      {
	for ( vi = reads_m . begin( ); vi != reads_m . end( ); vi ++ )
	  ss << *vi << '\n';
	msg . setField (F_READS, ss . str( ));
	ss . str("");
      }
  }
  catch (ArgumentException_t) {

    msg . clear( );
    throw;
  }
}


//----------------------------------------------------- writeRecord ------------
Size_t Fragment_t::writeRecord (ostream & fix,
				ostream & var) const
{
  Size_t streamsize = Universal_t::writeRecord (fix, var);
  Size_t size = reads_m . size( );

  //-- Write FIX data
  fix . write ((char *)&size, sizeof (Size_t));
  streamsize += sizeof (Size_t);
  fix . write ((char *)&ends_m, sizeof (pair<ID_t, ID_t>));
  streamsize += sizeof (pair<ID_t, ID_t>);
  fix . write ((char *)&library_m, sizeof (ID_t));
  streamsize += sizeof (ID_t);
  fix . write ((char *)&type_m, sizeof (FragmentType_t));
  streamsize += sizeof (FragmentType_t);

  //-- Write VAR data
  for ( Pos_t i = 0; i < size; i ++ )
    var . write ((char *)&reads_m [i], sizeof (ID_t));
  streamsize += size * sizeof (ID_t);

  return streamsize;
}
