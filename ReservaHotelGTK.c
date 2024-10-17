// ReservaHotelGTK.c
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

// Definições e Estruturas
#define QUARTOS_DISPONIVEIS 50
#define CAPACIDADE_TOTAL 100
#define PRECO_RESERVA 250.0 // Preço por pessoa por diária

typedef struct {
    int reserva_id;        // ID único da reserva
    int quarto_id;         // ID do quarto
    char nome_cliente[100];
    char data_inicio[11];  // Formato DD/MM/AAAA
    int num_diarias;
    int num_pessoas;
    float preco;
} Reserva;

typedef struct No {
    Reserva reserva;
    struct No* prox;
    struct No* ant;
} No;

// Definição da Estrutura AppWidgets
typedef struct {
    GtkWidget *window;
    GtkWidget *treeview;
} AppWidgets;

// Protótipos das Funções
No* inicializarLista();
int verificarDataValida(const char* data_str);
int isAnoBissexto(int ano);
int diasNoMes(int mes, int ano);
struct tm parse_date(const char* date_str);
struct tm add_days(struct tm date, int days);
int compare_dates(struct tm date1, struct tm date2);
int ranges_overlap(struct tm start1, int days1, struct tm start2, int days2);
int compare_reserva_dates(Reserva a, Reserva b);
No* split_list(No* head);
No* sorted_merge(No* first, No* second);
No* merge_sort(No* head);
int isRoomAvailable(No* lista, int room, struct tm new_start, int new_days);
No* inserirReserva(No* lista, Reserva nova_reserva);
No* removerReserva(No* lista, int id);
Reserva* buscarReserva(No* lista, int id);
int contarTotalPessoas(No* lista);
void adicionarReservasIniciais(No* lista);
void atualizarTreeView(AppWidgets *widgets);
void on_insert_reserva(GtkWidget *widget, gpointer data);
void on_remove_reserva(GtkWidget *widget, gpointer data);
void on_search_reserva(GtkWidget *widget, gpointer data);
void on_display_reservas(GtkWidget *widget, gpointer data);
GtkWidget* criar_tree_view();
void aplicar_css(GtkWidget *widget, const char *arquivo_css);

// Variáveis Globais
No* lista_reservas = NULL;
int current_id = 1;

// Implementação das Funções

