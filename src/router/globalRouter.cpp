#include "globalRouter.h"
#include "disjoint_set.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <climits>
#include <assert.h>
#include <time.h>
#include <unordered_map>

#define group_num 1

string Route::pos2str(Pos3d pos) {return "("+to_string(get<0>(pos))+","+to_string(get<1>(pos))+","+to_string(get<2>(pos))+")";}
int GlobalRouter_C::getHPWL(Pos pos1, Pos pos2){
    int z0, x0, y0,z1, x1, y1, z2, x2, y2;
    tie(z1, x1, y1) = pos1;
    tie(z2, x2, y2) = pos2;
    int via = 0;
    if(z1 == z2){
        if((z1%2 == 0 && x1 != x2) || (z1%2 == 1 && y1 != y2))
            via = 2;
    }
    return abs(z1-z2) + abs(x1-x2) + abs(y1-y2) + via;
}
Pos GlobalRouter_C::getCenterOfGravity(set<Pos> posSet){
    int z=0,y=0,x=0;
    for(auto &pos : posSet){
        z += get<0>(pos);
        y += get<1>(pos);
        x += get<2>(pos);
    }
    if(posSet.size()==0) return Pos(0,0,0);
    else return Pos(z/posSet.size(),y/posSet.size(),x/posSet.size());
}
int Route::getHPWL(Pos pos1, Pos pos2){
    int z0, x0, y0,z1, x1, y1, z2, x2, y2;
    tie(z1, x1, y1) = pos1;
    tie(z2, x2, y2) = pos2;
    int via = 0;
    if(z1 == z2){
        if((z1%2 == 0 && x1 != x2) || (z1%2 == 1 && y1 != y2))
            via = 2;
    }
    return abs(z1-z2) + abs(x1-x2) + abs(y1-y2) + via;
}

Pos3d GlobalRouter_C::getNearestPos(Pos3d pos,int net_id){
    Pos3d nearestPos = *(_net_to_used_pos[net_id].begin());
    int minDistance = getHPWL(nearestPos,pos);
    for(auto& p : _net_to_used_pos[net_id]){
        int distance = getHPWL(p,pos);
        if(minDistance < distance){
            minDistance = distance;
            nearestPos = p;
        }
    }
    return nearestPos;
}

Vertex::Vertex(const Pos &pos):
    _pos(pos)
{}

Pos Vertex::get_pos() const {return _pos;}

Edge::Edge(Vertex *v1, Vertex *v2):
    _v1(v1),
    _v2(v2)
{}
Edge::~Edge(){/*delete _v1;delete _v2;*/}

Vertex* Edge::get_v1() const {return _v1;}

Vertex* Edge::get_v2() const {return _v2;}

