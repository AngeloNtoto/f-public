#include "IdentificationDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

IdentificationDialog::IdentificationDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Demande d'Identification ONG / ASBL");
    resize(700, 500);
    setupUi();
}

IdentificationDialog::~IdentificationDialog() {}

void IdentificationDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QTabWidget *tabWidget = new QTabWidget(this);

    // --- Onglet 1 : Infos Générales ---
    QWidget *tab1 = new QWidget();
    QFormLayout *formLayout = new QFormLayout(tab1);
    
    nomEdit = new QLineEdit();
    sigleEdit = new QLineEdit();
    natureCombo = new QComboBox();
    natureCombo->addItems({"ONGD", "ASBL", "Fondation", "EUP"});
    dateCreationEdit = new QDateEdit(QDate::currentDate());
    dateCreationEdit->setCalendarPopup(true);
    adresseEdit = new QLineEdit();
    telephoneEdit = new QLineEdit();
    emailEdit = new QLineEdit();
    representantEdit = new QLineEdit();
    
    formLayout->addRow("Dénomination :", nomEdit);
    formLayout->addRow("Sigle :", sigleEdit);
    formLayout->addRow("Nature Juridique :", natureCombo);
    formLayout->addRow("Date de création :", dateCreationEdit);
    formLayout->addRow("Adresse Physique :", adresseEdit);
    formLayout->addRow("Téléphone :", telephoneEdit);
    formLayout->addRow("Email :", emailEdit);
    formLayout->addRow("Représentant Légal :", representantEdit);
    
    tabWidget->addTab(tab1, "1. Infos Générales");

    // --- Onglet 2 : Dirigeants ---
    QWidget *tab2 = new QWidget();
    QVBoxLayout *layout2 = new QVBoxLayout(tab2);
    QPushButton *btnAddDirigeant = new QPushButton("Ajouter un dirigeant");
    
    dirigeantsTable = new QTableWidget(0, 3);
    dirigeantsTable->setHorizontalHeaderLabels({"Nom complet", "Fonction", "Téléphone"});
    dirigeantsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    connect(btnAddDirigeant, &QPushButton::clicked, this, &IdentificationDialog::addDirigeantRow);
    addDirigeantRow(); // 1 ligne par défaut
    
    layout2->addWidget(btnAddDirigeant);
    layout2->addWidget(dirigeantsTable);
    tabWidget->addTab(tab2, "2. Dirigeants");

    // --- Onglet 3 : Intervention ---
    QWidget *tab3 = new QWidget();
    QVBoxLayout *layout3 = new QVBoxLayout(tab3);
    
    QGroupBox *groupDomaines = new QGroupBox("Domaines d'intervention");
    QGridLayout *gridDomaines = new QGridLayout(groupDomaines);
    QStringList listDomaines = {"Agriculture", "Santé", "Education", "Environnement", "Droits Humains", "Infrastructures"};
    for (int i = 0; i < listDomaines.size(); ++i) {
        QCheckBox *cb = new QCheckBox(listDomaines[i]);
        domainesChecks.append(cb);
        gridDomaines->addWidget(cb, i / 2, i % 2);
    }
    
    QGroupBox *groupZones = new QGroupBox("Zones d'intervention (Territoires)");
    QGridLayout *gridZones = new QGridLayout(groupZones);
    QStringList listZones = {"Bandundu", "Bulungu", "Gungu", "Idiofa", "Kikwit", "Masi-Manimba"};
    for (int i = 0; i < listZones.size(); ++i) {
        QCheckBox *cb = new QCheckBox(listZones[i]);
        zonesChecks.append(cb);
        gridZones->addWidget(cb, i / 2, i % 2);
    }
    
    layout3->addWidget(groupDomaines);
    layout3->addWidget(groupZones);
    layout3->addStretch();
    tabWidget->addTab(tab3, "3. Intervention");

    // --- Onglet 4 : Projets ---
    QWidget *tab4 = new QWidget();
    QVBoxLayout *layout4 = new QVBoxLayout(tab4);
    QPushButton *btnAddProjet = new QPushButton("Ajouter un projet");
    
    projetsTable = new QTableWidget(0, 3);
    projetsTable->setHorizontalHeaderLabels({"Nom du Projet", "Bailleur", "Budget ($)"});
    projetsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    connect(btnAddProjet, &QPushButton::clicked, this, &IdentificationDialog::addProjetRow);
    addProjetRow();
    
    layout4->addWidget(btnAddProjet);
    layout4->addWidget(projetsTable);
    tabWidget->addTab(tab4, "4. Projets");

    // --- Onglet 5 : Annexes & Décision ---
    QWidget *tab5 = new QWidget();
    QVBoxLayout *layout5 = new QVBoxLayout(tab5);
    
    QGroupBox *groupDocs = new QGroupBox("Documents physiques reçus");
    QVBoxLayout *docLayout = new QVBoxLayout(groupDocs);
    docStatutsCheck = new QCheckBox("Statuts notariés");
    docF3Check = new QCheckBox("F92 / F3 (Ministère de la Justice)");
    docLayout->addWidget(docStatutsCheck);
    docLayout->addWidget(docF3Check);
    
    QGroupBox *groupDecision = new QGroupBox("Décision Finale");
    QFormLayout *decLayout = new QFormLayout(groupDecision);
    decisionCombo = new QComboBox();
    decisionCombo->addItems({"En attente", "Enregistrement Accordé", "Rejeté"});
    decLayout->addRow("Statut du dossier :", decisionCombo);
    
    layout5->addWidget(groupDocs);
    layout5->addWidget(groupDecision);
    layout5->addStretch();
    tabWidget->addTab(tab5, "5. Décision");

    mainLayout->addWidget(tabWidget);

    // --- Boutons Actions ---
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnSave = new QPushButton("Enregistrer la demande", this);
    btnSave->setStyleSheet("padding: 8px; background-color: #27ae60; color: white; border-radius: 4px;");
    btnCancel = new QPushButton("Annuler", this);
    
    connect(btnSave, &QPushButton::clicked, this, &IdentificationDialog::saveToDatabase);
    connect(btnCancel, &QPushButton::clicked, this, &IdentificationDialog::reject);

    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);
}