No* inicializarLista() {
    return NULL;
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

// Função para parsear uma string de data no formato "DD/MM/AAAA" para struct tm
struct tm parse_date(const char* date_str) {
    struct tm date = {0};
    sscanf(date_str, "%d/%d/%d", &date.tm_mday, &date.tm_mon, &date.tm_year);
    date.tm_mon -= 1; // tm_mon é 0-based
    date.tm_year -= 1900; // tm_year é anos desde 1900
    return date;
}

// Função para adicionar dias a uma struct tm
struct tm add_days(struct tm date, int days) {
    date.tm_mday += days;
    mktime(&date); // Normaliza a data
    return date;
}

// Função para comparar duas datas. Retorna 1 se date1 <= date2, 0 caso contrário
int compare_dates(struct tm date1, struct tm date2) {
    time_t t1 = mktime(&date1);
    time_t t2 = mktime(&date2);
    return difftime(t1, t2) <= 0;
}

// Função para verificar se dois períodos de reserva se sobrepõem
int ranges_overlap(struct tm start1, int days1, struct tm start2, int days2) {
    struct tm end1 = add_days(start1, days1 - 1);
    struct tm end2 = add_days(start2, days2 - 1);
    
    // Períodos se sobrepõem se start1 <= end2 e start2 <= end1
    return (compare_dates(start1, end2) && compare_dates(start2, end1));
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

int compare_reserva_dates(Reserva a, Reserva b) {
    struct tm date_a = parse_date(a.data_inicio);
    struct tm date_b = parse_date(b.data_inicio);
    
    time_t t_a = mktime(&date_a);
    time_t t_b = mktime(&date_b);
    
    double diff = difftime(t_a, t_b);
    
    if (diff < 0)
        return -1;
    else if (diff > 0)
        return 1;
    else
        return 0;
}

// Função para dividir a lista em duas metades
No* split_list(No* head) {
    No* slow = head;
    No* fast = head;
    
    while (fast->prox && fast->prox->prox) {
        slow = slow->prox;
        fast = fast->prox->prox;
    }
    
    No* mid = slow->prox;
    slow->prox = NULL;
    if (mid)
        mid->ant = NULL;
    
    return mid;
}

// Função para mesclar duas listas ordenadas
No* sorted_merge(No* first, No* second) {
    if (!first)
        return second;
    if (!second)
        return first;
    
    // Comparar as datas das reservas
    if (compare_reserva_dates(first->reserva, second->reserva) <= 0) {
        first->prox = sorted_merge(first->prox, second);
        if (first->prox)
            first->prox->ant = first;
        first->ant = NULL;
        return first;
    } else {
        second->prox = sorted_merge(first, second->prox);
        if (second->prox)
            second->prox->ant = second;
        second->ant = NULL;
        return second;
    }
}

// Função recursiva Merge Sort
No* merge_sort(No* head) {
    if (!head || !head->prox)
        return head;
    
    // Dividir a lista em duas metades
    No* second = split_list(head);
    
    // Ordenar recursivamente cada metade
    head = merge_sort(head);
    second = merge_sort(second);
    
    // Mesclar as duas metades ordenadas
    return sorted_merge(head, second);
}

int isRoomAvailable(No* lista, int room, struct tm new_start, int new_days) {
    No* temp = lista;
    while(temp != NULL) {
        if(temp->reserva.quarto_id == room) {
            struct tm existing_start = parse_date(temp->reserva.data_inicio);
            int existing_days = temp->reserva.num_diarias;
            if(ranges_overlap(existing_start, existing_days, new_start, new_days)) {
                return 0; // Não disponível
            }
        }
        temp = temp->prox;
    }
    return 1; // Disponível
}

No* inserirReserva(No* lista, Reserva nova_reserva) {
    // Calcula o preço total
    nova_reserva.preco = PRECO_RESERVA * nova_reserva.num_pessoas * nova_reserva.num_diarias;
    nova_reserva.reserva_id = current_id++;

    No* novo_no = (No*) malloc(sizeof(No));
    if (novo_no == NULL) {
        printf("Erro de alocação de memória!\n");
        exit(1);
    }
    novo_no->reserva = nova_reserva;
    novo_no->prox = NULL;
    novo_no->ant = NULL;

    if (lista == NULL) {  
        return novo_no;
    } else {
        No* temp_insert = lista;
        while (temp_insert->prox != NULL) {
            temp_insert = temp_insert->prox;
        }
        temp_insert->prox = novo_no;
        novo_no->ant = temp_insert;
        return lista;
    }
}

No* removerReserva(No* lista, int id) {
    if (lista == NULL) {
        return NULL;
    }

    No* temp = lista;
    while (temp != NULL && temp->reserva.reserva_id != id) {
        temp = temp->prox;
    }

    if (temp == NULL) {
        return lista;
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
        return lista;
    }
}

Reserva* buscarReserva(No* lista, int id) {
    No* temp = lista;
    while (temp != NULL && temp->reserva.reserva_id != id) {
        temp = temp->prox;
    }

    if (temp == NULL) {
        return NULL;
    } else {
        return &temp->reserva;
    }
}

int contarTotalPessoas(No* lista) {
    int totalPessoas = 0;
    No* temp = lista;
    while (temp != NULL) {
        totalPessoas += temp->reserva.num_pessoas;
        temp = temp->prox;
    }
    return totalPessoas;
}

void adicionarReservasIniciais(No* lista) {
    Reserva r1 = {0, 5, "Alice Santos", "15/11/2024", 3, 2, 0.0};
    Reserva r2 = {0, 10, "Bruno Lima", "20/12/2024", 2, 4, 0.0};
    Reserva r3 = {0, 15, "Carla Oliveira", "05/01/2025", 5, 3, 0.0};
    Reserva r4 = {0, 20, "Daniel Costa", "25/12/2024", 1, 1, 0.0};
    Reserva r5 = {0, 25, "Eliana Martins", "10/11/2024", 2, 4, 0.0};

    lista = inserirReserva(lista, r1);
    lista = inserirReserva(lista, r2);
    lista = inserirReserva(lista, r3);
    lista = inserirReserva(lista, r4);
    lista = inserirReserva(lista, r5);

    lista_reservas = lista;
}

// Função para Configurar a TreeView
GtkWidget* criar_tree_view() {
    GtkListStore *store;
    GtkTreeViewColumn *col;
    GtkCellRenderer *renderer;

    // Cria a lista (modelo) com os tipos de dados
    // Alterado o último tipo para G_TYPE_STRING para exibir preço formatado
    store = gtk_list_store_new(7, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING);

    // Cria a TreeView e associa o modelo
    GtkWidget *treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(store);

    // Define os títulos das colunas
    const char *titles[] = {"ID", "Quarto", "Cliente", "Data de Início", "Pessoas", "Diárias", "Preço (R$)"};
    for(int i = 0; i < 7; i++) {
        renderer = gtk_cell_renderer_text_new();
        col = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);
    }

    return treeview;
}

// Função para Atualizar a TreeView com as reservas ordenadas
void atualizarTreeView(AppWidgets *widgets) {
    GtkListStore *store;
    GtkTreeIter iter;
    Reserva *reserva;
    
    // Ordenar a lista de reservas usando Merge Sort
    lista_reservas = merge_sort(lista_reservas);
    
    No* temp = lista_reservas;

    // Obtém o modelo da TreeView
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(widgets->treeview));
    store = GTK_LIST_STORE(model);

    // Limpa a lista existente
    gtk_list_store_clear(store);

    // Itera sobre as reservas e adiciona à TreeView
    while (temp != NULL) {
        reserva = &temp->reserva;
        gtk_list_store_append(store, &iter);
        
        // Formata o preço com duas casas decimais e vírgula
        char preco_str[20];
        snprintf(preco_str, sizeof(preco_str), "R$%.2f", reserva->preco);
        // Substitui ponto por vírgula
        for(int i = 0; preco_str[i] != '\0'; i++) {
            if(preco_str[i] == '.') {
                preco_str[i] = ',';
                break;
            }
        }

        gtk_list_store_set(store,
                           &iter,
                           0, reserva->reserva_id,
                           1, reserva->quarto_id,
                           2, reserva->nome_cliente,
                           3, reserva->data_inicio,
                           4, reserva->num_pessoas,
                           5, reserva->num_diarias,
                           6, preco_str, // Alterado para string
                           -1);
        temp = temp->prox;
    }
}

