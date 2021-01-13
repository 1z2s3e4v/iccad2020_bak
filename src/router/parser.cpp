#include "parser.h"
#include <sstream>

bool cmpLayer(const parser_Lay l, const parser_Lay r){ 
	return l.Idx < r.Idx; 
}

parser_C::parser_C(char* fileName){
	cout << "\033[94m[Parser]\033[0m - Parsing \"" << fileName << "\"...\n";
	ifstream file(fileName);
	string line,label;
	while(getline(file,line)){
		//cout << "[Parser] - print: " << line << "\n";
		stringstream ss(line);
		ss >> label;
		if(label == "MaxCellMove"){ ss >> maxMoveCount; }
		else if(label == "GGridBoundaryIdx"){
			ss >> gGridBoundaryIdx.rowBeginIdx >> gGridBoundaryIdx.colBeginIdx >> gGridBoundaryIdx.rowEndIdx >> gGridBoundaryIdx.colEndIdx;
		}
		else if(label == "NumLayer"){
			ss >> LayerCount;
			for(int i=0;i<LayerCount;++i){
				getline(file,line);
				stringstream ss2(line);
				ss2 >> label;
				
				parser_Lay layer;
				ss2 >> layer.layerName >> layer.Idx >> layer.RoutingDirection >> layer.defaultSupplyOfOneGGrid;
				v_Layer.emplace_back(layer);
			}
			sort(v_Layer.begin(),v_Layer.end(),cmpLayer); // to play it save
		}
		else if(label == "NumNonDefaultSupplyGGrid"){
			ss >> nonDefaultSupplyGGridCount;
			for(int i=0;i<nonDefaultSupplyGGridCount;++i){
				getline(file,line);
				stringstream ss2(line);
				
				parser_NonDefaultSupplyGGrid ndsgg;
				string incrOrDecrValue;
				int sign_of_incrOrDecrValue = 1;
				ss2 >> ndsgg.rowIdx >> ndsgg.colIdx >> ndsgg.LayIdx >> incrOrDecrValue;
				ndsgg.incrOrDecrValue = stoi(incrOrDecrValue);
				v_nonDefaultSupplyGGrid.emplace_back(ndsgg);
			}
		}
		else if(label == "NumMasterCell"){
			ss >> masterCellCount;
			for(int i=0;i<masterCellCount;++i){
				getline(file,line);
				stringstream ss2(line);
				ss2 >> label;
				
				parser_MasterCell mc;
				ss2 >> mc.masterCellName >> mc.pinCount >> mc.blockageCount;
				for(int j=0;j<mc.pinCount;++j){
					getline(file,line);
					stringstream ss3(line);
					ss3 >> label;

					parser_cellPin pin;
					ss3 >> pin.pinName >> pin.pinLayer;
					mc.v_pin.emplace_back(pin);
				}
				for(int j=0;j<mc.blockageCount;++j){
					getline(file,line);
					stringstream ss3(line);
					ss3 >> label;

					parser_Blkg blkg;
					ss3 >> blkg.blockageName >> blkg.blockageLayer >> blkg.demand;
					mc.v_blkg.emplace_back(blkg);
				}
				v_masterCell.emplace_back(mc);
			}
		}
		else if(label == "NumNeighborCellExtraDemand"){
			ss >> neighborCellExtraDemandCount;
			for(int i=0;i<neighborCellExtraDemandCount;++i){
				getline(file,line);
				stringstream ss2(line);

				parser_NeighborCellExtraDemand nced;
				ss2 >> nced.type >> nced.masterCellName1 >> nced.masterCellName2 >> nced.layerName >> nced.demand;
				v_neighborCellExtraDemand.emplace_back(nced);
			}
		}
		else if(label == "NumCellInst"){
			ss >> cellInstCount;
			for(int i=0;i<cellInstCount;++i){
				getline(file,line);
				stringstream ss2(line);
				ss2 >> label;

				parser_CellInst cell;
				ss2 >> cell.instName >> cell.masterCellName >> cell.gGridRowIdx >> cell.gGridColIdx >> cell.movableCstr;
				if(cell.movableCstr != "Fixed" && cell.movableCstr != "fixed")
					cell.fixed = false;
				v_cellInst.emplace_back(cell);
			}
		}
		else if(label == "NumNets"){
			ss >> netCount;
			for(int i=0;i<netCount;++i){
				getline(file,line);
				stringstream ss2(line);
				ss2 >> label;

				parser_Net net;
				ss2 >> net.netName >> net.numPins >> net.minRoutingLayConstraint;
				for(int j=0;j<net.numPins;++j){
					getline(file,line);

					replace(line.begin(), line.end(), '/', ' '); // replace '/' to ' ' 
					stringstream ss3(line);
					ss3 >> label;

					parser_netPin pin;
					ss3 >> pin.instName >> pin.masterPinName;
					net.v_pin.emplace_back(pin);
				}
				v_net.emplace_back(net);
			}
		}
		else if(label == "NumRoutes"){
			ss >> routeSegmentCount;
			for(int i=0;i<routeSegmentCount;++i){
				getline(file,line);
				stringstream ss2(line);

				parser_RouteSegment route;
				ss2 >> route.sRowIdx >> route.sColIdx >> route.sLayIdx >> route.eRowIdx >> route.eColIdx >> route.eLayIdx >> route.netName;
				v_routeSegment.emplace_back(route);
			}
		}
	}
	cout << "\033[94m[Parser]\033[0m - Parse \"" << fileName << "\" successfully!\n";
}

