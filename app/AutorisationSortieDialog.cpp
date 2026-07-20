#include "AutorisationSortieDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QDate>
#include <QDebug>
#include <QFileDialog>

AutorisationSortieDialog::AutorisationSortieDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Générer une Autorisation de Sortie");
    resize(500, 400);
    setupUi();
    loadAgents();
}

AutorisationSortieDialog::~AutorisationSortieDialog()
{
}

void AutorisationSortieDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    agentCombo = new QComboBox(this);
    destinationEdit = new QLineEdit(this);
    motifEdit = new QLineEdit(this);
    
    dateDepartEdit = new QDateEdit(QDate::currentDate(), this);
    dateDepartEdit->setCalendarPopup(true);
    
    dateRetourEdit = new QDateEdit(QDate::currentDate().addDays(7), this);
    dateRetourEdit->setCalendarPopup(true);
    
    transportCombo = new QComboBox(this);
    transportCombo->addItems({"Véhicule", "Moto", "Bateau", "Avion"});
    
    mentionsEdit = new QLineEdit(this);
    mentionsEdit->setPlaceholderText("Facultatif...");

    formLayout->addRow("Agent :", agentCombo);
    formLayout->addRow("Destination :", destinationEdit);
    formLayout->addRow("Motif :", motifEdit);
    formLayout->addRow("Date de départ :", dateDepartEdit);
    formLayout->addRow("Date de retour :", dateRetourEdit);
    formLayout->addRow("Transport :", transportCombo);
    formLayout->addRow("Mentions spéciales :", mentionsEdit);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnSave = new QPushButton("Générer et Sauvegarder", this);
    btnCancel = new QPushButton("Annuler", this);
    
    btnSave->setStyleSheet("padding: 8px; background-color: #2980b9; color: white; border-radius: 4px;");
    btnCancel->setStyleSheet("padding: 8px; border: 1px solid #7f8c8d; border-radius: 4px;");

    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);

    mainLayout->addLayout(btnLayout);

    connect(btnSave, &QPushButton::clicked, this, &AutorisationSortieDialog::saveAndGeneratePdf);
    connect(btnCancel, &QPushButton::clicked, this, &AutorisationSortieDialog::reject);
}

void AutorisationSortieDialog::loadAgents()
{
    QSqlQuery query("SELECT id, nom, postnom, matricule FROM Agents");
    while (query.next()) {
        int id = query.value(0).toInt();
        QString nom = query.value(1).toString();
        QString postnom = query.value(2).toString();
        QString matricule = query.value(3).toString();
        
        QString displayText = QString("%1 %2 (%3)").arg(nom, postnom, matricule);
        agentCombo->addItem(displayText, id);
    }
}

void AutorisationSortieDialog::saveAndGeneratePdf()
{
    int agentId = agentCombo->currentData().toInt();
    
    if (agentId == 0 || destinationEdit->text().isEmpty() || motifEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir les champs obligatoires (Agent, Destination, Motif).");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO AutorisationSortie (agent_id, destination, motif, duree, date_depart, date_retour, transport) "
                  "VALUES (:agent_id, :destination, :motif, :duree, :date_depart, :date_retour, :transport)");
    query.bindValue(":agent_id", agentId);
    query.bindValue(":destination", destinationEdit->text());
    query.bindValue(":motif", motifEdit->text());
    
    int duree = dateDepartEdit->date().daysTo(dateRetourEdit->date());
    query.bindValue(":duree", QString("%1 Jours").arg(duree));
    
    query.bindValue(":date_depart", dateDepartEdit->date().toString("dd/MM/yyyy"));
    query.bindValue(":date_retour", dateRetourEdit->date().toString("dd/MM/yyyy"));
    query.bindValue(":transport", transportCombo->currentText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur BDD", "Impossible d'enregistrer l'autorisation : " + query.lastError().text());
        return;
    }

    int autorisationId = query.lastInsertId().toInt();
    
    generatePdf(autorisationId);
    accept();
}

void AutorisationSortieDialog::generatePdf(int autorisationId)
{
    QSqlQuery query;
    query.prepare("SELECT nom, postnom, grade, matricule, fonction, service FROM Agents WHERE id = :id");
    query.bindValue(":id", agentCombo->currentData().toInt());
    query.exec();
    
    QString nomAgent, grade, matricule, fonction, service;
    if (query.next()) {
        nomAgent = query.value(0).toString() + " " + query.value(1).toString();
        grade = query.value(2).toString();
        matricule = query.value(3).toString();
        fonction = query.value(4).toString();
        service = query.value(5).toString();
    }

    QString html = R"(
        <h3 align="center">REPUBLIQUE DEMOCRATIQUE DU CONGO</h3>
        <p align="center"><b>MINISTERE DU PLAN ET DE LA COORDINATION DE L'AIDE AU DEVELOPPEMENT</b><br>
        PROVINCE DU KWILU<br>
        DIVISION PROVINCIALE DU KWILU</p>
        <hr>
        <h2 align="center">AUTORISATION DE SORTIE N° %1 /DPL-KLU/2026</h2>
        <p>Je soussigné <b>Firmin MAFUNDU LEMBUSA</b>, Chef de Division Provinciale du Plan, autorise par la présente Monsieur/Madame :</p>
        <table border="0" width="100%" cellspacing="5">
            <tr><td width="30%"><b>Noms</b></td><td>: %2</td></tr>
            <tr><td><b>Grade</b></td><td>: %3</td></tr>
            <tr><td><b>Matricule</b></td><td>: %4</td></tr>
            <tr><td><b>Fonction</b></td><td>: %5</td></tr>
            <tr><td><b>Service</b></td><td>: %6</td></tr>
            <tr><td><b>Se rendant à</b></td><td>: %7</td></tr>
            <tr><td><b>Motif</b></td><td>: %8</td></tr>
            <tr><td><b>Durée</b></td><td>: %9 Jours</td></tr>
            <tr><td><b>Départ prévu</b></td><td>: %10</td></tr>
            <tr><td><b>Retour probable</b></td><td>: %11</td></tr>
            <tr><td><b>Transport</b></td><td>: %12</td></tr>
        </table>
        <p><b>Mentions spéciales :</b> %13</p>
        <p>Les Autorités tant Civiles, Militaires que de la Police Nationale Congolaise sont priées d'apporter leur assistance au porteur de la présente en cas de nécessité.</p>
        <p align="right">Fait à Bandundu, le %14<br><br><b>Firmin MAFUNDU LEMBUSA</b><br>Chef de Division</p>
    )";

    int duree = dateDepartEdit->date().daysTo(dateRetourEdit->date());
    html = html.arg(QString::number(autorisationId), nomAgent, grade, matricule, fonction, service)
               .arg(destinationEdit->text(), motifEdit->text(), QString::number(duree))
               .arg(dateDepartEdit->date().toString("dd/MM/yyyy"), dateRetourEdit->date().toString("dd/MM/yyyy"))
               .arg(transportCombo->currentText(), mentionsEdit->text())
               .arg(QDate::currentDate().toString("dd/MM/yyyy"));

    QTextDocument document;
    document.setHtml(html);

    QString fileName = QFileDialog::getSaveFileName(this, "Sauvegarder le PDF", "Autorisation_Sortie_" + nomAgent.replace(" ", "_") + ".pdf", "*.pdf");
    if (!fileName.isEmpty()) {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        document.print(&printer);
        QMessageBox::information(this, "Succès", "Le PDF a été généré avec succès !");
    }
}