int Edge::get_wl() const
{
    return abs(get<0>(_v1->get_pos()) - get<0>(_v2->get_pos()))
        + abs(get<1>(_v1->get_pos()) - get<1>(_v2->get_pos()))
        + abs(get<2>(_v1->get_pos()) - get<2>(_v2->get_pos()));
}
//steiner tree class
steiner_tree_C::steiner_tree_C(Net_C* p_pNet, Chip_C* p_pChip):m_pChip(p_pChip){
    Pin_C* pPin;
    //add all vertex
    for(int pinId=0;pinId < p_pNet->get_pin_num();++pinId){
        pPin = p_pNet->get_pin_by_id(pinId);
        if(pPin->isOnChip())
            _vertex_list.push_back(new Vertex(pPin->get_pos()));
    }
    //add all edge
    Edge* pEdge;
    for(int i=0;i<_vertex_list.size();++i){
        for(int j=i+1;j<_vertex_list.size();++j){
            pEdge = new Edge(_vertex_list[i],_vertex_list[j]);
            _edge_list.push_back(pEdge);
        }
    }
}
steiner_tree_C::~steiner_tree_C(){
    for(auto& v: _vertex_list){delete v;}
    _vertex_list.clear();
    _edge_list.clear();
    _mst.clear();
}
bool cmpEdge(Edge* e1,Edge* e2){
    return e1->get_wl() > e2->get_wl();
}
void steiner_tree_C::sortEdge(vector<Edge*>& vEdge){
    sort(vEdge.begin(),vEdge.end(), cmpEdge);
}
void steiner_tree_C::builtMst(){
    _mst.clear();
    Edge* minEdge;
    vector<Edge*> vEdge = _edge_list;
    disjoint_set<Vertex*> ds;
    //create disjoint set by vertex 
    for(auto& vertex: _vertex_list){
        ds.insert(vertex);
    }
    make_heap(vEdge.begin(), vEdge.end(), cmpEdge);
    
    while(!vEdge.empty()){
        minEdge = vEdge.front();
        //cout << "minEdge length: "<< minEdge->get_wl() << endl;
        pop_heap(vEdge.begin(), vEdge.end(), cmpEdge); vEdge.pop_back();
        if(ds.find(minEdge->get_v1()) != ds.find(minEdge->get_v2())) {
            _mst.push_back(minEdge);
            ds.merge(minEdge->get_v1(),minEdge->get_v2());
        }
    }
}
void steiner_tree_C::layerAssignment(){
    double occupy_density=0.2;
    int layerAssign=-1;
    for(int i=0;i<m_pChip->get_layer_num();++i){
        //cout << "layer<"<<i<<"> " << m_pChip->get_layer_by_idx(i).cal_density()<< endl;
        if(m_pChip->get_layer_by_idx(i).cal_density() < occupy_density){
            ////cout << "assign layer<"<<i<<">" << endl;
            layerAssign = i; break;
        }
    }
    if(layerAssign == -1){layerAssign = 0;}
    vector<Vertex*> add_vertex;
    Pos3d pos;
    for(auto& vertex: _vertex_list){
        pos = make_tuple(get<0>(vertex->get_pos()),get<1>(vertex->get_pos()),get<2>(vertex->get_pos()));
        add_vertex.push_back(new Vertex(pos));//copy pin
        vertex->modify_layer(layerAssign);//modify mst layer
        _mst.push_back(new Edge(add_vertex.back(),vertex));//connect pin and mst on another layer
    }
}

Route::Route(Pos source, Pos target, Net_C* net){
    _net = net;
    _cost_grids = 0;
    _grids.push_back(source);
    if(!net->_chip->get_grid(source).isNetHasExist(net)){
        _cost_grids++;
        _total_density += net->_chip->get_grid(source).get_density();
    }
    _source = source;
    _curNode = source;
    _target = target;
    int z0, x0, y0,z1, x1, y1, z2, x2, y2;
    tie(z0, x0, y0) = _source;
    tie(z1, x1, y1) = _curNode;
    tie(z2, x2, y2) = target;
    _center = Pos((z0+z2)/2, (y0+y2)/2, (x0+x2)/2);
    _HPWL = abs(z0-z2) + abs(x0-x2) + abs(y0-y2);
    _distance2target = abs(z1-z2) + abs(x1-x2) + abs(y1-y2);
    _dis_cur2tar_2d = abs(x1-x2) + abs(y1-y2);
    _dis_src2tar_2d = abs(x1-x2) + abs(y1-y2);
    _cost = getCost();
}
Route::Route(Pos source, Pos target, Pos center_gravity, Net_C* net){
    _net = net;
    _cost_grids = 0;
    _grids.push_back(source);
    if(!net->_chip->get_grid(source).isNetHasExist(net)){
        _cost_grids++;
        _total_density += net->_chip->get_grid(source).get_density();
        _total_Centrifugal += (getHPWL(_center,center_gravity) > getHPWL(source,center_gravity));
    }
    _source = source;
    _curNode = source;
    _target = target;
    int z0, x0, y0,z1, x1, y1, z2, x2, y2;
    tie(z0, x0, y0) = _source;
    tie(z1, x1, y1) = _curNode;
    tie(z2, x2, y2) = target;
    _center = Pos((z0+z2)/2, (y0+y2)/2, (x0+x2)/2);
    _HPWL = abs(z0-z2) + abs(x0-x2) + abs(y0-y2);
    _distance2target = abs(z1-z2) + abs(x1-x2) + abs(y1-y2);
    _dis_cur2tar_2d = abs(x1-x2) + abs(y1-y2);
    _dis_src2tar_2d = abs(x1-x2) + abs(y1-y2);
    _cost = getCost();
}
Route::Route(Route* route, Pos curNode, Pos target, Pos center_gravity, Net_C* net){
    _net = net;
    _cost_grids = route->_cost_grids;
    _grids = route->_grids;
    _source = route->_source;
    _dis_src2tar_2d = route->_dis_src2tar_2d;
    _center = route->_center;
    _HPWL = route->_HPWL;
    _grids.push_back(curNode);
    if(!net->_chip->get_grid(curNode).isNetHasExist(net)){
        _cost_grids++;
        _total_density += net->_chip->get_grid(curNode).get_density();
        _total_Centrifugal += (getHPWL(_center,center_gravity) > getHPWL(curNode,center_gravity));
    }
    _curNode = curNode;
    _target = target;
    int z1, x1, y1, z2, x2, y2;
    tie(z1, x1, y1) = _curNode;
    tie(z2, x2, y2) = target;
    _distance2target = abs(z1-z2) + abs(x1-x2) + abs(y1-y2);
    _dis_cur2tar_2d = abs(x1-x2) + abs(y1-y2);
    _cost = getCost();
}
double Route::getCost(){
    int a=1, b=_HPWL/10, c=1, d=1;
    return a*(_cost_grids + _distance2target) + b*(_total_density); // + c*(_total_Centrifugal);
}

