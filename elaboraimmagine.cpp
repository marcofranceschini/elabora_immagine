/* 
 * Da riga di comando 
 * elaboraimmagine <fileinput> <comando> <dimensioni_x> <dimensioni_y> <fileoutput>
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>

using namespace std;

// header per file BitMap monocromatici
int Wsorgente = 0;
int Hsorgente = 0;
char* FIRMAsorgenre = new char[2];  // Vettore contenente la firma del file in input
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
    
    // controllo parametri
    if (!fileinput || !comando || dimensioni_x < 0 || dimensioni_y < 0 || !fileoutput) {
        cout << "Errore: mancano alcuni parametri, sintassi:\n";
        cout << "elaboraimmagine <fileinput> <comando> <dimensioni_x> <dimensioni_y> <fileoutput>\n";
        return 0;
    }
    
    // controllo comando
    if (strcmp(comando, "blur") != 0 && strcmp(comando, "sharpen") != 0) {
    	cout << "Errore: comando \"" << comando << "\" non riconosciuto\n";
        return 0;
    }
    
    // controllo dimensioni
    if (dimensioni_x < 0 || dimensioni_y < 0) {
    	cout << "Errore: dimensioni errate\n";
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
     
    if (!caricaBmp(fileinput)) {
        cout << "Errore: il file inserito non corrisponde ad un'immagine BMP\n";
        return 0;
    } else {
    	cout << "---------------------------------------------\n";
	    cout << "LETTURA FILE\n";
	    cout << "---------------------------------------------\n";    
	    cout << " - Firma:\t\t\t" << FIRMAsorgenre[0] << FIRMAsorgenre[1] << endl;
    	cout << " - Dimensione File:\t\t" << DIMsorgente << " byte" << endl;
    	cout << " - Offset:\t\t\t" << OFFSETsorgente << " byte" << endl;
    	cout << " - Larghezza:\t\t\t" << Wsorgente << " pixel" << endl;
    	cout << " - Altezza:\t\t\t" << Hsorgente << " pixel" << endl;
    	cout << " - Profonfita' colore:\t\t" << DEPTHsorgente << " bit" << endl;
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
    }
}

bool caricaBmp(const char* path) {
	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		cout << "\nErrore: impossibile caricare il file\n";
		exit(1);
	}
    fseek(f, 0, 0);
    fread(FIRMAsorgenre, 2, 1, f);
    
    // controllo file BMP
    if (FIRMAsorgenre[0] != 'B' && FIRMAsorgenre[1] != 'M')
	    return false;
        
    // lettura informazioni immagine
    fseek(f,2,0);
    fread(&DIMsorgente, 1, sizeof(int), f);
    fseek(f, 10, 0);
    fread(&OFFSETsorgente, 1, sizeof(int), f);
    fseek(f, 18, 0);
    fread(&Wsorgente, 1, sizeof(int), f);
    fread(&Hsorgente, 1, sizeof(int), f);
    fseek(f, 28, 0);
    fread(&DEPTHsorgente, 1, sizeof(int), f);
    fseek(f, 30, 0);
    fread(&COMPRsorgente, 1, sizeof(int), f);
	
    header = new char[OFFSETsorgente];
	sorgente = new char[Wsorgente * Hsorgente];
	fread(header, OFFSETsorgente, 1, f);
	fread(sorgente, Wsorgente * Hsorgente, 1, f);
	fclose(f);
	
	return true;
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