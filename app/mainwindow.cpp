#include "mainwindow.hpp"
#include "AutorisationSortieDialog.hpp"
#include "IdentificationDialog.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QDebug>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlRelation>
#include <QTabWidget>
#include <QDateEdit>
#include <QComboBox>
#include <QTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Fonction Publique - Province du Kwilu");
    resize(1024, 768);
    setupUi();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Sidebar
    sidebar = new QListWidget(this);
    sidebar->setFixedWidth(250);
    sidebar->setStyleSheet(
        "QListWidget { background-color: #2c3e50; color: white; border: none; font-size: 16px; }"
        "QListWidget::item { padding: 15px; border-bottom: 1px solid #34495e; }"
        "QListWidget::item:selected { background-color: #2980b9; }"
    );

    QStringList modules = {"Tableau de Bord", "Ressources Humaines", "Secteurs Sociaux", "Secteurs Productifs", "Infrastructures", "Secrétariat"};
    sidebar->addItems(modules);

    // Stacked Widget
    stackedWidget = new QStackedWidget(this);
    
    stackedWidget->addWidget(createDashboardPage());
    stackedWidget->addWidget(createRhPage());
    stackedWidget->addWidget(createSecteursSociauxPage());
    
    // Placeholders pour les pages non encore détaillées
    QWidget *prodPage = new QWidget();
    QVBoxLayout *prodLayout = new QVBoxLayout(prodPage);
    prodLayout->addWidget(new QLabel("<h2>Secteurs Productifs (En construction)</h2>"));
    stackedWidget->addWidget(prodPage);
    
    QWidget *infraPage = new QWidget();
    QVBoxLayout *infraLayout = new QVBoxLayout(infraPage);
    infraLayout->addWidget(new QLabel("<h2>Infrastructures (En construction)</h2>"));
    stackedWidget->addWidget(infraPage);
    
    stackedWidget->addWidget(createSecretariatPage());

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(stackedWidget);

    connect(sidebar, &QListWidget::currentRowChanged, this, &MainWindow::changePage);
    
    // Sélection par défaut
    sidebar->setCurrentRow(0);
}

void MainWindow::changePage(int index)
{
    stackedWidget->setCurrentIndex(index);
}

QWidget* MainWindow::createDashboardPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *title = new QLabel("<h2>Tableau de Bord Global</h2>", page);
    layout->addWidget(title);
    layout->addWidget(new QLabel("Bienvenue dans le système de gestion de la Fonction Publique du Kwilu.", page));
    layout->addStretch();
    return page;
}

QWidget* MainWindow::createRhPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *title = new QLabel("<h2>Ressources Humaines - Liste des Agents</h2>", page);
    layout->addWidget(title);

    // Initialisation du modèle SQL
    agentModel = new QSqlTableModel(this);
    agentModel->setTable("Agents");
    agentModel->select();
    
    // Personnalisation des en-têtes
    agentModel->setHeaderData(0, Qt::Horizontal, "ID");
    agentModel->setHeaderData(1, Qt::Horizontal, "Matricule");
    agentModel->setHeaderData(2, Qt::Horizontal, "Nom");
    agentModel->setHeaderData(3, Qt::Horizontal, "Postnom");
    agentModel->setHeaderData(4, Qt::Horizontal, "Grade");
    agentModel->setHeaderData(5, Qt::Horizontal, "Fonction");
    agentModel->setHeaderData(6, Qt::Horizontal, "Service");

    QTableView *tableView = new QTableView(page);
    tableView->setModel(agentModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::DoubleClicked); // Edition directe
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    layout->addWidget(tableView);

    // Boutons d'action
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("Ajouter un agent", page);
    QPushButton *btnDelete = new QPushButton("Supprimer l'agent", page);
    
    btnAdd->setStyleSheet("padding: 8px; background-color: #27ae60; color: white; border-radius: 4px;");
    btnDelete->setStyleSheet("padding: 8px; background-color: #e74c3c; color: white; border-radius: 4px;");
    
    connect(btnAdd, &QPushButton::clicked, [this]() {
        int row = agentModel->rowCount();
        agentModel->insertRow(row);
    });

    connect(btnDelete, &QPushButton::clicked, [this, tableView]() {
        int row = tableView->currentIndex().row();
        if(row >= 0) {
            agentModel->removeRow(row);
            agentModel->submitAll();
            agentModel->select(); // Refresh
        }
    });

    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnDelete);
    btnLayout->addStretch();
    
    layout->addLayout(btnLayout);
    
    return page;
}

