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
void* Estado_banco(void* arg);

char mensaje[100];

sem_t *semaforo;


int main(int argc, char* argv[]){

    semaforo = sem_open("semaforo_cuentas", 0);

    int numero_cuenta = atoi(argv[1]); //guardo ncuenta para operaciones

    pthread_t banco;
    pid_t pid_banco = atoi(argv[2]);
    pthread_create(&banco, NULL, Estado_banco, &pid_banco);

    
    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");

    pthread_t thread_deposito, thread_retiro, thread_transferencia, thread_saldo;

    int opcion;
    while(1){
        system("clear");
        printf("1. Depósito\n2. Retiro\n3. Transferencia\n4. Consultar saldo\n5. Salir\n");
        printf("Que operación desea relaizar: ");
        scanf("%d", &opcion);
        //while(getchar()!='\n');
        switch (opcion) {
            case 1: 
                pthread_create(&thread_deposito, NULL, realizar_deposito, &numero_cuenta);
                pthread_join(thread_deposito, NULL); 
                break;
            case 2: 
                pthread_create(&thread_retiro, NULL, realizar_retiro, &numero_cuenta);
                pthread_join(thread_retiro, NULL);
                break;
            case 3: 
                int resultado;
                resultado = pthread_create(&thread_transferencia, NULL, realizar_transferencia, &numero_cuenta);
                resultado = pthread_join(thread_transferencia, NULL);

                if (resultado == -2) {
                    printf("\nLa cuenta no existe\n");
                    sleep(4);
                } else if (resultado == -3)
                {
                    printf("\nSaldo insuficiente\n");
                    sleep(4);
                } else {
                    printf("\nOperacion realizada correctamente....\n");
                    sleep(2);
                }
                break;
            case 4: 
                consultar_saldo(&numero_cuenta);
                break;
            case 5: 
                exit(0);
            default: printf("Esa opcion no es posible, vuelva a intentarlo");
        }
    }   
    sem_close(semaforo);
    sem_unlink("semaforo_cuentas");


    return 0;
}

void* realizar_deposito(void* u){

    AgregarLog("Se esta esperando a que el semaforo este disponible");
    sem_wait(semaforo);

    int NumeroUsuario;

    NumeroUsuario = *(int*)u;
    Usuario usuario = AsignarUsuario(NumeroUsuario);

    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");
    float deposito;

    char linea[100];
    long linea_aux = -1;

    //Leo el archivo para sacar el saldo
    FILE *file = fopen(configuracion.archivo_cuentas, "r+");
    Usuario Aux;
    while (fgets(linea, sizeof(linea), file)) {
        // Leemos todos los datos de la linea
        if (sscanf(linea, "%d,%49[^,],%f,%d", &Aux.numero_cuenta, Aux.titular, &Aux.saldo, &Aux.num_transacciones) == 4) {
            // Verificar si el nombre y la contraseña coinciden
            if (Aux.numero_cuenta == usuario.numero_cuenta) {
                linea_aux = ftell(file) - strlen(linea);
                break;
            }   
        }
    }

    system("clear");
    printf("%s tiene un saldo de %f\n", usuario.titular, usuario.saldo);
    printf("\t¿Cuento dinero quiere depositar?: ");
    scanf(" %f", &deposito);
    usuario.saldo += deposito;
    system("clear");
    printf("El deposito se ha realizado con exito.");
    printf("\nSaldo actual: %f", usuario.saldo);
    printf("\nVolviendo al menu...");
    sleep(3);
    //actualizo el archivo
    fseek(file, linea_aux, SEEK_SET);
    fprintf(file, "%d,%s,%.2f,%d\n", usuario.numero_cuenta, usuario.titular, usuario.saldo, usuario.num_transacciones);
    fclose(file);

    file = fopen("../Archivos_datos/Transacciones.log", "a");
    fprintf(file,"%d,%d,%f",usuario.numero_cuenta,usuario.numero_cuenta,deposito);
    fclose(file);

    snprintf(mensaje, sizeof(mensaje), "Se ha realizado un deposito de: %f", deposito);
    AgregarLog(mensaje);

    sem_post(semaforo);
    system("clear");
}

