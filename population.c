#include <stdio.h>
#include <stdlib.h>

#include "population.h"
#include "individual.h"

struct Population_t
{
    Individual **tableau_individus;
    int length;
    int nbVal;
    int size;
    double (*fitness)(Individual *, void *);
    void *paramFitness;
    void (*init)(Individual *);
    void (*mutation)(Individual *, float);
    float pmutation;
    Individual* (*crossover)(Individual *, Individual *);
    int eliteSize;
    double *g;
    double *tableau_fitness;
};

int compareFunction( const void * ptr1, const void * ptr2);
int compareFunction( const void * ptr1, const void * ptr2)
{
    double a = * (double*) ptr1;
    double b = * (double*) ptr2;
    if(a > b)
        return -1;
    if(a < b)
        return 1;
    else
        return 0;
}

void precalculation(Population *pop);
void precalculation(Population *pop)
{
    pop->g[0] = 0;
    double numerateur = 0, denominateur = 0;
    for(int k = 1; k <= (pop->size); k++)
    {
        denominateur += pop->tableau_fitness[k - 1];
    }
    for(int i = 1; i <= (pop->size); i++)
    {
        numerateur += pop->tableau_fitness[i - 1];
        pop->g[i] = numerateur/denominateur;    
    }
}

//crée une population de size individus avec des génotypes de longueur
//length et de valeurs de gènes comprises entre 0 et nbVal-1 initialisés
//par la fonction init fournie en argument. Cette population sera amenée
//à évoluer ensuite en utilisant les opérateurs de mutations définis par
//les fonctions mutation et crossover fournies en arguments, avec une 
//probabilité de mutation pmutation et le paramètre eliteSize (on supposera
//qu’il sera toujours ≥ 1). paramFitness est un pointeur void permettant 
//de passer des paramètres à la fonction fitness.
Population *populationInit(int length, int nbVal, int size,
                           double (*fitness)(Individual *, void *),
			               void *paramFitness,
                           void (*init)(Individual *),
                           void (*mutation)(Individual *, float),
                           float pmutation,
                           Individual* (*crossover)(Individual *, Individual *),
                           int eliteSize)
{
    Population *new_population = malloc(sizeof(Population));
    if(!new_population)
        exit (-1);
    
    Individual **tableau =  malloc(size * sizeof(Individual*));
    if(!tableau)
        exit (-1);

    double *tableau_fitness = malloc(size * sizeof(double));
    if(!tableau_fitness)
        exit (-1);

    for(int i = 0; i < size; i++)
    {
        tableau[i] = individualCreate(length, nbVal);
        init(tableau[i]);
        tableau_fitness[i] = fitness(tableau[i], paramFitness);
    }
    new_population->tableau_individus = tableau;
    new_population->tableau_fitness = tableau_fitness;

    new_population->length = length;
    new_population->nbVal = nbVal;
    new_population->size = size;
    new_population->fitness = fitness;
    new_population->paramFitness = paramFitness;
    new_population->init = init;
    new_population->mutation = mutation;
    new_population->crossover = crossover;
    new_population->pmutation = pmutation;
    new_population->eliteSize = eliteSize; //>=1
    double *g = malloc(((new_population->size)+1) * sizeof(double));
    if(!g)
        exit (-1);
    new_population->g = g;
    precalculation(new_population);

    return new_population;
}

//renvoie le fitness maximum parmi tous les individus de la population.
double populationGetMaxFitness(Population *pop)
{
    double fitness_max = pop->tableau_fitness[0];
    for(int i = 1; i < pop->size; i++)
    {
        if(pop->tableau_fitness[i] > fitness_max)
            fitness_max = pop->tableau_fitness[i];
    }
    return fitness_max;
}

//renvoie le fitness minimum parmi tous les individus de la population.
double populationGetMinFitness(Population *pop)
{
    double fitness_min = pop->tableau_fitness[0];
    for(int i = 0; i < pop->size; i++)
    {
        if(pop->tableau_fitness[i] < fitness_min)
            fitness_min = pop->tableau_fitness[i];
    }
    return fitness_min;
}

