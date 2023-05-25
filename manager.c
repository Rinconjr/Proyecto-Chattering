//*****************************************************************
// Autores: Juan Diego Echeverry, Santiago Yañez y Nicolás Rincón
// Proyecto chattering con uso de pipes
// Compilación: "gcc manager.c -o manager"
// Ejecución: "./manager –n N -p pipeNom"
// Observaciones: el numero maximo de usuarios que pueden estar conectados es 100.
//
//  Archivo creado por Juan Diego Echeverry, Santiago Yañez y Nicolás Rincón
//  Fecha de inicio: 6/05/23
//  Fecha de finalización: 
//*****************************************************************

//*****************************************************************
// LIBRERIAS INCLUIDAS
//*****************************************************************
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

//*****************************************************************
// DEFINICION DE VARIABLES GLOBALES
//*****************************************************************
#define MAX_N 100
int lista_usuarios[MAX_N] = {};
int talker_num = 0;
char *pipeGeneral;
int permisosPipe = 0666;


typedef struct mensaje {
  int id;
  char opcion[200];
  char texto[100];
} mensaje;

//*****************************************************************
// DECLARACIÓN DE FUNCIONES
//*****************************************************************


//*****************************************************************
// DECLARACIÓN DE VARIABLES
//*****************************************************************
const char *fifo = "validar";


//************************************************************************
// FUNCIONES
//************************************************************************
//Funcion list (Listar usuarios conectados) (falta hacerlo)
void listarUsuarios(char* pipe) {
  for (int i = 0; i < talker_num; i++) {
    if(lista_usuarios[i] == 0) {
      
    }
    else {
      printf("Usuario %d\n", lista_usuarios[i]);
    }
  }
}
//Funcion list GID (Listar integrantes de un grupo dado el id del grupo) (falta hacerlo)
void listarGrupo(char* pipe) {
  
}

//Funcion Group (crea un grupo con los usuarios que se pasen por parametro) (falta hacerlo)
void crearGrupo(char* pipe) {
  
}

//sent msg Idi (Enviar mensaje al talker con id N) (falta hacerlo)
void enviarMsg(char* pipe) {
  
}

//sent msg GroupIdi (Enviar mensaje al grupo de id n) (falta hacerlo)
void enviarMsgGrupo(char* pipe) {
  
}

//Salir (Sale y le informa al manager que salio) (falta hacerlo)
void salir(char* pipe) {
  
}

//Valida los argumentos al ejecutar el master
int validar_args(int argc, char *argv[]) {
  int n_flag_cant = 0;
  int p_flag_cant = 0;
  int n = 0;
  char *pipe_name;

  if (argc != 5) {
    printf("--------------------------\n");
    printf("Debe ingresar argumentos de la siguiente  manera: \n ./manager -n MaximoTalkers -p nombrePipe\n");
    printf("--------------------------\n");
    return 0;
  }

  for (int i = 1; i < argc; i += 2) {
    //Verificar bandera -n
    if (strcmp(argv[i], "-n") == 0) {
      // Verificar que no se haya ingresado la bandera -n varias veces
      if (n_flag_cant) {
        printf("--------------------------\n");
        printf("Error: Bandera -n duplicada\n");
        printf("--------------------------\n");
        return 0;
      }
      // Verificar que el argumento siguiente sea un número
      char *arg = argv[i + 1];
      for (int j = 0; j < strlen(arg); j++) {
        if (!isdigit(arg[j])) {
          printf("--------------------------\n");
          printf("Error: Debe ingresar un numero para -n\n");
          printf("--------------------------\n");
          return 0;
        }
      }
      n = atoi(arg);
      // Verificar que el número esté entre 1 y 100
      if (n < 1 || n > MAX_N) {
        printf("--------------------------\n");
        printf("Error: La cantidad de talkers debe estar entre 1 y %d\n", MAX_N);
        printf("--------------------------\n");
        return 0;
      }
      n_flag_cant = 1;
    } 
    //Verificar bandera -p
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
    } 
    else {
      printf("--------------------------\n");
      printf("Error: Bandera %s invalida. \n Utilice solo -n y -p \n", argv[i]);
      printf("--------------------------\n");
      return 0;
    }
  }

  talker_num = n;
  pipeGeneral = pipe_name;
  
  return 1;
}

//Funcion para validar que el id del talker es valido
int registrar(int num) {
  
}

//Responde si el id es valido o no
void responder_registro(int n) {
    int fd;
    char *fifo = "tmp_validation";
    mkfifo(fifo, 0666);
    char arr1[80], arr2[80];
    fd = open(fifo, O_WRONLY);
  //El id del talker no es valido
  if (n == 0) {
    write(fd, "0", strlen("0")+1);
  }
  //El id del talker es valido
  else {
    write(fd, "1", strlen("1")+1);
  }
  close(fd);
}

int main(int argc, char *argv[])
{
  int fd1;
  mensaje mensajeGeneral;
  //Se validan los argumentos
  if(!validar_args(argc, argv)) {
    return 0;
  }
  
  printf("Manager iniciado y el sistema podra tener como maximo %d usuarios\n", talker_num);

  if(mkfifo(pipeGeneral,permisosPipe)==-1){
    perror("Error al crear el pipe");
    exit(1);
  }

  
  
  while(1){
    fd1=open(pipeGeneral, O_RDONLY);
    if(fd1==-1){
      perror("Error al abrir el pipe)");
    }
    read(fd1,&mensajeGeneral,sizeof(mensaje));
    close(fd1);

    if (mensajeGeneral.opcion == 'c') {
      printf("hola");
    }
    else {
      printf("mundo");
    }
    
    printf("Opcion recibida de PID %s: %s",mensajeGeneral.texto, mensajeGeneral.opcion);
  };
 
    
  return 0;
}