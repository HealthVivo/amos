////////////////////////////////////////////////////////////////////////////////
//! \file
//! \author Adam M Phillippy
//! \date 07/27/2004
//!
//! \brief Include file for AMOS utilities, #defines, inttypes and exceptions
//!
////////////////////////////////////////////////////////////////////////////////

#ifndef __utility_AMOS_HH
#define __utility_AMOS_HH 1

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "exceptions_AMOS.hh"
#include <new>
#include <cstdlib>
#include <cstring>





namespace AMOS {

//--------------------------------------------------- SafeCalloc ---------------
//! \brief Safe calloc wrapper
//!
//! Detects calloc failure and throws AMOS::AllocException_t exception instead
//! of returning a NULL pointer.
//!
//! \param num Number of elements to allocate
//! \param size Number of bytes per element
//! \post The memory is allocated
//! \throws AllocException_t on memory exhaustion
//! \return Pointer to the new memory block
//!
inline void * SafeCalloc (size_t num, size_t size)
{
  void * Q = calloc (num, size);
  if ( Q == NULL )
    AMOS_THROW_ALLOC ("calloc failed");

  return Q;
}


//--------------------------------------------------- SafeMalloc ---------------
//! \brief Safe malloc wrapper
//!
//! Detects malloc failure and throws AMOS::AllocException_t exception instead
//! of returning a NULL pointer.
//!
//! \param size Number of bytes to allocate
//! \post The memory is allocated
//! \throws AllocException_t on memory exhaustion
//! \return Pointer to the new memory block
//!
inline void * SafeMalloc (size_t size)
{
  void * Q = malloc (size);
  if ( Q == NULL )
    AMOS_THROW_ALLOC ("malloc failed");

  return Q;
}


//--------------------------------------------------- SafeRealloc --------------
//! \brief Safe realloc wrapper
//!
//! Detects realloc failure and throws AMOS::AllocException_t exception instead
//! of returning a NULL pointer. This helps prevent memory leaks by preventing
//! the original memory block from being lost when the realloc function returns
//! NULL.
//!
//! \param P The memory block to re-allocate
//! \param size Number of bytes to allocate
//! \post The memory is allocated
//! \throws AllocException_t on memory exhaustion
//! \return Pointer to the new memory block
//!
inline void * SafeRealloc (void * P, size_t size)
{
  void * Q = realloc (P, size);
  if ( Q == NULL  &&  size != 0 )
    AMOS_THROW_ALLOC ("realloc failed");

  return Q;
}


//--------------------------------------------------- SafeStrdup ---------------
//! \brief Safe strdup wrapper
//!
//! Detects malloc failure and throws AMOS::AllocException_t exception instead
//! of returning a NULL pointer.
//!
//! \param str The string to copy
//! \post The memory is allocated and the string is copied
//! \throws AllocException_t on memory exhaustion
//! \return Pointer to the new string copy
//!
inline char * SafeStrdup (const char * str)
{
  char * Q = strdup (str);
  if ( Q == NULL )
    AMOS_THROW_ALLOC ("strdup failed");
  return Q;
}

} // namespace AMOS

#endif // #ifndef __utility_AMOS_HH
