#include <stdio.h>
#include <stdlib.h>

#include "individual.h"

struct Individual_t{
    int length;
    int nbVal;
    int *tab;
};


//crée un individu dont le génotype aura une longueur length et
//sera constitué d’entiers devaleurs comprises entre 0 et nbVal-1.
Individual *individualCreate(int length, int nbVal)
{
    Individual *new_individual = malloc(sizeof(Individual));
    if(!new_individual)
        exit (-1);
    
    new_individual->length = length;
    new_individual->nbVal = nbVal;

    int *tableau = malloc(length * sizeof(int));
    if(!tableau)
        exit (-1);
    
    new_individual->tab = tableau;

    return new_individual;
}

//libère la mémoire prise par l’individu.
void individualFree(Individual *ind)
{
    free(ind->tab);
    free(ind);
}

// renvoie la longueur du génotype de l’individu.
int individualGetLength(Individual *ind)
{
    return ind->length;
}

//renvoie la valeur du gène à la position i de l’individu.
int individualGetGene(Individual *ind, int i)
{
    return ind->tab[i];
}

// met la valeur du gène à la position i à val.
void individualSetGene(Individual *ind, int i, int val)
{
    ind->tab[i] = val;
}

//Initialise le génotype de l’individu à
//une valeur aléatoire (voir la section 1).
void individualRandomInit(Individual *ind)
{
    int aleatoire;
    for(int i=0; i<ind->length; i++)
    {
        aleatoire = rand() % (ind->nbVal);
        ind->tab[i] = aleatoire;
    }
}

//Initialise le génotype de l’individu à une permutation aléatoire des
//valeurs entre 0 et nbVal-1 (voir la section 2).
void individualRandomPermInit(Individual *ind)
{
    for(int i =0; i < ind->length; i++)
    {
        ind->tab[i] = i;
    }

    for(int k = 0; k < ind->length; k++)
    {
        int aleatoire_1 = rand() % ind->length;
        int temporaire = ind->tab[aleatoire_1];
        ind->tab[aleatoire_1] = ind->tab[k];
        ind->tab[k] = temporaire;
    }

}

//crée et renvoie un nouvel individu qui
//est une copie parfaite de l’individu ind.
Individual *individualCopy(Individual *ind)
{
    Individual *copie = individualCreate(ind->length, ind->nbVal);
    for(int i = 0; i < ind->length; i++)
        copie->tab[i] = ind->tab[i];
    return copie;
}

// affiche le génotype de l’individu dans le fichier fp (fonction fprintf).
//Le format attendu est une liste des gènes (entiers)
//séparées par des virgules, sans retour à la ligne.
void individualPrint(FILE *fp, Individual *ind)
{
    for(int i = 0; i < ind->length; i++)
        fprintf(fp, "%u,", ind->tab[i]);
}

//O(l)
//implémente l’opérateur de mutation décrit 
//dans la section 1 pour une séquence non contrainte.
void individualSeqMutation(Individual *ind, float pm)
{
    for(int i = 0; i < ind->length; i++)
    {
        double aleatoire = ((double) rand())/ (double) RAND_MAX;
        if(aleatoire < pm)
        {
            int nb_aleatoire = rand() % ind->nbVal;
            ind->tab[i]=nb_aleatoire;
        }
    }
}

//O(l)
//implémente l’opérateur de recombinaison décrit dans la section 1 pour une
//séquence non contrainte.
Individual* individualSeqCrossOver(Individual *parent1, Individual *parent2)
{
    if(parent1->length != parent2->length)
    {
        fprintf(stderr, "Parents de genotype de taille différente");
        exit (-1);
    }
    int length = parent1->length;
    int nbVal = 0;
    if(parent1->nbVal >= parent2->nbVal)
        nbVal = parent1->nbVal;
    else
        nbVal = parent2->nbVal;
         
    Individual *fils = individualCreate(length, nbVal);
    int p = rand()%(length+1);
    //Probleme qq part
    for(int i = 0; i < p; i++)
    {
        fils->tab[i] = parent1->tab[i];
    }
    for(int i = p; i < length; i++)
    {
        fils->tab[i] = parent2->tab[i];
    }
    return fils;
}

//O(l)
//implémente l’opérateur de mutation décrit
//dans la section 2 pour une permutation d’entiers.
void individualPermMutation(Individual *ind, float pm)
{
    float au_hasard = (float)rand() / (float)(RAND_MAX);
    if(au_hasard < pm)
    {
        int p1 = rand() % ind->length;
        int p2 = rand() % ind->length;
        if(p1 > p2)
        {
            int temporaire = p1;
            p1 = p2;
            p2 = temporaire;
        }
        // nb de cases = (p2 - p1 + 1)
        // quand p2 - p1 + 1 est impaire, ca arrondi bien vers le bas
        // et donc n'echange pas la case du milieu
        int nb_echanges = ((p2 - p1 + 1)/2);
        for(int i = 0; i < nb_echanges; i++)
        {
            int temporaire = ind->tab[p1 + i];
            ind->tab[p1 + i] = ind->tab[p2 - i];
            ind->tab[p2 - i] = temporaire;
        }
    }
}

//O(l)
//implémente l’opérateur de recombinaison décrit dans
//la section 2 pour une permutation d’entiers.
Individual* individualPermCrossOver(Individual *parent1, Individual *parent2)
{
    if(parent1->length != parent2->length)
    {
        fprintf(stderr, "Parents de genotype de taille différente");
        exit (-1);
    }
    int length = parent1->length;

    int p1 = rand() % length;
    int p2 = rand() % length;
    while(p1 == p2)
        p2 = rand() % length;
    if(p1 > p2)
    {
        int temporaire = p1;
        p1 = p2;
        p2 = temporaire;
    }

    Individual *fils = individualCopy(parent1);

    int map[length];
    int j = 0;
    for(int i = 0; i < length; i++)
    {
        j = fils->tab[i];
        map[j] = i;
    }
    int echange = 0;
    int k = 0;
    for(int a = p1; a <= p2; a++)
    {
        k = parent2->tab[a];
        echange = map[k];
        if(echange != a)
        {
            fils->tab[echange] = fils->tab[a];
            map[fils->tab[echange]] = echange;
            fils->tab[a] = k;
            map[fils->tab[a]] = a;
        }
    }
    return fils;
}