void Route::print_info(){
    cout << "Length: " << _grids.size() << "\n";
    cout << "Cost: " << _cost<<"\n";
    cout << "source" << pos2str(_curNode) << " --> target" << pos2str(_target) <<"\n";
    for(const Pos3d &pos : _grids){
      cout<<pos2str(pos)<<" ";
    }
    cout << "\n";
}

//Global router class
bool cmpNet(Net_C* n1, Net_C* n2){
    if(n1->get_priority() != n2->get_priority()) return n1->get_priority() > n2->get_priority();
    else if(n1->get_pin_num() == n2->get_pin_num()) return n1->get_HPWL() < n2->get_HPWL();
    else return n1->get_pin_num() < n2->get_pin_num();
}

GlobalRouter_C::GlobalRouter_C(Chip_C* p_pChip, vector<Net_C*>& p_vNet_list)
{
    m_pChip = p_pChip;
    vNet_list = p_vNet_list;
    sorted_net_list = p_vNet_list;
    sort(sorted_net_list.begin(),sorted_net_list.end(),cmpNet);
    _net_to_used_pos.resize(vNet_list.size());
    _unLinked_v.resize(vNet_list.size());
    _net_routing_quota.resize(vNet_list.size());
    _net_routed_edge.resize(vNet_list.size(),0);
    _net_total_edge.resize(vNet_list.size());
    for(int i=0;i<vNet_list.size();++i){
        _net_routing_quota[i] = vNet_list[i]->get_ori_route().size();
        _net_total_edge[i] = vNet_list[i]->get_pin_num()-1;
    }
    //_vertex_list = vertex_list;
    //for(int i = 0; i<_vertex_list.size()-1; i++){
    //    for(int j = i+1; j<_vertex_list.size(); j++){
    //        _edge_list.push_back(
    //            new Edge(_vertex_list[i], _vertex_list[j]));
    //    }
    //}
}
bool GlobalRouter_C::is_net_connected(Net_C* p_pNet){
    m_pChip->label_sub_block();
    int first_label;
    int layer, row, col;
    for(int pinIter=0;pinIter<p_pNet->get_pin_num();++pinIter){
        Pin_C* pPin = p_pNet->get_pin_by_id(pinIter);
        tie(layer, row, col) = pPin->get_pos();
        if(m_pChip->get_label(layer,row,col) == -1) return false;
        if(pinIter == 0){first_label=m_pChip->get_label(layer, row, col);} 
        else if(first_label != m_pChip->get_label(layer, row, col)){return false;}
    }
    return true;
}

