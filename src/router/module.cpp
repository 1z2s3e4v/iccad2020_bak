#include "module.h"
#include <bits/stdc++.h> 
#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <assert.h>
Pos3d addPos3d(Pos3d a,Pos3d b){
    return Pos3d(get<0>(a)+get<0>(b), get<1>(a)+get<1>(b), get<2>(a)+get<2>(b));
}
Pos3d operator+(Pos3d a,Pos3d b){
    return Pos3d(get<0>(a)+get<0>(b), get<1>(a)+get<1>(b), get<2>(a)+get<2>(b));
}
Pos2d Subtraction(Pos2d a,Pos2d b){
    return Pos2d(get<0>(a)-get<0>(b), get<1>(a)-get<1>(b));
}
Pos2d  operator-(Pos2d a,Pos2d b){
    return Pos2d(get<0>(a)-get<0>(b), get<1>(a)-get<1>(b));
}
Pos2d addPos2d(Pos2d a,Pos2d b){
    return Pos2d(get<0>(a)+get<0>(b), get<1>(a)+get<1>(b));
}
Pos2d  operator+(Pos2d a,Pos2d b){
    return Pos2d(get<0>(a)+get<0>(b), get<1>(a)+get<1>(b));
}

Pos2d  operator/(Pos2d a,int n){
    return Pos2d(get<0>(a)/n, get<1>(a)/n);
}
Pos2d  pos3dTo2d(Pos3d a){
    return Pos2d(get<1>(a),get<2>(a));
}
int distance_2d(Pos2d a){return abs(get<0>(a))+abs(get<1>(a));}
int distance_3d(Pos3d a){return abs(get<0>(a))+abs(get<1>(a))+abs(get<2>(a));}

string pos2str(Pos3d pos) {return "("+to_string(get<0>(pos))+","+to_string(get<1>(pos))+","+to_string(get<2>(pos))+")";}
string pos2d2str(Pos2d pos) {return "("+to_string(get<0>(pos))+","+to_string(get<1>(pos))+")";}

int getHPWL(Pos2d pos1, Pos2d pos2){
    int x1, y1, x2, y2;
    tie(y1,x1) = pos1;
    tie(y2,x2) = pos2;
    return abs(x1-x2) + abs(y1-y2);
}
Direction getDir(Pos3d pos_s, Pos3d pos_e){
    if(get<0>(pos_s) != get<0>(pos_e))
        return Z;
    else {
        return get<0>(pos_s) % 2;
    }
}
int getLength(Pos3d pos_s, Pos3d pos_e){
    Direction dir = getDir(pos_s, pos_e);
    if(dir == Z)
        return abs(get<0>(pos_s) - get<0>(pos_e)) + 1;
    else if(dir == V)
        return abs(get<1>(pos_s) - get<1>(pos_e)) + 1;
    else if(dir == H)
        return abs(get<2>(pos_s) - get<2>(pos_e)) + 1;
    else return -1;
}
// class gGrid_C

gGrid_C::gGrid_C(int layer, int row, int col, int supply):
	_pos(layer, row, col),
	_supply(supply),
	_demand(0),
    _BlkgDmd(0),
    _ExtraDmd(0)
{}

Pos3d gGrid_C::get_pos() const {return _pos;}

int gGrid_C::get_demand() const {return _demand;}

int gGrid_C::get_remain_demand() const {return _supply - _demand;}

int gGrid_C::get_supply() const {return _supply;}

double gGrid_C::get_density() const {
    if(_supply==0) return 1;
    else return (double)_demand/_supply;
}

void gGrid_C::add_supply(int supply_bias){
    _supply += supply_bias;
}

void gGrid_C::init_demand(){
    _demand = 0;
}

bool gGrid_C::add_demand(int val)
{
    int new_demand = _demand + val;
    if(new_demand > _supply){
        ////cout << "\033[94m[module]\033[0m - gGrid" << pos2str(this->get_pos()) << " overflow occur.\n";
        return false;
    } 
    if(new_demand < 0){
        _demand = 0;
    }
    else
        _demand += val;
    return true;
}

bool gGrid_C::add_blkg(Blockage_C* blkg){
    if(add_demand(blkg->get_demand())){
        _blkg_list.push_back(blkg);
        _BlkgDmd += blkg->get_demand();
        return true;
    }
    else return false;
}

void gGrid_C::add_pin(Pin_C* pin){
    _pin_list.push_back(pin);
}

bool gGrid_C::add_net(Net_C* net){
    if(find(_net_id_list.begin(),_net_id_list.end(),net->get_id()) == _net_id_list.end()){
        if(add_demand(1)){ // add 1 demand to the grid
            _net_id_list.push_back(net->get_id());
            _net_list.push_back(net);
            return true;
        }
        else return false; 
    }
    else return true;
}

bool gGrid_C::add_extraDmd(int val){
    if(add_demand(val)){
        _ExtraDmd += val;
        return true;
    }
    else return false;
}

void gGrid_C::remove_blkg(Blockage_C* blkg){
    auto it = find(_blkg_list.begin(),_blkg_list.end(),blkg);
    if(it != _blkg_list.end()){
        _blkg_list.erase(it);
        add_demand(-1*blkg->get_demand());
        _BlkgDmd -= blkg->get_demand();
    }
}

void gGrid_C::remove_net(Net_C* net){
    auto it = find(_net_id_list.begin(),_net_id_list.end(),net->get_id());
    if(it != _net_id_list.end()){
        _net_id_list.erase(it);
        _net_list.erase(find(_net_list.begin(),_net_list.end(),net));
        add_demand(-1); // if there is no any same net, demand -1
    }
}

void gGrid_C::remove_pin(Pin_C* pin){
    auto it = find(_pin_list.begin(),_pin_list.end(),pin);
    if(it != _pin_list.end()){
        _pin_list.erase(it);
    }
}

void gGrid_C::remove_extraDmd(int val){
    _ExtraDmd -= val;
    add_demand(-1*val);
}

bool gGrid_C::isNetHasExist(Net_C* net){
    return (find(_net_id_list.begin(),_net_id_list.end(),net->get_id()) != _net_id_list.end());
}

void gGrid_C::print_info(){
    cout << "Supply: " << _supply << " Demand: " << _demand 
    << "(" << "PinCnt: " << _pin_list.size() << " WireDmd: " << _net_list.size() 
    << " BlkgDmd: " << _BlkgDmd << " ExtraDmd: " << _ExtraDmd
    << ")" << " in gGrid" << pos2str(get_pos()) << "\n";
}   

// class Layer_C

Layer_C::Layer_C(string name, int id, int idx, Direction dir, int supply, int num_rows, int num_cols):
    _name(name),
    _id(id),
    _idx(idx),
    _dir(dir),
    _supply(supply)
{
    for(int r = 0; r<num_rows; r++){
        vector<gGrid_C> grid_row;
        for(int c = 0; c<num_cols; c++){
            grid_row.push_back(gGrid_C(id, r, c, supply));
        }
        _gGrid2d.push_back(grid_row);
    }
}

Direction Layer_C::get_dir() const {return _dir;}

gGrid_C& Layer_C::get_grid(int row, int col) {return _gGrid2d[row][col];}

double Layer_C::cal_density(){
    double total_supply=0;
    double total_demand=0;
    for(auto& grid: _gGrid2d){
        for(auto& g: grid){
            total_supply += g.get_supply();
            total_demand += g.get_demand();
        }
    }
    if(total_supply==0) _density = 1;
    else _density =  total_demand/total_supply;
    return _density;
}

// class Chip_C

Chip_C::Chip_C(int row_begin, int col_begin, int row_end, int col_end):
	_row_begin_idx(row_begin),
	_col_begin_idx(col_begin),
	_num_rows(row_end - row_begin + 1),
	_num_cols(col_end - col_begin + 1)
{
    _pos2Cell_list.resize(_num_rows);
    for(int i=0;i<_num_rows;++i){
        _pos2Cell_list[i].resize(_num_cols);
        set<CellInstance_C*> empty_s;
        for(auto &s : _pos2Cell_list[i]){
            s = empty_s;
        }
    }
}

int Chip_C::get_num_rows() const {return _num_rows;}

int Chip_C::get_num_cols() const {return _num_cols;}

int Chip_C::get_layer_id_by_name(string name) const {
	if(_name2id.find(name)==_name2id.end()) return -1;
    return _name2id.at(name);
}
int Chip_C::get_layer_id_by_idx(int idx) const {
	if(_idx2id.find(idx)==_idx2id.end()) return -1;
    return _idx2id.at(idx);
}

