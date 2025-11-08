#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "estoque.h"

#define TAMANHOBLOCO 10

FILE* abrirArquivoEstoque(int modo){
    //1:estoque-ab	2:estoque-rb    3:estoque-rb+
    FILE* arquivo=NULL;
    switch(modo){
        case 1:
            arquivo=fopen("estoque.bin", "ab");
            break;
        case 2:
            arquivo=fopen("estoque.bin", "rb");
            break;
        case 3:
            arquivo=fopen("estoque.bin", "rb+");
            break;
        default:
            printf("Modo inválido.");
    }
    
    if(arquivo==NULL){
        printf("Erro ao abrir o arquivo.\n");
    }
    
    return arquivo;
}

int atualizarEstoque(char nomeProduto[], int quantidadeAlterar, int modo) {
    // modo = 1 -> venda (subtrai)
    // modo = 2 -> reposição (soma)

    FILE *arquivo = fopen("estoque.txt", "r");
    if (arquivo == NULL) {
        printf("estoque.txt não encontrado.\n");
        return 0;
    }

    //Lê tudo em arrays dinâmicos para regravar depois
    int *codigo = NULL;
    char *tipo = NULL;
    char (*nome)[31] = NULL;
    float *preco = NULL;
    int *quantidade = NULL;
    int total = 0;
    int encontrado = 0;
    size_t capacity = 0;
    int codigoEncontrado = -1;

    while (1) {
        int cod;
        char t;
        char n[31];
        float p;
        int q;
        int read = fscanf(arquivo, "%d %c %30s %f %d", &cod, &t, n, &p, &q);
        if (read == EOF || read == 0) break;

        if (total + 1 > (int)capacity) {
            capacity = (capacity == 0) ? 10 : capacity * 2;
            codigo = realloc(codigo, capacity * sizeof(int));
            tipo = realloc(tipo, capacity * sizeof(char));
            nome = realloc(nome, capacity * sizeof(*nome));
            preco = realloc(preco, capacity * sizeof(float));
            quantidade = realloc(quantidade, capacity * sizeof(int));
            if (!codigo || !tipo || !nome || !preco || !quantidade) {
                printf("Erro de memória.\n");
                fclose(arquivo);
                return 0;
            }
        }

        codigo[total] = cod;
        tipo[total] = t;
        strncpy(nome[total], n, 31);
        preco[total] = p;
        quantidade[total] = q;

        if (strcmp(nome[total], nomeProduto) == 0) {
            encontrado = 1;
            codigoEncontrado = codigo[total];
        }

        total++;
    }
    fclose(arquivo);

    // Se não encontrado, permitir procurar pelo código
    if (!encontrado) {
        char opcao;
        printf("Produto '%s' não encontrado no estoque.\n", nomeProduto);
        printf("Deseja procurar pelo código? (s/n): ");
        scanf(" %c", &opcao);

        if (opcao == 's' || opcao == 'S') {
            int codigoBusca;
            printf("Digite o código do produto: ");
            scanf("%d", &codigoBusca);

            for (int i = 0; i < total; i++) {
                if (codigo[i] == codigoBusca) {
                    encontrado = 1;
                    codigoEncontrado = codigo[i];
                    strcpy(nomeProduto, nome[i]);
                    break;
                }
            }
        }
    }

    if (!encontrado) {
        printf("Produto não encontrado, operação cancelada.\n");
        free(codigo); free(tipo); free(nome); free(preco); free(quantidade);
        return 0;
    }

    // Atualiza o item encontrado
    for (int i = 0; i < total; i++) {
        if (codigo[i] == codigoEncontrado) {
            if (modo == 1) { // venda
                if (quantidade[i] >= quantidadeAlterar) {
                    quantidade[i] -= quantidadeAlterar;
                    printf("Produto '%s' atualizado: nova quantidade %d\n", nomeProduto, quantidade[i]);
                } else {
                    printf("Estoque insuficiente de '%s'. Quantidade disponível: %d\n", nomeProduto, quantidade[i]);
                    free(codigo); free(tipo); free(nome); free(preco); free(quantidade);
                    return -1;
                }
            } else if (modo == 2) { // reposição
                quantidade[i] += quantidadeAlterar;
                printf("Produto '%s' reabastecido: nova quantidade %d\n", nomeProduto, quantidade[i]);
            }
            break;
        }
    }

    // Regrava todo o arquivo atualizado
    arquivo = fopen("estoque.txt", "w");
    if (arquivo == NULL) {
        printf("Erro ao abrir estoque.txt para escrita.\n");
        free(codigo); free(tipo); free(nome); free(preco); free(quantidade);
        return 0;
    }

    for (int i = 0; i < total; i++) {
        fprintf(arquivo, "%d %c %s %.2f %d\n",
                codigo[i], tipo[i], nome[i], preco[i], quantidade[i]);
    }
    fclose(arquivo);

    free(codigo); free(tipo); free(nome); free(preco); free(quantidade);
    return 1;
}

