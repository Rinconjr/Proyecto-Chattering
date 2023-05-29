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
#define SIG_DATA_AVAILABLE SIGUSR1

#define MAX_N 100
int fd;
int cant_talkers = 0;
int talker_num = 0;
int cant_grupos = 0;
char *pipeGeneral;
int permisosPipe = 0666;

typedef struct usuario {
  char id[20];
  char estado[10];
  pid_t pid;
}usuario;

typedef struct grupo {
  //char idGrupo[20];
  int numeroUsuarios;
  char idUsuarios[20];
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

//Salir (Sale y le informa al manager que salio) (falta hacerlo)
void salir(mensaje mensajeGeneral) {
  for (int i = 0; i < cant_talkers; i++) {
    if (strcmp(listaUsuarios[i].id, mensajeGeneral.idEnvia) == 0) {
      strcpy(listaUsuarios[i].estado, "0");

      //pid_t pid =listaUsuarios[i].pid;
      //kill(pid, SIGUSR1);
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
        listaUsuarios[i].pid = atoi(mensajeGeneral.texto);
        strcpy(listaUsuarios[i].estado, "1");
        respuesta = 2;
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
  printf("---------Mensaje-Enviado------\n");
  printf("IdEnvia: %s\n", mensajeGeneral.idEnvia);
  printf("IdRecibe: %s\n", mensajeGeneral.idRecibe);
  printf("Opcion: %s\n", mensajeGeneral.opcion);
  printf("Texto: %s\n", mensajeGeneral.texto);
  printf("---------------------------------------------------\n");

  
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
          printf("EL PID AL QUE SE LE ENVIA: %d\n", tmp_pid);
      }
    }
  }
  if(tmp_pid == 0) {
    return;
  }
  pid_t receptor_pid = tmp_pid;
  kill(receptor_pid, SIG_DATA_AVAILABLE);
  
  //PIPE CON EL TALKER
    // Crear el pipe no nominal
    if (mkfifo(pipeRegreso, 0666) == -1) {
        perror("Error al crear el pipe");
        exit(EXIT_FAILURE);
    }
    
    // Abrir el pipe para escribir y enviar
    pipe_fd = open(pipeRegreso, O_WRONLY);
    if (pipe_fd == -1) {
      perror("Error al abrir el pipe para escribir");
      exit(EXIT_FAILURE);
    }
    write(pipe_fd, &mensajeGeneral, sizeof(mensajeGeneral));
    close(pipe_fd);
    
    // Eliminar (unlink) el pipe
    if (unlink(pipeRegreso) == -1) {
        perror("Error al eliminar el pipe");
        exit(EXIT_FAILURE);
    }
  //PIPE CON EL TALKER

  printf("---------Mensaje enviado------\n");
  printf("IdEnvia: %s\n", mensajeGeneral.idEnvia);
  printf("IdRecibe: %s\n", mensajeGeneral.idRecibe);
  printf("Opcion: %s\n", mensajeGeneral.opcion);
  printf("Texto: %s\n", mensajeGeneral.texto);
  printf("---------------------------------------------------\n");
}

