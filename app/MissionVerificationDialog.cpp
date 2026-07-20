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

MissionVerificationDialog::MissionVerificationDialog(int orgId, const QString& orgName, QWidget *parent)
    : QDialog(parent), m_orgId(orgId), m_orgName(orgName)
{
    setWindowTitle("Rapport de Mission - " + orgName);
    resize(500, 400);
    setupUi();
}

MissionVerificationDialog::~MissionVerificationDialog() {}

void MissionVerificationDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    dateMissionEdit = new QDateEdit(QDate::currentDate());
    dateMissionEdit->setCalendarPopup(true);
    
    equipeEdit = new QLineEdit();
    equipeEdit->setPlaceholderText("Noms des inspecteurs...");
    
    gouvernanceCombo = new QComboBox();
    gouvernanceCombo->addItems({"Très Satisfaisant", "Satisfaisant", "Moyen", "Faible"});
    
    rhCombo = new QComboBox();
    rhCombo->addItems({"Très Satisfaisant", "Satisfaisant", "Moyen", "Faible"});
    
    constatsEdit = new QTextEdit();
    
    conclusionCombo = new QComboBox();
    conclusionCombo->addItems({"Favorable", "Défavorable"});

    formLayout->addRow("Date de la mission :", dateMissionEdit);
    formLayout->addRow("Équipe :", equipeEdit);
    formLayout->addRow("Éval. Gouvernance :", gouvernanceCombo);
    formLayout->addRow("Éval. Ressources Humaines :", rhCombo);
    formLayout->addRow("Constats généraux :", constatsEdit);
    formLayout->addRow("Conclusion :", conclusionCombo);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton("Générer PDF et Sauvegarder");
    btnSave->setStyleSheet("padding: 8px; background-color: #2980b9; color: white; border-radius: 4px;");
    QPushButton *btnCancel = new QPushButton("Annuler");

    connect(btnSave, &QPushButton::clicked, this, &MissionVerificationDialog::saveAndGeneratePdf);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);
}

void MissionVerificationDialog::saveAndGeneratePdf()
{
    QSqlQuery query;
    query.prepare("REPLACE INTO MissionsVerification (organisation_id, date_mission, equipe, gouvernance, rh, constats, conclusion) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(m_orgId);
    query.addBindValue(dateMissionEdit->date().toString("dd/MM/yyyy"));
    query.addBindValue(equipeEdit->text());
    query.addBindValue(gouvernanceCombo->currentText());
    query.addBindValue(rhCombo->currentText());
    query.addBindValue(constatsEdit->toPlainText());
    query.addBindValue(conclusionCombo->currentText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur BDD", query.lastError().text());
        return;
    }

    QString html = R"(
        <h3 align="center">RÉPUBLIQUE DÉMOCRATIQUE DU CONGO</h3>
        <p align="center"><b>PROVINCE DU KWILU</b></p>
        <h2 align="center">RAPPORT DE MISSION DE VÉRIFICATION SUR TERRAIN</h2>
        <hr>
        <p><b>Organisation :</b> %1</p>
        <p><b>Date de la mission :</b> %2</p>
        <p><b>Équipe chargée de l'inspection :</b> %3</p>
        <h3>1. Évaluation Structurale</h3>
        <ul>
            <li>Gouvernance : <b>%4</b></li>
            <li>Ressources Humaines : <b>%5</b></li>
        </ul>
        <h3>2. Constats</h3>
        <p>%6</p>
        <h3>3. Conclusion</h3>
        <p>Avis général : <b>%7</b></p>
        <br><br>
        <p align="right">Signatures de l'équipe</p>
    )";

    html = html.arg(m_orgName, dateMissionEdit->date().toString("dd/MM/yyyy"), equipeEdit->text(),
                    gouvernanceCombo->currentText(), rhCombo->currentText(), constatsEdit->toPlainText(),
                    conclusionCombo->currentText());

    QTextDocument document;
    document.setHtml(html);

    QString fileName = QFileDialog::getSaveFileName(this, "Sauvegarder", "Rapport_" + m_orgName.replace(" ", "_") + ".pdf", "*.pdf");
    if (!fileName.isEmpty()) {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        document.print(&printer);
        QMessageBox::information(this, "Succès", "Rapport sauvegardé avec succès.");
    }

    accept();
}
