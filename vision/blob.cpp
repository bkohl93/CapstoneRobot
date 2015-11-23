// blobdetect.cpp
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <ctime>
using namespace std;
using namespace cv;

const double CAMERA_HEIGHT_FROM_GROUND = 1.2;   // height of camera from ground in meters
const double CAMERA_HEIGHT_FROM_BEACON = 0.2;   // height of camera from beacon in meters
const double CAMERA_DIAGONAL_ANGLE = 69.0;      // diagonal angle of view for camera in degrees
                                                // logitech c525 fov is 69 degrees, Samsung Galaxy S5 is 90 degrees
                                                
struct hsvParams
  {
  int hL, sL, vL, hH, sH, vH;
  };
////////////////////////////////////////////
void removenoise(Mat&);
/////////////////////////////////////////////

Mat findBiggestBlob(Mat &src){
int largest_area=0;
int largest_contour_index=0;
Mat temp(src.rows,src.cols,CV_8UC1);
Mat dst(src.rows,src.cols,CV_8UC1,Scalar::all(0));
src.copyTo(temp);

vector<vector<Point> > contours; // storing contour
vector<Vec4i> hierarchy;

findContours( temp, contours, hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

for( int i = 0; i< contours.size(); i++ ) // iterate
{
    double a=contourArea( contours[i],false);  //Find the largest area of contour
    if(a>largest_area)
    {
        largest_area=a;
        largest_contour_index=i;
    }
}

drawContours( dst, contours,largest_contour_index, Scalar(255,0,0), CV_FILLED, 8, hierarchy );
// Draw the largest contour
return dst;
}

void findGrass(Mat &src, Mat &HSV){
  int iLowH = 30;
  int iHighH = 50;

  int iLowS = 60;
  int iHighS = 255;

  int iLowV = 0;
  int iHighV = 255;

  Mat imgThresholded;
  Mat temp;
  src.copyTo(temp);

  inRange(HSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    //dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    //morphological closing (fill small holes in the foreground)
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    //erode( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(50, 50)) );

   imgThresholded=findBiggestBlob(imgThresholded);

    for(int i=0;i<imgThresholded.rows-1;i++){
        for(int j=0;j<imgThresholded.cols-1;j++){
            if(imgThresholded.at<uchar>(i,j)==0){
                temp.at<Vec3b>(i,j)=(0,0,0);
            }
        }
    }

    for(int i=0;i<temp.cols-1;i++){
        int minRow=0;
        while(minRow<temp.rows-1 && temp.at<Vec3b>(minRow,i)[0]==0 && temp.at<Vec3b>(minRow,i)[1]==0 && temp.at<Vec3b>(minRow,i)[2]==0){
            src.at<Vec3b>(minRow,i)=(0,0,0);
            minRow++;
        }
        if(minRow!=temp.rows-1){
            int maxRow=temp.rows-1;
            while(maxRow>maxRow && temp.at<Vec3b>(maxRow,i)[0]==0 && temp.at<Vec3b>(maxRow,i)[1]==0 && temp.at<Vec3b>(maxRow,i)[2]==0){
                src.at<Vec3b>(maxRow,i)=(0,0,0);
                maxRow--;
            }
        }
    }
}

void removenoise(Mat& image)
  {
  //Morphologial opening
  erode(image,image,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));
  dilate(image,image,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));
  //Morphological closing
  dilate(image,image,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));
  erode(image,image,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));
  }

