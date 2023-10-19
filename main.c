#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include "List.h"
#include "Queue.h"

#ifndef DBG_HELP
#define DBG_HELP 1
#endif  

#if DBG_HELP > 0
#define DBG_PRINT( ... ) do{ fprintf( stderr, "DBG:" __VA_ARGS__ ); } while( 0 )
#else
#define DBG_PRINT( ... ) ;
#endif  


// Aunque en este ejemplo estamos usando tipos básicos, vamos a usar al alias |Item| para resaltar
// aquellos lugares donde estamos hablando de DATOS y no de índices.
typedef int Item;

/**
* @brief Colores para
*/
typedef enum
{
   WHITE, ///< vértice
   GRAY,  ///< vértice
   BLACK, ///< vértice
} eGraphColors;


//----------------------------------------------------------------------
//                           Vertex stuff:
//----------------------------------------------------------------------


/**
 * @brief Declara lo que es un vértice.
 */
typedef struct
{
   Item data;
   List* neighbors;
   
   int distance;
   int predecessor;
   eGraphColors color;
   
   int discovery_time;
   int finish_time;
   
} Vertex;

bool Vertex_HasNeighbors( Vertex* v )
{
   assert( v );

   return v->neighbors;
}

/**
 * @brief Hace que cursor libre apunte al inicio de la lista de vecinos. Se debe
 * de llamar siempre que se vaya a iniciar un recorrido de dicha lista.
 *
 * @param v El vértice de trabajo (es decir, el vértice del cual queremos obtener
 * la lista de vecinos).
 */
void Vertex_Start( Vertex* v )
{
   assert( v );

   List_Cursor_front( v->neighbors );
}

/**
 * @brief Mueve al cursor libre un nodo adelante.
 *
 * @param v El vértice de trabajo.
 *
 * @pre El cursor apunta a un nodo válido.
 * @post El cursor se movió un elemento a la derecha en la lista de vecinos.
 */
void Vertex_Next( Vertex* v )
{
   List_Cursor_next( v->neighbors );
}

/**
 * @brief Indica si se alcanzó el final de la lista de vecinos.
 *
 * @param v El vértice de trabajo.
 *
 * @return true si se alcanazó el final de la lista; false en cualquier otro
 * caso.
 */
bool Vertex_End( const Vertex* v )
{
   return List_Cursor_end( v->neighbors );
}


/**
 * @brief Devuelve el índice del vecino al que apunta actualmente el cursor en la lista de vecinos
 * del vértice |v|.
 *
 * @param v El vértice de trabajo (del cual queremos conocer el índice de su vecino).
 *
 * @return El índice del vecino en la lista de vértices.
 *
 * @pre El cursor debe apuntar a un nodo válido en la lista de vecinos.
 *
 * Ejemplo
 * @code
   Vertex* v = Graph_GetVertexByKey( grafo, 100 );
   for( Vertex_Start( v ); !Vertex_End( v ); Vertex_Next( v ) )
   {
      int index = Vertex_GetNeighborIndex( v );

      Item val = Graph_GetDataByIndex( g, index );

      // ...
   }
   @endcode
   @note Esta función debe utilizarse únicamente cuando se recorra el grafo con las funciones
   Vertex_Start(), Vertex_End() y Vertex_Next().
 */
Data Vertex_GetNeighborIndex( const Vertex* v )
{
   return List_Cursor_get( v->neighbors );
}

void Vertex_SetColor( Vertex* v, eGraphColors color )
{
   v->color = color;
}

eGraphColors Vertex_GetColor( Vertex* v )
{
   return v->color;
}

int Vertex_GetData( const Vertex* v )
{
   return v->data;
}

void Vertex_SetPredecessor( Vertex* v, int predecessor_idx )
{
    v->predecessor = predecessor_idx;
}

int Vertex_GetPredecessor( const Vertex* v )
{
    return v->predecessor;
}

void Vertex_SetDiscovery_time( Vertex* v, int time )
{
    v->discovery_time = time;
}

int Vertex_GetDiscovery_time( const Vertex* v )
{
    return v->discovery_time;
}

