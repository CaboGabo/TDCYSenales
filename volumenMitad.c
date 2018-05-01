#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
void salidaMitad(FILE*, FILE*, HEADER);

int main(int argc, char *argv[]) {
	HEADER cabecera;
	FILE* entrada;
	FILE* salida;

	if(argc < 3 ) {
		printf("No introdujiste todos los argumentos"); // Checamos si introdujo todos los argumentos
		return 0;
	}
	entrada = fopen(argv[1], "rb");
	salida = fopen(argv[2], "wb"); // Abrimos los archivos

	cabecera = obtenerCabecera(entrada, cabecera); // Obtenemos el encabezado del archivo de entrada
	salidaMitad(entrada, salida, cabecera); // Obtenemos el archivo de salida con la mitad del volumen

	fclose(entrada);
	fclose(salida);
	return 0;

}

void salidaMitad(FILE *entrada, FILE *salida, HEADER cabecera) {
	fwrite(&cabecera, sizeof(HEADER), 1, salida); // Escribimos el encabezado del archivo de entrada en el de salida

	fseek(entrada, 44, SEEK_SET); // Los datos empiezan a partir del byte 44

	int i;
	short nuevoValor=0;
	for (i=0; i<cabecera.data_size/2; i++) { // La mitad debido a que vamos tomando de 2 bytes en 2 bytes
		fread(&nuevoValor, sizeof(short), 1, entrada);
		nuevoValor*= 0.5; // Expresion para obtener la mitad del volumen
		fwrite(&nuevoValor, sizeof(short), 1, salida);
	}
	while(!feof(entrada)){							// El final del archivo lo copiamos tal cual
        int8_t fin;									// Es de 8 bits exactamente
        fread(&fin, sizeof(int8_t), 1, entrada);
        fwrite(&fin, sizeof(int8_t), 1, salida);
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

    printf("RIFF: %s\n", cabecera.riff);
    printf("Overall Size: %d\n", cabecera.overall_size);
    printf("WAVE: %s\n",cabecera.wave);
    printf("FMT chunk marker: %s\n",cabecera.fmt_chunk_marker);
    printf("Length of fmt: %d\n",cabecera.length_of_fmt);
    printf("Format type: %d\n",cabecera.format_type);
    printf("Num Channels: %d\n",cabecera.channels);
    printf("SampleRate: %d\n",cabecera.sample_rate);
    printf("ByteRate: %d\n",cabecera.byterate);
    printf("BlockAlign: %d\n",cabecera.block_align);
    printf("BitsPerSample: %d\n",cabecera.bits_per_sample);
    printf("Data chunk header: %s\n",cabecera.data_chunk_header);
    printf("Data size: %d\n",cabecera.data_size);
    return cabecera;
}
