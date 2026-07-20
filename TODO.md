# Plan de Développement Détaillé : Application Fonction Publique (Kwilu)

Cette TODO list détaille toutes les étapes techniques nécessaires pour finaliser l'application en C++ / Qt, en se basant sur la conception validée et les documents administratifs fournis.

---

## 🏗️ Phase 1 : Consolidation de l'Architecture de Base
- [ ] **Amélioration de la Base de Données (`DatabaseManager`)**
  - [ ] Ajouter les tables secondaires pour le module Secteurs Sociaux : `Dirigeants`, `ProjetsOrganisations`, `Partenaires`, `DomainesIntervention`.
  - [ ] Mettre en place la contrainte de clés étrangères (Foreign Keys) dans SQLite (`PRAGMA foreign_keys = ON;`).
- [ ] **Interface et Navigation**
  - [ ] Ajouter des icônes (`QIcon`) à côté des textes de la barre latérale.
  - [ ] Créer une page de connexion (Login) simple avec un système de rôles (Admin, Secrétaire, Agent RH).

---

## 🗄️ Phase 2 : Module Secrétariat (Priorité 1)
Ce module est le plus rapide à rendre fonctionnel grâce aux documents déjà fournis.

### A. Générateur d'Autorisation de Sortie
- [x] Créer une classe `AutorisationSortieDialog` (Fenêtre modale de formulaire).
- [x] **Création du formulaire de saisie :**
  - [x] Un `QComboBox` (liste déroulante) relié à la table `Agents` pour sélectionner l'agent (récupère automatiquement son Grade, Matricule, Fonction).
  - [x] Des `QLineEdit` pour "Destination" et "Motif".
  - [x] Des `QDateEdit` pour "Date de départ" et "Date de retour".
  - [x] Un `QComboBox` pour le "Mode de transport" (Véhicule, Moto, Avion, Bateau).
- [x] **Enregistrement en BDD :**
  - [x] Bouton "Générer et Sauvegarder" qui exécute la requête `INSERT` dans la table `AutorisationSortie`.
- [x] **Génération du document PDF :**
  - [x] Utiliser `QTextDocument` avec un template HTML reproduisant exactement le document Word (Logos de la RDC, en-tête de la Province, signatures).
  - [x] Remplacer les variables du template (ex: `{{NOM_AGENT}}`) par les données saisies.
  - [x] Utiliser `QPrinter` pour exporter en PDF ou lancer l'impression directe.

### B. Gestion des Présences
- [x] Ajouter une vue (Tab) dédiée dans la page Secrétariat.
- [x] Afficher un `QTableView` lié à la table `Presences`.
- [x] **Formulaire de pointage rapide :**
  - [x] Sélecteur de date (`QDateEdit`) défini sur la date du jour par défaut.
  - [x] Sélecteur d'agent.
  - [x] Bouton "Pointer Arrivée" (enregistre l'heure actuelle).
  - [x] Bouton "Pointer Départ" (met à jour la ligne d'arrivée avec l'heure de départ).

---

## 🌍 Phase 3 : Module Secteurs Sociaux (Priorité 2)
C'est le module le plus complexe car il contient le plus de champs.

### A. Fiche de Demande d'Identification (ONG/ASBL)
- [x] Créer une classe `IdentificationDialog`. Vu la taille du formulaire, utiliser un `QTabWidget` pour le diviser en étapes logiques :
  - [x] **Onglet 1 - Informations Générales :** Dénomination, Sigle, Nature Juridique, Date de création, Adresses, Infos du Représentant Légal.
  - [x] **Onglet 2 - Organe Dirigeant :** Un `QTableWidget` où l'utilisateur peut ajouter dynamiquement des lignes (Nom, Fonction, Téléphone).
  - [x] **Onglet 3 - Intervention :** Des `QGroupBox` contenant de nombreuses cases à cocher (`QCheckBox`) pour les "Domaines d'intervention" et les "Zones d'intervention" (Territoires).
  - [x] **Onglet 4 - Projets & Partenaires :** Tableaux dynamiques pour saisir les bailleurs, budgets, et projets en cours.
  - [x] **Onglet 5 - Annexes & Décision :** Cases à cocher pour les documents physiques fournis et décision finale du dossier.
- [x] **Logique Transactionnelle :** Lors du clic sur "Enregistrer", utiliser `QSqlDatabase::transaction()` pour s'assurer que toutes les tables (Organisation, Projets, Dirigeants) sont insérées correctement.

### B. Rapport de Mission de Vérification
- [x] Créer un bouton "Ajouter Rapport de Vérification" sur la ligne d'une organisation sélectionnée.
- [x] Créer le formulaire de grille d'évaluation (Boutons radio pour Très Satisfaisant, Satisfaisant, Moyen, Faible).
- [x] Générer le rapport PDF officiel ("Rapport certifié de mission") avec les noms de l'équipe de mission et les conclusions.

### C. Certification
- [x] Implémenter la logique métier : Si "Décision = Enregistrement accordé" ET "Rapport de mission = Validé", débloquer le bouton "Imprimer Certificat d'Enregistrement".
- [x] Implémenter la génération PDF du Certificat d'Enregistrement (Format paysage A4, avec armoiries et sceau de la province).

---

## 👥 Phase 4 : Module Ressources Humaines (Améliorations)
- [ ] Remplacer l'édition directe du `QTableView` par un formulaire complet `FicheAgentDialog` pour inclure toutes les informations (Ministère, Direction, Salaire, Date d'engagement, etc.).
- [ ] Mettre en place un système de recherche et de filtres en haut de la liste (rechercher par nom, filtrer par service).

---

## 📊 Phase 5 : Tableaux de Bord (Dashboard)
- [ ] Ajouter le module `Charts` (`QT += charts`) au fichier CMakeLists.txt.
- [ ] **Graphique 1 (Barres) :** Nombre d'autorisations de sortie délivrées par mois.
- [ ] **Graphique 2 (Secteurs / Pie Chart) :** Répartition des entités (ONG vs ASBL vs EUP).
- [ ] **Indicateurs textuels (KPIs) :** Nombre total d'agents, Taux de présence du jour (Présents / Total Agents).

---

## 🚀 Déploiement et Tests
- [ ] Mettre en place un installeur Windows/Linux avec `CQtDeployer` ou `windeployqt` pour empaqueter l'application et la base de données SQLite.
- [ ] Réaliser des tests d'impression sur une véritable imprimante pour valider les marges des PDF.
