#ifndef _MDL_EXPRESSION_GRAPH_HPP_
#define _MDL_EXPRESSION_GRAPH_HPP_

#include "LookupTable.hpp"
#include "Location.hpp"
#include <unordered_map>
#include <unordered_set>
#include <boost/pool/object_pool.hpp>
#include "FileStatus.hpp"
#include "Symbol.hpp"

namespace sdo
{

/**
 * A class that represents all definitions in a mdl file
 * as an expression graph.
 */
class ExpressionGraph : public FileStatus
{
public:
   enum Operator
   {
      /**
       * Integ operator from mdl
       *        INTEG(rate,initial)
       *
       * Differential equation with initial value 'initial'
       * and change rate 'rate'.
       */
      INTEG,
      /**
       * IF THEN ELSE operator from mdl
       *        IF THEN ELSE( cond, thenval, elseval )
       *
       * If cond evaluates to true the value is 'thenval'
       * else the value is 'elseval'.
       */
      IF,
      /**
       * ACTIVE INITIAL operator from mdl
       *        ACTIVE INITIAL( activeeq, initialeq )
       *
       * returns 'activeeq' as value but 'initialeq' when
       * used as initial value.
       */
      ACTIVE_INITIAL,
      /**
       * INITIAL operator from mdl
       *        INITIAL( arg )
       *
       * returns intial value of 'arg'.
       */
      INITIAL,
      /**
       * DELAY FIXED operator from mdl
       *        DELAY FIXED( input, delaytime, initial )
       *
       * Returns the value of 'input' from current time minus 'delaytime'.
       * If current time is smaller than 'delaytime' the value is the
       * initial value of 'initial'.
       */
      DELAY_FIXED,
      /**
       * PULSE operator from mdl
       *        PULSE( start, width )
       *
       * Returns 1.0 if the current time is between 'start'
       * and 'start'+'width' and 0.0 otherwise.
       */
      PULSE,
      /**
       * PULSE TRAIN operator from mdl
       *        PULSE( start, width )
       *
       * Returns 1.0 if the current time is between 'start'
       * and 'start'+'width' and 0.0 otherwise.
       */
      PULSE_TRAIN,
      /**
       *
       */
      STEP,
      /**
       *
       */
      RAMP,
      /**
       * Two children with the bounds of the interval for
       * choosing a random value with a uniform distribution.
       */
      RANDOM_UNIFORM,
      /**
       * Plus operator.
       */
      PLUS,
      /**
       * Minus operator.
       */
      MINUS,
      /**
       * Multiplication operator.
       */
      MULT,
      /**
       * Division operator.
       */
      DIV,
      /**
       * Greater than operator.
       */
      G,
      /**
       * Greater or equal operator
       */
      GE,
      /**
       * Lower than operator.
       */
      L,
      /**
       * Lower or equal operator.
       */
      LE,
      /**
       * Equal to operator.
       */
      EQ,
      /**
       * Not equal to operator.
       */
      NEQ,
      /**
       * :AND: operator
       */
      AND,
      /**
       * :OR: operator
       */
      OR,
      /** POWER(base, exponent) */
      POWER,
      /** LOG( x, base ) */
      LOG,
      /** MIN(a,b) */
      MIN,
      /** MAX(a,b) */
      MAX,
      /** MODULO(x,y) */
      MODULO,
      /**  Unary minus operator. */
      UMINUS,
      /** SQRT(x) */
      SQRT,
      /** EXP(x) */
      EXP,
      /** LN(x) */
      LN,
      /** ABS(x) */
      ABS,
      /**
       * Operator INTERGER(x): Round 'x' to the next integer value towards zero.
       */
      INTEGER,
      /**
       * :NOT: operator
       */
      NOT,
      SIN,
      COS,
      TAN,
      ARCSIN,
      ARCCOS,
      ARCTAN,
      SINH,
      COSH,
      TANH,
      /**
       * Represents the value of the current time.
       */
      TIME,
      /**
       * Represents a constant the value is stored
       * at node->value
       */
      CONSTANT,
      /**
       * Represents a control. Has three children than can be nullptr's.
       * First child is the lower bound, second child the start value and
       * third child the upper bound.
       */
      CONTROL,
      /**
       * Application of lookup table in child1 to the argument
       * in child2
       */
      APPLY_LOOKUP,
      /**
       * Contains the actual lookup table in node->lookup_table.
       */
      LOOKUP_TABLE,
      /** Undefined node. Will give an error during call to analyze() */
      NIL
   };

