#include "ExpressionGraph.hpp"
#include "RandomUniform.hpp"
#include <boost/functional/hash.hpp>
#include <deque>
#include <cassert>
#include <stack>

namespace sdo
{

bool ExpressionGraph::structural_node_eq::operator()( const Node *a, const Node *b ) const
{
   if( a->op != b->op )
   {
      switch( a->op )
      {
      case G:
         return b->op == LE && a->child1 == b->child2 && a->child2 == b->child1;

      case GE:
         return b->op == L && a->child1 == b->child2 && a->child2 == b->child1;

      case L:
         return b->op == GE && a->child1 == b->child2 && a->child2 == b->child1;

      case LE:
         return b->op == G && a->child1 == b->child2 && a->child2 == b->child1;

      default:
         return false;
      }
   }

   switch( a->op )
   {
   case LOOKUP_TABLE:
      return a->lookup_table == b->lookup_table;

   case CONSTANT:
      return a->value == b->value;

   case TIME:
      return true;

   case RANDOM_UNIFORM:
      return false;

   case IF:
   case DELAY_FIXED:
   case PULSE_TRAIN:
   case RAMP:
      return a->child1 == b->child1 && a->child2 == b->child2 && a->child3 == b->child3;

   case MULT:
   case PLUS:
   case MIN:
   case MAX:
   case EQ:
   case NEQ:
   case OR:
   case AND:
      return ( a->child1 == b->child1 && a->child2 == b->child2 ) ||
             ( a->child2 == b->child1 && a->child1 == b->child2 );

   case MINUS:
   case DIV:
   case G:
   case GE:
   case L:
   case LE:
   case POWER:
   case LOG:
   case MODULO:
   case INTEG:
   case ACTIVE_INITIAL:
   case PULSE:
   case STEP:
   case APPLY_LOOKUP:
      return a->child1 == b->child1 && a->child2 == b->child2;

   case UMINUS:
   case SQRT:
   case EXP:
   case LN:
   case ABS:
   case INTEGER:
   case NOT:
   case SIN:
   case COS:
   case TAN:
   case ARCSIN:
   case ARCCOS:
   case ARCTAN:
   case SINH:
   case COSH:
   case TANH:
   case INITIAL:
      return a->child1 == b->child1;

   case CONTROL:
   case NIL:
      return a == b;
   }

   assert( false );
   return false;
}

std::size_t ExpressionGraph::structural_node_hash::operator()( const Node *node ) const
{
   switch( node->op )
   {
   case LOOKUP_TABLE:
      return hash_value( *( node->lookup_table ) );

   case CONSTANT:
      return boost::hash_value( node->value );

   case TIME:
      return boost::hash_value( TIME );

   case RANDOM_UNIFORM:
      return 0;

   case IF:
   case DELAY_FIXED:
   case PULSE_TRAIN:
   case RAMP:
   {
      std::size_t hash = boost::hash_value( node->op );
      boost::hash_combine( hash, reinterpret_cast<uintptr_t>( node->child1 ) );
      boost::hash_combine( hash, reinterpret_cast<uintptr_t>( node->child2 ) );
      boost::hash_combine( hash, reinterpret_cast<uintptr_t>( node->child3 ) );
      return hash;
   }

   case MULT:
   case PLUS:
   case MIN:
   case MAX:
   case EQ:
   case NEQ:
   case OR:
   case AND:
   {
      std::size_t hash = boost::hash_value( node->op );
      boost::hash_combine( hash, reinterpret_cast<uintptr_t>( node->child1 ) ^
                           reinterpret_cast<uintptr_t>( node->child2 ) );
      return hash;
   }

   case G:
   case LE:
   {
      std::size_t hash = boost::hash_value( G );
      boost::hash_combine( hash, boost::hash_value( LE ) );
      boost::hash_combine( hash, reinterpret_cast<uintptr_t>( node->child1 ) ^
                           reinterpret_cast<uintptr_t>( node->child2 ) );
   }

   case L:
   case GE:
   {
      std::size_t hash = boost::hash_value( L );
      boost::hash_combine( hash, boost::hash_value( GE ) );
      boost::hash_combine( hash, reinterpret_cast<uintptr_t>( node->child1 ) ^
                           reinterpret_cast<uintptr_t>( node->child2 ) );
   }

   case MINUS:
   case DIV:
   case POWER:
   case LOG:
   case MODULO:
   case INTEG:
   case APPLY_LOOKUP:
   case ACTIVE_INITIAL:
   case PULSE:
   case STEP:
   {
      std::size_t hash = boost::hash_value( node->op );
      boost::hash_combine( hash, reinterpret_cast<uintptr_t>( node->child1 ) );
      boost::hash_combine( hash, reinterpret_cast<uintptr_t>( node->child2 ) );
      return hash;
   }

   case UMINUS:
   case SQRT:
   case EXP:
   case LN:
   case ABS:
   case INTEGER:
   case NOT:
   case SIN:
   case COS:
   case TAN:
   case ARCSIN:
   case ARCCOS:
   case ARCTAN:
   case SINH:
   case COSH:
   case TANH:
   case INITIAL:
   {
      std::size_t hash = boost::hash_value( node->op );
      boost::hash_combine( hash, reinterpret_cast<uintptr_t>( node->child1 ) );
      return hash;
   }

   case CONTROL:
   case NIL:
      return boost::hash_value( reinterpret_cast<uintptr_t>( node ) );
   }

   assert( false );

   return -1;
}


void ExpressionGraph::addSymbol( const Symbol &s, ExpressionGraph::Node *node )
{
   auto sym = symbol_table.find( s );

   if( sym != symbol_table.end() )
   {
      //if node is not previously undefined do not change it instead return
      if( sym->second->op != NIL )
         return;

      auto eq_range = node_table.equal_range( sym->second );
      std::vector<Symbol> prev_symbols;

      for( auto it = eq_range.first; it != eq_range.second; ++it )
      {
         if( it->second != s )
         {
            prev_symbols.emplace_back( it->second );
            symbol_table[it->second] = node;
         }
      }

      node_table.erase( eq_range.first, eq_range.second );

      for( auto & sym : prev_symbols )
      {
         node_table.emplace( node, std::move( sym ) );
      }

      substituteTmpNode( sym->second, node );
   }

   symbol_table[s] = node;
   node_table.emplace( node, s );
}

ExpressionGraph::Node *ExpressionGraph::getNode( const Symbol &s )
{
   auto i = symbol_table.find( s );

   if( i != symbol_table.end() )
      return i->second;

   Node *n = createTmpNode();
   symbol_table[s] = n;
   node_table.emplace( n, s );
   return n;
}

ExpressionGraph::Node *ExpressionGraph::getNode( Operator op, Node *child )
{
   Node *a = node_pool_.construct();
   a->child1 = child;
   a->op     = op;
   auto b = nodes_.find( a );

   if( b != nodes_.end() )
   {
      node_pool_.free( a );
      return *b;
   }

   if( child->op == NIL )
      temp_node_usages_.emplace( child, &( a->child1 ) );

   nodes_.emplace( a );
   return a;
}

ExpressionGraph::Node *ExpressionGraph::getNode( Operator op, Node *child1, Node *child2 )
{
   Node *a = node_pool_.construct();
   a->child1 = child1;
   a->child2 = child2;
   a->op     = op;
   auto b = nodes_.find( a );

   if( b != nodes_.end() )
   {
      node_pool_.free( a );
      return *b;
   }

   if( child1->op == NIL )
      temp_node_usages_.emplace( child1, &( a->child1 ) );

   if( child2->op == NIL )
      temp_node_usages_.emplace( child2, &( a->child2 ) );

   nodes_.emplace( a );
   return a;
}

ExpressionGraph::Node *ExpressionGraph::getNode( Operator op, Node *child1, Node *child2, Node *child3 )
{
   Node *a = node_pool_.construct();
   a->child1 = child1;
   a->child2 = child2;
   a->child3 = child3;
   a->op     = op;
   auto b = nodes_.find( a );

   if( b != nodes_.end() )
   {
      node_pool_.free( a );
      return *b;
   }

   if( child1 && child1->op == NIL )
      temp_node_usages_.emplace( child1, &( a->child1 ) );

   if( child2 && child2->op == NIL )
      temp_node_usages_.emplace( child2, &( a->child2 ) );

   if( child3 && child3->op == NIL )
      temp_node_usages_.emplace( child3, &( a->child3 ) );

   nodes_.emplace( a );
   return a;
}

ExpressionGraph::Node *ExpressionGraph::getNode( double val )
{
   Node *a = node_pool_.construct();
   a->op    = CONSTANT;
   a->value = val;
   a->type  = CONSTANT_NODE;
   a->init = CONSTANT_INIT;
   a->level = 0;

   if( !unique_constants )
   {
      auto b = nodes_.find( a );

      if( b != nodes_.end() )
      {
         node_pool_.free( a );
         return *b;
      }
   }

   nodes_.emplace( a );
   return a;
}

ExpressionGraph::Node *ExpressionGraph::getTimeNode()
{
   Node *a = node_pool_.construct();
   a->op = TIME;
   auto b = nodes_.find( a );

   if( b != nodes_.end() )
   {
      node_pool_.free( a );
      return *b;
   }

   nodes_.emplace( a );
   return a;
}

LookupTable *ExpressionGraph::createLookupTable()
{
   return lookup_pool_.construct();
}

ExpressionGraph::Node *ExpressionGraph::createTmpNode()
{
   Node *n = node_pool_.construct();
   n->op = NIL;
   return n;
}

void ExpressionGraph::analyze()
{
   std::deque<Node *> node_deque;
   Node *initial_time_node = getNode( Symbol( "INITIAL TIME" ) );
   Node *final_time_node   = getNode( Symbol( "FINAL TIME" ) );
   Node *time_step_node    = getNode( Symbol( "TIME STEP" ) );

   for( std::pair<const Symbol, Node *> &p : symbol_table )
   {
      if( p.second->op == INTEG )
      {
         node_deque.push_back( p.second );
      }
      else if( p.second != initial_time_node && p.second != final_time_node && p.second != time_step_node )
      {
         node_deque.push_front( p.second );
      }
   }

   node_deque.push_back( initial_time_node );
   node_deque.push_back( final_time_node );
   node_deque.push_back( time_step_node );
   node_deque.push_back( getTimeNode() );

   while( !node_deque.empty() )
   {
      Node *node = node_deque.back();

      if( node->type != UNKNOWN )
      {
         node_deque.pop_back();
         continue;
      }

      switch( node->op )
      {
      case INTEG:
         if( node->child2->init == UNKNOWN_INIT )
         {
            node_deque.push_back( node->child2 );
            continue;
         }

         node->type  = DYNAMIC_NODE;
         node->level = node->child2->level + 1;
         node->init  = node->child2->init;

         if( node->init == CONSTANT_INIT )
         {
            node->value = node->child2->value;
         }

         node_deque.pop_back();
         node_deque.push_front( node->child1 );
         continue;

      case IF:
      {
         node->type  = NodeType( node->child1->type | node->child2->type | node->child3->type );
         node->init  = InitialType( node->child1->init | node->child2->init | node->child3->init );
         node->level = std::max( {node->child1->level, node->child2->level, node->child3->level} ) + 1;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value ? node->child2->value : node->child3->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child3 );
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
         }

         continue;
      }

      case ACTIVE_INITIAL:
      {
         node->type  = NodeType( node->child2->type | node->child1->type );
         node->init  = node->child2->init;
         node->level = node->child2->level;

         if( node->init == UNKNOWN_INIT )
         {
            node_deque.push_back( node->child2 );
            continue;
         }

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            if( node->init == CONSTANT_INIT )
            {
               node->value = node->child2->value;
            }

            node->level = std::max( node->child1->level + 1, node->child2->level );
            node_deque.pop_back();
            continue;

         case CONSTANT_NODE:
            error( node->usages, "Use of ACTIVE INITIAL while active equation is constant." );
            node->level = std::max( node->child1->level + 1, node->child2->level );
            node->value = node->child1->value;
            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.pop_back();
            node_deque.push_front( node->child1 );
            node_deque.push_front( node );
         }

         continue;
      }