//CADASTRO DE PRODUTOS----------------------------------------------------------------------------------------------
int verificarProduto(int codigo){
    FILE* arquivo=abrirArquivoEstoque(2);
    if(arquivo==NULL){
        return 0;
    }
    
    Produto blocoProdutos[TAMANHOBLOCO];
    int produtosLidos;
    
    while((produtosLidos=fread(blocoProdutos, sizeof(Produto), TAMANHOBLOCO, arquivo))>0){
        for(int i=0; i<produtosLidos; i++){
            if(blocoProdutos[i].codigo==codigo){
                fclose(arquivo);
                return 1; //1=verdadeiro, produto existe
            }
        }
    }
    fclose(arquivo);
    return 0; //0=falso, produto não existe
}

void cadastrarProduto(){
    Produto produto;
    FILE *arquivo=abrirArquivoEstoque(1); //case 1: ("estoque", "ab")
    if(arquivo==NULL){
        return;
    }
    
    printf("\n-------Cadastro de Produtos-------\n");
    printf("Digite o código do produto: ");
    scanf("%d", &produto.codigo);
    
    if(verificarProduto(produto.codigo)){
        printf("Já existe um produto com o código %d no estoque\n", produto.codigo);
        printf("Cadastro cancelado\n");
        return;
    }
    
    printf("Digite o tipo de produto, 'C' para comidas e 'B' para bebidas: ");
    scanf(" %c", &produto.tipo);
    printf("Digite o nome do produto: ");
    scanf(" %[^\n]", produto.nomeProduto);
    printf("Digite o preço do produto: ");
    scanf("%f", &produto.preco);
    printf("Digite a quantidade do produto no estoque: ");
    scanf("%d", &produto.quantidade);
    
    produto.status=1; //Se o produto acabou de ser cadastrado, ele está ativo
    
    fwrite(&produto, sizeof(Produto), 1, arquivo);
    
    fclose(arquivo);
    
    printf("\nProduto cadastrado com sucesso!\n");
}

void menuAlterarProduto(Produto *produto){
    int opcao=0;
    do{
        printf("\n\n-------------Produto Encontrado-------------\n");
        printf("Código: %d\n", produto->codigo);
        printf("Tipo: %c\n", produto->tipo);
        printf("Nome: %s\n", produto->nomeProduto);
        printf("Preço: R$%.2f\n", produto->preco);
        printf("Quantidade: %d\n", produto->quantidade);
        printf("Status: %d\n", produto->status);
        printf("----------------------------------------------------\n");
        printf("Digite o número que deseja alterar.\n");
        printf("(1) Tipo\n(2) Nome\n(3) Preço\n(4)Quantidade\n(5) Status\n(0) Salvar alterações e Voltar ao menu\n");
        printf("Opção: ");
        scanf("%d", &opcao);

        switch(opcao){
            case 1:
                printf("Digite o novo tipo de produto: ");
                scanf(" %c", &produto->tipo);
                break;
            case 2:
                printf("Digite o novo nome: ");
                scanf(" %[^\n]", produto->nomeProduto); 
                break;
            case 3:
                printf("Digite o novo preço: ");
                scanf(" %f", &produto->preco); 
                break;
            case 4:
                printf("Digite a nova quantidade do produto: ");
                scanf(" %d", &produto->quantidade);
                break;
            case 5:
                printf("Digite o novo status do produto (1-Ativo, 0-inativo):");
                scanf(" %d", &produto->status);
                break;
            case 0:
                printf("Alterações salvas.\n");
                break;
            default:
                printf("Opção inválida\n");
                break;
        }
    }while(opcao!=0);
}

