#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define RAND_MAX 30001

int *tableauDeNombreAleatoires;
key_t cleTableau = 1424;



int idTableauPartage;


int main(int argc, char *argv[]) {
    int nbProcessus = atoi(argv[1]);

    /* création ou lien avec une zone partagée */
    idTableauPartage = shmget(cleTableau, RAND_MAX, 0666 | IPC_CREAT);

    // Vérification du partage de la mémoire
    if (idTableauPartage == -1) {
        perror("shmget"); return (EXIT_FAILURE);
    }

    /* montage en mémoire */
    tableauDeNombreAleatoires = shmat(idTableauPartage, NULL, 0);

    /*test*/
    printf("zone[0] = %d\n", tableauDeNombreAleatoires[0]++ );



    for (int i=0; i<nbProcessus; i++) {

        pid_t pid;
        pid = fork();

        if (pid<0){
            printf("fork failed\n");
            return 0;
        }

        if (pid == 0) {

            for (int i = 0; i < RAND_MAX / nbProcessus; i++) {
                tableauDeNombreAleatoires[i] = rand();
                printf("\nVal %d : %d", i, tableauDeNombreAleatoires[i]);
            };

            return (EXIT_SUCCESS);

        } else {

            tableauDeNombreAleatoires[0] = 34;
            /*Affichage*/
            for (int i = 0; i < RAND_MAX / nbProcessus; i++) {
                printf("\nVal %d : %d", i, tableauDeNombreAleatoires[i]);
            } // Premiere valeur bien changée
        }
    }

    shmctl(idTableauPartage,IPC_RMID,NULL);


    return 0;
}
