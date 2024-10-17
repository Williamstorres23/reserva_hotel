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
    int num_diarias;        // Quantidade de diárias
    float preco;            // Preço total da reserva
} Reserva;

typedef struct No {
    Reserva reserva;
    struct No* prox; 
    struct No* ant;  
} No;

// Função para inicializar a lista
No* inicializarLista() {
    return NULL;
}

// Função para contar o número de reservas na lista
int contarReservas(No* lista) {
    int count = 0;
    No* temp = lista;
    while (temp != NULL) {
        count++;
        temp = temp->prox;
    }
    return count;
}

// Função para verificar se um ano é bissexto
int isAnoBissexto(int ano) {
    return (ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0);
}

// Função para retornar o número de dias em um mês específico
int diasNoMes(int mes, int ano) {
    switch (mes) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12: return 31;
        case 4: case 6: case 9: case 11: return 30;
        case 2: return isAnoBissexto(ano) ? 29 : 28;
        default: return 0; // Mês inválido
    }
}

// Função para verificar se uma data é válida
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

// Função para verificar se um quarto está disponível
int isRoomAvailable(No* lista, int room) {
    No* temp = lista;
    while(temp != NULL) {
        if(temp->reserva.id == room) {
            return 0; // Não disponível
        }
        temp = temp->prox;
    }
    return 1; // Disponível
}

// Função para listar os quartos disponíveis em formato de tabela (5 colunas)
void listarQuartosDisponiveis(No* lista) {
    printf("\nQuartos Disponíveis:\n");
    int available = 0;

    for (int i = 1; i <= QUARTOS_DISPONIVEIS; i++) {
        if (isRoomAvailable(lista, i)) {
            printf("Quarto %02d\t", i); // Exibe o quarto com dois dígitos (01, 02, ... 50)
            available++;

            if (available % 5 == 0) printf("\n"); // Quebra a linha a cada 5 quartos
        }
    }
    if(available == 0) {
        printf("Nenhum quarto disponível.\n");
    } else if (available % 5 != 0) {
        printf("\n"); // Adiciona uma nova linha ao final para melhorar a organização
    }
}

// Função para inserir uma nova reserva na lista
// Parâmetro adicional 'showMessage' controla a exibição da mensagem de confirmação
No* inserirReserva(No* lista, Reserva nova_reserva, int showMessage) {
    // Calcula o preço total
    nova_reserva.preco = PRECO_RESERVA * nova_reserva.num_pessoas * nova_reserva.num_diarias;

    No* novo_no = (No*) malloc(sizeof(No));
    if (novo_no == NULL) {
        printf("Erro de alocação de memória!\n");
        exit(1);
    }
    novo_no->reserva = nova_reserva;
    novo_no->prox = NULL;
    novo_no->ant = NULL;

    if (lista == NULL) {  
        if(showMessage){
            printf("Reserva realizada com sucesso! ID (Quarto): %d, Total a pagar: R$ %.2f\n", 
                   nova_reserva.id, nova_reserva.preco);
        }
        return novo_no;
    } else {
        No* temp_insert = lista;
        while (temp_insert->prox != NULL) {
            temp_insert = temp_insert->prox;
        }
        temp_insert->prox = novo_no;
        novo_no->ant = temp_insert;
        if(showMessage){
            printf("Reserva realizada com sucesso! ID (Quarto): %d, Total a pagar: R$ %.2f\n", 
                   nova_reserva.id, nova_reserva.preco);
        }
        return lista;
    }
}

// Função para remover uma reserva da lista pelo ID
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
        printf("Reserva com ID (Quarto) %d não encontrada!\n", id);
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
        printf("Reserva com ID (Quarto) %d removida com sucesso!\n", id);
    }
    return lista;
}

// Função para buscar uma reserva pelo ID
void buscarReserva(No* lista, int id) {
    No* temp = lista;
    while (temp != NULL && temp->reserva.id != id) {
        temp = temp->prox;
    }

    if (temp == NULL) {
        printf("Reserva com ID (Quarto) %d não encontrada!\n", id);
    } else {
        printf("\nReserva Encontrada:\n");
        printf("ID (Quarto): %d\nCliente: %s\nData: %s\nNúmero de Pessoas: %d\nNúmero de Diárias: %d\nPreço: R$ %.2f\n",
               temp->reserva.id, temp->reserva.nome_cliente, temp->reserva.data, 
               temp->reserva.num_pessoas, temp->reserva.num_diarias, temp->reserva.preco);
    }
}

