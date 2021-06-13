#include <stdio.h>
#include <stdlib.h>

#pragma pack(1)

/*
Para compilar:

1 - Abrir o local do fonte

2 - Digitar: gcc -o teste imagens.c

3 - Digitar: ./teste
*/

struct cabecalho {
	unsigned short tipo;
	unsigned int tamanho_arquivo;
	unsigned short reservado1;
	unsigned short reservado2;
	unsigned int offset;
	unsigned int tamanho_image_header;
	int largura;
	int altura;
	unsigned short planos;
	unsigned short bits_por_pixel;
	unsigned int compressao;
	unsigned int tamanho_imagem;
	int largura_resolucao;
	int altura_resolucao;
	unsigned int numero_cores;
	unsigned int cores_importantes;
};
typedef struct cabecalho CABECALHO;

struct rgb{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
};
typedef struct rgb RGB;

/*
int mediana(float v[]){
	int T = 10; //tamanho do vetor
	int mediana;
	mediana = v[T/2-1];   //por algum motivo no meu só dava certo tirando 1 da conta
	printf("Mediana: %i", mediana);
}
*/

int main(int argc, char **argv ){
	char entrada[100], saida[100];
	CABECALHO cabecalho;
	RGB pixel;
	int i, j;
	int i2, j2;
	short media;
	char aux;
	int ali;
	int tamanhoMascara;

	printf("Digite o nome do arquivo de entrada:\n");
	scanf("%s", entrada);

	printf("Digite o nome do arquivo de saida:\n");
	scanf("%s", saida);

	printf("Digite o tamanho da mascara:\n");
	scanf("%d", &tamanhoMascara);

	FILE *fin = fopen(entrada, "rb");

	if ( fin == NULL ){
		printf("Erro ao abrir o arquivo %s\n", entrada);
		exit(0);
	}

	FILE *fout = fopen(saida, "wb");

	if ( fout == NULL ){
		printf("Erro ao abrir o arquivo %s\n", saida);
		exit(0);
	}

	fread(&cabecalho, sizeof(CABECALHO), 1, fin);

	printf("Tamanho da imagem: %u\n", cabecalho.tamanho_arquivo);
	printf("Largura: %d\n", cabecalho.largura);
	printf("Largura: %d\n", cabecalho.altura);
	printf("Bits por pixel: %d\n", cabecalho.bits_por_pixel);

	fwrite(&cabecalho, sizeof(CABECALHO), 1, fout);

	RGB **imagem = (RGB **)malloc(cabecalho.altura*sizeof(RGB *));
	RGB rgbAux;

	//Alocar imagem
	for(i=0; i<cabecalho.altura; i++){
		imagem[i] = (RGB *)malloc(cabecalho.largura*sizeof(RGB));
	}

	//Leitura da imagem
	for(i=0; i<cabecalho.altura; i++){
		ali = (cabecalho.largura * 3) % 4;

		if (ali != 0){
			ali = 4 - ali;
		}

		for(j=0; j<cabecalho.largura; j++){
			fread(&imagem[i][j], sizeof(RGB), 1, fin);
		}

		for(j=0; j<ali; j++){
			fread(&aux, sizeof(unsigned char), 1, fin);
		}
	}

	//Processar imagem
	for(i=0; i<cabecalho.altura; i++){
		for(j=0; j<cabecalho.largura; j++){
			if (tamanhoMascara == 3) {
				i2 = i-1;
				j2 = j-1;
			}
			else if (tamanhoMascara == 5) {
				i2 = i-2;
				j2 = j-2;
			}
			else if (tamanhoMascara == 7) {
				i2 = i-3;
				j2 = j-3;
			}

			if (i2 < 0) i2 = 0;
			if (j2 < 0) j2 = 0;
	
			//Calcular a mediana de cada pixel da imagem.
			for(i2; i2<(i2 + tamanhoMascara); i2++){
				for(j2; j2<(j2 + tamanhoMascara); j2++){
					rgbAux.red   = rgbAux.red   + imagem[i2][j2].red;
					rgbAux.green = rgbAux.green + imagem[i2][j2].green;
					rgbAux.blue  = rgbAux.blue  + imagem[i2][j2].blue;
				}
			}
			
			imagem[i][j].red    = (rgbAux.red / 9);
			imagem[i][j].green  = (rgbAux.green / 9);
			imagem[i][j].blue   = (rgbAux.blue / 9);
		}
	}

	//Escrever a imagem 
	for(i=0; i<cabecalho.altura; i++){
		ali = (cabecalho.largura * 3) % 4;

		if (ali != 0){
			ali = 4 - ali;
		}

		for(j=0; j<cabecalho.largura; j++){
			fwrite(&imagem[i][j], sizeof(RGB), 1, fout);
		}

		for(j=0; j<ali; j++){
			fwrite(&aux, sizeof(unsigned char), 1, fin);
		}
	}

	fclose(fin);
	fclose(fout);
}