#include <iostream>
#include "dm.h"
#include "globalRouter.h"
#include "module.h"
#include "parser.h"

using namespace std;

int main(int argc, char** argv){
    /*Preliminary Information*/
    cout << "     #############################################################" << endl;
    cout << "     #                                                           #" << endl;
	cout << "     #                         [VDA LAB]                         #" << endl;
	cout << "     #-----------------------------------------------------------#" << endl;
    cout << "     #                     Cell Move Router                      #" << endl;
    cout << "     #                                                           #" << endl;
	cout << "     #############################################################" << endl;
    cout << endl;


	//variable declaration
    clock_t start,dm_start,end,dm_end;
    start = clock();
    srand( time(NULL) );

	if(argc < 3){
		cout << "usage: ./cell_move_router <input> <output>\n";
		return 0;
	}

	parser_C* pParser = new parser_C(argv[1]);
    //pParser->print_info();
    dmMgr_C* pDmMgr = new dmMgr_C(pParser,start);
    dm_start = clock();
    pDmMgr->init();
    pDmMgr->normal_mod();
    //pDmMgr->dump_info();
    //pDmMgr->print_info();

    //pDmMgr->dump_info();
    //pDmMgr->print_info();

    ////pDmMgr->output_cellpos_svg(string(argv[2])+"_cell.html");
    pDmMgr->run();
    
    dm_end = clock();
    ////printf("DM run-time: %f seconds\n", (double)(dm_end-dm_start)/CLOCKS_PER_SEC);
    //pDmMgr->dump_info();
    ////pDmMgr->dump_dmd(argv[1]);
    ////pDmMgr->dump_dmd_svg(argv[1]);
    pDmMgr->output_result(argv[2]);
    
    //pDmMgr->output_svg();
	end = clock();
    printf("Execution Time: %f seconds\n", (double)(end-start)/CLOCKS_PER_SEC);
    delete pParser;
    return 0;
}
