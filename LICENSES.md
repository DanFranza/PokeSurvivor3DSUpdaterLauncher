# Licences et composants tiers

Les textes complets sont inclus dans ce dépôt et dans le paquet de distribution.
Une licence de composant n'est pas remplacée par celle du lanceur.

| Élément | Crédit et licence | Texte fourni |
| --- | --- | --- |
| Code propre au lanceur et adaptations du portage | Copyright 2026 Dan Franza ; PolyForm Noncommercial 1.0.0 | [LICENSE](LICENSE) |
| Éléments de code et présentation issus de PokéSurvivor | Copyright 2026 JPTER78 ; mentions amont conservées, PolyForm Noncommercial 1.0.0 pour les versions concernées | [Licence d'origine](licenses/PokeSurvivor-LICENSE.txt), [texte PolyForm amont](licenses/PokeSurvivor-NONCOMMERCIAL.md) |
| Police Pixelify Sans adaptée par le portage (`romfs/pixel.bcfnt`) | Pixelify Sans Project Authors ; SIL Open Font License 1.1 | [OFL](licenses/Pixelify-OFL.txt) |
| `source/picojson.h` | Cybozu Labs, Inc. et Kazuho Oku ; BSD 2 clauses | [BSD 2 clauses](licenses/picojson-BSD-2-Clause.txt) |
| citro2d | fincs ; zlib | [Licence citro2d](licenses/citro2d-LICENSE.txt) |
| citro3d | fincs ; zlib | [Licence citro3d](licenses/citro3d-LICENSE.txt) |
| libctru | Contributeurs libctru ; voir le texte amont | [Licence libctru](licenses/libctru-LICENSE.txt) |
| Contributions communautaires SpriteCollab | Attribution-NonCommercial 4.0 International selon la politique du dépôt | [Texte CC BY-NC 4.0](licenses/SpriteCollab-CC-BY-NC-4.0.md) |

| libcurl 8.23.0 | Daniel Stenberg et contributeurs ; licence curl | [Licence curl](licenses/curl-LICENSE.txt) |
| mbedTLS 3.6.7 | Projet Mbed TLS ; Apache 2.0 (option retenue parmi les licences proposées) | [Licences mbedTLS](licenses/mbedTLS-LICENSE.txt) |

## Éléments graphiques

Le sprite de Pikachu est repris du portage, qui crédite **CHUNSOFT** et
**SilverDeoxys563** pour le numéro 25. Son origine est le dépôt
[PMDCollab/SpriteCollab](https://github.com/PMDCollab/SpriteCollab).
Le fichier est une conversion en texture 3DS et le lanceur en affiche une animation
existante. La police est une conversion BCFNT adaptée par le portage.

La politique CC BY-NC de SpriteCollab concerne les contributions que leurs auteurs
peuvent placer sous cette licence. Les sprites officiels, personnages et marques
conservent les droits de leurs titulaires ; ce projet ne les réattribue pas à
Dan Franza et ne prétend pas les placer sous une nouvelle licence.
L'icône est reprise du jeu ; la bannière et les panneaux du lanceur sont adaptés
pour cette distribution. Conserver les crédits et l'usage non commercial.

## Outils et certificats

Les exécutables de devkitARM, makerom et bannertool ne sont pas redistribués dans
les sources. Le lanceur est construit avec ces outils ; les bibliothèques liées
conservent les notices ci-dessus.

`romfs/roots.pem` contient uniquement des certificats racines publics, extraits du
bundle Mozilla publié par curl ; aucune clé privée
ni aucun identifiant GitHub n'est inclus. `tools/update_roots.py` permet de
reconstruire ce fichier à partir d'un bundle PEM de confiance.

Required Notice: Copyright 2026 Dan Franza (https://github.com/DanFranza)

Required Notice: Copyright 2026 JPTER78 (https://github.com/JPTER78/pokesurvivor)


Les adaptations socket de curl reprennent les ajustements documentés par devkitPro
dans [curl-8.4.0.patch](https://github.com/devkitPro/pacman-packages/blob/master/3ds/curl/curl-8.4.0.patch),
portés sur curl 8.23.0. Les sources officielles, versions et empreintes de compilation
sont fixées dans `tools/build_network.py`. L’adaptateur de temps et d’entropie
utilise les services libctru de la console.
