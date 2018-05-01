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

int esEstereo(HEADER);
void multiplicaComplejos(FILE*, FILE*, HEADER, HEADER, FILE*);
void pieDeArchivo(FILE*, FILE*);
void multiplicar(FILE*, FILE*, FILE*, int, int, int);
float encuentraMaximo(float*, int);
float encuentraMinimo(float*, int);
void escalar(float[], float[], int, float, short[], short[]);

int main(int argc, char *argv[]) {
    FILE* entrada1;
    FILE* entrada2;
	FILE* salida;
    HEADER cabeceraEntrada1, cabeceraEntrada2;

    if(argc < 4 ) {
		printf("No introdujiste todos los argumentos"); // Checamos si introdujo todos los argumentos
		return 1;
	}
	entrada1 = fopen(argv[1], "rb");
    entrada2 = fopen(argv[2], "rb");
	salida = fopen(argv[3], "wb"); // Abrimos los archivos

    cabeceraEntrada1 = obtenerCabecera(entrada1, cabeceraEntrada1);
    cabeceraEntrada2 = obtenerCabecera(entrada2, cabeceraEntrada2);

    multiplicaComplejos(entrada1, entrada2, cabeceraEntrada1, cabeceraEntrada2, salida);

    fclose(entrada1);
    fclose(entrada2);
    fclose(salida);

    return 0;
}

int esEstereo(HEADER cabecera) {
    if(cabecera.channels == 2) return 1;
    else return 0;
}

void multiplicaComplejos(FILE* entrada1, FILE* entrada2, HEADER cabecera1, HEADER cabecera2, FILE*salida) {
    fseek(entrada1, 44, SEEK_SET);
    fseek(entrada2, 44, SEEK_SET);
    int N;
    if(esEstereo(cabecera1) && esEstereo(cabecera2)) {
        if(cabecera1.data_size >= cabecera2.data_size) {
            fwrite(&cabecera1, sizeof(HEADER), 1, salida);
            N = cabecera1.data_size/4;
            multiplicar(entrada1, entrada2, salida, N, esEstereo(cabecera1),esEstereo(cabecera2));
            pieDeArchivo(entrada1, salida);
        }
        else {
            fwrite(&cabecera2, sizeof(HEADER), 1, salida);
            N = cabecera2.data_size/4;
            multiplicar(entrada1, entrada2, salida, N, esEstereo(cabecera1),esEstereo(cabecera2));
            pieDeArchivo(entrada2, salida);
        }
    }
    else if(esEstereo(cabecera1) && (esEstereo(cabecera2)== 0)) {
        if(cabecera1.data_size/2 > cabecera2.data_size) {
            fwrite(&cabecera1, sizeof(HEADER), 1, salida);
            N = cabecera1.data_size/4;
            multiplicar(entrada1, entrada2, salida, N, esEstereo(cabecera1),esEstereo(cabecera2));
            pieDeArchivo(entrada1, salida);
        }
        else {
            HEADER nuevaCabeceraEstereo;
            nuevaCabeceraEstereo = obtenerCabeceraStereo(cabecera2, nuevaCabeceraEstereo);
            fwrite(&nuevaCabeceraEstereo, sizeof(HEADER), 1,salida);
            N = cabecera2.data_size/2;
            multiplicar(entrada1, entrada2, salida, N, esEstereo(cabecera1),esEstereo(cabecera2));
            pieDeArchivo(entrada2, salida);
        }
    }
    else if((esEstereo(cabecera1)== 0) && esEstereo(cabecera2)) {
        if(cabecera1.data_size >= cabecera2.data_size/2) {
            HEADER nuevaCabeceraEstereo;
            nuevaCabeceraEstereo = obtenerCabeceraStereo(cabecera1, nuevaCabeceraEstereo);
            fwrite(&nuevaCabeceraEstereo, sizeof(HEADER), 1,salida);
            N = cabecera1.data_size/2;
            multiplicar(entrada1, entrada2, salida, N, esEstereo(cabecera1),esEstereo(cabecera2));
            pieDeArchivo(entrada1, salida);
        }
        else {
            fwrite(&cabecera2, sizeof(HEADER), 1, salida);
            N = cabecera2.data_size/4;
            multiplicar(entrada1, entrada2, salida, N, esEstereo(cabecera1),esEstereo(cabecera2));
            pieDeArchivo(entrada2, salida);
        }
    }
    else {
        if(cabecera1.overall_size > cabecera2.overall_size) {
            fwrite(&cabecera1, sizeof(HEADER), 1, salida);
            N = cabecera1.data_size/2;
            multiplicar(entrada1, entrada2, salida, N, esEstereo(cabecera1),esEstereo(cabecera2));
            pieDeArchivo(entrada1, salida);

        }
        else {
            fwrite(&cabecera2, sizeof(HEADER), 1, salida);
            N = cabecera2.data_size/2;
            multiplicar(entrada1, entrada2, salida, N, esEstereo(cabecera1),esEstereo(cabecera2));
            pieDeArchivo(entrada2, salida);
        }
    }
}

