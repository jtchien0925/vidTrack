#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "project_IO.hpp"

using namespace cv;
using namespace std;



int main(int argc, char** argv){

    int b = 0;
    int g = 0;
    int r = 0;
    int v = 0;


    int i = 0;
    while(argv[1][i] != NULL){
        if(argv[1][i] == 'b') b = 1;
        if(argv[1][i] == 'g') g = 1;
        if(argv[1][i] == 'r') r = 1;
        if(argv[1][i] == 'v') v = 1;
        if(argv[1][i] == 'h') printHelp();
        ++i;
    }

    if(argc <= 2){
        cout << "Please run using at least one tracking option and one input file" << endl;
        cout << "Run '$ vidtrack -h' for help." << endl;
        return 0;
    }

    int mtds = b+g+r;
    if(mtds == 0){
        cout << "Please run command with at least one of the following options:" << endl;
        cout << "(b, g, r)" << endl;
        return 0;
    }
    int clrs[] = {b, g, r,v};



    int files = 2;
    //when video is analyzed, read a new chunk
    while(files < argc){
    int retval = vidAnalyze(argv[files], clrs);
    if (retval != 0){
        cout << "Something went wrong :/" << endl;
        exit(1);
    }
    files++;
    }

    return 0;
}