bool Chip_C::add_cell_toPos(CellInstance_C* cell){
    bool success = true;

    // put blkg
    for(int i=0;i<cell->get_blkg_num();++i){
        if(!this->add_blkg_to_grid(cell->get_blkg_by_id(i))){
            success = false;
            break;
        }
    }
    if(!success){ // back
        ////cout << "\033[94m[CHIP]\033[0m - add_blkg_to_grid return false\n";
        for(int i=0;i<cell->get_blkg_num();++i)
            this->remove_blkg_from_grid(cell->get_blkg_by_id(i));
        return false;
    }

    // put extra demand
    if(!cell->set_extra_demand()){ // back
        ////cout << "\033[94m[CHIP]\033[0m - set_extra_demand return false\n";
        cell->remove_extra_demand();
        for(int i=0;i<cell->get_blkg_num();++i)
            this->remove_blkg_from_grid(cell->get_blkg_by_id(i));
        return false;
    }

    // put pin
    _pos2Cell_list[cell->get_row()][cell->get_col()].insert(cell);
    for(int i=0;i<cell->get_pin_num();++i)
        this->add_pin_to_grid(cell->get_pin_by_id(i));
    if(cell->check_pin_demand()){ // here will put pin's route on grid (init_route for this cell)
        cell->set_onChip(true);
        return true;
    }
    else{ // back
        ////cout << "\033[94m[CHIP]\033[0m - check_pin_demand return false\n";
        _pos2Cell_list[cell->get_row()][cell->get_col()].erase(cell);
        for(int i=0;i<cell->get_blkg_num();++i)
            this->remove_blkg_from_grid(cell->get_blkg_by_id(i));
        cell->remove_extra_demand();
        for(int i=0;i<cell->get_pin_num();++i)
            this->remove_pin_from_grid(cell->get_pin_by_id(i));
        return false;
    }
}
void Chip_C::remove_cell_fromPos(CellInstance_C* cell){
    /*if(_pos2Cell_list[cell->get_row()][cell->get_col()].find(cell) == _pos2Cell_list[cell->get_row()][cell->get_col()].end()){
        return;
    }*/
    if(!cell->is_onChip()){
        return;
    }
    _pos2Cell_list[cell->get_row()][cell->get_col()].erase(cell);
    for(int i=0;i<cell->get_blkg_num();++i)
        this->remove_blkg_from_grid(cell->get_blkg_by_id(i));
    for(int i=0;i<cell->get_pin_num();++i)
        this->remove_pin_from_grid(cell->get_pin_by_id(i));
    cell->remove_extra_demand();
    cell->set_onChip(false);
}
void Chip_C::print_layer_density(){
    for(int i=0;i<_layer_list.size();++i){
        cout << "layer["<<i<<"] density: "<< _layer_list[i].cal_density()<< endl;
    }
}

void Chip_C::add_layer(string name, int idx, int supply)
{
    int layer_id = _layer_list.size();
    _name2id[name] = layer_id;
    _idx2id[idx] = layer_id;
    _layer_list.push_back(
        Layer_C(name, layer_id, idx, get_layer_dir(layer_id), supply, _num_rows, _num_cols));
}

void Chip_C::set_nonDefaultSupply(Pos3d pos, int supply_bias){
    _layer_list[get<0>(pos)].get_grid(get<1>(pos),get<2>(pos)).add_supply(supply_bias);
}

void Chip_C::init_demand(){
    for(int lay=0;lay<this->get_layer_num();++lay){
        for(int r=0;r<this->get_num_rows();++r){
            for(int c=0;c<this->get_num_cols();++c){
                _layer_list[lay].get_grid(r,c).init_demand();
            }
        }
    }
}

bool Chip_C::add_blkg_to_grid(Blockage_C* blkg){
    Pos3d pos = blkg->get_pos();
    return _layer_list[get<0>(pos)].get_grid(get<1>(pos),get<2>(pos)).add_blkg(blkg);
}

void Chip_C::add_pin_to_grid(Pin_C* pin){
    Pos3d pos = pin->get_ori_pos();
    return get_grid(pos).add_pin(pin);
}

bool Chip_C::add_route_to_grid(Pos3d pos, Net_C* net){
    if(!get_grid(pos).add_net(net))
        return false;
    else
        return true;
}

void Chip_C::remove_blkg_from_grid(Blockage_C* blkg){
    Pos3d pos = blkg->get_pos();
    get_grid(pos).remove_blkg(blkg);
}

void Chip_C::remove_pin_from_grid(Pin_C* pin){
    Pos3d pos = pin->get_ori_pos();
    get_grid(pos).remove_pin(pin);
}

void Chip_C::remove_route_from_grid(vector<Pos3d> route, Net_C* net){
    for(auto pos : route){
        get_grid(pos).remove_net(net);
    }
}
vector<int> Chip_C::get_all_layer_demand(Pos2d p_Pos2d){
    vector<int> demand;
    
    for(int layerIter=0;layerIter<get_layer_num();++layerIter){
        gGrid_C grid = get_layer_by_idx(layerIter).get_grid(get<0>(p_Pos2d),get<1>(p_Pos2d));
        demand.push_back(grid.get_demand());
    }
    return demand;
}
bool Chip_C::posIsLegal(Pos3d pos){
    return ((get<0>(pos)>=0) && (get<0>(pos)<_layer_list.size()) 
            && (get<1>(pos)>=0) && (get<1>(pos)<_num_rows) 
            && (get<2>(pos)>=0) && (get<2>(pos)<_num_cols));
}
void Chip_C::label_sub_block(){
    //reset all lebal
    for(int layerIter=0;layerIter<this->get_layer_num();++layerIter){
        for(int rowIter=0;rowIter<this->get_num_rows();++rowIter){
            for(int colIter=0;colIter<this->get_num_cols();++colIter){
                Layer_C& layer = this->get_layer_by_idx(layerIter);
                if(layer.get_grid(rowIter,colIter).get_remain_demand()<=0){
                    layer.get_grid(rowIter,colIter).set_label(-1);
                }
                else layer.get_grid(rowIter,colIter).set_label(0);
            }
        }
    }
    int label = 1;
    for(int layerIter=0;layerIter<this->get_layer_num();++layerIter){
        for(int rowIter=0;rowIter<this->get_num_rows();++rowIter){
            for(int colIter=0;colIter<this->get_num_cols();++colIter){
                if(recur_label(label, layerIter, rowIter, colIter)){
                    label++;
                }
            }
        }
    }       
}
bool Chip_C::recur_label(int label, int layer, int row, int col){
    
    if(layer<0 || layer>=this->get_layer_num()) return false;
    if(row<0 || row>=this->get_num_rows()) return false;
    if(col<0 || col>=this->get_num_cols()) return false;
    Layer_C& clayer = this->get_layer_by_idx(layer);
    gGrid_C& grid = clayer.get_grid(row, col);
    if(grid.get_label() != 0) return false;    
    Direction d = clayer.get_dir();
    grid.set_label(label);

    if(d == 0){
        recur_label(label, layer,row+1,col);
        recur_label(label, layer,row-1,col);
    }
    else{
        recur_label(label, layer,row,col-1);
        recur_label(label, layer,row,col+1);
    }   
        recur_label(label, layer-1,row,col);
        recur_label(label, layer+1,row,col);
    return true;
}
int Chip_C::get_label(int layer,int row,int col){
    Layer_C& clayer = this->get_layer_by_idx(layer);
    gGrid_C grid = clayer.get_grid(row, col);
    return grid.get_label();
}

void Chip_C::dump_supply_demand_table(ofstream& file, int layer_id){
    Layer_C lay = _layer_list[layer_id];
    file << lay.get_name() << ":\n";
    for(int row=0;row<_num_rows;++row){
        file << string(_num_cols*8+1,'-') << "\n";
        for(int col=0;col<_num_cols;++col){
            gGrid_C grid = lay.get_grid(row,col);
            file << "|" << setw(3) << grid.get_demand() << "/" << setw(3) << grid.get_supply();
        }
        file << "|\n";
    }
    file << string(_num_cols*8+1,'-') << "\n";
}

void Chip_C::print_all_supply_demand_table(){
    for(int i=0;i<get_layer_num();++i){
		print_supply_demand_table(i);
	}
}

void Chip_C::print_supply_demand_table(int layer_id){
    Layer_C lay = _layer_list[layer_id];
    cout << lay.get_name() << ":\n";
    for(int row=0;row<_num_rows;++row){
        cout << string(_num_cols*8+1,'-') << "\n";
        for(int col=0;col<_num_cols;++col){
            gGrid_C grid = lay.get_grid(row,col);
            cout << "|" << setw(3) << grid.get_demand() << "/" << setw(3) << grid.get_supply();
        }
        cout << "|\n";
    }
    cout << string(_num_cols*8+1,'-') << "\n";
}

void Chip_C::dump_Net_route(ofstream& file, Net_C* net){
    file << net->get_name() << " WL=" << net->get_WL() << "\n";
    for(int lay=0;lay<_layer_list.size();++lay){
        dump_Net_route_layer(file, net,lay);
    }
    file << "\n";
}

void Chip_C::print_Net_route(Net_C* net){
    cout << net->get_name() << " WL=" << net->get_WL() << "\n";
    for(int lay=0;lay<_layer_list.size();++lay){
        print_Net_route_layer(net,lay);
    }
    cout << "\n";
}

void Chip_C::dump_Net_route_layer(ofstream& file, Net_C* net, int layer_id){
    Layer_C lay = _layer_list[layer_id];
    file << net->get_name() << "(" << lay.get_name() << "):\n";
    for(int row=0;row<_num_rows;row++){
        file << string(_num_cols*4+1,'-') << "\n";
        for(int col=0;col<_num_cols;++col){
            gGrid_C grid = lay.get_grid(row,col);
            vector<int> net_id_list = grid.get_net_id_list();
            if(find(net_id_list.begin(),net_id_list.end(),net->get_id()) != net_id_list.end()){
                bool isPin = false;
                for(int i=0;i<net->get_pin_num();++i){
                    if(net->get_pin_by_id(i)->get_pos() == grid.get_pos()){
                        isPin = true;
                        break;
                    }
                }
                if(isPin) file << "| O ";
                else file << "| * ";
            }
            else file << "|   ";
        }
        file << "|\n";
    }
    file << string(_num_cols*4+1,'-') << "\n";
}

