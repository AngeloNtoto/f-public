#include "FicheAgentDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

FicheAgentDialog::FicheAgentDialog(QWidget *parent)
    : QDialog(parent), m_agentId(-1)
{
    setWindowTitle("Nouvel Agent");
    resize(400, 500);
    setupUi();
}

FicheAgentDialog::FicheAgentDialog(int agentId, QWidget *parent)
    : QDialog(parent), m_agentId(agentId)
{
    setWindowTitle("Éditer la Fiche Agent");
    resize(400, 500);
    setupUi();
    loadAgentData();
}

FicheAgentDialog::~FicheAgentDialog() {}

void FicheAgentDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    matriculeEdit = new QLineEdit();
    nomEdit = new QLineEdit();
    postnomEdit = new QLineEdit();
    prenomEdit = new QLineEdit();
    
    sexeCombo = new QComboBox();
    sexeCombo->addItems({"Homme", "Femme"});
    
    etatCivilCombo = new QComboBox();
    etatCivilCombo->addItems({"Célibataire", "Marié(e)", "Veuf(ve)", "Divorcé(e)"});
    
    gradeEdit = new QLineEdit();
    fonctionEdit = new QLineEdit();
    
    serviceCombo = new QComboBox();
    serviceCombo->addItems({"Secrétariat", "Informatique", "Ressources Humaines", "Logistique", "Comptabilité", "Inspection"});
    
    ministereEdit = new QLineEdit();
    ministereEdit->setText("Plan et Coordination de l'Aide au Développement");
    
    directionEdit = new QLineEdit();
    
    dateEngagementEdit = new QDateEdit(QDate::currentDate());
    dateEngagementEdit->setCalendarPopup(true);
    
    salaireEdit = new QLineEdit();
    primesEdit = new QLineEdit();
    telephoneEdit = new QLineEdit();
    adresseEdit = new QLineEdit();

    formLayout->addRow("Matricule :", matriculeEdit);
    formLayout->addRow("Nom :", nomEdit);
    formLayout->addRow("Post-nom :", postnomEdit);
    formLayout->addRow("Prénom :", prenomEdit);
    formLayout->addRow("Sexe :", sexeCombo);
    formLayout->addRow("État civil :", etatCivilCombo);
    formLayout->addRow("Téléphone :", telephoneEdit);
    formLayout->addRow("Adresse :", adresseEdit);
    formLayout->addRow("Grade :", gradeEdit);
    formLayout->addRow("Fonction :", fonctionEdit);
    formLayout->addRow("Service :", serviceCombo);
    formLayout->addRow("Ministère :", ministereEdit);
    formLayout->addRow("Direction :", directionEdit);
    formLayout->addRow("Date d'engagement :", dateEngagementEdit);
    formLayout->addRow("Salaire de base ($) :", salaireEdit);
    formLayout->addRow("Primes ($) :", primesEdit);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton("Enregistrer", this);
    btnSave->setStyleSheet("padding: 8px; background-color: #27ae60; color: white; border-radius: 4px;");
    QPushButton *btnCancel = new QPushButton("Annuler", this);
    
    connect(btnSave, &QPushButton::clicked, this, &FicheAgentDialog::saveToDatabase);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);
}

void FicheAgentDialog::loadAgentData()
{
    if (m_agentId == -1) return;

    QSqlQuery query;
    query.prepare("SELECT matricule, nom, postnom, grade, fonction, service, ministere, direction, date_engagement, salaire, prenom, sexe, etat_civil, telephone, adresse, primes FROM Agents WHERE id = ?");
    query.addBindValue(m_agentId);
    if(query.exec() && query.next()) {
        matriculeEdit->setText(query.value(0).toString());
        nomEdit->setText(query.value(1).toString());
        postnomEdit->setText(query.value(2).toString());
        gradeEdit->setText(query.value(3).toString());
        fonctionEdit->setText(query.value(4).toString());
        serviceCombo->setCurrentText(query.value(5).toString());
        ministereEdit->setText(query.value(6).toString());
        directionEdit->setText(query.value(7).toString());
        
        QDate dt = QDate::fromString(query.value(8).toString(), "dd/MM/yyyy");
        if(dt.isValid()) dateEngagementEdit->setDate(dt);
        
        salaireEdit->setText(query.value(9).toString());
        prenomEdit->setText(query.value(10).toString());
        sexeCombo->setCurrentText(query.value(11).toString());
        etatCivilCombo->setCurrentText(query.value(12).toString());
        telephoneEdit->setText(query.value(13).toString());
        adresseEdit->setText(query.value(14).toString());
        primesEdit->setText(query.value(15).toString());
    }
}

void FicheAgentDialog::saveToDatabase()
{
    if (matriculeEdit->text().isEmpty() || nomEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Le matricule et le nom sont obligatoires.");
        return;
    }

    QSqlQuery query;
    if (m_agentId == -1) {
        query.prepare("INSERT INTO Agents (matricule, nom, postnom, grade, fonction, service, ministere, direction, date_engagement, salaire, prenom, sexe, etat_civil, telephone, adresse, primes) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    } else {
        query.prepare("UPDATE Agents SET matricule=?, nom=?, postnom=?, grade=?, fonction=?, service=?, ministere=?, direction=?, date_engagement=?, salaire=?, prenom=?, sexe=?, etat_civil=?, telephone=?, adresse=?, primes=? "
                      "WHERE id=?");
    }
    
    query.addBindValue(matriculeEdit->text());
    query.addBindValue(nomEdit->text());
    query.addBindValue(postnomEdit->text());
    query.addBindValue(gradeEdit->text());
    query.addBindValue(fonctionEdit->text());
    query.addBindValue(serviceCombo->currentText());
    query.addBindValue(ministereEdit->text());
    query.addBindValue(directionEdit->text());
    query.addBindValue(dateEngagementEdit->date().toString("dd/MM/yyyy"));
    query.addBindValue(salaireEdit->text());
    query.addBindValue(prenomEdit->text());
    query.addBindValue(sexeCombo->currentText());
    query.addBindValue(etatCivilCombo->currentText());
    query.addBindValue(telephoneEdit->text());
    query.addBindValue(adresseEdit->text());
    query.addBindValue(primesEdit->text());
    
    if(m_agentId != -1) {
        query.addBindValue(m_agentId);
    }

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur BDD", "Impossible d'enregistrer la fiche agent:\n" + query.lastError().text());
        return;
    }

    accept();
}