void* realizar_retiro(void* u){

    AgregarLog("Se esta esperando a que el semaforo este disponible");
    sem_wait(semaforo);

    int NumeroUsuario;

    NumeroUsuario = *(int*)u;
    Usuario usuario = AsignarUsuario(NumeroUsuario);

    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");
    float retiro;

    char linea[100];
    long linea_aux = -1;

    //Leo el archivo para sacar el saldo
    FILE *file = fopen(configuracion.archivo_cuentas, "r+");
    Usuario Aux;
    while (fgets(linea, sizeof(linea), file)) {
        // Leemos todos los datos de la linea
        if (sscanf(linea, "%d,%49[^,],%f,%d", &Aux.numero_cuenta, Aux.titular, &Aux.saldo, &Aux.num_transacciones) == 4) {
            // Verificar si el nombre y la contraseña coinciden
            if (Aux.numero_cuenta == usuario.numero_cuenta) {
                linea_aux = ftell(file) - strlen(linea);
                break;
            }   
        }
    }

    system("clear");
    printf("%s tiene un saldo de %f\n", usuario.titular, usuario.saldo);
    printf("\t¿Cuento dinero quiere retirar?: ");
    scanf(" %f", &retiro);
    usuario.saldo -= retiro;
    system("clear");
    printf("El retiro se ha realizado con exito.");
    printf("\nSaldo actual: %f", usuario.saldo);
    printf("\nVolviendo al menu...");
    sleep(3);
    //actualizo el archivo
    fseek(file, linea_aux, SEEK_SET);
    fprintf(file, "%d,%s,%.2f,%d\n", usuario.numero_cuenta, usuario.titular, usuario.saldo, usuario.num_transacciones);
    fclose(file);

    file = fopen("../Archivos_datos/Transacciones.log", "a");
    fprintf(file,"%d,%d,%f",usuario.numero_cuenta,usuario.numero_cuenta,retiro);
    fclose(file);

    sem_post(semaforo);
    system("clear");

    snprintf(mensaje, sizeof(mensaje), "Se ha realizado un retiro de: %f", retiro);
    AgregarLog(mensaje);

}

void* realizar_transferencia(void* u){

    AgregarLog("Se esta esperando a que el semaforo este disponible");
    sem_wait(semaforo);

    Usuario Destino;

    int NumeroUsuario;

    NumeroUsuario = *(int*)u;
    Usuario usuario = AsignarUsuario(NumeroUsuario);

    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");

    //Datos de la transferencia
    int NumeroCuenta;
    float Cantidad;

    //Leo el archivo para sacar el saldo
    FILE *file = fopen(configuracion.archivo_cuentas, "r+");

    printf("\nDima el numero de cuenta al que quieres hacer una transferencia: ");
    scanf("%d", &NumeroCuenta);
    printf("\nQue cantidad le quieres ingresar: ");
    scanf("%f", &Cantidad);

    long posicion_origen = -1, posicion_destino = -1;
    char linea[100];
    Usuario Aux;

    // Buscar las cuentas de origen y destino en el archivo.
    while (fgets(linea, sizeof(linea), file)) {
        if (sscanf(linea, "%d,%49[^,],%f,%d", &Aux.numero_cuenta, Aux.titular, &Aux.saldo, &Aux.num_transacciones) == 4) {
            if (Aux.numero_cuenta == usuario.numero_cuenta) {
                posicion_origen = ftell(file) - strlen(linea);
            } else if (Aux.numero_cuenta == NumeroCuenta) {
                Destino = Aux;
                posicion_destino = ftell(file) - strlen(linea); // Guarda la posición de la línea.
            }
        }
    }

    // Verificar si se encontraron las cuentas.
    if (posicion_origen == -1 || posicion_destino == -1) {
        fclose(file);
        return "-2"; // Una o ambas cuentas no existen.
    }

    // Verificar si la cuenta de origen tiene suficiente saldo.
    if (usuario.saldo < Cantidad) {
        fclose(file);
        return "-3"; // Saldo insuficiente.
    }

    // Actualizar los saldos y el número de transacciones.
    usuario.saldo -= Cantidad;
    usuario.num_transacciones++;
    Destino.saldo += Cantidad;
    Destino.num_transacciones++;

    // Escribir los datos actualizados en el archivo.
    fseek(file, posicion_origen, SEEK_SET);
    fprintf(file, "%d,%s,%.2f,%d\n", usuario.numero_cuenta, usuario.titular, usuario.saldo, usuario.num_transacciones);

    fseek(file, posicion_destino, SEEK_SET);
    fprintf(file, "%d,%s,%.2f,%d\n", Destino.numero_cuenta, Destino.titular, Destino.saldo, Destino.num_transacciones);

    fclose(file);

    file = fopen("../Archivos_datos/Transacciones.log", "a");
    fprintf(file,"%d,%d,%f",usuario.numero_cuenta,Aux.numero_cuenta,Cantidad);
    fclose(file);

    sem_post(semaforo);

    snprintf(mensaje, sizeof(mensaje), "Se ha realizado una transferencia de: %f", Cantidad);
    AgregarLog(mensaje);

    return 0; // Transferencia exitosa.
}

void* consultar_saldo(void* u){

    char confirmacion;

    int NumeroUsuario;

    NumeroUsuario = *(int*)u;
    Usuario usuario = AsignarUsuario(NumeroUsuario);
    system("clear");
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

    system("clear");
}

Usuario AsignarUsuario(int NumeroCuenta) {

    char linea[100];    

    char nombreArchivo[] = "../Archivos_datos/cuentas.dat";

    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");

    AgregarLog("Se esta leyendo el archivo de usuarios");
    FILE *file = fopen(nombreArchivo, "r");
    Usuario usuario;

    while (fgets(linea, sizeof(linea), file) != NULL) {
        if (sscanf(linea, "%d,%49[^,],%f,%d", &usuario.numero_cuenta, usuario.titular, &usuario.saldo, &usuario.num_transacciones) == 4) {
            if (usuario.numero_cuenta == NumeroCuenta) {
                fclose(file);
                return usuario;
            }
        }
    } 
    fclose(file); 
    printf("\nNo se ha encontrado el usuario\n");
}
