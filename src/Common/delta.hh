////////////////////////////////////////////////////////////////////////////////
//! \file
//! \author Adam M Phillippy
//! \date 03/26/2003
//!
//! \brief Class declarations for the handling of delta alignment files
//!
//! \see delta.cc
////////////////////////////////////////////////////////////////////////////////

#ifndef __DELTA_HH
#define __DELTA_HH

#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <map>


const std::string NUCMER_STRING = "NUCMER"; //!< nucmer id string
const std::string PROMER_STRING = "PROMER"; //!< promer id string

typedef char AlignmentType_t;               //!< type of alignment data
const AlignmentType_t NULL_DATA = 0;        //!< unknown alignment data type
const AlignmentType_t NUCMER_DATA = 'N';    //!< nucmer alignment data
const AlignmentType_t PROMER_DATA = 'P';    //!< promer alignment data

typedef unsigned char Dir_t;                //!< directional type
const Dir_t FORWARD_DIR = 0;                //!< forward direction
const Dir_t REVERSE_DIR = 1;                //!< reverse direction



//===================================================== DeltaAlignment_t =======
struct DeltaAlignment_t
//!< A single delta encoded alignment region
{
  unsigned long int sR;    //!< start coordinate in the reference
  unsigned long int eR;    //!< end coordinate in the reference
  unsigned long int sQ;    //!< start coordinate in the reference
  unsigned long int eQ;    //!< end coordinate in the reference
  unsigned long int idyc;  //!< number of mismatches in the alignment
  unsigned long int simc;  //!< number of similarity scores < 1 in the alignment
  unsigned long int stpc;  //!< number of stop codons in the alignment

  float idy;               //!< percent identity [0 - 100]
  float sim;               //!< percent similarity [0 - 100]
  float stp;               //!< percent stop codon [0 - 100]

  std::vector<long int> deltas;  //!< delta encoded alignment informaiton

  DeltaAlignment_t ( )
  {
    clear ( );
  }

  void clear ( )
  {
    sR = eR = sQ = eQ = 0;
    idy = sim = stp = 0;
    deltas.clear ( );
  }
};



//===================================================== DeltaRecord_t ==========
struct DeltaRecord_t
//!< A delta record representing the alignments between two sequences
{
  std::string idR;         //!< reference contig ID
  std::string idQ;         //!< query contig ID
  unsigned long int lenR;  //!< length of the reference contig
  unsigned long int lenQ;  //!< length of the query contig

  std::vector<DeltaAlignment_t> aligns; //!< alignments between the two seqs

  DeltaRecord_t ( )
  {
    clear ( );
  }

  void clear ( )
  {
    idR.erase ( );
    idQ.erase ( );
    lenR = lenQ = 0;
    aligns.clear ( );
  }
};



//===================================================== DeltaReader_t ==========
//! \brief Delta encoded file reader class
//!
//! Handles the input of delta encoded alignment information for various MUMmer
//! utilities. Very basic functionality, can be expanded as necessary...
//!
//! \see DeltaRecord_t
//==============================================================================
class DeltaReader_t {

private:

  std::string delta_path_m;      //!< the name of the delta input file
  std::ifstream delta_stream_m;  //!< the delta file input stream
  std::string data_type_m;       //!< the type of alignment data
  std::string reference_path_m;  //!< the name of the reference file
  std::string query_path_m;      //!< the name of the query file
  DeltaRecord_t record_m;        //!< the current delta information record
  bool is_record_m;              //!< there is a valid record in record_m
  bool is_open_m;                //!< delta stream is open


  //--------------------------------------------------- readNextAlignment ------
  //! \brief Reads in a delta encoded alignment
  //!
  //! \param align read info into this structure
  //! \param read_deltas read delta information yes/no
  //! \pre delta_stream is positioned at the beginning of an alignment header
  //! \return void
  //!
  void readNextAlignment (DeltaAlignment_t & align, const bool read_deltas);


  //--------------------------------------------------- readNextRecord ---------
  //! \brief Reads in the next delta record from the delta file
  //!
  //! \param read_deltas read delta information yes/no
  //! \pre delta file must be open
  //! \return true on success, false on EOF
  //!
  bool readNextRecord (const bool read_deltas);


