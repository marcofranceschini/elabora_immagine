/* 
 * Da riga di comando 
 * elaboraimmagine <fileinput> <comando> <dimensioni_x> <dimensioni_y> <fileoutput>
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

#define LARG	512
#define LP2		9

// immagine sorgente
unsigned char ImmagineS[LARG * LARG];

// immagine destinazione
unsigned char ImmagineD[LARG * LARG];

// header per file BitMap monocromatici
#define DIM_HEAD_BMP 1078

unsigned char Header[DIM_HEAD_BMP];

static void caricaBmp(const char *Nome, unsigned char *Dove, int x, int y);
static void salvaBmp(const char *Nome, unsigned char *DaDove, int x, int y);

int main(int argc, char* argv[]) {
    
    char* fileinput = argv[1];
    char* comando = argv[2];
    double dimensioni_x = atof(argv[3]);
    double dimensioni_y = atof(argv[4]);
    char* fileoutput = argv[5];
    
    if (!fileinput || !comando || dimensioni_x < 0 || dimensioni_y < 0 || !fileoutput) {
        cout << "Errore: mancano alcuni parametri\n";
        return 0;
    }
    
    cout    << "fileinput: " << fileinput << "\n"
            << "comando: " << comando << "\n"
            << "dimensioni_x: " << dimensioni_x << "\n"
            << "dimensioni_y: " << dimensioni_y << "\n"
            << "fileoutput: " << fileoutput << "\n";
            
    
}

static void caricaBmp(const char *Nome, unsigned char *Dove, int x, int y) {
	FILE *fHan = fopen(Nome, "rb");
	if(fHan == NULL) {
		printf("errore!\n");
		exit(1);
	}

	fread(Header, DIM_HEAD_BMP, 1, fHan);
	fread(Dove, x * y, 1, fHan);
	fclose(fHan);
}

static void salvaBmp(const char *Nome, unsigned char *DaDove, int x, int y) {
	FILE *fHan = fopen(Nome, "wb");
	if(fHan == NULL) {
		printf("errore!\n");
		exit(1);
	}

	fwrite(Header, DIM_HEAD_BMP, 1, fHan);
	fwrite(DaDove, x * y, 1, fHan);
	fclose(fHan);
}