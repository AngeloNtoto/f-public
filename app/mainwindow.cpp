#include "mainwindow.hpp"
#include "AutorisationSortieDialog.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHeaderView>
#include <QDebug>

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
    
    QPushButton *btnAutoSortie = new QPushButton("Générer une Autorisation de Sortie", page);
    QPushButton *btnPresence = new QPushButton("Saisir une présence", page);
    
    btnAutoSortie->setStyleSheet("padding: 10px; background-color: #8e44ad; color: white; border-radius: 4px;");
    btnPresence->setStyleSheet("padding: 10px; background-color: #2980b9; color: white; border-radius: 4px;");
    
    connect(btnAutoSortie, &QPushButton::clicked, this, [this]() {
        AutorisationSortieDialog dialog(this);
        dialog.exec();
    });
    
    layout->addWidget(btnAutoSortie);
    layout->addWidget(btnPresence);
    layout->addStretch();
    return page;
}