  //--------------------------------------------------- checkStream ------------
  //! \brief Check stream status and abort program if an error has occured
  //!
  //! \return void
  //!
  void checkStream ( )
  {
    if ( !delta_stream_m.good ( ) )
      {
	std::cerr << "ERROR: Could not parse delta file, "
		  << delta_path_m << std::endl;
	exit (-1);
      }
  }


public:
  //--------------------------------------------------- DeltaReader_t ----------
  //! \brief Default constructor
  //!
  //! \return void
  //!
  DeltaReader_t ( )
  {
    is_record_m = false;
    is_open_m = false;
  }


  //--------------------------------------------------- ~DeltaReader_t ---------
  //! \brief Default destructor
  //!
  //! \return void
  //!
  ~DeltaReader_t ( )
  {
    close ( );
  }


  //--------------------------------------------------- open -------------------
  //! \brief Opens a delta file by path
  //!
  //! \param delta file to open
  //! \return void
  //!
  void open (const std::string & delta_path);


  //--------------------------------------------------- close ------------------
  //! \brief Closes any open delta file and resets class members
  //!
  //! \return void
  //!
  void close ( )
  {
    delta_path_m.erase ( );
    delta_stream_m.close ( );
    data_type_m.erase ( );
    reference_path_m.erase ( );
    query_path_m.erase ( );
    record_m.clear ( );
    is_record_m = false;
    is_open_m = false;
  }


  //--------------------------------------------------- readNext --------------
  //! \brief Reads in the next delta record from the delta file
  //!
  //! \param read_deltas read delta information yes/no
  //! \pre delta file must be open
  //! \return true on success, false on EOF
  //!
  inline bool readNext (bool getdeltas = true)
  {
    return readNextRecord (getdeltas);
  }



  //--------------------------------------------------- readNextHeadersOnly ----
  //! \brief Reads in the next delta record from the delta file
  //!
  //! Only reads the alignment header information, does not read in the delta
  //! information and leaves each alignment structure's delta vector empty.
  //!
  //! \pre delta file must be open
  //! \return true on success, false on EOF
  //!
  inline bool readNextHeadersOnly ( )
  {
    return readNextRecord (false);
  }


  //--------------------------------------------------- getRecord --------------
  //! \brief Returns a reference to the current delta record
  //!
  //! \pre readNext( ) was successfully called in advance
  //! \return true on success, false on failure or end of file
  //!
  const DeltaRecord_t & getRecord ( ) const
  {
    assert (is_record_m);
    return record_m;
  }


  //--------------------------------------------------- getDeltaPath -----------
  //! \brief Get the path of the current delta file
  //!
  //! \pre delta file is open
  //! \return the path of the delta file
  //!
  const std::string & getDeltaPath ( ) const
  {
    assert (is_open_m);
    return delta_path_m;
  }


  //--------------------------------------------------- getDataType ------------
  //! \brief Get the data type of the current delta file
  //!
  //! \pre delta file is open
  //! \return the data type of the current file
  //!
  const std::string & getDataType ( ) const
  {
    assert (is_open_m);
    return data_type_m;
  }


  //--------------------------------------------------- getReferencePath -------
  //! \brief Get the path of the MUMmer reference file
  //!
  //! \pre delta file is open
  //! \return the path of the MUMmer reference file
  //!
  const std::string & getReferencePath ( ) const
  {
    assert (is_open_m);
    return reference_path_m;
  }


  //--------------------------------------------------- getQueryPath -----------
  //! \brief Get the path of the MUMmer query file
  //!
  //! \pre delta file is open
  //! \return the path of the MUMmer query file
  //!
  const std::string & getQueryPath ( ) const
  {
    assert (is_open_m);
    return query_path_m;
  }
};



