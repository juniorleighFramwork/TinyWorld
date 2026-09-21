# TinyWorld

Projet personnel explorant si un modèle apprend les règles causales
d'un environnement, ou seulement les corrélations dans les données.

Philosophie : "Je ne sais pas. Construisons des expériences qui
permettent de réduire notre incertitude."

## Avancement

**V1.a — Chaîne causale minimale (terminée)**
Génère une chaîne causale A → B → C avec dépendances probabilistes,
exportée en CSV (10 000 observations). Vérification statistique :
P(B=1|A=1) ≈ 90%, conforme à la probabilité théorique programmée.

**V1.b — Facteur caché (terminée)**
Introduction d'une variable cachée Z, influençant à la fois A et C.
Crée une corrélation A↔C observable dans les données, alors qu'il
n'existe aucun lien causal direct entre A et C — uniquement une
cause commune. Vérifié statistiquement :
P(C=1|A=1) ≈ 68% vs P(C=1|A=0) ≈ 31%.

## Prochaines étapes

- Analyse des données en Python (pandas, statistiques)
- Premier modèle ML simple, testé pour voir s'il apprend le vrai
  mécanisme causal ou seulement la corrélation apparente
- Interventions (casser volontairement une corrélation, observer
  si le modèle reste robuste)

## Ce que ce n'est PAS

Ce projet n'est pas une contribution de recherche nouvelle — c'est
un laboratoire personnel d'apprentissage, inspiré de travaux établis
en causal representation learning (Judea Pearl, do-calculus, etc.).
