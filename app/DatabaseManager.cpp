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
                    "FOREIGN KEY(agent_id) REFERENCES Agents(id))")) {
        qCritical() << "Erreur lors de la création de la table AutorisationSortie:" << query.lastError().text();
        success = false;
    }

    // Table Presences
    if (!query.exec("CREATE TABLE IF NOT EXISTS Presences ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "agent_id INTEGER, "
                    "date TEXT, "
                    "heure_arrivee TEXT, "
                    "heure_depart TEXT, "
                    "FOREIGN KEY(agent_id) REFERENCES Agents(id))")) {
        qCritical() << "Erreur lors de la création de la table Presences:" << query.lastError().text();
        success = false;
    }

    // Table Organisations (Secteurs Sociaux)
    if (!query.exec("CREATE TABLE IF NOT EXISTS Organisations ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "denomination TEXT, "
                    "sigle TEXT, "
                    "nature_juridique TEXT, "
                    "date_creation TEXT, "
                    "adresse TEXT, "
                    "telephone TEXT, "
                    "email TEXT)")) {
        qCritical() << "Erreur lors de la création de la table Organisations:" << query.lastError().text();
        success = false;
    }

    // Ajout de colonnes à Organisations si non existantes
    query.exec("ALTER TABLE Organisations ADD COLUMN representant TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN domaines TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN zones TEXT");
    query.exec("ALTER TABLE Organisations ADD COLUMN decision TEXT");

    // Table Dirigeants
    if (!query.exec("CREATE TABLE IF NOT EXISTS Dirigeants ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "organisation_id INTEGER, "
                    "nom TEXT, "
                    "fonction TEXT, "
                    "telephone TEXT, "
                    "FOREIGN KEY(organisation_id) REFERENCES Organisations(id))")) {
        qCritical() << "Erreur Dirigeants:" << query.lastError().text();
        success = false;
    }

    // Table Projets
    if (!query.exec("CREATE TABLE IF NOT EXISTS Projets ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "organisation_id INTEGER, "
                    "nom TEXT, "
                    "bailleur TEXT, "
                    "budget TEXT, "
                    "FOREIGN KEY(organisation_id) REFERENCES Organisations(id))")) {
        qCritical() << "Erreur Projets:" << query.lastError().text();
        success = false;
    }

    // Table MissionsVerification
    if (!query.exec("CREATE TABLE IF NOT EXISTS MissionsVerification ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "organisation_id INTEGER UNIQUE, "
                    "date_mission TEXT, "
                    "equipe TEXT, "
                    "gouvernance TEXT, "
                    "rh TEXT, "
                    "constats TEXT, "
                    "conclusion TEXT, "
                    "FOREIGN KEY(organisation_id) REFERENCES Organisations(id))")) {
        qCritical() << "Erreur MissionsVerification:" << query.lastError().text();
        success = false;
    }

    return success;
}
