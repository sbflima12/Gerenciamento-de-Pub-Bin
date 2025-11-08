#ifndef ESTOQUE_H
#define ESTOQUE_H

typedef struct{
    int codigo;
    char tipo;
    char nome[20];
    float preco;
    int quantidade;
    int status;//1-Ativo e 0- inativo
}Produtos;

void menuCadastroProduto();
void menuConsultarProdutos();

void cadastrarProduto();
void mostrarEstoque();
void alterarProduto();
void excluirProduto();
int atualizarEstoque(char nomeProduto[], int quantidadeAlterar, int modo);
int obterPrecoQuantidade(const char nomeProduto[], float *precoUnitario, int *quantidadeDisponivel);
int obterPrecoQuantidadePorCodigo(int codigoBusca, float *precoUnitario, int *quantidadeDisponivel, char *nomeProduto);

#endif




