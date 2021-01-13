#ifndef DM_H
#define DM_H
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>
#include <numeric>
#include <map>
#include <sstream>
#include <set>
#include <math.h>
#include "globalRouter.h"
#include "placer.h"
#include "module.h"
#include "parser.h"

class dmMgr_C{
    Chip_C* m_pChip;
    Design_C* m_pDesign;
    ExtraDemand_C* m_pExtraDemand; // same_grid and adj_hor_grid demand
    CellLibrary_C* m_pCellLibrary; // master cell type list
    GlobalRouter_C* m_pGr;
    Placer_C* m_pPlacer;
    int m_origin_total_WL = 0;
    int debug_mode = 0;
    clock_t m_tStart;
public:
    dmMgr_C(parser_C* p_pParser, clock_t start);
    void print_info();
    void dump_info();
    void output_result(char*);
    void print_demand_table();
    void print_net_route(int);
    void dump_dmd(string);
    void dump_dmd_svg(string);
    void init();
    void run();
    Direction getDir(Pos3d pos_s, Pos3d pos_e);
    int getLength(Pos3d pos_s, Pos3d pos_e);
    void output_svg();
    void output_cellpos_svg(string);

    void write_net(ofstream& file, Net_C* net);

    void set_ori_route();
    //debug mode
    void debug_mod(){debug_mode=1;}
    void normal_mod(){debug_mode=0;}
    //access data
    Chip_C* getChip(){return m_pChip;}
    Design_C* getDesign(){return m_pDesign;}

    int get_origin_total_WL(){return m_origin_total_WL;}
    int get_total_WL() {return m_pDesign->get_total_WL();};

    void compareWithOverflowMes(char* fileName);
};


#endif
