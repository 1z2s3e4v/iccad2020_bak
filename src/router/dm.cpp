
#include "dm.h"
#include <assert.h>
Direction dmMgr_C::getDir(Pos3d pos_s, Pos3d pos_e){
    if(get<0>(pos_s) != get<0>(pos_e))
        return Z;
    else {
        return get<0>(pos_s) % 2;
    }
}
int dmMgr_C::getLength(Pos3d pos_s, Pos3d pos_e){
    Direction dir = getDir(pos_s, pos_e);
    if(dir == Z)
        return abs(get<0>(pos_s) - get<0>(pos_e)) + 1;
    else if(dir == V)
        return abs(get<1>(pos_s) - get<1>(pos_e)) + 1;
    else if(dir == H)
        return abs(get<2>(pos_s) - get<2>(pos_e)) + 1;
    else return -1;
}
void dmMgr_C::set_ori_route(){
    for(int netIter=0;netIter< m_pDesign->get_net_num();++netIter){
        m_pDesign->get_net_by_id(netIter)->set_ori_route();
    }
}
dmMgr_C::dmMgr_C(parser_C* p_pParser, clock_t start){
    cout << "\033[94m[Data Model]\033[0m - Creating Data Module ...\n";
    m_tStart = start;
    // Create main Chip
    m_pChip = new Chip_C(p_pParser->gGridBoundaryIdx.rowBeginIdx, p_pParser->gGridBoundaryIdx.colBeginIdx, p_pParser->gGridBoundaryIdx.rowEndIdx, p_pParser->gGridBoundaryIdx.colEndIdx);
    // Add layers to chip
    for(auto &lay : p_pParser->v_Layer){
        Direction d;
        switch (lay.RoutingDirection){
            case 'H': d = H; break;
            case 'V': d = V; break;
            default: cout << "\033[94m[Data Model]\033[0m - unknown RoutingDirection \'" << lay.RoutingDirection << "\' for " << lay.layerName << "\n"; exit(1);
        }
        // p_pParser->v_Layer has been sorted in parser
        m_pChip->add_layer(lay.layerName, lay.Idx, lay.defaultSupplyOfOneGGrid);
    }
    // Add NonDefaultSupply to GGrid
    for(auto &p_ndsg : p_pParser->v_nonDefaultSupplyGGrid){
        Pos3d pos(m_pChip->get_layer_id_by_idx(p_ndsg.LayIdx),p_ndsg.rowIdx-m_pChip->get_row_begin_idx(),p_ndsg.colIdx-m_pChip->get_col_begin_idx());
        m_pChip->set_nonDefaultSupply(pos,p_ndsg.incrOrDecrValue);
    }
    // Create master cell library
    m_pCellLibrary = new CellLibrary_C();
    for(int i=0;i<p_pParser->masterCellCount;++i){
        auto &p_ms = p_pParser->v_masterCell[i];
        MasterCell_C ms = MasterCell_C(p_ms.masterCellName, i);
        for(int j=0;j<p_ms.pinCount;++j){ // set pin
            ms.add_pin(p_ms.v_pin[j].pinName, j, m_pChip->get_layer_id_by_name(p_ms.v_pin[j].pinLayer));
        }
        for(int j=0;j<p_ms.blockageCount;++j){ // set blockage
            ms.add_blkg(p_ms.v_blkg[j].blockageName, j, m_pChip->get_layer_id_by_name(p_ms.v_blkg[j].blockageLayer), p_ms.v_blkg[j].demand);
        }
        m_pCellLibrary->add_master_cell(ms);
    }
    // Create Extra Demand lookup table
    m_pExtraDemand = new ExtraDemand_C();
    for(auto &condition : p_pParser->v_neighborCellExtraDemand){
        if(condition.type == "sameGGrid"){
            bool ms1_success=true, ms2_success=true;
            int ms1_id = m_pCellLibrary->get_master_cell_by_name(condition.masterCellName1,ms1_success).get_id();
            int ms2_id = m_pCellLibrary->get_master_cell_by_name(condition.masterCellName2,ms2_success).get_id();
            if(!ms1_success){
                cout << "\033[94m[Data Model]\033[0m - unknown master cell name \'" << condition.masterCellName1 << "\'\n";
                exit(1);
            }
            if(!ms2_success){
                cout << "\033[94m[Data Model]\033[0m - unknown master cell name \'" << condition.masterCellName2 << "\'\n";
                exit(1);
            }
            //cout << "add_same_demand ms1_id=" << ms1_id << " ms2_id=" << ms2_id << " condition.layerName=" << condition.layerName << "\n";
            m_pExtraDemand->add_same_demand(ms1_id, ms2_id, m_pChip->get_layer_id_by_name(condition.layerName), condition.demand);
        }
        else if(condition.type == "adjHGGrid"){
            bool ms1_success=true, ms2_success=true;
            int ms1_id = m_pCellLibrary->get_master_cell_by_name(condition.masterCellName1,ms1_success).get_id();
            int ms2_id = m_pCellLibrary->get_master_cell_by_name(condition.masterCellName2,ms2_success).get_id();
            if(!ms1_success){
                cout << "\033[94m[Data Model]\033[0m - unknown master cell name \'" << condition.masterCellName1 << "\'\n";
                exit(1);
            }
            if(!ms2_success){
                cout << "\033[94m[Data Model]\033[0m - unknown master cell name \'" << condition.masterCellName2 << "\'\n";
                exit(1);
            }
            //cout << "add_adj_demand ms1_id=" << ms1_id << " ms2_id=" << ms2_id << " condition.layerName=" << condition.layerName << "\n";
            m_pExtraDemand->add_adj_demand(ms1_id, ms2_id, m_pChip->get_layer_id_by_name(condition.layerName), condition.demand);
        }
        else{
            cout << "\033[94m[Data Model]\033[0m - unknown NeighborCellExtraDemand type \'" << condition.type << "\'\n";
            exit(1);
        }
    }

    // Create netlist
    m_pDesign = new Design_C(p_pParser->maxMoveCount, p_pParser->cellInstCount, p_pParser->netCount);
    for(int i=0;i<p_pParser->cellInstCount;++i){
        auto &p_cell = p_pParser->v_cellInst[i];
        bool ms_success=true;
        MasterCell_C ms = m_pCellLibrary->get_master_cell_by_name(p_cell.masterCellName,ms_success);
        if(!ms_success){
            cout << "\033[94m[Data Model]\033[0m - unknown master cell name \'" << p_cell.masterCellName << "\' in cell \'" << p_cell.instName << "'\\n";
            exit(1);
        }
        CellInstance_C* new_cell = new CellInstance_C(p_cell.instName, i, ms, p_cell.gGridRowIdx-m_pChip->get_row_begin_idx(), p_cell.gGridColIdx-m_pChip->get_col_begin_idx(), p_cell.fixed, m_pChip, m_pExtraDemand);
        m_pDesign->add_cell(new_cell);
    }
    for(int i=0;i<p_pParser->netCount;++i){
        auto &p_net = p_pParser->v_net[i];
        int min_layer = m_pChip->get_layer_num(); // default routing constrain = max
        if(p_net.minRoutingLayConstraint != "NoCstr")
            min_layer = m_pChip->get_layer_id_by_name(p_net.minRoutingLayConstraint);
        Net_C* new_net = new Net_C(p_net.netName, i, m_pChip->get_layer_id_by_name(p_net.minRoutingLayConstraint),m_pChip);
        for(int j=0;j<p_net.numPins;++j){
            auto &p_net_pin = p_net.v_pin[j];
            new_net->add_pin(m_pDesign->get_cell_by_name(p_net_pin.instName)->get_pin_by_name(p_net_pin.masterPinName));
        }
        m_pDesign->add_net(new_net);
    }

    // Add route to Nets
    for(auto &route : p_pParser->v_routeSegment){
        Net_C* net = m_pDesign->get_net_by_name(route.netName);
        Pos3d pos_s(m_pChip->get_layer_id_by_idx(route.sLayIdx),route.sRowIdx-m_pChip->get_row_begin_idx(),route.sColIdx-m_pChip->get_col_begin_idx());
        Pos3d pos_e(m_pChip->get_layer_id_by_idx(route.eLayIdx),route.eRowIdx-m_pChip->get_row_begin_idx(),route.eColIdx-m_pChip->get_col_begin_idx());
        Direction dir = getDir(pos_s, pos_e);
        int length = getLength(pos_s, pos_e);
        if(dir == Z){
            if(get<0>(pos_s) > get<0>(pos_e)) swap(pos_s,pos_e);
            for(int i=0;i<length;++i){
                net->add_route(Pos3d(get<0>(pos_s)+i,get<1>(pos_s),get<2>(pos_s)));
            }
        }
        else if(dir == V){
            if(get<1>(pos_s) > get<1>(pos_e)) swap(pos_s,pos_e);
            for(int i=0;i<length;++i){
                net->add_route(Pos3d(get<0>(pos_s),get<1>(pos_s)+i,get<2>(pos_s)));
            }
        }
        else if( dir == H){
            if(get<2>(pos_s) > get<2>(pos_e)) swap(pos_s,pos_e);
            for(int i=0;i<length;++i){
                net->add_route(Pos3d(get<0>(pos_s),get<1>(pos_s),get<2>(pos_s)+i));
            }
        }
    }
    for(int i=0;i<m_pDesign->get_net_num();++i){
        assert(m_pDesign->get_net_by_id(i)->init_route());
        m_pDesign->get_net_by_id(i)->set_all_pin_connect();
    }
    
    set_ori_route();
    m_origin_total_WL = get_total_WL();
    cout << "\033[94m[Data Model]\033[0m - Creating Data Module successfully!\n";
    system("[ -d dump ] && rm -rf dump; mkdir -p dump");
    cout << "\033[94m[DM]\033[0m - origin WL = " << m_origin_total_WL << "\n";
}