bool GlobalRouter_C::reRoute_particular_net(vector<Net_C*> p_sorted_net_list, bool recover){
    ////cout << "\033[94m[GR]\033[0m - reRouting all nets ...\n";

    int finish=0;
    int net_is_connected = 0;
    vector<steiner_tree_C*> v_steiner_tree;
    int oldWL;
    while(finish == 0){
        sort(p_sorted_net_list.begin(),p_sorted_net_list.end(),cmpNet);
        _part_WL = 0;
        net_is_connected = 1;
        oldWL = 0;
        for(auto &net : p_sorted_net_list){
            oldWL += net->get_WL();
            net->remove_all_route();

            assert(net->init_route());
            _net_to_used_pos[net->get_id()].clear();
            _unLinked_v[net->get_id()].clear();
            v_steiner_tree.push_back(new steiner_tree_C(net, m_pChip));
            v_steiner_tree.back()->builtMst();
            // put vertex to grid first
            vector<Edge*> _min_spanning_tree = v_steiner_tree.back()->getMst();
            _net_total_edge[net->get_id()] = _min_spanning_tree.size();
            for(auto &e : _min_spanning_tree){ // for each edge, link v1 and v2
                // net->add_route(e->get_v1()->get_pos());
                // net->add_route(e->get_v2()->get_pos());
                _unLinked_v[net->get_id()].insert(e->get_v1()->get_pos());
                _unLinked_v[net->get_id()].insert(e->get_v2()->get_pos());
            }
        }
        int netWL=0;
        for(int i=0;i<p_sorted_net_list.size();++i){
            netWL = reRoutePrim(p_sorted_net_list[i]);
            if(netWL == -2){
                oldWL = -100;
                break;
            }
            else if(netWL == -1){
                ////cout << "reroute all net after incPriority" << endl;
                oldWL = -100;
                break;
            } 
            else _part_WL += netWL;
        }

        if((_part_WL > oldWL&&recover==true) || oldWL==-100){
            if(_part_WL > oldWL&&recover== true){
                ////cout << "reroute all net after incPriority, so recover nets" << endl;
                ////cout << "oldWL="<<oldWL<< " netWL="<<netWL << endl;
                for(auto& net: p_sorted_net_list){
                    net->remove_all_route();
                }
                for(auto& net: p_sorted_net_list){
                    assert(recoverRoute(net));
                }    
                ////cout << "\033[94m[GR]\033[0m - no reRoute all nets\n";
                return false;
            }
            else{
                ////cout << "\033[94m[GR]\033[0m - no reRoute all nets\n";
                return false;    
            }
        }
        break; 
    }
    for(int i=0;i<p_sorted_net_list.size();++i){
       delete v_steiner_tree[i];
    }
    ////cout << "\033[94m[GR]\033[0m - reRoute all nets \033[92mcompleted!\033[0m\n";
    ////cout << "\033[94m[GR]\033[0m - new total WL = " << _part_WL << "\n";
    return true;
}