//Funcion Group (crea un grupo con los usuarios que se pasen por parametro) (falta hacerlo)
void crearGrupo(mensaje mensajeGeneral) { //Falta validar que no se puede crear el grupo con un usuario inexistente
  int count = 0;
  int existe = 0;
  char *token = strtok(mensajeGeneral.texto, ", ");

  while (token != NULL && count < MAX_N) {
    if (strcmp(mensajeGeneral.idEnvia, token) != 0) {
      existe = 0;
      for (int i = 0; i < cant_talkers; i++) {
        if (strcmp(listaUsuarios[i].id, token) == 0) {
          existe = 1;
        }
      }
      if(existe) {
        listaGrupos[cant_grupos].idUsuarios[count] = atoi(token);
        count++;
        token = strtok(NULL, ", ");
        listaGrupos[cant_grupos].numeroUsuarios++;
      }
      else {
        strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
        strcpy(mensajeGeneral.idEnvia, "0");
        strcpy(mensajeGeneral.texto, "No puede crear un grupo con usuarios que no existan");
        responderTalker(mensajeGeneral);
        return;
      }
    }
    else {
      strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
        strcpy(mensajeGeneral.idEnvia, "0");
        strcpy(mensajeGeneral.texto, "No agregue su id para la creacion de grupos,\nusted ya esta incluido al momento de crear grupos");
        responderTalker(mensajeGeneral);
        return;
    }
  }
  if (count == 0) {
    strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
    strcpy(mensajeGeneral.idEnvia, "0");
    strcpy(mensajeGeneral.texto, "Agregue usuarios para crear un grupo");
    responderTalker(mensajeGeneral);
    return;
  }

  listaGrupos[cant_grupos].idUsuarios[count] = atoi(mensajeGeneral.idEnvia);
  listaGrupos[cant_grupos].numeroUsuarios++;

  strcpy(mensajeGeneral.texto, "Grupo ");
  sprintf(mensajeGeneral.texto + strlen(mensajeGeneral.texto), "%d", cant_grupos+1);
  strcat(mensajeGeneral.texto, " creado. Los usuarios de este son:");
  
  for (int i = 0; i < listaGrupos[cant_grupos].numeroUsuarios; i++) {
    strcat(mensajeGeneral.texto, "\n");
    strcat(mensajeGeneral.texto, "Usuario ");
    sprintf(mensajeGeneral.texto + strlen(mensajeGeneral.texto), "%d", listaGrupos[cant_grupos].idUsuarios[i]);
  }
  cant_grupos++;

  responderTalker(mensajeGeneral); 
}

//Funcion list GID (Listar integrantes de un grupo dado el id del grupo) (falta hacerlo)
void listarGrupo(mensaje mensajeGeneral) {
  int grupoSolicitado = atoi(mensajeGeneral.texto);

  strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);

  if(grupoSolicitado < cant_grupos +1 && grupoSolicitado > 0 ) {

    strcpy(mensajeGeneral.texto, "El grupo ");
    sprintf(mensajeGeneral.texto + strlen(mensajeGeneral.texto), "%d", grupoSolicitado);
    strcat(mensajeGeneral.texto, " tiene los siguientes usuarios:");
    
    for(int e = 0; e <listaGrupos[grupoSolicitado-1].numeroUsuarios; e++) {
      strcat(mensajeGeneral.texto, "\n");
      strcat(mensajeGeneral.texto, "Usuario ");
      sprintf(mensajeGeneral.texto + strlen(mensajeGeneral.texto), "%d", listaGrupos[grupoSolicitado-1].idUsuarios[e]); 
    }
  }
  else {
    strcpy(mensajeGeneral.texto, "El grupo no existe");
  }
  responderTalker(mensajeGeneral);
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

