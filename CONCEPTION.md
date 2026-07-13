# Conception de l'application Fonction Publique

## Objectif

L'application vise à structurer et gérer les activités d'une administration publique autour de cinq domaines principaux :

- Ressources humaines (RH)
- Secteurs sociaux
- Secteurs productifs
- Infrastructures
- Secrétariat

Elle permet de centraliser les données, suivre les indicateurs clés et produire des rapports fiables pour la prise de décision.

## Périmètre fonctionnel

### 1. Module RH

Fonctions principales :

- Gestion des dossiers agents
- Suivi des congés et absences
- Gestion des événements de carrière (décès, désertion)
- Inventaire des cartes biométriques
- Statistiques mensuelles et annuelles

Indicateurs clés :

- Nombre d'agents
- Nombre de dossiers physiques en ordre
- Nombre de détenteurs de carte biométrique
- Nombre de déserteurs sur 6 mois
- Congés pris par mois

### 2. Module Secteurs sociaux

Fonctions principales :

- Enregistrement des entités : ONG, ASBL, fondations, établissements d'utilité publique (EUP)
- Certification et suivi des dossiers
- Évaluation des performances et conformité
- Publication des rapports par entité

Indicateurs clés :

- Nombre d'entités certifiées
- Nombre d'enregistrements validés
- Score de performance des EUP

### 3. Module Secteurs productifs

Fonctions principales :

- Suivi des activités agricoles
- Suivi de l'élevage
- Suivi des industries clés (SNEL, REGIDESO)
- Collecte de données sectorielles et production de tableaux de bord

Sous-domaines agricoles :

- Cultures vivrières : manioc, arachides, riz, niébé, courge, haricots, igname
- Cultures pérennes : banane plantain, plantain doux, palmier à huile, caféier

Élevage :

- Petit bétail
- Gros bétail
- Volaille

Industries :

- SNEL : MWh, kWh, nombre de cabines, etc.
- REGIDESO : m³ distribués, forages, état de fonctionnement

### 4. Module Infrastructures

Fonctions principales :

- Suivi des projets d'infrastructure
- Gestion des catégories : énergie, routes, eau, bâtiments
- Enregistrement du coût et de la durée des chantiers
- Suivi du statut des projets

Indicateurs clés :

- Nombre de projets en cours
- Budget engagé
- Avancement par projet
- Durée prévue vs durée réelle

### 5. Module Secrétariat

Fonctions principales :

- Gestion du courrier entrant
- Suivi des stagiaires (identité, période, établissement)
- Évaluation administrative
- Classement des documents et archivage

Indicateurs clés :

- Nombre de courriers reçus
- Nombre de stagiaires suivis
- Résultats d'évaluation

## Architecture proposée

### Vue générale

- Interface utilisateur web responsive
- API REST ou GraphQL
- Base de données relationnelle
- Authentification et rôles utilisateurs

### Composants

- Frontend : pages de tableau de bord, formulaires, listes, détails
- Backend : services métier, validation, calculs d'indicateurs
- Base de données : schéma des entités et relations
- Reporting : export PDF/CSV et visualisations

## Modèle de données

### Entités principales

- Agent
- DossierAgent
- Congé
- EvenementRH (décès, désertion, mutation)
- CarteBiometrique
- EntiteSociale (ONG, ASBL, Fondation, EUP)
- ProjetInfrastructure
- ActiviteAgricole
- Elevage
- StatistiqueIndustrielle
- Courrier
- Stagiaire

### Relations clés

- Un `Agent` peut avoir plusieurs `Congé`
- Un `Agent` peut avoir une `CarteBiometrique`
- Une `EntiteSociale` peut avoir plusieurs certifications et rapports
- Un `ProjetInfrastructure` dépend d'un type de projet et d'un budget
- Un `Stagiaire` est lié à un service ou un département

## Interfaces utilisateur

### Menu principal

- Dashboard
- Ressources humaines
- Secteurs sociaux
- Secteurs productifs
- Infrastructures
- Secrétariat
- Rapports
- Administration

### Tableaux de bord

- Vue synthétique des indicateurs par domaine
- Graphiques : évolution des congés, projets, entités certifiées
- Widgets de suivi rapide

### Pages de gestion

- Liste des agents
- Détail d'un agent
- Gestion des congés
- Liste des entités sociales
- Fiches projets
- Journal du courrier
- Suivi des stagiaires

## Cas d'usage

### Exemple 1 : Suivi d'un agent

1. Recherche d'un agent par nom ou matricule
2. Consultation du dossier physique et des congés
3. Ajout d'un événement RH (décès, désertion, mutation)
4. Mise à jour du statut de la carte biométrique

### Exemple 2 : Enregistrement d'une entité sociale

1. Ajout d'une ONG/ASBL/EUP
2. Vérification de la conformité
3. Attribution d'un statut de certification
4. Suivi et mise à jour des performances

### Exemple 3 : Gestion d'un projet d'infrastructure

1. Création d'un projet avec coût et durée
2. Suivi de l'état d'avancement
3. Comparaison entre durée planifiée et réalisée
4. Génération d'un rapport pour le décideur