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
void TDFinversa(FILE*, FILE*, HEADER);
float encuentraMaximo(float*, int);
float encuentraMinimo(float*, int);
void escalar(float[], float[], int, float, short[], short[]);

int main(int argc, char *argv[]) {
	HEADER cabecera;
    //HEADER cabeceraStereo;
	FILE* entrada;
	FILE* salida;

	if(argc < 3 ) {
		printf("No introdujiste todos los argumentos"); // Checamos si introdujo todos los argumentos
		return 0;
	}
	entrada = fopen(argv[1], "rb");
	salida = fopen(argv[2], "wb"); // Abrimos los archivos

	cabecera = obtenerCabecera(entrada, cabecera); // Obtenemos el encabezado del archivo de entrada
    //imprimirCabecera(cabecera);
    TDFinversa(entrada, salida, cabecera);
	fclose(entrada);
	fclose(salida);
	return 0;

}

void TDFinversa(FILE* entrada, FILE* salida, HEADER cabecera) {
    fwrite(&cabecera, sizeof(HEADER), 1, salida);
    //fseek(entrada, 44, SEEK_SET);
    int i,t,N = cabecera.data_size/2;
    short valorRe=0, valorIm=0;
    short valor;
    float valoresRe[N/2], valoresIm[N/2];
    short salidaRe[N/2], salidaIm[N/2];
    for(i = 0; i<N/2; i++) {
        fseek(entrada, 44, SEEK_SET);
        double sumreal = 0;
        double sumimag = 0;
        for(t =0; t<N/2;t++) {
            valorRe=0;
            valorIm=0;
            fread(&valorRe, sizeof(short), 1, entrada);
            fread(&valorIm, sizeof(short), 1, entrada);
            double angulo = 2*pi*i*t / (N/2);
            sumreal += (valorRe * cos(angulo)) - (valorIm * sin(angulo));
            sumimag += (valorRe * sin(angulo)) + (valorIm * cos(angulo));
        }
        sumreal = sumreal/(N/2);
        sumimag = sumimag/(N/2);

        valoresRe[i] = (float)sumreal;
        valoresIm[i] = (float)sumimag;

        //printf("valoresRe: %f\n", valoresRe[i]);
        //printf("valoresIm: %f\n", valoresIm[i]);
    }

    float maxRe = encuentraMaximo(valoresRe, N/2);
    float minRe = encuentraMinimo(valoresRe, N/2);
    float maxIm = encuentraMaximo(valoresIm, N/2);
    float minIm = encuentraMaximo(valoresIm, N/2);
    float max,min;

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
        //printf("Max: %f\n",max);
        
        escalar(valoresRe, valoresIm, N/2, max, salidaRe, salidaIm);

       for(i=0; i<N/2; i++) {
           //printf("salidaReal: %d\n", salidaRe[i]);
           //printf("salidaImag: %d\n", salidaIm[i]);
            fwrite(&salidaRe[i], sizeof(short), 1, salida);
            fwrite(&salidaIm[i], sizeof(short), 1, salida);
       }

    while(!feof(entrada)) { // Escribimos el pie del archivo .wav
        int8_t fin;
        fread(&fin, sizeof(int8_t),1, entrada);
        fwrite(&fin, sizeof(int8_t), 1, salida);
    }
}

float encuentraMaximo(float *x,int tamanioArreglo){
    float aux=x[0];
    int i=0;
    for(i=0;i<tamanioArreglo;i++){
        if(aux<x[i]){
            aux=x[i];
        }
    }
    return aux;
}

float encuentraMinimo(float *x,int tamanioArreglo){
    float aux=x[0];
    int i=0;
    for(i=0;i<tamanioArreglo;i++){
        if(aux>x[i]){
            aux=x[i];
        }
    }
    return aux;
}

void escalar(float mult1[], float mult2[], int N, float max, short sal1[], short sal2[]) {
    int i;

    if(max > 1) {
        for(i=0; i<N; i++) {
            mult1[i] = mult1[i]/max;
            mult2[i] = mult2[i]/max;
        }
    }
    for(i=0; i<N; i++) {
        sal1[i] = (short)(mult1[i]*32767);
        sal2[i] = (short)(mult2[i]*32767);
        //printf("salReal: %d\n", sal1[i]);
        //printf("salImag: %d\n", sal2[i]);
    }
    
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