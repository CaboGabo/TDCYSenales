#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define pi 3.14159265

int main(int argc, char *argv[]) {
  FILE *entrada;
  FILE *salida;
  int nsam=0, i=0, j=0, samr=0, src=0;
  short buffer=0;
  unsigned short b2=0;
  short sam[50], rc[50];

  if (argc<3) {
    printf("No introdujiste todos los argumentos");
    return 1;
  }
  entrada = fopen(argv[1], "rb");
  salida = fopen(argv[2], "wb");

  while(!feof(entrada)) {
    fread(&b2,2,1,entrada);
    i+=2;
    if(i==26)
		  samr=b2;
		if(i==28)
			samr+=b2<<16;
		if(i==42)
			nsam=b2;
		if(i==44)
			nsam+=b2<<16;
  }

  fclose(entrada);
  entrada = fopen(argv[1], "rb");

  src = llenarftt(rc,sam);
  i=0;
  while(!feof(entrada)){
		fread(&buffer,2,1,entrada);
		i+=2;
		if(i>44 && i<nsam+44){
			for(j=49;j>0;j--){
			sam[j]=sam[j-1];
			//printf("s=%02x",sam[j]);
		  }
      sam[0]=buffer;
      buffer=0;
      for(j=0;j<50;j++){
        buffer+=rc[j]*sam[j]/src;
        //if(buffer!=0) printf("\nb=%02X *** s=%02x *** rc=%02x",buffer,sam[j],rc[j]);
      }
		  //printf("\n");	
		}
		fwrite(&buffer,2,1,salida);
		}
    printf("La vida es hermosa :v\n");


  fclose(entrada);
  fclose(salida);

  return 0;
  }

int llenarftt(short fft[], short sam[]) {
  int i, suma=0;
  for(i=0; i<50;i++) {
    fft[i] = 1000*pi*exp(-1*1000*pi*i/44100);
    sam[i] = 0;
  }
  for(i=0; i<50;i++) {
    suma+=fft[i];
  }

  return suma;
}
