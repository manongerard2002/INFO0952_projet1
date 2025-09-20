# Algorithmes génétiques et problème du voyage de commerce

## Score

* Le projet a recu la note de *20/20*

## Execution

compile with: `make all`

execute with: `<file> ntowns size pm elitesize niterations` où
* `<file>` est un fichier contenant des coordonnées de villes,
* `ntowns` est le nombre de villes récupérées dans le fichier,
* `size` est la taille de la population,
* `pm` la probabilité de mutation
* `elitesize` le nombre k des meilleurs individus conservés à chaque évolution
* `niterations` le nombre d’évolution à effectuer.

A chaque exécution un fichier tour.gif est créé (qui écrase le précédent) reprenant le meilleur tour obtenu à l’issue de l’exécution de l’algorithme génétique. Deux fichiers de villes vous sont fournis :
— xy-random.csv : contenant des coordonnées de 1000 villes placées
— xy-belgium.csv : contenant les coordonnées de 589 villes belges.

exemple utilisé : ./testga 3 xy-belgium.csv 200  500  0.03  100  10000
