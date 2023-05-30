//*****************************************************************
// Autores: Juan Diego Echeverry, Santiago Yañez y Nicolás Rincón
// Proyecto chattering con uso de pipes
// Compilación: "gcc manager.c -o manager"
// Ejecución: "./manager –n N -p pipeNom"
// Observaciones: el numero maximo de usuarios que pueden estar conectados es 100.
//
//  Archivo creado por Juan Diego Echeverry, Santiago Yañez y Nicolás Rincón
//  Fecha de inicio: 6/05/23
//  Fecha de finalización: 29/05/23
//*****************************************************************

//*****************************************************************
// LIBRERIAS INCLUIDAS
//*****************************************************************
#include "headerManager.h"

//*********************************************************************************************************
// PROGRAMA PRINCIPAL
//*********************************************************************************************************
int main(int argc, char *argv[]) { 
  int fd;
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
    if(read(fd,&mensajeGeneral,sizeof(mensaje)) == -1){
      perror("Error al leer desde el fd");
    };
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