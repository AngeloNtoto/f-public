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

- Gestion des dossiers agents (Nom complet,)
- Suivi des congés et absences
- Gestion des événements de carrière (décès, désertion)
- Inventaire des cartes biométriques
- Statistiques mensuelles et annuelles
- Cadre organique
  Champs agents (formulaire) :
- Ministère D
- Secrétariat général D
- Direction D
- Division D
- Bureau D
- Nom M
- Postnom M
- Prénom M
- Sexe O(Féminin,masculin)
- État civil O(Célibataire,Marié(é),Divorcé)
- Date de naissance M(date)
- Matricule M
- Domaine C(Cadre Générale)
- Grade statutaire M
- Fonction D
- Date d'engagement M
- Réf. acte d'engagement M
- Réf. acte de nomination M
- Réf. acte de commissionnement M
- Niveau d'étude D
- Filière D
- Position administrative D
- Niveau d'affectation C(provincial)
- Province C (kwilu)
- Lieu d'affectation D
- Salaire perçu O(oui,Non)
- Prime perçue O(oui,Non)
- Statut recensement O(oui,non)
- Numéro téléphone M
- Adresse email M
- Adresse M
- Indicateurs clés :
- Nombre d'agents
- Nombre de dossiers physiques en ordre
- Nombre de détenteurs de carte biométrique
- Nombre de déserteurs sur 6 mois
- Congés pris par mois

### 2. Module Secteurs sociaux

Fonctions principales :

- Enregistrement des entités (ONG, ASBL, fondations, établissements d'utilité publique - EUP)

  - Fiche de demande d'identification :
    - Numéro d'enregistrement
    - Date de réception
    - Identification de l'organisation :
      - Dénomination officielle, sigle
      - Nature juridique
      - Date de création
      - Date de début des activités dans la province
      - Numéro de personnalité juridique (ou d'autorisation)
      - Autorité ayant délivré
      - Adresse du siège social
      - Adresse complète dans la province (ex. Kwilu)
      - Téléphone, E-mail, Site internet (si applicable)
    - Identification du représentant légal :
      - Noms, fonction, nationalité
      - Téléphone, adresse électronique, adresse physique
    - Composition de l'organe dirigeant : (numéro, nom et postnom, fonction, téléphone, email)
    - Domaine d'intervention (cocher) :
      - Agriculture et développement rural
      - Sécurité alimentaire
      - Élevage
      - Éducation / formation professionnelle
      - Eau, hygiène et assainissement
      - Santé, nutrition
      - Environnement et changement climatique
      - Développement communautaire
      - Genre et autonomisation de la femme
      - Promotion des droits humains
      - Entrepreneuriat et emploi
      - Assistance humanitaire
      - Autres
    - Zone d'intervention (cocher) :
      - Bandundu
      - Kikwit
      - Bagata
      - Bulungu
      - Gungu
      - Idiofa
      - Masi-Manimba
    - Partenaires techniques et financiers (table) :
      - Num. d'ordre, Nom du partenaire, Pays/organisation, Financement reçu, Projet réalisé, Période
    - Effectif de l'organisation (table) :
      - Num. d'ordre, Catégorie du personnel (permanent / temporaire / volontaire), Effectif
    - Principaux projets en cours (table) :
      - Num. d'ordre, Intitulé du projet, Localisation, Coût du projet, Source de financement, Période
    - Documents annexes (cocher) :
      - Lettre de demande, Statuts, Acte constitutif, Règlement intérieur, Personnalité juridique, Liste des membres dirigeants, Pièce d'identité, CV, Plan d'action, Rapport d'activité, Preuve de paiement, Autre
    - Déclarations :
      - Observations
      - Décision (dossier confirmé, dossier à compléter, enregistrement accordé, enregistrement refusé)
- Mission de vérification sur terrain

  - Rapport certifié de mission :
    - Équipe de mission (Noms et fonctions)
    - Situation administrative et juridique (Vérification des documents fournis)
    - Gouvernance et fonctionnement institutionnel (AG fonctionnelle, CA, Comité de gestion, Coordination Exécutive)
    - Ressources humaines et matérielles (Effectifs, bureaux fonctionnels, équipements disponibles)
    - Vérification des projets et activités (Secteurs d'intervention, réalisations observées, niveau d'exécution : Très satisfaisant, Satisfaisant, Moyen, Faible)
    - Impact communautaire (Confirmation par les bénéficiaires)
    - Contraintes et insuffisances observées
    - Conclusion de la mission (Existe et exerce, Existe avec insuffisances, Ne remplit pas les conditions)
    - Recommandations (À la structure, À la Division Provinciale)
- Suivi des projets

  - Fiche de suivi des projets :
    - Nom de l'Organisation
    - Titre du projet
    - Localisation
    - Bailleur
    - Budget
    - Durée
    - Bénéficiaire
    - État d'avancement
    - Observations
- Certification et suivi des dossiers
- Évaluation des performances et conformité
- Publication des rapports par entité

Indicateurs clés :

- Nombre d'entités certifiées
- Nombre d'enregistrements validés
- Score de performance des EUP
- Nombre de missions de vérification réalisées
- Nombre de projets suivis

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

- Gestion du courrier entrant (nature(rapport,lettres,factures,invitations,faire part,dossier))
- Autorisation de sortie :
  - N° de l'autorisation
  - Noms de l'agent
  - Grade
  - Matricule
  - Fonction
  - Service
  - Destination / Trajet (Se rendant à)
  - Motif de déplacement
  - Durée
  - Date de départ prévue
  - Date de retour probable
  - Mode de transport
  - Mentions spéciales éventuelles
- Gestion de présence :
  - Numéro
  - Nom
  - Post-nom
  - Matricule
  - Grade
  - Heure d'arrivée
  - Heure de départ
  - Jour / Date
  - Mois
  - Signature
- Suivi des stagiaires (identité, période, établissement)
- Évaluation administrative
- Classement des documents et archivage

Indicateurs clés :

- Nombre de courriers reçus
- Nombre d'autorisations de sortie délivrées
- Taux de présence
- Nombre de stagiaires suivis
- Résultats d'évaluation




Ajouter : Listes Des biens materiels et equipements de la dicvision provincial dual kwilu

N,Description,quantité,état,Observation