void parser_C::print_info(){
	cout << "MaxCellMove = " << maxMoveCount << "\n";
	cout << "\nGGridBoundaryIdx = (" << gGridBoundaryIdx.rowBeginIdx << "," << gGridBoundaryIdx.colBeginIdx << ") (" << gGridBoundaryIdx.rowEndIdx << "," << gGridBoundaryIdx.colEndIdx << ")\n";
	cout << "\nNumLayer = " << LayerCount << "\n";
	for(int i=0;i<LayerCount;++i){
		cout << "  L" << v_Layer[i].Idx << ": name=" << v_Layer[i].layerName << ", dir=" << v_Layer[i].RoutingDirection << ", supply=" << v_Layer[i].defaultSupplyOfOneGGrid << "\n";
	}
	cout << "\nNumNonDefaultSupplyGGrid = " << nonDefaultSupplyGGridCount << "\n";
	for(int i=0;i<nonDefaultSupplyGGridCount;++i){
		cout << "  (" << v_nonDefaultSupplyGGrid[i].rowIdx << "," << v_nonDefaultSupplyGGrid[i].colIdx << "," << v_nonDefaultSupplyGGrid[i].LayIdx << "): increase supply=" << v_nonDefaultSupplyGGrid[i].incrOrDecrValue << "\n";
	}
	cout << "\nNumMasterCell = " << masterCellCount << "\n";
	for(int i=0;i<masterCellCount;++i){
		cout << "  name=" << v_masterCell[i].masterCellName << ", pinCount=" << v_masterCell[i].pinCount << ", blockageCount=" << v_masterCell[i].blockageCount << "\n";
		for(int j=0;j<v_masterCell[i].pinCount;++j){
			cout << "    name=" << v_masterCell[i].v_pin[j].pinName << ", pinLayer=" << v_masterCell[i].v_pin[j].pinLayer << "\n";
		}
		for(int j=0;j<v_masterCell[i].blockageCount;++j){
			cout << "    name=" << v_masterCell[i].v_blkg[j].blockageName << ", pinLayer=" << v_masterCell[i].v_blkg[j].blockageLayer << "demand=" << v_masterCell[i].v_blkg[j].demand << "\n";
		}
	}
	cout << "\nNumNeighborCellExtraDemand = " << neighborCellExtraDemandCount << "\n";
	for(int i=0;i<neighborCellExtraDemandCount;++i){
		cout << "  type=" << v_neighborCellExtraDemand[i].type << " " << v_neighborCellExtraDemand[i].masterCellName1 << " " << v_neighborCellExtraDemand[i].masterCellName2 << " ...\n";
	}
	cout << "NumCellInst = " << cellInstCount << "\n";
	for(int i=0;i<cellInstCount;++i){
		cout << "  CellInst=" << v_cellInst[i].instName << "\n";
	}
	cout << "NumNets = " << netCount << "\n";
	for(int i=0;i<netCount;++i){
		cout << "  Net=" << v_net[i].netName << ", pinNum=" << v_net[i].numPins << "\n";
		for(int j=0;j<v_net[i].numPins;++j){
			cout << "    Pin" << j << ": " << v_net[i].v_pin[j].instName << "(" << v_net[i].v_pin[j].masterPinName << ")\n";
		}
	}
	cout << "NumRoutes = " << routeSegmentCount << "\n";
	for(int i=0;i<routeSegmentCount;++i){
		cout << "  Net=" << v_routeSegment[i].netName << ": (" << v_routeSegment[i].sRowIdx <<"," << v_routeSegment[i].sColIdx << "," << v_routeSegment[i].sLayIdx << ") --> (" << v_routeSegment[i].eRowIdx <<"," << v_routeSegment[i].eColIdx << "," << v_routeSegment[i].eLayIdx << ")\n";
	}
}