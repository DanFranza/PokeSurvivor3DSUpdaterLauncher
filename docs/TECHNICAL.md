# PokéSurvivor — lanceur 3DS (1.2.2)

Petit lanceur indépendant du jeu, compilé en **3DSX et CIA**. Interface française,
police Pixelify, panneaux bleus du portage, Pikachu animé, commandes tactiles et croix.

## Installation sur la console

Console équipée de Luma3DS avec son chargeur homebrew `hb:ldr`.

- **3DSX** : copier `pokesurvivor-launcher.3dsx` dans
  `sd:/3ds/PokeSurvivorLauncher/`, puis l'ouvrir dans Homebrew Launcher.
  Le RomFS du lanceur est intégré au fichier : aucun dossier graphique à copier.
- **CIA** : installer `pokesurvivor-launcher.cia` avec votre installateur habituel.
  Pour lancer le jeu 3DSX, le titre Homebrew Launcher Loader doit également être
  installé sur SD (ou le titre homebrew personnalisé configuré dans Luma).
  Le lanceur vérifie sa présence avant de passer la main. Il ne convertit pas le jeu en CIA.
- Au format 3DSX, le jeu est lu à **`sd:/3ds/pokesurvivor3ds.3dsx`**.
  Y placer le jeu actuel permet de jouer immédiatement, sans connexion.
- La sauvegarde existante **`sd:/3ds/pokesurvivor3ds.sav`** n'est jamais ouverte,
  copiée, supprimée ni modifiée par le lanceur.

Au premier démarrage, le lanceur crée :

`sd:/3ds/PokeSurvivorLauncher/config.ini`

```ini
manifest_url=https://api.github.com/repos/DanFranza/pokesurvivor3DS/contents/3ds/distribution/pokesurvivor3ds.3dsx?ref=port-3ds-danfranza
```

Le jeu est téléchargé directement depuis **DanFranza/pokesurvivor3DS**, branche
**port-3ds-danfranza**, fichier **3ds/distribution/pokesurvivor3ds.3dsx**.
Aucune release ni manifeste supplémentaire n'est nécessaire. L'ancienne adresse
`PokeSurvivor3DSUpdaterLauncher/releases/latest/...` de la version 1.0.0 est
corrigée automatiquement lors de la première recherche. Une adresse personnalisée
est conservée. La clé `manifest_url` garde son nom pour compatibilité ; elle peut
pointer vers les métadonnées GitHub ou vers un manifeste INI.

## Les trois actions

- **Jouer** : hors ligne, ouvre le titre CIA installé ou transmet le chemin du 3DSX au chargeur Luma, puis ferme
  le lanceur. Aucun RomFS du jeu n'est ouvert pendant une mise à jour.
