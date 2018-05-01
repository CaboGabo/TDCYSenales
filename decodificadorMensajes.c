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
float encuentraMaximo(float*, int);
float encuentraMinimo(float*, int);
void escalar(float[], float[], int, float, short[], short[]);
void descifrar(short[], int);


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
    int i,j=0;
    for(i=0; i<cabecera.data_size; i=i+64) {
        //printf("%d - %d\n", i, i+32);
        TDF(entrada, cabecera, cabeceraStereo, i, i+32);
        //j++;
    }

    /*while(!feof(entrada)) { // Escribimos el pie del archivo .wav
        int8_t fin;
        fread(&fin, sizeof(int8_t),1, entrada);
        fwrite(&fin, sizeof(int8_t), 1, salida);
    }*/

	fclose(entrada);
	//fclose(salida);
	return 0;

}

void TDF(FILE *entrada, /*FILE* salida,*/ HEADER cabecera, HEADER cabeceraStereo, int inicio, int fin) {
    //fwrite(&cabeceraStereo, sizeof(HEADER), 1, salida); // Escribimos el encabezado en formato stereo en el archivo de salida
    int i,t,N = cabecera.data_size/2;
    float sumreal[fin-inicio],sumimag[fin-inicio];
    short salRe[fin-inicio], salIm[fin-inicio];
    for(i = 0; i<fin-inicio; i++) {
        fseek(entrada, 44+inicio, SEEK_SET);
        short valor = 0;
        sumreal[i]=0;
        sumimag[i]=0;
        salRe[i] = 0;
        salIm[i] = 0;
        for(t =0; t<fin-inicio;t++) {
            fread(&valor, sizeof(short), 1, entrada);
            float angulo = 2*pi*i*t / (fin-inicio);
            sumreal[i] += valor * cos(angulo);
            sumimag[i] -= valor * sin(angulo);
        }
        //printf("sumreal[%d]: %f\n", i,sumreal[i]);
        //printf("sumimag[%d]: %f\n", i,sumimag[i]);

    }
    
    float maxRe = encuentraMaximo(sumreal, fin-inicio);
    float minRe = encuentraMinimo(sumreal, fin-inicio);
    float maxIm = encuentraMaximo(sumimag, fin-inicio);
    float minIm = encuentraMaximo(sumimag, fin-inicio);
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
    //printf("max: %f\n", max);
    escalar(sumreal, sumimag, fin-inicio, max, salRe, salIm);
    /*for(i=0;i<fin-inicio; i++) {
        printf("%d\n", salRe[i]);
    }
    printf("\n\n");*/
    descifrar(salRe, fin-inicio);
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

    //if(max > 32767) {
        for(i=0; i<N; i++) {
            mult1[i] = mult1[i]/max;
            mult2[i] = mult2[i]/max;
            
        }
    //}
    /*else {
        for(i=0; i<N; i++) {
            mult1[i] = mult1[i]/32767;
            mult2[i] = mult2[i]/32767;
        }
    }*/
    for(i=0; i<N; i++) {
        sal1[i] = (short)(mult1[i]*32767);
        sal2[i] = (short)(mult2[i]*32767);
    }
    
}

void descifrar(short valores[], int n) {
    int suma=0;
    if(valores[1] > 30000) {
        suma+=128;
    }
    if(valores[3] > 30000) {
        suma+=64;
    }
    if(valores[5] > 30000) {
        suma+=32;
    }
    if(valores[7] > 30000) {
        suma+=16;
    }
    if(valores[9] > 30000) {
        suma+=8;
    }
    if(valores[11] > 30000) {
        suma+=4;
    }
    if(valores[13] > 30000) {
        suma+=2;
    }
    if(valores[15] > 30000) {
        suma+=1;
    }
    printf("%c", suma);
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
