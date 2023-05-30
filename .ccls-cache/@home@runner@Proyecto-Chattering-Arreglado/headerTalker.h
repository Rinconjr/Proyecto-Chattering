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
#include <signal.h>

//*****************************************************************
// DEFINICION DE VARIABLES GLOBALES
//*****************************************************************
#define SIG_DATA_AVAILABLE SIGUSR1
#define MAX_i 100
int id_talker = 0;
int fd;
char *nombre_pipe;
char *pipe_talker;
int esperaInput = 0;
const char *pipeGeneral;
const int permisosPipe = 0666;

//*****************************************************************
// DECLARACION DE ESTRUCTURAS
//*****************************************************************
typedef struct mensaje {
  char idEnvia[20];
  char idRecibe[20];
  char opcion[200];
  char texto[100];
} mensaje;

//*****************************************************************
// FUNCIONES
//*****************************************************************

//****************************************************************************************************************************************************
// Función para mostrarle el menu al usuario
// Parametros de entrada: Ninguno
// Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************

void menu() {
  printf("\n----------Menu----------\n");
  printf("List\n");
  printf("List GID\n");
  printf("Group\n");
  printf("Sent msg id\n");
  printf("Sent msg Gid\n");
  printf("Salir\n");
  printf("--------Fin-Menu--------\n");
}

//****************************************************************************************************************************************************
// Función para validar los argumentos ingresados por el usuario
// Parametros de entrada: argc y argv
// Parametro que devuelve: Si todo sale bien, retorna un entero con valor 1, de lo contrario, retorna 0 si algo sale mal
//****************************************************************************************************************************************************

int validar_args(int argc, char *argv[]) {
  int i_flag_cant = 0;
  int p_flag_cant = 0;
  int id = 0;
  char *pipe_name="";

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
      for (int j = 0; j < (int)strlen(arg); j++) {
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

//****************************************************************************************************************************************************
// Función para revisar si es posible registrar al usuario con el id solicitado
// Parametros de entrada: Estructura tipo mensaje y pid del proceso que pide registrarse
// Parametro que devuelve: Si todo sale bien, retorna un booleano con valor 0 para mostrar que se pudo registrar
//****************************************************************************************************************************************************

bool registrar(mensaje mensajeGeneral, pid_t pid) {
  //Para preguntarle al manager si puede ingresar/
  fd = open(pipeGeneral, O_WRONLY);
  if(fd==-1){
    perror("Error al abrir el pipe");
    exit(1);
  }
  
  sprintf(mensajeGeneral.idEnvia, "%d", id_talker);
  strcpy(mensajeGeneral.idRecibe, "0");
  strcpy(mensajeGeneral.opcion, "registrar");
  sprintf(mensajeGeneral.texto, "%d", pid); //guarda el pid del proceso 
  
  if(write(fd, &mensajeGeneral, sizeof(mensajeGeneral))==-1){
    perror("Error al escribir en el pipe");
  }
  close(fd);
  //Para preguntarle al manager si puede ingresar/

  //RECIBIR RESPUESTA DEL MANAGER
  int fd1;
  int bytes_read;
    
  // Abrir el pipe para leer
  do {
    fd1 = open(mensajeGeneral.texto, O_RDONLY);
  } while(fd1 == -1);
    
  // Leer del pipe
  bytes_read = read(fd1,&mensajeGeneral,sizeof(mensaje));
  if (bytes_read == -1) {
    perror("Error al leer del pipe");
    exit(1);
  }

  if(strcmp(mensajeGeneral.opcion, "0") == 0) {
    printf("-------------------------\n");
    printf("Lo sentimos, no hay espacio en el chat\n");
    printf("-------------------------\n");
    exit(1);
  }
  else if(strcmp(mensajeGeneral.opcion, "1") == 0) {
    printf("-------------------------\n");
    printf("Lo sentimos, el id con el que quiere ingresar ya existe\n");
    printf("-------------------------\n");
    exit(1);
  }
  
  // Cerrar el pipe
  close(fd1);
  //RECIBIR RESPUESTA DEL MANAGER
  
  return 0;
}

//****************************************************************************************************************************************************
// Función para 
// Parametros de entrada: Estructura tipo mensaje del talker
// Parametro que devuelve: Guarda el mensaje en la variable global de mensaje
//****************************************************************************************************************************************************

void recibirRespuesta(mensaje* mensajeGeneral) {
  char nombrePipe[100];
  sprintf(nombrePipe, "%s%d", pipeGeneral, id_talker);

  // RECIBIR RESPUESTA DEL MANAGER
  int fd1;
  int bytes_read;
    
  // Abrir el pipe para leer
  do {
    fd1 = open(nombrePipe, O_RDONLY);
  } while(fd1 == -1);
    
  // Leer del pipe
  bytes_read = read(fd1, mensajeGeneral, sizeof(mensaje));
  if (bytes_read == -1) {
    perror("Error al leer del pipe");
    exit(EXIT_FAILURE);
  }
}

//****************************************************************************************************************************************************
// Función para manejar las señales provenientes por el manager
// Parametros de entrada: Ninguno
// Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************

void data_available_handler() {
  printf("\n------Mensaje-Nuevo------\n");
  mensaje miMensaje;
  recibirRespuesta(&miMensaje);
  printf("\n%s\n", miMensaje.texto);
  printf("\n-------------------------\n");

  if (strcmp(miMensaje.opcion, "kill") == 0) {
    exit(0);
  }

  //Si esta esperando un input, vuelve a imprimir menu
  //Una vez se salga del handler, reanudara el fget del main
  if (esperaInput) {
    menu();
    printf("Elige una opcion: \n");
  }
}