// Callback para Inserir Reserva
void on_insert_reserva(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;

    GtkWidget *dialog, *content_area;
    GtkWidget *grid;
    GtkWidget *label_nome, *entry_nome;
    GtkWidget *label_quarto_num, *combo_quarto;
    GtkWidget *label_pessoas, *spin_pessoas;
    GtkWidget *label_diarias, *spin_diarias;
    GtkWidget *label_data;
    GtkWidget *calendar;
    gint response;

    dialog = gtk_dialog_new_with_buttons("Inserir Reserva",
                                         GTK_WINDOW(widgets->window),
                                         GTK_DIALOG_MODAL,
                                         ("_Cancelar"),
                                         GTK_RESPONSE_CANCEL,
                                         ("_Confirmar"),
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    // Nome do Cliente
    label_nome = gtk_label_new("Nome do Cliente:");
    gtk_widget_set_halign(label_nome, GTK_ALIGN_END);
    entry_nome = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), label_nome, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_nome, 1, 0, 1, 1);

    // Seleção de Data com Calendário
    label_data = gtk_label_new("Selecione a Data:");
    gtk_widget_set_halign(label_data, GTK_ALIGN_END);
    calendar = gtk_calendar_new();
    gtk_grid_attach(GTK_GRID(grid), label_data, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), calendar, 1, 1, 1, 1);

    // Número do Quarto
    label_quarto_num = gtk_label_new("Número do Quarto:");
    gtk_widget_set_halign(label_quarto_num, GTK_ALIGN_END);
    combo_quarto = gtk_combo_box_text_new();
    for(int i = 1; i <= QUARTOS_DISPONIVEIS; i++) {
        char quarto_str[3];
        sprintf(quarto_str, "%d", i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_quarto), quarto_str);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_quarto), 0);
    gtk_grid_attach(GTK_GRID(grid), label_quarto_num, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo_quarto, 1, 2, 1, 1);

    // Número de Pessoas
    label_pessoas = gtk_label_new("Número de Pessoas:");
    gtk_widget_set_halign(label_pessoas, GTK_ALIGN_END);
    spin_pessoas = gtk_spin_button_new_with_range(1, 10, 1);
    gtk_grid_attach(GTK_GRID(grid), label_pessoas, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), spin_pessoas, 1, 3, 1, 1);

    // Número de Diárias
    label_diarias = gtk_label_new("Número de Diárias:");
    gtk_widget_set_halign(label_diarias, GTK_ALIGN_END);
    spin_diarias = gtk_spin_button_new_with_range(1, 30, 1);
    gtk_grid_attach(GTK_GRID(grid), label_diarias, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), spin_diarias, 1, 4, 1, 1);

    gtk_widget_show_all(dialog);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const gchar *nome = gtk_entry_get_text(GTK_ENTRY(entry_nome));
        
        // Obter a data selecionada no calendário
        guint year, month, day;
        gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
        month += 1; // GtkCalendar usa meses de 0 a 11
        char data_str[11];
        snprintf(data_str, sizeof(data_str), "%02d/%02d/%04d", day, month, year);

        const gchar *quarto_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_quarto));
        int quarto = quarto_str ? atoi(quarto_str) : 0;
        int pessoas = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_pessoas));
        int diarias = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_diarias));

        // Validações
        if(strlen(nome) == 0) {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                             GTK_DIALOG_MODAL,
                                                             GTK_MESSAGE_ERROR,
                                                             GTK_BUTTONS_OK,
                                                             "Nome do cliente não pode estar vazio.");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        } else {
            struct tm new_start = parse_date(data_str);

            // Verificar data válida
            int data_valida = verificarDataValida(data_str);
            if (!data_valida) {
                GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                                 GTK_DIALOG_MODAL,
                                                                 GTK_MESSAGE_ERROR,
                                                                 GTK_BUTTONS_OK,
                                                                 "Data inválida! A data deve ser hoje ou uma data futura.");
                gtk_dialog_run(GTK_DIALOG(error_dialog));
                gtk_widget_destroy(error_dialog);
            } else {
                // Verificar disponibilidade do quarto para todo o período
                if(!isRoomAvailable(lista_reservas, quarto, new_start, diarias)) {
                    GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                                     GTK_DIALOG_MODAL,
                                                                     GTK_MESSAGE_ERROR,
                                                                     GTK_BUTTONS_OK,
                                                                     "Quarto já está reservado para o período selecionado.");
                    gtk_dialog_run(GTK_DIALOG(error_dialog));
                    gtk_widget_destroy(error_dialog);
                } else {
                    // Verificar capacidade total
                    int totalPessoas = contarTotalPessoas(lista_reservas);
                    if (totalPessoas + pessoas > CAPACIDADE_TOTAL) {
                        GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                                         GTK_DIALOG_MODAL,
                                                                         GTK_MESSAGE_ERROR,
                                                                         GTK_BUTTONS_OK,
                                                                         "Capacidade total de pessoas excedida.");
                        gtk_dialog_run(GTK_DIALOG(error_dialog));
                        gtk_widget_destroy(error_dialog);
                    } else {
                        // Inserir Reserva
                        Reserva nova_reserva;
                        strcpy(nova_reserva.nome_cliente, nome);
                        strcpy(nova_reserva.data_inicio, data_str);
                        nova_reserva.quarto_id = quarto;
                        nova_reserva.num_pessoas = pessoas;
                        nova_reserva.num_diarias = diarias;
                        nova_reserva.preco = 0.0; // Será calculado na função de inserção

                        lista_reservas = inserirReserva(lista_reservas, nova_reserva);
                        atualizarTreeView(widgets);

                        GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                                        GTK_DIALOG_MODAL,
                                                                        GTK_MESSAGE_INFO,
                                                                        GTK_BUTTONS_OK,
                                                                        "Reserva realizada com sucesso!");
                        gtk_dialog_run(GTK_DIALOG(info_dialog));
                        gtk_widget_destroy(info_dialog);
                    }
                }
            }
        }
    }

    gtk_widget_destroy(dialog);
}

