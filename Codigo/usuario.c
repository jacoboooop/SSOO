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


int main(int argc, char* argv[]){

    semaforo = sem_open("/cuentas_sem", 0);

    int numero_cuenta = atoi(argv[1]); //guardo ncuenta para operaciones
    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");

    pthread_t thread_deposito, thread_retiro, thread_transferencia, thread_saldo;
    
    Usuario usuario = AsignarUsuario(numero_cuenta);

    int opcion;
    while(1){
        system("clear");
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
                int resultado;
                resultado = pthread_create(&thread_transferencia, NULL, realizar_transferencia, &usuario);
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

    Usuario Destino;

    Usuario* usuario = (Usuario*)u;

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
            if (Aux.numero_cuenta == usuario->numero_cuenta) {
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
    if (usuario->saldo < Cantidad) {
        fclose(file);
        return "-3"; // Saldo insuficiente.
    }

    // Actualizar los saldos y el número de transacciones.
    usuario->saldo -= Cantidad;
    usuario->num_transacciones++;
    Destino.saldo += Cantidad;
    Destino.num_transacciones++;

    // Escribir los datos actualizados en el archivo.
    fseek(file, posicion_origen, SEEK_SET);
    fprintf(file, "%d,%s,%.2f,%d\n", usuario->numero_cuenta, usuario->titular, usuario->saldo, usuario->num_transacciones);

    fseek(file, posicion_destino, SEEK_SET);
    fprintf(file, "%d,%s,%.2f,%d\n", Destino.numero_cuenta, Destino.titular, Destino.saldo, Destino.num_transacciones);

    fclose(file);
    return 0; // Transferencia exitosa.
}


void* consultar_saldo(void* u){

    char confirmacion;

    Usuario* usuario = (Usuario*)u;
    
    printf("Tienes un saldo de %f\n", usuario->saldo);
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

    char nombreArchivo[] = "../Archivos_datos/cuentas.dat";

    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");

    FILE *file = fopen(nombreArchivo, "r");
    rewind(file);
    Usuario usuario;

    while (fgets(linea, sizeof(linea), file)) {
        if (sscanf(linea, "%d,%49[^,],%f,%d", &usuario.numero_cuenta, usuario.titular, &usuario.saldo, &usuario.num_transacciones) == 4) {
            if (usuario.numero_cuenta == NumeroCuenta) {
                printf("%f", usuario.saldo);
                sleep(4);
                fclose(file);
                return usuario;
            }
        }
    } 
    fclose(file); 
    printf("No se ha encontrado el usuario");
    sleep(4);
}