void pieDeArchivo(FILE* entrada, FILE* salida) {
    while(!feof(entrada)) { // Escribimos el pie del archivo .wav
            int8_t fin;
            fread(&fin, sizeof(int8_t),1, entrada);
            fwrite(&fin, sizeof(int8_t), 1, salida);
        }
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
    }
    
}

void multiplicar(FILE *entrada1, FILE* entrada2, FILE *salida, int N, int ee1, int ee2) {
    short salidaRe[N], salidaIm[N];
    float entradaRe1[N], entradaIm1[N], entradaRe2[N], entradaIm2[N];
    float multRe[N], multIm[N];
    short valor;
    int i;
    for(i=0; i<N; i++) {
        if(ee1==0 && ee2==0) {
            fread(&valor, sizeof(short),1, entrada1);
            entradaRe1[i] = (float)valor/32767;
            entradaIm1[i] = 0;
            fread(&valor, sizeof(short),1,entrada2);
            entradaRe2[i] = (float)valor/32767;
            entradaIm2[i] = 0;
        }
        else if(ee1 && ee2==0) {
            fread(&valor, sizeof(short),1, entrada1);
            entradaRe1[i] = (float)valor/32767;
            fread(&valor, sizeof(short),1, entrada1);
            entradaIm1[i] = (float)valor/32767;
            fread(&valor, sizeof(short),1,entrada2);
            entradaRe2[i] = (float)valor/32767;
            entradaIm2[i] = 0;
        }
        else if(ee1==0 && ee2) {
            fread(&valor, sizeof(short),1, entrada1);
            entradaRe1[i] = (float)valor/32767;
            entradaIm1[i] = 0;
            fread(&valor, sizeof(short),1, entrada1);
            entradaRe2[i] = (float)valor/32767;
            fread(&valor, sizeof(short),1,entrada2);
            entradaIm2[i] = (float)valor/32767;
        }
        else {
            fread(&valor, sizeof(short),1, entrada1);
            entradaRe1[i] = (float)valor/32767;
            fread(&valor, sizeof(short),1, entrada1);
            entradaIm1[i] = (float)valor/32767;
            fread(&valor, sizeof(short),1, entrada2);
            entradaRe2[i] = (float)valor/32767;
            fread(&valor, sizeof(short),1, entrada2);
            entradaIm2[i] = (float)valor/32767;
        }
        //printf("Entrada Real 1: %f\n", entradaRe1[i]);
        //printf("Entrada Imag 1: %f\n", entradaIm1[i]);
        //printf("Entrada Real 2: %f\n", entradaRe2[i]);
        //printf("Entrada Imag 2: %f\n", entradaIm2[i]);
    }

    for(i=0; i<N; i++) {
        multRe[i] = (entradaRe1[i]*entradaRe2[i]) - (entradaIm1[i] * entradaIm2[i]);
        multIm[i] = (entradaRe1[i]*entradaIm2[i]) + (entradaRe2[i]*entradaIm1[i]);
    }

    float maxRe = encuentraMaximo(multRe, N);
    float minRe = encuentraMinimo(multRe, N);
    float maxIm = encuentraMaximo(multIm, N);
    float minIm = encuentraMaximo(multIm, N);
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

    escalar(multRe, multIm, N, max, salidaRe, salidaIm);

    for(i=0; i<N; i++) {
        if(ee1==0 && ee2==0) {
            fwrite(&salidaRe[i], sizeof(short), 1, salida);
        }
        else {
            fwrite(&salidaRe[i], sizeof(short), 1, salida);
            fwrite(&salidaIm[i], sizeof(short), 1, salida);
        }
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