#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>

#define RAND_MAX 3000

int *tableauDeNombreAleatoires;
int idTableauPartage;

key_t cleTableau;
int proj_id;

sem_t *sem;
sem_t *sem2;

int main(int argc, char *argv[]) {

    int nbProcessus = atoi(argv[1]);
    /*----------------------------------------*/
    /*      Création d'une clef unique.       */
    /*----------------------------------------*/

    //Création d'une clée unique avec l'iud du projet
    if ((getuid() & 0xff) != 0)
        proj_id =  getuid();
    else
        proj_id = -1;

    if((cleTableau=ftok(".", proj_id)) == -1)
        perror("ftok");

    printf("Clef générée : 0x%04x\n", cleTableau);

    /*----------------------------------------*/
    /*   Création de la mémoire partagée.     */
    /*----------------------------------------*/

    /* création ou lien avec une zone partagée */
    idTableauPartage = shmget(cleTableau, sizeof(int)*RAND_MAX, IPC_CREAT | IPC_EXCL | 0666);

    /*Vérifications*/
    if (idTableauPartage == -1) {
        if (errno == EEXIST){
            // Mémoire non crée car déjà existante donc ouverture de cette mémoire :
            idTableauPartage= shmget(cleTableau, sizeof(int)*RAND_MAX, IPC_CREAT | 0666);
            printf("Ouverture d'une mémoire partagée : %d\n", idTableauPartage);
        }
    }
    else
        printf("Création d'une mémoire partagée : %d\n", idTableauPartage);

    // Si impossible de créer la mémoire :
    // -1 = erreur
    if (idTableauPartage == -1) {
        printf("Erreur dans la création de la mémoire partagée.");
    }

    /* montage en mémoire */
    tableauDeNombreAleatoires = shmat(idTableauPartage, NULL, 0);

    //Vérifications d'autres erreurs :
    if(tableauDeNombreAleatoires == -1){
        printf("Erreur lors du partage de la mémoire !");
        switch (errno) {
            case EINVAL:
                printf("Identifiant invalide !\n");
                break;
            case EACCES:
                printf("Accès refusé !\n");
                break;
            case ENOMEM:
                printf("Mémoire saturée !");
                break;
            default:
                printf("Erreur inconnue");
                break;
        }
    }

    /*----------------------------------------*/
    /*      Traitement dans la mémoire        */
    /*----------------------------------------*/

    sem = sem_open("/semaphore1", O_CREAT, 0644, 1);

    srand(time(NULL));

    for (int i = 0; i < nbProcessus; i++) //Pour chaques processus
    {
        if (!fork())
        {
            sem_wait(sem);

            printf("Le fils rentre");

            for (int i = 0; i < 20000000 / nbProcessus; i++)
            {
                int rnd = rand() % RAND_MAX;
                tableauDeNombreAleatoires[rnd]++;
            }
            sleep(1);

            sem_post(sem);

            exit(1);
        }

    }

    for (int i = 0; i < nbProcessus; i++)
        wait(NULL);

    // Affichage
    /*for (int i = 0; i < RAND_MAX; i++)
        printf("\nVal %d : %d", i, tableauDeNombreAleatoires[i]);*/



    int valTotal = 0;
    for (int i = 0; i < RAND_MAX; i++)
    {
       valTotal += tableauDeNombreAleatoires[i];
    }
    printf("\n\nValeurs totales : %d ", valTotal);



    shmctl(idTableauPartage,IPC_RMID,NULL);

    sem_close(sem);
    sem_unlink("/semaphore1");

    printf("\n\nFin du traitement.\n");

    if (shmdt(tableauDeNombreAleatoires) == -1)
        printf("Erreur lors de la fermeture.\n");

    return 0;
}