   enum NodeType
   {
      /** The node is constant */
      CONSTANT_NODE = 0, // 000
      /** The node is constant for each time */
      STATIC_NODE   = 1,   // 001
      /** The node depends on a state */
      DYNAMIC_NODE  = 3,  // 011
      /** The node type is unknow. After analyze() no node shoudl have this type. */
      UNKNOWN       = 7        // 111
   };

   enum InitialType
   {
      /** The initial value is a constant */
      CONSTANT_INIT  = 0,
      /** The initial value is a control */
      CONTROLED_INIT = 1,
      /** Type is unknown */
      UNKNOWN_INIT = 2,
   };
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic" //anonymous union is not standard

   /**
    * A node in the expression graph.
    */
   struct Node
   {

      /**
       * The operator of the node
       */
      Operator    op;
      /**
       * The type of the node
       */
      NodeType    type = UNKNOWN;
      /**
       * Is the node integer (only variables)
       */
      bool    integer = false;
      /**
       * The initial type of the node
       */
      InitialType init = UNKNOWN_INIT;
      union
      {
         struct
         {
            /**
             * The first child if it exists
             */
            Node *child1;
            /**
             * The second child if it exists
             */
            Node *child2;
            /**
             * The third child if it exists
             */
            Node *child3;
         };
         /**
          * The lookup table if it exists
          */
         LookupTable *lookup_table;
      };
      /**
       * The level of the node, i.e. a number representing its topological
       * order in the expression graph. If a node represents a+b its level
       * is max(a->level,b->level)+1
       */
      int level;
      union
      {
         /**
          * The size of a control. E.g. the number of timesteps
          * a control is valid for. If this is 0 then the control
          * is the same for all times. If it is 1 there is a
          * different control var at each time, and if it is greater
          * than one it is a piecewise control.
          */
         int    control_size;
         /**
          * The initial value of a node. For constants this value
          * is valid at all times. For nodes with init = CONTROLED_INIT
          * this values means nothing.
          */
         double value;
      };

      Symbol unit;
      boost::optional<double> lb;
      boost::optional<double> ub;
      /**
       * Locations in the file where this node is used.
       */
      std::vector<FileLocation> usages;
   };
#pragma GCC diagnostic pop

   /**
    * Evaluate a static node at given time
    */
   double evaluateNode( const Node *node, double time, bool initial = false ) const;

   /**
    * Equality functor that compares two nodes by their structure, i.e.
    * a+b is equal to b+a and some more transformations.
    */
   struct structural_node_eq
   {
      bool operator()( const Node *a, const Node *b ) const;
   };

   /**
    * Hash functor that hashes two nodes, that are equal in their structure to
    * the same hash value, e.g. a+b, b+a.
    */
   struct structural_node_hash
   {
      std::size_t operator()( const Node *node ) const;
   };


   /**
    * Add a symbol to the expression graph.
    *
    * \param s the symbol
    * \param node the node for the symbol
    */
   void addSymbol( const Symbol &s, Node *node );

   /**
    * \return a reference to the symbol table.
    */
   std::unordered_map<Symbol, Node *> &getSymbolTable()
   {
      return symbol_table;
   }

   /**
    * \return a const reference to the symbol table.
    */
   const std::unordered_map<Symbol, Node *> &getSymbolTable() const
   {
      return symbol_table;
   }

   /**
    * \brief Get the node representing the given symbol.
    *
    * If the symbol does not exists yet, then a temporary node with
    * operator NIL is created. If this node is used as operator
    * to another node, then this usage is tracked. Later
    * when the symbol s is added to the expression graph
    * all usages of the temporary node will get substituted.
    *
    * \param s the symbol
    * \return a node representing the symbol
    */
   Node *getNode( const Symbol &s );

