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
const char *fifo = "validar";

//*****************************************************************
// DECLARACION DE ESTRUCTURAS
//*****************************************************************
typedef struct usuario {
  char id[20];
  char estado[10];
  pid_t pid;
}usuario;

typedef struct grupo {
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
// FUNCIONES
//*****************************************************************

//****************************************************************************************************************************************************
// Función para salir del programa e informar al manager
// Parametros de entrada: mensajeGeneral proveniente de la estructura "mensaje"
// Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************

void salir(mensaje mensajeGeneral) {
  for (int i = 0; i < cant_talkers; i++) {
    if (strcmp(listaUsuarios[i].id, mensajeGeneral.idEnvia) == 0) {
      strcpy(listaUsuarios[i].estado, "0");
    }
  }
}

//****************************************************************************************************************************************************
// Función para  validar los argumentos al ejecutar el master
// Parametros de entrada: Recibe los argumentos del usuario cuando se inicializa el programa
// Parametro que devuelve: Retorna 1 para informar de que si se puede iniciar el manager
//****************************************************************************************************************************************************

int validar_args(int argc, char *argv[]) {
  int n_flag_cant = 0;
  int p_flag_cant = 0;
  int n = 0;
  char *pipe_name = "";

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
      for (int j = 0; j < (int)strlen(arg); j++) {
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

//****************************************************************************************************************************************************
// Función para validar que el id del talker es valido
// Parametros de entrada: mensajeGeneral proveniente de la estructura "mensaje"
// Parametro que devuelve: Devuelve 0 para informar que no hubo errores en los recursos para registrar el talker
//****************************************************************************************************************************************************

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
  if(write(pipe_fd, &mensajeGeneral, sizeof(mensajeGeneral))==-1){
    perror("Error al escribir en el pipe");
  }
    
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

//****************************************************************************************************************************************************
// Función para responderle al talker al que se le debe enviar el mensaje
// Parametros de entrada: mensajeGeneral proveniente de la estructura "mensaje"
// Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************

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
  if(write(pipe_fd, &mensajeGeneral, sizeof(mensajeGeneral))==-1){
    perror("Error al escribir en el pipe");
  }
  
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

//****************************************************************************************************************************************************
// Función para crear un grupo con los usuarios que se pasen por parametro
// Parametros de entrada: mensajeGeneral proveniente de la estructura "mensaje"
// Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************

void crearGrupo(mensaje mensajeGeneral) { 
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

//****************************************************************************************************************************************************
// Función para listar integrantes de un grupo dado el id del grupo
// Parametros de entrada: mensajeGeneral proveniente de la estructura "mensaje"
// Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************

void listarGrupo(mensaje mensajeGeneral) {
  int buscarGrupo;
  strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
  //Validar que la primera letra es una G y el resto numeros
  if (mensajeGeneral.texto[0] == 'G') {
    for (int i = 1; mensajeGeneral.texto[i] != '\0'; i++) {
        if (!isdigit(mensajeGeneral.texto[i])) {
          strcpy(mensajeGeneral.texto, "No se puede listar. Ingrese el id del grupo");
          responderTalker(mensajeGeneral);
          return;
        }
    }
  }
  else {
    printf("No ingreso G\n");
    strcpy(mensajeGeneral.texto, "No se puede listar. Si desea listar un grupo ingrese la G primero");
    responderTalker(mensajeGeneral);
    return;
  }

  //Encontrar grupo, ignora la G inicial
  buscarGrupo = atoi(mensajeGeneral.texto + 1);

  //Buscar grupo
  if( buscarGrupo > cant_grupos || buscarGrupo <= 0) {
    strcpy(mensajeGeneral.texto, "No se pudo encontrar el grupo.");
    responderTalker(mensajeGeneral);
    return;
  }

 //Responder a talker
  else {
    strcpy(mensajeGeneral.texto, "El grupo ");
    sprintf(mensajeGeneral.texto + strlen(mensajeGeneral.texto), "%d", buscarGrupo);
    strcat(mensajeGeneral.texto, " tiene los siguientes usuarios:");
    
    for(int e = 0; e <listaGrupos[buscarGrupo-1].numeroUsuarios; e++) {
      strcat(mensajeGeneral.texto, "\n");
      strcat(mensajeGeneral.texto, "Usuario ");
      sprintf(mensajeGeneral.texto + strlen(mensajeGeneral.texto), "%d", listaGrupos[buscarGrupo-1].idUsuarios[e]); 
    }
    responderTalker(mensajeGeneral);
    return;
  }
}

//****************************************************************************************************************************************************
// Función para listar usuarios conectados
// Parametros de entrada: mensajeGeneral proveniente de la estructura "mensaje"
// Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************

void listarUsuarios(mensaje mensajeGeneral) {
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

//****************************************************************************************************************************************************
// Función para acabar con el proceso de forma espontena
// Parametros de entrada: mensajeGeneral proveniente de la estructura "mensaje"
// Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************

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

//****************************************************************************************************************************************************
// Función para obtener la longitud
// Parametros de entrada: Arreglo de caracteres
// Parametro que devuelve: Longitud del arreglo 
//****************************************************************************************************************************************************

int obtener_longitud(const char *arreglo) {
    int longitud = 0;
    while (arreglo[longitud] != '\0') {
        longitud++;
    }
    return longitud;
}

//****************************************************************************************************************************************************
// Función para enviar mensaje al talker con id N
// Parametros de entrada: mensajeGeneral proveniente de la estructura "mensaje"
// Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************

void MsgUsuario(mensaje mensajeGeneral) {
  char tmpText[100] = "Mensaje del Usuario ";
  strcat(tmpText, mensajeGeneral.idEnvia);
  strcat(tmpText, ": ");

  if (*mensajeGeneral.texto == '\0') {
    strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
    strcpy(mensajeGeneral.idEnvia, "0");
    strcpy(mensajeGeneral.texto, "Para enviar un mensaje ingrese el texto, y el destinatario");
    responderTalker(mensajeGeneral);
    return;
  }
  else if (*mensajeGeneral.idRecibe == '\0') {
    strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
    strcpy(mensajeGeneral.idEnvia, "0");
    strcpy(mensajeGeneral.texto, "Para enviar un mensaje ingrese el destinatario");
    responderTalker(mensajeGeneral);
    return;
  }
  
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

//****************************************************************************************************************************************************
// Función para comprobar si el dato es numerico o no lo es 
// Parametros de entrada: Referencia a la cadena ruta char
// Parametro que devuelve: 0 o 1 dependiendo de si es digito o no es digito
//****************************************************************************************************************************************************

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

//****************************************************************************************************************************************************
// Función para enviar mensaje al grupo de id n) (falta hacerlo
// Parametros de entrada: mensajeGenereal proveniente de la estructura "mensaje"
// Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************

void MsgGrupo(mensaje mensajeGeneral) {
  char tmpText[100] = "Mensaje del Usuario ";
  strcat(tmpText, mensajeGeneral.idEnvia);
  strcat(tmpText, ": ");
  
  int buscarGrupo;
  //Validar que la primera letra es una G y el resto numeros
  if (mensajeGeneral.idRecibe[0] == 'G') {
    for (int i = 1; mensajeGeneral.idRecibe[i] != '\0'; i++) {
        if (!isdigit(mensajeGeneral.idRecibe[i])) {
          strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
          strcpy(mensajeGeneral.texto, "No se puede enviar el mensaje. Revise el destinatario");
          responderTalker(mensajeGeneral);
          return;
        }
    }
  }
  else {
    printf("No ingreso G\n");
    strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
    strcpy(mensajeGeneral.texto, "No se puede enviar el mensaje. \nPara mandar mensaje a un grupo debe ingresar la G primero");
    responderTalker(mensajeGeneral);
    return;
  }

  //Encontrar grupo, ignora la G inicial
  buscarGrupo = atoi(mensajeGeneral.idRecibe + 1);

  //Buscar grupo
  if( buscarGrupo > cant_grupos || buscarGrupo <= 0) {
    strcpy(mensajeGeneral.idRecibe, mensajeGeneral.idEnvia);
    strcpy(mensajeGeneral.texto, "No se pudo encontrar el grupo. Revise el destinatario");
    responderTalker(mensajeGeneral);
    return;
  }

 //Responder a talkers
  else {
    strcat(tmpText, mensajeGeneral.texto);
    strcpy(mensajeGeneral.texto, tmpText);
    for(int e = 0; e <listaGrupos[buscarGrupo-1].numeroUsuarios; e++) {
      sprintf(mensajeGeneral.idRecibe, "%d", listaGrupos[buscarGrupo-1].idUsuarios[e]);
      responderTalker(mensajeGeneral);
    }
  }
}