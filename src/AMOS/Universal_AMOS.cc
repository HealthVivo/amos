////////////////////////////////////////////////////////////////////////////////
//! \file
//! \author Adam M Phillippy
//! \date 09/24/2003
//!
//! \brief Source for Universal_t
//!
////////////////////////////////////////////////////////////////////////////////

#include "Universal_AMOS.hh"
using namespace AMOS;
using namespace Message_k;
using namespace std;




//================================================ Universal_t =================
//----------------------------------------------------- readMessage ------------
void Universal_t::readMessage (const Message_t & msg)
{
  clear( );

  try {
    stringstream ss;

    if ( msg . exists (F_IID) )
      {
	ss . str (msg . getField (F_IID));
	ss >> iid_m;
	if ( !ss )
	  AMOS_THROW_ARGUMENT ("Invalid iid format");
      }    

    if ( msg . exists (F_EID) )
      setEID (msg . getField (F_EID));
    
    if ( msg . exists (F_COMMENT) )
      setComment (msg . getField (F_COMMENT));
  }
  catch (ArgumentException_t) {
    
    clear( );
    throw;
  }
}


//----------------------------------------------------- writeMessage -----------
void Universal_t::writeMessage (Message_t & msg) const
{
  msg . clear( );

  try {
    stringstream ss;

    msg . setMessageCode (NCode( ));

    if ( iid_m != NULL_ID )
      {
	ss << iid_m;
	msg . setField (F_IID, ss . str( ));
      }

    if ( eid_m . size( ) != 0 )
      msg . setField (F_EID, eid_m);
  
    if ( comment_m . size( ) != 0 )
      msg . setField (F_COMMENT, comment_m);
  }
  catch (ArgumentException_t) {

    msg . clear( );
    throw;
  }
}