void dmMgr_C::dump_info(){
    cout << "\033[94m[DM]\033[0m - dumping info ...\n";
    ofstream file("dump/info.txt");
    file << "MaxCellMove = " << m_pDesign->get_max_cell_move() << "\n";
    file << "\nNumLayer = " << m_pChip->get_layer_num() << "\n";
    for(int i=0;i<m_pChip->get_layer_num();++i){
        m_pChip->dump_supply_demand_table(file,i);
    }
    file << "\nNeighborCellExtraDemand: \n";
    m_pExtraDemand->dump_all_cond(file);

    file << "NumCellInst = " << m_pDesign->get_cell_num() << "\n";
    for(int i=0;i<m_pDesign->get_cell_num();++i){
        m_pDesign->dump_cell_info(file,i);
    }
    file << "NumNets = " << m_pDesign->get_net_num() << "\n";
    for(int i=0;i<m_pDesign->get_net_num();++i){
        m_pDesign->dump_net_info(file,i);
    }
    file << "\nRouting result:\n";
    file << "In each files.\n";
    for(int i=0;i<m_pDesign->get_net_num();++i){
        string routeNet_fileName="dump/"+m_pDesign->get_net_by_id(i)->get_name()+"_route.txt";
        ofstream file_route(routeNet_fileName);
        m_pChip->dump_Net_route(file_route,m_pDesign->get_net_by_id(i));
    }
    string cmd_sum_route = "echo \'total WL=" + to_string(m_pDesign->get_total_WL())+"\n---------------------------\n\' > dump/summary.route";
    system(cmd_sum_route.c_str());
    system("cat dump/*_route.txt >> dump/summary.route");
    system("[ ! -e dump/summary.route.ori ] && cp dump/summary.route dump/summary.route.ori");
    cout << "\033[94m[DM]\033[0m - dump info \033[92mcompleted! \033[0mThe files are generated in dump/\n";
}

