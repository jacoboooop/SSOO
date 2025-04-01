#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include "comun.h"

void* realizar_deposito(void* u);
void* realizar_retiro(void* u);
void* realizar_transferencia(void* u);
void* consultar_saldo(void* u);
Usuario AsignarUsuario(int NumeroCuenta);

sem_t *semaforo;

Usuario usuario;

int main(int argc, char* argv[]){

    semaforo = sem_open("/cuentas_sem", 0);

    int numero_cuenta = atoi(argv[1]); //guardo ncuenta para operaciones
    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");

    pthread_t thread_deposito, thread_retiro, thread_transferencia, thread_saldo;
    
    usuario = AsignarUsuario(numero_cuenta);

    int opcion;
    while(1){
        printf("1. Depósito\n2. Retiro\n3. Transferencia\n4. Consultar saldo\n5. Salir\n");
        printf("Que operación desea relaizar: ");
        scanf("%d", &opcion);
        //while(getchar()!='\n');
        switch (opcion) {
            case 1: 
                pthread_create(&thread_deposito, NULL, realizar_deposito, &usuario);
                pthread_join(thread_deposito, NULL); 
                break;
            case 2: 
                pthread_create(&thread_retiro, NULL, realizar_retiro, &usuario);
                pthread_join(thread_retiro, NULL);
                break;
            case 3: 
                pthread_create(&thread_transferencia, NULL, realizar_transferencia, &usuario);
                pthread_join(thread_transferencia, NULL);
                break;
            case 4: 
                consultar_saldo(&usuario);
                break;
            case 5: 
                exit(0);
            default: printf("Esa opcion no es posible, vuelva a intentarlo");
        }
    }   
    sem_close(semaforo);

    return 0;
}

// Cambiar funcion

void* realizar_deposito(void* u){

    sem_wait(semaforo);

    Usuario usuario;

    usuario = *(Usuario*)u;

    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");

    float ingreso;

    char linea[100];
    char linea_aux[100];

    //Pido la cantidad a ingresae
    printf("Va a realizar un deposito\n");
    printf("\t¿Cuanto dinero quiere ingresar?: ");
    scanf(" %f", &ingreso);


    //Leo el archivo para sacar el saldo
    FILE *file = fopen(configuracion.archivo_cuentas, "r+");
    Usuario Aux;
    while (fgets(linea, sizeof(linea), file)) {
        // Leemos todos los datos de la linea
        if (sscanf(linea, "%d,%49[^,],%f,%d", &Aux.numero_cuenta, Aux.titular, &Aux.saldo, &Aux.num_transacciones) == 4) {
            // Verificar si el nombre y la contraseña coinciden
            if (Aux.numero_cuenta == usuario.numero_cuenta) {
                break;
            }   
        }
    }
    printf("\tEl ingreso se ha realizado con exito.");
    printf("Saldo actual: %f", usuario.saldo);
    usuario.saldo += ingreso;
    printf("Volviendo al menu...");
    sleep(1);
    //actualizo el archivo con el nuevo saldo
    fseek(file, -strlen(linea_aux), SEEK_CUR);
    fprintf(file,"%d,%s,%f,%d\n",usuario.numero_cuenta, usuario.titular, usuario.saldo, usuario.num_transacciones);
    sem_post(semaforo);
}

void* realizar_retiro(void* u){

    sem_wait(semaforo);

    Usuario usuario;

    usuario = *(Usuario*)u;

    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");
    float retiro;

    char linea[100];
    char linea_aux[100];

    //Leo el archivo para sacar el saldo
    FILE *file = fopen(configuracion.archivo_cuentas, "r+");
    Usuario Aux;
    while (fgets(linea, sizeof(linea), file)) {
        // Leemos todos los datos de la linea
        if (sscanf(linea, "%d,%49[^,],%f,%d", &Aux.numero_cuenta, Aux.titular, &Aux.saldo, &Aux.num_transacciones) == 4) {
            // Verificar si el nombre y la contraseña coinciden
            if (Aux.numero_cuenta == usuario.numero_cuenta) {
                break;
            }   
        }
    }

    printf("%s tiene un saldo de %f\n", usuario.titular, usuario.saldo);
    printf("\t¿Cuento dinero quiere retirar?: ");
    scanf("%f", &retiro);
    while(getchar()!='\n');
    usuario.saldo -= retiro;
    printf("\tEl retiro se ha realizado con exito.");
    printf("Saldo actual: %f", usuario.saldo);
    printf("Volviendo al menu...");
    sleep(1);
    //actualizo el archivo
    fseek(file, -strlen(linea_aux), SEEK_CUR);
    fprintf(file,"%d,%s,%f,%d\n",usuario.numero_cuenta, usuario.titular, usuario.saldo, usuario.num_transacciones);
    fclose(file);

    file = fopen(configuracion.archivo_log, "a");
    fprintf(file,"%d,%d,%f",usuario.numero_cuenta,usuario.numero_cuenta,retiro);
    fclose(file);

    sem_post(semaforo);

}

