#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define pi 3.14159265

typedef struct{
    char    riff[4]; // Cadena RIFF
    int     overall_size; // Tamaño del archivo
    char    wave[4]; // Cadena WAVE
    char    fmt_chunk_marker[4]; // Cadena fmt
    int     length_of_fmt;
    short   format_type; // tipo
    short   channels; // Numero de canales
    int     sample_rate; // Bloques por segundo
    int     byterate; // sample_rate * num_channels * bitspersample/8
    short   block_align; // numChannels * bitsPerSample/8
    short   bits_per_sample;
    char    data_chunk_header[4]; // Cadena DATA o FLLR
    int     data_size; // Tamaño de los datos
}HEADER;

HEADER obtenerCabecera(FILE *, HEADER);

void copiarArreglo(char[],char[]);
HEADER obtenerCabeceraStereo(HEADER,HEADER);
void TDF(FILE*, HEADER, HEADER, int, int);
long encuentraMaximo(long*, int);
long encuentraMinimo(long*, int);
void escalar(long[], long[], int, long, short[], short[], int[]);
void descifrar(short[], int);
void llenarPosicionesArreglo(int[], float);
void obtenerNumero(short[], int[]);


int main(int argc, char *argv[]) {
    HEADER cabecera;
    HEADER cabeceraStereo;
	FILE* entrada;
	//FILE* salida;

	char nombre[50];
	gets(nombre);
	entrada = fopen(nombre, "rb");
	//salida = fopen(argv[2], "wb"); // Abrimos los archivos

	cabecera = obtenerCabecera(entrada, cabecera); // Obtenemos el encabezado del archivo de entrada
    cabeceraStereo = obtenerCabeceraStereo(cabecera,cabeceraStereo); // Obtenemos su equivalente en modo stereo.
    //imprimirCabecera(cabecera);
    //imprimirCabecera(cabeceraStereo);
    TDF(entrada, cabecera, cabeceraStereo, 0, cabecera.data_size/2);
	fclose(entrada);
	return 0;
}

void escalar(long mult1[], long mult2[], int N, long max, short sal1[], short sal2[], int posiciones[]) {
    int i;
    double resRe[N], resIm[N];
    //if(max > 32767) {
        for(i=0; i<N; i++) {
            resRe[i] = (double)mult1[i]/max;
            resIm[i] = (double)mult2[i]/max;
            /*if(i==posiciones[0] || i==posiciones[1] || i==posiciones[2] || i==posiciones[3] || i==posiciones[4] || i == posiciones[5] || i==posiciones[6] || i== posiciones[7]) {
                printf("[%d] = %f\n", i, resRe[i]);
            }*/
            
        }
    //}
    /*else {
        for(i=0; i<N; i++) {
            mult1[i] = mult1[i]/32767;
            mult2[i] = mult2[i]/32767;
        }
    }*/
    for(i=0; i<N; i++) {
        sal1[i] = (short)(resRe[i]*32767);
        sal2[i] = (short)(resIm[i]*32767);
    }
    
}

HEADER obtenerCabeceraStereo(HEADER cabecera,HEADER cabeceraStereo) {
    copiarArreglo(cabecera.riff,cabeceraStereo.riff);
    copiarArreglo(cabecera.wave, cabeceraStereo.wave);
    copiarArreglo(cabecera.fmt_chunk_marker, cabeceraStereo.fmt_chunk_marker);
    cabeceraStereo.length_of_fmt = cabecera.length_of_fmt;
    cabeceraStereo.format_type = cabecera.format_type;
    cabeceraStereo.channels = 2;
    cabeceraStereo.sample_rate = cabecera.sample_rate;
    cabeceraStereo.bits_per_sample = cabecera.bits_per_sample;
    copiarArreglo(cabecera.data_chunk_header, cabeceraStereo.data_chunk_header);
    cabeceraStereo.data_size = cabecera.data_size * 2;
    cabeceraStereo.block_align = cabeceraStereo.channels * (cabeceraStereo.bits_per_sample/8);
    cabeceraStereo.byterate = cabeceraStereo.sample_rate * cabeceraStereo.block_align;
    cabeceraStereo.overall_size = cabecera.overall_size + cabecera.data_size;

    return cabeceraStereo;
}

void copiarArreglo(char arreglo1[4], char arreglo2[4]) {
    int i;
    for (i=0; i<4; i++) {
        arreglo2[i] = 0;
        arreglo2[i] = arreglo1[i];
    }
}

long encuentraMaximo(long *x,int tamanioArreglo){
    long aux=x[0];
    int i=0;
    for(i=0;i<tamanioArreglo;i++){
        if(aux<x[i]){
            aux=x[i];
        }
    }
    return aux;
}

long encuentraMinimo(long *x,int tamanioArreglo){
    long aux=x[0];
    int i=0;
    for(i=0;i<tamanioArreglo;i++){
        if(aux>x[i]){
            aux=x[i];
        }
    }
    return aux;
}