void Chip_C::print_Net_route_layer(Net_C* net, int layer_id){
    Layer_C lay = _layer_list[layer_id];
    cout << net->get_name() << "(" << lay.get_name() << "):\n";
    for(int row=0;row<_num_rows;row++){
        cout << string(_num_cols*4+1,'-') << "\n";
        for(int col=0;col<_num_cols;++col){
            gGrid_C grid = lay.get_grid(row,col);
            vector<int> net_id_list = grid.get_net_id_list();
            if(find(net_id_list.begin(),net_id_list.end(),net->get_id()) != net_id_list.end()){
                bool isPin = false;
                for(int i=0;i<net->get_pin_num();++i){
                    if(net->get_pin_by_id(i)->get_pos() == grid.get_pos()){
                        isPin = true;
                        break;
                    }
                }
                if(isPin)
                    cout << "| O ";
                else
                    cout << "| * ";
            }
            else cout << "|   ";
        }
        cout << "|\n";
    }
    cout << string(_num_cols*4+1,'-') << "\n";
}

void Chip_C::print_grid_info(Pos3d pos){
    get_grid(pos).print_info();
}
// class Pin_C

Pin_C::Pin_C(string name, int id, CellInstance_C *cell, int layer_id):
	_name(name),
	_id(id),
	_cell(cell),
	_ori_layer_id(layer_id),
    _layer_id(layer_id),
    _isCopied(false)
{}

Pos3d Pin_C::get_pos() const {return Pos3d(_layer_id, _cell->get_row(), _cell->get_col());}
Pos3d Pin_C::get_best_pos() const {return Pos3d(_layer_id, get<0>(_cell->get_best_pos()), get<1>(_cell->get_best_pos()));}
Pos3d Pin_C::get_ori_pos() const {return Pos3d(_ori_layer_id, _cell->get_row(), _cell->get_col());}
string Pin_C::get_name() const {return _name;}
int Pin_C::get_id() const {return _id;}
int Pin_C::get_layer_id() const {return _layer_id;}
int Pin_C::get_ori_layer_id() const {return _ori_layer_id;}
CellInstance_C* Pin_C::get_cell() const {return _cell;}
Net_C* Pin_C::get_net_by_id(int id) const {return _net_list[id];}
void Pin_C::add_net(Net_C *net) {
    if(net->get_min_layer() > _layer_id){
        _layer_id = net->get_min_layer();
        _isCopied = true;
        for(int z=get<0>(this->get_ori_pos());z<=get<0>(this->get_pos());++z){
            net->add_route(Pos3d(z,get<1>(this->get_pos()),get<2>(this->get_pos())));
        }
    }
    _net_list.push_back(net);
    _cell->add_relate_net(net);
}
bool Pin_C::isOnChip() const {return _cell->is_onChip();}

// class Blockage_C

Blockage_C::Blockage_C(string name, int id, CellInstance_C *cell, int layer_id, int demand):
	_name(name),
	_id(id),
	_cell(cell),
	_layer_id(layer_id),
	_demand(demand)
{}

Pos3d Blockage_C::get_pos() const {return Pos3d(_layer_id, _cell->get_row(), _cell->get_col());}
string Blockage_C::get_name() const {return _name;}
int Blockage_C::get_id() const {return _id;}
int Blockage_C::get_layer_id() const {return _layer_id;}
int Blockage_C::get_demand() const {return _demand;}
CellInstance_C* Blockage_C::get_cell() const {return _cell;}

// class MasterCell_C

MasterCell_C::MasterCell_C(string name, int id):
	_name(name),
	_id(id)
{
    _pin_list.clear();
    _blkg_list.clear();
}

string MasterCell_C::get_name() const {return _name;}

int MasterCell_C::get_id() const {return _id;}

vector<Pin_C>& MasterCell_C::get_pins() {return _pin_list;}

vector<Blockage_C>& MasterCell_C::get_blkgs() {return _blkg_list;}

void MasterCell_C::set_id(int val) {_id = val;}

void MasterCell_C::add_pin(string name, int id, int layer_id)
{
    _pin_list.push_back(Pin_C(name, id, nullptr, layer_id));
}

void MasterCell_C::add_blkg(string name, int id, int layer_id, int demand)
{
    _blkg_list.push_back(
        Blockage_C(name, id, nullptr, layer_id, demand));
}


// class CellInstance_C

CellInstance_C::CellInstance_C(string name, int id, MasterCell_C &master_cell, int row, int col, bool fixed, Chip_C* p_pChip, ExtraDemand_C* p_pExtraDemand):
	_name(name),
	_id(id),
	_master_cell_id(master_cell.get_id()),
    _master_cell(master_cell),
    _row(row),
    _col(col),
    _ori_row(row),
    _ori_col(col),
	_fixed(fixed),
    _chip(p_pChip),
    _extraDemand(p_pExtraDemand)
{
    _cost_demand.resize(_chip->get_layer_num(),0);
    _pin_list.clear();
    vector<Pin_C>& pins = master_cell.get_pins();
    for(Pin_C &pin : pins){
        Pin_C *new_pin = new Pin_C(pin.get_name(),pin.get_id(),this,pin.get_layer_id());
        _pin_name2id[pin.get_name()] = pin.get_id();
        _pin_list.push_back(new_pin);
    }

    _blkg_list.clear();
    vector<Blockage_C>& blkgs = master_cell.get_blkgs();
    for(Blockage_C &blkg : blkgs){
        Blockage_C *new_blkg = new Blockage_C(blkg.get_name(),blkg.get_id(),this,blkg.get_layer_id(),blkg.get_demand());
        _blkg_name2id[blkg.get_name()] = blkg.get_id();
        _blkg_list.push_back(new_blkg);
        _cost_demand[blkg.get_layer_id()] += blkg.get_demand();
    }
    _layLsit_msId2sameG_cell.resize(_chip->get_layer_num());
    _layLsit_msId2adjHLG_cell.resize(_chip->get_layer_num());
    _layLsit_msId2adjHRG_cell.resize(_chip->get_layer_num());
    _chip->add_cell_toPos(this);
}

string CellInstance_C::get_name() const {return _name;}

int CellInstance_C::get_id() const {return _id;}
bool CellInstance_C::is_fixed() const {return _fixed;}
int CellInstance_C::get_row() const {return _row;}
int CellInstance_C::get_col() const {return _col;}

Pos2d CellInstance_C::get_pos() const {return Pos2d(_row, _col);}
Pos2d CellInstance_C::get_ori_pos() const {return Pos2d(_ori_row, _ori_col);}

Pin_C* CellInstance_C::get_pin_by_id(int id) const {return _pin_list.at(id);}
Pin_C* CellInstance_C::get_pin_by_name(string name) const {return _pin_list.at(_pin_name2id.at(name));}
Blockage_C* CellInstance_C::get_blkg_by_id(int id) const {return _blkg_list.at(id);}
Blockage_C* CellInstance_C::get_blkg_by_name(string name) const {return _blkg_list.at(_blkg_name2id.at(name));}
Net_C* CellInstance_C::get_relate_net_by_id(int id) const {return _relate_net_list.at(id);}

void CellInstance_C::add_relate_net(Net_C* net) {
    if(find(_relate_net_list.begin(),_relate_net_list.end(),net) == _relate_net_list.end())
        _relate_net_list.emplace_back(net);
}

void CellInstance_C::set_id(int val) {_id = val;}

void CellInstance_C::set_pos(Pos2d pos)
{
    if(is_fixed()) return;
    _row = get<0>(pos);
    _col = get<1>(pos);
}

void CellInstance_C::set_pos(int row, int col)
{
    if(is_fixed()) return;
    _row = row;
    _col = col;
}

