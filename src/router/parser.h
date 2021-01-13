#ifndef PARSER_H
#define PARSER_H
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cmath>
#include <climits>
#include <math.h>
#include <time.h>
#include<map>
#include<list>
#include <math.h>
#include <sstream>
using namespace std;

struct parser_GGridBoundaryIdx{
	int rowBeginIdx = 0; 
	int colBeginIdx = 0;
	int rowEndIdx = 0;
	int colEndIdx = 0;
};
struct parser_Lay{
	string layerName; // M1
	int Idx; // 1
	char RoutingDirection; // H
	int defaultSupplyOfOneGGrid; // 10
};
struct parser_NonDefaultSupplyGGrid{
	int rowIdx;
	int colIdx;
	int LayIdx;
	int incrOrDecrValue;
};

struct parser_cellPin{
	string pinName;
	string pinLayer;
};
struct parser_Blkg{
	string blockageName;
	string blockageLayer;
	int demand;
};
struct parser_MasterCell{
	string masterCellName;
	int pinCount;
	int blockageCount;
	vector<parser_cellPin> v_pin;
	vector<parser_Blkg> v_blkg;
};
struct parser_NeighborCellExtraDemand{
	string type;
	string masterCellName1;
	string masterCellName2;
	string layerName;
	int demand;
};
struct parser_CellInst{
	string instName;
	string masterCellName;
	int gGridRowIdx; // x
	int gGridColIdx; // y
	string movableCstr; // Moveable or Fixed
	bool fixed=true;
};
struct parser_netPin{
	string instName;
	string masterPinName;
};
struct parser_Net{
	string netName;
	int numPins;
	string minRoutingLayConstraint; // matal, M3代表只能在M1~M3繞
	vector<parser_netPin> v_pin;
};
struct parser_RouteSegment{
	int sRowIdx;
	int sColIdx;
	int sLayIdx;
	int eRowIdx;
	int eColIdx;
	int eLayIdx;
	string netName;
};

class parser_C{
public:
	parser_C(char*);
	void print_info();

	//------------- parameters ------------------
	// maxMoveNum
	int maxMoveCount = 0;
	// gGrid Boundary 
	parser_GGridBoundaryIdx gGridBoundaryIdx; // (x1,y1) (x2,y2)
	// Layer
	int LayerCount = 0; // NumLayer
	vector<parser_Lay> v_Layer;
	// nonDefault gGrid supply (default is in the Layer define)
	int nonDefaultSupplyGGridCount = 0;
	vector<parser_NonDefaultSupplyGGrid> v_nonDefaultSupplyGGrid;
	// master cell
	int masterCellCount = 0;
	vector<parser_MasterCell> v_masterCell;
	// NeighborCellExtraDemand
	int neighborCellExtraDemandCount = 0;
	vector<parser_NeighborCellExtraDemand> v_neighborCellExtraDemand;
	// cellInst
	int cellInstCount = 0;
	vector<parser_CellInst> v_cellInst;
	// Net
	int netCount = 0;
	vector<parser_Net> v_net;
	// RouteSegment
	int routeSegmentCount = 0;
	vector<parser_RouteSegment> v_routeSegment;
};


#endif
