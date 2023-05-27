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
#include <signal.h>

//*****************************************************************
// DEFINICION DE VARIABLES GLOBALES
//*****************************************************************
#define MAX_N 100
int fd;
int cant_talkers = 0;
int talker_num = 0;
char *pipeGeneral;
int permisosPipe = 0666;

typedef struct usuario {
  char id[20];
  char estado[10];
  pid_t pid;
}usuario;

typedef struct grupo {
  char igGrupo[20];
  usuario usuarios[MAX_N];
}grupo;

typedef struct mensaje {
  char idEnvia[20];
  char idRecibe[20];
  char opcion[200];
  char texto[100];
} mensaje;

struct usuario listaUsuarios[MAX_N];
struct grupo listaGrupos[MAX_N];


//*****************************************************************
// DECLARACIÓN DE VARIABLES
//*****************************************************************
const char *fifo = "validar";


//************************************************************************
// FUNCIONES
//************************************************************************

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
void salir(mensaje mensajeGeneral) {
  for (int i = 0; i < cant_talkers; i++) {
    if (strcmp(listaUsuarios[i].id, mensajeGeneral.idEnvia) == 0) {
      strcpy(listaUsuarios[i].estado, "0");
    }
  }
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
int registrar(mensaje mensajeGeneral) {
  int respuesta = 0;
  int pipe_fd;

  //COMPROBAR SI EL USUARIO EXITE, NO EXISTE O NO HAY ESPACIO
  for (int i = 0; i < cant_talkers; i++) {
    if(strcmp(mensajeGeneral.idEnvia, listaUsuarios[i].id) == 0) {
      if(strcmp(listaUsuarios[i].estado, "0") == 0) {
        listaUsuarios[cant_talkers].pid = atoi(mensajeGeneral.texto);
      }
      else {
        printf("usuario ya existe compañero\n");
        respuesta = 1;
      }
    }
  }
  if(respuesta == 0 && cant_talkers != talker_num) {
    printf("anadiendo usuario %s... \n", mensajeGeneral.idEnvia);
    
    strcpy(listaUsuarios[cant_talkers].id, mensajeGeneral.idEnvia);
    strcpy(listaUsuarios[cant_talkers].estado, "1");
    listaUsuarios[cant_talkers].pid = atoi(mensajeGeneral.texto);
    printf("test\n");
    cant_talkers++;
    respuesta = 2;
  }
  //COMPROBAR SI EL USUARIO EXITE, NO EXISTE O NO HAY ESPACIO

  //LLENAR MENSAJE DE ENVIO
  sprintf(mensajeGeneral.idRecibe, "%s", mensajeGeneral.idEnvia);
  sprintf(mensajeGeneral.idEnvia, "%d", 0);
  sprintf(mensajeGeneral.opcion, "%d", respuesta);
  //LLENAR MENSAJE DE ENVIO

  //PIPE CON EL TALKER
    // Crear el pipe no nominal
    if (mkfifo(mensajeGeneral.texto, 0666) == -1) {
        perror("Error al crear el pipe");
        exit(EXIT_FAILURE);
    }
    
    // Abrir el pipe para escribir
    pipe_fd = open(mensajeGeneral.texto, O_WRONLY);
    if (pipe_fd == -1) {
        perror("Error al abrir el pipe para escribir");
        exit(EXIT_FAILURE);
    }
    
    // Enviar el mensaje a través del pipe
    write(pipe_fd, &mensajeGeneral, sizeof(mensajeGeneral));
    
    // Cerrar el pipe
    close(pipe_fd);
    
    // Eliminar (unlink) el pipe
    if (unlink(mensajeGeneral.texto) == -1) {
        perror("Error al eliminar el pipe");
        exit(EXIT_FAILURE);
    }
  //PIPE CON EL TALKER
    return 0;
}


void responderTalker(mensaje mensajeGeneral) {
  pid_t tmp_pid = 0;
  int pipe_fd;
  char pipeRegreso[100];

  sprintf(pipeRegreso, "%s%s", pipeGeneral, mensajeGeneral.idRecibe);


  //COMPROBAR SI EL USUARIO ESTA CONECTADO, SI NO LO ESTA NO ENVIA NADA
  for (int i = 0; i < cant_talkers; i++) {
    if(strcmp(mensajeGeneral.idRecibe, listaUsuarios[i].id) == 0) {
      if(strcmp(listaUsuarios[i].estado, "1") == 0) {
          tmp_pid = listaUsuarios[i].pid;
      }
    }
  }
  kill(tmp_pid,SIGUSR1);
  if(tmp_pid == 0) {
    return;
  }
  
  //PIPE CON EL TALKER
    // Crear el pipe no nominal
    if (mkfifo(pipeRegreso, 0666) == -1) {
        perror("Error al crear el pipe");
        exit(EXIT_FAILURE);
    }
    
    // Abrir el pipe para escribir
    pipe_fd = open(pipeRegreso, O_WRONLY);
    if (pipe_fd == -1) {
        perror("Error al abrir el pipe para escribir");
        exit(EXIT_FAILURE);
    }
    
    // Enviar el mensaje a través del pipe
    write(pipe_fd, &mensajeGeneral, sizeof(mensajeGeneral));
    
    
    // Cerrar el pipe
    close(pipe_fd);
    
    // Eliminar (unlink) el pipe
  
    if (unlink(pipeRegreso) == -1) {
        perror("Error al eliminar el pipe");
        exit(EXIT_FAILURE);
    }
  //PIPE CON EL TALKER
  
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

//Funcion list (Listar usuarios conectados) (falta hacerlo)
void listarUsuarios(mensaje mensajeGeneral) {
  char numeroComoCadena[20];
  strcpy(mensajeGeneral.opcion, "RListar");
  strcpy(mensajeGeneral.texto, "Los usuarios actualmente en el sistema son: ");
  
  
  for (int i = 0; i < cant_talkers; i++) {
    if (strcmp(listaUsuarios[i].estado, "1") == 0) {
      strcat(mensajeGeneral.texto, listaUsuarios[i].id);
      if(i == cant_talkers-1) {
      break;
    }
    strcat(mensajeGeneral.texto, ",");
    }
  }

  strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
  responderTalker(mensajeGeneral);
}

int main(int argc, char *argv[])
{ 
  int fd;
  int vida = 1;
  char opcion[100];
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

  while(vida){
    fd=open(pipeGeneral, O_RDONLY);
    if(fd==-1){
      perror("Error al abrir el pipe)");
    }
    read(fd,&mensajeGeneral,sizeof(mensaje));
    close(fd);

    if (strcmp(mensajeGeneral.opcion, "registrar") == 0) {
      registrar(mensajeGeneral);
    }
    else if (strcmp(mensajeGeneral.opcion, "List") == 0) {
      printf("Opcion List\n");
      listarUsuarios(mensajeGeneral);
    }
    else if (strcmp(mensajeGeneral.opcion, "Group") == 0) {
      printf("Opcion Group\n");
    }
    else if (strcmp(mensajeGeneral.opcion, "Sent") == 0) {
      printf("Opcion Sent\n");

      //ESto es de prueba
      strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
      strcpy(mensajeGeneral.opcion, "llega");
      responderTalker(mensajeGeneral);
    }
    else if (strcmp(mensajeGeneral.opcion, "Salir") == 0) {
      printf("Opcion Salir\n");
      salir(mensajeGeneral);
    }
    else if (strcmp(mensajeGeneral.opcion, "kill") == 0) { //Listo para hacer
      //code
      vida = 0;
    }
    else {
      printf("Opcion de talker %s: %s\n",mensajeGeneral.idEnvia, mensajeGeneral.opcion);
    }

    printf(" ");
  };

  // Eliminar (unlink) el pipe
    if (unlink(pipeGeneral) == -1) {
        perror("Error al eliminar el pipe");
        exit(EXIT_FAILURE);
    }
    
  return 0;
}