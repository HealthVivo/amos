#ifndef IGraph_HH
#define IGraph_HH 1

#include <list>
#include <map>
#include "IEdge.hh"
#include "INode.hh"

class IEdge;
class INode;

typedef list< IEdge* >::iterator IEdgeIterator;
typedef list< INode* >::iterator INodeIterator;
typedef map< int, INode* >::iterator PairIterator;


using namespace std;


/**
 * The <b>IGraph</b> class
 *
 *
 * <p>Copyright &copy; 2004, The Institute for Genomic Research (TIGR).
 * <br>All rights reserved.
 *
 * @author  Dan Sommer
 *
 * <pre>
 * $RCSfile$
 * $Revision$
 * $Date$
 * $Author$
 * </pre>
 */
class IGraph {
  
public:

  /**
   * output dot file for the graph
   */
  virtual   void create_dot_file(const char* p_filename) = 0;
  
  /**
   * create new INode
   */
  virtual   INode* new_node(int key, void* p_element = NULL) = 0;
  virtual   INode* new_node(void* p_element = NULL) = 0;

  virtual   INode* get_node(int p_key) = 0;

  virtual   int num_nodes() = 0;

  virtual   PairIterator nodes_begin() = 0;
  virtual   PairIterator nodes_end() = 0;
  

  virtual   int degree(INode* p_node) const = 0;
  virtual   int out_degree(INode* p_node) const = 0;
  virtual   int in_degree(INode* p_node) const = 0;
  
  virtual   list< IEdge* > incident_edges(INode* p_node) const = 0;
  virtual   list< IEdge* > in_edges(INode* p_node) const = 0;
  virtual   list< IEdge* > out_edges(INode* p_node) const = 0;

  virtual   INode* aNode() = 0;
  virtual   void clear_flags() = 0;

  /**
   * create new IEdge
   */
  virtual   IEdge* new_edge(INode* p_n1, INode* p_n2, void* p_element = NULL) = 0;
  
  virtual   int num_edges() = 0;
  virtual   IEdgeIterator edges_begin() = 0;
  virtual   IEdgeIterator edges_end() = 0;

  virtual   INode* opposite(INode* p_node, IEdge* p_edge) = 0;
  virtual   INode* source(IEdge* p_edge) = 0;
  virtual   INode* target(IEdge* p_edge) = 0;

  virtual   list< INode* > adjacent_nodes(INode* p_node) = 0;
  virtual   list< INode* > out_adjacent(INode* p_node) = 0;
  virtual   list< INode* > in_adjacent(INode* p_node) = 0;

};


#endif // #ifndef IGraph_HH