void killAll(mensaje mensajeGeneral) {
  strcpy(mensajeGeneral.opcion, "kill");
  strcpy(mensajeGeneral.texto, "Se ha cerrado los servicios del manager");

  for (int i = 0; i < cant_talkers; i++) {
    if (strcmp(listaUsuarios[i].estado, "1") == 0) {
      strcpy(mensajeGeneral.idRecibe, listaUsuarios[i].id);
      responderTalker(mensajeGeneral);
    } 
  }

  // Eliminar (unlink) el pipe
  if (unlink(pipeGeneral) == -1) {
    perror("Error al eliminar el pipe");
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

  //sent msg Idi (Enviar mensaje al talker con id N) (falta hacerlo)
void MsgUsuario(mensaje mensajeGeneral) {
  char tmpText[100] = "Mensaje directo de usuario ";
  strcat(tmpText, mensajeGeneral.idEnvia);
  strcat(tmpText, ": ");
  
  for (int i = 0; i < cant_talkers; i++) {
    if(strcmp(mensajeGeneral.idRecibe, listaUsuarios[i].id) == 0) {
      if(strcmp(listaUsuarios[i].estado, "1") == 0) {
        strcat(tmpText, mensajeGeneral.texto);
        strcpy(mensajeGeneral.texto, tmpText);
        responderTalker(mensajeGeneral);
        return;
      }
      else {
        strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
        strcpy(mensajeGeneral.idEnvia, "0");
        strcpy(mensajeGeneral.texto, "El usuario no esta conectado");
        responderTalker(mensajeGeneral);
        return;
      }
    }
  }
  strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
  strcpy(mensajeGeneral.idEnvia, "0");
  strcpy(mensajeGeneral.texto, "El usuario no existe");
  responderTalker(mensajeGeneral);
  return;
}

int esNumerico(const char *cadena) {
    int i = 0;
    while (cadena[i] != '\0') {
        if (!isdigit(cadena[i])) {
            return 0;  // No es un dígito
        }
        i++;
    }
    return 1;
}

//sent msg GroupIdi (Enviar mensaje al grupo de id n) (falta hacerlo)
void MsgGrupo(mensaje mensajeGeneral) {
  int buscarGrupo;
  //Validar que la primera letra es una G y el resto numeros
  if (mensajeGeneral.idRecibe[0] == 'G') {
    for (int i = 1; mensajeGeneral.idRecibe[i] != '\0'; i++) {
        if (!isdigit(mensajeGeneral.idRecibe[i])) {
          strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
          strcpy(mensajeGeneral.texto, "No se puede enviar el mensaje. Revise el destinatario");
          //responderTalker(mensajeGeneral);
          return;
        }
    }
  }
  else {
    strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
    strcpy(mensajeGeneral.texto, "No se puede enviar el mensaje.\n");
    strcat(mensajeGeneral.texto, "Para enviar mensaje a un grupo, escriba G seguido del numero del grupo.");
    //responderTalker(mensajeGeneral);
    return;
  }

  //Encontrar grupo, ignora la G inicial
  buscarGrupo = atoi(mensajeGeneral.idRecibe + 1);

  //Buscar grupo
  if( buscarGrupo > cant_grupos || buscarGrupo < 0) {
    strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
    strcpy(mensajeGeneral.texto, "No se pudo encontrar el grupo. Revise el destinatario");
    //responderTalker(mensajeGeneral);
    return;
  }
  else {
    printf("El grupo tiene los siguientes usuarios: \n");

  }
}

int main(int argc, char *argv[])
{ 
  int fd;
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

  while(1){
    fd=open(pipeGeneral, O_RDONLY);
    if(fd==-1){
      perror("Error al abrir el pipe)");
    }
    read(fd,&mensajeGeneral,sizeof(mensaje));
    close(fd);

    printf("\n---------------------------------------------------\n");
    printf("---------Mensaje recibido------\n");
    printf("IdEnvia: %s\n", mensajeGeneral.idEnvia);
    printf("IdRecibe: %s\n", mensajeGeneral.idRecibe);
    printf("Opcion: %s\n", mensajeGeneral.opcion);
    printf("Texto: %s\n\n", mensajeGeneral.texto);

    if (strcmp(mensajeGeneral.opcion, "registrar") == 0) {
      registrar(mensajeGeneral);
    }
    else if (strcmp(mensajeGeneral.opcion, "List") == 0) {
      printf("Opcion List\n");
      if (obtener_longitud(mensajeGeneral.texto) == 0) {
        listarUsuarios(mensajeGeneral);
      }
      else {
        listarGrupo(mensajeGeneral);
      }
    }
    else if (strcmp(mensajeGeneral.opcion, "/kill") == 0) {
      killAll(mensajeGeneral);
      exit(0);
    }
    else if (strcmp(mensajeGeneral.opcion, "Sent") == 0) {
      if (esNumerico(mensajeGeneral.idRecibe)) {
        MsgUsuario(mensajeGeneral);
      }
      else {
        MsgGrupo(mensajeGeneral);
      }
    }
    else if(strcmp(mensajeGeneral.opcion, "Group") == 0){
      if(strcmp(mensajeGeneral.texto," ")){
        printf("Opcion Listar Grupo\n");
      }
      strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
      crearGrupo(mensajeGeneral);
    }
    else if (strcmp(mensajeGeneral.opcion, "Salir") == 0) {
      salir(mensajeGeneral);
    }
    else {
      strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
      strcpy(mensajeGeneral.texto, "Opcion invalida");
      responderTalker(mensajeGeneral);
    }

    printf(" ");
  };
  return 0;
}