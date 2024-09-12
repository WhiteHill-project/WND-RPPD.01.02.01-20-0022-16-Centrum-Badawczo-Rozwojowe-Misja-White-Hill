#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <QtCore/QCoreApplication>
#include "opencv2/highgui/highgui.hpp"

extern void testCV(int argc,char **argv);
int main(int argc, char* argv[])
{
    cv::setNumThreads(2);
    QCoreApplication app(argc, argv);
    testCV(argc,argv);
    return app.exec();
}
