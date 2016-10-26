#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "trackingMethods.hpp"
#include <iostream>
#include <fstream>



using namespace cv;
using namespace std;

/* Function declarations */
int csvOut(vector<Point>, vector<Point>, vector<Point>, vector<double>, char*);
int writeComposite(vector<Point>, vector<Point>, vector<Point>, char*, Size);

/*reads a video file, marks the pixel coordinates and writes to file*/
int r, g, b;
int vidAnalyze(char* filename, int* colors){
    int v;
    int e = 0;
    while(filename[e] != NULL) ++e;


    b = colors[0];
    g = colors[1];
    r = colors[2];
    v = colors[3];

    VideoCapture cap(filename);
    //equivalent to:
    //VideoCapture cap = open(filename);

    int numframes = cap.get(CV_CAP_PROP_FRAME_COUNT);

    if(!cap.isOpened()){
        cout << "Cannot open file!!!!!" << endl;
        return -1;
    }

    Size S = Size((int) cap.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT));

    //VideoWriter outVid("outVid.avi", cap.get(CV_CAP_PROP_FOURCC),cap.get(CV_CAP_PROP_FPS), S);

    Point bluePxl;
    Point greenPxl;
    Point redPxl;
    Mat frame; //Holds a single frame of video

//channels: blue == 0, green == 1, red == 2
    int blue = 0;
    int green = 1;
    int red = 2;

    //cout << "greenX, greenY, redX, redY" << endl;
    //store tracked location data in a vector
    vector<Point> trackLocb (numframes);
    vector<Point> trackLocg (numframes); //for multiple methods
    vector<Point> trackLocr (numframes);
    //contains interpoint distances
    vector<double> dists (numframes - 1);
    vector<double> directiong (numframes);
    directiong[0] = 0.0;


    cout << "Analyzing..." << endl;
    int cntr;
    for(cntr=0;cntr < numframes; cntr++){
        cap >> frame;
        //frame << cap;
        //cap.read(frame);

        if(b){ //track method logic goes here
            bluePxl = findLED(frame, blue);
            trackLocb[cntr] = bluePxl;
        }
        if(g){
            greenPxl = findLED(frame, green);
            trackLocg[cntr] = greenPxl;
            if(cntr!=0) directiong[cntr] = getOrientation(trackLocg[cntr], trackLocg[cntr-1]);
        }
        if(r){
            redPxl = findLED(frame, red);
            trackLocr[cntr] = redPxl;
        }
        //circle(frame, greenPxl, 5.0, Scalar(0,255,0), -1, 8);
        //circle(frame, redPxl, 5.0, Scalar(0,0,255), -1, 8);
//if(cntr!=0) dists[cntr - 1] = EucliDis(trackLoc[cntr-1], trackLoc[cntr]);
//cout << "dist: " << EucliDis(trackLoc[cntr-1], trackLoc[cntr]) << endl;
        //outVid << frame;

    }

/*  cout << "Brightest green pixel: (" << maxLoc.x << ", " << maxLoc.y << ")" << endl;
    cout << CV_MAT_TYPE(frame.type()) << endl;
    cout << frame.depth() << endl;
    cout << frame.channels() << endl;
 */

 //smooth pixel tracking
    vector<Point> smoothLoc (numframes);
    //smoothLoc = smoothTrack(trackLocg, dists, numframes);
    smoothLoc = smoothTrack2(trackLocg, trackLocr, numframes);
 //write video with better tracking
    cap.release();
    //outVid.release();

    VideoCapture cap2(filename);
    //equivalent to:
    //VideoCapture cap = open(filename);

    numframes = cap2.get(CV_CAP_PROP_FRAME_COUNT);

    if(!cap2.isOpened()){
        cout << "Cannot open file!!!!!" << endl;
        return -1;
    }

    Size S2 = Size((int) cap.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
                  (int) cap.get(CV_CAP_PROP_FRAME_HEIGHT));
