#ifndef MODULE_H
#define MODULE_H
#include <tuple>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <fstream>


using namespace std;

#define H 0
#define V 1
#define Z 2
typedef short Direction;
// placement related

typedef tuple<int , int, int> Pos3d; // layer, row, col (Z,V,H)
typedef tuple<int, int> Pos2d; // row, col
typedef tuple<int, int, int> ExtraDemandCondition; // ms1 id, ms2 id, layer id

Pos3d addPos3d(Pos3d a,Pos3d b);
Pos3d operator+(Pos3d a,Pos3d b);
Pos2d Subtraction(Pos2d a,Pos2d b);
Pos2d  operator-(Pos2d a,Pos2d b);
Pos2d addPos2d(Pos2d a,Pos2d b);
Pos2d  operator+(Pos2d a,Pos2d b);
Pos2d  operator/(Pos2d a,int n);
Pos2d  pos3dTo2d(Pos3d a);
int distance_2d(Pos2d a);
int distance_3d(Pos3d a);

class gGrid_C;
class Layer_C;
class Chip_C;
class Pin_C;
class Blockage_C;
class MasterCell_C;
class CellInstance_C;
//class Connect_C;
class Net_C;
class CellLibrary_C;
class ExtraDemand_C;
class Design_C;

class gGrid_C{
	// This is the basic P&R resource
	Pos3d _pos;
	int _supply;
	int _demand; // including extra demand
	int _BlkgDmd;
	int _ExtraDmd;

	vector<Blockage_C*> _blkg_list; // blkgs which cost demand in this grid
	vector<Pin_C*> _pin_list; // pins which is in this grid
	vector<Net_C*> _net_list; // nets which go through this grid
	vector<int> _net_id_list;
	//which block (0:haven't assign, -1:no supply, otherwise is x group)
	int label=0;
public:
	gGrid_C(int layer, int row, int col, int supply);

	Pos3d get_pos() const;
	int get_demand() const;
	int get_remain_demand() const;
	int get_supply() const;
	double get_density() const;
	vector<int> get_net_id_list(){return _net_id_list;}
	vector<Pin_C*> get_pin_list(){return _pin_list;}

	void add_supply(int supply_bias);
	void init_demand(); // set demand as 0
	bool add_demand(int val);
	
	bool add_blkg(Blockage_C*); // blkg which cost demand in this grid
	bool add_net(Net_C*); // net which go through this grid
	void add_pin(Pin_C*);
	bool add_extraDmd(int val);
	void remove_blkg(Blockage_C*);
	void remove_net(Net_C*);
	void remove_pin(Pin_C*);
	void remove_extraDmd(int val);

	bool isNetHasExist(Net_C*);
	// access label
	int get_label(){return label;}
	int set_label(int p){label = p;}
	//====

	void print_info();
};

class Layer_C{
	string _name;
	int _id; // 0 ~ N-1 (M1=0) // real pos use this
	int _idx; // 1 ~ N (for input/output file) // this is just for input and output
	Direction _dir;
	int _supply;
	vector<vector<gGrid_C> > _gGrid2d;
	double _density;
public:
	Layer_C(string name, int id, int idx, Direction dir, int supply, int num_rows, int num_cols);

	string get_name() const {return _name;}
	Direction get_dir() const;
	gGrid_C& get_grid(int row, int col);

	double cal_density();
};

class Chip_C{
	// assume idx starts from 0, need to be handled by parser
	// assume rows, columns and layers are continuous
	int _row_begin_idx;
	int _col_begin_idx;
	int _num_rows;
	int _num_cols;
	vector<Layer_C> _layer_list;
	map<string,int> _name2id;
	map<int,int> _idx2id;
	vector<vector<set<CellInstance_C*> > > _pos2Cell_list;
public:
	Chip_C(int row_begin, int col_begin, int row_end, int col_end);

	int get_num_rows() const;
	int get_num_cols() const;
	int get_row_begin_idx() const {return _row_begin_idx;}
	int get_col_begin_idx() const {return _col_begin_idx;}
	int get_layer_id_by_name(string name) const;
	int get_layer_id_by_idx(int idx) const;
	Layer_C& get_layer_by_idx(int idx) {return _layer_list[idx];}
	int get_layer_num() const {return _layer_list.size();}
	Direction get_layer_dir(int layer_id){return layer_id % 2;}
	gGrid_C& get_grid(Pos3d pos) {return _layer_list.at(get<0>(pos)).get_grid(get<1>(pos),get<2>(pos));}
	set<CellInstance_C*> get_cell_list_withPos(int row, int col){return _pos2Cell_list[row][col];}
	//cellmovement=====
	vector<int> get_all_layer_demand(Pos2d p_Pos2d);

