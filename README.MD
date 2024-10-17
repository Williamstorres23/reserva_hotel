# 🏨 Sistema de Reservas de Hotel

Este projeto é um sistema simples de reservas de hotel, utilizando uma lista duplamente encadeada para gerenciar as reservas e a biblioteca GTK para criar uma interface gráfica interativa e responsiva.

## 🚀 Funcionalidades

- ➕ **Inserir Reserva**: Adicione uma nova reserva, especificando o nome do cliente, quarto, data, número de diárias e número de pessoas.
- 🗑️ **Remover Reserva**: Remova uma reserva da lista com base no número do quarto.
- 🔍 **Buscar Reserva**: Encontre uma reserva pelo número do quarto e exiba seus detalhes.
- 📅 **Exibir Reservas**: Veja todas as reservas registradas, ordenadas por data, em um formato de lista.

## ⚙️ Estrutura do Projeto

- **ReservaHotelGTK.c**: Código principal com todas as funcionalidades e a interface gráfica.
- **style.css**: Estilos customizados para a interface gráfica.

Este sistema é ideal para pequenas operações de reservas e possui validações para evitar reservas sobrepostas para o mesmo quarto.

## Pré-requisitos

Antes de começar, você precisará das seguintes ferramentas:

- **MSYS2**: Ambiente Unix-like para Windows.
- **GTK**: Biblioteca para criação de interfaces gráficas.

### 1. Instalar MSYS2

Baixe e instale o [MSYS2](https://www.msys2.org/). Após a instalação, siga os passos abaixo:

1. Abra o terminal MSYS2 e atualize os pacotes:

   ```bash
   pacman -Syu

 - Reinicie o terminal e continue a atualização:
   
   ```bashh
   pacman -Su

2. Instalar GTK e Ferramentas de Desenvolvimento
No terminal MSYS2, instale o GTK3 e o conjunto de ferramentas de desenvolvimento:
   ```bash
   pacman -S mingw-w64-x86_64-gtk3 mingw-w64-x86_64-toolchain
   
   pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-pkg-config


3. Compilação e Execução

   Navegue até o seu diretorio ex:cd /c/Users/Aluno/Desktop/atividade

   1. Compile o código usando o GCC no terminal MSYS2:
    ```bash
    gcc ReservaHotelGTK.c -o ReservaHotelGTK `pkg-config --cflags --libs gtk+-3.0`

2. Execute o programa:
    ```bash
    ./ReservaHotelGTK.exe

### Verificar o PATH

Se após a instalação houver problemas com os comandos `gcc` ou `pkg-config`, você pode verificar se o caminho `/mingw64/bin` foi adicionado ao `PATH` do MSYS2. Isso garante que os comandos sejam encontrados corretamente.

Você pode adicionar o caminho manualmente executando o seguinte comando no terminal do MSYS2:

    ```bash
    export PATH=/mingw64/bin:$PATH