void dmMgr_C::print_info(){
    cout << "MaxCellMove = " << m_pDesign->get_max_cell_move() << "\n";
    cout << "\nNumLayer = " << m_pChip->get_layer_num() << "\n";
    m_pChip->print_all_supply_demand_table();
    cout << "\nNeighborCellExtraDemand: \n";
    m_pExtraDemand->print_all_cond();

    cout << "NumCellInst = " << m_pDesign->get_cell_num() << "\n";
    for(int i=0;i<m_pDesign->get_cell_num();++i){
        m_pDesign->print_cell_info(i);
    }
    cout << "NumNets = " << m_pDesign->get_net_num() << "\n";
    for(int i=0;i<m_pDesign->get_net_num();++i){
        m_pDesign->print_net_info(i);
    }
    cout << "\n\033[94m[DM]\033[0m - Example Routing result:\n";
    print_net_route(0); // id
}

void dmMgr_C::print_net_route(int net_id){
    m_pChip->print_Net_route(m_pDesign->get_net_by_id(net_id));
}

void dmMgr_C::print_demand_table(){
    cout << "\nNumLayer = " << m_pChip->get_layer_num() << "\n";
    for(int i=0;i<m_pChip->get_layer_num();++i){
        m_pChip->print_supply_demand_table(i);
    }
}

