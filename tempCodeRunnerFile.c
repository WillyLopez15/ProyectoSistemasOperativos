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
             /*Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(&memdatos,fent);
         grabardatos = 0;*/
         //...
         // Escritura de metadatos en comandos rename, remove, copy     
         if (strcmp(orden,"salir")==0){

            return 0;
         }
           else
            {
                printf("ERROR: Comando ilegal [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n");
            }
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

    // Si no se pudo leer un comando, muestra un error
    if (numArgs == 0)
    {
        printf("Error: No se pudo leer el comando.\n");
        return -1;
    }

    // Verifica si los numeros que solo requieren un argumento son correctos
    if ((strcmp(orden, "info") == 0 || strcmp(orden, "bytemaps") == 0 || strcmp(orden, "directorio") == 0 || strcmp(orden, "leesuperbloque") == 0) && numArgs > 1)
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
