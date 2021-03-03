#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <math.h>
#include <bblock/subgraphs.h>
#include <iostream>
#include <vector>
using namespace std;


SubGraphs::SubGraphs(int nVert, int subgraphSize, int** neighborlist, int nVertStop)
  {

  this->subgraphSize=subgraphSize;
  this->subgraphCounter=0;
  this->nVert=nVert;
  this->nVertStop=nVertStop;
  
  this->subgraph = new int*[this->subgraphSize];
  for (int i = 0; i < this->subgraphSize; i++)
    this->subgraph[i] = new int[this->subgraphSize+1];

  this->neighborlist = new int*[this->nVert];
  for(int i = 0; i < this->nVert; i++){ 
    this->neighborlist[i] = new int[neighborlist[i][0]+1];
    for(int j = 0;j < neighborlist[i][0]+1; j++)
      this->neighborlist[i][j] = neighborlist[i][j];
  }
  
  this->maxDegree=-1;
  
  for (int i = 0; i < this->nVert; i++)
    this->maxDegree=max(this->maxDegree,neighborlist[i][0]);
  

  this->Visited = new bool[this->nVert+1];
  for(int i = 1; i <= this->nVert; i++)
    this->Visited[i] = false;
  
  childSet = new unsigned int*[this->subgraphSize];
  Index    = new unsigned int*[this->subgraphSize];
	
  for(int i = 0; i < this->subgraphSize; i++) {
    childSet[i] = new unsigned int[this->maxDegree * this->subgraphSize + 1];
    Index[i] = new unsigned int[this->maxDegree * this->subgraphSize + 1];
  }

}

SubGraphs::~SubGraphs()
{
  
  for(int i = 0; i < this->subgraphSize; i++) {
    delete [] this->Index[i];
    delete [] this->childSet[i];
    delete [] this->subgraph[i];
  }
  delete [] this->subgraph;
  delete [] this->Index;
  delete [] this->childSet;
  delete [] this->Visited;

  for(int i = 0; i < this->nVert; i++){ 
    delete[] this->neighborlist[i];
  }
  delete[] this->neighborlist;

}


// SubGraphs::~SubGraphs()
// {
//   for(int i = 0; i < subgraphSize; i++) {
//     delete [] Index[i];
//     delete [] childSet[i];
//   }
	
//   for (int i = 0; i < subgraphSize; i++)
//     delete [] this->subgraph[i];
  
//   delete [] this->subgraph;
//   delete [] Index;
//   delete [] childSet;
//   delete [] this->Visited;   
//   delete g;
// }
// void SubGraphs::AddSubgraph(){
//   // number of arrays
//   int n = this->subgraphSize;
 
//   // to keep track of next element in each of
//   // the n arrays
//   int* indices = new int[n];
//   // initialize with first element's index
//   for (int i = 0; i < n; i++)
//     indices[i] = 1;
//   while (1) {
 
//     // print current combination
//     for (int i = 0; i < n; i++)
//       cout << subgraph[i][indices[i]] << " ";
//     cout << endl;
 
//     // find the rightmost array that has more
//     // elements left after the current element 
//     // in that array
//     int next = n - 1;
//     while (next >= 1 && 
// 	   (indices[next] + 1 >= subgraph[next][0]))
//       next--;
 
//     // no such array is found so no more 
//     // combinations left
//     if (next < 0)
//       return;
 
//     // if found move to next element in that 
//     // array
//     indices[next]++;
 
//     // for all arrays to the right of this 
//     // array current index again points to 
//     // first element
//     for (int i = next + 1; i < n; i++)
//       indices[i] = 1;
//   }
// }


// bool SubGraphs::ReadData(const char *path) {
// 	register int i, j;
// 	int graphSize;
// 	FILE * inFile = fopen(path, "r");
	
// 	if (inFile == NULL) {
// 		printf("Error opening %s file.\n", path);
// 		return false;
// 	}
	
// 	if (!feof(inFile))
// 		fscanf(inFile, "%d\n", &graphSize);	
	
		
	
// 	g = new Graph(graphSize, this->subgraphSize);
// 	while (!feof(inFile)) {
// 		fscanf(inFile, "%d %d\n", &i, &j);
// 		if(i == j) continue;
// 		g->addEdge(i, j);
// 	}
	
// 	g->Finalize();
	
// 	//g->Print();
	
// 	fclose(inFile);

// 	// childSet = new unsigned int*[this->subgraphSize];
// 	// Index    = new unsigned int*[this->subgraphSize];
	
// 	// for(int i = 0; i < this->subgraphSize; i++) {
// 	// 	childSet[i] = new unsigned int[this->maxDegree * this->subgraphSize + 1];
// 	// 	Index[i] = new unsigned int[this->maxDegree * this->subgraphSize + 1];
// 	// }

// 	return true;
// }

/***************************************************************
 * This function finds the valid children in each given level. *
***************************************************************/

void SubGraphs::initChildSet(int root, int level) {
  register int *N;
  register int i, j;
  const int *parents = this->subgraph[level-1];

#ifdef Debug
  std::cout<<"init_child_set, nvert="<<this->nVert<<endl;
  for(i = 0; i < this->nVert; i++){
    for(j = 0; j <= neighborlist[i][0]; j++){
      std::cout<<neighborlist[i][j]<<" ";
    }
    std::cout<<endl;
  }
  std::cout<<"done init_child_set"<<endl;
#endif
  childSet[level][0] = 0;
  for(i = 1; i <= parents[0]; i++) {
    N = this->neighborlist[parents[i]-1];
    for(j = 1; j <= N[0] && root <= N[j]; j++) {
      if(!this->Visited[N[j]]) {
	this->Visited[N[j]] = true;
	childSet[level][0]++;
	childSet[level][childSet[level][0]] = N[j];				
      }
    }		
  }
}