// ### INÍCIO DAS FUNÇÕES DE MERGE SORT ###

// Função para comparar duas datas
int compareDates(const char* d1, const char* d2) {
    int dia1, mes1, ano1;
    int dia2, mes2, ano2;
    sscanf(d1, "%d/%d/%d", &dia1, &mes1, &ano1);
    sscanf(d2, "%d/%d/%d", &dia2, &mes2, &ano2);

    if (ano1 < ano2) return -1;
    if (ano1 > ano2) return 1;
    if (mes1 < mes2) return -1;
    if (mes1 > mes2) return 1;
    if (dia1 < dia2) return -1;
    if (dia1 > dia2) return 1;
    return 0;
}

// Função para dividir a lista em duas metades
void splitList(No* source, No** frontRef, No** backRef) {
    No* fast;
    No* slow;
    slow = source;
    fast = source->prox;

    // Avança 'fast' dois nós e 'slow' um nó
    while (fast != NULL) {
        fast = fast->prox;
        if (fast != NULL) {
            slow = slow->prox;
            fast = fast->prox;
        }
    }

    // 'slow' está no ponto de divisão
    *frontRef = source;
    *backRef = slow->prox;
    slow->prox = NULL;
    if (*backRef != NULL) {
        (*backRef)->ant = NULL;
    }
}

// Função para mesclar duas listas ordenadas
No* sortedMerge(No* a, No* b) {
    if (a == NULL)
        return b;
    if (b == NULL)
        return a;

    No* result = NULL;

    // Compara as datas e seleciona o nó menor
    if (compareDates(a->reserva.data, b->reserva.data) <= 0) {
        result = a;
        result->prox = sortedMerge(a->prox, b);
        if (result->prox != NULL)
            result->prox->ant = result;
        result->ant = NULL;
    }
    else {
        result = b;
        result->prox = sortedMerge(a, b->prox);
        if (result->prox != NULL)
            result->prox->ant = result;
        result->ant = NULL;
    }
    return result;
}

// Função principal do Merge Sort
void mergeSort(No** headRef) {
    No* head = *headRef;
    if (head == NULL || head->prox == NULL)
        return;

    No* a;
    No* b;

    // Divide a lista em 'a' e 'b'
    splitList(head, &a, &b);

    // Ordena recursivamente as duas sub-listas
    mergeSort(&a);
    mergeSort(&b);

    // Mescla as sub-listas ordenadas
    *headRef = sortedMerge(a, b);
}

// ### FIM DAS FUNÇÕES DE MERGE SORT ###

// Função para exibir todas as reservas ordenadas por data
void exibirReservas(No** listaRef) {
    No* lista = *listaRef;
    if (lista == NULL) {
        printf("\nNenhuma reserva cadastrada.\n");
        return;
    }

    // Ordena a lista por data usando Merge Sort
    mergeSort(listaRef);

    // Atualiza a referência da lista após a ordenação
    lista = *listaRef;

    printf("\n--- Lista de Reservas Ordenadas por Data ---\n");
    while (lista != NULL) {
        printf("ID (Quarto): %d\tCliente: %s\tData: %s\tPessoas: %d\tDiárias: %d\tPreço: R$ %.2f\n", 
               lista->reserva.id, lista->reserva.nome_cliente, lista->reserva.data, 
               lista->reserva.num_pessoas, lista->reserva.num_diarias, lista->reserva.preco);
        lista = lista->prox;
    }
    printf("\n");
}

// Função para liberar a memória alocada para a lista
void liberarLista(No* lista) {
    No* temp;
    while (lista != NULL) {
        temp = lista;
        lista = lista->prox;
        free(temp);
    }
}

// Função para limpar o terminal
void limpar_terminal() {
    #ifdef _WIN32
        system("cls"); // Para Windows
    #else
        system("clear"); // Para Linux/Mac
    #endif
}

