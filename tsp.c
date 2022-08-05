#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "gifenc.h"
#include "population.h"
#include "individual.h"
#include "tsp.h"


// Opaque structure of a map.
struct Map_t {
	int nbTowns;
	double *x;
	double *y;
};


static void terminate(const char *message);

static void getField(char *inputstring, char *fields[3]);

static void terminate(const char *message) {
	fprintf(stderr,"%s\n", message);
	exit(EXIT_FAILURE);
}

static void getField(char *inputstring, char *fields[3]) {
	fields[0] = strtok (inputstring, ",");
	for (int i =1; i < 3; i++) {
		fields[i] = strtok (NULL, ",");
	}
}

// charge un fichier csv, appelé filename, contenant les coordonnées 
//d’un ensemble de villes. Seules les nbTowns premières villes sont 
//chargées du fichier. Renvoie un pointeur vers une structure de type Map
//contenant les coordonnées des villes chargées du fichier.
Map *tspLoadMapFromFile(const char *filename, int nbTowns) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
		terminate("tspLoadMapFromFile: file can not be opened");

	char line[1024];

	Map *map = malloc(sizeof(Map));

	if (map == NULL)
	  terminate("tspLoadMapFromFile: map allocation failed");

	map->nbTowns = nbTowns;
	map->x = malloc(nbTowns * sizeof(double));
	map->y = malloc(nbTowns * sizeof(double));

	int i = 0;
	while (i< nbTowns && fgets(line, 1024, fp)) {
		char *fields[4];
		getField(line, fields);
		map->x[i] = strtod(fields[1], NULL);
		map->y[i] = strtod(fields[2], NULL);
		i++;
	}

	printf("Loaded %d towns from %s\n", nbTowns, filename);

	return map;
}

//libére la mémoire prise par la structure map.
void tspFreeMap(Map *map) {
	free(map->x);
	free(map->y);
	free(map);
}

//crée un fichier au format d’image gif appelé gifName contenant une
//représentation du tour donné en argument en se servant des coordonnées
//de la structure map. tour est un simple tableau d’entiers contenant les
//indices de villes dans l’ordre du tour et size est la largeur de l’image
//générée en nombre de pixels.
void tspTourToGIF(int *tour, Map *map, const char *filename, int size) {

  double minX = map->x[0];
  double maxX = minX;
  double minY = map->y[0];
  double maxY = minY;

  for (int pos = 0; pos < map->nbTowns; pos++) {
    
    double currentX = map->x[pos];
    double currentY = map->y[pos];
    
    if (currentX < minX)
      minX = currentX;
    else if (currentX > maxX)
      maxX = currentX;
    
    if (currentY < minY)
      minY = currentY;
    else if (currentY > maxY)
      maxY = currentY;
  }
  
  double xRange = maxX - minX;
  double yRange = maxY - minY;
  
  int sizex, sizey;
  if (xRange > yRange) {
    sizex = size;
    sizey = size*yRange/xRange;
  } else {
    sizex = size*xRange/yRange;
    sizey = size;
  }

  ge_GIF *gif = ge_new_gif(filename,
			   sizex+10, sizey+10,     /* w=1000, h=700 */
			   (uint8_t []) {          /* palette */
			     0xFF, 0xFF, 0xFF,   /* 0 -> white */
			       0xFF, 0x00, 0x00,   /* 1 -> red */
			       0x00, 0x00, 0xFF,   /* 2 -> blue */
			       0x00, 0x00, 0x00    /* 3 -> black */
			       },
			   2,                      /* palette depth == log2(# of colors) */
			   1                       /* Play it once */
			   );
  
  double prevX = (map->x[tour[map->nbTowns-1]] - minX)*sizex/xRange+5;
  double prevY = (map->y[tour[map->nbTowns-1]] - minY)*sizey/yRange+5;
  double nextX, nextY;

  // clean
  for (int j = 0; j < (gif->w * gif->h); j++)
    gif->frame[j] = 0;
  
  for (int pos = 0; pos < map->nbTowns; pos++) {
    nextX = (map->x[tour[pos]] - minX)*sizex/xRange+5;
    nextY = (map->y[tour[pos]] - minY)*sizey/yRange+5;
    ge_draw_line(gif, (int) prevX, (int) prevY, (int) nextX, (int) nextY, 2);
    prevX = nextX;
    prevY = nextY;
  }

  ge_add_frame(gif, 0);
  ge_close_gif(gif);  
}