/****************************************************************************************
 * This function Explores the root vertex and generates subgraphs containing that node. *
****************************************************************************************/

void SubGraphs::Explore(int root, int level, int reminder) {
	register int i, j, k; // k is the number of selected nodes in the current level.
	//cout<<this->subgraphCounter<<" "<<root<<" "<<reminder<<" "<<level<<" "<<endl;

// #ifdef Debug
// 	printf("************************************\n");
// 	printf("*****   Exploring level %3d  *******\n", level);
// 	printf("************************************\n");
// #endif
	//cout<<this->subgraphCounter<<" "<<root<<" "<<reminder<<" "<<level<<" "<<endl;
	if (reminder == 0) { //reminder == 0 assures level <= this->subgraphSize
		this->subgraphCounter++;
		 
		//#ifdef Debug
		//		printf("--> Subgraph Number %d: \n", this->subgraphCounter);
		vector<int> subgraph_vec;
	   
		for(i = 0; i < level; i++) {
		  //		printf("Level %d: ", i);
			for(k = 1; k <= this->subgraph[i][0]; k++) {
			  //		printf("%d ", this->subgraph[i][k]);
				
				subgraph_vec.push_back(this->subgraph[i][k]);
				
				  //k_subgraphs.pushback(); 
			}
			
			//	printf("\n");
		}
		this->k_subgraphs.push_back(subgraph_vec);
		//printf("\n");
		//printf("------------------------------------\n");
		//#endif
		
		//g->Classify(this->subgraph, level);
		
		return;
	}
	
	
	initChildSet(root, level); 
	
// #ifdef Debug
// 	printf("Valid Children in level %d:\nN = { ", level);
// 	for(k = 1; k <= childSet[level][0]; k++) {
// 		printf("%d ", childSet[level][k]);
// 	}		
// 	printf("}\n");
// #endif

	for(k = 1; k <= reminder; k++) {
		if( childSet[level][0] < k ) { //There is not enough child to choose m from.
		  for(i = 1; i <= childSet[level][0]; i++) {
			  this->Visited[childSet[level][i]] = false;
			}
			return;
		}

// #ifdef Debug
// 		printf("Selecting %d node(s) from level %d\n", k, level);
// 		printf("Initial Selection = { ");
// 		for(i = 1; i <= k; i++) {
// 			printf("%d ", childSet[level][i]);
// 		}
// 		printf("}\n");
// #endif
		this->subgraph[level][0] = k;
		for(i = 1; i <= k; i++) {
			this->subgraph[level][i] = childSet[level][i];
			Index[level][i] = i;
		}	
		
		Explore(root, level + 1, reminder - k);
	    GEN( childSet[level][0], k, root, level, reminder, k);

#ifdef Debug
		printf("************************************\n");
		printf("*****    Back to level %3d   *******\n", level);
		printf("************************************\n");
#endif
	}
	
	for(i = 1; i <= childSet[level][0]; i++) {
		this->Visited[childSet[level][i]] = false;
	}
	this->subgraph[level][0] = 0;
	return;
}

/***************************************************************************************************
 * The following three functions generate all C(n, k) in Gray Code order, Adopted from Rusky code. *
***************************************************************************************************/

void SubGraphs::swap( int i, int j, int root, int level, int reminder, int m) {
#ifdef Debug
	printf("Switch %d with %d in level %d\n", childSet[level][j], childSet[level][i], level);
#endif

	Index[level][i] = Index[level][j];
	this->subgraph[level][Index[level][i]] = childSet[level][i];	
	Explore(root, level + 1, reminder - m);
}

/****************************************************************
****************************************************************/

void SubGraphs::GEN( int n, int k, int root, int level, int reminder, int m) {	
	if (k > 0 && k < n) {
    	GEN( n-1, k, root, level, reminder, m);
		if (k == 1) 
			swap( n, n-1, root, level, reminder, m);  
		else 
			swap( n, k-1, root, level, reminder, m);
    
		NEG( n-1, k-1, root, level, reminder, m);
    }
}

/****************************************************************
****************************************************************/

void SubGraphs::NEG( int n, int k, int root, int level, int reminder, int m) {	
	if (k > 0 && k < n) {
    	GEN( n-1, k-1, root, level, reminder, m);
    	
		if (k == 1) 
			swap( n-1, n, root, level, reminder, m);  
		else 
			swap( k-1, n, root, level, reminder, m);
    
		NEG( n-1, k, root, level, reminder, m);
	}
	
}

/***********************************************************************************
 * This function enumerates the subgraphs related to each vertex of inpur network. *
***********************************************************************************/
void SubGraphs::Enumerate() {


  register int v;

	for (v = 1; v <= this->nVert; v++)
	{

	  if(v>this->nVertStop)
	    break;

#ifdef Debug
	  printf("+ Exploring Node %d ...\n", v);
#endif Debug
	  this->subgraph[0][0] = 1;
	  this->subgraph[0][1] = v;
	  
	  this->Visited[v] = true;
	  //cout<<this->subgraphSize - 1<<endl;
	  Explore(v, 1, this->subgraphSize - 1);
	  this->Visited[v] = false;
	}	
} 

