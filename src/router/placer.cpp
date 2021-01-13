#include "placer.h"
#include "module.h"
#include <cmath>
#include <climits>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;
string Placer_C::pos2d2str(Pos2d pos) {return "("+to_string(get<0>(pos))+","+to_string(get<1>(pos))+")";}
Placer_C::Placer_C(Chip_C* p_pChip, Design_C* p_pDesign, GlobalRouter_C* p_pGr, clock_t p_start):
m_pChip(p_pChip),m_pDesign(p_pDesign), m_pGr(p_pGr), m_tStart(p_start){}

void Placer_C::calPinPos(){
    Net_C* pNet;
    Pin_C* pPin;
    Pos3d sumPos3d = make_tuple(0,0,0);
    for(int netIter=0;netIter < m_pDesign->get_net_num();++netIter){
        pNet = m_pDesign->get_net_by_id(netIter);
        sumPos3d = Pos3d( 0,0,0);
        for(int pinIter=0;pinIter<pNet->get_pin_num(); ++pinIter){
            pPin = pNet->get_pin_by_id(pinIter);
            sumPos3d = sumPos3d + pPin->get_pos();
        }
        if(pNet->get_pin_num()==0) sumPos3d = Pos3d(0,0,0);
        else{
            get<0>(sumPos3d) /= pNet->get_pin_num(); 
            get<1>(sumPos3d) /= pNet->get_pin_num(); 
            get<2>(sumPos3d) /= pNet->get_pin_num(); 
        }
        
        m_NetCenter[pNet] = sumPos3d;
        //mPinBestPos[pPin] = Pos3d(0,0,0);
    }/*
    for(auto& pin:mPinBestPos){
        sumPos3d = Pos3d(0,0,0);
        for(int netIter=0;netIter<pin->get_net_num();++netIter){
            pNet = pin->get_net_by_id(netIter);
            sumPos3d = addPos3d(sumPos3d, m_NetCenter[pNet]);
        }
        get<0>(sumPos3d) /= pNet->get_pin_num(); 
        get<1>(sumPos3d) /= pNet->get_pin_num(); 
        get<2>(sumPos3d) /= pNet->get_pin_num(); 
        mPinBestPos[pin] = sumPos3d;
    }*/
}


bool cmpCell(tuple<CellInstance_C*, Pos2d> a,tuple<CellInstance_C*, Pos2d> b){
    return (abs(get<0>(get<1>(a)))+abs(get<1>(get<1>(a)))) > (abs(get<0>(get<1>(b)))+abs(get<1>(get<1>(b)))); 
}
bool cmp(CellInstance_C* a, CellInstance_C* b){
    return a->get_diff_from_best() > b->get_diff_from_best(); 
}
bool cmpN(Net_C* n1, Net_C* n2){
    return n1->get_HPWL() < n2->get_HPWL();
}
double avgDemand(vector<int> vDemand){
    double avg=0;
}
bool cmp_by_netNum(CellInstance_C* a, CellInstance_C* b){
    int a_net_num=0;
    int b_net_num=0;
    Pin_C* a_pin;
    Pin_C* b_pin;
    return a->get_relate_net_num() > b->get_relate_net_num();
}
bool cmp_by_dFromCenter(CellInstance_C* a, CellInstance_C* b){
    Pos2d center = Pos2d(a->get_chip()->get_num_rows()/2,a->get_chip()->get_num_cols()/2);
    return distance_2d(a->get_pos()-center) > distance_2d(b->get_pos()-center);
}
void Placer_C::set_neededRoutingNet(CellInstance_C* p_pCell){
    Net_C* pNet;
    for(int netIter=0;netIter<p_pCell->get_relate_net_num();++netIter){
        //if(find(m_vNeedRouteNet.begin(),m_vNeedRouteNet.end(),p_pCell->get_relate_net_by_id(netIter)) == m_vNeedRouteNet.end()){
            m_vNeedRouteNet.push_back(p_pCell->get_relate_net_by_id(netIter));
        //}
    }
}