//renvoie la longueur du tour donné en argument
//selon les coordonnées de villes dans la structure map.
double tspGetTourLength(int *tour, Map *map) {
	// Implementation
	double longueur = 0;
	int ville_1, ville_2;
	double x1, x2, y1, y2;

	for(int i = 0; i < map->nbTowns - 1; i++)
	{
		ville_1 = tour[i];
		ville_2 = tour[i + 1];
		x1 = map->x[ville_1];
		x2 = map->x[ville_2];
		y1 = map->y[ville_1];
		y2 = map->y[ville_2];
		longueur += sqrt( (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2) );
	}
	ville_1 = tour[map->nbTowns - 1];
	ville_2 = tour[0];
	x1 = map->x[ville_1];
	x2 = map->x[ville_2];
	y1 = map->y[ville_1];
	y2 = map->y[ville_2];
	longueur += sqrt( (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2) );

	return longueur;
}

//définir une fonction fitness positive à maximiser et de déterminer
//une manière d’encoder les solutions sous la forme de séquences d’entiers.
//Pour la fonction fitness, nous utiliserons simplement l’inverse de la
//longueur du chemin.
double fonction_fitness(Individual *ind, void *map);
double fonction_fitness(Individual *ind, void *map)
{
	int tour[individualGetLength(ind)];
	for(int i = 0; i< individualGetLength(ind); i++)
	{
		tour[i] = individualGetGene(ind, i);
	}
	double longueur;
	longueur = tspGetTourLength(tour, (Map *)map);

	return 1.0/longueur;
}

//renvoie le meilleur tour déterminé par algorithme génétique selon le 
//schéma expliqué dans la section 1 avec les opérateurs définis dans la 
//section 2. L’algorithme doit être exécuté pendant nbIterations (t), avec
//une population de taille sizePopulation (n), en conservant à chaque 
//évolution les eliteSize meilleurs individus (k) et avec une probabilité de 
//mutation pmutation (pm). Si le paramètre verbose est égal à 0, aucun 
//affichage ne doit être fait à l’écran. S’il est égal à 1, vous êtes libres 
//d’afficher ce que vous souhaitez à l’écran pour votre débogage (par exemple,
//l’évolution de la longueur minimale du tour au cours des itérations).
int *tspOptimizeByGA(Map *map, int nbIterations, int sizePopulation, int eliteSize, float pmutation, int verbose)
{
	Population *pop = populationInit(map->nbTowns, map->nbTowns, sizePopulation,
                        		   	fonction_fitness,
			            		   				map,
                        		   	individualRandomPermInit,
                        	   		individualPermMutation,
                        	   		pmutation,
                        	   		individualPermCrossOver,
                        		   	eliteSize);
	
	for(int t = 0; t < nbIterations; t++)
	{
		populationEvolve(pop);
		if(verbose != 0)
		{
			//fprintf(stderr, "debug");
		}
	}

	Individual *best_individu = individualCreate(map->nbTowns, map->nbTowns);
	best_individu = populationGetBestIndividual(pop);

	FILE *fp;
	fp = fopen("tour.txt", "w");
	individualPrint(fp, best_individu);
	fclose(fp);

	int *tour = malloc(map->nbTowns * sizeof(int));
	if(!tour)
		exit (-1);
	
	for(int i = 0; i < map->nbTowns; i++)
	{
		tour[i] = individualGetGene(best_individu, i);
	}

	populationFree(pop);

	return tour;
}
