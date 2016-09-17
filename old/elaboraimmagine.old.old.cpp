/* 
* Progetto di Sistemi Multimediali
*	Progetto 1 - Elaborazione di Immagine
*
* Componenti del gruppo:
*	Daniele Favaro 1069357
*	Marco Franceschini 1072066 
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
#include "FFT.cpp"

using namespace std;

// informazioni header per file BitMap
int Wsorgente = 0;					// larghezza sorgente
int Hsorgente = 0;					// altezza sorgente
int Wdestinazione = 0;				// larghezza destinazione
int Hdestinazione = 0;				// altezza destinazione
char* FIRMAsorgente = new char[2];  // Vettore contenente la firma del file in input
int DIMsorgente = 0;				// dimensione file
int OFFSETsorgente = 0;				// dimensione totale header
int INFOHEADERsorgente = 0;			// dimensione blocco informazioni
int DEPTHsorgente = 0;				// profondita' (codifica) colore
int COMPRsorgente = 0;				// metodo di compressione
int BITMAPsorgente = 0;				// dimensione mappa dei pixel, puo' essere 0 se non vi e' compressione
int PALETTECOLORSsorgente = 0;		// numero di colori definiti nella palette


// informazioni sezioni file (header di destinazione? e' lo stesso?)
unsigned char* header;
unsigned char* sorgente;
unsigned char* destinazione;
unsigned char palette[1024]; // 4 byte x 256 colori
float* buffer;
float* temp;
float* rowR;
float* rowI;
int numMaggico = 9;



// kernel convoluzione
#define dimKernel	3
#define OFS	((dimKernel - 1) / 2)

//Sharpen
/*int kernel[dimKernel * dimKernel] = {
     0, -1, 0,
     -1, 5, -1,
     0, -1, 0
};*/

//Blur
float kernel[dimKernel * dimKernel] = {
	0.0625, 0.125, 0.0625,
	0.125, 0.25, 0.125,
	0.0625, 0.125, 0.0625
};

// headers funzoni
void stampaInfo();
bool caricaBmp(const char* path);
void salvaBmp(const char* path);
void caricaInBuffer(int canale);
void convoluzione();
void salvaDalBuffer(int canale);
int applicaKernel(int x, int y, int c);
void applicaFiltro(int *filtro, int c);
void trasformataFourier(int direzione);
void blur(int lv);
void sharpen(int lv);
void FiltroEsaltaAlto(int lv);
float Campana(int x, int y, int lv);

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
    
    Wdestinazione = dimensioni_x;
    Hdestinazione = dimensioni_y;
    
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
    
    rowR = new float[Wsorgente];
    rowI = new float[Wsorgente];
	temp = new float[Wsorgente * Hsorgente * 2];
    buffer = new float[Wsorgente * Hsorgente * 2];
	destinazione = new unsigned char[Wdestinazione * Hdestinazione * 3];
	sorgente = new unsigned char[Wsorgente * Hsorgente];

    convoluzione();
  
	//applicaFiltro(KernelIdentita);
	// applicaFiltro(KernelSharpen);
    
    salvaBmp(fileoutput);
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
    fseek(f, 2, 0);
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
    int dim_headers = INFOHEADERsorgente + 14;
    if (dim_headers != OFFSETsorgente) {
    	fseek(f, dim_headers, 0);
        fread(palette, sizeof(unsigned char), OFFSETsorgente - dim_headers, f);
    }
	
	// lettura sezioni header e bitmap immagine
	fseek(f, 0, 0);
	header = new unsigned char[OFFSETsorgente];
	//sorgente = new unsigned char[Wsorgente * Hsorgente * 3];
	fread(header, OFFSETsorgente, 1, f);
	fread(sorgente, Wsorgente * Hsorgente * 3, 1, f);
	// fread(sorgente, Wsorgente * Hsorgente, 1, f);
	
	fclose(f);
	return true;
}


void salvaBmp(const char *path) {
	FILE *f = fopen(path, "wb");
	if(f == NULL) {
		cout << "\nErrore: impossibile salvare il file\n";
		exit(1);
	}
	fwrite(header, OFFSETsorgente, 1, f);
	fwrite(destinazione, Wdestinazione * Hdestinazione * 3, 1, f);
	fclose(f);
}


/*void caricaInBuffer(int c) {
    float rgb;
	for (int y = 0; y < Hsorgente; y++) {
		for (int x = 0; x < Wsorgente; x++) {
			rgb = sorgente[x + (y * Wsorgente) + (c * Wsorgente * Hsorgente)];
			if (((y + x) & 1) == 0)
			    buffer[x + (y * Wsorgente)] = rgb;
			else 
			    buffer[x + (y * Wsorgente)] = -rgb;
			buffer[x + (y * Wsorgente) + (Wsorgente * Hsorgente)] = 0;
		}
	}
}*/