void alterarProduto(){
    int codigoAlterar;
    int encontrado=0;
    
    Produto blocoProdutos[TAMANHOBLOCO];//número de produtos que serão lidos de uma vez
    int produtosLidos;

    FILE* arquivo=abrirArquivoEstoque(3); //case 4: ("estoque", "rb+")
    if(arquivo==NULL){
        return;
    }
    
    printf("\n-------------------Alterar Produto-------------------\n");
    printf("Digite o código do produto que deseja alterar: ");
    scanf("%d", &codigoAlterar);
    
    //Lê o arquivo em blocos de 'TAMANHOBLOCO'
    //'produtosLidos' armazena quantos produtos foram realmente lidos
    //Para quando 'fread' retorna 0 (fim do arquivo)
    while((produtosLidos=fread(blocoProdutos, sizeof(Produto),TAMANHOBLOCO, arquivo))>0){
        for(int i=0; i<produtosLidos; i++){
            //Procura o produto com o código desejado
            if(blocoProdutos[i].codigo==codigoAlterar){
                encontrado=1;
                //Chama o menu de alteração, passando o endereço do produto em buffer
                menuAlterarProduto(&blocoProdutos[i]);
                long int deslocamento=-((long)produtosLidos*sizeof(Produto));
                fseek(arquivo, deslocamento, SEEK_CUR);
                fwrite(blocoProdutos, sizeof(Produto), produtosLidos, arquivo);
                fclose(arquivo);
                return;
            }
        }
    }
    
    fclose(arquivo);
    if(encontrado==0){
        printf("\nProduto com o código %d não encontrado\n", codigoAlterar);
    }
}

void excluirProduto(){
    int codigoExcluir;
    int encontrado=0;
    
    Produto blocoProdutos[TAMANHOBLOCO];//número de produtos que serão lidos de uma vez
    int produtosLidos;
    
    FILE* arquivo=abrirArquivoEstoque(3); //case 3: ("estoque.bin", "rb+")
    if(arquivo==NULL){
        return;
    }
    
    printf("\n-------------------Excluir Produto-------------------\n");
    printf("Digite o código do produto que deseja inativar: ");
    scanf("%d", &codigoExcluir);
    
    while((produtosLidos=fread(blocoProdutos, sizeof(Produto),TAMANHOBLOCO, arquivo))>0){
        for(int i=0; i<produtosLidos; i++){
            //Procura o produto com o código desejado
            if(blocoProdutos[i].codigo==codigoExcluir){
                if(blocoProdutos[i].codigo==0){
                    printf("O produto já está inativado\n");
                    fclose(arquivo);
                    return;
                }
                
                encontrado=1;
                blocoProdutos[i].status=0;
                //Calcula o deslocamento do cursor dentro do arquivo
                long int deslocamento=-((long)produtosLidos*sizeof(Produto));
                fseek(arquivo, deslocamento, SEEK_CUR);
                fwrite(blocoProdutos, sizeof(Produto), produtosLidos, arquivo);
                fclose(arquivo);
                return;
            }
        }
    }
    
    fclose(arquivo);
    
    if(encontrado==0){
        printf("Produto %d não encontrado\n", codigoExcluir);
    } else if(encontrado==1){
        printf("Produto %d inativado com sucesso\n", codigoExcluir);
    }
}

void menuCadastroProduto(){
    int opcao=-1;//Alteração do nome resposta para opcao
    void (*gerenciar[])()={cadastrarProduto, alterarProduto, excluirProduto};
    //Repete a pergunta e cadastra produtos no estoque e 
    //volta para o menu inicial quando o usuário digita 0
    while (opcao!=0) {
        printf("\n\n-------Menu de Cadastro-------\n");
        printf("(1) Cadastrar um produto\n");
        printf("(2) Alterar o produto\n");
        printf("(3) Inativar o produto\n");
        printf("(0) Voltar ao menu inicial\n");
        printf("Opção: ");
        scanf("%d", &opcao);
        if(opcao==0){ //Corrigido o problema de pilha que pode ocorrer colocando menuInicial dentro do array
            printf("Retornando ao menu inicial\n");
            break;
        }
        if((opcao<=3)&&(opcao>0)){
            gerenciar[opcao-1]();
        } else{
            printf("Resposta inválida\n");
        }
    }
}

//CONSULTA DE PRODUTOS (LISTA)---------------------------------------------------------------------------------------------------
//listar todos os itens do estoque
void listarTodos()
{
    FILE* arquivo;
    int codigo, quantidade;
    char tipo;
    float preco;
    char nome[31];
    arquivo = fopen ("estoque.txt", "r");
    if (arquivo == NULL)
    {
        perror("Erro ao abrir o arquivo!"); //mensagem de erro caso o arquivo não exista
    }else {
        while (fscanf(arquivo, "%d %c %s %f %d", &codigo, &tipo, nome, &preco, &quantidade) != EOF) {
            printf("%d %c %s %.2f %d\n", codigo, tipo, nome, preco, quantidade);
        }
    }

    fclose(arquivo);
}

