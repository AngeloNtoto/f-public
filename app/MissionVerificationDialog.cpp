// MissionVerificationDialog.cpp
// Rapport certifié de mission de vérification sur terrain
// Secteurs Sociaux - Province du Kwilu

#include "MissionVerificationDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QTextDocument>
#include <QPrinter>
#include <QFileDialog>
#include <QDate>
#include <QLabel>
#include <QHeaderView>
#include <QScrollArea>
#include <QGroupBox>

MissionVerificationDialog::MissionVerificationDialog(int orgId, const QString& orgName, QWidget *parent)
    : QDialog(parent), m_orgId(orgId), m_orgName(orgName)
{
    setWindowTitle("Rapport de Mission - " + orgName);
    resize(800, 600);
    setupUi();
}

MissionVerificationDialog::~MissionVerificationDialog() {}

void MissionVerificationDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    
    // --- 1. Équipe de mission ---
    QGroupBox *groupEquipe = new QGroupBox("Équipe de mission");
    QVBoxLayout *equipeLayout = new QVBoxLayout(groupEquipe);
    QPushButton *btnAddMembre = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Ajouter un membre à l'équipe");
    btnAddMembre->setStyleSheet("background-color: #2980b9; color: white; padding: 5px; border-radius: 4px;");
    
    equipeTable = new QTableWidget(0, 2);
    equipeTable->setHorizontalHeaderLabels({"Nom complet", "Fonction"});
    equipeTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    connect(btnAddMembre, &QPushButton::clicked, this, &MissionVerificationDialog::addEquipeRow);
    addEquipeRow(); // Ligne par défaut
    
    equipeLayout->addWidget(btnAddMembre);
    equipeLayout->addWidget(equipeTable);
    scrollLayout->addWidget(groupEquipe);
    
    // --- 2. Situation administrative et juridique ---
    QGroupBox *groupAdmin = new QGroupBox("Situation administrative et juridique (Vérification des documents)");
    QVBoxLayout *adminLayout = new QVBoxLayout(groupAdmin);
    situationAdminEdit = new QTextEdit();
    situationAdminEdit->setPlaceholderText("Vérification de la lettre de demande, des statuts, du règlement intérieur, etc.");
    situationAdminEdit->setMaximumHeight(80);
    adminLayout->addWidget(situationAdminEdit);
    scrollLayout->addWidget(groupAdmin);
    
    // --- 3. Gouvernance et fonctionnement institutionnel ---
    QGroupBox *groupGov = new QGroupBox("Gouvernance et fonctionnement institutionnel");
    QFormLayout *formGov = new QFormLayout(groupGov);
    
    agFonctionnelleCombo = new QComboBox();
    agFonctionnelleCombo->addItems({"Oui - Assemblée Générale Fonctionnelle", "Non - Non fonctionnelle"});
    
    caCombo = new QComboBox();
    caCombo->addItems({"Oui - Conseil d'Administration Actif", "Non - Inactif / Non structuré"});
    
    comiteGestionCombo = new QComboBox();
    comiteGestionCombo->addItems({"Oui - Comité de gestion opérationnel", "Non - Absent / Inopérant"});
    
    coordExecCombo = new QComboBox();
    coordExecCombo->addItems({"Oui - Coordination exécutive en place", "Non - Absente"});
    
    formGov->addRow("AG fonctionnelle :", agFonctionnelleCombo);
    formGov->addRow("Conseil d'Administration :", caCombo);
    formGov->addRow("Comité de gestion :", comiteGestionCombo);
    formGov->addRow("Coordination Exécutive :", coordExecCombo);
    scrollLayout->addWidget(groupGov);
    
    // --- 4. Ressources humaines et matérielles ---
    QGroupBox *groupRhMat = new QGroupBox("Ressources humaines et matérielles");
    QVBoxLayout *rhMatLayout = new QVBoxLayout(groupRhMat);
    rhMaterielEdit = new QTextEdit();
    rhMaterielEdit->setPlaceholderText("Description des effectifs réels, bureaux fonctionnels, équipements disponibles...");
    rhMaterielEdit->setMaximumHeight(80);
    rhMatLayout->addWidget(rhMaterielEdit);
    scrollLayout->addWidget(groupRhMat);
    
    // --- 5. Vérification des projets et activités ---
    QGroupBox *groupProjets = new QGroupBox("Vérification des projets et activités");
    QFormLayout *formProjets = new QFormLayout(groupProjets);
    
    verificationProjetsEdit = new QTextEdit();
    verificationProjetsEdit->setPlaceholderText("Secteurs d'intervention, réalisations observées, projets réels...");
    verificationProjetsEdit->setMaximumHeight(80);
    
    niveauExecutionCombo = new QComboBox();
    niveauExecutionCombo->addItems({"Très satisfaisant", "Satisfaisant", "Moyen", "Faible"});
    
    formProjets->addRow("Constats projets :", verificationProjetsEdit);
    formProjets->addRow("Niveau d'exécution :", niveauExecutionCombo);
    scrollLayout->addWidget(groupProjets);
    
    // --- 6. Impact communautaire & contraintes ---
    QGroupBox *groupImpact = new QGroupBox("Impact communautaire & Contraintes");
    QFormLayout *formImpact = new QFormLayout(groupImpact);
    
    impactEdit = new QTextEdit();
    impactEdit->setPlaceholderText("Confirmation par les bénéficiaires sur le terrain...");
    impactEdit->setMaximumHeight(60);
    
    contraintesEdit = new QTextEdit();
    contraintesEdit->setPlaceholderText("Contraintes et insuffisances observées...");
    contraintesEdit->setMaximumHeight(60);
    
    formImpact->addRow("Impact communautaire :", impactEdit);
    formImpact->addRow("Contraintes :", contraintesEdit);
    scrollLayout->addWidget(groupImpact);
    
    // --- 7. Conclusion & Recommandations ---
    QGroupBox *groupConclusion = new QGroupBox("Conclusion & Recommandations");
    QFormLayout *formConclusion = new QFormLayout(groupConclusion);
    
    conclusionCombo = new QComboBox();
    conclusionCombo->addItems({"Existe et exerce", "Existe avec insuffisances", "Ne remplit pas les conditions"});
    
    recoStructureEdit = new QTextEdit();
    recoStructureEdit->setPlaceholderText("Recommandations à la structure...");
    recoStructureEdit->setMaximumHeight(60);
    
    recoDivisionEdit = new QTextEdit();
    recoDivisionEdit->setPlaceholderText("Recommandations à la Division Provinciale...");
    recoDivisionEdit->setMaximumHeight(60);
    
    formConclusion->addRow("Conclusion mission :", conclusionCombo);
    formConclusion->addRow("Reco à la structure :", recoStructureEdit);
    formConclusion->addRow("Reco à la Division :", recoDivisionEdit);
    scrollLayout->addWidget(groupConclusion);
    
    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);
    
    // Boutons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton(style()->standardIcon(QStyle::SP_DialogSaveButton), "Sauvegarder & Générer PDF");
    btnSave->setStyleSheet("padding: 8px 16px; background-color: #27ae60; color: white; border-radius: 4px; font-weight: bold;");
    QPushButton *btnCancel = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Annuler");
    btnCancel->setStyleSheet("padding: 8px 16px; background-color: #7f8c8d; color: white; border-radius: 4px;");
    
    connect(btnSave, &QPushButton::clicked, this, &MissionVerificationDialog::saveAndGeneratePdf);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    
    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);
}