void Placer_C::cellMovement_relativeNet(int scanRange){//after routing, place by cell's relative net's center
    cout << "\033[94m[CM]\033[0m - cell movement after routing with range " << scanRange << " ..." << endl;
    m_vCell.clear();
    CellInstance_C* pCell;
    int choose_from_moved_cell=0;
    if(m_pDesign->get_moved_cell_num() < m_pDesign->get_max_cell_move()){
        m_vMoveCell.clear();
        for(int cellIter=0;cellIter<m_pDesign->get_cell_num();++cellIter){  
            pCell = m_pDesign->get_cell_by_id(cellIter);
            pCell->reset_best_pos();
            //pCell->setCellBestPos_nestestPin();
            m_vCell.push_back(pCell);
        }
        //m_vCell.resize(m_pDesign->get_max_cell_move());
        //for(auto& cell: m_vCell){
        //    cell->setCellBestPos_force("first");
        //}
    }
    else{
        for(auto& id: m_pDesign->get_moved_cell_id_list()){
            m_vCell.push_back(m_pDesign->get_cell_by_id(id));
        }
        choose_from_moved_cell=1;
    }
    sort(m_vCell.begin(), m_vCell.end(),cmp_by_dFromCenter);//sort it by netCenter

    int oldWL = 0;
    int moveNum = 1;

    ////cout <<"oldWL= "<<oldWL<<endl;
    int cellIter = 0;
    for(auto& pCell: m_vCell){
        Pos2d ori_pos = pCell->get_pos();
        pCell->setCellBestPos_nestestPin_toCenter();
        //pCell->setCellBestPos_rand();
        Pos2d best_pos = pCell->get_best_pos();
        
        int minWL = INT_MAX;
        int newX,newY;
        m_vNeedRouteNet.clear();
        this->set_neededRoutingNet(pCell);
        sort(m_vNeedRouteNet.begin(),m_vNeedRouteNet.end(), cmpN);
        oldWL =0;
        for(auto& net: m_vNeedRouteNet){//
            oldWL += net->get_WL();
        }
        int vXoffset[25] = {0,0,0,1,-1,1,-1,-1,1,-2,-2,-2,-2,-2,2,2,2, 2, 2,-1,0,1,-1, 0, 1};
        int vYoffset[25] = {0,1,-1,0,0,1,-1,1,-1,-2,-1,0,1 , 2, 2,1,0,-1,-2,2, 2,2,-2,-2,-2};
        int x,y,a;
        Pos2d best_target_pos = best_pos;
        double min_density_cost = 999;
        for(a=0;a<scanRange;a++){
            x = vXoffset[a]; y = vYoffset[a];
            Pos2d target_pos = Pos2d(get<0>(best_pos)+y, get<1>(best_pos)+x);
            if(target_pos!=ori_pos && get<1>(target_pos) >=0 && get<1>(target_pos) < m_pChip->get_num_cols() && get<0>(target_pos) >= 0 && get<0>(target_pos) < m_pChip->get_num_rows()){
                double cur_density_cost = pCell->cost_dmdDensity_in_newPos(target_pos);
                if(cur_density_cost < min_density_cost){
                    min_density_cost = cur_density_cost;
                    best_target_pos = target_pos;
                }
            }   
        }
        if(min_density_cost!=999 && best_target_pos!=ori_pos && get<1>(best_target_pos) >=0 && get<1>(best_target_pos) < m_pChip->get_num_cols() && get<0>(best_target_pos) >= 0 && get<0>(best_target_pos) < m_pChip->get_num_rows()){
            if(m_pDesign->move_cell_to_pos(pCell, best_target_pos)){
            //reRoute the group of net and compute the WL, and then recover.
                if(m_pGr->reRoute_particular_net(m_vNeedRouteNet,false)){
                    if(m_pGr->get_part_WL() < minWL && m_pGr->get_part_WL() < oldWL){
                        minWL = m_pGr->get_part_WL(); 
                    }
                }
            }
        }
        /*for(a=0;a<scanRange;a++){
            x = vXoffset[a]; y = vYoffset[a];
            Pos2d target_pos = Pos2d(get<0>(best_pos)+y, get<1>(best_pos)+x);
            if(target_pos!=ori_pos && get<1>(target_pos) >=0 && get<1>(target_pos) < m_pChip->get_num_cols() && get<0>(target_pos) >= 0 && get<0>(target_pos) < m_pChip->get_num_rows()){
                if(m_pDesign->move_cell_to_pos(pCell, target_pos)){
                //reRoute the group of net and compute the WL, and then recover.
                    //cout << "\033[94m[CMXX]\033[0 - m Move to ("<< get<0>(target_pos) <<','<< get<1>(target_pos) <<')' <<endl;
                    if(m_pGr->reRoute_particular_net(m_vNeedRouteNet,false)){
                        if(m_pGr->get_part_WL() < minWL && m_pGr->get_part_WL() < oldWL){
                            minWL = m_pGr->get_part_WL(); 
                            best_target_pos = target_pos;
                            //a=10;
                        }
                    }
                    //cout << "\033[94m[routing success]\033[0 - move to ("<<y<<','<<x<<')' << " partWL="<<m_pGr->get_part_WL()<< endl;
                }
            }   
        }*/
        if(minWL >= oldWL){//can't move, so recover
            ////cout << "\033[94m[CM]\033[0m -Recover"<<"["<<m_pDesign->get_moved_cell_num() <<'/'<<m_pDesign->get_max_cell_move() <<"]"<<endl;
            assert(m_pDesign->move_cell_to_pos(pCell, ori_pos));
            for(auto& net: m_vNeedRouteNet){
                net->remove_all_route();
            }
            for(auto& net: m_vNeedRouteNet){
                assert(m_pGr->recoverRoute(net));
            }
            //m_pChip->print_all_supply_demand_table();
        }
        else{
            //m_pDesign->move_cell_to_pos(pCell, best_target_pos);
            //m_pGr->reRoute_particular_net(m_vNeedRouteNet,false);
            for(auto& net:m_vNeedRouteNet){net->set_ori_route();}
            ////cout <<  "\033[94m[CM]\033[0m -"<<pCell->get_name() << " successfully move to ("<<get<1>(pCell->get_pos())<<','<<get<1>(pCell->get_pos()) <<  ") ["<<m_pDesign->get_moved_cell_num() <<'/'<<m_pDesign->get_max_cell_move() <<"] \033[92mcompleted!\033[0m\n";
        }
        ////cout << "\033[94m [CM]cell \033[0m- "<<"["<<++cellIter <<'/'<<m_vCell.size() <<"] finish" <<endl;
        ////cout <<"choose:"<< choose_from_moved_cell<< ' ' << scanRange<<endl;
        if(m_pDesign->get_moved_cell_num() == m_pDesign->get_max_cell_move() && choose_from_moved_cell==0) break;

        // timing constraint
        if((double)((clock()-m_tStart)/CLOCKS_PER_SEC) > 3450){
            cout << "\033[94m[CM]\033[0m - cell movement has been stopped because the timing constraint.\n\n";
            return;
        }

    }
    cout << "\033[94m[CM]\033[0m - cell movement \033[92mcompleted!\033[0m\n";
}