void dmMgr_C::dump_dmd(string fileName){
    fileName += ".myDmd";
    ofstream file(fileName);
    file << "row col lay supply demand" << "\n";
    for(int l=0;l<m_pChip->get_layer_num();++l){
        for(int r=0;r<m_pChip->get_num_rows();++r){
            for(int c=0;c<m_pChip->get_num_cols();++c){
                file << r+m_pChip->get_row_begin_idx() << " " << c+m_pChip->get_col_begin_idx() << " " << l+1 << " " << m_pChip->get_grid(Pos3d(l,r,c)).get_supply() << " " << m_pChip->get_grid(Pos3d(l,r,c)).get_demand() << "\n";
            }
        }
    }
    file.close();
}
void dmMgr_C::dump_dmd_svg(string fileName){
    fileName += "_myDmd.html";
    ofstream file(fileName);
    file << "<svg height=\"" <<m_pChip->get_num_rows()*(m_pChip->get_layer_num()+3)*10  << "\" width=\"" << (m_pChip->get_num_cols()+10)*10  << "\">\n\n";
    for(int l=0;l<m_pChip->get_layer_num();++l){
        for(int r=0;r<m_pChip->get_num_rows();++r){
            for(int c=0;c<m_pChip->get_num_cols();++c){                
            file << "<rect " << "\"" <<" x=\"" << c*10<< "\" y=\""<<  (r + l*(m_pChip->get_num_rows()+3))*10;
            file  << "\"  width=\""<< 10  << "\" height=\""<< 10 << "\"";
            switch(m_pChip->get_grid(Pos3d(l,r,c)).get_remain_demand()){
                case 0: file << " stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#2F0000" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 1: file << " stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#4D0000" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 2: file << " stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#600000" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 3: file << " stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#750000" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 4: file << " stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#930000" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 5: file << " stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#AE0000" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 6: file << " stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#CE0000" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 7: file << " stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#EA0000" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 8: file << " stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#FF0000" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 9: file << " stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#FF2D2D" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 10: file <<" stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#FF5151" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 11: file <<" stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#FF7575" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 12: file <<" stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#FF9797" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 13: file <<" stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#FFB5B5" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                case 14: file <<" stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#FFD2D2" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
                default: file <<" stroke=\"" << "#000000" << "\"  stroke-width=\"1\" fill=\""<< "#FFECEC" << "\" fill-opacity=\"1\" stroke-opacity=\"0.3\" />\n";
                        break;
            }
            }
        }
    }
    file.close();
}

