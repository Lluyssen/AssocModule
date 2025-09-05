# AssocModule

Un petit module générique en **C++17** qui permet de gérer des éléments de types différents (`dds`) avec une logique d’association et de synchronisation réseau.

---

## Principe

- Chaque type `T` peut être **enregistré** dans le module avec une fonction de comparaison (`comparator`).
- Si un type `T` **n’est pas enregistré**, il est directement envoyé via `Network::send`.
- Le module maintient une **pool par type** pour stocker les objets déjà connus.
- Lorsqu’un nouvel objet est reçu (via `process`) :
  1. Si le type est **associé avec un comparateur** :
     - Si un élément identique existe déjà dans la pool → `sendUpdate`.
     - Sinon → ajout dans la pool et `sendCreate`.
  2. Si le type **n’est pas enregistré** → `send` direct.
- Une fonction `erase<T>` permet de supprimer un élément de la pool s’il correspond au comparateur.

---

Résumé du comportement

Avec comparateur enregistré :

Si déjà en pool → sendUpdate

Sinon → sendCreate + ajout en pool


Sans comparateur enregistré :

send direct (aucune pool)




---

Avantages

Générique : fonctionne avec n’importe quel type.

Séparé : la logique d’envoi est centralisée dans Network.

Flexible : tu choisis quels types doivent être suivis via un comparateur.

Léger : pas besoin de spécialiser ou de faire des if par type.



---
