#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define QUARTOS_DISPONIVEIS 50
#define CAPACIDADE_TOTAL 100
#define PRECO_RESERVA 250.0 // Preço por pessoa por diária

typedef struct {
    int id;                
    char nome_cliente[100]; 
    char data[11];          
    int num_pessoas;        
    int num_diarias;        // Novo campo: quantidade de diárias
    float preco;            // Preço total da reserva
} Reserva;

typedef struct No {
    Reserva reserva;
    struct No* prox; 
    struct No* ant;  
} No;

No* inicializarLista() {
    return NULL;
}

int contarReservas(No* lista) {
    int count = 0;
    No* temp = lista;
    while (temp != NULL) {
        count++;
        temp = temp->prox;
    }
    return count;
}

int isAnoBissexto(int ano) {
    return (ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0);
}

int diasNoMes(int mes, int ano) {
    switch (mes) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12: return 31;
        case 4: case 6: case 9: case 11: return 30;
        case 2: return isAnoBissexto(ano) ? 29 : 28;
        default: return 0; // Mês inválido
    }
}

int verificarDataValida(const char* data_str) {
    int dia, mes, ano;
    sscanf(data_str, "%d/%d/%d", &dia, &mes, &ano);

    if (ano < 1900 || ano > 2100) return 0;
    if (mes < 1 || mes > 12) return 0;
    if (dia < 1 || dia > diasNoMes(mes, ano)) return 0;

    time_t agora = time(NULL);
    struct tm* hoje_tm = localtime(&agora);

    int dia_hoje = hoje_tm->tm_mday;
    int mes_hoje = hoje_tm->tm_mon + 1;  // Meses vão de 0 a 11
    int ano_hoje = hoje_tm->tm_year + 1900;

    if (ano < ano_hoje) return 0;
    if (ano == ano_hoje) {
        if (mes < mes_hoje) return 0;
        if (mes == mes_hoje && dia < dia_hoje) return 0;
    }

    return 1;
}

No* inserirReserva(No* lista, Reserva nova_reserva) {
    No* temp = lista;
    while (temp != NULL) {
        if (temp->reserva.id == nova_reserva.id) {
            printf("ID %d já existe! Insira um ID único.\n", nova_reserva.id);
            return lista;
        }
        temp = temp->prox;
    }

    int totalReservas = contarReservas(lista);
    int totalPessoas = 0;
    temp = lista;
    while (temp != NULL) {
        totalPessoas += temp->reserva.num_pessoas;
        temp = temp->prox;
    }

    if (totalReservas >= QUARTOS_DISPONIVEIS) {
        printf("Não há quartos disponíveis para nova reserva.\n");
        return lista;
    }

    if (totalPessoas + nova_reserva.num_pessoas > CAPACIDADE_TOTAL) {
        printf("Capacidade total de pessoas excedida. Limite é %d.\n", CAPACIDADE_TOTAL);
        return lista;
    }

    if (!verificarDataValida(nova_reserva.data)) {
        printf("Data inválida! A data deve ser hoje ou uma data futura.\n");
        return lista;
    }

    // Calcula o preço total
    nova_reserva.preco = PRECO_RESERVA * nova_reserva.num_pessoas * nova_reserva.num_diarias;

    No* novo_no = (No*) malloc(sizeof(No));
    novo_no->reserva = nova_reserva;
    novo_no->prox = NULL;
    novo_no->ant = NULL;

    if (lista == NULL) {  
        printf("Reserva realizada com sucesso! Total a pagar: R$ %.2f\n", nova_reserva.preco);
        return novo_no;
    } else {
        No* temp = lista;
        while (temp->prox != NULL) {
            temp = temp->prox;
        }
        temp->prox = novo_no;
        novo_no->ant = temp;
        printf("Reserva realizada com sucesso! Total a pagar: R$ %.2f\n", nova_reserva.preco);
        return lista;
    }
}