QWidget* MainWindow::createSecteursSociauxPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    
    layout->addWidget(new QLabel("<h2>Secteurs Sociaux - ONGD & ASBL</h2>", page));
    
    QPushButton *btnNewIdent = new QPushButton("Nouvelle Demande d'Identification", page);
    btnNewIdent->setStyleSheet("padding: 10px; background-color: #f39c12; color: white; border-radius: 4px;");
    
    connect(btnNewIdent, &QPushButton::clicked, this, [this]() {
        IdentificationDialog dialog(this);
        dialog.exec();
    });
    
    layout->addWidget(btnNewIdent);
    layout->addStretch();
    return page;
}

QWidget* MainWindow::createSecretariatPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    
    layout->addWidget(new QLabel("<h2>Secrétariat</h2>", page));
    
    QTabWidget *tabWidget = new QTabWidget(page);
    
    // Onglet Autorisations
    QWidget *tabAuto = new QWidget();
    QVBoxLayout *layoutAuto = new QVBoxLayout(tabAuto);
    QPushButton *btnAutoSortie = new QPushButton("Générer une Autorisation de Sortie", tabAuto);
    btnAutoSortie->setStyleSheet("padding: 10px; background-color: #8e44ad; color: white; border-radius: 4px;");
    connect(btnAutoSortie, &QPushButton::clicked, this, [this]() {
        AutorisationSortieDialog dialog(this);
        dialog.exec();
    });
    layoutAuto->addWidget(btnAutoSortie);
    layoutAuto->addStretch();
    tabWidget->addTab(tabAuto, "Autorisations de Sortie");
    
    // Onglet Présences
    QWidget *tabPresence = new QWidget();
    QVBoxLayout *layoutPresence = new QVBoxLayout(tabPresence);
    
    // Formulaire Pointage Rapide
    QHBoxLayout *pointageLayout = new QHBoxLayout();
    
    datePresenceEdit = new QDateEdit(QDate::currentDate(), tabPresence);
    datePresenceEdit->setCalendarPopup(true);
    
    agentPresenceCombo = new QComboBox(tabPresence);
    loadPresenceAgents(); // Remplit le combo
    
    QPushButton *btnArrivee = new QPushButton("Pointer Arrivée", tabPresence);
    QPushButton *btnDepart = new QPushButton("Pointer Départ", tabPresence);
    
    btnArrivee->setStyleSheet("padding: 8px; background-color: #27ae60; color: white; border-radius: 4px;");
    btnDepart->setStyleSheet("padding: 8px; background-color: #e67e22; color: white; border-radius: 4px;");
    
    connect(btnArrivee, &QPushButton::clicked, this, &MainWindow::pointerArrivee);
    connect(btnDepart, &QPushButton::clicked, this, &MainWindow::pointerDepart);
    
    pointageLayout->addWidget(new QLabel("Date :"));
    pointageLayout->addWidget(datePresenceEdit);
    pointageLayout->addWidget(new QLabel("Agent :"));
    pointageLayout->addWidget(agentPresenceCombo);
    pointageLayout->addWidget(btnArrivee);
    pointageLayout->addWidget(btnDepart);
    pointageLayout->addStretch();
    
    layoutPresence->addLayout(pointageLayout);
    
    // Modèle SQL pour voir les présences
    presencesModel = new QSqlRelationalTableModel(this);
    presencesModel->setTable("Presences");
    presencesModel->setRelation(1, QSqlRelation("Agents", "id", "nom")); // Colonne agent_id
    // Filtre sur la date du jour par défaut
    presencesModel->setFilter(QString("date = '%1'").arg(QDate::currentDate().toString("dd/MM/yyyy")));
    presencesModel->select();
    
    presencesModel->setHeaderData(1, Qt::Horizontal, "Agent");
    presencesModel->setHeaderData(2, Qt::Horizontal, "Date");
    presencesModel->setHeaderData(3, Qt::Horizontal, "Heure Arrivée");
    presencesModel->setHeaderData(4, Qt::Horizontal, "Heure Départ");
    
    QTableView *tableView = new QTableView(tabPresence);
    tableView->setModel(presencesModel);
    tableView->setItemDelegate(new QSqlRelationalDelegate(tableView));
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // Rafraîchir le tableau si on change la date
    connect(datePresenceEdit, &QDateEdit::dateChanged, [this](const QDate &date) {
        presencesModel->setFilter(QString("date = '%1'").arg(date.toString("dd/MM/yyyy")));
        presencesModel->select();
    });
    
    layoutPresence->addWidget(tableView);
    tabWidget->addTab(tabPresence, "Gestion des Présences");
    
    layout->addWidget(tabWidget);
    return page;
}

