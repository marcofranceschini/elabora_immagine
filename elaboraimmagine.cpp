/* 
 * Da riga di comando 
 * elaboraimmagine <fileinput> <comando> <dimensioni_x> <dimensioni_y> <fileoutput>
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

// header per file BitMap monocromatici
int Wsorgente = 0;
int Hsorgente = 0;
int DIMsorgente = 0;
int OFFSETsorgente = 0;
int DEPTHsorgente = 0;
int COMPRsorgente = 0;

char* sorgente = new char[Wsorgente * Hsorgente];
char* destinazione = new char[Wsorgente * Hsorgente];

char* header = new char[OFFSETsorgente];

bool caricaBmp(const char* path);
//void salvaBmp(const char* path, char* d, int x, int y);

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
    
    cout << "---------------------------------------------\n";
    cout << "INPUT\n";
    cout << "---------------------------------------------\n";
    cout    << " - fileinput:\t\t" << fileinput << "\n"
            << " - comando:\t\t" << comando << "\n"
            << " - dimensioni_x:\t" << dimensioni_x << "\n"
            << " - dimensioni_y:\t" << dimensioni_y << "\n"
            << " - fileoutput:\t\t" << fileoutput << "\n\n";
     
    cout << "---------------------------------------------\n";
    cout << "LETTURA FILE\n";
    cout << "---------------------------------------------\n";       
    if (!caricaBmp(fileinput)) {
        cout << "\n---------------------------------------------\n";
        cout << "ERRORE\n";
        cout << "---------------------------------------------\n";
        cout << "Il file inserito non corrisponde ad una immagine BMP\n";
        return 0;
    }
}

bool caricaBmp(const char* path) {
	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		cout << "\nErrore durante il caricamento!\n";
		exit(1);
	}
	
	char* firma = new char[2];  // Vettore contenente la firma del file in input
	
    fseek(f, 0, 0);
    fread(firma, 2, 1, f);
    cout << " - Firma:\t\t\t" << firma[0] << firma[1] << endl;
    
    fseek(f,2,0);
    fread(&DIMsorgente, 1, sizeof(int), f);
    cout << " - Dimensione File:\t\t" << DIMsorgente << " byte" << endl;
    
    fseek(f, 10, 0);
    fread(&OFFSETsorgente, 1, sizeof(int), f);
    cout << " - Offset:\t\t\t" << OFFSETsorgente << " byte" << endl;
    header = new char[OFFSETsorgente];
    
    fseek(f, 18, 0);
    fread(&Wsorgente, 1, sizeof(int), f);
    cout << " - Larghezza:\t\t\t" << Wsorgente << " pixel" << endl;
    fread(&Hsorgente, 1, sizeof(int), f);
    cout << " - Altezza:\t\t\t" << Hsorgente << " pixel" << endl;
    
    fseek(f, 28, 0);
    fread(&DEPTHsorgente, 1, sizeof(int), f);
    cout << " - Profonfita' colore:\t\t" << DEPTHsorgente << " bit" << endl;
    
    fseek(f, 30, 0);
    fread(&COMPRsorgente, 1, sizeof(int), f);
    if (COMPRsorgente==0)
        cout << " - Compressione:\t\t" << "Nessuna" << endl;
	else if (COMPRsorgente==1)
	    cout << " - Compressione:\t\t" << "RLE 8 Bit" << endl;
	else if (COMPRsorgente==2)
	    cout << " - Compressione:\t\t" << "RLE 4 Bit" << endl;
	else if (COMPRsorgente==3)
        cout << " - Compressione:\t\t" << "Bitfields" << endl;
	else
        cout << " - Compressione:\t\t" << COMPRsorgente << endl;
	
	sorgente = new char[Wsorgente * Hsorgente];
	fread(header, OFFSETsorgente, 1, f);
	fread(sorgente, Wsorgente * Hsorgente, 1, f);
	fclose(f);
	
	if (firma[0] == 'B' && firma[1] == 'M')
	    return true;
    else 
        return false;
}

/*void salvaBmp(const char *path, unsigned char *d, int x, int y) {
	FILE *f = fopen(path, "wb");
	if(f == NULL) {
		cout << "\nErrore durante il salvataggio!\n";
		exit(1);
	}
	fwrite(header, DIM_HEAD_BMP, 1, f);
	fwrite(d, x * y, 1, f);
	fclose(f);
}*/