No* removerReserva(No* lista, int id) {
    if (lista == NULL) {
        printf("A lista está vazia!\n");
        return NULL;
    }

    No* temp = lista;
    while (temp != NULL && temp->reserva.id != id) {
        temp = temp->prox;
    }

    if (temp == NULL) {
        printf("Reserva com ID %d não encontrada!\n", id);
    } else {
        if (temp->ant != NULL) {
            temp->ant->prox = temp->prox;
        } else {
            lista = temp->prox;
        }

        if (temp->prox != NULL) {
            temp->prox->ant = temp->ant;
        }

        free(temp);
        printf("Reserva com ID %d removida com sucesso!\n", id);
    }
    return lista;
}

void buscarReserva(No* lista, int id) {
    No* temp = lista;
    while (temp != NULL && temp->reserva.id != id) {
        temp = temp->prox;
    }

    if (temp == NULL) {
        printf("Reserva com ID %d não encontrada!\n", id);
    } else {
        printf("Reserva encontrada:\n");
        printf("ID: %d\nCliente: %s\nData: %s\nNúmero de Pessoas: %d\nNúmero de Diárias: %d\nPreço: %.2f\n",
               temp->reserva.id, temp->reserva.nome_cliente, temp->reserva.data, 
               temp->reserva.num_pessoas, temp->reserva.num_diarias, temp->reserva.preco);
    }
}

void exibirReservas(No* lista) {
    No* temp = lista;
    if (temp == NULL) {
        printf("Nenhuma reserva cadastrada.\n");
        return;
    }

    printf("\n--- Lista de Reservas ---\n");
    while (temp != NULL) {
        printf("ID: %d, Cliente: %s, Data: %s, Pessoas: %d, Diárias: %d, Preço: %.2f\n", 
               temp->reserva.id, temp->reserva.nome_cliente, temp->reserva.data, 
               temp->reserva.num_pessoas, temp->reserva.num_diarias, temp->reserva.preco);
        temp = temp->prox;
    }
    printf("\n");
}

void liberarLista(No* lista) {
    No* temp;
    while (lista != NULL) {
        temp = lista;
        lista = lista->prox;
        free(temp);
    }
}


void limpar_terminal() {
    #ifdef _WIN32
        system("cls"); // Para Windows
    #else
        system("clear"); // Para Linux/Mac
    #endif
}

int main() {
    No* lista = inicializarLista();
    int opcao, id;
    Reserva nova_reserva;
    
    do {
        printf("\n--- Sistema de Reservas ---\n");
        printf("1. Inserir reserva\n");
        printf("2. Remover reserva\n");
        printf("3. Buscar reserva\n");
        printf("4. Exibir todas as reservas\n");
        printf("5. Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);
        limpar_terminal();
        
        switch (opcao) {
            case 1:
                printf("\n--- Tabela de Preços ---\n");
                printf("Preço da diária por pessoa: R$ %.2f\n", PRECO_RESERVA);
                
                printf("\n--- Cadastrar Reserva ---\n");
                printf("\nID: ");
                scanf("%d", &nova_reserva.id);
                printf("Nome do cliente: ");
                scanf(" %[^\n]", nova_reserva.nome_cliente);
                printf("Data (DD/MM/AAAA): ");
                scanf("%s", nova_reserva.data);
                printf("Número de pessoas: ");
                scanf("%d", &nova_reserva.num_pessoas);
                printf("Número de diárias: ");
                scanf("%d", &nova_reserva.num_diarias);

                lista = inserirReserva(lista, nova_reserva);
                break;
                limpar_terminal();
            case 2:
                printf("--- Remover Reserva ---\n");
                printf("ID da reserva a remover: ");
                scanf("%d", &id);
                lista = removerReserva(lista, id);
                break;
                limpar_terminal();
            case 3:
                printf("--- Buscar Reserva ---\n");
                printf("ID da reserva a buscar: ");
                scanf("%d", &id);
                buscarReserva(lista, id);
                break;
                
            case 4:
                exibirReservas(lista);
                break;
                limpar_terminal();
            case 5:
                printf("Saindo...\n");
                liberarLista(lista);
                break;
            default:
                printf("Opção inválida!\n");
        }
    } while (opcao != 5);

    return 0;
}