void Vertex_SetFinish_time( Vertex* v, int time )
{
    v->finish_time = time;
}

int Vertex_GetFinish_time( const Vertex* v )
{
    return v->finish_time;
}



//----------------------------------------------------------------------
//                           Graph stuff:
//----------------------------------------------------------------------

/** Tipo del grafo.
 */
typedef enum
{
   eGraphType_UNDIRECTED, ///< grafo no dirigido
   eGraphType_DIRECTED    ///< grafo dirigido (digraph)
} eGraphType;

/**
 * @brief Declara lo que es un grafo.
 */
typedef struct
{
   Vertex* vertices; ///< Lista de vértices
   int size;         ///< Tamaño de la lista de vértices

   /**
    * Número de vértices actualmente en el grafo.
    * Como esta versión no borra vértices, lo podemos usar como índice en la
    * función de inserción
    */
   int len;  

   eGraphType type; ///< tipo del grafo, UNDIRECTED o DIRECTED
} Graph;

//----------------------------------------------------------------------
//                     Funciones privadas
//----------------------------------------------------------------------

// vertices: lista de vértices
// size: número de elementos en la lista de vértices
// key: valor a buscar
// ret: el índice donde está la primer coincidencia; -1 si no se encontró
static int find( Vertex vertices[], int size, int key )
{
   for( int i = 0; i < size; ++i )
   {
      if( vertices[ i ].data == key ) return i;
   }

   return -1;
}

// busca en la lista de vecinos si el índice del vértice vecino ya se encuentra ahí
static bool find_neighbor( Vertex* v, int index )
{
   if( v->neighbors )
   {
      return List_Find( v->neighbors, index );
   }
   return false;
}

// vertex: vértice de trabajo
// index: índice en la lista de vértices del vértice vecino que está por insertarse
static void insert( Vertex* vertex, int index, float weigth )
{
   // crear la lista si no existe!
   
   if( !vertex->neighbors )
   {
      vertex->neighbors = List_New();
   }

   if( vertex->neighbors && !find_neighbor( vertex, index ) )
   {
      List_Push_back( vertex->neighbors, index, weigth );

      DBG_PRINT( "insert():Inserting the neighbor with idx:%d\n", index );
   }
   else DBG_PRINT( "insert: duplicated index\n" );
}


//----------------------------------------------------------------------
//                     Funciones públicas
//----------------------------------------------------------------------


/**
 * @brief Crea un nuevo grafo.
 *
 * @param size Número de vértices que tendrá el grafo. Este valor no se puede
 * cambiar luego de haberlo creado.
 *
 * @return Un nuevo grafo.
 *
 * @pre El número de elementos es mayor que 0.
 */
Graph* Graph_New( int size, eGraphType type )
{
   assert( size > 0 );

   Graph* g = (Graph*) malloc( sizeof( Graph ) );
   if( g )
   {
      g->size = size;
      g->len = 0;
      g->type = type;

      g->vertices = (Vertex*) calloc( size, sizeof( Vertex ) );

      if( !g->vertices )
      {
         free( g );
         g = NULL;
      }
   }

   return g;
   // el cliente es responsable de verificar que el grafo se haya creado correctamente
}

void Graph_Delete( Graph** g )
{
   assert( *g );

   Graph* graph = *g;
   // para simplificar la notación

   for( int i = 0; i < graph->size; ++i )
   {
      Vertex* vertex = &graph->vertices[ i ];
      // para simplificar la notación.
      // La variable |vertex| sólo existe dentro de este for.

      if( vertex->neighbors )
      {
         List_Delete( &(vertex->neighbors) );
      }
   }

   free( graph->vertices );
   free( graph );
   *g = NULL;
}

/**
 * @brief Imprime un reporte del grafo
 *
 * @param g     El grafo.
 * @param depth Cuán detallado deberá ser el reporte (0: lo mínimo)
 */