// get angle to turn robot (right or left) and distance from robot to object
// img -> the current camera image
// objectRows -> rows from the top that the center of the object resides in (pixels)
// objectCols -> columns from the left that the center of the object resides in (pixels)
void tilt_turn_degrees(Mat img, int object_rows, int object_cols)
{
    int rows = img.rows; // height of camera image in pixels
    int cols = img.cols; // width of camera image in pixels

    double pixel_diagonal = sqrt(rows * rows + cols * cols);           // (pythagorean) diagonal length of image in pixels
    double degrees_per_pixel = CAMERA_DIAGONAL_ANGLE / pixel_diagonal; // ratio of real world degrees to pixels in the image
    
    int center_rows = rows / 2; // the center height is half of the total height
    int center_cols = cols / 2; // the center width is half of the total width
    
    int diff_rows = center_rows - object_rows; // difference between center and object rows
    int diff_cols = center_cols - object_cols; // difference between center and object cols
    
    double turn_robot_x_degrees = diff_cols * degrees_per_pixel;  // positive -> turn left, negative -> turn right
    double tilt_camera_x_degrees = diff_rows * degrees_per_pixel; // positive -> tilt up, negative -> tilt down
    
    double tilted_degrees = 90 - tilt_camera_x_degrees;         // assuming camera is parallel to ground (90 degrees)
    double tilted_radians = tilted_degrees * 3.1415962 / 180.0; // c++ tan() function uses radians

    double distance = CAMERA_HEIGHT_FROM_GROUND * tan(tilted_radians); // triangle formula for finding distance
    
    cout << "Turn robot " << turn_robot_x_degrees << " degrees." << endl;   // a negative number means turn right
    cout << "Tilt camera " << tilt_camera_x_degrees << " degrees." << endl; // this won't actually be needed
    cout << "Distance to object is " << distance << " meters" << endl;      // maybe make them move distance/2 and reassess?
}

int main()
  {
  hsvParams hsv = {0,0,200,180,90,255};

  //Set up blob detection parameters
  SimpleBlobDetector::Params params;
  params.minDistBetweenBlobs = 0.0f;
  params.filterByInertia = true;
  params.filterByConvexity = false;
  params.filterByColor = false;
  params.filterByCircularity = false;
  params.filterByArea = true;

        params.minThreshold = 200;
        params.maxThreshold = 250;
        params.thresholdStep = 1;

        params.minArea = 50;
        params.minConvexity = 0.3;
        params.minInertiaRatio = 0.15;

        params.maxArea = 8000;
        params.maxConvexity = 10;


  vector<KeyPoint> keypoints;

clock_t start;
double duration=0;

  const string filename("./webcam1.jpg");
  string text("Object not found");
  //Initialize camera
  VideoCapture cap(1);
  if ( !cap.isOpened() )  // if not success, exit program
    {
    cout << "Cannot open the web cam" << endl;
    return -1;
    }

while(true){
  Mat img, imgHSV, imgTHRESH, out;
//  img = imread(filename, CV_LOAD_IMAGE_COLOR);
cap>>img;

  if(img.empty()){
     cout << "can not open " << endl;
     return -1;
  }

  //convert color to HSV, threshold and remove noise
  cvtColor(img, imgHSV, COLOR_BGR2HSV);
  findGrass(img,imgHSV);
  cvtColor(img, imgHSV, COLOR_BGR2HSV);

  inRange(imgHSV, Scalar(hsv.hL, hsv.sL, hsv.vL),
         Scalar(hsv.hH, hsv.sH, hsv.vH), imgTHRESH);
  removenoise(imgTHRESH);

 // namedWindow("Input", WINDOW_AUTOSIZE);
 // namedWindow("Detection", WINDOW_AUTOSIZE);

  //Initialize blobdetector with predefine parameters
  SimpleBlobDetector blobDetect = SimpleBlobDetector(params);
  blobDetect.detect( imgTHRESH, keypoints );

  drawKeypoints(imgTHRESH, keypoints, out, CV_RGB(0,0,0), DrawMatchesFlags::DEFAULT);
  //Circle blobs
  for(int i = 0; i < keypoints.size(); i++)
    {
    circle(out, keypoints[i].pt, 1.5*keypoints[i].size, CV_RGB(0,255,0), 20, 8);
    }

   if(keypoints.size() == 1){
    text = "Object Found";
    cout<<endl<<endl<<"Object Found"<<endl;
    //cout<<out.rows<<" "<<out.cols<<endl;
    //cout<<keypoints[0].pt.x<<" "<<keypoints[0].pt.y<<endl;

    tilt_turn_degrees(img, keypoints[0].pt.y, keypoints[0].pt.x);
  }
  if(keypoints.size() > 1){
    text = "Error";
    cout<<"No Object Found"<<endl;
  }

  putText(out, text, Point(100,200), FONT_HERSHEY_PLAIN, 20, Scalar(0, 0, 255), 20);

  //for(;;)
    //{
    //imshow("Input", img);
    //imshow("Detection", out);
    //if(waitKey(30) >= 0 ) break;
    //}

   duration=0;
   start = std::clock();
   while(duration<4){
       cap>>img;
       duration = (clock() - start ) / (double) CLOCKS_PER_SEC;
   }
}
  return 0;
  }