//listar apenas as bebidas do estoque
void listarBebidas(){
    FILE* arquivo;
    int codigo, quantidade;
    char tipo;
    float preco;
    char nome[31];
    arquivo = fopen ("estoque.txt", "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo!"); //mensagem de erro caso o arquivo não exista
    } else {
        while (fscanf(arquivo, "%d %c %30s %f %d", &codigo, &tipo, nome, &preco, &quantidade) != EOF) {
            if(tipo == 'B' || tipo == 'b') {
                printf("%d %s %.2f %d\n", codigo, nome, preco, quantidade);
            }
        }
    }

    fclose(arquivo);
}

//listar todas as comidas do estoque
void listarComidas(){
    FILE* arquivo;
    int codigo, quantidade;
    char tipo;
    float preco;
    char nome[31];
    arquivo = fopen ("estoque.txt", "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo!"); //mensagem de erro caso o arquivo não exista
    } else {
        while (fscanf(arquivo, "%d %c %30s %f %d", &codigo, &tipo, nome, &preco, &quantidade) != EOF) {
            if(tipo == 'C' || tipo == 'c') {
                printf("%d %s %.2f %d\n", codigo, nome, preco, quantidade);
            }
        }
    }

    fclose(arquivo);
}

void consultarProdutoPorCodigo(){
    FILE* arquivo;
    int codigo, quantidade, procuraCodigo, encontrado = 0;
    char tipo;
    float preco;
    char nome[31];
    printf("\nDigite o código do produto: ");
    scanf("%d",&procuraCodigo);
    arquivo = fopen ("estoque.txt", "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo!"); //mensagem de erro caso o arquivo não exista
    } else {
        while (fscanf(arquivo, "%d %c %30s %f %d", &codigo, &tipo, nome, &preco, &quantidade) != EOF) {
            if(procuraCodigo == codigo) {
                printf("%d %c %s R$%.2f %d(unidades)\n", codigo, tipo, nome, preco, quantidade);
                encontrado = 1;
                break;
            }
            if (!encontrado){
                printf("Código não encontrado!\n");
            }
        }
    }

    fclose(arquivo);

}

//função para usuário escolher qual lista quer consultar
void menuConsultarProdutos() {
    int resposta;
    void (*gerenciar[])()={menuInicial, listarTodos, listarBebidas, listarComidas, consultarProdutoPorCodigo};
    printf("------------MENU DE CONSULTA------------\n");
    printf("Qual lista você deseja consultar?\n(1) Lista de todos os produtos\n(2) Lista de bebidas\n(3) Lista de comidas\n(4) Consultar por código\n(0) Voltar ao Menu Inicial\n");
    printf("Consultar: ");
    scanf("%d",&resposta);
    printf("\n----------------------------------------\n");
    if(resposta<=4 && resposta>=0){
        gerenciar[resposta]();
    } else{
        printf("Resposta inválida\n");
    }
}

int obterPrecoQuantidade(const char nomeProduto[], float *precoUnitario, int *quantidadeDisponivel) {
    FILE *arquivo = fopen("estoque.txt", "r");
    if (!arquivo) return 0;
    int codigo;
    char tipo;
    char nome[31];
    float preco;
    int quantidade;
    while (fscanf(arquivo, "%d %c %30s %f %d", &codigo, &tipo, nome, &preco, &quantidade) != EOF) {
        if (strcmp(nome, nomeProduto) == 0) {
            *precoUnitario = preco;
            *quantidadeDisponivel = quantidade;
            fclose(arquivo);
            return 1;
        }
    }
    fclose(arquivo);
    return 0;
}

int obterPrecoQuantidadePorCodigo(int codigoBusca, float *precoUnitario, int *quantidadeDisponivel, char *nomeProduto) {
    FILE *arquivo = fopen("estoque.txt", "r");
    if (!arquivo) return 0;

    int codigo;
    char tipo;
    char nome[31];
    float preco;
    int quantidade;

    while (fscanf(arquivo, "%d %c %30s %f %d", &codigo, &tipo, nome, &preco, &quantidade) != EOF) {
        if (codigo == codigoBusca) {
            *precoUnitario = preco;
            *quantidadeDisponivel = quantidade;
            strcpy(nomeProduto, nome);
            fclose(arquivo);
            return 1;
        }
    }

    fclose(arquivo);
    return 0;
}

