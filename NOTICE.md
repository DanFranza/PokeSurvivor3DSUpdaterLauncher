# Crédits et provenance

**Port PokéSurvivor 3DS : Dan Franza.**
Lanceur et programme de mise à jour du portage de Dan Franza.
Jeu d’origine : PokéSurvivor, JPTER78.

Required Notice: Copyright 2026 Dan Franza (https://github.com/DanFranza)
Required Notice: Copyright 2026 JPTER78 (https://github.com/JPTER78/pokesurvivor)

- PokéSurvivor et les éléments graphiques repris du portage :
  Copyright 2026 JPTER78, https://github.com/JPTER78/pokesurvivor.
  Voir `licenses/PokeSurvivor-LICENSE.txt` et `licenses/PokeSurvivor-NONCOMMERCIAL.md`.
- Police Pixelify Sans, Pixelify Sans Project Authors, SIL OFL 1.1 ;
  police adaptée par le portage pour les symboles. Licence dans
  `licenses/Pixelify-OFL.txt`.
- Sprite Pikachu repris du portage, issu de PMDCollab/SpriteCollab :
  CHUNSOFT et SilverDeoxys563, tels qu'attribués dans le CREDITS.md du jeu.
  Source : https://github.com/PMDCollab/SpriteCollab.
  Les contributions communautaires y sont proposées sous CC BY-NC 4.0.
  Les éléments officiels conservent les droits de leurs titulaires ; voir LICENSES.md.
- Icône HOME reprise de `assets/icons/icon-48.png` du jeu.
- Le lanceur est compilé avec devkitARM, libctru, citro2d et citro3d
  (devkitPro). Le protocole de démarrage est celui du chargeur `hb:ldr`
  de Luma3DS et de 3ds-hbmenu. Les sources de ces programmes n'ont pas
  été incorporées au lanceur.
- Les paramètres CIA ont été préparés en consultant le modèle RSF
  Universal-Updater (Universal-Team) ; les droits ont été restreints
  à ceux utiles au lanceur.
- Bannière fabriquée avec bannertool ; CIA fabriqué avec makerom.
  Ces outils ne sont pas inclus dans le paquet de sources.
- `romfs/roots.bin` contient les certificats racines DER du bundle système
  `/etc/ssl/cert.pem` disponible lors de la compilation du 3 novembre 2026.
  Ce sont des certificats publics, sans clé privée ; ils ne sont pas une
  attestation de compatibilité du service TLS de la console.

- Lecture JSON : picojson, Copyright Cybozu Labs, Inc. et Kazuho Oku, BSD 2 clauses. Le texte de licence est conservé en tête de `source/picojson.h` et dans `licenses/picojson-BSD-2-Clause.txt`.


## Réseau HTTPS (1.2.2)

libcurl 8.23.0 : Daniel Stenberg et contributeurs, licence curl.
mbedTLS 3.6.7 : Mbed TLS project, utilisé sous Apache 2.0.
Adaptations des sockets 3DS issues des ajustements devkitPro de curl.
Les textes complets figurent dans `licenses/`.