void GlobalRouter_C::reRoute_group_net(){
    vector<Net_C*> net_group;
    //int group_num = sorted_net_list.size()/group >0 ?sorted_net_list.size()/10:1;

    int group_num_idx = 1;
    sort(sorted_net_list.begin(),sorted_net_list.end(), cmpNet);
    
    for(int netIter=0;netIter < sorted_net_list.size();++netIter){
        net_group.push_back(sorted_net_list[netIter]);
        if(netIter == group_num*group_num_idx || netIter == sorted_net_list.size()-1){
            ////cout << "group["<< group_num_idx <<"]" << endl;
            reRoute_particular_net(net_group);
            net_group.clear();
            group_num_idx++;
        }
    }
}
void GlobalRouter_C::reRoute_all_net(){
    cout << "\033[94m[GR]\033[0m - reRouting all nets ...\n";
    _totoal_WL = 0;
    int finish=0;
    vector<steiner_tree_C*> v_steiner_tree;
    /*for(auto &net : sorted_net_list){
        net->remove_all_route();
        net->init_route();
        _net_to_used_pos[net->get_id()].clear();
        _unLinked_v[net->get_id()].clear();
        v_steiner_tree.push_back(new steiner_tree_C(net, m_pChip));
        v_steiner_tree.back()->builtMst();
        // put vertex to grid first
        vector<Edge*> _min_spanning_tree = v_steiner_tree.back()->getMst();
        for(auto &e : _min_spanning_tree){ // for each edge, link v1 and v2
            net->add_route(e->get_v1()->get_pos());
            net->add_route(e->get_v2()->get_pos());
            _unLinked_v[net->get_id()].insert(e->get_v1()->get_pos());
            _unLinked_v[net->get_id()].insert(e->get_v2()->get_pos());
        }
    }
    for(int i=0;i<sorted_net_list.size();++i){
        //net->remove_all_route();
        //sum_WL += reRoute(sorted_net_list[i], v_steiner_tree[i]);
        _totoal_WL += reRoutePrim(sorted_net_list[i]);
        delete v_steiner_tree[i];
    }*/
    while(finish == 0){
        sort(sorted_net_list.begin(),sorted_net_list.end(),cmpNet);
        _totoal_WL = 0;
        for(auto &net : sorted_net_list){
            net->remove_all_route();
            assert(net->init_route());
            _net_to_used_pos[net->get_id()].clear();
            _unLinked_v[net->get_id()].clear();
            v_steiner_tree.push_back(new steiner_tree_C(net, m_pChip));
            v_steiner_tree.back()->builtMst();
            // put vertex to grid first
            vector<Edge*> _min_spanning_tree = v_steiner_tree.back()->getMst();
            _net_total_edge[net->get_id()] = _min_spanning_tree.size();
            for(auto &e : _min_spanning_tree){ // for each edge, link v1 and v2
                // net->add_route(e->get_v1()->get_pos());
                // net->add_route(e->get_v2()->get_pos());
                _unLinked_v[net->get_id()].insert(e->get_v1()->get_pos());
                _unLinked_v[net->get_id()].insert(e->get_v2()->get_pos());
            }
        }

        for(int i=0;i<sorted_net_list.size();++i){
            //net->remove_all_route();
            //sum_WL += reRoute(sorted_net_list[i], v_steiner_tree[i]);
            int netWL = reRoutePrim(sorted_net_list[i]);
            //m_pChip->print_all_supply_demand_table();
            if(netWL == -2) exit(1);
            else if(netWL == -1){
                cout << "reroute all net after incPriority" << endl;
                break;
            }
            else _totoal_WL += netWL;
            if(i == sorted_net_list.size()-1) finish = 1;
        }
    }
    for(int i=0;i<sorted_net_list.size();++i){
       delete v_steiner_tree[i];
    }
    cout << "\033[94m[GR]\033[0m - reRoute all nets \033[92mcompleted!\033[0m\n";
    cout << "\033[94m[GR]\033[0m - new total WL = " << _totoal_WL << "\n";
}
bool GlobalRouter_C::recoverRoute(Net_C* net){
    net->remove_all_route();
    assert(net->init_route());
    //net->recover_ori_route();
    _net_to_used_pos[net->get_id()].clear();
    _unLinked_v[net->get_id()].clear();
    for(auto &pos : net->get_ori_route()){// set best route grids to net and chip
        if(!net->add_route(pos)) {
            ////cout << "\033[94m[GR]\033[0m - put origin route back failed.\n";
            return false;
        }
    }
    return true;
}
int count_net = 1;
Pos rootPin = Pos(0,0,0);
bool cmpPin(const Pos &pos1, const Pos &pos2){
    int z0, x0, y0,z1, x1, y1, z2, x2, y2;
    tie(z0, x0, y0) = rootPin;
    tie(z1, x1, y1) = pos1;
    tie(z2, x2, y2) = pos2;
    int via1 = 0, via2 = 0;
    if(z1 == z0){
        if((z1%2 == 0 && x1 != x0) || (z1%2 == 1 && y1 != y0))
            via1 = 2;
    }
    if(z0 == z1){
        if((z0%2 == 0 && x0 != x2) || (z0%2 == 1 && y0 != y2))
            via2 = 2;
    }
    return (abs(z1-z0) + abs(x1-x0) + abs(y1-y0) + via1) < (abs(z0-z2) + abs(x0-x2) + abs(y0-y2) + via2);
}

