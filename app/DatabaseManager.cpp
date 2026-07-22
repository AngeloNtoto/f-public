#include "DatabaseManager.hpp"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool DatabaseManager::init()
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString dbPath = dataPath + "/fonction_publique.sqlite";

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qCritical() << "Erreur : Impossible d'ouvrir la base de données:" << db.lastError().text();
        return false;
    }

    QSqlQuery pragmaQuery;
    pragmaQuery.exec("PRAGMA foreign_keys = ON;");

    qDebug() << "Base de données connectée avec succès:" << dbPath;
    return createTables();
}

bool DatabaseManager::createTables()
{
    QSqlQuery query;
    bool success = true;

    // Table Agents
    if (!query.exec("CREATE TABLE IF NOT EXISTS Agents ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "matricule TEXT UNIQUE, "
                    "nom TEXT, "
                    "postnom TEXT, "
                    "grade TEXT, "
                    "fonction TEXT, "
                    "service TEXT)")) {
        qCritical() << "Erreur lors de la création de la table Agents:" << query.lastError().text();
        success = false;
    }

    // Ajout de colonnes à Agents si non existantes
    query.exec("ALTER TABLE Agents ADD COLUMN ministere TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN direction TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN date_engagement TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN salaire TEXT");
    
    // Nouvelles colonnes RH
    query.exec("ALTER TABLE Agents ADD COLUMN prenom TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN sexe TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN etat_civil TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN telephone TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN adresse TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN primes TEXT");

    // Complément des 31 champs RH selon CONCEPTION.md
    query.exec("ALTER TABLE Agents ADD COLUMN secretariat_general TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN division TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN bureau TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN date_naissance TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN domaine TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN ref_engagement TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN ref_nomination TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN ref_commissionnement TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN niveau_etude TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN filiere TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN position_admin TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN niveau_affectation TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN province TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN lieu_affectation TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN statut_recensement TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN email TEXT");

    // Extension pour Carte Biométrique, Dossier Physique et Désertion/Statut Carrière
    query.exec("ALTER TABLE Agents ADD COLUMN carte_biometrique TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN dossier_physique TEXT");
    query.exec("ALTER TABLE Agents ADD COLUMN statut_carriere TEXT");

    // Table Suivi des Congés et Absences
    if (!query.exec("CREATE TABLE IF NOT EXISTS Conges ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "agent_id INTEGER, "
                    "type_conge TEXT, "
                    "date_debut TEXT, "
                    "date_fin TEXT, "
                    "duree_jours INTEGER, "
                    "mois_annee TEXT, "
                    "motif TEXT, "
                    "FOREIGN KEY(agent_id) REFERENCES Agents(id) ON DELETE CASCADE)")) {
        qCritical() << "Erreur lors de la création de la table Conges:" << query.lastError().text();
        success = false;
    }

    // Table AutorisationSortie
    if (!query.exec("CREATE TABLE IF NOT EXISTS AutorisationSortie ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "agent_id INTEGER, "
                    "destination TEXT, "
                    "motif TEXT, "
                    "duree TEXT, "
                    "date_depart TEXT, "
                    "date_retour TEXT, "
                    "transport TEXT, "
                    "mentions TEXT, "
                    "FOREIGN KEY(agent_id) REFERENCES Agents(id))")) {
        qCritical() << "Erreur lors de la création de la table AutorisationSortie:" << query.lastError().text();
        success = false;
    }
    query.exec("ALTER TABLE AutorisationSortie ADD COLUMN mentions TEXT");

    // Table LettresExpediees (Registre N°1)
    if (!query.exec("CREATE TABLE IF NOT EXISTS LettresExpediees ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "numero TEXT, "
                    "date_jour TEXT, "
                    "num_lettre TEXT, "
                    "destinataire TEXT, "
                    "objet TEXT, "
                    "nbre_pages INTEGER DEFAULT 1, "
                    "signature TEXT)")) {
        qCritical() << "Erreur LettresExpediees:" << query.lastError().text();
        success = false;
    }
    // Ajout de la colonne numero pour les bases existantes
    query.exec("ALTER TABLE LettresExpediees ADD COLUMN numero TEXT");

    // Table LettresRecues (Registre N°2)
    if (!query.exec("CREATE TABLE IF NOT EXISTS LettresRecues ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "date_jour TEXT, "
                    "num_lettre TEXT, "
                    "date_lettre TEXT, "
                    "objet TEXT, "
                    "expediteur TEXT, "
                    "indice TEXT, "
                    "nbre_pages INTEGER DEFAULT 1, "
                    "signature TEXT)")) {
        qCritical() << "Erreur LettresRecues:" << query.lastError().text();
        success = false;
    }

    // Table Presences
    if (!query.exec("CREATE TABLE IF NOT EXISTS Presences ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "agent_id INTEGER, "
                    "date TEXT, "
                    "heure_arrivee TEXT, "
                    "heure_depart TEXT, "
                    "signature TEXT, "
                    "FOREIGN KEY(agent_id) REFERENCES Agents(id))")) {
        qCritical() << "Erreur lors de la création de la table Presences:" << query.lastError().text();
        success = false;
    }
    query.exec("ALTER TABLE Presences ADD COLUMN signature TEXT");

    // Table Organisations (Secteurs Sociaux)
    if (!query.exec("CREATE TABLE IF NOT EXISTS Organisations ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "num_enregistrement TEXT, "
                    "date_reception TEXT, "
                    "denomination TEXT, "
                    "sigle TEXT, "
                    "nature_juridique TEXT, "
                    "date_creation TEXT, "
                    "date_debut_province TEXT, "
                    "num_personnalite_juridique TEXT, "
                    "autorite_delivrance TEXT, "
                    "adresse_siege_social TEXT, "
                    "adresse_province TEXT, "
                    "telephone TEXT, "
                    "email TEXT, "
                    "site_web TEXT, "
                    "representant_nom TEXT, "
                    "representant_fonction TEXT, "
                    "representant_nationalite TEXT, "
                    "representant_phone TEXT, "
                    "representant_email TEXT, "
                    "representant_adresse TEXT, "
                    "domaines TEXT, "
                    "zones TEXT, "
                    "documents_annexes TEXT, "
                    "observations TEXT, "
                    "decision TEXT)")) {
        qCritical() << "Erreur lors de la création de la table Organisations:" << query.lastError().text();
        success = false;
    }

    // Adaptations de colonnes pour la compatibilité existante
    query.exec("ALTER TABLE Organisations ADD COLUMN num_enregistrement TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN date_reception TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN date_debut_province TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN num_personnalite_juridique TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN autorite_delivrance TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN adresse_siege_social TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN adresse_province TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN site_web TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN representant_nom TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN representant_fonction TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN representant_nationalite TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN representant_phone TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN representant_email TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN representant_adresse TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN documents_annexes TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN observations TEXT");

    // Table Dirigeants
    if (!query.exec("CREATE TABLE IF NOT EXISTS Dirigeants ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "organisation_id INTEGER, "
                    "num_ordre INTEGER, "
                    "nom TEXT, "
                    "fonction TEXT, "
                    "telephone TEXT, "
                    "email TEXT, "
                    "FOREIGN KEY(organisation_id) REFERENCES Organisations(id) ON DELETE CASCADE)")) {
        qCritical() << "Erreur Dirigeants:" << query.lastError().text();
        success = false;
    }
    query.exec("ALTER TABLE Dirigeants ADD COLUMN num_ordre INTEGER");
    query.exec("ALTER TABLE Dirigeants ADD COLUMN email TEXT");

    // Table Partenaires (Techniques & Financiers)
    if (!query.exec("CREATE TABLE IF NOT EXISTS Partenaires ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "organisation_id INTEGER, "
                    "num_ordre INTEGER, "
                    "nom_partenaire TEXT, "
                    "pays_org TEXT, "
                    "financement TEXT, "
                    "projet_realise TEXT, "
                    "periode TEXT, "
                    "FOREIGN KEY(organisation_id) REFERENCES Organisations(id) ON DELETE CASCADE)")) {
        qCritical() << "Erreur Partenaires:" << query.lastError().text();
        success = false;
    }

    // Table Effectifs (Catégories de personnel)
    if (!query.exec("CREATE TABLE IF NOT EXISTS Effectifs ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "organisation_id INTEGER, "
                    "num_ordre INTEGER, "
                    "categorie TEXT, "
                    "effectif INTEGER, "
                    "FOREIGN KEY(organisation_id) REFERENCES Organisations(id) ON DELETE CASCADE)")) {
        qCritical() << "Erreur Effectifs:" << query.lastError().text();
        success = false;
    }

    // Table Projets
    if (!query.exec("CREATE TABLE IF NOT EXISTS Projets ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "organisation_id INTEGER, "
                    "num_ordre INTEGER, "
                    "nom TEXT, "
                    "intitule TEXT, "
                    "localisation TEXT, "
                    "bailleur TEXT, "
                    "budget TEXT, "
                    "duree TEXT, "
                    "beneficiaire TEXT, "
                    "etat_avancement TEXT, "
                    "observations TEXT, "
                    "FOREIGN KEY(organisation_id) REFERENCES Organisations(id) ON DELETE CASCADE)")) {
        qCritical() << "Erreur Projets:" << query.lastError().text();
        success = false;
    }
    query.exec("ALTER TABLE Projets ADD COLUMN num_ordre INTEGER");
    query.exec("ALTER TABLE Projets ADD COLUMN intitule TEXT");
    query.exec("ALTER TABLE Projets ADD COLUMN localisation TEXT");
    query.exec("ALTER TABLE Projets ADD COLUMN duree TEXT");
    query.exec("ALTER TABLE Projets ADD COLUMN beneficiaire TEXT");
    query.exec("ALTER TABLE Projets ADD COLUMN etat_avancement TEXT");
    query.exec("ALTER TABLE Projets ADD COLUMN observations TEXT");

    // Table MissionsVerification
    if (!query.exec("CREATE TABLE IF NOT EXISTS MissionsVerification ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "organisation_id INTEGER UNIQUE, "
                    "date_mission TEXT, "
                    "equipe TEXT, "
                    "situation_admin TEXT, "
                    "gouvernance TEXT, "
                    "rh_materiel TEXT, "
                    "verification_projets TEXT, "
                    "niveau_execution TEXT, "
                    "impact_communautaire TEXT, "
                    "contraintes TEXT, "
                    "conclusion TEXT, "
                    "recommandations_structure TEXT, "
                    "recommandations_division TEXT, "
                    "FOREIGN KEY(organisation_id) REFERENCES Organisations(id) ON DELETE CASCADE)")) {
        qCritical() << "Erreur MissionsVerification:" << query.lastError().text();
        success = false;
    }
    query.exec("ALTER TABLE MissionsVerification ADD COLUMN situation_admin TEXT");
    query.exec("ALTER TABLE MissionsVerification ADD COLUMN rh_materiel TEXT");
    query.exec("ALTER TABLE MissionsVerification ADD COLUMN verification_projets TEXT");
    query.exec("ALTER TABLE MissionsVerification ADD COLUMN niveau_execution TEXT");
    query.exec("ALTER TABLE MissionsVerification ADD COLUMN impact_communautaire TEXT");
    query.exec("ALTER TABLE MissionsVerification ADD COLUMN contraintes TEXT");
    query.exec("ALTER TABLE MissionsVerification ADD COLUMN recommandations_structure TEXT");
    query.exec("ALTER TABLE MissionsVerification ADD COLUMN recommandations_division TEXT");

    // Table Users pour le système de login
    if (!query.exec("CREATE TABLE IF NOT EXISTS Users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT, role TEXT)")) {
        qCritical() << "Erreur Users:" << query.lastError().text();
        success = false;
    } else {
        // Insérer les utilisateurs par défaut s'ils n'existent pas
        query.exec("INSERT OR IGNORE INTO Users (username, password, role) VALUES ('admin', 'admin', 'Admin')");
        query.exec("INSERT OR IGNORE INTO Users (username, password, role) VALUES ('secretaire', '1234', 'Secrétaire')");
        query.exec("INSERT OR IGNORE INTO Users (username, password, role) VALUES ('agentrh', '1234', 'Agent RH')");
    }

    // Table Secteurs Productifs (Entreprises)
    if (!query.exec("CREATE TABLE IF NOT EXISTS Entreprises (id INTEGER PRIMARY KEY AUTOINCREMENT, nom TEXT, categorie TEXT, rccm TEXT, statut TEXT)")) {
        qCritical() << "Erreur Entreprises:" << query.lastError().text();
        success = false;
    }

    // Table Infrastructures
    if (!query.exec("CREATE TABLE IF NOT EXISTS ProjetsInfra (id INTEGER PRIMARY KEY AUTOINCREMENT, nom TEXT, type TEXT, localisation TEXT, budget TEXT, avancement INTEGER)")) {
        qCritical() << "Erreur ProjetsInfra:" << query.lastError().text();
        success = false;
    }

    // Table RefOptions pour les choix dynamiques (D)
    if (!query.exec("CREATE TABLE IF NOT EXISTS RefOptions ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "categorie TEXT NOT NULL, "
                    "valeur TEXT NOT NULL, "
                    "UNIQUE(categorie, valeur))")) {
        qCritical() << "Erreur RefOptions:" << query.lastError().text();
        success = false;
    } else {
        // Pré-remplissage des options par défaut pour les champs dynamiques (D)
        QStringList defaultMinisteres = {"Plan et Coordination de l'Aide au Développement", "Fonction Publique", "Finances", "Budget", "Éducation Nationale", "Santé Publique"};
        for (const QString &val : defaultMinisteres) {
            query.exec(QString("INSERT OR IGNORE INTO RefOptions (categorie, valeur) VALUES ('ministere', '%1')").arg(val));
        }

        QStringList defaultSecGen = {"Secrétariat Général au Plan", "Secrétariat Général à la Fonction Publique", "Secrétariat Général aux Finances"};
        for (const QString &val : defaultSecGen) {
            query.exec(QString("INSERT OR IGNORE INTO RefOptions (categorie, valeur) VALUES ('secretariat_general', '%1')").arg(val));
        }

        QStringList defaultDirections = {"Direction des Ressources Humaines", "Direction d'Études et Planification", "Direction Administrative et Financière", "Direction d'Informatique"};
        for (const QString &val : defaultDirections) {
            query.exec(QString("INSERT OR IGNORE INTO RefOptions (categorie, valeur) VALUES ('direction', '%1')").arg(val));
        }

        QStringList defaultDivisions = {"Division Provinciale du Plan", "Division des Personnels", "Division de la Gestion Budgétaire"};
        for (const QString &val : defaultDivisions) {
            query.exec(QString("INSERT OR IGNORE INTO RefOptions (categorie, valeur) VALUES ('division', '%1')").arg(val));
        }

        QStringList defaultBureaux = {"Bureau Paie et Carrière", "Bureau Courrier et Archives", "Bureau Informatique", "Bureau Suivi et Évaluation"};
        for (const QString &val : defaultBureaux) {
            query.exec(QString("INSERT OR IGNORE INTO RefOptions (categorie, valeur) VALUES ('bureau', '%1')").arg(val));
        }

        QStringList defaultFonctions = {"Chef de Division", "Chef de Bureau", "Attaché de Bureau 1ère classe", "Attaché de Bureau 2ème classe", "Agent d'Exécution", "Informaticien", "Secrétaire"};
        for (const QString &val : defaultFonctions) {
            query.exec(QString("INSERT OR IGNORE INTO RefOptions (categorie, valeur) VALUES ('fonction', '%1')").arg(val));
        }

        QStringList defaultNiveauxEtude = {"Doctorat", "Master / Licence", "Graduat", "Diplôme d'État", "Autre"};
        for (const QString &val : defaultNiveauxEtude) {
            query.exec(QString("INSERT OR IGNORE INTO RefOptions (categorie, valeur) VALUES ('niveau_etude', '%1')").arg(val));
        }

        QStringList defaultFilieres = {"Droit", "Sciences Économiques", "Informatique de Gestion", "Administration Publique", "Sociologie", "Gestion des Ressources Humaines"};
        for (const QString &val : defaultFilieres) {
            query.exec(QString("INSERT OR IGNORE INTO RefOptions (categorie, valeur) VALUES ('filiere', '%1')").arg(val));
        }

        QStringList defaultPositionsAdmin = {"En activité", "Détachement", "Mise à disposition", "Disponibilité", "Suspension"};
        for (const QString &val : defaultPositionsAdmin) {
            query.exec(QString("INSERT OR IGNORE INTO RefOptions (categorie, valeur) VALUES ('position_admin', '%1')").arg(val));
        }

        QStringList defaultLieuxAffectation = {"Bandundu", "Kikwit", "Bulungu", "Gungu", "Idiofa", "Masi-Manimba", "Bagata"};
        for (const QString &val : defaultLieuxAffectation) {
            query.exec(QString("INSERT OR IGNORE INTO RefOptions (categorie, valeur) VALUES ('lieu_affectation', '%1')").arg(val));
        }
    }

    return success;
}
