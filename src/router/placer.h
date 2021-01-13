#ifndef PLACER_H
#define PLACER_H
#include <tuple>
#include <vector>
#include "module.h"
#include <unordered_map>
#include "globalRouter.h"
#include <assert.h>
using namespace std;

typedef tuple<int, int, int> Pos; // layer, row, col

class Placer_C{
    Chip_C* m_pChip;
    Design_C* m_pDesign;
    GlobalRouter_C* m_pGr;
    unordered_map<Pin_C*, Pos3d> mPinBestPos;
    unordered_map<Net_C*, Pos3d> m_NetCenter;
    unordered_map<CellInstance_C*, Pos2d> mCellBestPos;
    vector<CellInstance_C*> m_vCell;
    vector<CellInstance_C*> m_vMoveCell;
    vector<Net_C*> m_vNeedRouteNet;
    clock_t m_tStart;
public:
    string pos2d2str(Pos2d pos);
    Placer_C(Chip_C* p_pChip, Design_C* p_pDesign,  GlobalRouter_C* p_pGr, clock_t p_start);
    void calPinPos();
    void calCellBestPos();
    void set_neededRoutingNet(CellInstance_C*);

    void cellMovement();
    void cellMovement_relativeNet(int range=0);
    bool group_cell_Movement(vector<CellInstance_C*> p_vCell);
    void cellMovement_group();
    void cellMovement_SA();
    void clear();
};

#endif