void IdentificationDialog::addDirigeantRow()
{
    dirigeantsTable->insertRow(dirigeantsTable->rowCount());
}

void IdentificationDialog::addProjetRow()
{
    projetsTable->insertRow(projetsTable->rowCount());
}

void IdentificationDialog::saveToDatabase()
{
    if (nomEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "La dénomination est obligatoire.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    // 1. Sauvegarde de l'organisation
    QSqlQuery query;
    query.prepare("INSERT INTO Organisations (denomination, sigle, nature_juridique, date_creation, adresse, telephone, email, representant, domaines, zones, decision) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(nomEdit->text());
    query.addBindValue(sigleEdit->text());
    query.addBindValue(natureCombo->currentText());
    query.addBindValue(dateCreationEdit->date().toString("dd/MM/yyyy"));
    query.addBindValue(adresseEdit->text());
    query.addBindValue(telephoneEdit->text());
    query.addBindValue(emailEdit->text());
    query.addBindValue(representantEdit->text());
    
    QStringList domaines;
    for(auto cb : domainesChecks) if(cb->isChecked()) domaines << cb->text();
    query.addBindValue(domaines.join(", "));
    
    QStringList zones;
    for(auto cb : zonesChecks) if(cb->isChecked()) zones << cb->text();
    query.addBindValue(zones.join(", "));
    
    query.addBindValue(decisionCombo->currentText());

    if (!query.exec()) {
        db.rollback();
        QMessageBox::critical(this, "Erreur", "Erreur lors de l'enregistrement de l'organisation :\n" + query.lastError().text());
        return;
    }

    int orgId = query.lastInsertId().toInt();

    // 2. Sauvegarde des dirigeants
    for (int i = 0; i < dirigeantsTable->rowCount(); ++i) {
        QTableWidgetItem *itemNom = dirigeantsTable->item(i, 0);
        if (!itemNom || itemNom->text().isEmpty()) continue;
        
        QString fct = dirigeantsTable->item(i, 1) ? dirigeantsTable->item(i, 1)->text() : "";
        QString tel = dirigeantsTable->item(i, 2) ? dirigeantsTable->item(i, 2)->text() : "";
        
        QSqlQuery qDirigeant;
        qDirigeant.prepare("INSERT INTO Dirigeants (organisation_id, nom, fonction, telephone) VALUES (?, ?, ?, ?)");
        qDirigeant.addBindValue(orgId);
        qDirigeant.addBindValue(itemNom->text());
        qDirigeant.addBindValue(fct);
        qDirigeant.addBindValue(tel);
        if(!qDirigeant.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Erreur", "Erreur (Dirigeants) : " + qDirigeant.lastError().text());
            return;
        }
    }

    // 3. Sauvegarde des projets
    for (int i = 0; i < projetsTable->rowCount(); ++i) {
        QTableWidgetItem *itemNom = projetsTable->item(i, 0);
        if (!itemNom || itemNom->text().isEmpty()) continue;
        
        QString bailleur = projetsTable->item(i, 1) ? projetsTable->item(i, 1)->text() : "";
        QString budget = projetsTable->item(i, 2) ? projetsTable->item(i, 2)->text() : "";
        
        QSqlQuery qProjet;
        qProjet.prepare("INSERT INTO Projets (organisation_id, nom, bailleur, budget) VALUES (?, ?, ?, ?)");
        qProjet.addBindValue(orgId);
        qProjet.addBindValue(itemNom->text());
        qProjet.addBindValue(bailleur);
        qProjet.addBindValue(budget);
        if(!qProjet.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Erreur", "Erreur (Projets) : " + qProjet.lastError().text());
            return;
        }
    }

    db.commit();
    QMessageBox::information(this, "Succès", "La demande d'identification a été enregistrée avec succès !");
    accept();
}