// Callback para Remover Reserva
void on_remove_reserva(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;

    GtkWidget *dialog, *content_area;
    GtkWidget *grid;
    GtkWidget *label_id, *spin_id;
    gint response;

    dialog = gtk_dialog_new_with_buttons("Remover Reserva",
                                         GTK_WINDOW(widgets->window),
                                         GTK_DIALOG_MODAL,
                                         ("_Cancelar"),
                                         GTK_RESPONSE_CANCEL,
                                         ("_Remover"),
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    // ID da Reserva
    label_id = gtk_label_new("ID da Reserva:");
    gtk_widget_set_halign(label_id, GTK_ALIGN_END);
    spin_id = gtk_spin_button_new_with_range(1, 100000, 1);
    gtk_grid_attach(GTK_GRID(grid), label_id, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), spin_id, 1, 0, 1, 1);

    gtk_widget_show_all(dialog);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        int id = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_id));

        Reserva* reserva = buscarReserva(lista_reservas, id);
        if (reserva == NULL) {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                             GTK_DIALOG_MODAL,
                                                             GTK_MESSAGE_ERROR,
                                                             GTK_BUTTONS_OK,
                                                             "Reserva não encontrada!");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        } else {
            lista_reservas = removerReserva(lista_reservas, id);
            atualizarTreeView(widgets);

            GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                            GTK_DIALOG_MODAL,
                                                            GTK_MESSAGE_INFO,
                                                            GTK_BUTTONS_OK,
                                                            "Reserva removida com sucesso!");
            gtk_dialog_run(GTK_DIALOG(info_dialog));
            gtk_widget_destroy(info_dialog);
        }
    }

    gtk_widget_destroy(dialog);
}

