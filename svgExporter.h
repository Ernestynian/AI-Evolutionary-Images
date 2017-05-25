#ifndef SVGEXPORTER_H
#define SVGEXPORTER_H

#include <opencv2/core/types.hpp>
#include <iostream>

void saveTrianglesToSvg(const char* name, Point2i** points, Scalar* colors, int triangleCount, int cols, int rows) {
    // open file
    std::ofstream outputFile;
    
    outputFile.open(name);
    // write beginning
    outputFile << "<svg viewBox=\"0 0 " << cols << " " << rows << "\">\n";
    // write trangles
    for(int i = 0; i < triangleCount; i++) {
        outputFile << "<path style=\"fill:rgb("
                << (int)(colors[i][0]*255)<<","
                << (int)(colors[i][1]*255)<<","
                << (int)(colors[i][2]*255) 
                << ");fill-rule:evenodd;stroke:none;opacity:" 
                << colors[i][3] << "\"\n"
                << "d=\"M "
				<< points[i][0] << "," 
                << points[i][0].y
                << " " 
                << points[i][1].x << "," 
                << points[i][1].y
                << " " 
                << points[i][2] << "," 
                << points[i][2]
                << " Z\"/>\n";
    }
    //write ending
    outputFile << "</svg>\n";
    outputFile.close();
}

#endif /* SVGEXPORTER_H */

