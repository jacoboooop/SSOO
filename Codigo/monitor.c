#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "comun.h"

typedef struct Monitor
{
    int Usuario;
    int Usuario2;
    float Cantidad
} Monitor;

int main()
{

    Config configuracion = leer_configuracion("../Archivos_datos/config.txt");
    Monitor monitor;

    char Transacciones[] = "../Archivos_datos/Transacciones.log";

    char linea[256];
    int ContadorUsuarios;
    int Aux;
    int Aux2;
    int ContadorTransacciones;

    while (1)
    {
        FILE *file = fopen(configuracion.archivo_cuentas, "r");

        while (fgets(linea, sizeof(linea), file))
        {
            // Leemos todos los datos de la linea
            ContadorUsuarios++;
        }

        

        Aux = ContadorUsuarios;
        Aux2 = Aux;


        while (ContadorUsuarios != 0)
        {
            file = fopen(Transacciones, "r");
            Aux2--;
            while (fgets(linea, sizeof(linea), file))
            {
                // Leemos todos los datos de la linea
                if (sscanf(linea, "%d,%d,%f", &monitor.Usuario, &monitor.Usuario2, &monitor.Cantidad) == 4)
                {
                    if ((monitor.Usuario == monitor.Usuario2) && (monitor.Cantidad >= configuracion.umbral_retiros))
                    {
                        // Avisar de que el usuario ha superado el monto
                    }
                    else if (monitor.Usuario2 == Aux && (monitor.Usuario != monitor.Usuario2))
                    {
                        if (monitor.Cantidad >= configuracion.limite_transferencia)
                        {
                            // Avisar de exceso de limite de transferencia
                        }
                        ContadorTransacciones++;
                    }
                }
            }
            if (ContadorTransacciones >= configuracion.umbral_transferencias)
            {
                // Avisar de que el usuario ha superado las transacciones permitidas
            }
            Aux = Aux2;
            ContadorUsuarios--;
            fclose(file);
        }

        sleep(5);
    }
}