   /**
    * \brief Get the node representing the given operator applied to
    * the given child node
    *
    * \param op the operator
    * \param child the child node
    * \return the node
    */
   Node *getNode( Operator op, Node *child );

   /**
    * \brief Get the node representing the given operator applied to
    * the given child nodes.
    *
    * \param op the operator
    * \param child1 the first child node
    * \param child2 the second child node
    * \return the node
    */
   Node *getNode( Operator op, Node *child1, Node *child2 );

   /**
    * \brief Get the node representing the given operator applied to
    * the given child nodes.
    *
    * \param op the operator
    * \param child1 the first child node
    * \param child2 the second child node
    * \param child3 the third child node
    * \return the node
    */
   Node *getNode( Operator op, Node *child1, Node *child2, Node *child3 );

   /**
    * \brief Get the node representing the given constant value.
    *
    * \param val the constant value
    * \return the node
    */
   Node *getNode( double val );

   /**
    * \brief Get the node representing the current time.
    *
    * \return the node
    */
   Node *getTimeNode();

   /**
    * \brief Wrap a lookup table as a node.
    *
    * \return the node containing the lookup table.
    */
   Node *getNode( LookupTable *table );

   LookupTable *createLookupTable();

   Node *createTmpNode();

   void substituteTmpNode( Node *tmp, Node *subst );

   void analyze();

   /**
    * If set to true, each constant will get a unique node.
    * Useful if it is not desired that the symbols defined
    * by A=1 and B=1 in a mdl file cannot be distinguished.
    */
   void useUniqueConstants( bool val );

   /**
    * A range of two iterators represented as an iterable
    * object.
    */
   template<typename ITER>
   class IteratorRange
   {
   public:
      IteratorRange( std::pair<ITER, ITER> p ) : begin_( p.first ), end_( p.second ) {}

      ITER begin() const
      {
         return begin_;
      }

      ITER end() const
      {
         return end_;
      }

      bool empty() const
      {
         return begin_ == end_;
      }

      std::size_t size() const
      {
         return end_ - begin_;
      }

   private:
      ITER begin_;
      ITER end_;
   };

   /**
    * Get an iterator range containing all symbols for the given node.
    *
    * \param node the node for which to retireve the symbols
    *
    * \return the iterator range that contains the symbols.
    */
   IteratorRange<std::unordered_multimap<Node *, Symbol>::iterator> getSymbol( Node *const node )
   {
      return IteratorRange<std::unordered_multimap<Node *, Symbol>::iterator>( node_table.equal_range( node ) );
   }

   /**
    * Get an iterator range containing all comments for the given symbol.
    *
    * \param s the symbol
    *
    * \return the iterator range that contains the comments.
    */
   IteratorRange<std::unordered_multimap<Symbol, Symbol>::iterator> getComments( const Symbol &s )
   {
      return IteratorRange<std::unordered_multimap<Symbol, Symbol>::iterator>( comments.equal_range( s ) );
   }

   /**
    * Add comments in given container to the comments of the given symbols.
    *
    * \param s the symbol
    * \param c the container which should have begin and end methods to be iterable.
    */
   template<typename Iterable>
   void addComments( const Symbol &s, Iterable c )
   {
      for( auto & comment : c )
         comments.emplace( s, comment );
   }

private:
   std::unordered_map<Symbol, Node *>                                   symbol_table;
   std::unordered_multimap<Node *, Symbol>                              node_table;
   std::unordered_multimap<Symbol, Symbol>                             comments;
   std::unordered_set<Node *, structural_node_hash, structural_node_eq> nodes_;
   boost::object_pool<Node>                                            node_pool_;
   boost::object_pool<LookupTable>                                     lookup_pool_;
   std::unordered_multimap<Node *, Node **>                              temp_node_usages_;
   bool unique_constants = false;
};


}

#endif