	void add_layer(string name, int idx, int supply);
	
	void set_nonDefaultSupply(Pos3d pos, int supply_bias);
	void init_demand(); // set all grid demand as 0
	bool add_blkg_to_grid(Blockage_C* blkg);
	void add_pin_to_grid(Pin_C* pin);
	bool add_route_to_grid(Pos3d, Net_C*);
	bool add_cell_toPos(CellInstance_C* cell);
	void remove_blkg_from_grid(Blockage_C* blkg);
	void remove_pin_from_grid(Pin_C* pin);
	void remove_route_from_grid(vector<Pos3d>, Net_C*);
	void remove_cell_fromPos(CellInstance_C* cell);


	bool posIsLegal(Pos3d pos);
	void dump_supply_demand_table(ofstream&,int);
	void print_supply_demand_table(int);
	void print_all_supply_demand_table();
	void dump_Net_route(ofstream&,Net_C*);
	void dump_Net_route_layer(ofstream&,Net_C*,int);
	void print_Net_route(Net_C*);
	void print_Net_route_layer(Net_C*,int);
	void print_layer_density();
	//DFS find subBlock of chip(each block is not connected.)
	void label_sub_block();
	bool recur_label(int label, int layer, int row, int col);
	int get_label(int,int,int);

	void resetNetUsedCost(int net_id); // set all _net_use_cost[net] = 0

	void print_grid_info(Pos3d);
};

class Pin_C{
	string _name;
	int _id;
	CellInstance_C *_cell;
	int _layer_id;
	int _ori_layer_id;
	bool _isCopied;
	vector<Net_C*> _net_list;
	bool _isConnected;
public:
	Pin_C(string name, int id, CellInstance_C *cell, int layer_id);

	int get_net_num(){return _net_list.size();}
	Net_C* get_net_by_id(int id) const;
	string get_name() const;
	int get_id() const;
	int get_layer_id() const;
	int get_ori_layer_id() const;
	bool isCopiedPin() const {return _isCopied;}
	bool isOnChip() const;
	Pos3d get_pos() const;
	Pos3d get_best_pos() const;
	Pos3d get_ori_pos() const;
	CellInstance_C* get_cell() const;
	
	
	void setConnect(bool b) {_isConnected = b;}
	bool isConnected() const {return _isConnected;};

	void add_net(Net_C *net);
	
};

class Blockage_C{
	string _name;
	int _id;
	CellInstance_C *_cell;
	int _layer_id;
	int _demand; // only default demand
public:
	Blockage_C(string name, int id, CellInstance_C *cell, int layer_id, int demand);

	string get_name() const;
	int get_id() const;
	int get_layer_id() const;
	Pos3d get_pos() const;
	int get_demand() const;
	CellInstance_C* get_cell() const;
};

class MasterCell_C{
	// This is the cell type
	string _name;
	int _id; // 0 ~ n-1
	vector<Pin_C> _pin_list;
	vector<Blockage_C> _blkg_list;
	
public:
	MasterCell_C(string name, int id);

	string get_name() const;
	int get_id() const;
	vector<Pin_C>& get_pins();
	vector<Blockage_C>& get_blkgs();

	void set_id(int val);
	void add_pin(string name, int id, int layer_id);
	void add_blkg(string name, int id, int layer_id, int demand);
};

class CellInstance_C{
	string _name;
	int _id;
	int _master_cell_id;
	bool _fixed;
	vector<Pin_C*> _pin_list;
	map<string,int> _pin_name2id;
	vector<Blockage_C*> _blkg_list;
	map<string,int> _blkg_name2id;
	vector<Net_C*> _relate_net_list;
	int _row;
	int _col;
	int _ori_row;
	int _ori_col;
	Chip_C* _chip;
	MasterCell_C _master_cell;
	ExtraDemand_C* _extraDemand;
	vector<map<int,CellInstance_C*> > _layLsit_msId2sameG_cell;
	vector<map<int,CellInstance_C*> > _layLsit_msId2adjHLG_cell;
	vector<map<int,CellInstance_C*> > _layLsit_msId2adjHRG_cell;
	vector<int> _cost_demand; // incomplete
	bool _onChip;
	//placer
	Pos2d _best_pos;//best pos

public:
	CellInstance_C(string name, int id, MasterCell_C &master_cell, int row, int col, bool fixed, Chip_C* p_pChip, ExtraDemand_C* p_pExtraDemand);