      case INITIAL:
         if( node->child1->init == UNKNOWN_INIT )
         {
            node_deque.push_back( node->child1 );
            continue;
         }

         node->level = node->child1->level;

         if( node->child1->init == CONSTANT_INIT )
         {
            node->type  = CONSTANT_NODE;
            node->init  = CONSTANT_INIT;
            node->value = node->child1->value + 1;
         }
         else
         {
            node->type = DYNAMIC_NODE;
            node->init = CONTROLED_INIT;
         }

         node_deque.pop_back();
         continue;

      case DELAY_FIXED:
      {
         if( node->child3->type == UNKNOWN )
            node->type = UNKNOWN;
         else
            node->type = NodeType( node->child1->type | node->child2->type | STATIC_NODE );

         if( node->type != UNKNOWN )
         {
            node->init = node->child3->init;

            if( node->child1->type == CONSTANT_NODE )
               warning( node->usages,
                        "DELAY FIXED used with constant input. Consider using STEP instead." );

            if( node->child2->type != CONSTANT_NODE )
               warning( node->usages,
                        "DELAY FIXED used with non constant delay time. Only initial value will be used." );
         }

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level     = std::max( {node->child1->level, node->child2->level, node->child3->level, getTimeNode()->level} ) + 1;

            if( node->init == CONSTANT_INIT )
               node->value = node->child3->value;

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child3 );
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         case CONSTANT_NODE:
            assert( false );
         }

         continue;
      }

      case PULSE:
      {
         node->type = NodeType( node->child1->type | node->child2->type | STATIC_NODE );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
            error( node->usages, "Using PULSE with non constant arguments" );

         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level, getTimeNode()->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
               double initial_time = initial_time_node->value;
               double time_step    = time_step_node->value;
               double time_plus    = initial_time + 0.5 * time_step;
               double start        = node->child1->value;
               double width        = node->child2->value;
               node->value          = time_plus > start && time_plus < start + width ? 1. : 0.;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         case CONSTANT_NODE:
            assert( false );
         }

         continue;
      }

      case PULSE_TRAIN:
      {
         node->type = NodeType(
                         node->child1->child1->type | node->child1->child2->type | node->child2->type | node->child3->type | STATIC_NODE );
         node->init = InitialType(
                         node->child1->child1->init | node->child1->child2->init | node->child2->init | node->child3->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
            error( node->usages, "Using PULSE TRAIN with non constant arguments" );

         case STATIC_NODE:
            node->level = std::max( {node->child1->child1->level, node->child1->child2->level, node->child2->level,
                                     node->child3->level, getTimeNode()->level
                                    } ) + 1;

            if( node->init == CONSTANT_INIT )
            {
               double initial_time = initial_time_node->value;
               double time_step    = time_step_node->value;
               double time_plus    = initial_time + 0.5 * time_step;
               double start        = node->child1->child1->value;
               double width        = node->child1->child2->value;
               double tbetween     = node->child2->value;
               double end          = node->child3->value;

               if( tbetween < width )
               {
                  node->value = time_plus > start && time_plus < end ? 1. : 0.;
               }
               else
               {
                  node->value = time_plus > start && time_plus < start + width ? 1. : 0.;
               }
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child3 );
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1->child2 );
            node_deque.push_back( node->child1->child1 );
            continue;

         case CONSTANT_NODE:
            assert( false );
         }

         continue;
      }

      case STEP:
      {
         node->type           = NodeType( node->child1->type | node->child2->type | STATIC_NODE );
         double initial_time = initial_time_node->value;

         if( node->type != UNKNOWN )
         {
            if( node->child2->type != CONSTANT_NODE )
            {
               error( node->usages, "STEP used with non constant step time" );
            }
            else if( node->child2->value <= initial_time )
            {
               warning( node->usages,
                        "Usage of STEP has no effect because step time is at or before initial time" );
               node->init = node->child1->init;

               if( node->init == CONSTANT_INIT )
               {
                  node->value = node->child1->value;
               }
            }
            else
            {
               node->init  = CONSTANT_INIT;
               node->value = 0.0;
            }
         }

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level, getTimeNode()->level} ) + 1;
            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         case CONSTANT_NODE:
            assert( false );
         }

         continue;
      }

      case RAMP:
      {
         node->type = NodeType( node->child1->type | node->child2->type | node->child3->type | STATIC_NODE );

         if( node->type != UNKNOWN )
         {
            int non_const_count = ( node->child1->type != CONSTANT_NODE ) +
                                  ( node->child2->type != CONSTANT_NODE ) +
                                  ( node->child3->type != CONSTANT_NODE );

            if( non_const_count )
            {
               std::string msg {"Use of RAMP with "};

               switch( non_const_count )
               {
               case 3:
                  msg += "arguments one, two and three";
                  break;

               case 2:
               {
                  msg += "arguments ";

                  if( node->child1->type == CONSTANT_NODE )
                     msg += "two and three";

                  if( node->child2->type == CONSTANT_NODE )
                     msg += "one and three";

                  if( node->child3->type == CONSTANT_NODE )
                     msg += "one and two";

                  break;
               }

               case 1:
               {
                  msg += "argument ";

                  if( node->child1->type != CONSTANT_NODE )
                     msg += "one";

                  if( node->child2->type != CONSTANT_NODE )
                     msg += "two";

                  if( node->child3->type != CONSTANT_NODE )
                     msg += "three";
               }
               }

               msg += " not constant";
               error( node->usages, msg );
            }

            node->init  = CONSTANT_INIT;
            node->value = 0.0;
            node->level = std::max( {node->child1->level, node->child2->level, node->child3->level, getTimeNode()->level} ) + 1;;
            node_deque.pop_back();
            continue;
         }

         node_deque.push_back( node->child3 );
         node_deque.push_back( node->child2 );
         node_deque.push_back( node->child1 );
         continue;
      }

      case RANDOM_UNIFORM:
      {
         node->type = NodeType( node->child1->type | node->child2->type | STATIC_NODE );

         switch( node->type )
         {
         case DYNAMIC_NODE:
            error( node->usages, "RANDOM UNIFORM used with non constant arguments." );

         case STATIC_NODE:
            node->init  = CONSTANT_INIT;
            node->value = sdo::random_uniform( node->child1->value, node->child2->value );
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;
            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         case CONSTANT_NODE:
            assert( false );
         }
      };

      case PLUS:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value + node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         }
      }

      case MINUS:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value - node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case MULT:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value * node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case DIV:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value / node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         }
      }

      case G:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value > node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case GE:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value >= node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case L:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value < node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         }
      }

      case LE:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value <= node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         }
      }

      case EQ:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value == node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         }
      }

      case NEQ:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value != node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case AND:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value && node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case OR:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value || node->child2->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         }
      }

      case POWER:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::pow( node->child1->value, node->child2->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case LOG:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::log( node->child1->value ) / std::log( node->child2->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case MIN:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::min( node->child1->value, node->child2->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         }
      }

      case MAX:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::max( node->child1->value, node->child2->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;

         }
      }

      case MODULO:
      {
         node->type = NodeType( node->child1->type | node->child2->type );
         node->init = InitialType( node->child1->init | node->child2->init );

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = std::max( {node->child1->level, node->child2->level} ) + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->value -
                             node->child2->value * std::floor( node->child1->value / node->child2->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case UMINUS:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = -node->child1->value;
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case SQRT:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::sqrt( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case EXP:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::exp( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case LN:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::log( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case ABS:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::abs( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case INTEGER:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::floor( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case NOT:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = !( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case SIN:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::sin( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case COS:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::cos( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case TAN:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::tan( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case ARCSIN:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::asin( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case ARCCOS:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::acos( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case ARCTAN:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::atan( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case SINH:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::sinh( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case COSH:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::cosh( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case TANH:
      {
         node->type = node->child1->type;
         node->init = node->child1->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child1->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = std::tanh( node->child1->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child1 );
            continue;
         }
      }

      case TIME:
      {
         node->type  = STATIC_NODE;
         node->init  = CONSTANT_INIT;
         node->level = 1;
         node->value = initial_time_node->value;
         node_deque.pop_back();
         continue;
      }

      case CONSTANT:
      {
         assert( false );
         node_deque.pop_back();
         continue;
      }

      case CONTROL:
      {
         node->type  = DYNAMIC_NODE;
         node->init = CONTROLED_INIT;
         node->level = 0;
         node_deque.pop_back();
         continue;
      }

      case APPLY_LOOKUP:
      {
         if( node->child1->op != LOOKUP_TABLE )
         {
            error( node->child1->usages, "Symbol not a lookup table" );
         }

         node->type = node->child2->type;
         node->init = node->child2->init;

         switch( node->type )
         {
         case DYNAMIC_NODE:
         case STATIC_NODE:
            node->level = node->child2->level + 1;

            if( node->init == CONSTANT_INIT )
            {
            case CONSTANT_NODE:
               node->value = node->child1->lookup_table->operator()( node->child2->value );
            }

            node_deque.pop_back();
            continue;

         case UNKNOWN:
            node_deque.push_back( node->child2 );
            continue;
         }
      }

      case LOOKUP_TABLE:
      {
         assert( false );
         node_deque.pop_back();
         continue;
      }

      case NIL:
      {
         auto s = getSymbol( node );
         std::string msg;

         if( !s.empty() )
         {
            msg += "Use of undefined symbol '";
            msg += s.begin()->second.get();
            msg += "'";
         }
         else
         {
            msg += "Something has gone terribly wrong. NIL node found but it has no symbol attached";
         }

         error( node->usages, msg );
         node->type  = CONSTANT_NODE;
         node->value = 0.0;
         node->init  = CONSTANT_INIT;
         node->level = 0;
         node_deque.pop_back();
         continue;
      }
      }
   }


   if( initial_time_node->type != CONSTANT_NODE )
      error( initial_time_node->usages, "INITIAL TIME is not constant" );

   if( time_step_node->type != CONSTANT_NODE )
      error( time_step_node->usages, "TIME STEP is not constant" );

   if( final_time_node->type != CONSTANT_NODE )
      error( final_time_node->usages, "FINAL TIME is not constant" );

   if( hasErrors() )
      throw parse_error( *this );
}

void ExpressionGraph::useUniqueConstants( bool val )
{
   unique_constants = val;
}

void ExpressionGraph::substituteTmpNode( ExpressionGraph::Node *tmp, ExpressionGraph::Node *subst )
{
   auto eq_range = temp_node_usages_.equal_range( tmp );
   subst->usages.insert( subst->usages.end(), tmp->usages.begin(), tmp->usages.end() );

   if( eq_range.first != eq_range.second )
   {
      if( subst->op == NIL )
      {
         std::vector<Node **> usages;

         for( auto i = eq_range.first; i != eq_range.second; ++i )
         {
            *( i->second ) = subst;
            usages.emplace_back( i->second );
         }

         temp_node_usages_.erase( eq_range.first, eq_range.second );

         for( Node **usg : usages )
            temp_node_usages_.emplace( subst, usg );
      }
      else
      {
         for( auto i = eq_range.first; i != eq_range.second; ++i )
            *( i->second ) = subst;

         temp_node_usages_.erase( eq_range.first, eq_range.second );
      }

      node_pool_.free( tmp );
   }
}

ExpressionGraph::Node *ExpressionGraph::getNode( LookupTable *table )
{
   Node *a = node_pool_.construct();
   a->op           = LOOKUP_TABLE;
   a->lookup_table = table;
   a->type         = CONSTANT_NODE;
   a->level        = 0;
   auto b = nodes_.find( a );

   if( b != nodes_.end() )
   {
      node_pool_.free( a );
      lookup_pool_.destroy( table );
      return *b;
   }

   nodes_.emplace( a );
   return a;
}

double ExpressionGraph::evaluateNode( const Node *node, double time, bool initial ) const
{
   assert( node->type == STATIC_NODE || node->type == CONSTANT_NODE );
   std::stack<const Node *> nodes;
   nodes.push( nullptr );
   std::stack<double> vals;
   std::stack<int> valsoffset;
   valsoffset.push(0);

   auto push_ternary = [&]()
   {
      nodes.push( node );
      valsoffset.push(vals.size());
      nodes.push( node->child1 );
      valsoffset.push(vals.size());
      nodes.push( node->child2 );
      valsoffset.push(vals.size());
      node = node->child3;
   };

   auto push_binary = [&]()
   {
      nodes.push( node );
      valsoffset.push(vals.size());
      nodes.push( node->child1 );
      valsoffset.push(vals.size());
      node = node->child2;
   };

   auto push_unary = [&]()
   {
      nodes.push( node );
      valsoffset.push(vals.size());
      node = node->child1;
   };

   auto pop_node = [&]()
   {
      node = nodes.top();
      valsoffset.pop();
      nodes.pop();
   };

   double time_step = symbol_table.find( Symbol( "TIME STEP" ) )->second->value;
   double time_plus = time + time_step / 2;
   do
   {
      if( node->type == CONSTANT_NODE )
      {
         vals.push( node->value );
         pop_node();
      }
      else
      {
         switch( node->op )
         {
         case DELAY_FIXED:
            //TODO:
            continue;

         case CONTROL:
         case LOOKUP_TABLE:
         case NIL:
         case CONSTANT:
         case INTEG:
            assert( false );
            continue;

         case SQRT:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::sqrt( vals.top() );
               pop_node();
            }

            continue;

         case UMINUS:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = -vals.top();
               pop_node();
            }

            continue;

         case ACTIVE_INITIAL:
            if( initial )
               node = node->child2;
            else
               node = node->child1;

            continue;

         case APPLY_LOOKUP:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               auto &lkptbl = *( node->lookup_table );
               vals.top() = lkptbl( vals.top() );
               pop_node();
            }

            continue;

         case ARCCOS:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::acos( vals.top() );
               pop_node();
            }

            continue;

         case ARCSIN:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::asin( vals.top() );
               pop_node();
            }

            continue;

         case ARCTAN:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::atan( vals.top() );
               pop_node();
            }

            continue;

         case SIN:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::sin( vals.top() );
               pop_node();
            }

            continue;

         case COS:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::cos( vals.top() );
               pop_node();
            }

            continue;

         case TAN:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::tan( vals.top() );
               pop_node();
            }

            continue;

         case COSH:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::cosh( vals.top() );
               pop_node();
            }

            continue;

         case SINH:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::sinh( vals.top() );
               pop_node();
            }

            continue;

         case TANH:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::tanh( vals.top() );
               pop_node();
            }

            continue;

         case DIV:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = ( a / vals.top() );
               pop_node();
            }

            continue;

         case EQ:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = ( a == vals.top() );
               pop_node();
            }

            continue;

         case EXP:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::exp( vals.top() );
               pop_node();
            }

            continue;

         case ABS:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::abs( vals.top() );
               pop_node();
            }

            continue;

         case AND:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = a && vals.top();
               pop_node();
            }

            continue;

         case G:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = a > vals.top();
               pop_node();
            }

            continue;

         case GE:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = a >= vals.top();
               pop_node();
            }

            continue;

         case IF:
            if( vals.size() - valsoffset.top() < 3 )
            {
               push_ternary();
            }
            else
            {
               double cond = vals.top();
               vals.pop();
               double thenval = vals.top();
               vals.pop();
               vals.top() = cond ? thenval : vals.top();
               pop_node();
            }

            continue;

         case INITIAL:
            vals.push( node->value );
            pop_node();
            continue;

         case TIME:
            vals.push( time );
            pop_node();
            continue;

         case INTEGER:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::floor( vals.top() );
               pop_node();
            }

            continue;

         case L:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = a < vals.top();
               pop_node();
            }

            continue;

         case LE:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = a <= vals.top();
               pop_node();
            }

            continue;

         case LN:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = std::log( vals.top() );
               pop_node();
            }

            continue;

         case LOG:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = std::log( a ) / std::log( vals.top() );
               pop_node();
            }

            continue;

         case MAX:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = std::max( a, vals.top() );
               pop_node();
            }

            continue;

         case MIN:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = std::min( a, vals.top() );
               pop_node();
            }

            continue;

         case MINUS:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = a - vals.top();
               pop_node();
            }

            continue;

         case MODULO:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = std::fmod( a, vals.top() );
               pop_node();
            }

            continue;

         case MULT:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = a * vals.top();
               pop_node();
            }

            continue;

         case NEQ:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = a != vals.top();
               pop_node();
            }

            continue;

         case NOT:
            if( vals.size() - valsoffset.top() < 1 )
            {
               push_unary();
            }
            else
            {
               vals.top() = !vals.top();
               pop_node();
            }

            continue;

         case OR:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = a || vals.top();
               pop_node();
            }

            continue;

         case PLUS:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = a + vals.top();
               pop_node();
            }

            continue;

         case POWER:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = std::pow( a, vals.top() );
               pop_node();
            }

            continue;

         case PULSE:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double start = vals.top();
               vals.pop();
               double width = std::max( time_step, vals.top() );
               vals.top() = ( time_plus > start ) && ( time_plus < start + width ) ? 1 : 0;
               pop_node();
            }

            continue;

         case PULSE_TRAIN:
            if( vals.size() - valsoffset.top() < 4 )
            {
               nodes.push( node );
               valsoffset.push(vals.size());
               nodes.push( node->child1->child1 );
               valsoffset.push(vals.size());
               nodes.push( node->child1->child2 );
               valsoffset.push(vals.size());
               nodes.push( node->child2 );
               valsoffset.push(vals.size());
               node = node->child3;
            }
            else
            {
               pop_node();
               double start = vals.top();
               vals.pop();
               double width = std::max( time_step, vals.top() );
               vals.pop();
               double tbetween = vals.top();
               vals.pop();
               double end = vals.top();

               if( time_plus < start || end < time_plus )
               {
                  vals.top() = 0;
                  continue;
               }

               if( tbetween < width )
               {
                  vals.top() = 1;
                  continue;
               }

               double tmodplus = std::fmod( time_plus, tbetween );
               double smod = std::fmod( start, tbetween );

               vals.top() = ( tmodplus > smod ) && ( tmodplus < smod + width ) ? 1 : 0;
            }

            continue;

         case ExpressionGraph::STEP:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double height = vals.top();
               vals.pop();
               vals.top() = time_plus > vals.top() ? height : 0;
               pop_node();
            }

            continue;

         case ExpressionGraph::RAMP:
            if( vals.size() - valsoffset.top() < 3 )
            {
               push_ternary();
            }
            else
            {
               double slope = vals.top();
               vals.pop();
               double start_time = vals.top();
               vals.pop();
               vals.top() = time > start_time ?
                            ( time < vals.top() ?
                              slope * ( time - start_time ) :
                              slope * ( vals.top() - start_time ) )
                            : 0;
               pop_node();
            }

            continue;

         case ExpressionGraph::RANDOM_UNIFORM:
            if( vals.size() - valsoffset.top() < 2 )
            {
               push_binary();
            }
            else
            {
               double a = vals.top();
               vals.pop();
               vals.top() = sdo::random_uniform( a, vals.top() );
               pop_node();
            }

            continue;
         }
      }
   }
   while( node );
   assert(nodes.empty());
   assert(vals.size() == 1 && valsoffset.empty());
   return vals.top();
}

}