// Callback para Buscar Reserva
void on_search_reserva(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;

    GtkWidget *dialog, *content_area;
    GtkWidget *grid;
    GtkWidget *label_id, *spin_id;
    gint response;

    dialog = gtk_dialog_new_with_buttons("Buscar Reserva",
                                         GTK_WINDOW(widgets->window),
                                         GTK_DIALOG_MODAL,
                                         ("_Cancelar"),
                                         GTK_RESPONSE_CANCEL,
                                         ("_Buscar"),
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    // ID da Reserva
    label_id = gtk_label_new("ID da Reserva:");
    gtk_widget_set_halign(label_id, GTK_ALIGN_END);
    spin_id = gtk_spin_button_new_with_range(1, 100000, 1);
    gtk_grid_attach(GTK_GRID(grid), label_id, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), spin_id, 1, 0, 1, 1);

    gtk_widget_show_all(dialog);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        int id = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_id));

        Reserva* reserva = buscarReserva(lista_reservas, id);
        if (reserva == NULL) {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                             GTK_DIALOG_MODAL,
                                                             GTK_MESSAGE_ERROR,
                                                             GTK_BUTTONS_OK,
                                                             "Reserva não encontrada!");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        } else {
            // Exibir detalhes da reserva
            char detalhes[512];
            sprintf(detalhes, "ID da Reserva: %d\nQuarto: %d\nCliente: %s\nData de Início: %s\nPessoas: %d\nDiárias: %d\nPreço: ",
                    reserva->reserva_id, reserva->quarto_id, reserva->nome_cliente, reserva->data_inicio,
                    reserva->num_pessoas, reserva->num_diarias);

            // Formatar o preço com duas casas decimais e vírgula
            char preco_str[20];
            snprintf(preco_str, sizeof(preco_str), "R$%.2f", reserva->preco);
            // Substitui ponto por vírgula
            for(int i = 0; preco_str[i] != '\0'; i++) {
                if(preco_str[i] == '.') {
                    preco_str[i] = ',';
                    break;
                }
            }
            strcat(detalhes, preco_str);

            GtkWidget *info_dialog = gtk_message_dialog_new(GTK_WINDOW(dialog),
                                                            GTK_DIALOG_MODAL,
                                                            GTK_MESSAGE_INFO,
                                                            GTK_BUTTONS_OK,
                                                            "%s", detalhes);
            gtk_dialog_run(GTK_DIALOG(info_dialog));
            gtk_widget_destroy(info_dialog);
        }
    }

    gtk_widget_destroy(dialog);
}