	int get_pin_num() const {return _pin_list.size();}
	int get_blkg_num() const {return _blkg_list.size();}
	int get_relate_net_num() const {return _relate_net_list.size();}
	int get_master_cell_id() const{return _master_cell_id;}

	string get_name() const;
	int get_id() const;
	bool is_fixed() const;
	int get_row() const;
	int get_col() const;
	Pos2d get_pos() const;
	Pos2d get_ori_pos() const;
	Pin_C* get_pin_by_id(int id) const;
	Pin_C* get_pin_by_name(string name) const;
	Blockage_C* get_blkg_by_id(int id) const;
	Net_C* get_relate_net_by_id(int id) const;
	Blockage_C* get_blkg_by_name(string name) const;
	bool is_onChip() const {return _onChip;}
	bool set_onChip(bool b) {_onChip = b;}

	void add_relate_net(Net_C*);
	void set_id(int val);
	void set_pos(Pos2d pos);
	void set_pos(int row, int col);
	void set_ori_pos(Pos2d pos);
	bool move_to(Pos2d); 
	void remove_all_relation_net();
	bool check_pin_demand();
	void remove_blkg_demand();
	double cost_dmdDensity_in_newPos(Pos2d); // if move failed return 999, else return -1~1
	vector<int> new_demand_in_newPos(Pos2d); // incomplete // compute demand before move to.
	vector<int> cost_demand_in_oriPos(); // incomplete // compute demand cost before remove.
	void remove_from_chip();
	bool add_to_chip(Pos2d pos);
	Chip_C* get_chip(){return _chip;}

	bool add_same_gird_extra_demand_cell(CellInstance_C* cell,int layer);
	void remove_same_gird_extra_demand_cell(CellInstance_C* cell,int layer);
	bool add_adjHL_gird_extra_demand_cell(CellInstance_C* cell,int layer);
	void remove_adjHL_gird_extra_demand_cell(CellInstance_C* cell,int layer);
	bool add_adjHR_gird_extra_demand_cell(CellInstance_C* cell,int layer);
	void remove_adjHR_gird_extra_demand_cell(CellInstance_C* cell,int layer);
	void remove_extra_demand();
	bool set_extra_demand();
	bool canUseOnlyOneLayer();
	//placer 

	void setCellBestPos();
	void setCellBestPos_force(string sel = "normal");
	void setCellBestPos_nestestPin();
	void setCellBestPos_nestestPin_toCenter();
	void setCellBestPos_rand();

	void reset_best_pos();
	Pos2d get_best_pos(){return _best_pos;}
	int get_diff_from_best(){return (abs(get<0>(_best_pos)-_row)+abs(get<1>(_best_pos)-_col));}	
	bool is_moved(){return get_pos()!=get_ori_pos();}
	void get_relate_cell(vector<CellInstance_C*>& );
	//predict where we insert cell good
	int distance_pin_to_relate_netRoute();//more actual result

};

/*class Connect_C{
	Pos3d _pos_s;
	Pos3d _pos_e;
	int _length;
	Direction _dir;
	Net_C* _net;
public:
	Connect_C(Pos3d pos_s, Pos3d pos_e, Net_C* net);
	Pos3d get_pos_s() {return _pos_s;}
	Pos3d get_pos_e() {return _pos_e;}
	int get_length() {return _length;}
	Direction get_dir() {return _dir;}
	Net_C* get_net() {return _net;}
	
	vector<Pos3d> v_pos;
};*/

class Net_C{
	string _name;
	int _id;
	int _min_layer;
	int HPWL;
	int _route_priority;
	vector<Pin_C*> _pins;
	vector<Pos3d> _pins_grid;
	vector<Pos3d> _route; // all grids
	vector<Pos3d> _ori_route; // original route for reservation
	vector<pair<Pos3d,Pos3d> > _wires; // for output
public:
	Net_C(string name, int id, int min_layer, Chip_C* p_pChip);

	int get_pin_num() const {return _pins.size();}
	int get_route_num() const {return _route.size();}

