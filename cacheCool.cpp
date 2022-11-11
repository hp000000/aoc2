#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

int substitui(char *sub, int bits_val [],int n_bits_indice);

static void Help(){
	printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	printf("\t\tBem-vindo à cacheCool:\n");
	printf(" Você digitou um número incorreto de argumentos. Utilize:\n");
	printf("./cacheCool <nsets> <bsize> <assoc> <substituição> <flag_saida> arquivo_de_entrada\n");
	printf("\n  Ex.: ./cacheCool 64 8 1 R 0 bin_100.bin\n");
	printf("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
	exit(EXIT_FAILURE);
}

int main( int argc, char *argv[ ] )
{
	if (argc != 7){
		Help();
	}
	int nsets = atoi(argv[1]);
	int bsize = atoi(argv[2]);
	int assoc = atoi(argv[3]);
	char *subst = argv[4];
	int flagOut = atoi(argv[5]);
	char *arquivoEntrada = argv[6];

	int miss_compulsorio =0, n_bits_tag =0, Ncheio = 0, vazio =0;
	int endereco = 0, tag =0, indice = 0, miss =0, hit =0, i =0, miss_conflito =0, miss_capacidade=0, qtd_acessos=0;
	float taxa_hit=0.0, taxa_miss=0.0, taxa_miss_comp=0.0,taxa_miss_conf=0.0, taxa_miss_cap=0.0;
	int n_bits_offset =0, n_bits_indice =0;
	int cache_val [nsets * assoc] = {0};  //é a lista dos bits de validade da cache
	int cache_tag [nsets * assoc] = {0};  //é a lista dos bits de tag da cache
	n_bits_offset = (log2(bsize));
	n_bits_indice = (log2(nsets));
	n_bits_tag = 32 - n_bits_offset - n_bits_indice;
	//descobre o número de bits de cada parcela do endereço para todos os endereços do arquivo

	//----------------------------------------------------------
	//Variaveis para o conjunto associativo;
	int achou = 0;
	typedef struct{
		int val;
		int tag;
		int info;
	}Cache; //cria uma struct de cache pq vai ter uma posicao em cada conjunto para o mesmo indice
	Cache cache[nsets * assoc][assoc] = {0}; //inicializa tudo em 0

	//----------------------------------------------------------
	//variaveis p/ o arquivo:
	FILE *fptr;
	fptr = fopen(arquivoEntrada, "rb"); //rb  para arquivos binarios.
	//----------------------------------------------------------
	//variveis apos apresentaçao:
	int qntdLinhas = 0;
	int CA_val_assoc[nsets * assoc]={0};
	int verificaCap = 0;


	if(fptr == NULL) {
		printf("Cannot open file \n");
		exit(0); //se não puder abrir o arquivo
	}
	while (!feof(fptr))
	{
	//para ler o arq binario:
	unsigned char buffer[4]; 
	fread(buffer,sizeof(int),1,fptr);
	endereco = buffer[3]; 

	qtd_acessos++;
	
	uint32_t mask = 0xffffffff >> n_bits_tag;
	tag = endereco >> ( n_bits_offset + n_bits_indice );
	if(nsets!=1){
		indice = endereco >> n_bits_offset;
		indice = ( indice & mask );
	}else{indice =0;}
	

	if(assoc == 1){ //Mapeamento Direto
			if (cache_val[indice] == 0)
			{
				miss++; 		
				miss_compulsorio++;
				cache_val[indice] = 1;
				cache_tag[indice] = tag;
			}
			else
				if (cache_tag[indice] == tag)
					hit++;
				else 			// a pos cheia e miss
				{
					miss++;

					// conflito ou capacidade?
					while(qntdLinhas != (nsets)){          //coloquei um contador ao inves do for p/ percorrer cache
						if(cache_val[qntdLinhas] == 0){
							miss_conflito++;
							cache_val[indice] = 1;
							cache_tag[indice] = tag;
						}
						else{miss_capacidade++;
							cache_val[indice] = 1;
							cache_tag[indice] = tag;
						}
						qntdLinhas++;
					}
				}
			} 
	if (assoc != 1) { //Conjunto Associativo
		for(i=0; i<assoc; i++){
			if(cache[indice][i].info == endereco &&  cache[indice][i].tag == tag && cache[indice][i].val == 1 ){ //i representa o conjunto naquela posiçao. Ex.: Assoc = 2 temos cache[indice][0] e cache[indice][1]
				hit++;
				CA_val_assoc[i] = 1;

				//----
				break;
			}
			else 
				miss++;
				if(cache[indice][i].val == 0){
					miss_compulsorio++;
					cache[indice][i].tag = tag;
					cache[indice][i].val = 1;
					cache[indice][i].info = endereco;
					CA_val_assoc[i] = 1;
					verificaCap += 1;

					//-----
					break;
				}
				else if(cache[indice][i].val == 1){
					miss_conflito++;
					cache[indice][i].tag = tag;
					cache[indice][i].val =1;
					cache[indice][i].info = endereco;
					CA_val_assoc[i] = 1;
					verificaCap +=1;

					//-----
					break;
				}
				else if(verificaCap == assoc){
					miss_capacidade++;
					cache[indice][i].tag = tag;
					cache[indice][i].val =1;
					cache[indice][i].info = endereco;
					verificaCap =0;
					//----
					break;	
				}
			}
		}
	}
	qtd_acessos = qtd_acessos-1;//Lembrando que, o -1 pq o acesso do EOF tambem eh 1 acesso
	taxa_hit = (((float)hit/qtd_acessos)*100);
	taxa_miss = (((float)miss/qtd_acessos)*100);
	taxa_miss_cap = (((float)miss_capacidade/qtd_acessos)*100);
	taxa_miss_comp = (((float)miss_compulsorio/qtd_acessos)*100);
	taxa_miss_conf = (((float)miss_conflito/qtd_acessos)*100);

	if(flagOut==0){
		printf("-------------------------------\n");
		printf("Qntd Acessos: %d\n",qtd_acessos); 
		printf("hit: %d\n",hit);
		printf("miss: %d\n",miss);
		printf("-------------------------------\n");
		printf("Taxa de hit: %.3f\n",taxa_hit);
		printf("Taxa de miss: %.3f\n",taxa_miss);
		printf("Taxa de miss de compulsório: %.3f\n",taxa_miss_comp);
		printf("Taxa de miss de capacidade: %.3f\n",taxa_miss_cap);
		printf("Taxa de miss de conflito: %.3f\n",taxa_miss_conf);
		printf("-------------------------------\n");
	}
	else if (flagOut == 1){
		printf("%d, %.3f, %.3f, %.3f, %.3f, %.3f\n",qtd_acessos, taxa_hit/100, taxa_miss/100, taxa_miss_comp/100, taxa_miss_cap/100, taxa_miss_conf/100);
	}
	fclose(fptr);
		
	return 0;
}
int substitui(char *sub, int bits_val [],int n_bits_indice) {
	if(*sub == 'r'){
		return 1 + (rand() % sizeof(n_bits_indice));		// gera aleatório entre 1 e a quantidade total de índice   
	}
}
