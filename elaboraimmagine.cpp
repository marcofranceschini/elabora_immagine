/* 
* Progetto di Sistemi Multimediali
*	Progetto 1 - Elaborazione di Immagine
*
* Componenti del gruppo:
*	Daniele Favaro 1069357
*	Marco Franceschini 
*	Fabiano Tavallini 1069358
*
* Anno Accademico:
*	2015/2016
*
* Esecuzione da riga di comando: 
*	elaboraimmagine <fileinput> <comando> <dimensioni_x> <dimensioni_y> <fileoutput>
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>

using namespace std;

// informazioni header per file BitMap
int Wsorgente = 0;					// larghezza
int Hsorgente = 0;					// altezza
char* FIRMAsorgente = new char[2];  // Vettore contenente la firma del file in input
int DIMsorgente = 0;				// dimensione file
int OFFSETsorgente = 0;				// dimensione totale header
int INFOHEADERsorgente = 0;			// dimensione blocco informazioni
int DEPTHsorgente = 0;				// profondita' (codifica) colore
int COMPRsorgente = 0;				// metodo di compressione
int BITMAPsorgente = 0;				// dimensione mappa dei pixel, puo' essere 0 se non vi e' compressione
int PALETTECOLORSsorgente = 0;		// numero di colori definiti nella palette

// informazioni sezioni file (header di destinazione? e' lo stesso?)
char* header = new char[OFFSETsorgente];
char* sorgente = new char[Wsorgente * Hsorgente];
char* destinazione = new char[Wsorgente * Hsorgente];
unsigned char palette[1024]; // 4 byte x 256 colori

// headers funzoni
void stampaInfo();
bool caricaBmp(const char* path);
//void salvaBmp(const char* path, char* d, int x, int y);

int main(int argc, char* argv[]) {
    
    // recupero parametri in input
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
    
    /*
	cout << "---------------------------------------------\n";
    cout << "INPUT\n";
    cout << "---------------------------------------------\n";
    cout    << " - fileinput:\t\t" << fileinput << "\n"
            << " - comando:\t\t" << comando << "\n"
            << " - dimensioni_x:\t" << dimensioni_x << "\n"
            << " - dimensioni_y:\t" << dimensioni_y << "\n"
            << " - fileoutput:\t\t" << fileoutput << "\n\n";
    */
    
    // controllo corretto caricamento in formato BMP
    if (!caricaBmp(fileinput)) {
        cout << "Errore: il file inserito non corrisponde ad un'immagine BMP\n";
        return 0;
    }
    
    stampaInfo();
}

