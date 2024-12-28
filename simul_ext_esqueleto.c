#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main(){
	 char *comando[LONGITUD_COMANDO];
	 char *orden[LONGITUD_COMANDO];
	 char *argumento1[LONGITUD_COMANDO];
	 char *argumento2[LONGITUD_COMANDO];
	 
	 int i,j;
	 unsigned long int m;
     EXT_SIMPLE_SUPERBLOCK ext_superblock;
     EXT_BYTE_MAPS ext_bytemaps;
     EXT_BLQ_INODOS ext_blq_inodos;
     EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
     EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
     int entradadir;
     int grabardatos;
     FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     //...
     
     fent = fopen("particion.bin","r+b");
     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
     
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
     // Buce de tratamiento de comandos
     for (;;){
		 do {
		 printf (">> ");
		 fflush(stdin);
		 fgets(comando, LONGITUD_COMANDO, stdin);
		 } while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
	     if (strcmp(orden,"dir")==0) {
            Directorio(&directorio,&ext_blq_inodos);
            continue;
            }
            else if (strcmp(orden, "bytemaps") == 0)
            {
                Printbytemaps(&ext_bytemaps);
                continue;
            }
            else if (strcmp(orden, "info") == 0)
            {
                LeeSuperBloque(&ext_superblock);
                continue;
            }
            if (strcmp(orden, "dir") == 0)
            {                                            // Si un strcmp da 0 es porque coinciden los campos
                Directorio(directorio, &ext_blq_inodos); // antes &directorio
                continue;
            }
             else if (strcmp(orden, "imprimir") == 0)
            {
                if (argumento1[0] != '\0')
                {
                    Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
                    continue;
                }
                else
                {
                    printf("Error: Uso incorrecto del comando 'imprimir'. Debe proporcionar el nombre del archivo a imprimir.\n");
                    continue;
                }
            }
            else if (strcmp(orden, "remove") == 0)
            {
                if (argumento1[0] != '\0')
                {
                    int resultado = Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);

                    if (resultado == 0)
                    {
                        printf("Archivo '%s' eliminado correctamente.\n", argumento1);
                        grabardatos = 1; // Marcar para grabar datos después de la eliminación
                    }
                    else
                    {
                        printf("ERROR: No se pudo eliminar el archivo '%s'.\n", argumento1);
                    }
                    continue;
                }
                else
                {
                    printf("Error: Uso incorrecto del comando 'remove'. Debe proporcionar el nombre del archivo a eliminar.\n");
                    continue;
                }
            }
              else if (strcmp(orden, "rename") == 0)
            {
                if (argumento1[0] != '\0' && argumento2[0] != '\0')
                {
                    if (Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2) == 0)
                    {
                        grabardatos = 1;
                    }
                    continue;
                }
                else
                {
                    printf("Error: Uso incorrecto del comando 'rename'. Debe proporcionar el nombre antiguo y el nuevo.\n");
                    continue;
                }
                
            }
             else if (strcmp(orden, "copy") == 0)
            {
                if (argumento1[0] != '\0' && argumento2[0] != '\0')
                {
                    int resultado = Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent);

                    if (resultado == 0)
                    {
                        printf("Archivo '%s' copiado a '%s' exitosamente.\n", argumento1, argumento2);
                    }
                    else if (resultado == -1)
                    {
                        printf("ERROR: No se pudo copiar el archivo '%s' a '%s'.\n", argumento1, argumento2);
                    }
                    else
                    {
                        printf("ERROR: Problema al intentar copiar el archivo '%s' a '%s'.\n", argumento1, argumento2);
                    }

                    grabardatos = 1; // Marcar para grabar datos después de la copia
                    continue;
                }
                else
                {
                    printf("Error: Uso incorrecto del comando 'copy'. Debe proporcionar el nombre del archivo de origen y destino.\n");
                    continue;
                }
            }
            if (strcmp(orden,"salir")==0){

            return 0;
         }
           else
            {
                printf("ERROR: Comando ilegal [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n");
            }
             Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(&memdatos,fent);
         grabardatos = 0;
     }
}  

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps)
{
    printf("Inodos :");
    for (int i = 0; i < MAX_INODOS; i++)
    {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");

    printf("Bloques [0-%d]: ", MAX_INODOS);
    for (int i = 0; i < MAX_INODOS + 1; i++)
    {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2)
{
    // Inicializa las variables a cadenas vacías
    orden[0] = '\0';
    argumento1[0] = '\0';
    argumento2[0] = '\0';

    int numArgs = sscanf(strcomando, "%99s %99s %99s", orden, argumento1, argumento2);

    // Verifica si los numeros que solo requieren un argumento son correctos
    if ((strcmp(orden, "info") == 0 || strcmp(orden, "bytemaps") == 0 || strcmp(orden, "dir") == 0 || strcmp(orden, "leesuperbloque") == 0) && numArgs > 1)
    {
        printf("Error: El comando '%s' solo debe tener un argumento.\n", orden);
        return -1;
    }

    return 0;  
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup)
{
    printf("Bloque %d Bytes\n", SIZE_BLOQUE);
    printf("inodos particion = %d\n", psup->s_inodes_count);
    printf("inodos libres = %d\n", psup->s_free_inodes_count);
    printf("Bloques particion = %d\n", psup->s_blocks_count);
    printf("Bloques libres = %d\n", psup->s_free_blocks_count);
    printf("Primer bloque de datos = %d\n", psup->s_first_data_block);
}


void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos)
{

    for (int i = 1; i < MAX_FICHEROS; i++)
    {
        if (directorio[i].dir_inodo != NULL_INODO)
        {
            printf(" %s\t", directorio[i].dir_nfich);

            int numInodo = directorio[i].dir_inodo;
            if (numInodo >= 0 && numInodo < MAX_INODOS)
            {
                EXT_SIMPLE_INODE inodo = inodos->blq_inodos[numInodo];
                printf("tamanio: %u \t", inodo.size_fichero);
                printf("inodo: %d ", directorio[i].dir_inodo);
                printf("bloques:");

                for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++)
                {
                    if (inodo.i_nbloque[j] != NULL_BLOQUE)
                    {
                        printf(" %u", inodo.i_nbloque[j]);
                    }
                }

                printf("\n");
            }
        }
    }
}
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre)
{
    for (int i = 0; i < MAX_FICHEROS; i++)
    {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0)
        {
            return directorio[i].dir_inodo; // Devuelve el número de inodo si se encuentra el archivo
        }
    }

    return -1;
}