void dmMgr_C::output_result(char* fileName){
    ofstream file(fileName);
    set<int> moved_cell_id = m_pDesign->get_moved_cell_id_list();
    file << "NumMovedCellInst " << moved_cell_id.size() << "\n";
    for(int id : moved_cell_id){
        auto cell = m_pDesign->get_cell_by_id(id);
        file << "CellInst " << cell->get_name() << " " << get<0>(cell->get_pos())+m_pChip->get_row_begin_idx() << " " << get<1>(cell->get_pos())+m_pChip->get_col_begin_idx() << "\n";
    }
    int sumRoutes = 0;
    vector<Net_C*> p_pNet;
    m_pDesign->get_net_list(p_pNet);
    for(auto net : p_pNet){
        net->trans_routes2wires();
        sumRoutes += net->get_wires_num();
    }
    file << "NumRoutes " << sumRoutes << "\n";
    for(auto net : p_pNet){
        vector<pair<Pos3d,Pos3d> > wires = net->get_all_wire();
        for(auto wire : wires){
            file << get<1>(wire.first)+m_pChip->get_row_begin_idx() << " " << get<2>(wire.first)+m_pChip->get_col_begin_idx() << " " << get<0>(wire.first)+1 << " " << get<1>(wire.second)+m_pChip->get_row_begin_idx() << " " << get<2>(wire.second)+m_pChip->get_col_begin_idx() << " " << get<0>(wire.second)+1 << " " << net->get_name() << "\n";
        }
    }
}
void draw_grid(ofstream& file, double x1, double y1, double z1, double x2, double y2, double z2, double originalY){
    double offset =100;
    file << "<path d=\"M " << (x1+(y1/sqrt(2)))*offset+300 << " " << -(z1+(y1/sqrt(2)))*offset+originalY;
    file << " L " << (x2+(y2/sqrt(2)))*offset+300 << " " << -(z2+(y2/sqrt(2)))*offset+originalY << "\" ";
    file << "stroke=\"#CCCCCC\" stroke-width=\"3\" stroke-opacity=\"0.2\" />\n";
}