bool CellInstance_C::add_same_gird_extra_demand_cell(CellInstance_C* cell, int layer){
    auto it_this = _layLsit_msId2sameG_cell[layer].find(cell->get_master_cell_id());
    auto it_cell = cell->_layLsit_msId2sameG_cell[layer].find(this->get_master_cell_id());
    if(it_this == _layLsit_msId2sameG_cell[layer].end() && it_cell == cell->_layLsit_msId2sameG_cell[layer].end()){
        // there's no one pair with this msType for both cell
        _layLsit_msId2sameG_cell[layer].emplace(cell->get_master_cell_id(),cell);
        cell->_layLsit_msId2sameG_cell[layer].emplace(this->get_master_cell_id(),this);
        return true;
    }
    else{
        return false;
    }
}
void CellInstance_C::remove_same_gird_extra_demand_cell(CellInstance_C* cell, int layer){
    _layLsit_msId2sameG_cell[layer].erase(cell->get_master_cell_id());
    cell->_layLsit_msId2sameG_cell[layer].erase(this->get_master_cell_id());
}
bool CellInstance_C::add_adjHL_gird_extra_demand_cell(CellInstance_C* cell, int layer){
    auto it_this = _layLsit_msId2adjHLG_cell[layer].find(cell->get_master_cell_id());
    auto it_cell = cell->_layLsit_msId2adjHRG_cell[layer].find(this->get_master_cell_id());
    if(it_this == _layLsit_msId2adjHLG_cell[layer].end() && it_cell == cell->_layLsit_msId2adjHRG_cell[layer].end()){
        // there's no one pair with this msType for both cell
        _layLsit_msId2adjHLG_cell[layer].emplace(cell->get_master_cell_id(),cell);
        cell->_layLsit_msId2adjHRG_cell[layer].emplace(this->get_master_cell_id(),this);
        return true;
    }
    else{
        return false;
    }
}
void CellInstance_C::remove_adjHL_gird_extra_demand_cell(CellInstance_C* cell, int layer){
    _layLsit_msId2adjHLG_cell[layer].erase(cell->get_master_cell_id());
    cell->_layLsit_msId2adjHRG_cell[layer].erase(this->get_master_cell_id());
}
bool CellInstance_C::add_adjHR_gird_extra_demand_cell(CellInstance_C* cell, int layer){
    auto it_this = _layLsit_msId2adjHRG_cell[layer].find(cell->get_master_cell_id());
    auto it_cell = cell->_layLsit_msId2adjHLG_cell[layer].find(this->get_master_cell_id());
    if(it_this == _layLsit_msId2adjHRG_cell[layer].end() && it_cell == cell->_layLsit_msId2adjHLG_cell[layer].end()){
        // there's no one pair with this msType for both cell
        _layLsit_msId2adjHRG_cell[layer].emplace(cell->get_master_cell_id(),cell);
        cell->_layLsit_msId2adjHLG_cell[layer].emplace(this->get_master_cell_id(),this);
        return true;
    }
    else{
        return false;
    }
}
void CellInstance_C::remove_adjHR_gird_extra_demand_cell(CellInstance_C* cell, int layer){
    _layLsit_msId2adjHRG_cell[layer].erase(cell->get_master_cell_id());
    cell->_layLsit_msId2adjHLG_cell[layer].erase(this->get_master_cell_id());
}
bool CellInstance_C::set_extra_demand(){
    bool success = true;
    // same grid demand
    set<CellInstance_C*> sameGridCells = _chip->get_cell_list_withPos(_row,_col);
    for(int layer=0;layer<_chip->get_layer_num();++layer){
        for(auto &sameGridCell : sameGridCells){
            int extra_demand = _extraDemand->get_same_demand(this->get_master_cell_id(), sameGridCell->get_master_cell_id(), layer);
            if(extra_demand > 0){
                // there's an extra demand on Pos3d(row,col,later_id) between thisCell and itCell
                if(this->add_same_gird_extra_demand_cell(sameGridCell,layer)){
                    if(!_chip->get_grid(Pos3d(layer,_row,_col)).add_extraDmd(extra_demand)){
                        this->remove_same_gird_extra_demand_cell(sameGridCell,layer);
                        success = false;
                        break;
                    }
                }
            }
        }
        if(!success){
            this->remove_extra_demand();
            return false;
        }
    }

    // adjH grid demand
    if(_col-1 >= 0){ // left grid
        set<CellInstance_C*> LeftGridCells = _chip->get_cell_list_withPos(_row,_col-1);
        for(int layer=0;layer<_chip->get_layer_num();++layer){
            for(auto &leftGridCell : LeftGridCells){
                int extra_demand = _extraDemand->get_adj_demand(this->get_master_cell_id(), leftGridCell->get_master_cell_id(), layer);
                if(extra_demand > 0){
                    // there's an extra demand on Pos3d(row,col,later_id) between thisCell and itCell
                    if(this->add_adjHL_gird_extra_demand_cell(leftGridCell,layer)){
                        if(!_chip->get_grid(Pos3d(layer,_row,_col)).add_extraDmd(extra_demand)){
                            this->remove_adjHL_gird_extra_demand_cell(leftGridCell,layer);
                            success = false;
                            break;
                        }
                        if(success && !_chip->get_grid(Pos3d(layer,_row,_col-1)).add_extraDmd(extra_demand)){
                            _chip->get_grid(Pos3d(layer,_row,_col)).remove_extraDmd(extra_demand);
                            this->remove_adjHL_gird_extra_demand_cell(leftGridCell,layer);
                            success = false;
                            break;
                        }
                    }
                }
            }
            if(!success){
                this->remove_extra_demand();
                return false;
            }
        }
    }

    if(_col+1 < _chip->get_num_cols()){ // right grid
        set<CellInstance_C*> RightGridCells = _chip->get_cell_list_withPos(_row,_col+1);
        for(int layer=0;layer<_chip->get_layer_num();++layer){
            for(auto &rightGridCell : RightGridCells){
                int extra_demand = _extraDemand->get_adj_demand(this->get_master_cell_id(), rightGridCell->get_master_cell_id(), layer);
                if(extra_demand > 0){
                    // there's an extra demand on Pos3d(row,col,later_id) between thisCell and itCell
                    if(this->add_adjHR_gird_extra_demand_cell(rightGridCell,layer)){
                        if(!_chip->get_grid(Pos3d(layer,_row,_col)).add_extraDmd(extra_demand)){
                            this->remove_adjHR_gird_extra_demand_cell(rightGridCell,layer);
                            success = false;
                            break;
                        }
                        if(success && !_chip->get_grid(Pos3d(layer,_row,_col+1)).add_extraDmd(extra_demand)){
                            _chip->get_grid(Pos3d(layer,_row,_col)).remove_extraDmd(extra_demand);
                            this->remove_adjHR_gird_extra_demand_cell(rightGridCell,layer);
                            success = false;
                            break;
                        }
                    }
                }
            }
            if(!success){
                this->remove_extra_demand();
                return false;
            }
        }
    }
    return true;
}
void CellInstance_C::remove_extra_demand(){
    // this cell has been removed from _chip->_pos2Cell_list
    // So first should remove from cells which paired with this cell
    // Then check if there is a new cell can pair with it
    // same grid demand
    for(int layer=0;layer<_chip->get_layer_num();++layer){
        auto it = _layLsit_msId2sameG_cell[layer].begin();
        while(it!=_layLsit_msId2sameG_cell[layer].end()){
            int extra_demand = _extraDemand->get_same_demand(this->get_master_cell_id(), it->first, layer);
            this->remove_same_gird_extra_demand_cell(it->second,layer);
            _chip->get_grid(Pos3d(layer,_row,_col)).remove_extraDmd(extra_demand);
            it->second->set_extra_demand();
            it = _layLsit_msId2sameG_cell[layer].begin();
        }
    }

    // adjH grid demand
    for(int layer=0;layer<_chip->get_layer_num();++layer){ // left grid
        auto it = _layLsit_msId2adjHLG_cell[layer].begin();
        while(it!=_layLsit_msId2adjHLG_cell[layer].end()){
            int extra_demand = _extraDemand->get_adj_demand(this->get_master_cell_id(), it->first, layer);
            this->remove_adjHL_gird_extra_demand_cell(it->second,layer);
            _chip->get_grid(Pos3d(layer,_row,_col)).remove_extraDmd(extra_demand);
            _chip->get_grid(Pos3d(layer,_row,_col-1)).remove_extraDmd(extra_demand);
            it->second->set_extra_demand();
            it = _layLsit_msId2adjHLG_cell[layer].begin();
        }
    }
    for(int layer=0;layer<_chip->get_layer_num();++layer){ // right grid
        auto it = _layLsit_msId2adjHRG_cell[layer].begin();
        while(it!=_layLsit_msId2adjHRG_cell[layer].end()){
            int extra_demand = _extraDemand->get_adj_demand(this->get_master_cell_id(), it->first, layer);
            this->remove_adjHR_gird_extra_demand_cell(it->second,layer);
            _chip->get_grid(Pos3d(layer,_row,_col)).remove_extraDmd(extra_demand);
            _chip->get_grid(Pos3d(layer,_row,_col+1)).remove_extraDmd(extra_demand);
            it->second->set_extra_demand();
            it = _layLsit_msId2adjHRG_cell[layer].begin();
        }
    }
}
void CellInstance_C::remove_from_chip(){
    _chip->remove_cell_fromPos(this);
}
bool CellInstance_C::add_to_chip(Pos2d pos){
    set_pos(pos);
    if(_chip->add_cell_toPos(this)){ // here will call init_route()
        return true;
    }
    else{ // clean up the cell
        ////cout << "\033[94m[Cell]\033[0m - Cell \'" << _name << "\' cannot add to "<< pos2d2str(pos) << ".\n";
        return false;
    }
}
bool CellInstance_C::move_to(Pos2d pos){
    Pos2d ori_pos = get_pos();
    _chip->remove_cell_fromPos(this);
    for(auto &net : _relate_net_list){
        net->remove_all_route();
        net->init_route();
    }

    set_pos(pos);
    if(_chip->add_cell_toPos(this)){ // here will do init_route() for only this cell
        return true;
    }
    // put it back
    ////cout << "\033[94m[Cell]\033[0m - Cell \'" << _name << "\' move back.\n";
    for(auto &net : _relate_net_list)
        net->remove_all_route();
    set_pos(ori_pos);
    assert(_chip->add_cell_toPos(this));
    for(auto &net : _relate_net_list)
        net->init_route();
    return false;
}