- **Mettre à jour** (ou **Installer le jeu** s'il est absent) : télécharge le
  manifeste, demande confirmation de la version et de la taille, contrôle l'espace
  libre, télécharge par blocs de 32 Kio, vérifie la taille, l’empreinte Git du fichier et son en-tête 3DSX,
  ou l’identité CIA, puis installe. B annule le téléchargement et la copie de restauration.
  Les échanges de fichiers à la fin ne sont pas annulables.
- **Restaurer la version précédente** : confirmation, copie et vérification du
  secours, puis échange des deux versions. La version quittée devient le secours,
  ce qui permet de revenir à la version récente. La sauvegarde actuelle est conservée ;
  le comportement des données nouvelles dans un ancien jeu relève du jeu lui-même.

La recherche manuelle est complétée par un contrôle automatique en arrière-plan (voir ci-dessous). Les boutons Jouer/Restaurer
restent grisés si le fichier correspondant est absent. Une version déjà présente
est vérifiée par empreinte ; si son fichier est altéré, elle peut être réinstallée.

Une installation manuelle préexistante s'affiche avec une version « inconnue ».
Il est possible d'écrire son tag exact, sans saut de ligne, dans
`sd:/3ds/pokesurvivor3ds.3dsx.version`. Ce fichier est ensuite géré par le lanceur.
Le manifeste désigne la version à installer ; aucun tri sémantique des tags n'est fait.

## Publier une nouvelle version du jeu

Compiler le jeu avec son RomFS intégré, puis remplacer le fichier
`3ds/distribution/pokesurvivor3ds.3dsx` sur la branche `port-3ds-danfranza` du dépôt
`DanFranza/pokesurvivor3DS`. Le lanceur consulte l'API Contents de GitHub pour
obtenir la taille, le lien de téléchargement et l'identifiant de l'objet Git.
Une modification du binaire est automatiquement détectée, sans mettre à jour
le lanceur. La version affichée est `3ds-` suivi des douze premiers caractères
de l'empreinte du binaire.

L'identifiant Git complet est vérifié en calculant SHA-1 sur
`"blob " + taille + octet nul + contenu`, comme Git. SHA-1 sert ici à faire
correspondre les octets à l'objet fourni par GitHub via HTTPS, pas à vérifier une
signature d'auteur. Si le fichier change pendant le téléchargement, la vérification
échoue sans remplacer le jeu ; relancer la mise à jour récupère ses nouvelles
métadonnées. Les copies de secours restent contrôlées avec SHA-256.

GitHub limite les appels non authentifiés à son API (généralement 60 par heure
et par adresse IP). Le contrôle automatique conserve un cache de quinze minutes pour limiter les appels.

### Alternative : releases et manifeste SHA-256

Le format INI 1.0.0 reste utilisable en personnalisant `manifest_url`.
`tools/make_release.py` prépare localement les cinq champs `format=1`, `version`,
`url`, `size`, `sha256` à partir d'un vrai binaire et d'un lien HTTPS fixe :

```sh
python3 tools/make_release.py /chemin/pokesurvivor3ds.3dsx \
  --version v1.0.0 \
  --url https://github.com/DanFranza/pokesurvivor3DS/releases/download/v1.0.0/pokesurvivor3ds.3dsx
```

Cette alternative nécessite de publier le manifeste et le binaire. Le mode par
défaut utilise le fichier déjà disponible sur la branche du portage.

## Fichiers de récupération

Tous sont à côté du jeu dans `sd:/3ds/` :

| Suffixe après `pokesurvivor3ds.3dsx` | Usage |
| --- | --- |
| `.bak` | version de secours |
| `.version` / `.bak.version` | tags des deux versions |
| `.bak.sha256` | contrôle de la copie de secours |
| `.part` | transfert ou copie inachevé, supprimé au prochain démarrage |
| `.new` | nouveau jeu vérifié, prêt à installer |
| `.previous` | jeu déplacé pendant la transaction |
| `.journal` | versions nécessaire à la récupération |

Le journal est écrit et synchronisé avant de déplacer le jeu. Si `.new` existe
encore, une installation interrompue est annulée et le jeu précédent est remis en
place. Si `.new` a déjà pris la place du jeu, la finalisation reprend : version
installée, empreinte du secours, puis suppression du journal.

Si les fichiers sont incohérents ou le journal illisible, les actions sont
bloquées et les fichiers sont conservés pour une récupération manuelle.
La récupération suppose que le système de fichiers FAT reste lisible ; elle ne
répare pas une carte SD endommagée. Ne pas retirer la SD pendant l'installation.
Prévoir environ la taille d'un jeu + 1 Mio **libre**, en plus du jeu et du secours
qui sont déjà présents. Pour un jeu de 46 Mo, les trois copies peuvent donc
occuper environ 138 Mo au pic de l'opération.

## Réseau et confiance

HTTPS obligatoire, certificats vérifiés avec le jeu de racines intégré. Aucun
contournement TLS. Redirections limitées à huit sauts, avec refus d'un retour en
HTTP. Délai de 15 secondes sur la réponse et les lectures ; les détails d'erreur
sont affichés. Taille maximale d'un jeu : 512 Mio ; manifeste : 8 Kio.

L’empreinte Git (ou SHA-256 en mode manifeste INI) vérifie la correspondance
du binaire aux métadonnées ; elle ne remplace pas une signature de publication. La confiance repose sur HTTPS et le dépôt configuré.
Le bundle de certificats devra être actualisé si les racines utilisées changent.

## Compiler

Prérequis : devkitPro (`3ds-dev`, libctru, citro2d, citro3d), et `makerom` pour CIA.
Les petits fichiers RomFS et la bannière CIA sont fournis dans les sources.

```sh
./tools/build.sh                     # 3DSX
MAKEROM=/chemin/makerom ./tools/build.sh --cia   # 3DSX + CIA
./tools/test.sh                      # tests sur ordinateur, C++17 et Python 3
```

`DEVKITPRO` et `DEVKITARM` peuvent être adaptés. `make` seul conserve le flux
habituel devkitPro. Le fichier `launcher.rsf` porte le Title ID
`000400000ED53000`, choisi pour ce prototype et modifiable avant publication.
La bannière peut être reconstruite avec bannertool :

```sh
bannertool makebanner -i assets/banner.png -a assets/silence.wav -o assets/banner.bnr
```

`tools/update_roots.py` permet de régénérer `romfs/roots.bin` à partir d'un
bundle PEM de racines de confiance à jour, puis de recompiler les deux formats.

## Validation effectuée et limites

- Compilation ARM des deux formats, sans avertissement dans les sources du lanceur.
- Exécution et contrôle visuel du 3DSX dans Azahar 2126.2.
- 322 vérifications de stockage et de SHA-256 : installation initiale, mises à
  jour, restaurations successives, sauvegarde inchangée, copie corrompue, annulation,
  états persistants de transactions interrompues, journal invalide.
- 52 vérifications réseau avec un service HTTP simulé : blocs, redirections,
  refus HTTP, boucle, erreur 404, dépassement de taille, téléchargement tronqué,
  annulation et délai dépassé. Trois tests Python pour le manifeste de publication.
- Tests exécutés avec détection des erreurs mémoire et des comportements indéfinis.

**À valider sur une vraie 3DS** : installation et ouverture du CIA, passage au jeu
par Luma depuis les deux formats, HTTPS GitHub avec ses vraies redirections,
transfert sur console du jeu complet et restauration. Les tests réseau simulés ne
valident pas la compatibilité TLS de la console avec les serveurs GitHub actuels.
Aucun essai de coupure physique de SD n’a été effectué. La fonction de mise à jour du lanceur lui-même n'est pas incluse.

## Correctif 1.0.1

- Source réelle du jeu trouvée sur la branche du portage ; aucune publication
  supplémentaire demandée au propriétaire du dépôt.
- Correction automatique de l'ancienne adresse de configuration.
- Lecture JSON des métadonnées GitHub, vérification de l'objet Git et détection
  des changements du binaire.
- Tests du lecteur JSON, de la migration et de SHA-1 ; vérification du binaire
  réellement téléchargé (48 334 904 octets) contre l'empreinte de GitHub.

- Tampons de lecture et de vérification alloués sur le tas ; pile du thread principal portée à 256 Kio pour éviter un débordement pendant la vérification d’un jeu complet.

- Test réel Azahar réussi : recherche en ligne, téléchargement des 48 334 904 octets depuis GitHub, vérification, installation et affichage « Mise à jour terminée ». Le fichier présent sur la SD virtuelle est identique au jeu publié ; aucun fichier de transaction ne reste.

## Langues du lanceur (1.1.0)

La préférence est indépendante de `config.ini` : `language.txt` contient un code
parmi `fr`, `en`, `es`, `it`, `pt`. Un fichier absent ou invalide ouvre le sélecteur
au démarrage. Une sélection confirmée est enregistrée avec écriture et synchronisation
avant renommage. Une erreur d'écriture est signalée dans la langue choisie.

Les clés françaises et leurs quatre traductions sont dans `translations/launcher.tsv`.
`tools/generate_translations.py` vérifie les cinq colonnes, les clés uniques et les
paramètres `{0}`, `{1}` ou printf. `source/translations.inc` est généré et versionné.
Les codes de version et les fichiers de transaction restent indépendants de la langue.
Les messages de stockage sont traduits au moment de leur affichage ; les messages
paramétrés et les erreurs réseau sont composés dans la langue active.

`tools/localize_smdh.py` localise les descriptions d'application utilisées par le
Homebrew Launcher et le menu HOME selon la langue de la console. Cette langue
système est distincte de la préférence choisie dans l'interface du lanceur.

## Contrôle automatique et CIA (1.2.0)

Un thread de priorité inférieure démarre après le premier affichage du menu.
Il consulte uniquement les métadonnées du format actif, sans relire le binaire.
Budget global de trois secondes, lectures interrogées toutes les 100 ms, annulation
coopérative avant une action manuelle. Les services HTTP ne sont jamais utilisés
simultanément par la recherche et le téléchargement. Cache par format et URL,
valable quinze minutes ; erreurs réseau silencieuses. Un changement d'identifiant
de contenu est signalé, sans comparaison sémantique des versions.

Le choix 3DSX/CIA précède la recherche manuelle. Le CIA utilise le fichier officiel
`3ds/distribution/pokesurvivor3ds.cia`. La taille, l'objet Git, le TMD et le programme
NCCH sont vérifiés : seul le titre `000400000ED53100` est accepté, distinct du lanceur.
L'installation utilise AM_StartCiaInstall, ou AM_StartCiaInstallOverwrite pour
actualiser un titre présent, puis AM_FinishCiaInstall. Aucun titre ni ticket n'est
supprimé préalablement. L'ouverture utilise aptSetChainloader sur SD.

Le paquet CIA et son secours restent sur SD. Le journal est écrit avant l'import
natif ; un import interrompu peut reprendre au prochain démarrage à partir du
paquet vérifié. La restauration réimporte le CIA précédent avant l'échange des
paquets. Une panne pendant une écriture système peut nécessiter cette reprise ;
le journal SD ne rend pas l'installation native atomique. La sauvegarde du jeu
reste externe et n'est jamais touchée.

Les tests hôte couvrent l'import initial et le remplacement, annulation, erreurs
d'écriture/finalisation, identité du titre, restauration et reprise du journal.
Ils ne remplacent pas une validation du service AM sur console réelle.

## Certificats sur console (1.2.1)

Le RomFS embarque huit racines de confiance choisies dans le bundle Mozilla
publié par curl, au lieu de charger tout le magasin de bureau (128 certificats).
Les familles Sectigo, USERTrust, ISRG et DigiCert couvrent GitHub et ses serveurs
de téléchargement. La vérification des certificats serveur reste activée.
Un hôte personnalisé signé par une autre autorité nécessite de régénérer le bundle.

```sh
python3 tools/update_roots.py cacert.pem --github
```

Sans `--github`, l'outil conserve son mode de conversion du bundle complet.
Une erreur affiche désormais le numéro du certificat refusé et le code système,
ou RomFS/DER pour un fichier absent ou invalide. Le chargeur refuse aussi un
ensemble vide ou tronqué. Les tests imposent un plafond simulé de huit racines.
Ce plafond de test n'est pas une limite matérielle documentée ; la réduction vise
à éviter une saturation possible du service système. Validation réelle attendue.


## Remplacement du transport HTTPS (1.2.2)

Les éditions précédentes utilisaient le service HTTP système de la 3DS.
Le correctif 1.2.1 a résolu le premier obstacle de chargement des certificats
mais un échec de connexion 0xD8A0A030 a ensuite été signalé sur console.

Le transport utilise désormais libcurl 8.23.0 et mbedTLS 3.6.7, compilés
statiquement. Les sockets sont fournis par soc:U ; ssl:C est uniquement utilisé
pour le générateur aléatoire cryptographique, et ses erreurs sont propagées.
La négociation et la validation TLS sont effectuées par mbedTLS.
Les huit racines publiques sont fournies en PEM dans le RomFS.
Le nom du serveur, la chaîne et les dates sont vérifiés. Aucun mode sans
vérification n’est proposé. TLS 1.2 est le minimum.

L’interface multi de curl laisse le programme actualiser l’écran et annuler
les transferts pendant les attentes. La limite de taille est appliquée avant
chaque écriture. Seul un statut 200 écrit le binaire ; redirections HTTPS
vérifiées à chaque saut, huit maximum. Chaque échec supprime le fichier partiel.
Les transferts manuels ont un délai de connexion et une limite d’inactivité
de quinze secondes. Le contrôle automatique garde son budget de trois secondes.
Le résolveur DNS de soc:U est synchrone : son délai système peut dépasser ce
budget. Il reste exécuté dans le thread de fond ; le menu continue de s’afficher.

Compiler les dépendances avec `tools/build_network.py --directory /chemin/externe`,
puis définir NETWORK_PREFIX=/chemin/externe/prefix pour le build du lanceur.
Les archives officielles sont contrôlées par SHA-256. Le script inclut les
adaptations sockets devkitPro et laisse les autres composants du SDK en place.

Les tests réseau hôte utilisent un vrai serveur HTTPS local : certificat valide,
mauvais nom, certificat absent, redirections relatives et absolues, retour HTTP
refusé, boucle, 404, fichier trop grand, tronqué, annulation et budget dépassé.
Ils utilisent le libcurl de l’hôte et ne remplacent pas une validation de soc:U
et de mbedTLS sur une vraie console. Les tests de stockage, cache, langues et
import CIA restent exécutés.


Validation 1.2.2 dans Azahar : connexion GitHub via soc:U et mbedTLS,
métadonnées du CIA récupérées, téléchargement réel du 3DSX de 48 335 352 octets
et vérification de l’objet Git réussis. La validation sur matériel reste nécessaire.
Les détails du dernier échec curl sont conservés localement dans
`sd:/3ds/PokeSurvivorLauncher/network-error.txt` (code curl, erreur socket
et diagnostic de bibliothèque).


## Suppression du secours (1.2.3)

L’action principale « Version précédente » ouvre un menu Restaurer/Supprimer.
La suppression affiche la version et la taille puis exige une confirmation.
Elle agit uniquement sur les chemins exacts .bak, .bak.version et .bak.sha256
du format actif. Elle n’appelle pas le service AM et ne désinstalle aucun titre.
Le jeu courant, ses métadonnées, l’autre format et la sauvegarde restent intacts.

La suppression est refusée si un journal ou .previous indique une transaction
à récupérer. Les répertoires ne sont jamais supprimés. En cas d’échec du retrait
du binaire, ses métadonnées restent conservées ; si le nettoyage des métadonnées
échoue après retrait du binaire, le menu permet de réessayer. Les métadonnées
orphelines sont détectées. La création automatique d’un secours à la prochaine
mise à jour reste inchangée.

Les tests couvrent CIA et 3DSX, conservation des autres fichiers, suppression
répétée, recréation au prochain remplacement, secours corrompu, journal présent,
.previous présent, métadonnées orphelines et erreurs de nettoyage.
