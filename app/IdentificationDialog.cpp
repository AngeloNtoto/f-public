// IdentificationDialog.cpp
// Formulaire complet de demande d'identification ONG/ASBL/Fondation/EUP
// Secteurs Sociaux - Province du Kwilu

#include "IdentificationDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

IdentificationDialog::IdentificationDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Fiche de Demande d'Identification - ONG / ASBL / Fondation / EUP");
    resize(900, 650);
    setupUi();
}

IdentificationDialog::~IdentificationDialog() {}

void IdentificationDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QTabWidget *tabWidget = new QTabWidget(this);

    // ================================================================
    // ONGLET 1 : Identification de l'Organisation & Représentant Légal
    // ================================================================
    QWidget *tab1 = new QWidget();
    QScrollArea *scroll1 = new QScrollArea();
    scroll1->setWidgetResizable(true);
    QWidget *scrollContent1 = new QWidget();
    QVBoxLayout *layout1 = new QVBoxLayout(scrollContent1);

    // --- Groupe : Enregistrement ---
    QGroupBox *groupEnreg = new QGroupBox("Enregistrement");
    QFormLayout *formEnreg = new QFormLayout(groupEnreg);
    numEnregistrementEdit = new QLineEdit();
    numEnregistrementEdit->setPlaceholderText("Ex: ENR-2026-001");
    dateReceptionEdit = new QDateEdit(QDate::currentDate());
    dateReceptionEdit->setCalendarPopup(true);
    formEnreg->addRow("N° d'enregistrement :", numEnregistrementEdit);
    formEnreg->addRow("Date de réception :", dateReceptionEdit);
    layout1->addWidget(groupEnreg);

    // --- Groupe : Identification de l'Organisation ---
    QGroupBox *groupOrg = new QGroupBox("Identification de l'Organisation");
    QFormLayout *formOrg = new QFormLayout(groupOrg);
    nomEdit = new QLineEdit();
    nomEdit->setPlaceholderText("Dénomination officielle complète");
    sigleEdit = new QLineEdit();
    sigleEdit->setPlaceholderText("Sigle / Acronyme");
    natureCombo = new QComboBox();
    natureCombo->addItems({"ONGD", "ASBL", "Fondation", "EUP"});
    dateCreationEdit = new QDateEdit(QDate::currentDate());
    dateCreationEdit->setCalendarPopup(true);
    dateDebutProvinceEdit = new QDateEdit(QDate::currentDate());
    dateDebutProvinceEdit->setCalendarPopup(true);
    numPersoJuridiqueEdit = new QLineEdit();
    numPersoJuridiqueEdit->setPlaceholderText("Numéro de personnalité juridique ou d'autorisation");
    autoriteDelivranceEdit = new QLineEdit();
    autoriteDelivranceEdit->setPlaceholderText("Autorité ayant délivré");
    adresseSiegeEdit = new QLineEdit();
    adresseSiegeEdit->setPlaceholderText("Adresse du siège social");
    adresseProvinceEdit = new QLineEdit();
    adresseProvinceEdit->setPlaceholderText("Adresse complète dans la province (Kwilu)");
    telephoneEdit = new QLineEdit();
    telephoneEdit->setPlaceholderText("+243...");
    emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText("email@exemple.com");
    siteWebEdit = new QLineEdit();
    siteWebEdit->setPlaceholderText("www.exemple.org (si applicable)");

    formOrg->addRow("Dénomination officielle * :", nomEdit);
    formOrg->addRow("Sigle :", sigleEdit);
    formOrg->addRow("Nature juridique :", natureCombo);
    formOrg->addRow("Date de création :", dateCreationEdit);
    formOrg->addRow("Date début activités province :", dateDebutProvinceEdit);
    formOrg->addRow("N° personnalité juridique :", numPersoJuridiqueEdit);
    formOrg->addRow("Autorité ayant délivré :", autoriteDelivranceEdit);
    formOrg->addRow("Adresse siège social :", adresseSiegeEdit);
    formOrg->addRow("Adresse dans la province :", adresseProvinceEdit);
    formOrg->addRow("Téléphone :", telephoneEdit);
    formOrg->addRow("E-mail :", emailEdit);
    formOrg->addRow("Site internet :", siteWebEdit);
    layout1->addWidget(groupOrg);

    // --- Groupe : Représentant Légal ---
    QGroupBox *groupRep = new QGroupBox("Identification du Représentant Légal");
    QFormLayout *formRep = new QFormLayout(groupRep);
    repNomEdit = new QLineEdit();
    repNomEdit->setPlaceholderText("Noms complets du représentant");
    repFonctionEdit = new QLineEdit();
    repFonctionEdit->setPlaceholderText("Fonction dans l'organisation");
    repNationaliteEdit = new QLineEdit();
    repNationaliteEdit->setPlaceholderText("Nationalité");
    repPhoneEdit = new QLineEdit();
    repPhoneEdit->setPlaceholderText("+243...");
    repEmailEdit = new QLineEdit();
    repEmailEdit->setPlaceholderText("email@exemple.com");
    repAdresseEdit = new QLineEdit();
    repAdresseEdit->setPlaceholderText("Adresse physique du représentant");

    formRep->addRow("Noms :", repNomEdit);
    formRep->addRow("Fonction :", repFonctionEdit);
    formRep->addRow("Nationalité :", repNationaliteEdit);
    formRep->addRow("Téléphone :", repPhoneEdit);
    formRep->addRow("Adresse électronique :", repEmailEdit);
    formRep->addRow("Adresse physique :", repAdresseEdit);
    layout1->addWidget(groupRep);

    scroll1->setWidget(scrollContent1);
    QVBoxLayout *tab1Layout = new QVBoxLayout(tab1);
    tab1Layout->addWidget(scroll1);
    tabWidget->addTab(tab1, style()->standardIcon(QStyle::SP_FileIcon), "1. Identification");

    // ================================================================
    // ONGLET 2 : Organe Dirigeant & Effectifs
    // ================================================================
    QWidget *tab2 = new QWidget();
    QVBoxLayout *layout2 = new QVBoxLayout(tab2);

    // --- Composition de l'organe dirigeant ---
    QGroupBox *groupDirig = new QGroupBox("Composition de l'Organe Dirigeant");
    QVBoxLayout *dirigLayout = new QVBoxLayout(groupDirig);
    QPushButton *btnAddDirigeant = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Ajouter un dirigeant");
    btnAddDirigeant->setStyleSheet("padding: 6px; background-color: #2980b9; color: white; border-radius: 4px;");

    dirigeantsTable = new QTableWidget(0, 5);
    dirigeantsTable->setHorizontalHeaderLabels({"N°", "Nom et Post-nom", "Fonction", "Téléphone", "Email"});
    dirigeantsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(btnAddDirigeant, &QPushButton::clicked, this, &IdentificationDialog::addDirigeantRow);
    addDirigeantRow(); // 1 ligne par défaut

    dirigLayout->addWidget(btnAddDirigeant);
    dirigLayout->addWidget(dirigeantsTable);
    layout2->addWidget(groupDirig);

    // --- Effectif de l'organisation ---
    QGroupBox *groupEffectifs = new QGroupBox("Effectif de l'Organisation");
    QVBoxLayout *effLayout = new QVBoxLayout(groupEffectifs);
    QPushButton *btnAddEffectif = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Ajouter une catégorie");
    btnAddEffectif->setStyleSheet("padding: 6px; background-color: #27ae60; color: white; border-radius: 4px;");

    effectifsTable = new QTableWidget(0, 3);
    effectifsTable->setHorizontalHeaderLabels({"N°", "Catégorie du personnel", "Effectif"});
    effectifsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(btnAddEffectif, &QPushButton::clicked, this, &IdentificationDialog::addEffectifRow);
    // Pré-remplir les 3 catégories réglementaires
    effectifsTable->insertRow(0);
    effectifsTable->setItem(0, 0, new QTableWidgetItem("1"));
    effectifsTable->setItem(0, 1, new QTableWidgetItem("Personnel permanent"));
    effectifsTable->insertRow(1);
    effectifsTable->setItem(1, 0, new QTableWidgetItem("2"));
    effectifsTable->setItem(1, 1, new QTableWidgetItem("Personnel temporaire"));
    effectifsTable->insertRow(2);
    effectifsTable->setItem(2, 0, new QTableWidgetItem("3"));
    effectifsTable->setItem(2, 1, new QTableWidgetItem("Volontaire"));

    effLayout->addWidget(btnAddEffectif);
    effLayout->addWidget(effectifsTable);
    layout2->addWidget(groupEffectifs);

    tabWidget->addTab(tab2, style()->standardIcon(QStyle::SP_DirIcon), "2. Dirigeants & Effectifs");

    // ================================================================
    // ONGLET 3 : Domaines & Zones d'Intervention
    // ================================================================
    QWidget *tab3 = new QWidget();
    QScrollArea *scroll3 = new QScrollArea();
    scroll3->setWidgetResizable(true);
    QWidget *scrollContent3 = new QWidget();
    QVBoxLayout *layout3 = new QVBoxLayout(scrollContent3);

    // --- Domaines d'intervention ---
    QGroupBox *groupDomaines = new QGroupBox("Domaines d'intervention (cocher)");
    QGridLayout *gridDomaines = new QGridLayout(groupDomaines);
    QStringList listDomaines = {
        "Agriculture et développement rural",
        "Sécurité alimentaire",
        "Élevage",
        "Éducation / formation professionnelle",
        "Eau, hygiène et assainissement",
        "Santé, nutrition",
        "Environnement et changement climatique",
        "Développement communautaire",
        "Genre et autonomisation de la femme",
        "Promotion des droits humains",
        "Entrepreneuriat et emploi",
        "Assistance humanitaire",
        "Autres"
    };
    for (int i = 0; i < listDomaines.size(); ++i) {
        QCheckBox *cb = new QCheckBox(listDomaines[i]);
        domainesChecks.append(cb);
        gridDomaines->addWidget(cb, i / 2, i % 2);
    }
    layout3->addWidget(groupDomaines);

    // --- Zones d'intervention ---
    QGroupBox *groupZones = new QGroupBox("Zone d'intervention (cocher)");
    QGridLayout *gridZones = new QGridLayout(groupZones);
    QStringList listZones = {
        "Bandundu",
        "Kikwit",
        "Bagata",
        "Bulungu",
        "Gungu",
        "Idiofa",
        "Masi-Manimba"
    };
    for (int i = 0; i < listZones.size(); ++i) {
        QCheckBox *cb = new QCheckBox(listZones[i]);
        zonesChecks.append(cb);
        gridZones->addWidget(cb, i / 2, i % 2);
    }
    layout3->addWidget(groupZones);
    layout3->addStretch();

    scroll3->setWidget(scrollContent3);
    QVBoxLayout *tab3Layout = new QVBoxLayout(tab3);
    tab3Layout->addWidget(scroll3);
    tabWidget->addTab(tab3, style()->standardIcon(QStyle::SP_FileDialogDetailedView), "3. Intervention");

    // ================================================================
    // ONGLET 4 : Partenaires & Projets en cours
    // ================================================================
    QWidget *tab4 = new QWidget();
    QVBoxLayout *layout4 = new QVBoxLayout(tab4);

    // --- Partenaires techniques et financiers ---
    QGroupBox *groupPartenaires = new QGroupBox("Partenaires Techniques et Financiers");
    QVBoxLayout *partLayout = new QVBoxLayout(groupPartenaires);
    QPushButton *btnAddPartenaire = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Ajouter un partenaire");
    btnAddPartenaire->setStyleSheet("padding: 6px; background-color: #8e44ad; color: white; border-radius: 4px;");

    partenairesTable = new QTableWidget(0, 6);
    partenairesTable->setHorizontalHeaderLabels({"N°", "Nom du partenaire", "Pays/Organisation", "Financement reçu", "Projet réalisé", "Période"});
    partenairesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(btnAddPartenaire, &QPushButton::clicked, this, &IdentificationDialog::addPartenaireRow);
    addPartenaireRow();

    partLayout->addWidget(btnAddPartenaire);
    partLayout->addWidget(partenairesTable);
    layout4->addWidget(groupPartenaires);

    // --- Principaux projets en cours ---
    QGroupBox *groupProjets = new QGroupBox("Principaux Projets en Cours");
    QVBoxLayout *projLayout = new QVBoxLayout(groupProjets);
    QPushButton *btnAddProjet = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Ajouter un projet");
    btnAddProjet->setStyleSheet("padding: 6px; background-color: #e67e22; color: white; border-radius: 4px;");

    projetsTable = new QTableWidget(0, 6);
    projetsTable->setHorizontalHeaderLabels({"N°", "Intitulé du projet", "Localisation", "Coût du projet", "Source de financement", "Période"});
    projetsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(btnAddProjet, &QPushButton::clicked, this, &IdentificationDialog::addProjetRow);
    addProjetRow();

    projLayout->addWidget(btnAddProjet);
    projLayout->addWidget(projetsTable);
    layout4->addWidget(groupProjets);

    tabWidget->addTab(tab4, style()->standardIcon(QStyle::SP_FileDialogListView), "4. Partenaires & Projets");

    // ================================================================
    // ONGLET 5 : Documents Annexes
    // ================================================================
    QWidget *tab5 = new QWidget();
    QVBoxLayout *layout5 = new QVBoxLayout(tab5);

    QGroupBox *groupDocs = new QGroupBox("Documents Annexes (cocher les documents reçus)");
    QGridLayout *gridDocs = new QGridLayout(groupDocs);
    QStringList listDocs = {
        "Lettre de demande",
        "Statuts",
        "Acte constitutif",
        "Règlement intérieur",
        "Personnalité juridique",
        "Liste des membres dirigeants",
        "Pièce d'identité du représentant",
        "CV du représentant",
        "Plan d'action",
        "Rapport d'activité",
        "Preuve de paiement",
        "Autre"
    };
    for (int i = 0; i < listDocs.size(); ++i) {
        QCheckBox *cb = new QCheckBox(listDocs[i]);
        annexesChecks.append(cb);
        gridDocs->addWidget(cb, i / 3, i % 3);
    }
    layout5->addWidget(groupDocs);
    layout5->addStretch();

    tabWidget->addTab(tab5, style()->standardIcon(QStyle::SP_FileDialogContentsView), "5. Documents Annexes");

    // ================================================================
    // ONGLET 6 : Déclarations & Décision
    // ================================================================
    QWidget *tab6 = new QWidget();
    QVBoxLayout *layout6 = new QVBoxLayout(tab6);

    QGroupBox *groupObs = new QGroupBox("Observations");
    QVBoxLayout *obsLayout = new QVBoxLayout(groupObs);
    observationsEdit = new QTextEdit();
    observationsEdit->setPlaceholderText("Observations du service d'enregistrement...");
    observationsEdit->setMaximumHeight(120);
    obsLayout->addWidget(observationsEdit);
    layout6->addWidget(groupObs);

    QGroupBox *groupDecision = new QGroupBox("Décision Finale");
    QFormLayout *decLayout = new QFormLayout(groupDecision);
    decisionCombo = new QComboBox();
    decisionCombo->addItems({
        "En attente",
        "Dossier confirmé",
        "Dossier à compléter",
        "Enregistrement accordé",
        "Enregistrement refusé"
    });
    decLayout->addRow("Statut du dossier :", decisionCombo);
    layout6->addWidget(groupDecision);
    layout6->addStretch();

    tabWidget->addTab(tab6, style()->standardIcon(QStyle::SP_DialogApplyButton), "6. Décision");

    // ================================================================
    // Boutons d'action
    // ================================================================
    mainLayout->addWidget(tabWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnSave = new QPushButton(style()->standardIcon(QStyle::SP_DialogSaveButton), "Enregistrer la demande", this);
    btnSave->setStyleSheet("padding: 10px 20px; background-color: #27ae60; color: white; border-radius: 4px; font-weight: bold;");
    btnCancel = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Annuler", this);
    btnCancel->setStyleSheet("padding: 10px 20px; background-color: #7f8c8d; color: white; border-radius: 4px;");

    connect(btnSave, &QPushButton::clicked, this, &IdentificationDialog::saveToDatabase);
    connect(btnCancel, &QPushButton::clicked, this, &IdentificationDialog::reject);

    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);
}