bool CellInstance_C::check_pin_demand(){
    vector<set<Net_C*>> layer_Net;
    layer_Net.resize(_chip->get_layer_num());
    for(auto &pin : _pin_list){
        for(int i=0;i<pin->get_net_num();++i){
            layer_Net[pin->get_layer_id()].insert(pin->get_net_by_id(i));
            if(pin->isCopiedPin()){
                for(int z=pin->get_ori_layer_id();z<pin->get_layer_id();++z){
                    layer_Net[z].insert(pin->get_net_by_id(i));
                }
            }
        }
    }
    for(int i=0;i<layer_Net.size();++i){
        int count_adding_demand = 0;
        for(auto &net : layer_Net[i]){
            if(!_chip->get_grid(Pos3d(i,_row,_col)).isNetHasExist(net)){
                count_adding_demand++;
            }
        }
        if(_chip->get_grid(Pos3d(i,_row,_col)).get_remain_demand() >=0 && _chip->get_grid(Pos3d(i,_row,_col)).get_remain_demand() < count_adding_demand){
            return false;
        }
    }

    for(auto &pin : _pin_list){
        for(int i=0;i<pin->get_net_num();++i){
            assert(pin->get_net_by_id(i)->add_route(pin->get_pos()));
            if(pin->isCopiedPin()){
                for(int z=pin->get_ori_layer_id();z<pin->get_layer_id();++z)
                    assert(pin->get_net_by_id(i)->add_route(Pos3d(z,get<1>(pin->get_pos()),get<2>(pin->get_pos()))));
            }
        }
    }
    return true;
}

bool CellInstance_C::canUseOnlyOneLayer(){
    for(auto &net : _relate_net_list)
        if(net->get_min_layer() == _chip->get_layer_num()-1)
            return true;
    return false;
}

vector<int> CellInstance_C::new_demand_in_newPos(Pos2d pos){
    vector<int> v_size0;
    vector<int> v_all0;
    v_all0.resize(_chip->get_layer_num(),0);

    Pos2d _ori_pos = get_pos();
    vector<int> old_demand0, new_demand0; // left or behind
    vector<int> old_demand1, new_demand1;
    vector<int> old_demand2, new_demand2; // right or front
    vector<int> demand_count; // legal demand for each layer
    old_demand0.resize(_chip->get_layer_num());
    new_demand0.resize(_chip->get_layer_num());
    old_demand1.resize(_chip->get_layer_num());
    new_demand1.resize(_chip->get_layer_num());
    old_demand2.resize(_chip->get_layer_num());
    new_demand2.resize(_chip->get_layer_num());
    for(int i=0;i<_chip->get_layer_num();++i){
        if(_chip->get_layer_dir(i) == V){
            if(get<0>(pos)-1 >= 0)
                old_demand0[i] = _chip->get_grid(Pos3d(i,get<0>(pos)-1,get<1>(pos))).get_demand();
            else
                old_demand0[i] = 0;
            if(get<0>(pos)+1 < _chip->get_num_rows())
                old_demand2[i] = _chip->get_grid(Pos3d(i,get<0>(pos)+1,get<1>(pos))).get_demand();
            else
                old_demand2[i] = 0;
        }
        else{
            if(get<1>(pos)-1 >= 0)
                old_demand0[i] = _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos)-1)).get_demand();
            else
                old_demand0[i] = 0;
            if(get<1>(pos)+1 < _chip->get_num_rows())
                old_demand2[i] = _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos)+1)).get_demand();
            else
                old_demand2[i] = 0;
        }
        old_demand1[i] = _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos))).get_demand();
    }
    
    if(this->get_pos() == pos)
        return v_all0;
    if(this->is_fixed())
        return v_size0;
    if(this->move_to(pos)){
        for(int i=0;i<_chip->get_layer_num();++i){
            if(_chip->get_layer_dir(i) == V){
                if(get<0>(pos)-1 >= 0)
                    new_demand0[i] = _chip->get_grid(Pos3d(i,get<0>(pos)-1,get<1>(pos))).get_demand() - old_demand0[i];
                if(get<0>(pos)+1 < _chip->get_num_rows())
                    new_demand2[i] = _chip->get_grid(Pos3d(i,get<0>(pos)+1,get<1>(pos))).get_demand() - old_demand2[i];
            } else{
                if(get<1>(pos)-1 >= 0)
                    new_demand0[i] = _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos)-1)).get_demand() - old_demand0[i];
                if(get<1>(pos)+1 < _chip->get_num_rows())
                    new_demand2[i] = _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos)+1)).get_demand() - old_demand2[i];
            }
            new_demand1[i] = _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos))).get_demand() - old_demand1[i];
            new_demand1[i] = (new_demand0[i] + new_demand1[i] + new_demand2[i]) / 3;
        }
        move_to(_ori_pos);
        return new_demand1;
    }
    else{
        return v_size0;
    }
}

double CellInstance_C::cost_dmdDensity_in_newPos(Pos2d pos){
    double old_density=0.0, new_density=0.0;
    if(this->get_pos() == pos)
        return 999;
    if(this->is_fixed())
        return 999;

    Pos2d ori_pos = get_pos();
    int cnt_legal_grid = 0;
    for(int i=0;i<_chip->get_layer_num();++i){
        old_density += _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos))).get_density();
        if(get<1>(pos)-1 >= 0){
            old_density += _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos)-1)).get_density();
            cnt_legal_grid++;
        }
        if(get<1>(pos)+1 < _chip->get_num_rows()){
            old_density += _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos)+1)).get_density();
            cnt_legal_grid++;
        }
    }
    if(cnt_legal_grid==0) cnt_legal_grid=1;
    old_density /= cnt_legal_grid;
    
    if(this->move_to(pos)){
        cnt_legal_grid = 0;
        for(int i=0;i<_chip->get_layer_num();++i){
            new_density += _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos))).get_density();
            if(get<1>(pos)-1 >= 0){
                new_density += _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos)-1)).get_density();
                cnt_legal_grid++;
            }
            if(get<1>(pos)+1 < _chip->get_num_rows()){
                new_density += _chip->get_grid(Pos3d(i,get<0>(pos),get<1>(pos)+1)).get_density();
                cnt_legal_grid++;
            }
        }
        if(cnt_legal_grid==0) cnt_legal_grid=1;
        new_density /= cnt_legal_grid;
        move_to(ori_pos);
        return (new_density - old_density);
    }
    else{
        return 999;
    }
}

vector<int> CellInstance_C::cost_demand_in_oriPos(){
    Pos2d _ori_pos = get_pos();
    _chip->remove_cell_fromPos(this);

    vector<int> old_demand0, cost_demand0;
    vector<int> old_demand1, cost_demand1;
    vector<int> old_demand2, cost_demand2;
    old_demand0.resize(_chip->get_layer_num());
    cost_demand0.resize(_chip->get_layer_num());
    old_demand2.resize(_chip->get_layer_num());
    cost_demand2.resize(_chip->get_layer_num());
    old_demand1.resize(_chip->get_layer_num());
    cost_demand1.resize(_chip->get_layer_num());
    for(int i=0;i<_chip->get_layer_num();++i){
        if(_chip->get_layer_dir(i) == V){
            if(get<0>(_ori_pos)-1 >= 0)
                old_demand0[i] = _chip->get_grid(Pos3d(i,get<0>(_ori_pos)-1,get<1>(_ori_pos))).get_demand();
            else
                old_demand0[i] = 0;
            if(get<0>(_ori_pos)+1 < _chip->get_num_rows())
                old_demand2[i] = _chip->get_grid(Pos3d(i,get<0>(_ori_pos)+1,get<1>(_ori_pos))).get_demand();
            else
                old_demand2[i] = 0;
        }
        else{
            if(get<1>(_ori_pos)-1 >= 0)
                old_demand0[i] = _chip->get_grid(Pos3d(i,get<0>(_ori_pos),get<1>(_ori_pos)-1)).get_demand();
            else
                old_demand0[i] = 0;
            if(get<1>(_ori_pos)+1 < _chip->get_num_rows())
                old_demand2[i] = _chip->get_grid(Pos3d(i,get<0>(_ori_pos),get<1>(_ori_pos)+1)).get_demand();
            else
                old_demand2[i] = 0;
        }
        old_demand1[i] = _chip->get_grid(Pos3d(i,get<0>(_ori_pos),get<1>(_ori_pos))).get_demand();
    }
    _chip->remove_cell_fromPos(this);
    for(int i=0;i<_chip->get_layer_num();++i){
        if(_chip->get_layer_dir(i) == V){
            if(get<0>(_ori_pos)-1 >= 0)
                cost_demand0[i] = _chip->get_grid(Pos3d(i,get<0>(_ori_pos)-1,get<1>(_ori_pos))).get_demand() - old_demand0[i];
            if(get<0>(_ori_pos)+1 < _chip->get_num_rows())
                cost_demand2[i] = _chip->get_grid(Pos3d(i,get<0>(_ori_pos)+1,get<1>(_ori_pos))).get_demand() - old_demand2[i];
        } else{
            if(get<1>(_ori_pos)-1 >= 0)
                cost_demand0[i] = _chip->get_grid(Pos3d(i,get<0>(_ori_pos),get<1>(_ori_pos)-1)).get_demand() - old_demand0[i];
            if(get<1>(_ori_pos)+1 < _chip->get_num_rows())
                cost_demand2[i] = _chip->get_grid(Pos3d(i,get<0>(_ori_pos),get<1>(_ori_pos)+1)).get_demand() - old_demand2[i];
        }
        cost_demand1[i] = old_demand1[i] - _chip->get_grid(Pos3d(i,get<0>(_ori_pos),get<1>(_ori_pos))).get_demand();
        cost_demand1[i] = (cost_demand0[i]+cost_demand1[i]+cost_demand2[i]) / 3;
    }
    set_pos(_ori_pos);
    _chip->add_cell_toPos(this);
    
    return cost_demand1;
}
int CellInstance_C::distance_pin_to_relate_netRoute(){
    Pin_C* pPin;
    for(auto& pin: _pin_list){

    }
}

