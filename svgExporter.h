#ifndef SVGEXPORTER_H
#define SVGEXPORTER_H

#include <opencv2/core/types.hpp>
#include <iostream>

void saveTrianglesToSvg(char* name, Point2f** points, Scalar* colors, int triangleCount, int cols, int rows) {
    //open file
    std::ofstream outputFile;
    
    std::cout <<"test\n";
    outputFile.open(name);
    //write beginning
    outputFile << "<svg viewBox=\"0 0 " 
            << cols << " " << rows << "\">\n";
    //write trangles
    std::cout <<((points[0][0].x)*cols);
    for(int i = 0; i < triangleCount; i++) {
        outputFile << "<path style=\"fill:rgb("
                << (int)(colors[i][0]*255)<<","
                << (int)(colors[i][1]*255)<<","
                << (int)(colors[i][2]*255) 
                <<");fill-rule:evenodd;stroke:none;opacity:" 
                << colors[i][3] << "\"\n"
                << "d=\"M "<< ((points[i][0].x+1)*cols/2) << "," 
                << ((points[i][0].y+1) *rows/2)
                << " " 
                << ((points[i][1].x+1)*cols/2) << "," 
                << ((points[i][1].y +1)*rows/2)
                << " " 
                << ((points[i][2].x+1)*cols/2) << "," 
                << ((points[i][2].y +1)*rows/2)
                << " Z\"/>\n";
    }
    //write ending
    outputFile << "</svg>\n";
    outputFile.close();
}

#endif /* SVGEXPORTER_H */