// --- Ajout de lignes dans les tableaux ---

void IdentificationDialog::addDirigeantRow()
{
    int row = dirigeantsTable->rowCount();
    dirigeantsTable->insertRow(row);
    dirigeantsTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
}

void IdentificationDialog::addEffectifRow()
{
    int row = effectifsTable->rowCount();
    effectifsTable->insertRow(row);
    effectifsTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
}

void IdentificationDialog::addPartenaireRow()
{
    int row = partenairesTable->rowCount();
    partenairesTable->insertRow(row);
    partenairesTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
}

void IdentificationDialog::addProjetRow()
{
    int row = projetsTable->rowCount();
    projetsTable->insertRow(row);
    projetsTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
}

// --- Sauvegarde complète en BDD ---

void IdentificationDialog::saveToDatabase()
{
    // Validation minimale
    if (nomEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "La dénomination officielle est obligatoire.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    // 1. Sauvegarde de l'organisation
    QSqlQuery query;
    query.prepare(
        "INSERT INTO Organisations ("
        "num_enregistrement, date_reception, denomination, sigle, nature_juridique, "
        "date_creation, date_debut_province, num_personnalite_juridique, autorite_delivrance, "
        "adresse_siege_social, adresse_province, telephone, email, site_web, "
        "representant_nom, representant_fonction, representant_nationalite, "
        "representant_phone, representant_email, representant_adresse, "
        "domaines, zones, documents_annexes, observations, decision"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );
    query.addBindValue(numEnregistrementEdit->text());
    query.addBindValue(dateReceptionEdit->date().toString("dd/MM/yyyy"));
    query.addBindValue(nomEdit->text());
    query.addBindValue(sigleEdit->text());
    query.addBindValue(natureCombo->currentText());
    query.addBindValue(dateCreationEdit->date().toString("dd/MM/yyyy"));
    query.addBindValue(dateDebutProvinceEdit->date().toString("dd/MM/yyyy"));
    query.addBindValue(numPersoJuridiqueEdit->text());
    query.addBindValue(autoriteDelivranceEdit->text());
    query.addBindValue(adresseSiegeEdit->text());
    query.addBindValue(adresseProvinceEdit->text());
    query.addBindValue(telephoneEdit->text());
    query.addBindValue(emailEdit->text());
    query.addBindValue(siteWebEdit->text());
    query.addBindValue(repNomEdit->text());
    query.addBindValue(repFonctionEdit->text());
    query.addBindValue(repNationaliteEdit->text());
    query.addBindValue(repPhoneEdit->text());
    query.addBindValue(repEmailEdit->text());
    query.addBindValue(repAdresseEdit->text());

    // Domaines cochés
    QStringList domaines;
    for (auto cb : domainesChecks) if (cb->isChecked()) domaines << cb->text();
    query.addBindValue(domaines.join(", "));

    // Zones cochées
    QStringList zones;
    for (auto cb : zonesChecks) if (cb->isChecked()) zones << cb->text();
    query.addBindValue(zones.join(", "));

    // Documents annexes cochés
    QStringList docs;
    for (auto cb : annexesChecks) if (cb->isChecked()) docs << cb->text();
    query.addBindValue(docs.join(", "));

    query.addBindValue(observationsEdit->toPlainText());
    query.addBindValue(decisionCombo->currentText());

    if (!query.exec()) {
        db.rollback();
        QMessageBox::critical(this, "Erreur", "Erreur Organisation :\n" + query.lastError().text());
        return;
    }

    int orgId = query.lastInsertId().toInt();

    // 2. Dirigeants
    for (int i = 0; i < dirigeantsTable->rowCount(); ++i) {
        QTableWidgetItem *itemNom = dirigeantsTable->item(i, 1);
        if (!itemNom || itemNom->text().isEmpty()) continue;

        QSqlQuery qD;
        qD.prepare("INSERT INTO Dirigeants (organisation_id, num_ordre, nom, fonction, telephone, email) VALUES (?, ?, ?, ?, ?, ?)");
        qD.addBindValue(orgId);
        qD.addBindValue(i + 1);
        qD.addBindValue(itemNom->text());
        qD.addBindValue(dirigeantsTable->item(i, 2) ? dirigeantsTable->item(i, 2)->text() : "");
        qD.addBindValue(dirigeantsTable->item(i, 3) ? dirigeantsTable->item(i, 3)->text() : "");
        qD.addBindValue(dirigeantsTable->item(i, 4) ? dirigeantsTable->item(i, 4)->text() : "");
        if (!qD.exec()) { db.rollback(); QMessageBox::critical(this, "Erreur", "Erreur Dirigeants : " + qD.lastError().text()); return; }
    }

    // 3. Effectifs
    for (int i = 0; i < effectifsTable->rowCount(); ++i) {
        QTableWidgetItem *itemCat = effectifsTable->item(i, 1);
        if (!itemCat || itemCat->text().isEmpty()) continue;

        QSqlQuery qE;
        qE.prepare("INSERT INTO Effectifs (organisation_id, num_ordre, categorie, effectif) VALUES (?, ?, ?, ?)");
        qE.addBindValue(orgId);
        qE.addBindValue(i + 1);
        qE.addBindValue(itemCat->text());
        qE.addBindValue(effectifsTable->item(i, 2) ? effectifsTable->item(i, 2)->text().toInt() : 0);
        if (!qE.exec()) { db.rollback(); QMessageBox::critical(this, "Erreur", "Erreur Effectifs : " + qE.lastError().text()); return; }
    }

    // 4. Partenaires techniques et financiers
    for (int i = 0; i < partenairesTable->rowCount(); ++i) {
        QTableWidgetItem *itemNom = partenairesTable->item(i, 1);
        if (!itemNom || itemNom->text().isEmpty()) continue;

        QSqlQuery qP;
        qP.prepare("INSERT INTO Partenaires (organisation_id, num_ordre, nom_partenaire, pays_org, financement, projet_realise, periode) VALUES (?, ?, ?, ?, ?, ?, ?)");
        qP.addBindValue(orgId);
        qP.addBindValue(i + 1);
        qP.addBindValue(itemNom->text());
        qP.addBindValue(partenairesTable->item(i, 2) ? partenairesTable->item(i, 2)->text() : "");
        qP.addBindValue(partenairesTable->item(i, 3) ? partenairesTable->item(i, 3)->text() : "");
        qP.addBindValue(partenairesTable->item(i, 4) ? partenairesTable->item(i, 4)->text() : "");
        qP.addBindValue(partenairesTable->item(i, 5) ? partenairesTable->item(i, 5)->text() : "");
        if (!qP.exec()) { db.rollback(); QMessageBox::critical(this, "Erreur", "Erreur Partenaires : " + qP.lastError().text()); return; }
    }

    // 5. Projets en cours
    for (int i = 0; i < projetsTable->rowCount(); ++i) {
        QTableWidgetItem *itemNom = projetsTable->item(i, 1);
        if (!itemNom || itemNom->text().isEmpty()) continue;

        QSqlQuery qPr;
        qPr.prepare("INSERT INTO Projets (organisation_id, num_ordre, intitule, localisation, budget, bailleur, duree) VALUES (?, ?, ?, ?, ?, ?, ?)");
        qPr.addBindValue(orgId);
        qPr.addBindValue(i + 1);
        qPr.addBindValue(itemNom->text());
        qPr.addBindValue(projetsTable->item(i, 2) ? projetsTable->item(i, 2)->text() : "");
        qPr.addBindValue(projetsTable->item(i, 3) ? projetsTable->item(i, 3)->text() : "");
        qPr.addBindValue(projetsTable->item(i, 4) ? projetsTable->item(i, 4)->text() : "");
        qPr.addBindValue(projetsTable->item(i, 5) ? projetsTable->item(i, 5)->text() : "");
        if (!qPr.exec()) { db.rollback(); QMessageBox::critical(this, "Erreur", "Erreur Projets : " + qPr.lastError().text()); return; }
    }

    db.commit();
    QMessageBox::information(this, "Succès", "La demande d'identification a été enregistrée avec succès !");
    accept();
}