// class Connect_C
/*Connect_C::Connect_C(Pos3d pos_s, Pos3d pos_e, Net_C* net){
    _pos_s = pos_s;
    _pos_e = pos_e;
    _net = net;
    _dir = getDir(pos_s, pos_e);
    _length = getLength(pos_s, pos_e);
    if(_dir == Z){
        if(get<0>(pos_s) > get<0>(pos_e)) swap(_pos_s,_pos_e);
        for(int i=0;i<_length;++i){
            v_pos.push_back(Pos3d(get<0>(_pos_s)+i,get<1>(_pos_s),get<2>(_pos_s)));
        }
    }
    else if(_dir == V){
        if(get<1>(pos_s) > get<1>(pos_e)) swap(_pos_s,_pos_e);
        for(int i=0;i<_length;++i){
            v_pos.push_back(Pos3d(get<0>(_pos_s),get<1>(_pos_s)+i,get<2>(_pos_s)));
        }
    }
    else if( _dir == H){
        if(get<2>(pos_s) > get<2>(pos_e)) swap(_pos_s,_pos_e);
        for(int i=0;i<_length;++i){
            v_pos.push_back(Pos3d(get<0>(_pos_s),get<1>(_pos_s),get<2>(_pos_s)+i));
        }
    }
}*/

// class Net_C

Net_C::Net_C(string name, int id, int min_layer, Chip_C* p_pChip):
_name(name), _id(id), _min_layer(min_layer), _chip(p_pChip),_route_priority(0)
{}

string Net_C::get_name() const {return _name;}

int Net_C::get_id() const {return _id;}

int Net_C::get_min_layer() const {return _min_layer;}

void Net_C::set_id(int val) {_id = val;}

void Net_C::add_pin(Pin_C *pin)
{
    _pins.push_back(pin);
    pin->add_net(this);
    _pins_grid.push_back(pin->get_pos());
}

int Net_C::get_HPWL(){
    Pos3d p;
    int max_row=0;
    int min_row=INT_MAX;
    int max_col=0;
    int min_col=INT_MAX;
    int max_layer=0;
    int min_layer=INT_MAX;
    for(auto& pin: _pins){
        p = pin->get_pos();
        max_row = max(max_row, get<1>(p));
        min_row = min(min_row, get<1>(p));
        max_col = max(max_col, get<2>(p));
        min_col = min(min_col, get<2>(p));
        max_layer = max(max_layer, get<0>(p));
        min_layer = min(min_layer, get<0>(p));
    }
    return max_row + max_col + max_layer - min_layer - min_col- min_row ;
}

bool Net_C::add_route(Pos3d pos) {
    vector<int> grid_net_list = _chip->get_grid(pos).get_net_id_list();
    if(find(grid_net_list.begin(),grid_net_list.end(),this->get_id())!=grid_net_list.end()){
        return true; // the grid has been used by this net
    }
    if(_chip->add_route_to_grid(pos,this)){
        for(auto &pin : _pins){
            if(pin->get_pos() == pos){
                pin->setConnect(true);
            }
        }
        _route.emplace_back(pos);
        return true;
    }
    else {
        ////cout << "\033[94m[module]\033[0m - Net \'" << this->_name << "\' overflow in grid " << pos2str(pos) << "\n";
        return false;
    }
}

void Net_C::remove_route(Pos3d pos){
    auto it = find(_route.begin(),_route.end(),pos);
    if(it != _route.end()){
        _chip->get_grid(pos).remove_net(this);
        _route.erase(it);
    }
}

void Net_C::remove_all_route(){
    _chip->remove_route_from_grid(_route,this);
    for(auto &pin : _pins){
        pin->setConnect(false);
    }
    _route.clear();
}

bool Net_C::init_route(){
    // preAdd route for all pin
    // reconnect the via which is lower than min routing layer
    for(auto &pin : _pins){
        if(!pin->isOnChip()) continue;
        if(pin->isCopiedPin()){
            for(int z=get<0>(pin->get_ori_pos());z<=get<0>(pin->get_pos());++z){
                if(!this->add_route(Pos3d(z,get<1>(pin->get_pos()),get<2>(pin->get_pos())))){
                    this->remove_all_route();
                    return false;
                }
            }
        }
        else{
            if(!this->add_route(pin->get_pos())){
                this->remove_all_route();
                return false;
            }
        }
    }
    return true;
}

bool Net_C::check_all_pin_connected(){
    for(auto &pin : _pins){
        if(!pin->isConnected())
            return false;
    }
    return true;
}
void Net_C::trans_routes2wires(){
    int lay = _chip->get_layer_num();
    int row = _chip->get_num_rows();
    int col = _chip->get_num_cols();
    bool Table[lay][row][col];
    // initialize
    for(int i=0;i<lay;++i)
        for(int j=0;j<row;++j)
            for(int k=0;k<col;++k)
                Table[i][j][k] = false;

    // put route
    for(auto &pos : _route){
        Table[get<0>(pos)][get<1>(pos)][get<2>(pos)] = true;
    }
    for(auto& pos: _route){
        for(int l=-1;l<=1;l+=2){
            if(get<0>(pos)+l>=0&& get<0>(pos)+l<lay && Table[get<0>(pos)+l][get<1>(pos)][get<2>(pos)] ){
                Pos3d end = Pos3d(get<0>(pos)+l,get<1>(pos),get<2>(pos));
                _wires.emplace_back(make_pair(pos,end)); 
                break;   
            }             
        }
        if(get<0>(pos)%2 == 1 && get<0>(pos)>=_min_layer){
            for(int r=-1;r<=1;r+=2){
                if(get<1>(pos)+r>=0&& get<1>(pos)+r<row && Table[get<0>(pos)][get<1>(pos)+r][get<2>(pos)] ){
                    Pos3d end = Pos3d(get<0>(pos),get<1>(pos)+r,get<2>(pos));
                    _wires.emplace_back(make_pair(pos,end)); 
                    break;   
                }    
            }      
        }
        if(get<0>(pos)%2 == 0 && get<0>(pos)>=_min_layer){
            for(int c=-1;c<=1;c+=2){
                if(get<2>(pos)+c>=0&& get<2>(pos)+c<col && Table[get<0>(pos)][get<1>(pos)][get<2>(pos)+c] ){
                    Pos3d end = Pos3d(get<0>(pos),get<1>(pos),get<2>(pos)+c);
                    _wires.emplace_back(make_pair(pos,end)); 
                    break;   
                }       
            }    
        }
    }
}

void Net_C::set_all_pin_connect(){
    for(auto &pin : _pins){
        pin->setConnect(true);
    }
}

bool Net_C::isPin(Pos3d pos){
    return (find(_pins_grid.begin(),_pins_grid.end(),pos) != _pins_grid.end());
}
Pos2d Net_C::get_route_center(){
    Pos2d sum2d;
    for(auto& route:_route){
        sum2d = sum2d + Pos2d(get<1>(route),get<2>(route));
    }
    if(_route.size()==0) return sum2d;
    else return sum2d/_route.size();
}

// class CellLibrary_C

CellLibrary_C::CellLibrary_C(){}

MasterCell_C CellLibrary_C::get_master_cell_by_name(string name, bool &success) const
{
    int id = get_id_by_name(name);
    if(id==-1){
        success = false;
        return MasterCell_C(0,0);
    }
    return _master_cell_list.at(id);
}

void CellLibrary_C::add_master_cell(MasterCell_C ms)
{
    int id = _master_cell_list.size();
    ms.set_id(id);
    _master_cell_list.push_back(ms);
    _name2id[ms.get_name()] = id;
}


// class ExtraDemand_C

ExtraDemand_C::ExtraDemand_C(){}

int ExtraDemand_C::get_same_demand(int ms1, int ms2, int layer) const
{
    ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
    if(same_grid.find(cond)==same_grid.end()) return 0;
    return same_grid.at(cond);
}

int ExtraDemand_C::get_adj_demand(int ms1, int ms2, int layer) const
{
    ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
    if(adj_hor_grid.find(cond)==adj_hor_grid.end()) return 0;
    return adj_hor_grid.at(cond);
}

void ExtraDemand_C::add_same_demand(int ms1, int ms2, int layer, int demand)
{
    ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
    same_grid[cond] = demand;
}

void ExtraDemand_C::add_adj_demand(int ms1, int ms2, int layer, int demand)
{
    ExtraDemandCondition cond = get_cond(ms1, ms2, layer);
    adj_hor_grid[cond] = demand;
}

void ExtraDemand_C::dump_all_cond(ofstream& file){
    for(auto &cond : same_grid){
        file << "  same_grid: ms1=" << get<0>(cond.first) << " ms2=" << get<1>(cond.first) << ", layer=" << get<2>(cond.first) << ", demand=" << get_same_demand(get<0>(cond.first),get<1>(cond.first),get<2>(cond.first)) << "\n";
    }
    for(auto &cond : adj_hor_grid){
        file << "  adj_hor_grid: ms1=" << get<0>(cond.first) << " ms2=" << get<1>(cond.first) << ", layer=" << get<2>(cond.first) << ", demand=" << get_adj_demand(get<0>(cond.first),get<1>(cond.first),get<2>(cond.first)) << "\n";
    }
}

