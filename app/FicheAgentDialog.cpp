#include "FicheAgentDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include <QDebug>
#include <QScrollArea>
#include <QHeaderView>

FicheAgentDialog::FicheAgentDialog(QWidget *parent)
    : QDialog(parent), m_agentId(-1)
{
    setWindowTitle("Nouvel Agent - Fiche RH Complète");
    resize(680, 600);
    setupUi();
}

FicheAgentDialog::FicheAgentDialog(int agentId, QWidget *parent)
    : QDialog(parent), m_agentId(agentId)
{
    setWindowTitle("Édition Fiche Agent");
    resize(680, 600);
    setupUi();
    loadAgentData();
}

FicheAgentDialog::~FicheAgentDialog() {}

QWidget* FicheAgentDialog::createDynamicField(QComboBox *combo, const QString &category, const QString &labelTitle)
{
    QWidget *container = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    combo->setEditable(false); // Les champs D ne sont pas éditables manuellement, on choisit uniquement dans la liste RefOptions
    loadDynamicOptions(combo, category);

    // Bouton d'ajout +
    QPushButton *btnAdd = new QPushButton("+", container);
    btnAdd->setToolTip("Ajouter une nouvelle option à " + labelTitle);
    btnAdd->setFixedSize(28, 28);
    btnAdd->setCursor(Qt::PointingHandCursor);
    btnAdd->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; border-radius: 4px; font-weight: bold; font-size: 15px; border: none; }"
        "QPushButton:hover { background-color: #219150; }"
    );

    // Bouton de suppression -
    QPushButton *btnDel = new QPushButton("-", container);
    btnDel->setToolTip("Supprimer l'option sélectionnée de " + labelTitle);
    btnDel->setFixedSize(28, 28);
    btnDel->setCursor(Qt::PointingHandCursor);
    btnDel->setStyleSheet(
        "QPushButton { background-color: #e74c3c; color: white; border-radius: 4px; font-weight: bold; font-size: 15px; border: none; }"
        "QPushButton:hover { background-color: #c0392b; }"
    );

    connect(btnAdd, &QPushButton::clicked, this, [this, combo, category, labelTitle]() {
        addOptionInteractively(combo, category, labelTitle);
    });

    connect(btnDel, &QPushButton::clicked, this, [this, combo, category, labelTitle]() {
        removeOptionInteractively(combo, category, labelTitle);
    });

    layout->addWidget(combo, 1);
    layout->addWidget(btnAdd);
    layout->addWidget(btnDel);
    return container;
}

void FicheAgentDialog::loadDynamicOptions(QComboBox *combo, const QString &category)
{
    QString current = combo->currentText();
    combo->clear();

    QSqlQuery query;
    query.prepare("SELECT valeur FROM RefOptions WHERE categorie = ? ORDER BY valeur ASC");
    query.addBindValue(category);
    if (query.exec()) {
        while (query.next()) {
            combo->addItem(query.value(0).toString());
        }
    }
    if (!current.isEmpty()) {
        int idx = combo->findText(current);
        if (idx != -1) combo->setCurrentIndex(idx);
        else combo->setEditText(current);
    }
}

void FicheAgentDialog::addOptionInteractively(QComboBox *combo, const QString &category, const QString &labelTitle)
{
    bool ok = false;
    QString newOption = QInputDialog::getText(this, "Ajouter une option",
                                             QString("Nouvelle entrée pour '%1' :").arg(labelTitle),
                                             QLineEdit::Normal, "", &ok);
    newOption = newOption.trimmed();

    if (ok && !newOption.isEmpty()) {
        QSqlQuery query;
        query.prepare("INSERT OR IGNORE INTO RefOptions (categorie, valeur) VALUES (?, ?)");
        query.addBindValue(category);
        query.addBindValue(newOption);

        if (!query.exec()) {
            QMessageBox::warning(this, "Erreur", "Impossible d'ajouter l'option : " + query.lastError().text());
            return;
        }

        loadDynamicOptions(combo, category);
        int idx = combo->findText(newOption);
        if (idx != -1) {
            combo->setCurrentIndex(idx);
        } else {
            combo->setEditText(newOption);
        }
    }
}

