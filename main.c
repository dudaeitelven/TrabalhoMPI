#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#pragma pack(1)

/*
Para compilar:

1 - Abrir o local do fonte

2 - Digitar para compilar: mpicc -o mpi main.c

3 - Digitar para rodar: mpirun -np <numero_processos> ./mpi <imagem_entrada> <imagem_saida> <mascara>
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
	char *entrada, *saida;
	int tamanhoMascara, nth;
	CABECALHO cabecalho;
	int iForImagem, jForImagem;
	int i2, j2;
	char aux;
	int ali, limiteI, limiteJ, iForOrdenar, jForOrdenar;
	int iTamanhoAux, posicaoMediana, lacoI, lacoJ, iTamanhoAux2;



	if ( argc != 4){
		printf("%s <img_entrada> <img_saida> <mascara> \n", argv[0]);
		exit(0);
	}

	entrada = argv[1];
	saida = argv[2];
    tamanhoMascara = atoi(argv[3]);


	/*
	printf("Digite o nome do arquivo de entrada:\n");
	scanf("%s", entrada);

	printf("Digite o nome do arquivo de saida:\n");
	scanf("%s", saida);

    while ((tamanhoMascara != 3) && (tamanhoMascara != 5) && (tamanhoMascara != 7)) {
        printf("Digite o tamanho da mascara:\n");
        scanf("%d", &tamanhoMascara);
    }
	*/

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
	RGB **imagemSaida  = (RGB **)malloc(cabecalho.altura*sizeof(RGB *));

	//Alocar imagem
	for(iForImagem=0; iForImagem<cabecalho.altura; iForImagem++){
		imagem[iForImagem] = (RGB *)malloc(cabecalho.largura*sizeof(RGB));
		imagemSaida[iForImagem] = (RGB *)malloc(cabecalho.largura*sizeof(RGB));
	}


	//Leitura da imagem
	for(iForImagem=0; iForImagem<cabecalho.altura; iForImagem++){
		ali = (cabecalho.largura * 3) % 4;

		if (ali != 0){
			ali = 4 - ali;
		}

		for(jForImagem=0; jForImagem<cabecalho.largura; jForImagem++){
			fread(&imagem[iForImagem][jForImagem], sizeof(RGB), 1, fin);
		}

		for(jForImagem=0; jForImagem<ali; jForImagem++){
			fread(&aux, sizeof(unsigned char), 1, fin);
		}
	}

	//Processar imagem
	for(iForImagem=0; iForImagem<cabecalho.altura; iForImagem++){
		for(jForImagem=0; jForImagem<cabecalho.largura; jForImagem++){
			if (tamanhoMascara == 3) {
				lacoI = iForImagem-1;
				limiteI = iForImagem + 1;

				lacoJ = jForImagem-1;
				limiteJ = jForImagem + 1;

				posicaoMediana = 4;
			}
			else if (tamanhoMascara == 5) {
				lacoI = iForImagem-2;
				limiteI = iForImagem + 2;

				lacoJ = jForImagem-2;
				limiteJ = jForImagem + 2;

				posicaoMediana = 12;
			}
			else if (tamanhoMascara == 7) {
				lacoI = iForImagem-3;
				limiteI = iForImagem + 3;

				lacoJ = jForImagem-3;
				limiteJ = jForImagem + 3;

				posicaoMediana = 24;
			}

            if (lacoI < 0) lacoI = 0;
			if (lacoJ < 0) lacoJ = 0;

            if (limiteI > (cabecalho.altura - 1))  limiteI = (cabecalho.altura - 1);
			if (limiteJ > (cabecalho.largura - 1)) limiteJ = (cabecalho.largura - 1);

			RGB rgbAux[tamanhoMascara*tamanhoMascara];
            for(iTamanhoAux2=0; iTamanhoAux2<tamanhoMascara*tamanhoMascara; iTamanhoAux2++){
                rgbAux[iTamanhoAux2].red   = 0;
                rgbAux[iTamanhoAux2].green = 0;
                rgbAux[iTamanhoAux2].blue  = 0;
			}

			RGB rgbAux2;
			rgbAux2.red   = 0;
            rgbAux2.green = 0;
            rgbAux2.blue  = 0;

            iTamanhoAux  = 0;
            iTamanhoAux2 = 0;

			//Calcular a mediana de cada pixel da imagem.
			for(i2=lacoI; i2<=limiteI; i2++){
				for(j2=lacoJ; j2<=limiteJ; j2++){
					rgbAux[iTamanhoAux].red   = imagem[i2][j2].red;
					rgbAux[iTamanhoAux].green = imagem[i2][j2].green;
					rgbAux[iTamanhoAux].blue  = imagem[i2][j2].blue;

					iTamanhoAux++;
				}
			}

			//Ordenar vetores red
            for (iForOrdenar = 0; iForOrdenar < iTamanhoAux; iForOrdenar++)
            {
                for (jForOrdenar = 0; jForOrdenar < iTamanhoAux; jForOrdenar++)
                {
                    if (rgbAux[iForOrdenar].red < rgbAux[jForOrdenar].red)
                    {
                        rgbAux2.red             = rgbAux[iForOrdenar].red;
                        rgbAux[iForOrdenar].red = rgbAux[jForOrdenar].red;
                        rgbAux[jForOrdenar].red = rgbAux2.red;
                    }
                }
            }

            //Ordenar vetores green
            for (iForOrdenar = 0; iForOrdenar < iTamanhoAux; iForOrdenar++)
            {
                for (jForOrdenar = 0; jForOrdenar < iTamanhoAux; jForOrdenar++)
                {
                    if (rgbAux[iForOrdenar].green < rgbAux[jForOrdenar].green)
                    {
                        rgbAux2.green             = rgbAux[iForOrdenar].green;
                        rgbAux[iForOrdenar].green = rgbAux[jForOrdenar].green;
                        rgbAux[jForOrdenar].green = rgbAux2.green;
                    }
                }
            }

            //Ordenar vetores blue
            for (iForOrdenar = 0; iForOrdenar < iTamanhoAux; iForOrdenar++)
            {
                for (jForOrdenar = 0; jForOrdenar < iTamanhoAux; jForOrdenar++)
                {
                    if (rgbAux[iForOrdenar].blue < rgbAux[jForOrdenar].blue)
                    {
                        rgbAux2.blue             = rgbAux[iForOrdenar].blue;
                        rgbAux[iForOrdenar].blue = rgbAux[jForOrdenar].blue;
                        rgbAux[jForOrdenar].blue = rgbAux2.blue;
                    }
                }
            }

            //Substituir valores pela mediana de cada pixel
			imagemSaida[iForImagem][jForImagem].red    = rgbAux[posicaoMediana].red;
			imagemSaida[iForImagem][jForImagem].green  = rgbAux[posicaoMediana].green;
			imagemSaida[iForImagem][jForImagem].blue   = rgbAux[posicaoMediana].blue;
		}
	}

	//Escrever a imagem
	for(iForImagem=0; iForImagem<cabecalho.altura; iForImagem++){
		ali = (cabecalho.largura * 3) % 4;

		if (ali != 0){
			ali = 4 - ali;
		}

		for(jForImagem=0; jForImagem<cabecalho.largura; jForImagem++){
			fwrite(&imagemSaida[iForImagem][jForImagem], sizeof(RGB), 1, fout);
		}

		for(jForImagem=0; jForImagem<ali; jForImagem++){
			fwrite(&aux, sizeof(unsigned char), 1, fout);
		}
	}

	fclose(fin);
	fclose(fout);
}