int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo)
{
    int i;
    for (i = 0; i < MAX_FICHEROS; ++i)
    {
        if (strcmp(directorio[i].dir_nfich, nombreantiguo) == 0)
        {
            if (BuscaFich(directorio, inodos, nombrenuevo) != -1)
            {
                printf("ERROR: El archivo %s ya existe.\n", nombrenuevo);
                return -1;
            }
            strcpy(directorio[i].dir_nfich, nombrenuevo);
            return 1;
        }
    }
    printf("ERROR: El archivo %s no se encontro.\n", nombreantiguo);
    return 0;
}
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre)
{
    int inodo = BuscaFich(directorio, inodos, nombre);
    if (inodo != -1)
    {
        printf("\n");
        int inodoIndex = inodo - 2; // El índice del inodo es el número del inodo menos 2 (reservados)
        for (int i = 0; i < MAX_NUMS_BLOQUE_INODO && inodos->blq_inodos[inodoIndex].i_nbloque[i] != NULL_BLOQUE; ++i)
        {
            int bloqueIndex = inodos->blq_inodos[inodoIndex].i_nbloque[i];
            printf("%s", memdatos[bloqueIndex].dato);
        }
        printf("\n");
        return 1; // Éxito al imprimir
    }
    else
    {
        printf("ERROR: El archivo %s no encontrado.\n", nombre);
        return 0; // Error al imprimir
    }
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich){
  if(BuscaFich(directorio, inodos, nombre) == -1){
    printf("ERROR: el fichero no existe\n");
    return -1;
  }
  for(int i = 0; i < MAX_INODOS; i++) {
    if(directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, nombre) == 0) {
      int inodo = directorio[i].dir_inodo;
      for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
        int bloque = inodos->blq_inodos[inodo].i_nbloque[j];
        if (bloque != NULL_BLOQUE) {
          ext_bytemaps->bmap_bloques[bloque] = 0;
          ext_superblock->s_free_blocks_count++; 
        }
      }
      ext_bytemaps->bmap_inodos[inodo] = 0;
      ext_superblock->s_free_inodes_count++; 

      memset(&directorio[i], 0, sizeof(EXT_ENTRADA_DIR));
      directorio[i].dir_inodo = NULL_INODO;
      return 0;
    }
  }
  return 1;
}
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich)
{
    int i;
    for (i = 0; i < MAX_FICHEROS; i++)
    {
        if (strcmp(directorio[i].dir_nfich, nombreorigen) == 0)
        {
            int inodeIndexSrc = directorio[i].dir_inodo;

            for (int j = 0; j < MAX_FICHEROS; j++)
            {
                if (strcmp(directorio[j].dir_nfich, nombredestino) == 0)
                {
                    printf("Error: El archivo '%s' ya existe.\n", nombredestino);
                    return -1; 
                }
            }

            int freeDirEntry = -1;
            for (int j = 0; j < MAX_FICHEROS; j++)
            {
                if (directorio[j].dir_inodo == NULL_INODO)
                {
                    freeDirEntry = j;
                    break;
                }
            }

            if (freeDirEntry == -1)
            {
                printf("Error: No hay entradas de directorio disponibles para '%s'.\n", nombredestino);
                return -1; 
            }

            int freeInode = -1;
            for (int j = 0; j < MAX_INODOS; j++)
            {
                if (ext_bytemaps->bmap_inodos[j] == 0)
                {
                    freeInode = j;
                    break;
                }
            }

            if (freeInode == -1)
            {
                printf("Error: No hay inodos disponibles para '%s'.\n", nombredestino);
                return -1; 
            }

            for (int k = 0; k < MAX_NUMS_BLOQUE_INODO; k++)
            {
                if (inodos->blq_inodos[inodeIndexSrc].i_nbloque[k] != NULL_BLOQUE)
                {
                    int freeBlock = -1;
                    for (int l = 0; l < MAX_BLOQUES_DATOS; l++)
                    {
                        if (ext_bytemaps->bmap_bloques[l] == 0)
                        {
                            freeBlock = l;
                            break;
                        }
                    }

                    if (freeBlock == -1)
                    {
                        printf("Error: No hay bloques de datos disponibles para '%s'.\n", nombredestino);
                        return -1; 
                    }

                    // Actualizar bytemaps para el archivo destino
                    ext_bytemaps->bmap_inodos[freeInode] = 1;
                    ext_bytemaps->bmap_bloques[freeBlock] = 1;

                    strcpy(directorio[freeDirEntry].dir_nfich, nombredestino);
                    directorio[freeDirEntry].dir_inodo = freeInode;
                    inodos->blq_inodos[freeInode].size_fichero = inodos->blq_inodos[inodeIndexSrc].size_fichero;

                    for (int m = 0; m < MAX_NUMS_BLOQUE_INODO; m++)
                    {
                        inodos->blq_inodos[freeInode].i_nbloque[m] = NULL_BLOQUE;
                    }

                    memcpy(memdatos[freeBlock].dato, memdatos[inodos->blq_inodos[inodeIndexSrc].i_nbloque[k]].dato, SIZE_BLOQUE);
                    inodos->blq_inodos[freeInode].i_nbloque[k] = freeBlock;
                }
            }
            return 0; 
        }
    }
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich)
    {
        fseek(fich, SIZE_BLOQUE * 3, SEEK_SET);
        fwrite(directorio, sizeof(EXT_ENTRADA_DIR), MAX_FICHEROS, fich);
        fwrite(inodos, sizeof(EXT_BLQ_INODOS), 1, fich);
    }

    void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich)
    {
        fseek(fich, SIZE_BLOQUE, SEEK_SET);
        fwrite(ext_bytemaps, sizeof(EXT_BYTE_MAPS), 1, fich);
    }

    void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich)
    {
        fseek(fich, 0, SEEK_SET);
        fwrite(ext_superblock, sizeof(EXT_SIMPLE_SUPERBLOCK), 1, fich);
    }

    void GrabarDatos(EXT_DATOS *memdatos, FILE *fich)
    {
        fseek(fich, SIZE_BLOQUE * 4, SEEK_SET);
        fwrite(memdatos, sizeof(EXT_DATOS), MAX_BLOQUES_DATOS, fich);
}
