
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <math.h>
#include "params.h"


using namespace cv;
using namespace std;

//returns the point of the brightest pixel, on the channel of the passed color
Point findLED(InputArray _imgFrame, int colorChnl){

    Mat imgFrame = _imgFrame.getMat();

    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point pt;

/* //measures by total intensity not individual color channels...wrong datatype here, doesn't work
    Vec3b intensity = frame.at<Vec3b>(0, 0);
    uchar blue = intensity.val[0];
    uchar green = intensity.val[1];
    uchar red = intensity.val[2];
*/

    //separate image into color channels
    vector<Mat> spl;
    split(imgFrame, spl);
    minMaxLoc( spl[ colorChnl ], &minVal, &maxVal, &minLoc, &maxLoc);

    return maxLoc;
}

//get euclidean distance between two points
double EucliDis(Point a, Point b){
    return sqrt( (b.y - a.y)*(b.y - a.y) + (b.x - a.x)*(b.x - a.x)  );

}


//attempt to remove noise from pixel tracking vector, only one LED
vector<Point> smoothTrack(vector<Point> coords, vector<double> dist, int length){
    //dist[i] is distance between coords[i-1] and coords[i]
    double maxDist = 4.0;

    //the vector we will return:
    vector<Point> newcoords (length);

    newcoords[0] = coords[0];
    Point lastKnown = coords[0];

    int i;
    for(i=1; i < length; ++i){
        if(EucliDis(lastKnown, coords[i]) < maxDist){
            newcoords[i] = coords[i];
            lastKnown = coords[i];
        } else{
            newcoords[i] = lastKnown;
        }

    }

    return newcoords;
}

//attempt to remove noise from tracked pixel, using two LEDs
vector<Point> smoothTrack2(vector<Point> coords1, vector<Point> coords2, int length){
    //dist[i] is distance between coords[i-1] and coords[i]
    double ledDist = LEDDIST;
    double maxDist = MAXDIST;
    int sameSpot = 0;

    //the vector we will return:
    vector<Point> newcoords (length);

    newcoords[0] = coords1[0];
    Point lastKnown = coords1[0];
    int known =0;

    int i;
    for(i=1; i < length; ++i){
        if(  (EucliDis(coords1[i], coords2[i]) < ledDist && (EucliDis(lastKnown, coords1[i]) < maxDist*(i-known))) || //red/green close and not big jump OR
           sameSpot > SAMESPOT)                                                                               //fourth time on same spot
        {
            //the spot gets stuck by stationary lights...
            /*if(EucliDis(newcoords[i], coords1[i]) == 0){
                newcoords[i] = coords1[i+10];
            } else {*/
                newcoords[i] = coords1[i];
                lastKnown = coords1[i];
                known = i;
                if(sameSpot > 6) newcoords[i] = coords1[i+8];
            //}

        } else{
            newcoords[i] = lastKnown;
        }
        if(EucliDis(newcoords[i], newcoords[i-1]) == 0){
            sameSpot++;
        } else {
            sameSpot = 0;
        }

    }

    return newcoords;
}


//returns the current orientation vector from two points
//between -pi and pi
//Point a should be the earlier frame
double getOrientation(Point a, Point b){
    // get slope
    double m = ((double)(b.y - a.y)) / (b.x - a.x);
    double pi = 3.1415;
    double rads;

    //arctan range is [-pi/2, pi/2]
    if(b.x < a.x){ //if object is moving leftward
        if(m < 0) rads = atan(m) + pi;
        if(m > 0) rads = atan(m) - pi;
    } else{
        rads = atan(m);
    }

    return rads;
}

void printHelp(void){

    cout << "vTrack: Laboratory Animal Video Tracking" << endl << endl;

    cout << "Usage: vidtrack OPTIONS input_video_filename [input_video_filename2 ...]" << endl << endl;

    cout << "OPTIONS:" << endl;

    cout << "May include any combination of the following options, with or without preceding '-' (must be lowercase)" << endl << endl;

    cout << "r" << "\tTrack the brightest red pixel" << endl;
    cout << "b" << "\tTrack the brightest blue pixel" << endl;
    cout << "g" << "\tTrack the brightest green pixel" << endl;

    cout << "v" << "\tOutput video showing tracking results" << endl;
    cout <<"\t\tNote: this increases running time nontrivially." << endl;
    cout <<"\t\tNot recommended for batch mode." << endl;
    cout << "h" << "\tDisplay this help text and exit" << endl << endl;

    cout <<"Example: track red and green pixels for two video files," << endl;
    cout << "\t and output videos for each" << endl;
    cout <<"$ vidtrack -rgv video1.avi video2.avi" << endl << endl;

    cout << "Parameters for smart tracking can be adjusted in params.h" << endl;

    exit(0);
}