void Placer_C::cellMovement_group(){
    vector<CellInstance_C*> vCell; 
    m_pDesign->get_cell_list(vCell);
    sort(vCell.begin(), vCell.end(),cmp);
    vector<CellInstance_C*> vCellGroup;
    int group_num_idx = 1;
    int group_num = 1;
    for(int i=0;i<vCell.size();i++){
        vCell[i]->get_relate_cell(vCellGroup);
        if(vCellGroup.size() > m_pDesign->get_max_cell_move()-m_pDesign->get_moved_cell_num()){
            break;
        }
        cout << vCellGroup.size() <<endl;
        
        if(group_cell_Movement(vCellGroup)){
            cout << "group["<< group_num_idx++ <<"] cell num: "<< vCellGroup.size() << endl;
        }
        vCellGroup.clear();
        
    }
}
bool Placer_C::group_cell_Movement(vector<CellInstance_C*> p_vCell){//after routing, place by cell's relative net's center
    cout << "\033[94m[CM]\033[0m - cell movement after routing..." << endl;
    m_vCell.clear();
    for(auto& pCell: p_vCell){  
        pCell->reset_best_pos();
        pCell->setCellBestPos_nestestPin();
        m_vCell.push_back(pCell);
    }
    //sort(m_vCell.begin(), m_vCell.end(),cmp);//sort it by netCenter
    int oldWL = 0;

    CellInstance_C* pCell;
    vector<Pos2d> ori_pos;
    vector<Pos2d> best_pos;
    m_vNeedRouteNet.clear();
    for(int i=0;i<m_vCell.size();i++){
        pCell = m_vCell[i];
        ori_pos.push_back(pCell->get_pos());
        best_pos.push_back(pCell->get_best_pos());
        this->set_neededRoutingNet(pCell);
    }
    sort(m_vNeedRouteNet.begin(),m_vNeedRouteNet.end(), cmpN);
    for(auto& net: m_vNeedRouteNet){//
        oldWL += net->get_WL();
    }
    for(auto& pCell: m_vCell){m_pDesign->remove_cell_from_pos(pCell);}
    for(auto& net: m_vNeedRouteNet){
        net->remove_all_route();
        net->init_route();
    }
    
    
    cout << "oldWL: " << oldWL << endl;
    int cellIter = 0;
    int minWL = INT_MAX;
    int vXoffset[9] = {0,0,0,1,1,1,-1,-1,-1};
    int vYoffset[9] = {0,1,-1,-1,0,1,-1,0,1};
    int x,y,a;
    for(int i = 0; i <m_vCell.size();++i){//move all cell
        pCell = m_vCell[i];
        cout << "\033[94m[CM]\033[0m - " << pCell->get_name() << " best_pos = " << pos2d2str(best_pos[i]+Pos2d(y,x)) << "[" << i+1 << "/" << m_vCell.size() <<"]\n";
        //cout << "\033[94m[CM]\033[0m -" << pCell->get_name() << " oldWL: "<< oldWL << endl;
        for(a=0;a<=8;a++){
            x = vXoffset[a]; y = vYoffset[a];
            if(best_pos[i]+Pos2d(y,x)!=ori_pos[i] && get<1>(best_pos[i])+x >=0 && get<1>(best_pos[i])+x < m_pChip->get_num_cols() && get<0>(best_pos[i])+y >= 0 && get<0>(best_pos[i])+y < m_pChip->get_num_rows()){
                if(m_pDesign->add_cell_to_pos(pCell, best_pos[i]+Pos2d(y,x))){
                    a=100;
                    cout << "\033[94m[CM]\033[0m - " << pCell->get_name() << " add to " << pos2d2str(best_pos[i]+Pos2d(y,x)) << " successfully"<<endl;
                }
            }
        }
        cout << "a: "<< a << endl;
        if(a != 101) {
            cout << "\033[94m[CM]\033[0m - Can't move to certain pos, so recover the group.\n"; 
            for(auto& pCell: m_vCell)
                m_pDesign->remove_cell_from_pos(pCell);
            for(auto& net: m_vNeedRouteNet)
                net->remove_all_route();
            for(int i=0;i<m_vCell.size();++i)
                assert(m_pDesign->add_cell_to_pos(m_vCell[i],ori_pos[i]));
            for(auto& net: m_vNeedRouteNet){
                assert(m_pGr->recoverRoute(net));
            }
            return false;
        }
    }
    if(m_pGr->reRoute_particular_net(m_vNeedRouteNet,false)){
        minWL = m_pGr->get_part_WL();
        cout << "new WL: "<< minWL<< endl;
    }
    else cout << "reRoute failed" << endl;

    if(minWL > oldWL){//can't move, so recover
        cout << "\033[94m[CM]\033[0m - Recover"<<"["<<m_pDesign->get_moved_cell_num() <<'/'<<m_pDesign->get_max_cell_move() <<"]"<<endl;
        cout << "\033[94m[CM]\033[0m - new WL = " << minWL << " old WL = " << oldWL << "\n";

        m_pDesign->move_cells_to_pos(m_vCell,ori_pos);
        for(auto& net: m_vNeedRouteNet){
            assert(m_pGr->recoverRoute(net));
        }
    }
    else{
        cout << "\033[94m[CM]\033[0m - group_move successfully! new WL = " << minWL << " old WL = " << oldWL << "\n";
        for(auto& net:m_vNeedRouteNet){net->set_ori_route();}
    }
    cout << "\033[94m[CM]\033[0m - group cell movement \033[92mcompleted!\033[0m\n";
}
void Placer_C::cellMovement_SA(){//after routing, place by SA
    cout << "\033[94m[CM]\033[0m - cell movement SA after routing..." << endl;

    CellInstance_C* pCell;
    for(int cellIter=0;cellIter<m_pDesign->get_cell_num();++cellIter){  
        pCell = m_pDesign->get_cell_by_id(cellIter);
        pCell->setCellBestPos();
        m_vCell.push_back(pCell);
    }
    sort(m_vCell.begin(), m_vCell.end(),cmp);//sort it by netCenter

    int oldWL = 0;
    int moveNum = 1;

    cout <<"oldWL= "<<oldWL<<endl;
    
    for(auto& pCell: m_vCell){
        Pos2d ori_pos = pCell->get_pos();
        int minWL = INT_MAX;
        Pos2d new_pos;
        this->set_neededRoutingNet(pCell);
        sort(m_vNeedRouteNet.begin(),m_vNeedRouteNet.end(), cmpN);
        oldWL =0;
        for(auto& net: m_vNeedRouteNet){//
            oldWL += net->get_WL();
        }
        cout << "\033[94m[CM]\033[0m -" << pCell->get_name() << " oldWL: "<< oldWL << endl;
        for(int x=0;x<=10;x++){
            Pos2d best_pos = Pos2d(rand()% m_pChip->get_num_rows(),rand()% m_pChip->get_num_cols());

            if(m_pDesign->move_cell_to_pos(pCell, Pos2d(get<0>(best_pos), get<1>(best_pos)))){
            //reRoute the group of net and compute the WL, and then recover.
                cout << "\033[94m[CMXX]\033[0 - m Move to ("<< get<0>(best_pos) <<','<<get<1>(best_pos)<<')' <<endl;

                if(m_pGr->reRoute_particular_net(m_vNeedRouteNet,false)){
                    if(m_pGr->get_part_WL() < minWL && m_pGr->get_part_WL() < oldWL){
                        minWL = m_pGr->get_part_WL();
                        new_pos = best_pos;
                        //x=1000;
                    }
                }
            }
        }
        if(minWL > oldWL){//can't move, so recover
            cout << "Recover"<<endl;
            for(auto& net: m_vNeedRouteNet){
                net->remove_all_route();
            }
            m_pDesign->move_cell_to_pos(pCell, Pos2d(get<0>(ori_pos), get<1>(ori_pos)));
            for(auto& net: m_vNeedRouteNet){
                net->remove_all_route();
            }
            for(auto& net: m_vNeedRouteNet){
                assert(m_pGr->recoverRoute(net));
            }
        }
        else{
            m_pDesign->move_cell_to_pos(pCell, Pos2d(get<0>(new_pos), get<1>(new_pos)));
            m_pGr->reRoute_particular_net(m_vNeedRouteNet,false);
            for(auto& net:m_vNeedRouteNet){net->set_ori_route();}
            cout <<  "\033[94m[CM]\033[0m -"<<pCell->get_name() << " successfully move to ("<<get<1>(pCell->get_pos())<<','<<get<1>(pCell->get_pos()) <<  ") \033[92mcompleted!\033[0m\n";
            if(moveNum == m_pDesign->get_max_cell_move()) break;
            moveNum++;
        }
    }
    cout << "\033[94m[CM]\033[0m - cell movement \033[92mcompleted!\033[0m\n";


    

}
void Placer_C::cellMovement(){//after routing
    cout << "\033[94m[CM]\033[0m - cell movement after routing..." << endl;

    CellInstance_C* pCell;
    for(int cellIter=0;cellIter<m_pDesign->get_cell_num();++cellIter){  
        pCell = m_pDesign->get_cell_by_id(cellIter);
        m_vCell.push_back(pCell);
    }
    
    sort(m_vCell.begin(), m_vCell.end(),cmp_by_netNum);//sort it by netNum
    pCell = m_vCell.back();
    int oldWL = 0;
    int moveNum = 1;

    cout <<"oldWL= "<<oldWL<<endl;
    
    for(auto& pCell: m_vCell){
        Pos2d ori_pos = pCell->get_pos();

        int minWL = INT_MAX;
        int newX,newY;
        this->set_neededRoutingNet(pCell);
        sort(m_vNeedRouteNet.begin(),m_vNeedRouteNet.end(), cmpN);
        oldWL =0;
        for(auto& net: m_vNeedRouteNet){//
            oldWL += net->get_WL();
        }
        cout << "\033[94m[CM]\033[0m -" << pCell->get_name() << " oldWL: "<< oldWL << endl;
        for(int x=-5;x<=5;x++){
            for(int y=-5;y<=5;y++){   
                if(x!=0 && y!=0 && get<1>(ori_pos)+x >=0 && get<1>(ori_pos)+x < m_pChip->get_num_cols() && get<0>(ori_pos)+y >= 0 && get<0>(ori_pos)+y < m_pChip->get_num_rows()){
                    if(m_pDesign->move_cell_to_pos(pCell, Pos2d(get<0>(ori_pos)+y, get<1>(ori_pos)+x))){
                    //reRoute the group of net and compute the WL, and then recover.
                        cout << "\033[94m[CMXX]\033[0 - m Move to ("<< get<0>(ori_pos)+y <<','<<get<1>(ori_pos)+x<<')' <<endl;
 
                        if(m_pGr->reRoute_particular_net(m_vNeedRouteNet,false)){
                            if(m_pGr->get_part_WL() < minWL){
                                minWL = m_pGr->get_part_WL();
                                newX = x; newY = y;
                            }
                        }
                        cout << "\033[94m[routing success]\033[0 - move to ("<<y<<','<<x<<')' << " partWL="<<m_pGr->get_part_WL()<< endl;

                    }
                }
            }
        }
        if(minWL > oldWL){//can't move, so recover
            for(auto& net: m_vNeedRouteNet){
                net->remove_all_route();
            }
            m_pDesign->move_cell_to_pos(pCell, Pos2d(get<0>(ori_pos), get<1>(ori_pos)));
            for(auto& net: m_vNeedRouteNet){
                net->remove_all_route();
            }

            for(auto& net: m_vNeedRouteNet){
                assert(m_pGr->recoverRoute(net));
            }
        }
        else{
            m_pDesign->move_cell_to_pos(pCell, Pos2d(get<0>(ori_pos)+newY, get<1>(ori_pos)+newX));
            m_pGr->reRoute_particular_net(m_vNeedRouteNet);
            for(auto& net:m_vNeedRouteNet){net->set_ori_route();}
            cout <<  "\033[94m[CM]\033[0m -"<<pCell->get_name() << " successfully move to ("<<get<1>(pCell->get_pos())<<','<<get<1>(pCell->get_pos()) <<  ") \033[92mcompleted!\033[0m\n";
            if(moveNum == m_pDesign->get_max_cell_move()) break;
            moveNum++;
        }
    }
    cout << "\033[94m[CM]\033[0m - cell movement \033[92mcompleted!\033[0m\n";

}
/*
void Placer_C::cellMovement(){
    cout << "\033[94m[CM]\033[0m - initial cell movement ..." << endl;

    for(int i=0;i<m_pDesign->get_net_num();++i){
        m_pDesign->get_net_by_id(i)->remove_all_route();
        m_pDesign->get_net_by_id(i)->init_route();
        
    }
    calPinPos();
    calCellBestPos();
    //tuple<CellInstance_C*, Pos2d> tCell;//second number is cell offset from best point
    vector<tuple<CellInstance_C*, Pos2d>> vCellOffset;
    tuple<CellInstance_C*, Pos2d> tCell;
    Pos2d bestPos;
    CellInstance_C* pCell;
    for(int cellIter=0;cellIter<m_pDesign->get_cell_num();++cellIter){  
        pCell = m_pDesign->get_cell_by_id(cellIter);
        bestPos = mCellBestPos[pCell];
        vCellOffset.push_back(tuple<CellInstance_C*, Pos2d> (pCell, Subtraction( bestPos, pCell->get_pos()) ));
    }
    sort(vCellOffset.begin(), vCellOffset.end(), cmpCell);
    vector<int> newDemand;
    vector<int> oldDemand;
    double avgNewDemand;
    double avgOldDemand;
    int moveNum=0;
    for(int cellIter=0; moveNum<m_pDesign->get_max_cell_move() && cellIter<m_pDesign->get_cell_num();++cellIter){
        
        tCell = vCellOffset[cellIter];
        cout << 1<< endl;
        Pos2d center = addPos2d(get<1>(tCell),get<0>(tCell)->get_pos());
        cout<< "center x: " << get<1>(center) << " y: " << get<0>(center)<< endl; 

        cout << 2<< endl;
        oldDemand = get<0>(tCell)->cost_demand_in_oriPos();
        cout << 3<< endl;
        avgOldDemand = accumulate( oldDemand.begin(), oldDemand.end(), 0.0)/ oldDemand.size();
        cout << 4<< endl;
        int minNewDemand=INT_MAX;
        cout << 5<< endl;
        Pos2d minNewPoint = center;
        cout << 6<< endl;
        Pos2d point;
        cout << 7<< endl;

        //for(int x=-1;x<=1;x++){
        //    for(int y=-1;y<=1;y++){
        //        point = addPos2d(center,Pos2d(y,x));
        //        if(get<1>(point) >=0 && get<1>(point) <m_pChip->get_num_cols() && get<0>(point) >= 0 && get<0>(point) <m_pChip->get_num_rows()){
        //            newDemand = get<0>(tCell)->new_demand_in_newPos(point);
        //            if(newDemand.size()==0){
        //                cout << "grid will overflow, can't move to this pos" << endl;
        //                continue;
        //                }
        //            avgNewDemand = accumulate( newDemand.begin(), newDemand.end(), 0.0)/ newDemand.size();
        //            if(avgNewDemand < minNewDemand){
        //                minNewDemand = avgNewDemand;
        //                minNewPoint = addPos2d(center,Pos2d(y,x));
        //            }
        //        }
        //    }
        //}
        //if(minNewDemand ==INT_MAX){
        //    continue;
        //}
        cout<< "minNewPoint x: " << get<1>(minNewPoint) << " minNewPoint y: " << get<0>(minNewPoint)<< endl; 
        cout << get<0>(tCell)->get_name() << " (" <<get<0>(get<0>(tCell)->get_pos())<<","<<get<1>(get<0>(tCell)->get_pos())<<")" << endl;
      
        newDemand = get<0>(tCell)->new_demand_in_newPos(minNewPoint);
        cout << "newDemand size: " << newDemand.size() << endl;
        cout << 8<< endl;

        if(newDemand.size()==0){continue;}
        avgNewDemand = accumulate( newDemand.begin(), newDemand.end(), 0.0)/ newDemand.size();
        cout << 9<< endl;
        
        if(/*avgNewDemand < 3*avgOldDemand && m_pDesign->move_cell_to_pos(get<0>(tCell), minNewPoint)){
            cout <<"\033[94m[CM]\033[0m - " << get<0>(tCell)->get_name() << " successful movement" <<endl;
            moveNum++;
        }
        else{
            cout <<"\033[94m[CM]\033[0m - " << "can't move cell \'"<< get<0>(tCell)->get_name()<< endl;
            //cellIter--;
        }
        cout << 10<< endl;

    }
    cout << "\033[94m[CM]\033[0m - initial cell movement \033[92mcompleted!\033[0m\n";
}
*/

void Placer_C::clear(){
   
}