void Graph_Print( Graph* g, int depth )
{
   for( int i = 0; i < g->len; ++i )
   {
      Vertex* vertex = &g->vertices[ i ];
      // para simplificar la notación.

      printf( "[%d]%d=>", i, vertex->data );
      if( vertex->neighbors )
      {
         for( List_Cursor_front( vertex->neighbors );
              ! List_Cursor_end( vertex->neighbors );
              List_Cursor_next( vertex->neighbors ) )
         {

            Data d = List_Cursor_get( vertex->neighbors );
            int neighbor_idx = d.index;

            printf( "%d->", g->vertices[ neighbor_idx ].data );
         }
      }
      printf( "Nil\n" );

   }
   printf( "\n" );
}

/**
 * @brief Crea un vértice a partir de los datos reales.
 *
 * @param g     El grafo.
 * @param data  Es la información.
 */
void Graph_AddVertex( Graph* g, int data )
{
   assert( g->len < g->size );

   Vertex* vertex = &g->vertices[ g->len ];
   // para simplificar la notación

   vertex->data      = data;
   vertex->neighbors = NULL;

   ++g->len;
}

int Graph_GetSize( Graph* g )
{
   return g->size;
}


/**
 * @brief Inserta una relación de adyacencia del vértice |start| hacia el vértice |finish|.
 *
 * @param g      El grafo.
 * @param start  Vértice de salida (el dato)
 * @param finish Vertice de llegada (el dato)
 *
 * @return false si uno o ambos vértices no existen; true si la relación se creó con éxito.
 *
 * @pre El grafo no puede estar vacío.
 */
bool Graph_AddEdge( Graph* g, int start, int finish )
{
   assert( g->len > 0 );

   // obtenemos los índices correspondientes:
   int start_idx = find( g->vertices, g->size, start );
   int finish_idx = find( g->vertices, g->size, finish );

   DBG_PRINT( "AddEdge(): from:%d (with index:%d), to:%d (with index:%d)\n", start, start_idx, finish, finish_idx );

   if( start_idx == -1 || finish_idx == -1 ) return false;
   // uno o ambos vértices no existen

   insert( &g->vertices[ start_idx ], finish_idx, 0.0 );
   // insertamos la arista start-finish

   if( g->type == eGraphType_UNDIRECTED ) insert( &g->vertices[ finish_idx ], start_idx, 0.0 );
   // si el grafo no es dirigido, entonces insertamos la arista finish-start

   return true;
}


int Graph_GetLen( const Graph* g )
{
   return g->len;
}


/**
 * @brief Devuelve la información asociada al vértice indicado.
 *
 * @param g          Un grafo.
 * @param vertex_idx El índice del vértice del cual queremos conocer su información.
 *
 * @return La información asociada al vértice vertex_idx.
 */
Item Graph_GetDataByIndex( const Graph* g, int vertex_idx )
{
   assert( 0 <= vertex_idx && vertex_idx < g->len );

   return g->vertices[ vertex_idx ].data;
}

/**
 * @brief Devuelve una referencia al vértice indicado.
 *
 * Esta función puede ser utilizada con las operaciones @see Vertex_Start(), @see Vertex_End(), @see Vertex_Next().
 *
 * @param g          Un grafo
 * @param vertex_idx El índice del vértice del cual queremos devolver la referencia.
 *
 * @return La referencia al vértice vertex_idx.
 */
Vertex* Graph_GetVertexByIndex( const Graph* g, int vertex_idx )
{
   assert( 0 <= vertex_idx && vertex_idx < g->len );

   return &(g->vertices[ vertex_idx ] );
}

/**
 * @brief Devuelve una referencia al vértice indicado.
 *
 * Esta función puede ser utilizada con las operaciones @see Vertex_Start(), @see Vertex_End(), @see Vertex_Next().
 *
 * @param g   Un grafo
 * @param key Llave de búsqueda (esto es, el |dato|) del vértice del cual queremos devolver la referencia.
 *
 * @return La referencia al vértice que coincida con key (esto es, con el |dato|).
 */
Vertex* Graph_GetVertexByKey( const Graph* g, Item key )
{
   for( int i = 0; i < Graph_GetLen( g ); ++i )
   {
      if( g->vertices[ i ].data == key )
      {
         return &(g->vertices[i]);
      }
   }
   
   return NULL;
}

int Graph_Size( Graph* g )
{
   return g->size;
}


