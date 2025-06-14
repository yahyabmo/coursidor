# Coursidor
Jeu de stratégie inspiré de Quoridor, avec un moteur serveur–client en C et des stratégies clientes chargées dynamiquement.


## Compilation
make
make install
Remarque: bibliothèque GSL nécessaire pour l'execution

## Execution
./install/server ./install/player1.so ./install/player2.so -m <TAILLE> -t <TYPE> -M <ROUNDS>
<TAILLE> : taille du graphe (ex: -m 5)
<TYPE> : T (triangulaire), C (cyclique) ou H (troué)
<ROUNDS> (optionnelle) : nombre maximum de tours
Si vous mettez pas -M -M <ROUNDS> ca va tourner jusqu'un joueur gagne la partie.
## Tests
make test




# Sujet de programmation impérative

La page du sujet :

<https://www.labri.fr/perso/renault/working/teaching/projets/2024-25-S6-C-Coursidor.php>
