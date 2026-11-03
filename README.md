# PokéSurvivor 3DS — Dan Franza

**Launcher & Updater pour le port PokéSurvivor 3DS de Dan Franza.**

Une application indépendante pour jouer, télécharger les mises à jour et revenir
à la version précédente du portage. Disponible en **3DSX** et **CIA**, avec une
interface française inspirée du jeu : police pixel, panneaux bleus et Pikachu animé.

Le jeu d'origine est **[PokéSurvivor de JPTER78](https://github.com/JPTER78/pokesurvivor)**.
Le portage Nintendo 3DS est maintenu par **[Dan Franza](https://github.com/DanFranza/pokesurvivor3DS/tree/port-3ds-danfranza)**.
Ce dépôt contient uniquement le lanceur et son programme de mise à jour.

<img src="assets/launcher.png" alt="Lanceur du port PokéSurvivor 3DS de Dan Franza" width="400">

## Télécharger

➡ **[Dernière version : 3DSX, CIA et paquet complet](https://github.com/DanFranza/PokeSurvivor3DSUpdaterLauncher/releases/latest)**

- `pokesurvivor-launcher.3dsx` : ouverture depuis Homebrew Launcher.
- `pokesurvivor-launcher.cia` : icône installable dans le menu HOME.
- `PokeSurvivorLauncher-1.0.2.zip` : les deux formats, les sources et les licences.

## Installer

### Version 3DSX

1. Copier `pokesurvivor-launcher.3dsx` dans `sd:/3ds/PokeSurvivorLauncher/`.
2. Ouvrir le lanceur depuis Homebrew Launcher.
3. Choisir **Installer le jeu** et confirmer le téléchargement.

### Version CIA

1. Installer `pokesurvivor-launcher.cia` avec votre installateur habituel.
2. Ouvrir l'icône **PokéSurvivor 3DS — Dan Franza** dans le menu HOME.
3. Choisir **Installer le jeu**.

**Sur console, Luma3DS est requis.** Pour démarrer le jeu 3DSX depuis le CIA,
Homebrew Launcher Loader doit aussi être installé sur la SD, ou le titre homebrew
correspondant doit être configuré dans Luma. Le lanceur vérifie ce prérequis.
Le CIA est celui du lanceur : le jeu reste un fichier 3DSX sur la carte SD.

Le RomFS du lanceur est intégré : aucun dossier graphique supplémentaire à copier.

## Les trois options

| Option | Fonction |
| --- | --- |
| **Jouer** | Ouvre le jeu installé, sans exiger de connexion Internet. |
| **Mettre à jour** | Recherche le binaire publié, télécharge et vérifie le fichier, puis conserve l'ancienne version comme secours. |
| **Restaurer la version précédente** | Rétablit le secours et conserve la version quittée pour pouvoir revenir dessus. |

Si le jeu est absent, **Mettre à jour** devient **Installer le jeu**.
Navigation à la croix ou au tactile ; A valide et START quitte. Les confirmations
et l'annulation du téléchargement sont aussi accessibles au tactile.

Le jeu est installé à `sd:/3ds/pokesurvivor3ds.3dsx`.
**La sauvegarde `sd:/3ds/pokesurvivor3ds.sav` n'est jamais modifiée par le lanceur.**
Les versions du portage peuvent la relire dans les deux sens ; une ancienne version
n'affichera pas les nouveautés récentes.

## D'où vient le jeu ?

Le lanceur télécharge le fichier déjà publié dans le dépôt de Dan Franza :

- Dépôt : [`DanFranza/pokesurvivor3DS`](https://github.com/DanFranza/pokesurvivor3DS).
- Branche : `port-3ds-danfranza`.
- Fichier : [`3ds/distribution/pokesurvivor3ds.3dsx`](https://github.com/DanFranza/pokesurvivor3DS/blob/port-3ds-danfranza/3ds/distribution/pokesurvivor3ds.3dsx).

Aucune release du jeu ni manifeste supplémentaire n'est nécessaire. Remplacer ce
binaire dans la branche suffit pour que le lanceur détecte une nouvelle version.
La version affichée, `3ds-…`, identifie le contenu du fichier publié.

Les connexions utilisent HTTPS avec vérification des certificats. La taille et
l'identifiant de l'objet Git sont contrôlés avant l'installation ; les copies de
secours sont vérifiées avec SHA-256. Un téléchargement incomplet ou refusé laisse
le jeu actuel intact. Un journal permet de récupérer une installation interrompue,
sous réserve que la carte SD et son système de fichiers restent lisibles.

Pour un jeu de 46 Mo, prévoir au moins environ 47 Mo libres en plus du jeu et du
secours déjà présents. Ne pas retirer la SD pendant l'installation.

## Configuration et mise à niveau du lanceur

La configuration est créée automatiquement dans
`sd:/3ds/PokeSurvivorLauncher/config.ini`. Il n'est pas nécessaire de la modifier
pour utiliser le dépôt de Dan Franza.

L'ancienne adresse erronée de la version 1.0.0 est corrigée automatiquement lors
d'une recherche de mise à jour. Une adresse personnalisée reste conservée.

Pour actualiser le **lanceur lui-même**, remplacer son 3DSX ou réinstaller son CIA.
L'option **Mettre à jour** concerne le jeu, pas le lanceur.

## Compiler et tester

Prérequis : devkitPro avec `3ds-dev`, libctru, citro2d et citro3d ; `makerom` pour CIA.
Le RomFS et la bannière nécessaires sont fournis.

```sh
./tools/build.sh
MAKEROM=/chemin/makerom ./tools/build.sh --cia
./tools/test.sh
```

Les tests sur ordinateur nécessitent un compilateur C++17 et Python 3.
Voir [la documentation technique](docs/TECHNICAL.md) pour le journal de récupération,
le format de configuration, les outils et le mode de publication alternatif.

### État de validation

- Compilation des versions 3DSX et CIA.
- Interface contrôlée dans Azahar 2126.2.
- Téléchargement réel de 48 334 904 octets depuis GitHub, contrôle d'intégrité et
  installation complète réussis dans Azahar.
- Tests automatisés de stockage, restauration, interruptions, transfert HTTP,
  configuration, lecture JSON et empreintes.

**À valider sur une vraie console :** installation du CIA, passage au jeu via Luma
et fonctionnement réseau sur le matériel. Azahar permet de tester le téléchargement
et l'installation ; le chargeur Luma nécessaire au bouton **Jouer** n'y est pas garanti.

## Crédits et licences

Le code du projet est distribué sous **PolyForm Noncommercial 1.0.0**, en cohérence
avec les éléments repris du jeu d'origine. Les composants tiers conservent leurs
licences propres : police SIL OFL, picojson BSD 2 clauses, bibliothèques devkitPro
et éléments SpriteCollab. Aucun droit sur les marques Pokémon n'est revendiqué.

- [Licence du projet](LICENSE)
- [Détail des licences et périmètres](LICENSES.md)
- [Crédits et provenances](NOTICE.md)
- [Historique des versions](CHANGELOG.md)

Projet de fans indépendant, sans affiliation avec Nintendo, Game Freak, Creatures
ou The Pokémon Company.
