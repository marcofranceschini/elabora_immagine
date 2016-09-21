/* 
* Progetto di Sistemi Multimediali
* 
* Progetto 1 - Elaborazione di Immagine
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
*	elaboraimmagine <fileinput> <filtro> <dimensioni_x> <dimensioni_y> <fileoutput>
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>

using namespace std;

// informazioni header per file BitMap
int Wsorgente = 0;					// larghezza sorgente
int Hsorgente = 0;					// altezza sorgente
int Wdestinazione = 0;				// larghezza destinazione
int Hdestinazione = 0;				// altezza destinazione
int DIMsorgente = 0;				// dimensione file
int OFFSETsorgente = 0;				// dimensione totale header
int INFOHEADERsorgente = 0;			// dimensione blocco informazioni
int DEPTHsorgente = 0;				// profondita' (codifica) colore
int COMPRsorgente = 0;				// metodo di compressione
int BITMAPsorgente = 0;				// dimensione mappa dei pixel, puo' essere 0 se non vi e' compressione
int PALETTECOLORSsorgente = 0;		// numero di colori definiti nella palette
char* FIRMAsorgente = new char[2];  // Vettore contenente la firma del file in input

// informazioni sezioni file (header di destinazione? e' lo stesso?)
unsigned char* header;
unsigned char* sorgente;		// spazio di memoria per l'immagine iniziale
unsigned char* buffer;			// spazio di memoria per l'immagine filtrata
unsigned char* destinazione;	// spazio di memoria per l'immagine filtrata ed eventualmente riscalata
unsigned char palette[1024]; // 4 byte x 256 colori

// kernel
#define dimKernel 3
#define OFS	((dimKernel - 1) / 2)

//Sharpen
int sharpen[dimKernel * dimKernel] = {
     0, -1, 0,
     -1, 5, -1,
     0, -1, 0
};

//Blur
float blur[dimKernel * dimKernel] = {
	0.0625, 0.125, 0.0625,
	0.125, 0.25, 0.125,
	0.0625, 0.125, 0.0625
};

//Outline
float outline[dimKernel * dimKernel] = {
	-1, -1, -1,
	-1, 8, -1,
	-1, -1, -1
};

//Emboss
float emboss[dimKernel * dimKernel] = {
	-2, -1, 0,
	-1, 1, 1,
	0, 1, 2
};

//Identity
float identity[dimKernel * dimKernel] = {
	0, 0, 0,
	0, 1, 0,
	0, 0, 0
};

float kernel[dimKernel * dimKernel];

// header funzoni
static bool caricaBmp(const char* path);
void stampaInfo();
void convoluzione();
int applicaKernel(int x, int y, int c);
int interpolazione(float x, float y, int c);
void ridimensiona();
static void salvaBmp(const char *path);


int main(int argc, char* argv[]) {
	
	// Recupero parametri in input
    char* fileinput = argv[1];
    char* comando = argv[2];
    double dimensioni_x = atof(argv[3]);
    double dimensioni_y = atof(argv[4]);
    char* fileoutput = argv[5];
    
    // Controllo parametri
    if (!fileinput || !comando || dimensioni_x < 0 || dimensioni_y < 0 || !fileoutput) {
        cout << "Errore: mancano alcuni parametri, sintassi:\n";
        cout << "elaboraimmagine <fileinput> <comando> <dimensioni_x> <dimensioni_y> <fileoutput>\n";
        return 0;
    }
    
    // Controllo comando
    if (strcmp(comando, "blur") != 0 && strcmp(comando, "sharpen") != 0 && strcmp(comando, "outline") != 0 && strcmp(comando, "emboss") != 0 && strcmp(comando, "identity") != 0) {
    	cout << "Errore: comando \"" << comando << "\" non riconosciuto\n";
        return 0;
    }
    
    // Controllo dimensioni della destinazione
    if (dimensioni_x < 0 || dimensioni_y < 0) {
    	cout << "Errore: dimensioni errate\n";
        return 0;
    }
    
    Wdestinazione = dimensioni_x;
    Hdestinazione = dimensioni_y;
    
	if (!caricaBmp(fileinput)) {
        cout << "Errore: il file inserito non corrisponde ad un'immagine BMP\n";
        return 0;
    }
    
    stampaInfo();	// Stampa le informazioni dell'header
    
	// Applica il filtro
	buffer = new unsigned char[Wsorgente * Hsorgente * 3];
	if(strcmp(comando, "blur") == 0) {
		std::copy(blur, blur + sizeof(blur), kernel);
	}
	if(strcmp(comando, "sharpen") == 0) {
		std::copy(sharpen, sharpen + sizeof(sharpen), kernel);
	}
	if(strcmp(comando, "outline") == 0) {
		std::copy(outline, outline + sizeof(outline), kernel);
	}
	if(strcmp(comando, "emboss") == 0) {
		std::copy(emboss, emboss + sizeof(emboss), kernel);
	}
	if(strcmp(comando, "identity") == 0) {
		std::copy(identity, identity + sizeof(identity), kernel);
	}
	convoluzione();

	// Riscala l'immagine
	destinazione = new unsigned char[Wdestinazione * Hdestinazione * 3];
	ridimensiona();

	// Scrive output
	salvaBmp(fileoutput);

	return 0;
}

static bool caricaBmp(const char* path) {
	// Apertura file
	FILE *f = fopen(path, "rb");
	if(f == NULL) {
		cout << "\nErrore: impossibile caricare il file\n";
		exit(1);
	}
	
	// Controllo file BMP
    fseek(f, 0, 0);
    fread(FIRMAsorgente, 2, 1, f);
    if (FIRMAsorgente[0] != 'B' && FIRMAsorgente[1] != 'M')
	    return false;
        
    // Lettura informazioni immagine
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
    
    // Lettura palette colori (opzionale)
    int dim_headers = INFOHEADERsorgente + 14;
    if (dim_headers != OFFSETsorgente) {
    	fseek(f, dim_headers, 0);
        fread(palette, sizeof(unsigned char), OFFSETsorgente - dim_headers, f);
    }

	// Lettura sezioni header e bitmap immagine
	fseek(f, 0, 0);
    header = new unsigned char[OFFSETsorgente];
	sorgente = new unsigned char[Wsorgente * Hsorgente * 3];
	fread(header, OFFSETsorgente, 1, f);
	fread(sorgente, Wsorgente * Hsorgente * 3, 1, f);
	
	fclose(f);
	return true;
}

// Stampa le informazioni lette dal file
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
	}
}

void convoluzione() {
	for (int c = 0; c < 3; c++)
		for (int y = 0; y < Hsorgente; y++)
			for (int x = 0; x < Wsorgente; x++)
				buffer[x * 3 + (y * Wsorgente * 3) + c] = applicaKernel(x, y, c);
}

int applicaKernel(int x, int y, int c) {
	int rgb;
	int pixel = 0;

	for (int v = -OFS; v <= OFS; v++) {
		if ((y + v) < 0 || Hsorgente <= (y + v)) continue;
		for (int u = -OFS; u <= OFS; u++) {
			if ((x + u) * 3 < 0 || (3 * Wsorgente) <= (x + u) * 3) continue;
			rgb = sorgente[(x + u) * 3 + ((y + v) * Wsorgente * 3) + c];
			pixel += (rgb * kernel[u + OFS + ((v + OFS) * dimKernel)]);
		}
	}

	if (pixel < 0) pixel = 0;
	if (pixel > 255) pixel = 255;
	
	return(pixel);
}

void ridimensiona() {
	int pixel;

	for (int c = 0; c < 3; c++) {
		for (int y = 0; y < Hdestinazione; y++) {
			for (int x = 0; x < Wdestinazione; x++) {
				float scalex = ((float) Wsorgente / Wdestinazione) * x;
				float scaley = ((float) Hsorgente / Hdestinazione) * y;
				pixel = interpolazione(scalex, scaley, c);
				if (pixel > 255) pixel = 255;
				if (pixel < 0) pixel = 0;
				destinazione[x * 3 + (y * Wdestinazione * 3) + c] = pixel;
			}
		}
	}
}

int interpolazione(float x, float y, int c) {
	float v1, v2, v3, v4;
	int X = (int) x;
	int Y = (int) y;

	x -= X;
	y -= Y;
	
	if (X < 0) X = 0;
	if (X >= Wsorgente - 1) X = Wsorgente - 1;
	if (Y < 0) Y = 0;
	if (Y >= Hsorgente - 1) Y = Hsorgente - 1;

	v1 = buffer[X * 3 + (Wsorgente * Y * 3) + c];
	v2 = buffer[(X + 1) * 3 + (Wsorgente * Y * 3) + c];
	v3 = buffer[X * 3 + (Wsorgente * (Y + 1) * 3) + c];
	v4 = buffer[(X + 1) * 3 + (Wsorgente * (Y + 1) * 3) + c];

	return( (v1 * (1 - x) * (1 - y)) +
			(v2 * x * (1 - y)) +
			(v3 * (1 - x) * y) +
			(v4 * x * y) );
}

static void salvaBmp(const char *path) {
	FILE *f = fopen(path, "wb");
	if(f == NULL) {
		cout << "\nErrore: impossibile salvare il file\n";
		exit(1);
	}

	fwrite(header, OFFSETsorgente, 1, f);
	fwrite(destinazione, Wdestinazione * Hdestinazione * 3, 1, f);
	
	// Scrivo nuove dimensioni nell'header del file
	if (Wsorgente != Wdestinazione || Hsorgente != Hdestinazione) {
		fseek(f, 18, 0);
	    fwrite(&Wdestinazione, sizeof(int), 1, f);
	    fwrite(&Hdestinazione, sizeof(int), 1, f);
	}
	fclose(f);
}