void ExtraDemand_C::print_all_cond(){
    for(auto &cond : same_grid){
        cout << "  same_grid: ms1=" << get<0>(cond.first) << " ms2=" << get<1>(cond.first) << ", layer=" << get<2>(cond.first) << ", demand=" << get_same_demand(get<0>(cond.first),get<1>(cond.first),get<2>(cond.first)) << "\n";
    }
    for(auto &cond : adj_hor_grid){
        cout << "  adj_hor_grid: ms1=" << get<0>(cond.first) << " ms2=" << get<1>(cond.first) << ", layer=" << get<2>(cond.first) << ", demand=" << get_adj_demand(get<0>(cond.first),get<1>(cond.first),get<2>(cond.first)) << "\n";
    }
}

// class Design_C

Design_C::Design_C(int max_cell_move, int num_cells, int num_nets) :
	_max_cell_move(max_cell_move)
{
    _cell_list.clear();
    _net_list.clear();
    _moved_cell_id.clear();
    _cell_name2id.clear();
    _net_name2id.clear();
}

CellInstance_C* Design_C::get_cell_by_id(int id) const {return _cell_list.at(id);}

Net_C* Design_C::get_net_by_id(int id) const {return _net_list.at(id);}

CellInstance_C* Design_C::get_cell_by_name(string name) const {return _cell_list.at(_cell_name2id.at(name));}

void CellInstance_C::setCellBestPos(){
    CellInstance_C* pCell;
    Net_C* pNet;
    Pin_C* pPin;
    Pos2d sum_pos;
    int routeNum=0;
    if(_relate_net_list.size()==0){
        _best_pos = this->get_pos();
        return;
    }
    for(auto& net: _relate_net_list){
        for(int routeIter=0;routeIter<net->get_route_num();++routeIter){
            sum_pos = sum_pos+ Pos2d(get<1>(net->get_route_by_id(routeIter)),get<2>(net->get_route_by_id(routeIter)));
        }
        routeNum += net->get_route_num();
    }
    /*
    for(auto& net: _relate_net_list){
        sum_pos = sum_pos+ net->get_route_center();
    }*/
    if(routeNum==0) routeNum=1;
    _best_pos = sum_pos/routeNum;
}
void CellInstance_C::setCellBestPos_force(string sel){
    CellInstance_C* pCell;
    Net_C* pNet;
    Pin_C* pPin;
    Pos2d sum_pos;
    int routeNum=0;
    int pinNum=0;
    if(sel == "first"){_best_pos = Pos2d(_row,_col);
        return;
    }

    if(_relate_net_list.size()==0){
        _best_pos = this->get_pos();
        return;
    }
    for(auto& net: _relate_net_list){
        for(int pinIter=0;pinIter<net->get_pin_num();++pinIter){
            pPin = net->get_pin_by_id(pinIter);
            //if(this-> != get<1>(pPin) || this != get<2>(pPin)){
                sum_pos = sum_pos+ Pos2d(get<1>(pPin->get_best_pos()),get<2>(pPin->get_best_pos()));
                pinNum++;
        }
    }
    if(pinNum==0) pinNum = 1;
    _best_pos = sum_pos/pinNum;
}
Pos2d get_nearest_pos(CellInstance_C* p_pCell, Net_C* p_pNet){
    Pin_C* pPin=0;
    int minDistance=INT_MAX;
    int minId=-1;
    Pos2d distance;
    for(int pinIter=0;pinIter<p_pNet->get_pin_num();++pinIter){
        pPin = p_pNet->get_pin_by_id(pinIter);
        if(pos3dTo2d(pPin->get_best_pos()) != p_pCell->get_best_pos()){
            distance = pos3dTo2d(pPin->get_best_pos()) - p_pCell->get_best_pos();
            if(abs(get<0>(distance))+abs(get<1>(distance)) < minDistance){
                minDistance = abs(get<0>(distance))+abs(get<1>(distance));
                minId = pinIter;
            }
        }
    }
    if(minDistance == INT_MAX)  p_pCell->get_pos();
    else    return pos3dTo2d(p_pNet->get_pin_by_id(minId)->get_pos());
}
void CellInstance_C::setCellBestPos_nestestPin(){
    CellInstance_C* pCell;
    Net_C* pNet;
    Pin_C* pPin;
    Pos2d sum_pos;
    int routeNum=0;
    int pinNum=0;
    if(_relate_net_list.size()==0){
        _best_pos = this->get_pos();
        return;
    }

    for(auto& net: _relate_net_list){
        sum_pos = get_nearest_pos( this, net)+sum_pos;
    }
    if(_relate_net_list.size()==0) _best_pos = sum_pos;
    else _best_pos = sum_pos/_relate_net_list.size();
}
void CellInstance_C::setCellBestPos_nestestPin_toCenter(){
    CellInstance_C* pCell;
    Pos2d center = Pos2d(_chip->get_num_rows(),_chip->get_num_cols());
    int ori_distance_to_center = abs(get<0>(center)-get<0>(this->get_pos())) + abs(get<1>(center)-get<1>(this->get_pos()));
    int to_pin_row;
    int to_pin_col;
    Pos2d pos;
    Net_C* pNet;
    Pin_C* pPin;
    Pos2d sum_pos;
    int routeNum=0;
    int pinNum=0;
    if(_relate_net_list.size()==0){
        _best_pos = this->get_pos();
        return;
    }
    int cnt_force_pin = 0;
    for(auto& net: _relate_net_list){
        pos = get_nearest_pos( this, net);
        int new_distance_to_center = abs(get<0>(center)-get<0>(pos)) + abs(get<1>(center)-get<1>(pos));
        if(new_distance_to_center > ori_distance_to_center){
            sum_pos = pos+sum_pos;
            cnt_force_pin++;
        }
    }
    if(cnt_force_pin == 0){
        for(auto& net: _relate_net_list){
            pos = get_nearest_pos( this, net);
            sum_pos = pos + sum_pos;
            cnt_force_pin++;
        }
    }
    if(cnt_force_pin==0) _best_pos = sum_pos;
    else _best_pos = sum_pos / cnt_force_pin;
}


Pos2d get_u(vector<Pos2d> v_pos){
    Pos2d sum_pos;
    for(auto& pos: v_pos)
        sum_pos = sum_pos + pos;
    if(v_pos.size() == 0) return sum_pos;
    else return (sum_pos / v_pos.size());
}
double compute_deviation(vector<Pos2d> v_pos){
    Pos2d u_pos = get_u(v_pos);
    
    double deviation = 0;
    for(auto& pos : v_pos){
        deviation += pow(getHPWL(pos,u_pos),2);
    }
    return deviation;
}
bool rand_access(double p){
    int i = 100000000*p;
    if(p == 0) i = 2;
    return rand()%100000000 < i ;
}
void CellInstance_C::setCellBestPos_rand(){
    CellInstance_C* pCell;
    Net_C* pNet;
    Pin_C* pPin;
    Pos2d sum_pos = Pos2d(0,0);
    int routeNum=0;
    int pinNum=0;
    double min_deviation = INT_MAX;
    vector<Pos2d> vec_pos;
    vector<Pos2d> vec_best_pos;
    vec_pos.resize(_relate_net_list.size());

    if(_relate_net_list.size()==0){
        _best_pos = this->get_pos();
        return;
    }

    // init rand pin
    for(int i=0;i<_relate_net_list.size();++i){
        int rand_pin = rand() % _relate_net_list[i]->get_pin_num();
        int cnt_loop=0;
        while(_relate_net_list[i]->get_pin_by_id(rand_pin)->get_cell()!=this){
            cnt_loop++;
            rand_pin = rand() % _relate_net_list[i]->get_pin_num();
            if(cnt_loop>5) break;
        }
        vec_pos[i] = pos3dTo2d(_relate_net_list[i]->get_pin_by_id(rand_pin)->get_pos());
    }
    vec_best_pos = vec_pos;
    int epoch = 0;
    double T = (double)INT_MAX; // init temperature
    while(epoch < 9000){ // SA
        int rand_net = rand() % _relate_net_list.size();
        int rand_pin = rand() % _relate_net_list[rand_net]->get_pin_num();
        Pos2d ori_pin_pos = vec_pos[rand_net];
        vec_pos[rand_net] = pos3dTo2d(_relate_net_list[rand_net]->get_pin_by_id(rand_pin)->get_pos());
        int cur_deviation = compute_deviation(vec_pos);
        if(cur_deviation==0){
            ////cout << "[module] - SA find best_pos in epoch " << epoch << ", cur_deviation=" << cur_deviation << "\n";
            break;
        }
        if(cur_deviation < min_deviation){
            min_deviation = cur_deviation;
            vec_best_pos = vec_pos;
        }
        if(cur_deviation > min_deviation && !rand_access(exp(-abs(cur_deviation-min_deviation)/T))){ // recover
            vec_pos[rand_net] = ori_pin_pos;
        }

        if(epoch % 1000 == 0){
            ////cout << "[module] - SA for best_pos in epoch " << epoch << ", cur_deviation=" << cur_deviation << "\n";
        }
        epoch++;
        T *=0.99;
    }

    for(auto& pos: vec_best_pos){ // get avg pos
        sum_pos = pos + sum_pos;
    }
    if( _relate_net_list.size()==0) _best_pos = sum_pos;
    else _best_pos = sum_pos / _relate_net_list.size();
}
void CellInstance_C::reset_best_pos(){
    _best_pos = get_pos();
}