void* realizar_transferencia(void* u){

    sem_wait(semaforo);

    Usuario usuario;

    usuario = *(Usuario*)u;

    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");

    char linea[100];
    char linea_aux[100];

    //Datos de la transferencia
    int NumeroCuenta;
    float Cantidad;

    //Leo el archivo para sacar el saldo
    FILE *file = fopen(configuracion.archivo_cuentas, "r+");
    Usuario Aux;
    while (fgets(linea, sizeof(linea), file)) {
        // Leemos todos los datos de la linea
        if (sscanf(linea, "%d,%49[^,],%f,%d", &Aux.numero_cuenta, Aux.titular, &Aux.saldo, &Aux.num_transacciones) == 4) {
            // Verificar si el nombre y la contraseña coinciden
            if (Aux.numero_cuenta == usuario.numero_cuenta) {
                break;
            }   
        }
    }

    printf("\nDima el numero de cuenta al que quieres hacer una transferencia: ");
    scanf("%d", &NumeroCuenta);
    printf("\nQue cantidad le quieres ingresar: ");
    scanf("%f", &Cantidad);

    usuario.saldo -= Cantidad;

    fseek(file, -strlen(linea_aux), SEEK_CUR);
    fprintf(file,"%d,%s,%f,%d\n",usuario.numero_cuenta, usuario.titular, usuario.saldo, usuario.num_transacciones);
    fclose(file);    

    file = fopen(configuracion.archivo_cuentas, "r+");
    while (fgets(linea, sizeof(linea), file)) {
        // Leemos todos los datos de la linea
        if (sscanf(linea, "%d,%49[^,],%f,%d", &Aux.numero_cuenta, Aux.titular, &Aux.saldo, &Aux.num_transacciones) == 4) {
            // Verificar si el nombre y la contraseña coinciden
            if (Aux.numero_cuenta == NumeroCuenta) {
                break;
            }   
        }
    }

    Aux.saldo += Cantidad;
    fseek(file, -strlen(linea_aux), SEEK_CUR);
    fprintf(file,"%d,%s,%f,%d\n",Aux.numero_cuenta, Aux.titular, Aux.saldo, Aux.num_transacciones);
    fclose(file);

    file = fopen(configuracion.archivo_log, "a");
    fprintf(file,"%d,%d,%f",usuario.numero_cuenta,Aux.numero_cuenta,Cantidad);
    fclose(file);

    sem_post(semaforo);
} 

void* consultar_saldo(void* u){

    char confirmacion;

    Usuario usuario;

    usuario = *(Usuario*)u;
    
    printf("Tienes un saldo de %f\n", usuario.saldo);
    printf("¿Desea volver al menú?: ");
    scanf(" %c", &confirmacion);
    do {
        if(confirmacion == 's' || confirmacion == 'S'){
            break;
        } else if (confirmacion == 'N' || confirmacion == 'n'){
            
        }else {
            printf("Opción no válida, vuelva a intentarlo.");
        }
    } while (confirmacion != 'S' && confirmacion != 's' && confirmacion != 'N' && confirmacion != 'n');
}

Usuario AsignarUsuario(int NumeroCuenta) {

    char linea[100];    

    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");

    FILE *file = fopen(configuracion.archivo_cuentas, "r+");
    Usuario usuario, Aux;

    while (fgets(linea, sizeof(linea), file)) {
        // Leemos todos los datos de la linea
        if (sscanf(linea, "%d,%49[^,],%f,%d", &usuario.numero_cuenta, usuario.titular, &usuario.saldo, &usuario.num_transacciones) == 4) {
            // Verificar si numero cuenta coinciden
            if (usuario.numero_cuenta == NumeroCuenta) {
                fclose(file);
                return usuario;
            }   
        }
    }   
    fclose(file); 
}