// Callback para Exibir Todas as Reservas
void on_display_reservas(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    atualizarTreeView(widgets);
}

// Função para Aplicar CSS
void aplicar_css(GtkWidget *widget, const char *arquivo_css) {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);
    gtk_css_provider_load_from_path(provider, arquivo_css, NULL);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}

int main(int argc, char *argv[]) {
    // Define a localidade para usar vírgula como separador decimal
    setlocale(LC_ALL, "pt_BR.UTF-8");

    gtk_init(&argc, &argv);

    // Inicializa a lista e adiciona reservas iniciais
    lista_reservas = inicializarLista();
    adicionarReservasIniciais(lista_reservas);

    // Cria os widgets principais
    AppWidgets *widgets = g_slice_new(AppWidgets);

    // Cria a janela principal
    widgets->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(widgets->window), "Sistema de Reservas de Hotel");
    gtk_window_set_default_size(GTK_WINDOW(widgets->window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(widgets->window), GTK_WIN_POS_CENTER);

    // Sinal para fechar a aplicação
    g_signal_connect(widgets->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Cria uma caixa vertical para organizar os widgets
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(widgets->window), vbox);

    // Cria a barra de menu
    GtkWidget *menubar = gtk_menu_bar_new();

    // Menu "Arquivo"
    GtkWidget *menu_arquivo = gtk_menu_new();
    GtkWidget *menuitem_arquivo = gtk_menu_item_new_with_label("Arquivo");
    GtkWidget *menuitem_sair = gtk_menu_item_new_with_label("Sair");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_arquivo), menuitem_sair);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem_arquivo), menu_arquivo);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitem_arquivo);

    // Menu "Reservas"
    GtkWidget *menu_reservas = gtk_menu_new();
    GtkWidget *menuitem_reservas = gtk_menu_item_new_with_label("Reservas");
    GtkWidget *menuitem_inserir = gtk_menu_item_new_with_label("Inserir Reserva");
    GtkWidget *menuitem_remover = gtk_menu_item_new_with_label("Remover Reserva");
    GtkWidget *menuitem_buscar = gtk_menu_item_new_with_label("Buscar Reserva");
    GtkWidget *menuitem_exibir = gtk_menu_item_new_with_label("Exibir Todas Reservas");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_reservas), menuitem_inserir);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_reservas), menuitem_remover);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_reservas), menuitem_buscar);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_reservas), menuitem_exibir);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem_reservas), menu_reservas);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menuitem_reservas);

    // Adiciona a barra de menu à caixa vertical
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    // Cria a TreeView para exibir reservas
    widgets->treeview = criar_tree_view();
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), widgets->treeview);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    // Conecta os sinais dos menu items
    g_signal_connect(menuitem_sair, "activate", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(menuitem_inserir, "activate", G_CALLBACK(on_insert_reserva), widgets);
    g_signal_connect(menuitem_remover, "activate", G_CALLBACK(on_remove_reserva), widgets);
    g_signal_connect(menuitem_buscar, "activate", G_CALLBACK(on_search_reserva), widgets);
    g_signal_connect(menuitem_exibir, "activate", G_CALLBACK(on_display_reservas), widgets);

    // Aplica o CSS
    aplicar_css(widgets->window, "style.css");

    // Atualiza a TreeView com as reservas iniciais
    atualizarTreeView(widgets);

    // Mostra todos os widgets
    gtk_widget_show_all(widgets->window);

    gtk_main();

    // Libera a memória alocada para a lista
    // (Opcional: implementar uma função para liberar a lista)
    return 0;
}
