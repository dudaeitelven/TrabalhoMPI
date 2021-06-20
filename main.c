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

	int iForImagem, jForImagem;
	int i2, j2;
	char aux;
	int ali, limiteI, limiteJ, iForOrdenar, jForOrdenar;
	int iTamanhoAux, posicaoMediana, lacoI, lacoJ, iTamanhoAux2;
	int range;
	int np, id;
	RGB *imagem, *imagemSaida;
	RGB *imagemSaidaFinal;
	RGB *imagemAux;

	MPI_Init(&argc, &argv);
	//printf("***************************INIT\n");
	MPI_Comm_size(MPI_COMM_WORLD, &np);
	//printf("***************************np\n");
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	//printf("***************************id\n");

	CABECALHO cabecalho;

	if ( argc != 4){
		printf("%s <img_entrada> <img_saida> <mascara> \n", argv[0]);
		exit(0);
	}

	entrada = argv[1];
	saida = argv[2];
    tamanhoMascara = atoi(argv[3]);

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
	//printf("***************************cabecalho leitura\n");
	fread(&cabecalho, sizeof(CABECALHO), 1, fin);

	printf("Tamanho da imagem: %u\n", cabecalho.tamanho_arquivo);
	printf("Largura: %d\n", cabecalho.largura);
	printf("Largura: %d\n", cabecalho.altura);
	printf("Bits por pixel: %d\n", cabecalho.bits_por_pixel);
	
	fwrite(&cabecalho, sizeof(CABECALHO), 1, fout);
	//printf("***************************cabecalho saida\n");



	//Alocar imagem
	RGB rgbAux[tamanhoMascara*tamanhoMascara];
	RGB rgbAux2;	
	
	imagemSaida  = (RGB *)malloc(cabecalho.altura*cabecalho.largura*sizeof(RGB));
	//printf("***************************imgsaida\n");
	imagemAux  = (RGB *)malloc(cabecalho.altura/np*cabecalho.largura*sizeof(RGB));
	//printf("***************************imgaux\n");
	if (id == 0){
		imagem  = (RGB *)malloc(cabecalho.altura*cabecalho.largura*sizeof(RGB));
		imagemSaidaFinal  = (RGB *)malloc(cabecalho.altura*cabecalho.largura*sizeof(RGB));
		//printf("***************************ALOCA\n");

			//Leitura da imagem
		//printf("***************************LEITURA\n");
		for(iForImagem=0; iForImagem<cabecalho.altura; iForImagem++){
			//printf("***************************Dentro for\n");
			ali = (cabecalho.largura * 3) % 4;

			if (ali != 0){
				ali = 4 - ali;
			}

			for(jForImagem=0; jForImagem<cabecalho.largura; jForImagem++){
				//printf("***************************antes ler\n");
				fread(&imagem[iForImagem * cabecalho.altura + jForImagem], sizeof(RGB), 1, fin);
				//printf("***************************LEu\n");
			}

			for(jForImagem=0; jForImagem<ali; jForImagem++){
				fread(&aux, sizeof(unsigned char), 1, fin);
			}
		}

	}

	MPI_Bcast(imagemSaida, (cabecalho.altura*cabecalho.largura)*sizeof(RGB), MPI_BYTE, 0, MPI_COMM_WORLD);
	MPI_Scatter(imagem,cabecalho.altura/np*cabecalho.largura*sizeof(RGB), MPI_BYTE,
		imagemAux,cabecalho.altura/np*cabecalho.largura*sizeof(RGB),MPI_BYTE,0,MPI_COMM_WORLD);
	
	//MPI_Bcast(imagem, (cabecalho.altura*cabecalho.largura)*sizeof(RGB), MPI_BYTE, 0, MPI_COMM_WORLD);
	


/*

	

	//Processar imagem
	for(iForImagem=0; iForImagem<cabecalho.altura/np; iForImagem++){
		for(jForImagem=0; jForImagem<cabecalho.largura; jForImagem++){
			if (tamanhoMascara == 3) {
				range = 1;
				posicaoMediana = 4;
			}
			else if (tamanhoMascara == 5) {
				range = 2;
				posicaoMediana = 12;
			}
			else if (tamanhoMascara == 7) {
				range = 3;
				posicaoMediana = 24;
			}

			//Calcular range do laco for para mediana
			lacoI = iForImagem-range;
			limiteI = iForImagem + range;

			lacoJ = jForImagem-range;
			limiteJ = jForImagem + range;

            if (lacoI < 0) lacoI = 0;
			if (lacoJ < 0) lacoJ = 0;

            if (limiteI > (cabecalho.altura - 1))  limiteI = (cabecalho.altura - 1);
			if (limiteJ > (cabecalho.largura - 1)) limiteJ = (cabecalho.largura - 1);

			//Limpar variaveis auxiliares
            for(iTamanhoAux2=0; iTamanhoAux2<tamanhoMascara*tamanhoMascara; iTamanhoAux2++){
                rgbAux[iTamanhoAux2].red   = 0;
                rgbAux[iTamanhoAux2].green = 0;
                rgbAux[iTamanhoAux2].blue  = 0;
			}

			rgbAux2.red   = 0;
            rgbAux2.green = 0;
            rgbAux2.blue  = 0;

            iTamanhoAux  = 0;
            iTamanhoAux2 = 0;

			//Calcular a mediana de cada pixel da imagem.
			for(i2=lacoI; i2<=limiteI; i2++){
				for(j2=lacoJ; j2<=limiteJ; j2++){
					rgbAux[iTamanhoAux].red   = imagemAux[i2][j2].red;
					rgbAux[iTamanhoAux].green = imagemAux[i2][j2].green;
					rgbAux[iTamanhoAux].blue  = imagemAux[i2][j2].blue;

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
	for(iForImagem=0; iForImagem<cabecalho.altura/np; iForImagem++){
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
*/
	fclose(fin);
	fclose(fout);

	MPI_Finalize();
}