void MissionVerificationDialog::addEquipeRow()
{
    int row = equipeTable->rowCount();
    equipeTable->insertRow(row);
}

void MissionVerificationDialog::saveAndGeneratePdf()
{
    // Construire la liste de l'équipe
    QStringList membres;
    for (int i = 0; i < equipeTable->rowCount(); ++i) {
        QTableWidgetItem *itemNom = equipeTable->item(i, 0);
        QTableWidgetItem *itemFct = equipeTable->item(i, 1);
        if (itemNom && !itemNom->text().isEmpty()) {
            QString role = itemFct ? itemFct->text() : "Inspecteur";
            membres << QString("%1 (%2)").arg(itemNom->text(), role);
        }
    }
    QString equipeStr = membres.join(", ");
    
    // Sauvegarder dans la base de données
    QSqlQuery query;
    query.prepare(
        "REPLACE INTO MissionsVerification ("
        "organisation_id, date_mission, equipe, situation_admin, gouvernance, "
        "rh_materiel, verification_projets, niveau_execution, impact_communautaire, "
        "contraintes, conclusion, recommandations_structure, recommandations_division"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    );
    query.addBindValue(m_orgId);
    query.addBindValue(QDate::currentDate().toString("dd/MM/yyyy"));
    query.addBindValue(equipeStr);
    query.addBindValue(situationAdminEdit->toPlainText());
    
    // Concaténer la gouvernance
    QString govStr = QString("AG: %1, CA: %2, Comité: %3, Coord: %4")
        .arg(agFonctionnelleCombo->currentText())
        .arg(caCombo->currentText())
        .arg(comiteGestionCombo->currentText())
        .arg(coordExecCombo->currentText());
    query.addBindValue(govStr);
    
    query.addBindValue(rhMaterielEdit->toPlainText());
    query.addBindValue(verificationProjetsEdit->toPlainText());
    query.addBindValue(niveauExecutionCombo->currentText());
    query.addBindValue(impactEdit->toPlainText());
    query.addBindValue(contraintesEdit->toPlainText());
    query.addBindValue(conclusionCombo->currentText());
    query.addBindValue(recoStructureEdit->toPlainText());
    query.addBindValue(recoDivisionEdit->toPlainText());
    
    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur BDD", query.lastError().text());
        return;
    }
    
    // Générer Rapport PDF certifié
    QString html = R"(
        <h3 align="center">RÉPUBLIQUE DÉMOCRATIQUE DU CONGO</h3>
        <h4 align="center">PROVINCE DU KWILU</h4>
        <h2 align="center">RAPPORT CERTIFIÉ DE MISSION DE VÉRIFICATION SUR TERRAIN</h2>
        <hr>
        <p><b>Organisation inspectée :</b> %1</p>
        <p><b>Date d'inspection :</b> %2</p>
        <p><b>Équipe de mission :</b> %3</p>
        
        <h3>1. Situation administrative et juridique</h3>
        <p>%4</p>
        
        <h3>2. Gouvernance et fonctionnement institutionnel</h3>
        <p>%5</p>
        
        <h3>3. Ressources humaines et matérielles</h3>
        <p>%6</p>
        
        <h3>4. Projets et activités</h3>
        <p><b>Constats sur le terrain :</b> %7</p>
        <p><b>Niveau d'exécution :</b> %8</p>
        
        <h3>5. Impact communautaire</h3>
        <p>%9</p>
        
        <h3>6. Contraintes et insuffisances</h3>
        <p>%10</p>
        
        <h3>7. Conclusion de la mission</h3>
        <p><b>Statut final :</b> <font color="blue"><b>%11</b></font></p>
        
        <h3>8. Recommandations</h3>
        <p><b>À la structure :</b> %12</p>
        <p><b>À la Division Provinciale :</b> %13</p>
        
        <br><br>
        <p align="right">Fait à Bandundu, le %2</p>
        <p align="right"><b>L'Équipe d'Inspection</b></p>
    )";
    
    html = html.arg(
        m_orgName, QDate::currentDate().toString("dd/MM/yyyy"), equipeStr,
        situationAdminEdit->toPlainText(), govStr, rhMaterielEdit->toPlainText(),
        verificationProjetsEdit->toPlainText(), niveauExecutionCombo->currentText(),
        impactEdit->toPlainText(), contraintesEdit->toPlainText(),
        conclusionCombo->currentText(), recoStructureEdit->toPlainText(),
        recoDivisionEdit->toPlainText()
    );
    
    QTextDocument document;
    document.setHtml(html);
    
    QString fileName = QFileDialog::getSaveFileName(this, "Sauvegarder le Rapport", "Rapport_" + m_orgName.replace(" ", "_") + ".pdf", "*.pdf");
    if (!fileName.isEmpty()) {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        document.print(&printer);
        QMessageBox::information(this, "Succès", "Rapport sauvegardé avec succès.");
    }
    
    accept();
}
