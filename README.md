# projet1_INFO0952
INFO0952 - Complément d'informatique : algorithmes génétiques et problème du voyage de commerce

compile with:
make all

execute with:
file ntowns size pm elitesize niterations
où file est un fichier contenant des coordonnées de villes, ntowns est le nombre de villes
récupérées dans le fichier, size est la taille de la population, pm la probabilité de mutation et elitesize le nombre k des meilleurs individus conservés à chaque évolution, et
niterations le nombre d’évolution à effectuer. A chaque exécution un fichier tour.gif est
créé (qui écrase le précédent) reprenant le meilleur tour obtenu à l’issue de l’exécution de
l’algorithme génétique. Deux fichiers de villes vous sont fournis :
— xy-random.csv : contenant des coordonnées de 1000 villes placées

exemple utilisé : ./testga 3 xy-belgium.csv 200  500  0.03  100  10000
