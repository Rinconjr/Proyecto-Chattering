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

//*****************************************************************
// DECLARACIÓN DE VARIABLES
//*****************************************************************

const char *pipeGeneral;
const int permisosPipe = 0666;

//********************************************************************************
// DECLARACION DE ESTRUCTURAS
//********************************************************************************
typedef struct mensaje {
  char idEnvia[20];
  char idRecibe[20];
  char opcion[200];
  char texto[100];
  
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
  
  write(fd, &mensajeGeneral, sizeof(mensajeGeneral));
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
    exit(EXIT_FAILURE);
  }
    
  // Mostrar el mensaje recibido
  printf("Mensaje recibido mi pid es %s y la respuesta es: %s\n", mensajeGeneral.texto, mensajeGeneral.opcion);

  if(strcmp(mensajeGeneral.opcion, "0") == 0) {
    printf("-------------------------\n");
    printf("Lo sentimos, no hay espacio que triste\n");
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

int obtener_longitud(const char *arreglo) {
    int longitud = 0;
    while (arreglo[longitud] != '\0') {
        longitud++;
    }
    return longitud;
}

void popOpcion(char *opt, char *opcion) {
  sscanf(opt, "%s", opcion);
  memmove(opt, opt + strlen(opcion) + 1, strlen(opt) - strlen(opcion));
}

void data_available_handler(int signum) {
  printf("\n------mensaje nuevo------\n");
  mensaje miMensaje;
  recibirRespuesta(&miMensaje);
  printf("\n%s\n", miMensaje.texto);
  printf("\n-------------------------\n");

  //Si esta esperando un input, vuelve a imprimir menu
  //Una vez se salga del handler, reanudara el fget del main
  if (esperaInput) {
    menu();
    printf("Elige una opcion: \n");
  }
}
//---------------------------------------------------------------------------------------------

//*********************************************************************************************************
// PROGRAMA PRINCIPAL
//*********************************************************************************************************

int main(int argc, char *argv[]) {
  pid_t pid = getpid();
  char input[100];
  mensaje mensajeGeneral;

  if (!validar_args(argc, argv)) {
    return 0;
  }
  registrar(mensajeGeneral, pid);

  signal(SIG_DATA_AVAILABLE, data_available_handler);
  
  while(1){
    menu();
    printf("Elige una opcion: ");

    esperaInput = 1;
    fgets(input, sizeof(input), stdin);
    esperaInput = 0;
    
    memset(mensajeGeneral.opcion, 0, sizeof(mensajeGeneral.opcion));
    memset(mensajeGeneral.texto, 0, sizeof(mensajeGeneral.texto));
    memset(mensajeGeneral.idRecibe, 0, sizeof(mensajeGeneral.idRecibe));

    sscanf(input, "%s", mensajeGeneral.opcion);
    //printf("Opcion ingresada: %s\n", mensajeGeneral.opcion);

    if(strcmp(mensajeGeneral.opcion, "List") == 0){
      sscanf(input, "%*s %19s", mensajeGeneral.idRecibe);
    }
    else if(strcmp(mensajeGeneral.opcion, "Group") == 0){
      sscanf(input, "%*s %19s", mensajeGeneral.texto);
    }
    else if(strcmp(mensajeGeneral.opcion, "Sent") == 0){
      sscanf(input, "%*s \"%99[^\"]\"", mensajeGeneral.texto);
      sscanf(strrchr(input, '\"') + 1, " %19[^\"]%*c", mensajeGeneral.idRecibe);

      //Esto lo agregue porque el regex esta pasando un caracter extraño al final
      char ir[100];
      int indice = 0;
      for (int i = 0; mensajeGeneral.idRecibe[i] != '\0'; i++) {
        if (isalnum(mensajeGeneral.idRecibe[i])) {
            ir[indice] = mensajeGeneral.idRecibe[i];
            indice++;
        }
      }
      ir[indice] = '\0';
      strcpy(mensajeGeneral.idRecibe,ir);
    }
    else{
      printf("Dentro del if Salir\n");
    }

    sprintf(mensajeGeneral.idEnvia, "%d", id_talker);
    
    //printf("Texto ingresado: %s\n", mensajeGeneral.texto);
    //printf("Id para enviar ingresado: %s\n", mensajeGeneral.idRecibe);
    fd = open(pipeGeneral, O_WRONLY);
    if(fd==-1){
      perror("Error al abrir el pipe");
      exit(1);
    }
    write(fd, &mensajeGeneral, sizeof(mensajeGeneral));
    close(fd); 

    usleep(500000);
  }
  return 0;
} 