// stampa le informazioni lette dal file
void stampaInfo() {
	cout << "---------------------------------------------\n";
    cout << "LETTURA FILE\n";
    cout << "---------------------------------------------\n";    
    cout << " - Firma:\t\t\t" << FIRMAsorgente[0] << FIRMAsorgente[1] << endl;
    float fl = DIMsorgente;		// conversione a float
    float megabyte = (fl/1024)/1024;
    megabyte = roundf(megabyte * 100)/100;
    float kilobyte = fl/1024;
    kilobyte = roundf(kilobyte * 100)/100;
    if (megabyte > 1)
    	cout << " - Dimensione File:\t\t" << megabyte << " MB" << endl;
	else if (kilobyte > 1)
		cout << " - Dimensione File:\t\t" << kilobyte << " KB" << endl;
	else
		cout << " - Dimensione File:\t\t" << DIMsorgente << " byte" << endl;
	cout << " - Offset:\t\t\t" << OFFSETsorgente << " byte" << endl;
	if (INFOHEADERsorgente == 12)
		cout << " - Formato BMP:\t\t\t" <<  "Versione 2" << endl;
	else if (INFOHEADERsorgente == 40)
		cout << " - Formato BMP:\t\t\t" <<  "Versione 3" << endl;
	else if (INFOHEADERsorgente == 108)
		cout << " - Formato BMP:\t\t\t" <<  "Versione 4" << endl;
	else if (INFOHEADERsorgente == 124)
		cout << " - Formato BMP:\t\t\t" <<  "Versione 5" << endl;
	cout << " - Larghezza:\t\t\t" << Wsorgente << " pixel" << endl;
	cout << " - Altezza:\t\t\t" << Hsorgente << " pixel" << endl;
	cout << " - Profonfita' colore:\t\t" << DEPTHsorgente << " bit" << endl;
	if (COMPRsorgente == 0)
        cout << " - Compressione:\t\t" << "Nessuna" << endl;
	else if (COMPRsorgente == 1)
	    cout << " - Compressione:\t\t" << "RLE 8 Bit" << endl;
	else if (COMPRsorgente == 2)
	    cout << " - Compressione:\t\t" << "RLE 4 Bit" << endl;
	else if (COMPRsorgente == 3)
        cout << " - Compressione:\t\t" << "Bitfields" << endl;
	else
        cout << " - Compressione:\t\t" << COMPRsorgente << endl;
    if (BITMAPsorgente > 0)
    	cout << " - Dimensione BitMap:\t\t" << BITMAPsorgente << " byte" << endl;
    else
    	cout << " - Dimensione BitMap:\t\t" << "Non definita" << endl;
    int dim_headers = INFOHEADERsorgente+14;
	if (dim_headers == OFFSETsorgente) {
		cout << " - Palette:\t\t\t" << "Non presente" << endl;;
	} else {
		if (PALETTECOLORSsorgente != 0)
			cout << " - Palette:\t\t\t" << PALETTECOLORSsorgente << " colori" << endl;
		else
			cout << " - Palette:\t\t\t" << "Usa tutti i colori" << endl;
		/*
    	for (int n=0, j=0; n<(OFFSETsorgente-dim_headers)/4; n++) {
        	printf("%02X^col. :%02X",n, palette[j++]);
        	printf("%02X",palette[j++]);
        	printf("%02X | ",palette[j++]);
        	if (((n+1)%4) == 0)
        		printf("\n");
        	j++;
        }
        cout << endl;
    	*/
	}
}

// apre il file e carica le informazioni nelle variabili globali
bool caricaBmp(const char* path) {
	
	// apertura file
	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		cout << "\nErrore: impossibile caricare il file\n";
		exit(1);
	}
	
	// controllo file BMP
    fseek(f, 0, 0);
    fread(FIRMAsorgente, 2, 1, f);
    if (FIRMAsorgente[0] != 'B' && FIRMAsorgente[1] != 'M')
	    return false;
        
    // lettura informazioni immagine
    fseek(f,2,0);
    fread(&DIMsorgente, 1, sizeof(int), f);
    fseek(f, 10, 0);
    fread(&OFFSETsorgente, 1, sizeof(int), f);
    fseek(f, 14, 0);
    fread(&INFOHEADERsorgente, 1, sizeof(int), f);
    fseek(f, 18, 0);
    fread(&Wsorgente, 1, sizeof(int), f);
    fread(&Hsorgente, 1, sizeof(int), f);
    fseek(f, 28, 0);
    fread(&DEPTHsorgente, 1, sizeof(short), f);
    fseek(f, 30, 0);
    fread(&COMPRsorgente, 1, sizeof(int), f);
    fseek(f, 34, 0);
    fread(&BITMAPsorgente, 1, sizeof(int), f);
    fseek(f, 46, 0);
    fread(&PALETTECOLORSsorgente, 1, sizeof(int), f);
    
    // lettura palette colori (opzionale)
    int dim_headers = INFOHEADERsorgente+14;
    if (dim_headers != OFFSETsorgente) {
    	fseek(f, dim_headers, 0);
        fread(palette, sizeof(unsigned char), OFFSETsorgente-dim_headers, f);
    }
	
	// lettura sezioni header e bitmap immagine
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