//===================================================== DeltaEdgelet_t =========
struct DeltaEdgelet_t
//!< A piece of a delta graph edge, a single alignment
{
  unsigned char isGOOD : 1;   //!< meets the requirements
  unsigned char isQLIS : 1;   //!< is part of the query's LIS
  unsigned char isRLIS : 1;   //!< is part of the reference's LIS
  unsigned char isGLIS : 1;   //!< is part of the reference/query LIS
  unsigned char dirR   : 1;   //!< reference match direction
  unsigned char dirQ   : 1;   //!< query match direction

  float idy, sim, stp;                    //!< percent identity [0 - 1]
  unsigned long int idyc, simc, stpc;     //!< idy, sim, stp counts
  unsigned long int loQ, hiQ, loR, hiR;   //!< alignment bounds

  std::string delta;                      //!< delta information

  DeltaEdgelet_t ( )
  {
    isGOOD = true;
    isQLIS = isRLIS = isGLIS = false;
    dirR = dirQ = FORWARD_DIR;
    idy = sim = stp = 0;
    idyc = simc = stpc = 0;
    loQ = hiQ = loR = hiR = 0;
  }
};



//===================================================== DeltaEdge_t ============
struct DeltaNode_t;
struct DeltaEdge_t
//!< A delta graph edge, alignments between a single reference and query
{
  DeltaNode_t * refnode;      //!< the adjacent reference node
  DeltaNode_t * qrynode;      //!< the adjacent query node
  std::vector<DeltaEdgelet_t *> edgelets;  //!< the set of individual alignments

  DeltaEdge_t ( )
  {
    refnode = qrynode = NULL;
  }

  ~DeltaEdge_t ( )
  {
    std::vector<DeltaEdgelet_t *>::iterator i;
    for ( i = edgelets . begin( ); i != edgelets . end( ); ++ i )
      delete (*i);
  }

  void build (const DeltaRecord_t & rec);
};


//===================================================== DeltaNode_t ============
struct DeltaNode_t
//!< A delta graph node, contains the sequence information
{
  const std::string * id;             //!< the id of the sequence
  unsigned long int len;              //!< the length of the sequence
  std::vector<DeltaEdge_t *> edges;   //!< the set of related edges

  DeltaNode_t ( )
  {
    id = NULL;
    len = 0;
  }

  // DeltaGraph_t will take care of destructing the edges
};



//===================================================== DeltaGraph_t ===========
//! \brief A graph of sequences (nodes) and their alignments (edges)
//!
//!  A bipartite graph with two partite sets, R and Q, where R is the set of
//!  reference sequences and Q is the set of query sequences. These nodes are
//!  named "DeltaNode_t". We connect a node in R to a node in Q if an alignment
//!  is present between the two sequences. The group of all alignments between
//!  the two is named "DeltaEdge_t" and a single alignment between the two is
//!  named a "DeltaEdgelet_t". Alignment coordinates reference the forward
//!  strand and are stored lo before hi.
//!
//==============================================================================
class DeltaGraph_t
{
public:

  std::map<std::string, DeltaNode_t> refnodes;
  //!< the reference graph nodes, 1 per aligned sequence

  std::map<std::string, DeltaNode_t> qrynodes;
  //!< the query graph nodes, 1 per aligned sequence

  std::string refpath;         //!< path of the reference FastA file
  std::string qrypath;         //!< path of the query FastA file
  AlignmentType_t datatype;    //!< alignment data type

  DeltaGraph_t ( )
  {
    datatype = NULL_DATA;
  }

  ~DeltaGraph_t ( )
  {
    clear( );
  }

  //--------------------------------------------------- build ------------------
  //! \brief Build a new graph from a deltafile
  //!
  //! \param deltapath The path of the deltafile to read
  //! \param getdeltas Read the delta-encoded gap positions? yes/no
  //! \return void
  //!
  void build (const std::string & deltapath, bool getdeltas = true);


  //--------------------------------------------------- clean ------------------
  //! \brief Clean the graph of all edgelets where isGOOD = false
  //!
  //! Removes all edgelets from the graph where isGOOD = false. Afterwhich, all
  //! now empty edges or nodes are also removed.
  //!
  //! \return void
  //!
  void clean ( );


