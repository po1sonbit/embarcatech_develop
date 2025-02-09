#include <stdio.h>
#include <string.h>

#define NIVEL_CRITICO_LIXO 80

void verificarContainer(char *local, int *nivelLixo);
void avaliarESolicitarColeta(char* local, int nivelLixo);
void solicitarColetaAutonoma(char* local);
void coletaEspecial(char* local, char* tipoResiduos);

int main() {
    printf("Sistema de Gestao de Residuos de Zenithia (SGR)\n");
    while(1) {
        int nivelRuaFlores, nivelAvenidaCentral, nivelPracaLiberdade;

        verificarContainer("Rua das Flores", &nivelRuaFlores);
        verificarContainer("Avenida Central", &nivelAvenidaCentral);
        verificarContainer("Praça da Liberdade", &nivelPracaLiberdade);

        avaliarESolicitarColeta("Rua das Flores", nivelRuaFlores);
        avaliarESolicitarColeta("Avenida Central", nivelAvenidaCentral);
        avaliarESolicitarColeta("Praça da Liberdade", nivelPracaLiberdade);
    }
    coletaEspecial("Hospital Geral", "lixo hospitalar");
    coletaEspecial("Centro Tecnologico", "eletronicos");
}

void verificarContainer(char *local, int *nivelLixo) {
    printf("\nVerificando container em %s...\n", local);

    if(strcmp(local, "Rua das Flores") == 0)
        *nivelLixo = 30;
    else if(strcmp(local, "Avenida Central") == 0)
        *nivelLixo = 80;
    else if(strcmp(local, "Praça da Liberdade") == 0)
        *nivelLixo = 100;
    else
        *nivelLixo = 0;

    printf("O container em %s está com %d%% de lixo.\n", local, *nivelLixo);
}

void avaliarESolicitarColeta(char* local, int nivelLixo) {
    if(nivelLixo >= NIVEL_CRITICO_LIXO) {
        printf("O container em %s atingiu o nivel citico de lixo.\n", local);
        solicitarColetaAutonoma(local);
    }
    else
        printf("O container em %s ainda não precisa de coleta.\n", local);
}

void solicitarColetaAutonoma(char* local) {
    printf("Veiculo autonomo a caminho para coletar o lixo em %s.\n", local);
}

void coletaEspecial(char* local, char* tipoResiduos) {
    printf("\nSolicitando coleta especial para %s em %s.\n", tipoResiduos, local);
}