void FicheAgentDialog::removeOptionInteractively(QComboBox *combo, const QString &category, const QString &labelTitle)
{
    QString selectedValue = combo->currentText().trimmed();
    if (selectedValue.isEmpty()) {
        QMessageBox::information(this, "Information", "Veuillez sélectionner une option à supprimer.");
        return;
    }

    auto reply = QMessageBox::question(this, "Confirmation de suppression",
                                       QString("Voulez-vous vraiment supprimer l'option '%1' de la liste des '%2' ?")
                                       .arg(selectedValue, labelTitle),
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM RefOptions WHERE categorie = ? AND valeur = ?");
        query.addBindValue(category);
        query.addBindValue(selectedValue);

        if (!query.exec()) {
            QMessageBox::critical(this, "Erreur", "Impossible de supprimer l'option : " + query.lastError().text());
            return;
        }

        loadDynamicOptions(combo, category);
        if (combo->count() > 0) {
            combo->setCurrentIndex(0);
        } else {
            combo->clearEditText();
        }
    }
}

void FicheAgentDialog::setupUi()
{
    setStyleSheet(R"(
        QDialog { background-color: #f8f9fa; }
        QTabWidget::pane { border: 1px solid #dcdfe6; background: white; border-radius: 6px; padding: 10px; }
        QTabBar::tab { background: #e9ecef; color: #495057; padding: 10px 16px; margin-right: 2px; border-top-left-radius: 4px; border-top-right-radius: 4px; font-weight: bold; }
        QTabBar::tab:selected { background: #2980b9; color: white; }
        QLineEdit, QComboBox, QDateEdit { padding: 6px 10px; border: 1px solid #ced4da; border-radius: 4px; background-color: #ffffff; font-size: 13px; }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border: 1px solid #2980b9; background-color: #f0f7fc; }
        QLineEdit:read-only { background-color: #e9ecef; color: #6c757d; font-weight: bold; }
        QLabel { font-weight: 600; color: #2c3e50; font-size: 13px; }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    QTabWidget *tabWidget = new QTabWidget(this);

    // ==========================================
    // TAB 1 : Infos Personnelles & Contact
    // ==========================================
    QWidget *tab1 = new QWidget();
    QFormLayout *form1 = new QFormLayout(tab1);
    form1->setSpacing(10);

    nomEdit = new QLineEdit();
    postnomEdit = new QLineEdit();
    prenomEdit = new QLineEdit();

    sexeCombo = new QComboBox();
    sexeCombo->addItems({"Féminin", "Masculin"});

    etatCivilCombo = new QComboBox();
    etatCivilCombo->addItems({"Célibataire", "Marié(e)", "Divorcé(e)", "Veuf(ve)"});

    dateNaissanceEdit = new QDateEdit(QDate::currentDate().addYears(-30));
    dateNaissanceEdit->setCalendarPopup(true);
    dateNaissanceEdit->setDisplayFormat("dd/MM/yyyy");

    telephoneEdit = new QLineEdit();
    telephoneEdit->setPlaceholderText("+243 ...");

    emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText("agent@fonctionpublique.gouv.cd");

    adresseEdit = new QLineEdit();

    form1->addRow("Nom (M) * :", nomEdit);
    form1->addRow("Post-nom (M) :", postnomEdit);
    form1->addRow("Prénom (M) :", prenomEdit);
    form1->addRow("Sexe (O) :", sexeCombo);
    form1->addRow("État civil (O) :", etatCivilCombo);
    form1->addRow("Date de naissance (M) :", dateNaissanceEdit);
    form1->addRow("N° Téléphone (M) :", telephoneEdit);
    form1->addRow("Adresse Email (M) :", emailEdit);
    form1->addRow("Adresse (M) :", adresseEdit);

    tabWidget->addTab(tab1, style()->standardIcon(QStyle::SP_FileIcon), "Infos Personnelles");

    // ==========================================
    // TAB 2 : Affectation & Structure Administrative
    // ==========================================
    QWidget *tab2 = new QWidget();
    QFormLayout *form2 = new QFormLayout(tab2);
    form2->setSpacing(10);

    ministereCombo = new QComboBox();
    secretariatGeneralCombo = new QComboBox();
    directionCombo = new QComboBox();
    divisionCombo = new QComboBox();
    bureauCombo = new QComboBox();
    serviceCombo = new QComboBox();
    serviceCombo->addItems({"Secrétariat", "Informatique", "Ressources Humaines", "Logistique", "Comptabilité", "Inspection", "Études et Planification"});

    niveauAffectationEdit = new QLineEdit("Provincial");
    niveauAffectationEdit->setReadOnly(true);

    provinceEdit = new QLineEdit("Kwilu");
    provinceEdit->setReadOnly(true);

    lieuAffectationCombo = new QComboBox();

    form2->addRow("Ministère (D) :", createDynamicField(ministereCombo, "ministere", "Ministère"));
    form2->addRow("Secrétariat Général (D) :", createDynamicField(secretariatGeneralCombo, "secretariat_general", "Secrétariat Général"));
    form2->addRow("Direction (D) :", createDynamicField(directionCombo, "direction", "Direction"));
    form2->addRow("Division (D) :", createDynamicField(divisionCombo, "division", "Division"));
    form2->addRow("Bureau (D) :", createDynamicField(bureauCombo, "bureau", "Bureau"));
    form2->addRow("Service :", serviceCombo);
    form2->addRow("Niveau d'affectation (C) :", niveauAffectationEdit);
    form2->addRow("Province (C) :", provinceEdit);
    form2->addRow("Lieu d'affectation (D) :", createDynamicField(lieuAffectationCombo, "lieu_affectation", "Lieu d'affectation"));

    tabWidget->addTab(tab2, style()->standardIcon(QStyle::SP_DirIcon), "Affectation & Structure");

    // ==========================================
    // TAB 3 : Carrière & Actes Administratifs
    // ==========================================
    QWidget *tab3 = new QWidget();
    QFormLayout *form3 = new QFormLayout(tab3);
    form3->setSpacing(10);

    matriculeEdit = new QLineEdit();
    domaineEdit = new QLineEdit("Cadre Général");
    domaineEdit->setReadOnly(true);

    gradeEdit = new QLineEdit();
    fonctionCombo = new QComboBox();

    dateEngagementEdit = new QDateEdit(QDate::currentDate());
    dateEngagementEdit->setCalendarPopup(true);
    dateEngagementEdit->setDisplayFormat("dd/MM/yyyy");

    positionAdminCombo = new QComboBox();
    statutRecensementCombo = new QComboBox();
    statutRecensementCombo->addItems({"Oui", "Non"});

    refEngagementEdit = new QLineEdit();
    refNominationEdit = new QLineEdit();
    refCommissionnementEdit = new QLineEdit();

    form3->addRow("Matricule (M) * :", matriculeEdit);
    form3->addRow("Domaine (C) :", domaineEdit);
    form3->addRow("Grade statutaire (M) :", gradeEdit);
    form3->addRow("Fonction (D) :", createDynamicField(fonctionCombo, "fonction", "Fonction"));
    form3->addRow("Date d'engagement (M) :", dateEngagementEdit);
    form3->addRow("Position administrative (D) :", createDynamicField(positionAdminCombo, "position_admin", "Position administrative"));
    form3->addRow("Statut recensement (O) :", statutRecensementCombo);
    form3->addRow("Réf. acte d'engagement (M) :", refEngagementEdit);
    form3->addRow("Réf. acte de nomination (M) :", refNominationEdit);
    form3->addRow("Réf. commissionnement (M) :", refCommissionnementEdit);

    tabWidget->addTab(tab3, style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Carrière & Actes");

    // ==========================================
    // TAB 4 : Formation & Rémunération
    // ==========================================
    QWidget *tab4 = new QWidget();
    QFormLayout *form4 = new QFormLayout(tab4);
    form4->setSpacing(12);

    niveauEtudeCombo = new QComboBox();
    filiereCombo = new QComboBox();

    salaireCombo = new QComboBox();
    salaireCombo->addItems({"Oui", "Non"});

    primeCombo = new QComboBox();
    primeCombo->addItems({"Oui", "Non"});

    form4->addRow("Niveau d'étude (D) :", createDynamicField(niveauEtudeCombo, "niveau_etude", "Niveau d'étude"));
    form4->addRow("Filière d'études (D) :", createDynamicField(filiereCombo, "filiere", "Filière d'études"));
    form4->addRow("Salaire perçu (O) :", salaireCombo);
    form4->addRow("Prime perçue (O) :", primeCombo);

    tabWidget->addTab(tab4, style()->standardIcon(QStyle::SP_DialogApplyButton), "Formation & Solde");

    // ==========================================
    // TAB 5 : Contrôle, Biométrie & Congés
    // ==========================================
    QWidget *tab5 = new QWidget();
    QVBoxLayout *layout5 = new QVBoxLayout(tab5);
    layout5->setSpacing(10);

    QFormLayout *form5 = new QFormLayout();
    form5->setSpacing(10);

    carteBiometriqueCombo = new QComboBox();
    carteBiometriqueCombo->addItems({"Non", "Oui"});

    dossierPhysiqueCombo = new QComboBox();
    dossierPhysiqueCombo->addItems({"En ordre", "Incomplet", "Non fourni"});

    statutCarriereCombo = new QComboBox();
    statutCarriereCombo->addItems({"En activité", "Déserteur", "Décédé", "Retraité"});

    form5->addRow("Carte Biométrique (O) :", carteBiometriqueCombo);
    form5->addRow("Dossier Physique (O) :", dossierPhysiqueCombo);
    form5->addRow("Statut Carrière / Désertion (O) :", statutCarriereCombo);

    layout5->addLayout(form5);

    // Section Suivi des Congés
    QLabel *lblCongesTitle = new QLabel("<b>Historique & Prise de Congés / Absences</b>");
    layout5->addWidget(lblCongesTitle);

    QHBoxLayout *congeFormLayout = new QHBoxLayout();
    typeCongeCombo = new QComboBox();
    typeCongeCombo->addItems({"Congé Annuel", "Congé de Maladie", "Congé de Maternité", "Absence Autorisée", "Autre"});

    dateDebutCongeEdit = new QDateEdit(QDate::currentDate());
    dateDebutCongeEdit->setCalendarPopup(true);

    dateFinCongeEdit = new QDateEdit(QDate::currentDate().addDays(7));
    dateFinCongeEdit->setCalendarPopup(true);

    motifCongeEdit = new QLineEdit();
    motifCongeEdit->setPlaceholderText("Motif du congé...");

    QPushButton *btnAddConge = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Enregistrer Congé", this);
    btnAddConge->setStyleSheet("background-color: #2980b9; color: white; border-radius: 4px; padding: 6px 12px; font-weight: bold;");
    connect(btnAddConge, &QPushButton::clicked, this, &FicheAgentDialog::ajouterConge);

    congeFormLayout->addWidget(new QLabel("Type:"));
    congeFormLayout->addWidget(typeCongeCombo);
    congeFormLayout->addWidget(new QLabel("Début:"));
    congeFormLayout->addWidget(dateDebutCongeEdit);
    congeFormLayout->addWidget(new QLabel("Fin:"));
    congeFormLayout->addWidget(dateFinCongeEdit);
    congeFormLayout->addWidget(motifCongeEdit, 1);
    congeFormLayout->addWidget(btnAddConge);

    layout5->addLayout(congeFormLayout);

    congesTable = new QTableWidget();
    congesTable->setColumnCount(5);
    congesTable->setHorizontalHeaderLabels({"Type", "Début", "Fin", "Durée (j)", "Motif"});
    congesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    congesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    congesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout5->addWidget(congesTable, 1);

    tabWidget->addTab(tab5, style()->standardIcon(QStyle::SP_FileDialogContentsView), "Contrôle & Congés");

    mainLayout->addWidget(tabWidget);

    // Boutons d'action professionnels
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnSave = new QPushButton(style()->standardIcon(QStyle::SP_DialogSaveButton), "Enregistrer la fiche", this);
    btnSave->setCursor(Qt::PointingHandCursor);
    btnSave->setStyleSheet("padding: 10px 20px; background-color: #27ae60; color: white; border: none; border-radius: 4px; font-weight: bold;");

    QPushButton *btnCancel = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "Annuler", this);
    btnCancel->setCursor(Qt::PointingHandCursor);
    btnCancel->setStyleSheet("padding: 10px 20px; background-color: #7f8c8d; color: white; border: none; border-radius: 4px;");

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
    query.prepare("SELECT matricule, nom, postnom, prenom, sexe, etat_civil, date_naissance, telephone, email, adresse, "
                  "ministere, secretariat_general, direction, division, bureau, service, niveau_affectation, province, lieu_affectation, "
                  "grade, fonction, domaine, date_engagement, position_admin, statut_recensement, ref_engagement, ref_nomination, ref_commissionnement, "
                  "niveau_etude, filiere, salaire, primes, carte_biometrique, dossier_physique, statut_carriere FROM Agents WHERE id = ?");
    query.addBindValue(m_agentId);

    if (query.exec() && query.next()) {
        matriculeEdit->setText(query.value(0).toString());
        nomEdit->setText(query.value(1).toString());
        postnomEdit->setText(query.value(2).toString());
        prenomEdit->setText(query.value(3).toString());
        sexeCombo->setCurrentText(query.value(4).toString());
        etatCivilCombo->setCurrentText(query.value(5).toString());

        QDate dn = QDate::fromString(query.value(6).toString(), "dd/MM/yyyy");
        if (dn.isValid()) dateNaissanceEdit->setDate(dn);

        telephoneEdit->setText(query.value(7).toString());
        emailEdit->setText(query.value(8).toString());
        adresseEdit->setText(query.value(9).toString());

        ministereCombo->setEditText(query.value(10).toString());
        secretariatGeneralCombo->setEditText(query.value(11).toString());
        directionCombo->setEditText(query.value(12).toString());
        divisionCombo->setEditText(query.value(13).toString());
        bureauCombo->setEditText(query.value(14).toString());
        serviceCombo->setCurrentText(query.value(15).toString());
        niveauAffectationEdit->setText(query.value(16).toString().isEmpty() ? "Provincial" : query.value(16).toString());
        provinceEdit->setText(query.value(17).toString().isEmpty() ? "Kwilu" : query.value(17).toString());
        lieuAffectationCombo->setEditText(query.value(18).toString());

        gradeEdit->setText(query.value(19).toString());
        fonctionCombo->setEditText(query.value(20).toString());
        domaineEdit->setText(query.value(21).toString().isEmpty() ? "Cadre Général" : query.value(21).toString());

        QDate de = QDate::fromString(query.value(22).toString(), "dd/MM/yyyy");
        if (de.isValid()) dateEngagementEdit->setDate(de);

        positionAdminCombo->setEditText(query.value(23).toString());
        statutRecensementCombo->setCurrentText(query.value(24).toString());
        refEngagementEdit->setText(query.value(25).toString());
        refNominationEdit->setText(query.value(26).toString());
        refCommissionnementEdit->setText(query.value(27).toString());

        niveauEtudeCombo->setEditText(query.value(28).toString());
        filiereCombo->setEditText(query.value(29).toString());
        salaireCombo->setCurrentText(query.value(30).toString());
        primeCombo->setCurrentText(query.value(31).toString());

        carteBiometriqueCombo->setCurrentText(query.value(32).toString().isEmpty() ? "Non" : query.value(32).toString());
        dossierPhysiqueCombo->setCurrentText(query.value(33).toString().isEmpty() ? "En ordre" : query.value(33).toString());
        statutCarriereCombo->setCurrentText(query.value(34).toString().isEmpty() ? "En activité" : query.value(34).toString());
    }

    loadCongesData();
}

void FicheAgentDialog::loadCongesData()
{
    congesTable->setRowCount(0);
    if (m_agentId == -1) return;

    QSqlQuery q;
    q.prepare("SELECT type_conge, date_debut, date_fin, duree_jours, motif FROM Conges WHERE agent_id = ? ORDER BY id DESC");
    q.addBindValue(m_agentId);
    if (q.exec()) {
        int r = 0;
        while (q.next()) {
            congesTable->insertRow(r);
            congesTable->setItem(r, 0, new QTableWidgetItem(q.value(0).toString()));
            congesTable->setItem(r, 1, new QTableWidgetItem(q.value(1).toString()));
            congesTable->setItem(r, 2, new QTableWidgetItem(q.value(2).toString()));
            congesTable->setItem(r, 3, new QTableWidgetItem(q.value(3).toString()));
            congesTable->setItem(r, 4, new QTableWidgetItem(q.value(4).toString()));
            r++;
        }
    }
}

void FicheAgentDialog::ajouterConge()
{
    if (m_agentId == -1) {
        QMessageBox::information(this, "Enregistrement requis", "Veuillez enregistrer la fiche de l'agent avant de lui ajouter un congé.");
        return;
    }

    int duree = dateDebutCongeEdit->date().daysTo(dateFinCongeEdit->date());
    if (duree <= 0) {
        QMessageBox::warning(this, "Erreur Date", "La date de fin doit être postérieure à la date de début.");
        return;
    }

    QString moisAnnee = dateDebutCongeEdit->date().toString("MM/yyyy");

    QSqlQuery q;
    q.prepare("INSERT INTO Conges (agent_id, type_conge, date_debut, date_fin, duree_jours, mois_annee, motif) VALUES (?, ?, ?, ?, ?, ?, ?)");
    q.addBindValue(m_agentId);
    q.addBindValue(typeCongeCombo->currentText());
    q.addBindValue(dateDebutCongeEdit->date().toString("dd/MM/yyyy"));
    q.addBindValue(dateFinCongeEdit->date().toString("dd/MM/yyyy"));
    q.addBindValue(duree);
    q.addBindValue(moisAnnee);
    q.addBindValue(motifCongeEdit->text().trimmed());

    if (q.exec()) {
        loadCongesData();
        motifCongeEdit->clear();
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible d'enregistrer le congé : " + q.lastError().text());
    }
}

void FicheAgentDialog::saveToDatabase()
{
    if (matriculeEdit->text().trimmed().isEmpty() || nomEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Champs Obligatoires", "Le matricule et le nom sont obligatoires.");
        return;
    }

    QSqlQuery query;
    if (m_agentId == -1) {
        query.prepare("INSERT INTO Agents (matricule, nom, postnom, prenom, sexe, etat_civil, date_naissance, telephone, email, adresse, "
                      "ministere, secretariat_general, direction, division, bureau, service, niveau_affectation, province, lieu_affectation, "
                      "grade, fonction, domaine, date_engagement, position_admin, statut_recensement, ref_engagement, ref_nomination, ref_commissionnement, "
                      "niveau_etude, filiere, salaire, primes, carte_biometrique, dossier_physique, statut_carriere) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    } else {
        query.prepare("UPDATE Agents SET matricule=?, nom=?, postnom=?, prenom=?, sexe=?, etat_civil=?, date_naissance=?, telephone=?, email=?, adresse=?, "
                      "ministere=?, secretariat_general=?, direction=?, division=?, bureau=?, service=?, niveau_affectation=?, province=?, lieu_affectation=?, "
                      "grade=?, fonction=?, domaine=?, date_engagement=?, position_admin=?, statut_recensement=?, ref_engagement=?, ref_nomination=?, ref_commissionnement=?, "
                      "niveau_etude=?, filiere=?, salaire=?, primes=?, carte_biometrique=?, dossier_physique=?, statut_carriere=? WHERE id=?");
    }

    query.addBindValue(matriculeEdit->text().trimmed());
    query.addBindValue(nomEdit->text().trimmed());
    query.addBindValue(postnomEdit->text().trimmed());
    query.addBindValue(prenomEdit->text().trimmed());
    query.addBindValue(sexeCombo->currentText());
    query.addBindValue(etatCivilCombo->currentText());
    query.addBindValue(dateNaissanceEdit->date().toString("dd/MM/yyyy"));
    query.addBindValue(telephoneEdit->text().trimmed());
    query.addBindValue(emailEdit->text().trimmed());
    query.addBindValue(adresseEdit->text().trimmed());

    query.addBindValue(ministereCombo->currentText().trimmed());
    query.addBindValue(secretariatGeneralCombo->currentText().trimmed());
    query.addBindValue(directionCombo->currentText().trimmed());
    query.addBindValue(divisionCombo->currentText().trimmed());
    query.addBindValue(bureauCombo->currentText().trimmed());
    query.addBindValue(serviceCombo->currentText());
    query.addBindValue(niveauAffectationEdit->text().trimmed());
    query.addBindValue(provinceEdit->text().trimmed());
    query.addBindValue(lieuAffectationCombo->currentText().trimmed());

    query.addBindValue(gradeEdit->text().trimmed());
    query.addBindValue(fonctionCombo->currentText().trimmed());
    query.addBindValue(domaineEdit->text().trimmed());
    query.addBindValue(dateEngagementEdit->date().toString("dd/MM/yyyy"));
    query.addBindValue(positionAdminCombo->currentText().trimmed());
    query.addBindValue(statutRecensementCombo->currentText());
    query.addBindValue(refEngagementEdit->text().trimmed());
    query.addBindValue(refNominationEdit->text().trimmed());
    query.addBindValue(refCommissionnementEdit->text().trimmed());

    query.addBindValue(niveauEtudeCombo->currentText().trimmed());
    query.addBindValue(filiereCombo->currentText().trimmed());
    query.addBindValue(salaireCombo->currentText());
    query.addBindValue(primeCombo->currentText());

    query.addBindValue(carteBiometriqueCombo->currentText());
    query.addBindValue(dossierPhysiqueCombo->currentText());
    query.addBindValue(statutCarriereCombo->currentText());

    if (m_agentId != -1) {
        query.addBindValue(m_agentId);
    }

    if (!query.exec()) {
        QMessageBox::critical(this, "Erreur BDD", "Impossible d'enregistrer la fiche agent :\n" + query.lastError().text());
        return;
    }

    accept();
}