void dmMgr_C::write_net(ofstream& file, Net_C* net){
    int offset = 100;
    int netOffSetX=0;
    int netOffSetY=0;
    int originalY=(m_pChip->get_num_rows()/sqrt(2) +m_pChip->get_layer_num()+1)*offset; 
    int originalX=(m_pChip->get_num_rows()/sqrt(2) +m_pChip->get_num_cols()+1)*offset; 
    vector<pair<Pos3d,Pos3d> > wires = net->get_all_wire();
    // random generate hex color_code (one net, one color)
    char hex_char[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    string color_code = "#";
    for(int i=0;i<6;i++)
        color_code += hex_char[rand()%16];

    netOffSetX = rand() % 40 -20; 
    netOffSetY = rand() % 40 -20; 
    for(auto wire : wires){
        int z1 = get<0>(wire.first);
        int y1 = get<1>(wire.first);
        int x1 = get<2>(wire.first);
        int z2 = get<0>(wire.second);
        int y2 = get<1>(wire.second);
        int x2 = get<2>(wire.second);
        file << "<path "<< "net=\"" << net->get_name() << "\"" <<" d=\"M " << (x1+(y1/sqrt(2)))*offset+300+netOffSetX << " " << -(z1+(y1/sqrt(2)))*offset+netOffSetY+originalY;
        file << " L " << (x2+(y2/sqrt(2)))*offset+300+netOffSetX << " " << -(z2+(y2/sqrt(2)))*offset+originalY+netOffSetY << "\" ";
        file << "stroke=\"" << color_code << "\" stroke-width=\"3\" stroke-opacity=\"0.7\" />\n";
    }
    for(int i=0;i<net->get_pin_num();++i){
        int z = get<0>(net->get_pin_by_id(i)->get_pos());
        int y = get<1>(net->get_pin_by_id(i)->get_pos());
        int x = get<2>(net->get_pin_by_id(i)->get_pos());
        int rx = (x+(y/sqrt(2)))*offset+300+netOffSetX;
        int ry = -(z+(y/sqrt(2)))*offset+netOffSetY+originalY;
        file << "<circle stroke=\""<< color_code <<"\" stroke-width=\"2\" stroke-opacity=\"0.7\" cx=\"" << rx << "\" cy=\""<<ry<< "\" r=\"2\"/>" << endl;
    }
}
void dmMgr_C::output_cellpos_svg(string fileName){
    vector<Net_C*> vNet;
    vector<CellInstance_C*> vCell;
    m_pDesign->get_net_list(vNet);
    m_pDesign->get_cell_list(vCell);
    ofstream file(fileName);
    int cellOffSetX = 0;
    int cellOffSetY = 0;
    int offset = 20;
    int originalY=(m_pChip->get_num_rows()+1)*offset; 
    int originalX=(m_pChip->get_num_cols()+1)*offset; 
    file << "<svg height=\"" <<originalY  << "\" width=\"" << originalX  << "\">\n\n";
    char hex_char[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E'};
    for(auto& cell: vCell){
        string color_code = "#";
        if(cell->is_moved()){
            color_code = "#FF0000";
        }
        else
            for(int i=0;i<6;i++){
                color_code += hex_char[rand()%15]; 
            } 
        cellOffSetX = rand() % 10 ; 
        cellOffSetY = rand() % 10 ; 
        if(cell->is_moved()){
            file << "<path "<< "cell=\"" << cell->get_name() << "\"" <<" d=\"M " << (get<1>(cell->get_ori_pos()))*offset+cellOffSetX << " " << (get<0>(cell->get_ori_pos()))*offset+cellOffSetY;
            file << " L " << (cell->get_col())*offset+cellOffSetX << " " << (cell->get_row())*offset+cellOffSetY << "\" ";
            file << "stroke=\"" << color_code << "\" stroke-width=\"3\" stroke-opacity=\"0.7\" />\n";
            file << "<circle stroke=\""<< color_code <<"\" stroke-width=\"5\" stroke-opacity=\"0.7\" cx=\"" << (cell->get_col())*offset+cellOffSetX << "\" cy=\""<<(cell->get_row())*offset+cellOffSetY  << "\" r=\"2\"/>" << endl;
        }
        else
            file << "<circle stroke=\""<< color_code <<"\" stroke-width=\"2\" stroke-opacity=\"0.7\" cx=\"" << (cell->get_col())*offset+cellOffSetX << "\" cy=\""<<(cell->get_row())*offset+cellOffSetY  << "\" r=\"2\"/>" << endl;
    }
    int max_row=0;
    int min_row=INT_MAX;
    int max_col=0;
    int min_col=INT_MAX;
    int max_layer=0;
    int min_layer=INT_MAX;
    Pin_C* pin;
    Pos3d p;
    //draw the net region
    int i=0;
    string color_code = "#";
        for(int i=0;i<6;i++){
            color_code += hex_char[rand()%15]; 
    } 
    for(auto& net:vNet){
        if(i++ == 10) break;
        cellOffSetY = rand() % 10 ; 
        cellOffSetY = rand() % 10 ; 
        
        max_row=0;
        min_row=INT_MAX;
        max_col=0; 
        min_col=INT_MAX;
        max_layer=0;
        min_layer=INT_MAX;
        for(int pinIter=0;pinIter<net->get_pin_num();++pinIter){
            pin = net->get_pin_by_id(pinIter);
            p = pin->get_pos();
            max_row = max(max_row, get<1>(p));
            min_row = min(min_row, get<1>(p));
            max_col = max(max_col, get<2>(p));
            min_col = min(min_col, get<2>(p));
            max_layer = max(max_layer, get<0>(p));
            min_layer = min(min_layer, get<0>(p));
        }
        file << "<rect "<< "net=\"" << net->get_name() << "\"" <<" x=\"" << min_col*offset;
        file << "\" y=\""<<  min_row*offset << "\"  width=\""<<(max_col-min_col)*offset  << "\" height=\""<< (max_row-min_row)*offset << "\"";
        file << " stroke=\"" << color_code << "\"  stroke-width=\"1\" fill=\""<< color_code << "\" fill-opacity=\"0.1\" stroke-opacity=\"0.3\" />\n";
    }

    file.close();
}
void dmMgr_C::output_svg(){
    vector<Net_C*> p_pNet;
    vector<CellInstance_C*> vCell;
    m_pDesign->get_net_list(p_pNet);
    m_pDesign->get_cell_list(vCell);
    system("[ -d dumpSVG ] && rm -rf dumpSVG; mkdir -p dumpSVG");
    system("[ -d dumpRN ] && rm -rf dumpRN; mkdir -p dumpRN");

    ofstream file("svg.html");
    int offset = 100;
    int netOffSetX=0;
    int netOffSetY=0;
    int originalY=(m_pChip->get_num_rows()/sqrt(2) +m_pChip->get_layer_num()+1)*offset; 
    int originalX=(m_pChip->get_num_rows()/sqrt(2) +m_pChip->get_num_cols()+1)*offset; 
    file << "<svg height=\"" <<originalY + 1000 << "\" width=\"" << originalX + 1000 << "\">\n\n";
    
    /*
    for(auto net : p_pNet){
        string fileName = "P_"+to_string(net->get_pin_num()) + net->get_name()  ;  
        ofstream fileNet("./dumpSVG/"+fileName+".html");
        fileNet << "<svg height=\"" <<originalY + 1000 << "\" width=\"" << originalX + 1000 << "\">\n\n";
        write_net(fileNet,net);
        write_net(file, net);
        fileNet.close();
    }*/

    for(auto& cell: vCell){
        string fileName = cell->get_name();  
        ofstream fileNet("./dumpRN/"+fileName+".html");
        fileNet << "<svg height=\"" <<originalY + 1000 << "\" width=\"" << originalX + 1000 << "\">\n\n";
        for(int netIter=0; netIter<cell->get_relate_net_num();++netIter){
            Net_C* net = cell->get_relate_net_by_id(netIter);
            write_net(fileNet,net);
        }
        fileNet.close();
    }   
    //draw grid
    draw_grid(file, 0, 0, 0, 0, 0, m_pChip->get_layer_num(), originalY);
    for(double x=0;x<m_pChip->get_num_cols();++x){
        for(double y=0;y<m_pChip->get_num_rows();++y){
                double z=0;
                draw_grid(file, x, y, z, x+1, y, z, originalY);
                draw_grid(file, x, y, z, x, y+1, z, originalY);
    //            draw_grid(file, x-0.5, y-0.5, z-0.5, x-0.5, y-0.5, z+0.5, originalY);
//
    //            //draw_grid(file, x+0.5, y-0.5, z-0.5, x+0.5, y+0.5, z-0.5, originalY);
    //            //draw_grid(file, x+0.5, y-0.5, z-0.5, x+0.5, y-0.5, z+0.5, originalY);
//////
    //            //draw_grid(file, x-0.5, y+0.5, z-0.5, x+0.5, y+0.5, z-0.5, originalY);
    //            //draw_grid(file, x-0.5, y+0.5, z-0.5, x-0.5, y+0.5, z+0.5, originalY);
//////
    //            //draw_grid(file, x-0.5, y-0.5, z+0.5, x-0.5, y+0.5, z+0.5, originalY);
    //            //draw_grid(file, x-0.5, y-0.5, z+0.5, x+0.5, y-0.5, z+0.5, originalY);
//////
    //            //draw_grid(file, x+0.5, y+0.5, z-0.5, x+0.5, y+0.5, z+0.5, originalY);
    //            //draw_grid(file, x+0.5, y-0.5, z+0.5, x+0.5, y+0.5, z+0.5, originalY);
    //            //draw_grid(file, x-0.5, y+0.5, z+0.5, x+0.5, y+0.5, z+0.5, originalY);
    //        
        }    
    }
}

void dmMgr_C::init(){
    cout << "\033[94m[DM]\033[0m - initializing chip...\n";
    vector<Net_C*> pNet;
    this->m_pDesign->get_net_list(pNet);
    for(auto &net : pNet){
        assert(net->init_route());
    }
    cout << "\033[94m[DM]\033[0m - initializing chip \033[92mcompleted!\033[0m\n";
    cout << "\033[94m[DM]\033[0m - initializing global router...\n";
    m_pGr = new GlobalRouter_C(this->m_pChip, pNet);
    //m_pPlacer = new Placer_C(this->m_pChip, this->m_pDesign);
    cout << "\033[94m[DM]\033[0m - initializing global router \033[92mcompleted!\033[0m\n";
    cout << "\033[94m[DM]\033[0m - initializing global placer...\n";
    m_pPlacer = new Placer_C(m_pChip,m_pDesign, m_pGr, m_tStart);
    cout << "\033[94m[DM]\033[0m - initializing global placer \033[92mcompleted!\033[0m\n";
}

void dmMgr_C::run(){
    // vector<CellInstance_C*> moving_cell;
    // vector<Pos2d> moving_pos;
    // moving_cell.push_back(m_pDesign->get_cell_by_id(5));
    // moving_pos.push_back(Pos2d(1,2));
    // moving_cell.push_back(m_pDesign->get_cell_by_id(2));
    // moving_pos.push_back(Pos2d(1,2));
    // m_pChip->print_all_supply_demand_table();
    // m_pDesign->move_cells_to_pos(moving_cell,moving_pos);
    // m_pChip->print_all_supply_demand_table();
    // m_pChip->print_all_supply_demand_table();
    // cout << "[DM] - move " << m_pDesign->get_cell_by_id(5)->get_name() << " to (1,2)\n";
    // if(m_pDesign->move_cell_to_pos(m_pDesign->get_cell_by_id(5),Pos2d(1,2)))
    //     cout << "[DM] - ok\n";
    // else cout << "[DM] - failed\n";
    // cout << "[DM] - moved_cell_list.size() = " << m_pDesign->get_moved_cell_id_list().size() << "\n";
    // m_pChip->print_all_supply_demand_table();
    // cout << "[DM] - move " << m_pDesign->get_cell_by_id(2)->get_name() << " to (2,2)\n";
    // if(m_pDesign->move_cell_to_pos(m_pDesign->get_cell_by_id(2),Pos2d(2,2)))
    //     cout << "[DM] - ok\n";
    // else cout << "[DM] - failed\n";
    // cout << "[DM] - moved_cell_list.size() = " << m_pDesign->get_moved_cell_id_list().size() << "\n";
    // m_pChip->print_all_supply_demand_table();

    ////if(debug_mode) m_pChip->print_all_supply_demand_table();
    //m_pGr->reRoute_all_net();
    m_pGr->reRoute_group_net();
    //m_pGr->reRoute_all_net_ori_route("use_prim");
    set_ori_route();
    //m_pChip->print_all_supply_demand_table();
    ////ofstream file("net_analysis.csv");
    //m_pPlacer->cellMovement_group();
    ////m_pDesign->net_analysis(file);
    ////file.close();
    ////file.open("net_analysis_afterPlace.csv");
    m_pPlacer->cellMovement_relativeNet(1);
    m_pPlacer->cellMovement_relativeNet(9);
    m_pPlacer->cellMovement_relativeNet(9);
    m_pPlacer->cellMovement_relativeNet(9);
    //m_pPlacer->cellMovement_relativeNet(25);
    ////m_pDesign->net_analysis(file);
    ////file.close();

    ////m_pChip->print_layer_density();
    //m_pPlacer->cellMovement();
    
    ////if(debug_mode) m_pChip->print_all_supply_demand_table();
    //cout << "\n\033[94m[DM]\033[0m - Example new Routing result:\n";
    //print_net_route(0);
    cout << "\033[94m[DM]\033[0m - final total WL = " << m_pDesign->get_total_WL() << "\n";
}