void MainWindow::loadPresenceAgents()
{
    agentPresenceCombo->clear();
    QSqlQuery query("SELECT id, nom, postnom, matricule FROM Agents");
    while (query.next()) {
        int id = query.value(0).toInt();
        QString nom = query.value(1).toString();
        QString postnom = query.value(2).toString();
        QString matricule = query.value(3).toString();
        
        QString displayText = QString("%1 %2 (%3)").arg(nom, postnom, matricule);
        agentPresenceCombo->addItem(displayText, id);
    }
}

void MainWindow::pointerArrivee()
{
    int agentId = agentPresenceCombo->currentData().toInt();
    if(agentId == 0) {
        QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un agent.");
        return;
    }
    
    QString dateStr = datePresenceEdit->date().toString("dd/MM/yyyy");
    
    // Vérifier si l'agent a déjà pointé aujourd'hui
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM Presences WHERE agent_id = :agent_id AND date = :date");
    checkQuery.bindValue(":agent_id", agentId);
    checkQuery.bindValue(":date", dateStr);
    checkQuery.exec();
    
    if (checkQuery.next()) {
        QMessageBox::information(this, "Info", "Cet agent a déjà une ligne de présence pour ce jour.");
        return;
    }
    
    QString heureActuelle = QTime::currentTime().toString("HH:mm");
    
    QSqlQuery query;
    query.prepare("INSERT INTO Presences (agent_id, date, heure_arrivee, heure_depart) "
                  "VALUES (:agent_id, :date, :heure_arrivee, '')");
    query.bindValue(":agent_id", agentId);
    query.bindValue(":date", dateStr);
    query.bindValue(":heure_arrivee", heureActuelle);
    
    if(!query.exec()) {
        QMessageBox::critical(this, "Erreur", query.lastError().text());
    } else {
        presencesModel->select(); // Refresh table
    }
}

void MainWindow::pointerDepart()
{
    int agentId = agentPresenceCombo->currentData().toInt();
    if(agentId == 0) return;
    
    QString dateStr = datePresenceEdit->date().toString("dd/MM/yyyy");
    QString heureActuelle = QTime::currentTime().toString("HH:mm");
    
    QSqlQuery query;
    // On met à jour l'heure de départ de la ligne existante pour aujourd'hui
    query.prepare("UPDATE Presences SET heure_depart = :heure_depart "
                  "WHERE agent_id = :agent_id AND date = :date");
    query.bindValue(":heure_depart", heureActuelle);
    query.bindValue(":agent_id", agentId);
    query.bindValue(":date", dateStr);
    
    if(!query.exec()) {
        QMessageBox::critical(this, "Erreur", query.lastError().text());
    } else {
        if(query.numRowsAffected() == 0) {
            QMessageBox::warning(this, "Attention", "Aucune arrivée n'a été enregistrée pour cet agent aujourd'hui.");
        }
        presencesModel->select(); // Refresh table
    }
}
