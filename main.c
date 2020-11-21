/**
 * \file equilibreMemPartagee.c
 * \author Mattias Eyherabide
 * \version 1.0
 * \date 21 Novembre 2020
 *
 * Créer un tableau en mémoire partagée (IPC) et
 * remplis chaques cases du nombre d'une valeure aléatoire et vérifie
 * si le tableau est équilibré.
 * Plusieurs processus feront le traitement. Le nombre de processus est
 * défini en argument.
 */

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
#define NB_TOTAL 20000000


int *tableauDeNombreAleatoires; /*!< Tableau crée en mémoire */
int idTableauPartage; /*!< Identifiant du tableau */

key_t cleTableau; /*!< Clé unique de la zone mémoire à partagée*/
int proj_id; /*!< Identifiant du projet afin de créer la clé */

sem_t *sem; /*!< Sémaphore gérant l'entrée des processus */

/*!
 * \brief Equilibre du tableau
 * Méthode permetant de vérifier si le tableau généré est équilibré.
 * Pour qu'il soit équilibré, la valeur entre la valeure minimale et maximale
 * doit être inférieure à 5%.
 */
void equilibre()
{
    /* Vérifie si le nombres d'éléments totaux est correct. */
    /* - - - - - - - - - - - - - - - - - - - - - - - - - -  */

    int valMin, valMax, valTotal = 0;
    double ecart;

    printf("\n\nVERIFICATIONS"
           "\n---------------");

    for (int i = 0; i < RAND_MAX; i++)
        valTotal += tableauDeNombreAleatoires[i];

    printf("\nNombre de valeurs totales : %d\nNombre de valeurs attendues : %d", valTotal, NB_TOTAL);

    if(valTotal == NB_TOTAL ) printf("\nOK"); else printf("\nErreur lors du remplissage du tableau.");


    /* Vérifie si le tableau est équibré    */
    /* - - - - - - - - - - - - - - - - - - -*/
    for(int i=0; i<RAND_MAX; i++)
    {
        if(tableauDeNombreAleatoires[i] > valMax) valMax = tableauDeNombreAleatoires[i];
        if(tableauDeNombreAleatoires[i] < valMin) valMin = tableauDeNombreAleatoires[i];

        if(i==0) valMin = valMax = tableauDeNombreAleatoires[i];
    }

    printf("\n\nValeur minimale : %d, Valeur maximale : %d\n", valMin, valMax);

    ecart = (((double)valMax-(double)valMin)/valMin)*100;

    printf("\nLe tableau est équilibré si la différence entre min et max est inférieure à 5%%.");
    printf("\nPourcentage : %f%%", ecart);
    if (ecart > 5) printf("\n\nTableau non équilibré");
    else printf("\n\nTableau équilibré");
}


int main(int argc, char *argv[]) {

    int nbProcessus = atoi(argv[1]); /*!< Nombres de processus faisant le traitement. */

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
            printf("Ouverture d'une mémoire partagée. Identifiant : %d\n\n", idTableauPartage);
        }
    }
    else
        printf("Création d'une mémoire partagée. Identifiant : %d\n\n", idTableauPartage);

    // Si impossible de créer la mémoire :
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
                printf("Mémoire saturée !\n");
                break;
            default:
                printf("Erreur inconnue.\n");
                break;
        }
    }

    /*----------------------------------------*/
    /*      Traitement dans la mémoire        */
    /*----------------------------------------*/

    sem = sem_open("/semaphore1", O_CREAT, 0644, 1);

    srand(time(NULL));

    for (int i = 0; i < nbProcessus; i++) //Pour chaques processus
        if (!fork())
        {
            sem_wait(sem);

            printf("Le fils rentre..\n");

            for (int i = 0; i < NB_TOTAL / nbProcessus; i++)
            {
                int rnd = rand() % RAND_MAX;
                tableauDeNombreAleatoires[rnd]++;
            }
            sleep(1);
            printf("Tableau remplis par un processus\n\n");

            sem_post(sem);

            exit(1);
        }

    for (int i = 0; i < nbProcessus; i++)
        wait(NULL);

    equilibre();


    shmctl(idTableauPartage,IPC_RMID,NULL);

    sem_close(sem);
    sem_unlink("/semaphore1");

    printf("\n\nFin du traitement.\n");

    if (shmdt(tableauDeNombreAleatoires) == -1)
        printf("Erreur lors de la fermeture.\n");

    return 0;
}