if(v){
    //get output video name
    char outfile[e+5];
    int i;
    for(i=0; i < (e-3); ++i){
        outfile[i] = filename[i];
    }

    outfile[e-3] = 'o';
    outfile[e-2] = 'u';
    outfile[e-1] = 't';
    outfile[e] = '.';
    outfile[e+1] = 'a';
    outfile[e+2] = 'v';
    outfile[e+3] = 'i';
    outfile[e+4] = filename[e];

VideoWriter outVideo(outfile, cap2.get(CV_CAP_PROP_FOURCC),cap2.get(CV_CAP_PROP_FPS), S);
    //VideoWriter outVideo("led_sample_result.avi", cap2.get(CV_CAP_PROP_FOURCC),cap2.get(CV_CAP_PROP_FPS), S);
    for(cntr = 0; cntr < numframes; ++cntr){
        cap2 >> frame;

        if(b) circle(frame, trackLocb[cntr], 5.0, Scalar(255,0,0), -1, 8);
        if(g+r==2) circle(frame, smoothLoc[cntr], 5.0, Scalar(0,255,0), -1, 8);
        else {
            if(r) circle(frame, trackLocr[cntr], 5.0, Scalar(0,0,255), -1, 8);
            if(g) circle(frame, trackLocg[cntr], 5.0, Scalar(0,255,0), -1, 8);
        }
        outVideo << frame;
    }
    cout << "Writing " << outfile << endl;
}



    csvOut(trackLocb, trackLocg, trackLocr, directiong, filename);
    writeComposite(trackLocb, trackLocg, trackLocr, filename, S);

    return 0;
}


//write 2D coordinates to text file in csv format
int csvOut(vector<Point> Locb, vector<Point> Locg, vector<Point> Locr, vector<double> dir, char* csvfile){
    //rename file to .csv
    int e = 0;
    while (csvfile[e] != NULL ){
        e++;
    }
    //cout << e << endl;
    csvfile[e-3] = 'c';
    csvfile[e-2] = 's';
    csvfile[e-1] = 'v';

cout << "Writing " << csvfile << endl;
    ofstream csvFl;
    csvFl.open( csvfile, ios::out | ios::trunc );
    //csvFl.open( "/home/nda/code/sweng/led_sample.csv", ios::out | ios::trunc );
    if(b)    csvFl << "blueX,blueY,";
    if(g)    csvFl << "greenX,greenY,";
    if(r)    csvFl << "redX,redY,";
    csvFl << "deg, rad" << endl;
//    cout << Locs.size() << endl;
    int i;
    for(i=0; i < Locg.size(); ++i){
        if(b) csvFl << Locb[i].x << "," << Locb[i].y << ",";
        if(g) csvFl << Locg[i].x << "," << Locg[i].y << ",";
        if(r) csvFl << Locr[i].x << "," << Locr[i].y << ",";

        csvFl << (dir[i]*180/3.141) <<"," << dir[i] << endl;
    }

    return 0;
}
/*
int vidOut(vector<Point> Locs){
    ofstream vidFl;
    vidFl.open( "/home/nda/code/sweng/led_sample.csv", ios::out | ios::trunc );


}
*/

int writeComposite(vector<Point> Locb, vector<Point> Locg, vector<Point> Locr, char* jpgfile, Size s){
    Mat composite(s, CV_8UC3, Scalar(255,255,255));
    //Mat composite(5,5,CV_16U);

    vector<int> qual;
    qual.push_back(CV_IMWRITE_JPEG_QUALITY);
    qual.push_back(90);

    //composite = Scalar(0, 0 , 0);

    //rename file to .jpg
    int e = 0;
    while (jpgfile[e] != NULL ){
        e++;
    }

    jpgfile[e-3] = 'j';
    jpgfile[e-2] = 'p';
    jpgfile[e-1] = 'g';


cout << "Writing " << jpgfile << endl;
    //ofstream jpgFl;
    //jpgFl.open( jpgfile, ios::out | ios::trunc );
    int i;
    for(i=0; i < Locg.size(); ++i){
        if(b) circle(composite, Locb[i], 0.5, Scalar(255,0,0), -1, 8);
        if(g) circle(composite, Locg[i], 0.5, Scalar(0,255,0), -1, 8);
        if(r) circle(composite, Locr[i], 0.5, Scalar(0,0,255), -1, 8);
    }
    //imshow("shown", composite);
    //waitKey();

    bool ret = imwrite(jpgfile, composite);

    return 0;
}


