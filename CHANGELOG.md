# Historique

## 1.2.3

- Menu « Version précédente » : choix de restaurer ou supprimer la copie de secours.
- Confirmation avec version, taille et avertissement sur la perte de restauration.
- Jeu installé et sauvegarde conservés ; secours CIA et 3DSX indépendants.
- Refus de suppression pendant une transaction, nettoyage des métadonnées et tests.
- Nouveaux messages traduits dans les cinq langues.

## 1.2.2

- Remplacement de HTTPC par libcurl 8.23.0 et mbedTLS 3.6.7 : la négociation TLS est intégrée au lanceur.
- Permissions CIA pour les sockets, source d’entropie cryptographique et contrôle d’initialisation.
- Vérification de la chaîne, du nom d’hôte et des dates conservée ; message pour vérifier l’horloge en cas de refus.
- Même transfert borné, contrôle d’intégrité, annulation, cache et mises à jour CIA/3DSX.
- Tests avec serveur HTTPS réel, certificats valides/invalides, redirections, limites et annulation.
- Scripts de compilation des dépendances et licences inclus.

## 1.2.1

- Ensemble HTTPS réduit de 128 à huit racines utiles à GitHub, pour limiter les ressources demandées au service de la console.
- Racines actualisées depuis le bundle Mozilla de curl ; vérification HTTPS toujours active.
- Erreurs de chargement détaillées : RomFS, données DER ou numéro du certificat et code système.
- Refus des ensembles vides ou tronqués et tests de régression sur les certificats intégrés.
- Correctif compilé et testé sur ordinateur ; confirmation sur console réelle attendue.

## 1.2.0

- Notification de changement du jeu en arrière-plan, budget de trois secondes et cache de quinze minutes.
- Choix 3DSX ou CIA ; téléchargement et installation native du CIA du jeu.
- Lancement du format installé, secours indépendants et restauration CIA avec reprise après interruption.
- Nouveaux messages traduits dans les cinq langues, tests réseau et installation étendus.


## 1.1.0 — 3 novembre 2026

- Menu de langue sur l’écran inférieur, affiché au premier démarrage et accessible au tactile, à la croix ou avec X.
- Français, anglais, espagnol, italien et portugais : 121 messages, confirmations, progression et erreurs traduits.
- Choix conservé dans `language.txt`, sans modifier la configuration GitHub ni la sauvegarde du jeu.
- Informations d’application SMDH traduites pour les cinq langues.
- Tests de couverture des traductions, paramètres de messages et conservation de la préférence ; contrôle visuel des cinq langues dans Azahar.

## 1.0.2 — 3 novembre 2026

- Mention explicite « Port PokéSurvivor 3DS de Dan Franza » à l'écran.
- Nom et auteur mis à jour dans les informations d'application 3DSX/CIA.
- Bannière du menu HOME aux couleurs du portage de Dan Franza.
- README de publication, documentation technique, licences et attributions séparées.

## 1.0.1 — 3 novembre 2026

- Téléchargement du binaire existant sur `DanFranza/pokesurvivor3DS`, branche
  `port-3ds-danfranza`, sans release intermédiaire.
- Correction automatique de l'ancienne adresse de téléchargement.
- Lecture des métadonnées GitHub et vérification de l'empreinte Git du binaire.
- Correction du dépassement de pile lors du contrôle d'un jeu complet.
- Téléchargement, vérification et installation complets validés dans Azahar.

## 1.0.0 — 3 novembre 2026

- Premier lanceur séparé en 3DSX/CIA : jouer, installer/mettre à jour, restaurer.
- Téléchargement temporaire, sauvegarde du précédent binaire et journal de récupération.
- La sauvegarde du jeu reste séparée et n'est pas modifiée par le lanceur.
