#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "comun.h"

#define MAX_LINE_LENGTH 256  

void menu();
void RegistrarCuenta(const char *nombreArchivo, const char *nombre_input);
void LoginCuenta(int numeroCuenta);
void init_cuentas(const char *nombre_archivo);
void AgregarLog(const char *operacion);
bool VerificarCuenta(char nombre[], int numero, const char *nombreArchivo);

sem_t *semaforo;

Config configuracion;

Usuario usuario;

int main(){

    semaforo = sem_open("/cuentas_sem", O_CREAT, 0644, 1);

    configuracion = leer_configuracion("../Archivos_datos/config.txt");

    menu(configuracion.archivo_cuentas);

    //*******************************inicializo el monitor***********************************
    int fd[2]; 
    pid_t pid = pid = fork();

    if (pipe(fd) == -1) {
        perror("Error al crear la tubería");
        exit(1);
    }
    if (pid < 0) {
        perror("Error al crear el monitor");
        exit(1);
    }
    else if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        execl("./monitor", "monitor", NULL);
        perror("Error en exec de monitor");
        exit(1);
    }
    else {
        //mensaje?
        close(fd[1]);  //cierro escritura
        wait(NULL);  //espero a que termine mointor
        printf("Monitor finalizado.\n");
    }

    AgregarLog("Se ha iniciado el menu");
   
    return (0);

}


void menu(const char *nombreArchivo){
    int caso;
    printf("1. Login\n2. Registeer\n3. Salir\n");
    printf("Respuesta: ");
    scanf("%d", &caso);
    char NombreUsuario[50];
    int NumeroCuenta;

    switch (caso) {
        case 1:
            AgregarLog("Se esta intentando hacer login");
            // Proceso de login
            printf("\nNombre de usuario: ");
            scanf("%s", NombreUsuario);
            printf("\nNumeroCuenta: ");
            scanf("%d", &NumeroCuenta);
            if (VerificarCuenta(NombreUsuario, NumeroCuenta, nombreArchivo)) {
                /*
                int NumeroCuenta;
                FILE *file = fopen(nombreArchivo, "r");
                char linea[MAX_LINE_LENGTH];
                Usuario usuario;
                while (fgets(linea, sizeof(linea), file)) {
                    // Leemos todos los datos de la linea
                    if (sscanf(linea, "%d,%49[^,],%f,%d", &usuario.numero_cuenta, usuario.titular, &usuario.saldo, &usuario.num_transacciones) == 4) {
                        // Verificar si el nombre y la contraseña coinciden
                        if (strcmp(usuario.titular, NombreUsuario) == 0) {
                            NumeroCuenta = usuario.numero_cuenta;
                            fclose(file);
                        }
                    }
                }*/
                LoginCuenta(NumeroCuenta);
                AgregarLog("Se ha hecho login");
            }
            else {
                AgregarLog("El usuario no existe");
                printf("El usuario no existe");
            }
            menu(nombreArchivo);
            break;
        case 2:
            AgregarLog("Se esta registrando un nuevo usuario");
            //registro
            printf("\nNombre de usuario: ");
            scanf("%s", NombreUsuario);
            RegistrarCuenta(nombreArchivo, NombreUsuario);
            AgregarLog("Se ha registrado un nuevo usuario");
            AgregarLog("Se ha hecho login");
            int NumeroCuenta;
            FILE *file = fopen(nombreArchivo, "r");
            char linea[MAX_LINE_LENGTH];
            Usuario usuario;
            while (fgets(linea, sizeof(linea), file)) {
                // Leemos todos los datos de la linea
                if (sscanf(linea, "%d,%49[^,],%f,%d", &usuario.numero_cuenta, usuario.titular, &usuario.saldo, &usuario.num_transacciones) == 4) {
                    // Verificar si el nombre y la contraseña coinciden
                    if (strcmp(usuario.titular, NombreUsuario) == 0) {
                        NumeroCuenta = usuario.numero_cuenta;
                        fclose(file);
                    }
                }
            }
            LoginCuenta(NumeroCuenta);
            menu(nombreArchivo);
            break;
        case 3: 
            AgregarLog("Se ha cerrado el banco");
            printf("\nCerrando el banco....\n");
            return;
        default:
            AgregarLog("Se ha introducido un parametor incorrecto");
            printf("\nOpción no válida. Inténtalo de nuevo.\n");
            break;
    }
}

void LoginCuenta(int numeroCuenta){
    pid_t pid2 = fork();

    if (pid2 < 0) {
        perror("Error al crear el usuario");
        exit(1);
    }
    else if (pid2 == 0) {
        sleep(1);
        char comando[250];
        snprintf(comando, sizeof(comando), "./usuario '%d'", numeroCuenta);
        execlp("gnome-terminal", "gnome-terminal", "--", "./usuario", comando, NULL);
        perror("Error en exec"); 
        exit(1);
    }
    else {
        printf("Esperando al inicio de sesion...\n");
        wait(NULL);
        printf("\nAccediendo al usuairo...\n\n");
    }
}

void RegistrarCuenta(const char *nombreArchivo, const char *nombre_input){

    char linea[MAX_LINE_LENGTH];

    FILE *Registro = fopen(nombreArchivo, "r");
    int contador = 0;

    // Leer el archivo línea por línea
    while (fgets(linea, sizeof(linea), Registro)) {
        // Leemos todos los datos de la linea
        contador++;
    }
    fclose(Registro);

    FILE *Registro1 = fopen(nombreArchivo, "a");

    fprintf(Registro1,"%d,%s,500,0\n",contador + 1,nombre_input);
    fclose(Registro1);
}

void AgregarLog(const char *operacion) {

    FILE *archivoLog = fopen(configuracion.archivo_log, "a");
    
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char hora_actual[20];
    strftime(hora_actual, sizeof(hora_actual), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(archivoLog, "%s ---- %s\n", hora_actual, operacion);
    fclose(archivoLog);

}

bool VerificarCuenta(char nombre[], int numero, const char *nombreArchivo){
    FILE *file = fopen(nombreArchivo, "r");
    char linea[MAX_LINE_LENGTH];
    Usuario usuario;
    while (fgets(linea, sizeof(linea), file)) {
        // Leemos todos los datos de la linea
        if (sscanf(linea, "%d,%49[^,],%f,%d", &usuario.numero_cuenta, usuario.titular, &usuario.saldo, &usuario.num_transacciones) == 4) {
            // Verificar si el nombre y la contraseña coinciden
            if (strcmp(usuario.titular, nombre) == 0 && (usuario.numero_cuenta == numero)) {
                fclose(file);
                return true;
            }
            
        }
    }
    fclose(file);
    return false;
}