// Função para adicionar reservas iniciais
No* adicionarReservasIniciais(No* lista) {
    Reserva r1 = {5, "Alice Santos", "15/11/2024", 2, 3, 0.0};
    Reserva r2 = {10, "Bruno Lima", "20/12/2024", 4, 2, 0.0};
    Reserva r3 = {15, "Carla Oliveira", "05/01/2025", 3, 5, 0.0};
    Reserva r4 = {20, "Daniel Costa", "25/12/2024", 1, 1, 0.0};
    Reserva r5 = {25, "Eliana Martins", "10/11/2024", 2, 4, 0.0};

    lista = inserirReserva(lista, r1, 0);
    lista = inserirReserva(lista, r2, 0);
    lista = inserirReserva(lista, r3, 0);
    lista = inserirReserva(lista, r4, 0);
    lista = inserirReserva(lista, r5, 0);

    return lista;
}

int main() {
    No* lista = inicializarLista();

    // Adiciona reservas iniciais sem exibir mensagens
    lista = adicionarReservasIniciais(lista);

    int opcao, id, selected_room;
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
                // Listar quartos disponíveis
                listarQuartosDisponiveis(lista);

                // Verificar se há quartos disponíveis
                {
                    int total_disponiveis = 0;
                    for(int i =1; i <= QUARTOS_DISPONIVEIS; i++) {
                        if(isRoomAvailable(lista, i)) {
                            total_disponiveis++;
                        }
                    }
                    if(total_disponiveis == 0) {
                        printf("Não há quartos disponíveis para reserva.\n");
                        break;
                    }
                }

                // Solicitar seleção do quarto
                printf("Selecione o número do quarto desejado: ");
                scanf("%d", &selected_room);

                // Validar seleção do quarto
                if(selected_room < 1 || selected_room > QUARTOS_DISPONIVEIS) {
                    printf("Número de quarto inválido. Deve ser entre 1 e %d.\n", QUARTOS_DISPONIVEIS);
                    break;
                }

                if(!isRoomAvailable(lista, selected_room)) {
                    printf("Quarto %d já está reservado. Escolha outro quarto.\n", selected_room);
                    break;
                }

                // Atribuir o quarto selecionado
                nova_reserva.id = selected_room;

                // Solicitar outras informações
                printf("Nome do cliente: ");
                scanf(" %[^\n]", nova_reserva.nome_cliente);
                printf("Data (DD/MM/AAAA): ");
                scanf("%s", nova_reserva.data);
                printf("Número de pessoas: ");
                scanf("%d", &nova_reserva.num_pessoas);
                printf("Número de diárias: ");
                scanf("%d", &nova_reserva.num_diarias);

                // Validar data
                if (!verificarDataValida(nova_reserva.data)) {
                    printf("Data inválida! A data deve ser hoje ou uma data futura.\n");
                    break;
                }

                // Validar capacidade
                {
                    int totalReservas = contarReservas(lista);
                    int totalPessoas = 0;
                    No* temp = lista;
                    while (temp != NULL) {
                        totalPessoas += temp->reserva.num_pessoas;
                        temp = temp->prox;
                    }

                    if (totalReservas >= QUARTOS_DISPONIVEIS) {
                        printf("Não há quartos disponíveis para nova reserva.\n");
                        break;
                    }

                    if (totalPessoas + nova_reserva.num_pessoas > CAPACIDADE_TOTAL) {
                        printf("Capacidade total de pessoas excedida. Limite é %d.\n", CAPACIDADE_TOTAL);
                        break;
                    }
                }

                // Inserir a reserva com exibição de mensagem
                lista = inserirReserva(lista, nova_reserva, 1);
                break;

            case 2:
                printf("--- Remover Reserva ---\n");
                printf("ID (Quarto) da reserva a remover: ");
                scanf("%d", &id);
                lista = removerReserva(lista, id);
                break;

            case 3:
                printf("--- Buscar Reserva ---\n");
                printf("ID (Quarto) da reserva a buscar: ");
                scanf("%d", &id);
                buscarReserva(lista, id);
                break;

            case 4:
                exibirReservas(&lista); // Passa o endereço da lista para ordenar
                break;

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