void CellInstance_C::get_relate_cell(vector<CellInstance_C*>& p_vCell){
    Net_C* pNet=0;
    Pin_C* pPin=0;
    p_vCell.clear();
    for(int netIter=0;netIter<this->get_relate_net_num();++netIter){
        pNet = this->get_relate_net_by_id(netIter);
        for(int pinIter=0;pinIter<pNet->get_pin_num();++pinIter){
            pPin = pNet->get_pin_by_id(pinIter);
            if(find(p_vCell.begin(), p_vCell.end(), pPin->get_cell()) == p_vCell.end()){
                p_vCell.push_back(pPin->get_cell());
            }
        }
    }
}



Net_C* Design_C::get_net_by_name(string name) const {return _net_list.at(_net_name2id.at(name));}

int Design_C::get_total_WL() const{
    int sum_WL=0;
    for(auto &net : _net_list){
        sum_WL += net->get_WL();
    }
    return sum_WL;
}

void Design_C::add_cell(CellInstance_C *cell){
    int id = _cell_list.size();
    cell->set_id(id);
    _cell_list.push_back(cell);
    _cell_name2id[cell->get_name()] = id;
}

void Design_C::add_net(Net_C *net){
    int id = _net_list.size();
    net->set_id(id);
    _net_list.push_back(net);
    _net_name2id[net->get_name()] = id;
}

bool Design_C::move_cell_to_pos(CellInstance_C* cell, Pos2d pos){
    if(cell->get_pos() == pos)
        return true;
    if((_moved_cell_id.size() >= _max_cell_move && !cell->is_moved()) || cell->is_fixed()){
        return false;
    }
    if(cell->canUseOnlyOneLayer()){
        return false;
    }
    if(cell->move_to(pos)){
        if(cell->get_ori_pos() == pos) // check moved
            _moved_cell_id.erase(cell->get_id());
        else
            _moved_cell_id.insert(cell->get_id());
        return true;
    }
    else{
        return false;
    }
}



bool Design_C::move_cell_to_pos(int cell_id, Pos2d pos){
    if(get_cell_by_id(cell_id)->get_pos() == pos)
        return true;
    if(_moved_cell_id.size() >= _max_cell_move || get_cell_by_id(cell_id)->is_fixed()){
        return false;
    }
    if(get_cell_by_id(cell_id)->move_to(pos)){
        if(get_cell_by_id(cell_id)->get_ori_pos() == pos)
            _moved_cell_id.erase(cell_id);
        else
            _moved_cell_id.insert(cell_id);
        return true;
    }
    else{
        return false;
    }
}

void Design_C::remove_cell_from_pos(CellInstance_C* cell){
    if(cell->is_fixed()){
        ////cout << "[Warning] - Trying to move fixed cell \'" << cell->get_name() << "\'\n";
        return;
    }
    if(cell->canUseOnlyOneLayer()){
        ////cout << "[Warning] - Trying to move canUseOnlyOneLayer_cell \'" << cell->get_name() << "\'\n";
        return;
    }
    _moved_cell_id.erase(cell->get_id());
    cell->remove_from_chip();
}
bool Design_C::add_cell_to_pos(CellInstance_C* cell, Pos2d pos){
    if(cell->is_fixed()){
        ////cout << "Can't move. The cell is fixed.\n";
        return true;
    }
    if(_moved_cell_id.size() >= _max_cell_move){
        ////cout << "Can't move. Over the max movement constraint.\n";
        return false;
    }
    if(cell->canUseOnlyOneLayer()){
        ////cout << "Can't move. The cell canUseOnlyOneLayer.\n";
        return true;
    }
    if(cell->add_to_chip(pos)){
        if(cell->get_ori_pos() != pos) // check moved
            _moved_cell_id.insert(cell->get_id());
        return true;
    }
    else{
        return false;
    }
}
bool Design_C::move_cells_to_pos(vector<CellInstance_C*> vec_cell, vector<Pos2d> vec_pos){
    vector<Pos2d> vec_ori_pos;
    for(auto cell : vec_cell){
        vec_ori_pos.push_back(cell->get_pos());
        remove_cell_from_pos(cell); 
    }
    for(auto cell : vec_cell){
        for(int i=0;i<cell->get_relate_net_num();++i){
            cell->get_relate_net_by_id(i)->remove_all_route();
            cell->get_relate_net_by_id(i)->init_route();
        }
    }
    
    int success = 0;
    for(int i=0;i<vec_cell.size();++i){
        if(add_cell_to_pos(vec_cell[i],vec_pos[i]))
            success++;
        else break;
    }

    if(success == vec_cell.size()){
        return true;
    }
    else{
        ////cout << "[Design] - move_cells_to_pos failed.\n";
        for(int i=0;i<success;++i){
            vec_cell[i]->remove_from_chip();
            for(int i=0;i<vec_cell[i]->get_relate_net_num();++i)
                vec_cell[i]->get_relate_net_by_id(i)->remove_all_route();
        }
        for(int i=0;i<vec_cell.size();++i)
            add_cell_to_pos(vec_cell[i],vec_ori_pos[i]);
    }
}

void Design_C::dump_cell_info(ofstream& file, int id){
    CellInstance_C* cell = _cell_list[id]; 
    file << cell->get_name() << ": row=" << cell->get_row() << ", col="<< cell->get_col() << ", ms=" << cell->get_master_cell_id() << ", fixed=" << cell->is_fixed() << "\n";
    for(int j=0;j<cell->get_pin_num();++j){
        Pin_C* pin = cell->get_pin_by_id(j);
        file << "  " << pin->get_name() << "(" << pin->get_cell()->get_name() << ") M" << pin->get_layer_id()+1 << ", nets=[";
        for(int k=0;k<pin->get_net_num();++k){
            file << pin->get_net_by_id(k)->get_name();
            if(k != pin->get_net_num()-1) file << ",";
        }
        file << "]\n";
    }
    for(int j=0;j<cell->get_blkg_num();++j){
        Blockage_C* blkg = cell->get_blkg_by_id(j);
        file << "  " << blkg->get_name() << " M" << blkg->get_layer_id()+1 << " demand=" << blkg->get_demand() << " pos=" << pos2str(blkg->get_pos()) << "\n";
    } 
}

void Design_C::print_cell_info(int id){
    CellInstance_C* cell = _cell_list[id]; 
    cout << cell->get_name() << ": row=" << cell->get_row() << ", col="<< cell->get_col() << ", ms=" << cell->get_master_cell_id() << ", fixed=" << cell->is_fixed() << "\n";
    for(int j=0;j<cell->get_pin_num();++j){
        Pin_C* pin = cell->get_pin_by_id(j);
        cout << "  " << pin->get_name() << "(" << pin->get_cell()->get_name() << ") M" << pin->get_layer_id()+1 << ", nets=[";
        for(int k=0;k<pin->get_net_num();++k){
            cout << pin->get_net_by_id(k)->get_name();
            if(k != pin->get_net_num()-1) cout << ",";
        }
        cout << "]\n";
    }
    for(int j=0;j<cell->get_blkg_num();++j){
        Blockage_C* blkg = cell->get_blkg_by_id(j);
        cout << "  " << blkg->get_name() << " M" << blkg->get_layer_id()+1 << " demand=" << blkg->get_demand() << " pos=" << pos2str(blkg->get_pos()) << "\n";
    } 
}

void Design_C::dump_net_info(ofstream& file, int id){
    Net_C* net = _net_list[id];
    file << net->get_name() << ": minRouting=" << net->get_min_layer() << ", Pins=[";
    for(int j=0;j<net->get_pin_num();++j){
        file << net->get_pin_by_id(j)->get_cell()->get_name() << "/" << net->get_pin_by_id(j)->get_name() << pos2str(net->get_pin_by_id(j)->get_ori_pos());
        if(j != net->get_pin_num()-1) file << ",";
    }
    file << "]\n";
    file << "route(all grids):\n";
    for(int j=0;j<net->get_route_num();++j){
        Pos3d pos = net->get_route_by_id(j);
        file << pos2str(pos) << " ";
    }
    file << "\n";
}

void Design_C::print_net_info(int id){
    Net_C* net = _net_list[id];
    cout << net->get_name() << ": minRouting=" << net->get_min_layer() << ", Pins=[";
    for(int j=0;j<net->get_pin_num();++j){
        cout << net->get_pin_by_id(j)->get_cell()->get_name() << "/" << net->get_pin_by_id(j)->get_name() << pos2str(net->get_pin_by_id(j)->get_ori_pos());
        if(j != net->get_pin_num()-1) cout << ", ";
    }
    cout << "]\n";
    cout << "route(all grids):\n";
    for(int j=0;j<net->get_route_num();++j){
        Pos3d pos = net->get_route_by_id(j);
        cout << pos2str(pos) << " ";
    }
    cout << "\n";
}
//net analysis
void Design_C::net_analysis(ofstream& file){
    int net_WL;
    int net_min_layer;
    Pin_C* pPin;
    file <<"netName,minLayer,pinNum,HPWL,WL_without_minlayerVia,WL"<< endl;

    for(auto& net:_net_list){
        net_WL = net->get_WL();
        net_min_layer = net->get_min_layer();
        net_WL=0;
        for(int routeId=0;routeId<net->get_route_num();++routeId){
            if(get<0>(net->get_route_by_id(routeId)) >= net_min_layer){net_WL++;}
        }
        file <<net->get_name()<<','<<net->get_min_layer()<<','<< net->get_pin_num()<< "," << net->get_HPWL()<< ","<<net_WL<<','<<net->get_WL()<< endl;
    }
}
