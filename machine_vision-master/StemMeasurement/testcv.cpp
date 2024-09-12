#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <cstdio>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/ml/ml.hpp>
//#include "opencv2/gpu/gpu.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/videostab/videostab.hpp"
#include "opencv2/opencv_modules.hpp"
//#include <fann.h>
#include <opencv2/ml/ml.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include <fann_cpp.h>
using namespace std;
using namespace cv;

std::map<int,vector<Vec4i> > same_angle;
void remove_horizontals(vector<Vec4i> &lines) {
    double best_angle = M_PI/2;
    double angle_diff = 8*(M_PI/180);
    double angle;
    for(auto iter=lines.begin(); iter!=lines.end();) {
        const Vec4i &ln(*iter);
        if(fabs(ln[0]-ln[3])<0.0001)
            angle = best_angle;
        else
            angle = atan(fabs(ln[1]-ln[3])/fabs(ln[0]-ln[2]));

        if((angle<(best_angle-angle_diff)) || (angle>(best_angle+angle_diff)))
            iter = lines.erase(iter);
        else {
            int deg = 2*(angle*180/M_PI);
            assert(deg>0);
            same_angle[deg].push_back(ln);
            ++iter;
        }
    }
}
float DistanceSquared(Point v0,Point v1,float X, float Y)
{
    float vx = v0.x - X, vy = v0.y - Y, ux = v1.x - v0.x, uy = v1.y - v0.y;
    float length = ux * ux + uy * uy;

    float det = (-vx * ux) + (-vy * uy); //if this is < 0 or > length then its outside the line segment
    if (det < 0)
        return (v0.x - X) * (v0.x - X) + (v0.y - Y) * (v0.y - Y);
    if (det > length)
        return (v1.x - X) * (v1.x - X) + (v1.y - Y) * (v1.y - Y);

    det = ux * vy - uy * vx;
    return (det * det) / length;
}
#include <utility>
vector<std::pair<Vec4i,Vec4i> > paired_lines;
static bool in_range_inclusive(float s,float e,float v,float w) {
    float s1 = std::min(s,e);
    float e1 = std::max(s,e);
    float v1 = std::min(v,w);
    float w1 = std::max(v,w);
    return (e1>v1) && (s1<w1);
}
static bool overlap(Point v0,Point v1,const Vec4i &other) {

    if(!(in_range_inclusive(v0.x,v1.x,other[0],other[2])))
        return false;
    else
        if(!(in_range_inclusive(v0.y,v1.y,other[1],other[3])))
            return false;
    return true;
}
static bool overlapY(Point v0,Point v1,const Vec4i &other) {

    return in_range_inclusive(v0.y,v1.y,other[1],other[3]);
}
bool isLeft(Point a, Point b, Point c){
    return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) > 0;
}
static bool linesCross(Point v0,Point v1,const Vec4i &other) {
    Point o1(other[0],other[1]),o2(other[2],other[3]);
    return isLeft(v0,v1,o1)!=isLeft(v0,v1,o2);
}
static double xSeparation(Point v0,Point v1,const Vec4i &other) {
    double low_x;
    double high_x;
    double low_x2;
    double high_x2;
    if(v0.y<v1.y) {
        low_x=v0.x;
        high_x=v1.x;
    }
    else {
        low_x=v1.x;
        high_x=v0.x;
    }
    if(other[1]<other[3]) {
        low_x2=other[0];
        high_x2=other[2];
    }
    else {
        low_x2=other[2];
        high_x2=other[0];
    }
    return std::min(fabs(low_x-low_x2),fabs(high_x-high_x2));
}
cv::Point findWireUpperCenterpoint() {
    // filter image
    cv::Point res;
}
bool scanLines(Point v0,Point v1,vector<Vec4i> & lines,double &closest,vector<Vec4i>::iterator &closest_line,
               vector<Vec4i>::iterator skip_iter) {
    bool found_closer=false;
    for(auto iter3=lines.begin(); iter3!=lines.end(); ++iter3) {
        if(iter3==skip_iter)
            continue;
        if(linesCross(v0,v1,*iter3))
            continue;
        if(!overlapY(v0,v1,*iter3))
            continue;
        double dist1 = DistanceSquared(v0,v1,(*iter3)[0],(*iter3)[1]);
        double dist2 = DistanceSquared(v0,v1,(*iter3)[2],(*iter3)[3]);
        if((dist1<0) || (dist2<0))
            continue;

        dist1=sqrt(dist1);
        dist2=sqrt(dist2);

        double avg_dist         = (dist1+dist2)/2.0;
        if(avg_dist<closest) {
            closest = avg_dist;
            closest_line = iter3;
            found_closer = true;
        }
    }
    return found_closer;
}
void remove_singletons(std::map<int,vector<Vec4i> > &lines) {
    for(auto iter=lines.begin(); iter!=lines.end(); ++iter) {
        vector<Vec4i> &x(iter->second);
        if(x.empty()) {
            continue;
        }

        auto iter_next = iter;
        auto iter_prev = iter;
        iter_next++;
        iter_prev--;
        if(iter==lines.begin()) {
            iter_prev= (++lines.rbegin()).base();
        }
        if(iter_next==lines.end())
            iter_next = lines.begin();
        vector<Vec4i> &prev_x(iter_prev->second);
        vector<Vec4i> &next_x(iter_next->second);
        for(auto iter2=x.begin(); iter2!=x.end(); ) {
            Point v0 = Point((*iter2)[0],(*iter2)[1]);
            Point v1 = Point((*iter2)[2],(*iter2)[3]);
            vector<Vec4i>::iterator closest_line;
            double closest = 1000000;
            int container = 0;
            if(scanLines(v0,v1,x,closest,closest_line,iter2))
                container=0;
            if(scanLines(v0,v1,prev_x,closest,closest_line,iter2))
                container=1;
            if(scanLines(v0,v1,next_x,closest,closest_line,iter2))
                container=2;
            if((closest>5) && closest<120) {
                paired_lines.push_back(make_pair(*iter2,*closest_line));
                switch(container) {
                    case 0: x.erase(closest_line); break;
                    case 1: prev_x.erase(closest_line); break;
                    case 2: next_x.erase(closest_line); break;
                }

                iter2 = x.erase(iter2);
            }
            else
                ++iter2;
        }
    }


    printf("We have %d pairs lines \n",paired_lines.size());
}
Mat createWhiteMask(Mat src) {
    Mat dst;
    vector<Mat> colors;
    cvtColor(src,dst,CV_RGB2HSV);
    split(dst,colors);
    Mat hue_mask1 = colors[0]>65;
    cv::imwrite("maskH.png",hue_mask1);
    Mat white_mask  = colors[1]<55;
    cv::imwrite("maskS.png",white_mask);
    Mat white_mask2 = colors[2]>158;
    cv::imwrite("maskV.png",white_mask);
    Mat comb_mask2;
    bitwise_and(hue_mask1,white_mask,comb_mask2);
    bitwise_and(comb_mask2,white_mask2,dst);
    comb_mask2 = dst.clone();
    cv::imwrite("maskALL.png",comb_mask2);
    return comb_mask2;
}
int findHangLine(cv::Mat &pic);
void testCV(int argc, char **argv)
{
    cv::Mat src;
    src = cv::imread(argv[1])(cv::Rect(1000,500,900,3700));
    findHangLine(src);
}
Mat filterHSV(vector<Mat> &planes,int Hbelow,int Habove,int S,int V) {
    Mat1b res = Mat1b::zeros(planes[0].size());
    res.setTo(255,(planes[2]<V) | (planes[1]>S) | (planes[0]>Hbelow) | (planes[0]<Habove) );
    return res;
}
vector<pair<int,int> > findPossibleLineStarts(const cv::Mat1b &edges,int row) {
    int discard_start=-1;
    int discard_end=-1;
    cv::imwrite("input.png",edges);
    vector<pair<int,int> > possible_starts;
    for(int i=0; i<edges.cols; ++i) {
        if(0==edges.row(row)(i))
            continue;
        //printf("%d\n",i);
        // scan for second line edge
        int line_end=i+1;
        for(; line_end<edges.cols; ++line_end) {
            if((edges.row(row)(line_end)!=0) || ((line_end-i) > 100)) { // next line edge found
                break;
            }
        }
        if(line_end==edges.cols)
            break;
        if(line_end-i < 5) { // not wide enough
            i = line_end-1;
            continue;
        }
        if(((line_end-i) > 4) &&  ((line_end-i) < 15)){ // line width looks ok
            possible_starts.push_back(make_pair(i,line_end));
            i = line_end; // continue from next point
            continue;
        }
        i = line_end-1; // to wide or to thin
    }
    return possible_starts;
}
vector<pair<int,int> > findPossibleStemStarts(const cv::Mat1b &edges,int row) {
    int discard_start=-1;
    int discard_end=-1;
    vector<pair<int,int> > possible_starts;
    for(int i=0; i<edges.cols; ++i) {
        if(0==edges.row(row)(i))
            continue;
        //printf("%d\n",i);
        // scan for second line edge
        int line_end=i+1;
        for(; line_end<edges.cols; ++line_end) {
            if((edges.row(row)(line_end)!=0) || ((line_end-i) > 100)) { // next line edge found
                break;
            }
        }
        if(line_end==edges.cols)
            break;
        if(line_end-i < 5) { // not wide enough
            i = line_end-1;
            continue;
        }
        if(((line_end-i) > 25) &&  ((line_end-i) < 90)){ // line width looks ok
            possible_starts.push_back(make_pair(i,line_end));
            i = line_end; // continue from next point
            continue;
        }
        i = line_end-1; // to wide or to thin
    }
    return possible_starts;
}
int highestGreenCrossingTheLine(int line_center,const cv::Mat &pic,const Mat &edges);
int nextEdgeRight(Point start_at,const cv::Mat1b &edges,int maxX) {
    for(int x=start_at.x; (x<edges.cols) && (x<maxX); ++x) {
        if(!edges(Point(x,start_at.y)))
            continue;
        return x;
    }
    return edges.cols;
}
int nextEdgeLeft(Point start_at,const cv::Mat1b &edges,int minX) {
    for(int x=start_at.x; (x>0) && (x>minX); --x) {
        if(!edges(Point(x,start_at.y)))
            continue;
        return x;
    }
    return 0;
}
bool verifyStems(pair<int,int> stem_sides,int row,const cv::Mat1b &edges,double &width_max) {
    // walk up from current center of stem
    // verify that width is similar to the one below
    // if number of correct width checks > 5 calculate stem width
    vector<Point> leftSide;
    vector<Point> rightSide;
    int collected_points=45;
    Point center((stem_sides.second+stem_sides.first)/2,row);
    float width=stem_sides.second-stem_sides.first;
    int failed_stem_locations=0;
    rightSide.push_back(Point(stem_sides.first,row));
    leftSide.push_back(Point(stem_sides.first,row));
    float start_width = width;
    for (;failed_stem_locations<4;) {
        center.y -= 1; // go up one row
        if(center.y<0)
            break;
        int nextRight = nextEdgeRight(center,edges,center.x+(width/2)+10);
        int nextLeft = nextEdgeLeft(center,edges,center.x-(width/2)-10);
        if(abs((nextRight-nextLeft)-width)>10) {
            failed_stem_locations++; // to wide or to narrow
            continue;
        }
        rightSide.push_back(Point(nextRight,center.y));
        leftSide.push_back(Point(nextLeft,center.y));
        center.x = (nextRight+nextLeft)/2;
        width = nextRight-nextLeft;
//        if(rightSide.size()>collected_points)
//            break;
    }
    if((start_width-width)<-2)
        return false;
    if(rightSide.size()<collected_points)
        return false;
    float width_sum=width;
    // parallel test
    Vec4f rightLine,leftLine;
    fitLine(rightSide,rightLine,CV_DIST_FAIR,0,0.01,0.01);
    fitLine(leftSide,leftLine,CV_DIST_FAIR,0,0.01,0.01);
    Vec2f rVec(rightLine(0),rightLine(1));
    Vec2f lVec(leftLine(0),leftLine(1));
    Mat hst_merged;
    //cout << rightLine << ' ' << leftLine << '\n';

    double angle=acos(rVec.dot(lVec))*180/M_PI;
    cout << "line line angle is " << angle << '\n';
    if((angle>10) && (angle < 350)) {
        return false;
    }
    //cout << rightLine << ' ' << leftLine << '\n';
    for(int i=0; i<rightSide.size(); ++i) {
        width_sum = width_sum*0.9 + (rightSide[i].x-leftSide[i].x)*0.1;
    }
    printf("Stem at %d,%d has width of %f mm / %f mm\n",stem_sides.first,row,width_sum*0.3,start_width*0.3);
    if(width_max>start_width) {
        return false;
    }
    width_max=start_width;
    cvtColor(edges,hst_merged,CV_GRAY2RGB);
    {
        Mat cdst2;
        cdst2 = hst_merged;

        vector<Vec4i> lines;
        Point p1(rightLine(2)-90*rVec(0),rightLine(3)- 90*rVec(1));
        Point p2(p1.x + 180*rVec(0),p1.y + 180*rVec(1));
        line( cdst2, p1, p2, Scalar(0,0,255), 1, CV_AA);

        Point p3(leftLine(2)-90*lVec(0),leftLine(3)- 90*lVec(1));
        Point p4(p3.x + 180*lVec(0),p3.y + 180*lVec(1));
        line( cdst2, p3, p4, Scalar(0,255,0), 1, CV_AA);
        imwrite("res2a.png",cdst2);
    }
    return true;
}
float stemWidth(const cv::Mat &pic) {

    int morph_size=11;
    int morph_size2=4;
    int morph_size3=1;
    int canny_param=13;
    Mat element = getStructuringElement( MORPH_ELLIPSE, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
    Mat element2 = getStructuringElement( MORPH_ELLIPSE, Size( 2*morph_size2 + 1, 2*morph_size2+1 ), Point( morph_size2, morph_size2 ) );
    Mat element3 = getStructuringElement( MORPH_CROSS, Size( 2*morph_size3 + 1, 2*morph_size3+1 ), Point( morph_size3, morph_size3 ) );

    vector<Mat> split_planes;
    Mat filtered,resul_g,res_blr,dst;

    cv::bilateralFilter(pic,filtered,-1,6,13);
    cv::imwrite("search_area.png",pic);
    Mat hsl,hst,hst_merged;

    morphologyEx( pic, hst_merged, MORPH_CLOSE, element );
    imwrite("search_area_H.png",hst_merged);
    hst_merged.copyTo(filtered );


    cvtColor(pic,hsl,CV_RGB2HSV_FULL);
    split(hsl,split_planes);
    Mat filter = (split_planes[1]<40);
    morphologyEx( filter, filter, MORPH_ERODE, element2);

    //Mat filter = filterHSV(split_planes,256*(76.0f/360.0f),256*(170.0f/360.0f),5,100); // 76 deg - 170 deg
    //    equalizeHist(split_planes[2],hst);
    //    split_planes[2] = hst;
    //    merge(split_planes,pic);
    //    cvtColor(pic,hst_merged,CV_HSV2RGB_FULL);

    //    cv::bilateralFilter(hst_merged,filtered,-1,13,13);
    //    hst_merged = filtered;
    imwrite("search_areaH.png",hst_merged);
    imwrite("search_areaH_FL.png",filter);

    cvtColor(hst_merged,resul_g,CV_RGB2GRAY);
    blur(resul_g,res_blr,Size(3,3));
    Canny(res_blr, dst, canny_param, canny_param*3, 3,true);
    morphologyEx( dst, dst, MORPH_CLOSE, element3 );
    dst.setTo(0,filter);
    imwrite("search_areaHE.png",dst);

    int i = dst.rows-1;
    // 45 is the number of checked stem parts
    for(i = dst.rows-1; i>45; --i) {
        vector<pair<int,int> > stemStarts = findPossibleStemStarts(dst,i);
        bool verified=false;
        double width;
        for(pair<int,int> &ln : stemStarts) {
            //cout << "Possible stem at " << i << " " <<ln.first << ' ' << ln.second <<'\n';
            verified|=verifyStems(ln,i,dst,width);
        }
        if(verified)
            return 1;
    }
    return 0;
}
bool greenAt(const Mat3b &pic,int x,int y,int minDiff=4) {
    cv::Vec3b sc = pic(y,x);
    double max_othe = std::max(sc(0),sc(2));
    return sc(1)>(max_othe+minDiff);
}
bool columnIsGreen(const Mat3b &pic,int x,int minDiff=2) {

    Mat3b column(pic.col(x));
    cv::Scalar sc = cv::mean(column);
    double max_othe = std::max(sc(0),sc(2));
    if(!(sc(1)>(max_othe+minDiff)))
        return false;
    return true;
}
bool columnHasGreen(const Mat3b &pic,int x,int minDiff=2) {
    Mat3b column(pic.col(x));
    for(int i=0; i<pic.rows; ++i) {
        cv::Vec3b sc = column(i);
        double max_othe = std::max(sc(0),sc(2));
        if(sc(1)>(max_othe+minDiff))
            return true;
    }
    return false;
}
void trimToGreen(Rect &r,const Mat3b &pic_) {
    cv::imwrite("gg.png",pic_(r));
    Mat3b pic;
    blur(pic_(r),pic,Size(3,3));
    int left=0,right=-1;
    for(int x=0; x<r.width; ++x ) {
        left=x;
        if(columnIsGreen(pic,x))
            break;
    }
    for(; left>0; --left) {
        if(!columnHasGreen(pic,left)) {
            break;
        }
    }
    if(left>200) {
        int max_left=left;
        left-=200;
        for(; left<max_left; ++left) {
            if(columnHasGreen(pic,left)) {
                break;
            }
        }
        printf("Trimmed %d\n",left);
        r.x += left;
        r.width-=left;
    }
}
static bool brighter(Scalar a,Scalar b) {
    return (a(0)+a(1)+a(2)) > (b(0)+b(1)+b(2));
}
int centerOfBrightestLine(vector<pair<int,int> > &lines,Mat &pic) {
    Mat r0(pic.row(0));
    Scalar max_val(0,0,0,0);
    int max_idx=0;
    int idx=0;
    for(pair<int,int> & ln : lines) {
        Scalar current = mean(r0.colRange(ln.first,ln.second));
        if(brighter(current,max_val)) {
            max_val = current;
            max_idx=idx;
        }
        idx++;
    }
    return (lines[max_idx].first+lines[max_idx].second)/2;
}
int findHangLine(cv::Mat &pic) {
    double minval,maxVal;
    int morph_size=3;
    Mat element = getStructuringElement( MORPH_ELLIPSE, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
    //Mat elementS = getStructuringElement( MORPH_RECT, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
    Mat dst;
    Mat resul;
    Mat hsl,hst,hst_merged;
    //medianBlur(pic,resul,3);
    pic.copyTo(resul);
    medianBlur(pic,pic,3);
    cv::imwrite("bling.png",pic);
    vector<Mat> split_planes;
    cvtColor(pic,hsl,CV_RGB2HSV_FULL);
    split(hsl,split_planes);
    equalizeHist(split_planes[2],hst);
    split_planes[2] = hst;
    merge(split_planes,hst_merged);
    hst_merged.copyTo(resul);
    cvtColor(hst_merged,hst_merged,CV_HSV2RGB_FULL);
    cv::imwrite("blingH.png",hst_merged);
    minMaxLoc(split_planes[0],&minval,&maxVal);
    //cout << minval << " - " << maxVal << '\n';
    //    cout << split_planes[0].rowRange(549,554).colRange(400,420) << '\n';
    //resul.setTo(0,(split_planes[2]<200) | (split_planes[1]>25) | (split_planes[0]>245) | (split_planes[0]<65) );
    //resul.setTo(0,(split_planes[2]<190) | (split_planes[1]>65) | (split_planes[0]>160) );
    Mat hsv_mask = filterHSV(split_planes,205,5,65,100);
    //resul.setTo(0,(split_planes[2]<190) | (split_planes[1]>55) | (split_planes[0]>160) | (split_planes[0]<65) );

    morphologyEx( hsv_mask, hsv_mask, MORPH_CLOSE, element );
    int canny_param=15;
    Mat res_blr,resul_g;
    cvtColor(hst_merged,resul_g,CV_RGB2GRAY);
    blur(resul_g,res_blr,Size(3,3));
    Canny(res_blr, dst, canny_param, canny_param*3, 3,true);
    cv::imwrite("edg1.png",dst);
    Mat dstcopy;
    dst.copyTo(dstcopy);
    dst.setTo(0,hsv_mask);
    cv::imwrite("edg.png",dst);
    //resul.setTo(0,(split_planes[2]<100) | (split_planes[1]>15) | (split_planes[0]>195) | (split_planes[0]<65) );
    //resul.setTo(0,(split_planes[2]<200));
    //    for(int x=0; x<pic.rows/3; ++x) {
    //        result1.push_back(resul);
    //    }
    int start_row=0;
    vector<pair<int,int> > line_starts;
    for(int i=0; i<100; ++i) {
        line_starts = findPossibleLineStarts(Mat1b(dst),i);
        for(pair<int,int> &ln : line_starts) {
            cout << "Possible line at " << i << " " <<ln.first << ' ' << ln.second <<'\n';
        }
        if(!line_starts.empty()) {
            start_row = i;
            break;
        }
    }
    if(line_starts.empty()) {
        cout << " No hang line found\n";
        exit(1);
    }
    hst_merged = hst_merged.rowRange(start_row,hst_merged.rows);
    int center_of_line = centerOfBrightestLine(line_starts,hst_merged);
    //cout << "Center line is at " <<  center_of_line << '\n';
    int plant_top = highestGreenCrossingTheLine(center_of_line,hst_merged,dst);
    if(plant_top==-1) {
        cout << " Failed to find top of the plant\n";
        exit(1);
    }
    cout << "Plant top at " <<  plant_top << '\n';
    float pixel_to_mm = (3.0f/9.0f);// 3mm is 9 pixels
    int thirty_centimeters_down=300.0f/pixel_to_mm;
    Rect selected_area(0,plant_top+thirty_centimeters_down-50,hst_merged.cols,100);
    trimToGreen(selected_area,hst_merged);
    stemWidth(pic(selected_area));
    //    cv::imwrite("edg.png",dstcopy(selected_area));
    //    hst_merged.setTo(0,hsv_mask);
    //    cv::imwrite("top.png",hst_merged);
    return 0;
}
int highestGreenCrossingTheLine(int line_center, const Mat &pic, const Mat &edges) {
    // just walk down until green is encountered
    int y = 0;
    int x = line_center;
    while(!greenAt(pic,x,y) && y<pic.rows) {
        y++;
    }
    if(y==pic.rows)
        return -1;
    return y;
    //    double minval,maxVal;
    //    Mat result1 = Mat::zeros(1,pic.cols,pic.type());
    //    Mat resul = pic.rowRange(0,3),masking=pic.rowRange(0,3);
    //    Mat hsl;
    //    vector<Mat> split_planes;
    //    cvtColor(resul,hsl,CV_RGB2HSV_FULL);
    //    split(hsl,split_planes);
    //    minMaxLoc(split_planes[0],&minval,&maxVal);
    //    resul = pic.rowRange(0,3);
    //    resul.setTo(0,(split_planes[2]<200) | (split_planes[1]>5));
    //    for(int x=0; x<pic.rows/3; ++x) {
    //        result1.push_back(resul);
    //    }
    //    cv::imwrite("top.png",result1);
}