//----------------------------------------------------------------------
//                          dfs_traverse()
//----------------------------------------------------------------------
#define MAX_VERTICES 9
void dfs_topol_traverse( Graph* g, Vertex* v, int* pTiempo, Queue* listado)
{
   *pTiempo += 1;
   Vertex_SetDiscovery_time(v, *pTiempo);
   Vertex_SetColor(v, GRAY);
   
   if( Vertex_HasNeighbors( v ) )
   {
      for( Vertex_Start( v ); ! Vertex_End( v ); Vertex_Next( v ) )
      {
         Vertex* w = Graph_GetVertexByIndex( g, Vertex_GetNeighborIndex( v ).index );

         if( Vertex_GetColor( w ) == WHITE )
         {
            DBG_PRINT( "Visiting vertex: (p:%d)->%d\n", Vertex_GetData( v ), Vertex_GetData( w ) );

            Vertex_SetColor( w, GRAY );
            Vertex_SetPredecessor(w, Vertex_GetData(v));

            dfs_topol_traverse( g, w, pTiempo, listado);
         }
      }
      DBG_PRINT( "Returning to: %d\n", Vertex_GetData( v ) );
   }
   else
   {
      DBG_PRINT( "Vertex %d doesn't have any neighbors\n", Vertex_GetData( v ) );
   }

   Vertex_SetColor( v, BLACK );
   *pTiempo += 1;
   Vertex_SetDiscovery_time(v,*pTiempo);
   
   Queue_Enqueue( &listado, v->data );
}

void dfs_topol( Graph* g, int start ){
   for( int i = 0; i < Graph_GetLen( g ); ++i )
   {
      Vertex* v = Graph_GetVertexByIndex( g, i );

      Vertex_SetColor( v, WHITE );
      Vertex_SetPredecessor( v, -1 );
      Vertex_SetDiscovery_time(v, 0);
      Vertex_SetFinish_time(v, 0);
   }

   Queue* lista = Queue_New(MAX_VERTICES);

   Vertex_SetColor( Graph_GetVertexByKey( g, start ), GRAY );
   DBG_PRINT( "Visiting start node: %d\n", start );
   
   int time_ = 0;
   dfs_topol_traverse( g, Graph_GetVertexByKey( g, start), &time_ , &lista);
   
   for( int i = 0; i < Queue_Len(lista); ++i )
   {
      int guardado = Queue_Dequeue(lista);
      Vertex* v = Graph_GetVertexByKey( g, guardado );

      printf( "[%d] (%d) -- Pred: %d\n",
            i,
            Vertex_GetData( v ),
            Vertex_GetPredecessor( v ) );
   }
}

int main()
{
   Graph* grafo = Graph_New(
         MAX_VERTICES,            // cantidad máxima de vértices
         eGraphType_DIRECTED );   // será un grafo dirigido


   // crea los vértices. El orden de inserción no es importante
   Graph_AddVertex( grafo, 100 ); //A
   Graph_AddVertex( grafo, 200 ); //B
   Graph_AddVertex( grafo, 300 ); //C
   Graph_AddVertex( grafo, 400 ); //D
   Graph_AddVertex( grafo, 500 ); //E
   Graph_AddVertex( grafo, 600 ); //F
   Graph_AddVertex( grafo, 700 ); //G
   Graph_AddVertex( grafo, 800 ); //H
   Graph_AddVertex( grafo, 900 ); //I

   // crea las aristas (conexiones entre vértices):
   Graph_AddEdge( grafo, 100, 400 );
   Graph_AddEdge( grafo, 200, 400 );
   Graph_AddEdge( grafo, 300, 400 );
   Graph_AddEdge( grafo, 400, 600 );
   Graph_AddEdge( grafo, 400, 800 );
   Graph_AddEdge( grafo, 500, 600 );
   Graph_AddEdge( grafo, 600, 700 );
   Graph_AddEdge( grafo, 700, 900 );
   Graph_AddEdge( grafo, 800, 900 );

   Graph_Print( grafo, 0 );
   // imprime el grafo completo (esta versión no usa al segundo argumento)

   dfs_topol( grafo, 100 );

   Graph_Delete( &grafo );
   assert( grafo == NULL );
}