int GlobalRouter_C::reRoutePrim(Net_C* net){
    ////cout << "\033[94m[GR]\033[0m - reRouting Net with prim \'" << net->get_name() << "\' ... [" << count_net++ << "/" << vNet_list.size() << "]\n";

    vector<Pos> vPinS;
    vector<Pos> vPinSecond;
    for(int i=0;i< net->get_pin_num();i++){
        vPinS.push_back(net->get_pin_by_id(i)->get_pos());
    }
    rootPin = vPinS[0];
    sort(vPinS.begin(),vPinS.end(),cmpPin);
    
    for(int iterLimit=0;!vPinS.empty();++iterLimit){
        for(Pos addPin : vPinS){ // for each pin
            Pos center_gravity = getCenterOfGravity(_unLinked_v[net->get_id()]);
            Route* edgeRoute = Astar_search(net, rootPin, addPin,center_gravity,true);
            if(edgeRoute == nullptr){
                ////cout << " A* search cannot found a route for Net \'" << net->get_name() << "\' at "<<iterLimit<<" times \n";
                if(iterLimit == 0){
                    if(net->get_priority()==1){
                        ////cout << " A* search cannot found a route for Net with incPriority\n";
                        return -2;
                    }
                    else{
                        net->inc_Priority();
                        ////cout << " A* search cannot found a route for Net, and then increase the priority(" << net->get_priority() << ")\n";
                        return -1;// mean no solution
                    }
                } 
                vPinSecond.push_back(addPin);
                continue;
            }
            for(auto &pos : edgeRoute->_grids){// set route grids to net and chip
                assert(net->add_route(pos));
                _net_to_used_pos[net->get_id()].insert(pos); // the grids already used to connect
            }
            _net_routed_edge[net->get_id()]++;
        }    
        vPinS = vPinSecond;
        vPinSecond.clear();
    }

    return net->get_WL();
}
int GlobalRouter_C::reRoute(Net_C* net, steiner_tree_C* st){
    ////cout << "\033[94m[GR]\033[0m - reRouting Net \'" << net->get_name() << "\' ...\n";
    //m_pSteinerTree->layerAssignment();
    vector<Edge*> _min_spanning_tree = st->getMst();

    set<Pos> netRoute_all_grids;
    for(auto &e : _min_spanning_tree){ // for each edge, link v1 and v2
        Vertex* v1 = e->get_v1();
        Vertex* v2 = e->get_v2();

        Pos center_gravity = getCenterOfGravity(_unLinked_v[net->get_id()]);
        Route* edgeRoute = Astar_search(net,v1->get_pos(), v2->get_pos(),center_gravity,true);
        if(edgeRoute == nullptr){
            ////cout << " A* search cannot found a route for Net \'" << net->get_name() << "\'\n";
            exit(1);
        }
        netRoute_all_grids.insert(edgeRoute->_grids.begin(),edgeRoute->_grids.end()); 
        for(auto &pos : edgeRoute->_grids){
            if(!net->add_route(pos))
                exit(1);
            _net_to_used_pos[net->get_id()].insert(pos); // the grids already used to connect
            _unLinked_v[net->get_id()].erase(_unLinked_v[net->get_id()].find(pos));
        }
        _net_routed_edge[net->get_id()]++;
        delete edgeRoute;
    }
    // set best route grids to net and chip
    /*for(auto &pos : netRoute_all_grids){
        if(!net->add_route(pos))
            exit(1);
    }*/
    
    ////if(net->check_all_pin_connected())
        ////cout << "\033[94m[GR]\033[0m - reRouting Net \'" << net->get_name() << "\' ..." << " \033[92mok!\033[0m\n";
        //cout << " \033[92mok!\033[0m\n";
    /*////else {
        cout << " some pins are unconnected.\n";
        cout << "MST Edges:\n";
        for(auto &e : _min_spanning_tree){ // for each edge, link v1 and v2
            Vertex* v1 = e->get_v1();
            Vertex* v2 = e->get_v2();
            string v1_pos_str = "("+to_string(get<0>(v1->get_pos()))+","+to_string(get<1>(v1->get_pos()))+","+to_string(get<2>(v1->get_pos()))+")";
            string v2_pos_str = "("+to_string(get<0>(v2->get_pos()))+","+to_string(get<1>(v2->get_pos()))+","+to_string(get<2>(v2->get_pos()))+")";
            cout << "  " << v1_pos_str << " --> " << v2_pos_str << "\n";
        }
    }////*/

    return net->get_WL();
}

