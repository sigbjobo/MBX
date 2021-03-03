#include <stdio.h>
/* #include <stdlib.h> */
/* #include <map> */
/* #include <math.h> */
/* //#include <subgraphs.h> */
/* #include <iostream> */
#include <vector>
using namespace std;
// #include <vector> 
class SubGraphs
{
 private:
  int  **subgraph;
  int subgraphSize;
  int  num_random_graphs;
  bool *Visited;
  unsigned int **childSet;
  unsigned int **Index;  
  int** neighborlist;

  long long subgraphCounter; 
  //Graph *g;
  long num;  
  bool isRand;
  int maxDegree;
  int nVert;
  int nVertStop;

  std::vector<std::vector<int>> k_subgraphs;
  
 public:
  SubGraphs(int nVert, int subgraphSize, int** neighborlist, int nVertStop);
  ~SubGraphs();
  /* ~SubGraphs(){ */
  /*   for(int i = 0; i < subgraphSize; i++) { */
  /*     delete [] Index[i]; */
  /*     delete [] childSet[i]; */
  /*   } */
	
  /*   for (int i = 0; i < subgraphSize; i++) */
  /*     delete [] this->subgraph[i]; */
  
  /*   delete [] this->subgraph; */
  /*   delete [] Index; */
  /*   delete [] childSet; */
  /*   delete [] this->Visited;    */
  /* }; */
  bool ReadData(const char *path);
  std::vector<std::vector<int>> GetSubgraphs(){return k_subgraphs;};
  void Explore(int root, int level, int reminder);
  void GEN ( int n, int k, int root, int level, int reminder, int m);
  void NEG ( int n, int k, int root, int level, int reminder, int m);
  void initChildSet(int root, int level);
  void swap( int i, int j, int root, int level, int reminder, int m);
  void Enumerate();
  void AddSubgraph();
  long long Get_SubgraphCounter(){return subgraphCounter;}; 
  //Graph* Get_Graph(){return g;}; 

};
