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

int main(int argc, char **argv ){
	char entrada[100], saida[100];
	CABECALHO cabecalho;
	int i, j;
	int i2, j2;
	char aux;
	int ali;
	int tamanhoMascara;
	int limiteI;
	int limiteJ;
    int iFor;
    int iAux;
    int posicaoMediana;
    int lacoI, lacoJ;
    int iAux2;

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

	RGB **imagem  = (RGB **)malloc(cabecalho.altura*sizeof(RGB *));

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
				lacoI = i-1;
				limiteI = i + 1;

				lacoJ = j-1;
				limiteJ = j + 1;

				posicaoMediana = 5;
			}
			else if (tamanhoMascara == 5) {
				lacoI = i-2;
				limiteI = i + 2;

				lacoJ = j-2;
				limiteJ = j + 2;

				posicaoMediana = 13;
			}
			else if (tamanhoMascara == 7) {
				lacoI = i-3;
				limiteI = i + 3;

				lacoJ = j-3;
				limiteJ = j + 3;

				posicaoMediana = 25;
			}

            if (lacoI < 0) lacoI = 0;
			if (lacoJ < 0) lacoJ = 0;

            if (limiteI > cabecalho.altura)  limiteI = cabecalho.altura;
			if (limiteJ > cabecalho.largura) limiteJ = cabecalho.largura;

			RGB rgbAux[tamanhoMascara*tamanhoMascara];
            for(iAux2=0; iAux2<tamanhoMascara*tamanhoMascara; iAux2++){
                rgbAux[iAux2].red   = 0;
                rgbAux[iAux2].green = 0;
                rgbAux[iAux2].blue  = 0;
			}

			RGB rgbAux2;
			rgbAux2.red   = 0;
            rgbAux2.green = 0;
            rgbAux2.blue  = 0;

			//Calcular a mediana de cada pixel da imagem.
			for(i2=lacoI; i2<=limiteI; i2++){
				for(j2=lacoJ; j2<=limiteJ; j2++){
					rgbAux[iAux].red   = imagem[i2][j2].red;
					rgbAux[iAux].green = imagem[i2][j2].green;
					rgbAux[iAux].blue  = imagem[i2][j2].blue;

					iAux++;
				}
			}

            /*
			//Ordenar vetores red
            for (iFor = 0; iFor < iAux; iFor++) {
                if (rgbAux[iFor].red > rgbAux[iFor + 1].red) {
                    rgbAux2.red          = rgbAux[iFor].red;
                    rgbAux[iFor].red     = rgbAux[iFor + 1].red;
                    rgbAux[iFor + 1].red = rgbAux2.red;
                }
            }

            //Ordenar vetores green
            for (iFor = 0; iFor < iAux; iFor++) {
                if (rgbAux[iFor].green > rgbAux[iFor + 1].green) {
                    rgbAux2.green          = rgbAux[iFor].green;
                    rgbAux[iFor].green     = rgbAux[iFor + 1].green;
                    rgbAux[iFor + 1].green = rgbAux2.green;
                }
            }

            //Ordenar vetores blue
            for (iFor = 0; iFor < iAux; iFor++) {
                if (rgbAux[iFor].blue > rgbAux[iFor + 1].blue) {
                    rgbAux2.blue          = rgbAux[iFor].blue;
                    rgbAux[iFor].blue     = rgbAux[iFor + 1].blue;
                    rgbAux[iFor + 1].blue = rgbAux2.blue;
                }
            }

            */

            iAux = 0;

			imagem[i][j].red    = rgbAux[posicaoMediana].red;
			imagem[i][j].green  = rgbAux[posicaoMediana].green;
			imagem[i][j].blue   = rgbAux[posicaoMediana].blue;
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
			fwrite(&aux, sizeof(unsigned char), 1, fout);
		}
	}

	fclose(fin);
	fclose(fout);
}