Route* GlobalRouter_C::Astar_search(Net_C* net, Pos source, Pos target, Pos center_gravity, bool source_is_from_connected){ // a* search
    //cout << "A* begin, " << pos2str(source) << "-->" << pos2str(target) << "\n";
    Route* best_route = nullptr;
    Route* new_route = new Route(source, target, center_gravity, net); 
    if(source == target){
        //cout << "A* end, arrived!\n";
        return new_route;
    }

    priority_queue<Route*, vector<Route*>, SortRouteByCost> minHeap_route; // OPEN
    //set<Pos> Astar_using_pos;
    vector<vector<vector<bool> > > useTable;
    // initialize
    useTable.resize(m_pChip->get_layer_num());

    for(int i=0;i<m_pChip->get_layer_num();++i){
        useTable[i].resize(m_pChip->get_num_rows());
        for(int j=0;j<m_pChip->get_num_rows();++j){
            useTable[i][j].resize(m_pChip->get_num_cols(),false);
        }
    }
    
    minHeap_route.push(new_route);
    //Astar_using_pos.insert(source);
    useTable[get<0>(source)][get<1>(source)][get<2>(source)] = true;

    // check if this curNode is already be used by net
    vector<Pos> s_source;
    if(source_is_from_connected){
        for(auto &pos : _net_to_used_pos[net->get_id()]){
            new_route = new Route(pos, target, center_gravity, net);
            minHeap_route.push(new_route);
            //Astar_using_pos.insert(pos);
            useTable[get<0>(pos)][get<1>(pos)][get<2>(pos)] = true;
            s_source.push_back(pos);
        }
    }
    if(!isAchievable(net,s_source,target)){
        return best_route;
    }
    while(!minHeap_route.empty()){
        Route* route = minHeap_route.top(); // get the min_cost route
        minHeap_route.pop();
        //route->print_info();
        // check if Arrive
        if(route->_curNode == target){ 
            best_route = route;
            break;
        }

        // get curNode
        int layer, row, col;
        tie(layer, row, col) = route->_curNode;
        Direction dir = m_pChip->get_layer_dir(layer);

        // check if there's already having this->net point near curNode
        //target = check_ExistBetterTarget(net,route->_curNode,target,5,5,5); // if no, target=ori_target; else target=new_target
        
        // propagate curNode to 4 neibor grid (adjacent*2, up, down)
        // upper layer(Z), +1
        Pos curNode = Pos(layer+1,row,col);
        if(check_route_legal(net, route, curNode, Z, useTable)){
            new_route = new Route(route, curNode, target, center_gravity, net);
            minHeap_route.push(new_route);
            useTable[get<0>(curNode)][get<1>(curNode)][get<2>(curNode)] = true;
        }
        // lower layer(Z), -1
        curNode = Pos(layer-1,row,col);
        if(check_route_legal(net, route, curNode, Z, useTable)){
            new_route = new Route(route, curNode, target, center_gravity, net);
            minHeap_route.push(new_route);
            useTable[get<0>(curNode)][get<1>(curNode)][get<2>(curNode)] = true;
        }
        // left(H)/back(V), -1
        if(dir == V) curNode = Pos(layer,row-1,col); // V (move back)
        else curNode = Pos(layer,row,col-1); // H (move left)
        if(check_route_legal(net, route, curNode, dir, useTable)){
            new_route = new Route(route, curNode, target, center_gravity, net);
            minHeap_route.push(new_route);
            useTable[get<0>(curNode)][get<1>(curNode)][get<2>(curNode)] = true;
        }
        // right(H)/front(V), +1
        if(dir == V) curNode = Pos(layer,row+1,col); // V (move back)
        else curNode = Pos(layer,row,col+1); // H (move left)
        if(check_route_legal(net, route, curNode, dir, useTable)){
            new_route = new Route(route, curNode, target, center_gravity, net);
            minHeap_route.push(new_route);
            useTable[get<0>(curNode)][get<1>(curNode)][get<2>(curNode)] = true;
        }
        delete route; // have been pop from OPEN(minHeap) and merge to CLOSE
    }

    // clear Min heap
    while(!minHeap_route.empty()){
        Route* route = minHeap_route.top(); // get the min_cost route
        minHeap_route.pop();
        if(route != best_route) delete route;
    }
    // reRoute from target to source
    if(best_route == nullptr && source_is_from_connected) 
        best_route = Astar_search(net, target, source, center_gravity,false);
    
    //if(best_route == nullptr)
    //    cout << "A* end, not arrived QQ\n";
    //else cout << "A* end, arrived!\n";
    return best_route;
}