  //--------------------------------------------------- clear ------------------
  //! \brief Clear the graph of all nodes, edges and edgelets
  //!
  //! \return void
  //!
  void clear ( )
  {
    //-- Make sure the edges only get destructed once
    std::map<std::string, DeltaNode_t>::iterator i;
    std::vector<DeltaEdge_t *>::iterator j;
    for ( i = refnodes . begin( ); i != refnodes . end( ); ++ i )
      for ( j  = i -> second . edges . begin( );
            j != i -> second . edges . end( ); ++ j )
        delete (*j);

    refnodes.clear( );
    qrynodes.clear( );
    refpath.erase( );
    qrypath.erase( );
    datatype = NULL_DATA;
  }


  //--------------------------------------------------- getNodeCount -----------
  //! \brief Counts and returns the number of graph nodes (sequences)
  //!
  //! \return The number of graph nodes
  //!
  long int getNodeCount ( );


  //--------------------------------------------------- getEdgeCount -----------
  //! \brief Counts and returns the number of graph edges
  //!
  //! \return void
  //!
  long int getEdgeCount ( );


  //--------------------------------------------------- getEdgeletCount --------
  //! \brief Counts and returns the number of graph edgelets (alignments)
  //!
  //! \return void
  //!
  long int getEdgeletCount ( );


  //--------------------------------------------------- flagGLIS ---------------
  //! \brief Flag edgelets in the global LIS and unflag those who are not
  //!
  //! Runs a length*identity weigthed LIS to determine the longest, mutually
  //! consistent set of alignments between all pairs of sequences. This
  //! essentially constructs the global alignment between all sequence pairs.
  //!
  //! Sets isGLIS flag for good and unsets isGOOD flag for bad.
  //!
  //! \param epsilon Keep repeat alignments within epsilon % of the best align
  //! \return void
  //!
  void flagGLIS (float epsilon = -1);


  //--------------------------------------------------- flagQLIS ---------------
  //! \brief Flag edgelets in the query LIS and unflag those who are not
  //!
  //! Runs a length*identity weighted LIS to determine the longest, QUERY
  //! consistent set of alignments for all query sequences. This effectively
  //! identifies the "best" alignments for all positions of each query.
  //!
  //! Sets isQLIS flag for good and unsets isGOOD flag for bad.
  //!
  //! \param epsilon Keep repeat alignments within epsilon % of the best align
  //! \param maxolap Only allow alignments to overlap by maxolap percent [0-100]
  //! \return void
  //!
  void flagQLIS (float epsilon = -1, float maxolap = 100.0);


  //--------------------------------------------------- flagRLIS ---------------
  //! \brief Flag edgelets in the reference LIS and unflag those who are not
  //!
  //! Runs a length*identity weighted LIS to determine the longest, REFERENCE
  //! consistent set of alignments for all reference sequences. This effectively
  //! identifies the "best" alignments for all positions of each reference.
  //!
  //! Sets isRLIS flag for good and unsets isGOOD flag for bad.
  //!
  //! \param epsilon Keep repeat alignments within epsilon % of the best align
  //! \param maxolap Only allow alignments to overlap by maxolap percent [0-100]
  //! \return void
  //!
  void flagRLIS (float epsilon = -1, float maxolap = 100.0);


  //--------------------------------------------------- flagScore --------------
  //! \brief Flag edgelets with scores below a score threshold
  //!
  //! Unsets isGOOD for bad.
  //!
  //! \param minlen Flag edgelets if less than minlen in length
  //! \param minidy Flag edgelets if less than minidy identity [0-100]
  //! \return void
  //!
  void flagScore (long int minlen, float minidy);


  //--------------------------------------------------- flagUNIQ ---------------
  //! \brief Flag edgelets with uniqueness below a certain threshold
  //!
  //! Unsets isGOOD for bad.
  //!
  //! \param minuniq Flag edgelets if less that minuniq percent [0-100] unique
  //! \return void
  //!
  void flagUNIQ (float minuniq);


  //--------------------------------------------------- outputDelta ------------
  //! \brief Outputs the contents of the graph as a deltafile
  //!
  //! \param out The output stream to write to
  //! \return The output stream
  //!
  std::ostream & outputDelta (std::ostream & out);
};

#endif // #ifndef __DELTA_HH
