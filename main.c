/* 
 * File:   main.c
 * Author: Francisco Martínez Esteso
 *
 * Created on 27 de diciembre de 2015
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// Procesos máximos que se pueden gestionar
#define MAXPRO 100

// ESTRUCTURAS
typedef struct{
    char nombre[10];
    int tllegada;
    int tinicio;
    int duracion;
    int tentrada;
    boolean usadoSJF;
    boolean usadoRR;
    int trespuesta;
    int tespera;
    int cicloentradaPRO, ciclosalidaPRO;
} PROCESO;

// DATOS comprobacion ficheros y procesos
typedef PROCESO ListaProcesos[MAXPRO];
HANDLE hFind;
WIN32_FIND_DATA datos;
char* SJFF = "SolucionSJF_";
char* RoundRobinn = "SolucionRoundRobin_";
char* nombreficheroSJF;
char* nombreficheroRoundRobin;

// FILEs
FILE *archivoSJF;
FILE *archivoRoundRobin;

// DATOS gestion procesos
int quantum;
ListaProcesos lista;
int nprocesos;
int ciclo = 0;
float utilizacion = 0;

// DATOS RoundRobin
int tpermanencia;


// MÉTODOS
void CrearProcesos(char fichero[]);
void RoundRobin();
void SJF();
int FTtotal();
int EscogerProcesoActualSJF();
void GestionarActualSJF(PROCESO x);
int EscogerProcesoActualRoundRobin();
void GestionarActualRoundRobin(PROCESO x);

// MAIN
int main(int argc, char** argv) {
    
    
    ListaProcesos lista;
    
    hFind = FindFirstFile(argv[1], &datos);
    
    // Inicias el nombre de los ficheros que se dan como salida
    nombreficheroSJF = malloc(strlen(SJFF) + strlen(argv[1]) + 2);
    nombreficheroRoundRobin = malloc(strlen(RoundRobinn) + strlen(argv[1]) + 2);
    strcpy(nombreficheroSJF, SJFF);
    strcat(nombreficheroSJF, argv[1]);
    strcpy(nombreficheroRoundRobin, RoundRobinn);
    strcat(nombreficheroRoundRobin, argv[1]);
    
    if(argc == 2) {
        if(hFind == INVALID_HANDLE_VALUE){
        printf("El fichero indicado no existe.\n");
        }
        else{

        CrearProcesos(argv[1]);
        
        printf("\n\n*************************************************\n");
        printf("                       SJF                       \n");
        printf("*************************************************\n\n");
        
        SJF();
        
        printf("\n\n*************************************************\n");
        printf("                       RR                       \n");
        printf("*************************************************\n\n");
        
        RoundRobin();
        }
     
    }
    
    else {
        printf("Se debe introducir un unico parametro\n"
                "Sintaxis correcta: ./proyectoSO parametro1\n");
    }
    
    // Cerramos descriptor
    FindClose(hFind);
    // Para que nos deje ver los resultados.
    printf("\n\n** Pulse Intro para terminar el programa. **\n");
    getchar();
}

// Coge datos de el fichero pasado como parametro y los almacena en estructuras 
// de procesos. También cuenta el numero de procesos aprovechando el bucle.

void CrearProcesos(char fichero[]){
    
    FILE *archivo = fopen(fichero,"r");
    int i = 0;

    PROCESO nuevo;
    printf("\n*** PROCESOS ***\n\n");
    
    fscanf(archivo, "%d", &quantum);
    
    while(feof(archivo) == 0){
    
    fscanf(archivo, "%s %d %d", nuevo.nombre, &nuevo.tllegada, &nuevo.duracion);
        nuevo.usadoSJF = 0;
        nuevo.usadoRR = 0;
        lista[i] = nuevo;
    printf("Proceso %s = %d %d \n", lista[i].nombre, lista[i].tllegada, lista[i].duracion);
        nprocesos++;
        i++;
    }
    
    printf("Numero de procesos: %i\n\n", nprocesos);
    
    fclose(archivo);
}

// Gestiona procesos SJF 

void SJF(){
    
    int Ttotal = FTtotal();
    PROCESO actual;
    int i, j=0, nproceso;
    float utilizacion = 0;
    
    // MONTAS EL NOMBRE DEL FICHERO
    
    // ABRE FICHERO
    archivoSJF = fopen( nombreficheroSJF, "w" );
    
    fprintf(archivoSJF, "Tiempo total: %i\n\n", Ttotal);
    printf("Tiempo total: %i\n\n", Ttotal);
    fprintf(archivoSJF, "*** REPARTO TEMPORAL DE LA CPU ***\n\n");
    printf("*** REPARTO TEMPORAL DE LA CPU ***\n\n");
    fprintf(archivoSJF, "\t\tTpermanencia\tTrespuesta/espera\tCicloActual\n");
    printf("\t\tTpermanencia\tTrespuesta/espera\tCicloActual\n");
    
    while(ciclo < Ttotal){
        nproceso = EscogerProcesoActualSJF();
        actual = lista[nproceso];
        
        if(nproceso == 99){
            ciclo++;          
        }
        else{
        actual.tentrada = ciclo;
        ciclo += actual.duracion;
        GestionarActualSJF(actual);
        }
    }
    
    while(j < nprocesos){
        
        utilizacion += lista[j].duracion;
        j++;
    }
    
    utilizacion = utilizacion/Ttotal *100;
    
    // Imprime en pantalla y en el fichero la utilizacion (C).
    fprintf(archivoSJF, "Utilizacion del procesador %f \n\n", utilizacion);
    printf("Utilizacion del procesador %f \n\n", utilizacion);
    
    // CIERRA FICHERO
    fclose(archivoSJF);
}

// Obtiene tiempo total de procesamiento

int FTtotal(){
    
    int i, x = 0;
    PROCESO muestra = lista[0];
    
    for(i=0; i<nprocesos; i++){
       
       x += lista[i].duracion;
       
       if(lista[i].tllegada < muestra.tllegada){
             muestra = lista[i];
       }
       else if(muestra.tllegada == lista[i].tllegada){
           if(lista[i].duracion < muestra.duracion){
               muestra = lista[i];
           }
       }
    }
    
    x += muestra.tllegada;
    
    return x;
}

// Escoge el proceso a gestionar SJF

int EscogerProcesoActualSJF(){
    
    int i, a;
    // PROCESO de apoyo para encontrar el menor, se inicializa al proceso 0, aunque no siempre entra el 0;
    PROCESO pro = lista[0];
    boolean encontrado;
    
    // "Algoritmo" que busca si hay algun proceso, que no haya sido utilizado (ya que no son expulsables), 
    // que este listo para procesar y que sea el menor de todos los posibles a procesar (para cumplir el SJF).
    
    for(i=0; i<nprocesos; i++){

       if((lista[i].tllegada <= ciclo) && (lista[i].duracion <= pro.duracion) && (lista[i].usadoSJF == 0)){
             pro = lista[i];
             a = i;
             encontrado = 1;
       }
    }
    
    // Comprueba si se ha selecionado algun proceso. En caso contrario, devuelve
    // un 99 simbólico de que no hay ningún proceso selecionado, por lo que debe 
    // incrementar el ciclo y esperar que lleguen nuevos procesos.
    
    if(encontrado == 1){
        lista[a].usadoSJF = 1;
        return a;
    }
    else{
        return 99;
    }
}

// Gestiona proceso actual SJF (imprime en pantall y en el fichero de salida los resutados).
void GestionarActualSJF(PROCESO x){
    
    fprintf(archivoSJF, "Proceso %s \t\t %i \t\t %i \t\t %i \n\n", x.nombre, (ciclo-x.tllegada), (x.tentrada-x.tllegada), ciclo); 
    printf("Proceso %s \t\t %i \t\t %i \t\t %i \n\n", x.nombre, (ciclo-x.tllegada), (x.tentrada-x.tllegada), ciclo);
}


// Gestiona procesos RoundRobin
void RoundRobin(){
    
    int Ttotal = FTtotal();
    PROCESO actual;
    int i, j=0, nproceso;
    ciclo = 0;
    
    // ABRE FICHERO
    archivoRoundRobin = fopen( nombreficheroRoundRobin, "w" );
    
    fprintf(archivoRoundRobin, "Quantum: %i\n", quantum);
    printf("Quantum: %i\n", quantum);
    fprintf(archivoRoundRobin, "Tiempo total: %i\n\n", Ttotal);
    printf("Tiempo total: %i\n\n", Ttotal);
    fprintf(archivoRoundRobin, "*** REPARTO TEMPORAL DE LA CPU ***\n\n");
    printf("*** REPARTO TEMPORAL DE LA CPU ***\n\n");
    fprintf(archivoRoundRobin, "\t\tTpermanencia\tTespera\t\tTrespuesta\tCicloActual\n");
    printf("\t\tTpermanencia\tTespera\t\tTrespuesta\tCicloActual\n");
    
    while(ciclo < Ttotal){
        nproceso = EscogerProcesoActualRoundRobin();
        actual = lista[nproceso];
        if(nproceso == 99){
            ciclo++;          
        }
        else{
            if(actual.usadoRR == 0){   
                actual.cicloentradaPRO = 0;
                actual.ciclosalidaPRO = 0;
                actual.tespera = 0;
                actual.tentrada = actual.tllegada;
                actual.trespuesta = ciclo - actual.tentrada;
                actual.usadoRR = 1;
                if(actual.duracion >= quantum){
                    actual.ciclosalidaPRO = ciclo;
                }
                else{
                    actual.ciclosalidaPRO = ciclo;
                }
            }
            
            // Hacemos la comprobación por si el tiempo restante a gestionar fuera menor que el quantum.
            if(actual.duracion >= quantum){
            actual.cicloentradaPRO = ciclo;
            ciclo += quantum;
            actual.tespera = actual.tespera + (actual.cicloentradaPRO - actual.ciclosalidaPRO);
            actual.ciclosalidaPRO = ciclo;
            tpermanencia = quantum;
            }
            else{
            actual.cicloentradaPRO = ciclo;
            ciclo += actual.duracion;
            actual.tespera = actual.tespera + (actual.cicloentradaPRO - actual.ciclosalidaPRO);
            actual.ciclosalidaPRO = ciclo;
            tpermanencia = actual.duracion;
            }
            
        // Gestionamos el proceso y actualizamos parámetros.
        GestionarActualRoundRobin(actual);
        actual.duracion -= quantum;
        actual.tllegada = ciclo;
        lista[nproceso] = actual;
        }
    }
    
    utilizacion = utilizacion/Ttotal *100;
    
    // Imprime en pantalla y en el fichero la utilizacion (C).
    fprintf(archivoRoundRobin, "Utilizacion del procesador %f \n", utilizacion);
    printf("Utilizacion del procesador %f \n", utilizacion);
    
    // CIERRA FICHERO
    fclose(archivoRoundRobin);
}
  
// Escoge el proceso a gestionar RoundRobin
int EscogerProcesoActualRoundRobin(){
    
    int i, a;
    // PROCESO de apoyo para encontrar el menor, se inicializa al proceso 0, aunque no siempre entra el 0;
    PROCESO pro = lista[0];
    boolean encontrado;
    
    // "Algoritmo" que busca si hay algun proceso que este listo para procesar, que su tiempo de llegada sea el menor
    // (ya que lo utilizamos para darles preferencia a modo de cola de listos), y que su duracion sea mayor de 0, es decir,
    // que todavia tenga que ser gestionado.
    for(i=0; i<nprocesos; i++){

       if((lista[i].tllegada <= ciclo) && (lista[i].tllegada <= pro.tllegada) && (lista[i].duracion > 0)){
             pro = lista[i];
             a = i;
             encontrado = 1;
       }
    }
    
    // Comprueba si se ha selecionado algun proceso. En caso contrario, devuelve
    // un 99 simbólico de que no hay ningún proceso selecionado, por lo que debe 
    // incrementar el ciclo y esperar que lleguen nuevos procesos.
    if(encontrado == 1){
        return a;
    }
    else{
        return 99;
    }
}

// Gestiona proceso actual Round Robin (imprime en pantall y en el fichero de salida los resutados).
void GestionarActualRoundRobin(PROCESO x){
    
    utilizacion += tpermanencia;
    if(x.duracion-quantum <= 0){
    fprintf(archivoRoundRobin, "Proceso %s \t\t %i \t %i \t\t %i \t\t %i \n\n", x.nombre, (ciclo-x.tentrada), (x.tespera+x.trespuesta), x.trespuesta, ciclo); 
    printf("Proceso %s \t\t %i \t %i \t\t %i \t\t %i \n\n", x.nombre, (ciclo-x.tentrada), (x.tespera+x.trespuesta), x.trespuesta, ciclo);
}
}