bool GlobalRouter_C::check_route_legal(Net_C* net, Route* route, Pos curNode, Direction dir, vector<vector<vector<bool> > > &useTable){
    // check curNode
    if(!m_pChip->posIsLegal(curNode)) return false;
    // check min_layer
    if(get<0>(curNode) < net->get_min_layer()) return false;
    // check if the grid has been used by the A* search
    if(useTable[get<0>(curNode)][get<1>(curNode)][get<2>(curNode)]) return false;
    // check remain supply
    if(m_pChip->get_grid(curNode).get_remain_demand() <= 0 && !m_pChip->get_grid(curNode).isNetHasExist(net)) return false;
    // 2d WL constraint
    if(route->_dis_cur2tar_2d > route->_dis_src2tar_2d*1.2) return false;
    // 3d WL constraint
    int edge_routing_quota = _net_routing_quota[net->get_id()] - net->get_WL();
    int unRouted_edge = _net_total_edge[net->get_id()]-_net_routed_edge[net->get_id()];
    if(edge_routing_quota==0) edge_routing_quota = 1;
    if(unRouted_edge==0) unRouted_edge=1;
    //if(route->_cost_grids + route->_distance2target > (edge_routing_quota/unRouted_edge)*2) return false;

    // if all pass
    return true;
}

Pos GlobalRouter_C::check_ExistBetterTarget(Net_C* net, Pos cueNode, Pos target,int ext_z, int ext_y, int ext_x){
    int cur_z=get<0>(cueNode),cur_y=get<1>(cueNode),cur_x=get<2>(cueNode);
    for(int lay=cur_z-ext_z;((lay<cur_z+ext_z)&&(lay>=net->get_min_layer())&&(lay<m_pChip->get_layer_num()));++lay){
        for(int row=cur_y-ext_y;((row<cur_y+ext_y)&&(row>=0)&&(row<m_pChip->get_num_rows()));++row){
            for(int col=cur_x-ext_x;((col<cur_x+ext_x)&&(col>=0)&&(col<m_pChip->get_num_cols()));++col){
                if(getHPWL(cueNode,target) > getHPWL(cueNode,Pos(lay,row,col))){
                    target = Pos(lay,row,col);
                }
            }
        }
    }
    return target;
}

bool GlobalRouter_C::isAchievable(Net_C* net, vector<Pos> s_source, Pos target){
    Direction dir = net->_chip->get_layer_dir(get<0>(target));
    set<Pos> neiberPos;
    Pos pos1= Pos(get<0>(target)-1,get<1>(target),get<2>(target));
    if(get<0>(target)-1>=0 && m_pChip->get_grid(pos1).get_remain_demand() > 0)
        neiberPos.insert(pos1);
    Pos pos2= Pos(get<0>(target)+1,get<1>(target),get<2>(target));
    if(get<0>(target)+1<m_pChip->get_layer_num() && m_pChip->get_grid(pos2).get_remain_demand() > 0)
        neiberPos.insert(pos2);
    
    if(dir == H){
        Pos pos3= Pos(get<0>(target),get<1>(target),get<2>(target)-1);
        if(get<2>(target)-1>=0 && m_pChip->get_grid(pos3).get_remain_demand() > 0)
            neiberPos.insert(pos3);
        Pos pos4= Pos(get<0>(target),get<1>(target),get<2>(target)+1);
        if(get<2>(target)+1<m_pChip->get_num_cols() && m_pChip->get_grid(pos4).get_remain_demand() > 0)
            neiberPos.insert(pos4);
    }
    else if(dir == V){
        Pos pos3= Pos(get<0>(target),get<1>(target)-1,get<2>(target));
        if(get<1>(target)-1>=0 && m_pChip->get_grid(pos3).get_remain_demand() > 0)
            neiberPos.insert(pos3);
        Pos pos4= Pos(get<0>(target),get<1>(target)+1,get<2>(target));
        if(get<1>(target)+1<m_pChip->get_num_rows() && m_pChip->get_grid(pos4).get_remain_demand() > 0)
            neiberPos.insert(pos4);
    }
    
    if(neiberPos.size() == 0){
        return false;
    }
    else return true;
}

/*
void GlobalRouter_C::global_route()
{
    
}*/
void GlobalRouter_C::clear(){
    sorted_net_list.clear();
}
int GlobalRouter_C::get_totoal_WL(){
    int total_WL=0;
    for(auto& net:sorted_net_list){
        total_WL += net->get_WL();
    }
    return total_WL;
}