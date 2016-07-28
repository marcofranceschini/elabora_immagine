/* 
 * Da riga di comando 
 * elaboraimmagine <fileinput> <comando> <dimensioni_x> <dimensioni_y> <fileoutput>
*/

#include <stdlib.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
    
    char* fileinput = argv[1];
    char* comando = argv[2];
    double dimensioni_x = atof(argv[3]);
    double dimensioni_y = atof(argv[4]);
    char* fileoutput = argv[5];
    
    if (!fileinput || !comando || dimensioni_x < 0 || dimensioni_y < 0 || !fileoutput) {
        cout << "missing params\n";
        return 0;
    }
    
    cout    << "fileinput: " << fileinput << "\n"
            << "comando: " << comando << "\n"
            << "dimensioni_x: " << dimensioni_x << "\n"
            << "dimensioni_y: " << dimensioni_y << "\n"
            << "fileoutput: " << fileoutput << "\n";
            
    
    
    
    
    
}