void TDF(FILE *entrada, /*FILE* salida,*/ HEADER cabecera, HEADER cabeceraStereo, int inicio, int fin) {
    //fwrite(&cabeceraStereo, sizeof(HEADER), 1, salida); // Escribimos el encabezado en formato stereo en el archivo de salida
    int i,t,N = cabecera.data_size/2;
	float duracion = (float)N/cabecera.sample_rate;
	int posiciones[8];
	llenarPosicionesArreglo(posiciones,duracion);
    long sumreal[fin-inicio],sumimag[fin-inicio];
    short salRe[fin-inicio], salIm[fin-inicio];
    for(i = 0; i<fin-inicio; i++) {
        fseek(entrada, 44+inicio, SEEK_SET);
        short valor = 0;
        sumreal[i]=0;
        sumimag[i]=0;
        salRe[i] = 0;
        salIm[i] = 0;
		if(i==posiciones[0] || i==posiciones[1] || i==posiciones[2] || i==posiciones[3] || i==posiciones[4] || i == posiciones[5] || i==posiciones[6] || i== posiciones[7]) {
			for(t =0; t<fin-inicio;t++) {
				fread(&valor, sizeof(short), 1, entrada);
				long angulo = 2*pi*i*t / (fin-inicio);
				sumreal[i] += valor * cos(angulo);
				sumimag[i] -= valor * sin(angulo);
        	}
        //printf("sumreal[%d]: %ld\n", i,sumreal[i]);
        //printf("sumimag[%d]: %f\n", i,sumimag[i]);
		}    
    }
    
    long maxRe = encuentraMaximo(sumreal, fin-inicio);
    long minRe = encuentraMinimo(sumreal, fin-inicio);
    long maxIm = encuentraMaximo(sumimag, fin-inicio);
    long minIm = encuentraMaximo(sumimag, fin-inicio);
    long max,min;
    if(maxRe>maxIm) {
        max = maxRe;
    } 
    else {
        max = maxIm;
    }

    if(minRe<minIm) {
        min = minRe;
    }
    else {
        min = minIm;
    }

    if(max < min*-1) {
        max = -1*min;
    }
    //printf("max: %f\n", max);
    escalar(sumreal, sumimag, fin-inicio, max, salRe, salIm, posiciones);
    /*for(i=0;i<fin-inicio; i++) {
        if(i==posiciones[0] || i==posiciones[1] || i==posiciones[2] || i==posiciones[3] || i==posiciones[4] || i == posiciones[5] || i==posiciones[6] || i== posiciones[7]) {
            printf("[%d] = %d\n", i,salRe[i]);
        }
    }
    printf("\n\n");*/
    obtenerNumero(salRe,posiciones);
    /*for(i=0; i<fin-inicio; i++) {
        fwrite(&salRe[i], sizeof(short), 1, salida);
        fwrite(&salIm[i], sizeof(short), 1, salida);
    }*/

    /*while(!feof(entrada)) { // Escribimos el pie del archivo .wav
        int8_t fin;
        fread(&fin, sizeof(int8_t),1, entrada);
        fwrite(&fin, sizeof(int8_t), 1, salida);
    }*/
}

HEADER obtenerCabecera(FILE *entrada, HEADER cabecera){
    fread(cabecera.riff, sizeof(char), 4, entrada);
    fread(&cabecera.overall_size, sizeof(int), 1, entrada);
    fread(cabecera.wave, sizeof(char), 4, entrada);
    fread(cabecera.fmt_chunk_marker, sizeof(char), 4, entrada);
    fread(&cabecera.length_of_fmt, sizeof(int), 1, entrada);
    fread(&cabecera.format_type, sizeof(short), 1, entrada);
    fread(&cabecera.channels, sizeof(short), 1, entrada);
    fread(&cabecera.sample_rate, sizeof(int), 1, entrada);
    fread(&cabecera.byterate, sizeof(int), 1, entrada);
    fread(&cabecera.block_align, sizeof(short), 1, entrada);
    fread(&cabecera.bits_per_sample, sizeof(short), 1, entrada);
    fread(cabecera.data_chunk_header, sizeof(char), 4, entrada);
    fread(&cabecera.data_size, sizeof(int), 1, entrada);

    return cabecera;
}

void llenarPosicionesArreglo(int posiciones[], float duracion) {
	posiciones[0] = duracion*1209;
	posiciones[1] = duracion*1336;
	posiciones[2] = duracion*1477;
	posiciones[3] = duracion*1633;
	posiciones[4] = duracion*697;
	posiciones[5] = duracion*770;
	posiciones[6] = duracion*852;
	posiciones[7] = duracion*941;
}

void obtenerNumero(short arreglo[], int posiciones[]) {
	char numero = ' ';
	if(arreglo[posiciones[0]]>14000) {
		if(arreglo[posiciones[4]]>14000) {
			numero = '1';
		}
		if(arreglo[posiciones[5]]>14000) {
			numero = '4';
		}
		if(arreglo[posiciones[6]]>14000) {
			numero = '7';
		}
		if(arreglo[posiciones[7]]>14000) {
			numero = '*';
		}
	}
	if(arreglo[posiciones[1]]>14000) {
		if(arreglo[posiciones[4]]>14000) {
			numero = '2';
		}
		if(arreglo[posiciones[5]]>14000) {
			numero = '5';
		}
		if(arreglo[posiciones[6]]>14000) {
			numero = '8';
		}
		if(arreglo[posiciones[7]]>14000) {
			numero = '0';
		}
	}
	if(arreglo[posiciones[2]]>14000) {
		if(arreglo[posiciones[4]]>14000) {
			numero = '3';
		}
		if(arreglo[posiciones[5]]>14000) {
			numero = '6';
		}
		if(arreglo[posiciones[6]]>14000) {
			numero = '9';
		}
		if(arreglo[posiciones[7]]>14000) {
			numero = '#';
		}
	}
	if(arreglo[posiciones[3]]>14000) {
		if(arreglo[posiciones[4]]>14000) {
			numero = 'A';
		}
		if(arreglo[posiciones[5]]>14000) {
			numero = 'B';
		}
		if(arreglo[posiciones[6]]>14000) {
			numero = 'C';
		}
		if(arreglo[posiciones[7]]>14000) {
			numero = 'D';
		}
	}
	printf("%c", numero);
}
