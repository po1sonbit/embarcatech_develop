#include <stdio.h>

int inverterNumero(int numero) {
    int invertido = 0, resto;

    while(numero != 0) {
        resto = numero % 10;
        invertido = invertido * 10 + resto;
        numero /= 10;
    }
    return invertido;
}

int main() {
    int numero, numeroInvertido;

    printf("Digite um numero de tres digitos: ");
    scanf("%d", &numero);
    if(numero >= 100 && numero <= 999) {
        numeroInvertido = inverterNumero(numero);
        printf("O numero invertido eh: %d\n", numeroInvertido);
    }
    else {
        printf("Numero invalido. Digite um numero entre 100 e 999.\n");
    }
    return 0;
}