	string get_name() const;
	int get_id() const;
	int get_min_layer() const;
	Pin_C* get_pin_by_id(int id) const {return _pins[id];}
	Pos3d get_route_by_id(int id) const {return _route[id];}

	void set_id(int val);
	void add_pin(Pin_C *pin);
	bool add_route(Pos3d pos);
	bool init_route(); // add_route() for all pin and pin_lower_minRouteLayer
	void remove_route(Pos3d pos);
	void remove_all_route(); // _route.clear(), remove_route_from_grid()
	int get_WL(){return _route.size();}
	int get_HPWL();
	bool check_all_pin_connected();
	void trans_routes2wires(); // incompleted, for output, transfer grids to wires
	int get_wires_num() const {return _wires.size();}
	vector<pair<Pos3d,Pos3d> > get_all_wire() const {return _wires;}
	void set_all_pin_connect();
	bool isPin(Pos3d);
	//priority
	void inc_Priority(){_route_priority++;}
	int get_priority(){return _route_priority;}
	//recover ori route
	void set_ori_route(){_ori_route = _route ;}
	vector<Pos3d> get_ori_route(){return _ori_route;}
	Pos2d get_route_center();
	Chip_C* _chip;
};

class CellLibrary_C{
	vector<MasterCell_C> _master_cell_list;
	map<string, int> _name2id;

	int get_id_by_name(string name) const {
		if(_name2id.find(name)==_name2id.end()) return -1;
		return _name2id.at(name);
	}
public:
	CellLibrary_C();

	MasterCell_C get_master_cell_by_name(string name, bool &success) const;

	void add_master_cell(MasterCell_C ms);
};

class ExtraDemand_C{
	map<ExtraDemandCondition, int> same_grid;
	map<ExtraDemandCondition, int> adj_hor_grid;

	ExtraDemandCondition get_cond(int ms1, int ms2, int layer) const {
		if(ms2 < ms1){
			int tmp = ms2;
			ms2 = ms1;
			ms1 = tmp;
		}
		return ExtraDemandCondition(ms1, ms2, layer);
	}
public:
	ExtraDemand_C();

	int get_same_demand(int ms1, int ms2, int layer) const;
	int get_adj_demand(int ms1, int ms2, int layer) const;

	void add_same_demand(int ms1, int ms2, int layer, int demand);
	void add_adj_demand(int ms1, int ms2, int layer, int demand);

	void dump_all_cond(ofstream&);
	void print_all_cond();
};

class Design_C{
	int _max_cell_move;
	vector<CellInstance_C*> _cell_list;
	vector<Net_C*> _net_list;
	set<int> _moved_cell_id;
	map<string, int> _cell_name2id;
	map<string, int> _net_name2id;

public:
	Design_C(int max_cell_move, int num_cells, int num_nets);
	
	int get_cell_num() const {return _cell_list.size();}
	int get_net_num() const {return _net_list.size();}
	// get list
	void get_cell_list(vector<CellInstance_C*> &p_pCell){p_pCell = _cell_list;}
	void get_net_list(vector<Net_C*> &p_pNet){p_pNet = _net_list;}
	// get by id
	CellInstance_C* get_cell_by_id(int id) const;
	Net_C* get_net_by_id(int id) const;
	// get by name
	CellInstance_C* get_cell_by_name(string name) const;
	Net_C* get_net_by_name(string name) const;
	// get other
	int get_max_cell_move() const {return _max_cell_move;}
	int get_total_WL() const;
	set<int> get_moved_cell_id_list() const {return _moved_cell_id;}
	int get_moved_cell_num(){return _moved_cell_id.size();}
	// access data
	void add_cell(CellInstance_C *cell);
	void add_net(Net_C *net);
	bool move_cell_to_pos(CellInstance_C* cell, Pos2d);
	bool move_cell_to_pos(int cell_id, Pos2d);
	// group placer
	void remove_cell_from_pos(CellInstance_C* cell);
	bool add_cell_to_pos(CellInstance_C* cell, Pos2d);
	bool move_cells_to_pos(vector<CellInstance_C*> vec_cell, vector<Pos2d> vec_pos);

	// other
	void dump_cell_info(ofstream&, int id);
	void print_cell_info(int id);
	void dump_net_info(ofstream&, int id);
	void print_net_info(int id);

	//analysis
	void net_analysis(ofstream& file);
};

#endif