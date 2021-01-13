#ifndef GR_H
#define GR_H
#include <tuple>
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <climits>
#include "module.h"

using namespace std;

typedef tuple<int, int, int> Pos; // layer, row, col

class Vertex{
    Pos _pos;
    bool _connected;
public:
    Vertex(const Pos &pos);
    Pos get_pos() const;
    void modify_layer(int layer){get<0>(_pos) = layer;}
};

class Edge{
    Vertex *_v1;
    Vertex *_v2;
public:
    Edge(Vertex *v1, Vertex *v2);
    ~Edge();
    Vertex* get_v1() const;
    Vertex* get_v2() const;
    int get_wl() const;
    
};
class steiner_tree_C{
public:     
    Chip_C* m_pChip;
    vector<Vertex*> _vertex_list;
    vector<Edge*> _edge_list;
    vector<Edge*> _mst;
   
    steiner_tree_C(Net_C*,Chip_C*);
    ~steiner_tree_C();
    void sortEdge(vector<Edge*>& vEdge);
    void builtMst();
    vector<Edge*> getMst(){return _mst;}
    void layerAssignment();

};

class Route {
public:
    Route(Pos source, Pos target, Net_C* net);
    Route(Pos source, Pos target, Pos center_gravity, Net_C* net);
    Route(Route* route, Pos curNode, Pos target, Pos center_gravity, Net_C* net);
    Route(set<Pos>, Pos curNode, Pos target, Pos center_gravity, Net_C* net);
    void print_info();
    string pos2str(Pos3d pos);
    int getHPWL(Pos pos1, Pos pos2);
    double getCost();
    
    Net_C* _net;
    vector<Pos> _grids;
    int _cost_grids=0;
    Pos _source, _curNode, _target, _center;
    int _HPWL=0;
    int _distance2target=0;
    int _dis_src2tar_2d = 0;
    int _dis_cur2tar_2d = 0;
    double _total_density=0;
    int _total_Centrifugal=0; // more far from center_of_gravity, more large number
    double _cost=0;
};

struct SortRouteByCost{
    bool operator() (const Route *r1, const Route *r2) const{
        return r1->_cost > r2->_cost;
    }
};

class GlobalRouter_C{
    //steiner_tree_C* m_pSteinerTree;
    vector<Net_C*> vNet_list;
    vector<set<Pos> > _net_to_used_pos; // connecting _routes
    vector<set<Pos> > _unLinked_v;
    vector<int> _net_routing_quota; // ori_route
    vector<int> _net_routed_edge;
    vector<int> _net_total_edge;
    vector<Net_C*> sorted_net_list;
    Chip_C* m_pChip;
    int _totoal_WL;
    int _part_WL;
    double _max_route_time = (double)INT_MAX;
public:
    GlobalRouter_C(Chip_C* p_pChip, vector<Net_C*>& p_vNet_list);
    void reRoute_all_net();
    //group net route
    void reRoute_group_net();
    bool reRoute_particular_net(vector<Net_C*> sorted_net_list, bool recover = 1);
    bool recoverRoute(Net_C* net);

    //=====================

    int reRoute(Net_C*,steiner_tree_C*);
    void reRoute_all_net_by_routeSize();
    int reRoutePrim(Net_C*);
    Route* Astar_search(Net_C*, Pos, Pos, Pos, bool); // source, target, center_of_gravity, bool(source_is_from_connected) 
    bool check_route_legal(Net_C*, Route*, Pos curNode, Direction dir, vector<vector<vector<bool> > > &); // bool*** is A* used Table
    Pos check_ExistBetterTarget(Net_C*,Pos cueNode, Pos target,int ext_z,int ext_v,int ext_h); // find in 2z*zv*2h cube
    
    // reRoute with origin route (safe mode)
    void reRoute_all_net_ori_route(string choice); // use_prim or not

    //void setRoute2Net(Net_C* net, set<Pos> grids);
    
    //void global_route();

    int getHPWL(Pos pos1, Pos pos2);
    void clear();
    string pos2str(Pos3d pos) {return "("+to_string(get<0>(pos))+","+to_string(get<1>(pos))+","+to_string(get<2>(pos))+")";}
    Pos getCenterOfGravity(set<Pos>);
    int get_totoal_WL();
    int get_part_WL(){return _part_WL;}
    bool isAchievable(Net_C* net, vector<Pos> s_source, Pos target);
    //test connectivity with DFS
    bool is_net_connected(Net_C* p_pNet);
    Pos3d getNearestPos(Pos3d pos,int net_id);
};

#endif