void convoluzione() {
	for (int c = 0; c < 3; c++) {
		for (int y = 0; y < Hsorgente; y++) {
			for (int x = 0; x < Wdestinazione; x++) {
				destinazione[x * 3 + (y * Wdestinazione * 3) + c] = applicaKernel(x, y, c);
			}
		}
	}
}

void salvaDalBuffer(int c) {
	int u;
	for (int y = 0; y < Hsorgente; y++) {
		for (int x = 0; x < Wsorgente; x++) {
			if(((y + x) & 1) == 0) 
			    u = buffer[x + (y * Wsorgente)];
			else 
			    u = -buffer[x + (y * Wsorgente)];
			if(u < 0)
			    u = 0;
			if(u > 255)
			    u = 255;
			destinazione[x + (y * Wsorgente) + (c * Wsorgente * Hsorgente)] = u;
		}
	}
}


int Scala = 1;

int applicaKernel(int x, int y, int c) {
	int rgb;
	int pixel = 0;

	for (int v = -OFS; v <= OFS; v++) {
		if ((y + v) < 0 || Hsorgente <= (y + v)) continue;
		for (int u = -OFS; u <= OFS; u++) {
			if ((x + u) * 3 < 0 || Wsorgente * 3 <= (x + u) * 3) continue;
			rgb = sorgente[(x + u) * 3 + ((y + v) * Wsorgente * 3) + c];
			pixel += (rgb * kernel[u + OFS + ((v + OFS) * dimKernel)]);
		}
	}

	pixel /= Scala;

	if(pixel < 0)
		pixel = 0;
	if(pixel > 255)
		pixel = 255;

	return(pixel);
}

/*void applicaFiltro(int *filtro, int c) {
	// for(int c = 0; c < 3; c ++) {
		for(int y = 0; y < Hsorgente; y++) {
			for(int x = 0; x < Wsorgente; x++) {
				destinazione[x + (y * Wsorgente) + (c * Wsorgente * Hsorgente)] = applicaKernel(x, y, c, filtro);	// dimensioni di destinazione
			}
		}
	// }
}

void trasformataFourier(int direzione) {
	// Righe
	for (int y = 0; y < Hsorgente; y++) {
		for (int x = 0; x < Wsorgente; x++) {
			rowR[x] = buffer[x + (y * Wsorgente)];
			rowI[x] = buffer[x + (y * Wsorgente) + (Wsorgente * Hsorgente)];
		}
		FFT(direzione, numMaggico, rowR, rowI);
		for (int x = 0; x < Wsorgente; x++) {
			temp[x + (y * Wsorgente)] = rowR[x];
			temp[x + (y * Wsorgente) + (Wsorgente * Hsorgente)] = rowI[x];
		}
	}

	// Colonne
	for (int x = 0; x < Wsorgente; x++) {
		for (int y = 0; y < Hsorgente; y++) {
			rowR[y] = temp[x + (y * Wsorgente)];
			rowI[y] = temp[x + (y * Wsorgente) + (Wsorgente * Hsorgente)];
		}
		FFT(direzione, numMaggico, rowR, rowI);
		for (int y = 0; y < Hsorgente; y++) {
			buffer[x + (y * Wsorgente)] = rowR[y];
			buffer[x + (y * Wsorgente) + (Wsorgente * Hsorgente)] = rowI[y];
		}
	}
}*/

/*
void blur(int lv) {
	int x, y;
	int dx, dy;
	float l;

	for(y = 0;y < Hsorgente;y++) {
		for(x = 0;x < Wsorgente;x++) {
			l = Campana(x, y, lv);
			buffer[x + (y * Wsorgente)] *= l;
			buffer[x + (y * Wsorgente) + (Wsorgente * Hsorgente)] *= l;
		}
	}
}

void sharpen(int lv) {
	int x, y;

	for(y = 0;y < Hsorgente;y++) {
		for(x = 0;x < Wsorgente;x++) {
			buffer[x + (y * Wsorgente)] *= 3 - 2 * Campana(x, y, lv);
			buffer[x + (y * Wsorgente) + (Wsorgente * Hsorgente)] *= 3 - 2 * Campana(x, y, lv);
		}
	}

}

float Campana(int x, int y, int lv) {
	float d;

	x -= Wsorgente / 2;
	y -= Hsorgente / 2;

	d = sqrt(x * x + y * y);

	return(exp(-d / lv));
}*/
