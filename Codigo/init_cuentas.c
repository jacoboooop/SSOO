#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct Cuenta {
    int numero_cuenta;
    char titular[50];
    float saldo;
    int num_transacciones;
};

int main(){

    char nombre_archivo[] = "../Archivos_datos/cuentas.dat";

    FILE *file = fopen(nombre_archivo, "a");

    struct Cuenta cuentas;

    for (int i = 1; i < 6; i++){
        cuentas.numero_cuenta = i;
        sprintf(cuentas.titular, "usuario%d", i);
        cuentas.saldo = 500;
        cuentas.num_transacciones = 0;
        fprintf(file, "%d,%s,%f,%d\n", cuentas.numero_cuenta, cuentas.titular, cuentas.saldo, cuentas.num_transacciones);
    }

    fclose(file);
    printf("Archivo de cuentas inicializado correctamente.\n");   

    return 0; 

}