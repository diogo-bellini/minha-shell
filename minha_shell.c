/*
Autore:
    - Diogo Conforti Vaz Bellini 
Objetivo:
    Criação de uma shell simplificada
*/

//Bibliotecas
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

//Constantes
#define MAX_ARGS 30 //Número máximo de argumengtos que pode ser passado
#define MAX_TAM_ARG 100 //Tamanho máximo de cada argumento
#define MAX_PATH 200 //Tamanho máximo do caminho

//Protótipos
void tratador_de_sinal();

//Programa principal
int main()
{
    char diretorio[MAX_PATH]; //String de diretório corrente
    char input[MAX_ARGS * MAX_TAM_ARG]; //String que recebe o que será escrito pelo usuário
    char * argumentos[MAX_ARGS]; //Vetor de ponteiros para armazenar os argumentos

    int ehForeground; //Verificador de foreground

    int ultimo_arg; //Índice do ultimo argumento

    pid_t result; //Vai ser usado para verificar o fork

    //Usa o tratador_de_sinal toda vez que recebe um SIGCHLD
    signal(SIGCHLD, tratador_de_sinal);

    //Loop infinito da shell
    while(1)
    {
        ehForeground = 0;

        //Pegar diretóido corrente
        getcwd(diretorio, sizeof(diretorio));
        
        //Padrão da shell
        printf("%s $", diretorio);

        //Ler o que o usuário deseja fazer
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // Remove o '\n' do final da string de entrada para os comandos funcionarem no execvp
        
        //Ve se o usuário deseja sair do prompt
        if (strcmp(input, "exit") == 0)
        {
            exit(0);
        }

        //Separar o input em arguementos
        argumentos[0] = strtok(input, " ");
        
        for (int i = 1; i < MAX_ARGS; i++)
        {
            argumentos[i] = strtok(NULL, " "); //Usa NULL porque já foi dito antes que é do input, se não vai encher o vetor inteiro com o primeiro argumento

            if (argumentos[i] == NULL)
            {
                ultimo_arg = i-1;
                break;
            }
        }

        //Ve se o usuário nao escreveu nada
        if (argumentos[0] == NULL)
        {
            continue; //Vai para o próx loop
        }

        //Ve se usuário deseja fazer o Change Directory (cd)
        if (strcmp(argumentos[0], "cd") == 0) 
        {
            // Verifica se o usuário não forneceu um argumento para o diretório ou forneceu ~
            if (argumentos[1] == NULL || strcmp(argumentos[1], "~") == 0) 
            {
                // Se nenhum diretório for especificado, muda para o diretório home do usuário
                char *home_dir = getenv("HOME");

                if (home_dir != NULL) 
                {
                    if (chdir(home_dir) != 0) 
                    {
                        perror("cd"); // Se houver erro, exibe a mensagem de erro
                    }
                } 
                else 
                {
                    printf("cd: HOME não está definido\n");
                }
            } 
            else
            {
                // Tenta mudar para o diretório especificado
                if (chdir(argumentos[1]) != 0) 
                {
                    perror("cd"); // Se houver erro, exibe a mensagem de erro
                }
            }
            continue; // Após executar o comando cd, volta ao início do loop para exibir o próximo prompt
        }
               

        //Definindo se é background ou foreground
        if (strcmp(argumentos[ultimo_arg], "&") == 0)
        {
            argumentos[ultimo_arg] = NULL; //Remove o & do vetor de argumentos
        }
        else
        {
            ehForeground = 1;
        }

        //Cria novo processo filho que irá tratar de fato o que o usuário quer (Processo pai é a shell em si)
        result = fork();

        if (result < 0) //Erro no fork
        {
            printf("Erro na criação do processo filho!!!\n");
            exit(1);
        } 
        else if (result == 0) //Processo filho
        {
            if (execvp(argumentos[0], argumentos) == -1) 
            {
                perror("execvp"); // Exibe uma mensagem de erro se a execução falhar
                exit(EXIT_FAILURE); // Sai com código de erro
            }

            // Se execvp retornar, houve um erro
            perror("execvp");
            exit(EXIT_FAILURE); // Sai com código de erro
        } 
        else //Processo pai (shell) 
        {
            if (ehForeground) 
            {
                waitpid(result, NULL, 0); // Espera pelo processo filho, se estiver em foreground
            }
            else 
            {
                printf("Processo em background com PID: %d\n", result);
            }
        } 
    }
}//Fim do program principal


//Função para tratar do sinal SIGCHLD -> libera os recursos o filhos terminados
void tratador_de_sinal()
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/*

Chat GPT:
Quando você chama waitpid() com WNOHANG, ela retorna 0 se nenhum filho terminou, -1 se houve um erro ou o
PID do filho que terminou, se houver algum. Isso permite que você verifique se 
há processos filhos que terminaram sem bloquear a execução do programa.

*/
