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
//  Fecha de finalización: 29/05/23
//*****************************************************************

//*****************************************************************
// LIBRERIAS INCLUIDAS
//*****************************************************************
#include "headerTalker.h"

//*********************************************************************************************************
// PROGRAMA PRINCIPAL
//*********************************************************************************************************
int main(int argc, char *argv[]) {
  int continuar = 1;
  pid_t pid = getpid();
  char input[100];
  mensaje mensajeGeneral;

  if (!validar_args(argc, argv)) {
    return 0;
  }
  registrar(mensajeGeneral, pid);

  signal(SIG_DATA_AVAILABLE, data_available_handler);
  
  while(continuar){
    menu();
    printf("Elige una opcion: ");

    esperaInput = 1;
    if(fgets(input, sizeof(input), stdin)==NULL){
      perror("Error al leer desde stdin");
    }
    esperaInput = 0;

    memset(mensajeGeneral.opcion, 0, sizeof(mensajeGeneral.opcion));
    memset(mensajeGeneral.texto, 0, sizeof(mensajeGeneral.texto));
    memset(mensajeGeneral.idRecibe, 0, sizeof(mensajeGeneral.idRecibe));

    sscanf(input, "%s", mensajeGeneral.opcion);

    if(strcmp(mensajeGeneral.opcion, "List") == 0){
      sscanf(input, "%*s %19s", mensajeGeneral.texto);
    }
    else if(strcmp(mensajeGeneral.opcion, "Group") == 0){
      sscanf(input, "%*s %19s", mensajeGeneral.texto);
    }
    else if(strcmp(mensajeGeneral.opcion, "Sent") == 0){
      sscanf(input, "%*s \"%99[^\"]\"", mensajeGeneral.texto);
      if (*mensajeGeneral.texto != '\0') {
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
      else {
        strcpy(mensajeGeneral.texto, "");
      }
    }
    else if(strcmp(mensajeGeneral.opcion, "Salir") == 0){
      continuar = 0;
    }
    sprintf(mensajeGeneral.idEnvia, "%d", id_talker);
    fd = open(pipeGeneral, O_WRONLY);
    if(fd==-1){
      perror("Error al abrir el pipe");
      exit(1);
    }
    if(write(fd, &mensajeGeneral, sizeof(mensajeGeneral))==-1){
      perror("Error al escribir en el pipe");
    }
    close(fd); 

    usleep(500000);
  }
  return 0;
} 