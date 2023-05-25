//*****************************************************************
// Autores: Juan Diego Echeverry, Santiago Yañez y Nicolás Rincón
// Proyecto chattering con uso de pipes
// Compilación: "gcc talker.c -o talker"
// Ejecución: "./talker –i idTalker -p pipeNom"
// Observaciones: el numero maximo de usuarios que pueden estar conectados es
// 100.
//
//  Archivo creado por Juan Diego Echeverry, Santiago Yañez y Nicolás Rincón
//  Fecha de inicio: 6/05/23
//  Fecha de finalización:
//*****************************************************************

//*****************************************************************
// LIBRERIAS INCLUIDAS
//*****************************************************************
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

//*****************************************************************
// DEFINICION DE VARIABLES GLOBALES
//*****************************************************************
#define MAX_i 100

int id_talker = 0;
char *nombre_pipe;
char *pipe_talker;

//*****************************************************************
// DECLARACIÓN DE VARIABLES
//*****************************************************************

const char *pipeGeneral;
const int permisosPipe = 0666;

//********************************************************************************
// DECLARACION DE ESTRUCTURAS
//********************************************************************************
typedef struct mensaje {
  int id;
  char opcion;
  char texto[10];
} mensaje;

//************************************************************************
// FUNCIONES
//************************************************************************

// Poner que es la funcion para mostrar el menu
void menu() {
  printf("----------------------\n");
  printf("Menu\n");
  printf("List\n");
  printf("List GID\n");
  printf("Group\n");
  printf("Sent msg id\n");
  printf("Sent msg Gid\n");
  printf("Salir\n");
  printf("----------------------\n");
}

// Valida los argumentos
int validar_args(int argc, char *argv[]) {
  int i_flag_cant = 0;
  int p_flag_cant = 0;
  int id = 0;
  char *pipe_name;

  if (argc != 5) {
    printf("--------------------------\n");
    printf("Debe ingresar argumentos de la siguiente  manera: \n ./talker -i "
           "ID -p nombrePipe\n");
    printf("--------------------------\n");
    return 0;
  }

  for (int i = 1; i < argc; i += 2) {
    // Verificar bandera -i
    if (strcmp(argv[i], "-i") == 0) {
      // Verificar que no se haya ingresado la bandera -i varias veces
      if (i_flag_cant) {
        printf("--------------------------\n");
        printf("Error: Bandera -i duplicada\n");
        printf("--------------------------\n");
        return 0;
      }
      // Verificar que el argumento siguiente sea un número
      char *arg = argv[i + 1];
      for (int j = 0; j < strlen(arg); j++) {
        if (!isdigit(arg[j])) {
          printf("--------------------------\n");
          printf("Error: Debe ingresar un numero para -i\n");
          printf("--------------------------\n");
          return 0;
        }
      }
      id = atoi(arg);
      // Verificar que el número esté entre 1 y 100
      if (id < 1 || id > MAX_i) {
        printf("--------------------------\n");
        printf("Error: El id del talker debe estar entre 1 y %d\n", MAX_i);
        printf("--------------------------\n");
        return 0;
      }
      i_flag_cant = 1;
    }
    // Verificar bandera -p
    else if (strcmp(argv[i], "-p") == 0) {
      // Verificar que no se haya ingresado la bandera -p antes
      if (p_flag_cant) {
        printf("--------------------------\n");
        printf("Error: Bandera -p duplicada\n");
        printf("--------------------------\n");
        return 0;
      }
      // Verificar que el argumento siguiente no sea vacío
      char *arg = argv[i + 1];
      if (strlen(arg) == 0) {
        printf("--------------------------\n");
        printf("Error: Argumento invalido para -p\n");
        printf("--------------------------\n");
        return 0;
      }
      pipe_name = arg;
      p_flag_cant = 1;
    } else {
      printf("--------------------------\n");
      printf("Error: Bandera %s invalida. \n Utilice solo -n y -p \n", argv[i]);
      printf("--------------------------\n");
      return 0;
    }
  }
  id_talker = id;
  pipeGeneral = pipe_name;
  return 1;
}

// Valida el id del talker
bool registrar() {}

//---------------------------------------------------------------------------------------------

//*********************************************************************************************************
// PROGRAMA PRINCIPAL
//*********************************************************************************************************

int main(int argc, char *argv[]) {
  pid_t pid = getpid();                     //ESTO LO CAMBIE
  char inputChar;                           //ESTO LO CAMBIE
  int fd1;
  mensaje mensajeGeneral;

  if (!validar_args(argc, argv)) {
    return 0;
  }

  printf("Se inicia un talker con id %d y PID  %d\n", id_talker, pid);
  printf("Nombre pipe del argv: %s\n", pipeGeneral);

  fd1 = open(pipeGeneral, O_WRONLY);
  if(fd1==-1){
    perror("Error al abrir el pipe");
    exit(1);
  }
  
  while(1){
    menu();
    printf("Elige una opcion: ");


    //ESTO LO CAMBIE
    scanf(" %c", &mensajeGeneral.opcion);
    while ((inputChar = getchar()) != '\n' && inputChar != EOF);// Descartar los caracteres restantes

    sprintf(mensajeGeneral.texto, "%d", pid); //guarda el pid del proceso 
    
    printf("\nOpcion ingresada: %c\n", mensajeGeneral.opcion);
    //ESTO LO CAMBIE

    
    write(fd1, &mensajeGeneral, sizeof(mensaje));
  }
  close(fd1);
  return 0;
} 