//renvoie le fitness moyen parmi tous les individus de la population.
double populationGetAvgFitness(Population *pop)
{
    double fitness_moy = 0;
    for(int i = 0; i < pop->size; i++)
    {
        fitness_moy += pop->tableau_fitness[i];
    }
    fitness_moy = (double)fitness_moy/(double)(pop->size);
    return fitness_moy;
}

//renvoie le meilleur individu de la population.
Individual *populationGetBestIndividual(Population *pop)
{
    double meilleur_fitness = populationGetMaxFitness(pop);
    for(int i = 0; i < pop->size; i++)
    {
        if(meilleur_fitness == pop->tableau_fitness[i])
        {
            return pop->tableau_individus[i];
        }
    }
    fprintf(stderr, "Echec de la fonction populationGetBestIndividual");
    exit (-1);
}

//libère la mémoire prise par la population.
void populationFree(Population *pop)
{
    free(pop->g);
    for(int i = 0; i< pop->size; i++)
    {
        individualFree(pop->tableau_individus[i]);
    }
    free(pop->tableau_individus);
    free(pop->tableau_fitness);
    free(pop);
}

//Doit etre O(log N) !!!
//tire un individu au hasard dans la population
//selon les valeurs de fitness (voir section 1).
Individual *populationSelection(Population *pop)
{
    double r = ((double) rand())/ (double) RAND_MAX;
    while(r == 1)
    {
        r = ((double) rand())/ (double) RAND_MAX;
    }
    double *g = pop->g;
    
    int hi = pop->size;
    int lo = 1;
    int milieu;

    // g[i-1] ≤ r < g[i]
    while(lo <= hi)
    {
        // Si hi - lo est pair, le + 1 n'aura pas d'effet
        // Si hi - lo est impair, ca permet d'arrondir vers le haut la fraction
        milieu = lo + (hi - lo + 1) / 2;
        if(r >= g[milieu]){
            lo = milieu + 1;
        }
        else if(r < g[milieu  - 1]){
            hi = milieu - 1;
        }
        if(g[milieu - 1] <= r && r < g[milieu])
        {
            return pop->tableau_individus[milieu - 1];
        }
    }
    fprintf(stderr, "Echec de la fonction populationSelection");
    exit (-1);
}

//O(n log(n))
//effectue une itération d’évolution de la population selon l’algorithme
//de la section 1. Cette fonction doit appeler populationSelection
//pour la sélection des parents pour la recombinaison.
void populationEvolve(Population *pop)
{   
    double fitness_tries[pop->size][2];
    for(int i = 0; i < pop->size; i++)
    {
        fitness_tries[i][0] = pop->tableau_fitness[i];
        fitness_tries[i][1] = i;
    }
    qsort(fitness_tries, pop->size, 2*sizeof(double), compareFunction);
    //On a fait une compareFunction de tel sorte que le tri soit decroissant

    Individual **tableau = malloc((pop->size) * sizeof(Individual*));
    if(!tableau)
        exit (-1);

    for(int k = 0; k < pop->eliteSize; k++)
    {
        int position = (int) fitness_tries[k][1];
        tableau[k] = individualCopy(pop->tableau_individus[position]);

    }
    for(int i = pop->eliteSize; i < pop->size; i++)
    {
        Individual *parent1 = individualCopy(populationSelection(pop));
        Individual *parent2 = individualCopy(populationSelection(pop));

        tableau[i] = (pop->crossover)(parent1, parent2);

        individualFree(parent1);
        individualFree(parent2);
    }
    for(int t = 1; t < pop->size; t++)
    {
        pop->mutation(tableau[t], pop->pmutation);
    }
    for(int j = 0; j < pop->size; j++)
    {
        pop->tableau_individus[j] = individualCopy(tableau[j]);
    }
    for(int a = 0; a < pop->size; a++)
    {
        individualFree(tableau[a]);
    }
    free(tableau);

    for(int i = 0; i < pop->size; i++)
    {
        pop->tableau_fitness[i] = pop->fitness(pop->tableau_individus[i], pop->paramFitness);
    }
    precalculation(pop);   
}
