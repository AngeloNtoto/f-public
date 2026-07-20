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
#include <QSqlRecord>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QFileDialog>
#include <QLineEdit>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionProgressBar>
#include <QProgressBar>
#include <QApplication>
#include "MissionVerificationDialog.hpp"
#include "FicheAgentDialog.hpp"

MainWindow::MainWindow(const QString &userRole, const QString &userName, QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QString("Fonction Publique - Province du Kwilu [%1 : %2]").arg(userRole, userName));
    resize(1024, 768);
    setupUi();
    
    // Application des droits d'accès
    if (userRole == "Secrétaire") {
        sidebar->item(1)->setHidden(true); // RH
        sidebar->item(2)->setHidden(true); // Sociaux
        sidebar->item(3)->setHidden(true); // Productifs
        sidebar->item(4)->setHidden(true); // Infra
        sidebar->setCurrentRow(5); // Focus direct sur Secrétariat
    } else if (userRole == "Agent RH") {
        sidebar->item(2)->setHidden(true);
        sidebar->item(3)->setHidden(true);
        sidebar->item(4)->setHidden(true);
        sidebar->item(5)->setHidden(true);
        sidebar->setCurrentRow(1); // Focus direct sur RH
    }
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

    struct ModuleDef { QString name; QStyle::StandardPixmap icon; };
    QList<ModuleDef> modules = {
        {"Tableau de Bord", QStyle::SP_ComputerIcon},
        {"Ressources Humaines", QStyle::SP_DirIcon},
        {"Secteurs Sociaux", QStyle::SP_FileDialogDetailedView},
        {"Secteurs Productifs", QStyle::SP_FileDialogListView},
        {"Infrastructures", QStyle::SP_DirHomeIcon},
        {"Secrétariat", QStyle::SP_FileIcon}
    };
    
    for (const auto &mod : modules) {
        QListWidgetItem *item = new QListWidgetItem(style()->standardIcon(mod.icon), mod.name);
        sidebar->addItem(item);
    }

    // Stacked Widget
    stackedWidget = new QStackedWidget(this);
    
    stackedWidget->addWidget(createDashboardPage());
    stackedWidget->addWidget(createRhPage());
    stackedWidget->addWidget(createSecteursSociauxPage());
    stackedWidget->addWidget(createSecteursProductifsPage());
    stackedWidget->addWidget(createInfrastructuresPage());
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
    if (index == 0) {
        refreshDashboard();
    }
}

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

QWidget* MainWindow::createDashboardPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);
    
    layout->addWidget(new QLabel("<h2>Tableau de Bord</h2>", page));
    
    // KPIs
    QHBoxLayout *kpiLayout = new QHBoxLayout();
    
    QFrame *kpiFrame1 = new QFrame();
    kpiFrame1->setFrameStyle(QFrame::StyledPanel);
    kpiFrame1->setStyleSheet("background-color: #2980b9; color: white; border-radius: 8px; padding: 10px;");
    QVBoxLayout *kL1 = new QVBoxLayout(kpiFrame1);
    lblTotalAgents = new QLabel("<h3>0</h3>");
    kL1->addWidget(new QLabel("<b>Total Agents</b>"));
    kL1->addWidget(lblTotalAgents);
    
    QFrame *kpiFrame2 = new QFrame();
    kpiFrame2->setFrameStyle(QFrame::StyledPanel);
    kpiFrame2->setStyleSheet("background-color: #27ae60; color: white; border-radius: 8px; padding: 10px;");
    QVBoxLayout *kL2 = new QVBoxLayout(kpiFrame2);
    lblPresenceRate = new QLabel("<h3>0 %</h3>");
    kL2->addWidget(new QLabel("<b>Taux de Présence (Aujourd'hui)</b>"));
    kL2->addWidget(lblPresenceRate);
    
    kpiLayout->addWidget(kpiFrame1);
    kpiLayout->addWidget(kpiFrame2);
    layout->addLayout(kpiLayout);
    
    // Graphiques
    QHBoxLayout *chartsLayout = new QHBoxLayout();
    
    // Bar Chart
    barChart = new QChart();
    barChart->setTitle("Autorisations de sortie (Mois)");
    barChart->setAnimationOptions(QChart::SeriesAnimations);
    QChartView *barChartView = new QChartView(barChart);
    barChartView->setRenderHint(QPainter::Antialiasing);
    
    // Pie Chart
    pieChart = new QChart();
    pieChart->setTitle("Répartition des Organisations (ONG/ASBL)");
    pieChart->setAnimationOptions(QChart::SeriesAnimations);
    QChartView *pieChartView = new QChartView(pieChart);
    pieChartView->setRenderHint(QPainter::Antialiasing);
    
    chartsLayout->addWidget(barChartView);
    chartsLayout->addWidget(pieChartView);
    
    layout->addLayout(chartsLayout);
    
    refreshDashboard();
    
    return page;
}

void MainWindow::refreshDashboard()
{
    // KPI 1: Total Agents
    QSqlQuery q;
    q.exec("SELECT COUNT(*) FROM Agents");
    int totalAgents = 0;
    if (q.next()) totalAgents = q.value(0).toInt();
    lblTotalAgents->setText(QString("<h3>%1</h3>").arg(totalAgents));
    
    // KPI 2: Présences
    q.prepare("SELECT COUNT(*) FROM Presences WHERE date = ?");
    q.addBindValue(QDate::currentDate().toString("dd/MM/yyyy"));
    q.exec();
    int presents = 0;
    if (q.next()) presents = q.value(0).toInt();
    
    int rate = totalAgents > 0 ? (presents * 100) / totalAgents : 0;
    lblPresenceRate->setText(QString("<h3>%1 % (%2 présents)</h3>").arg(rate).arg(presents));
    
    // Pie Chart: Organisations
    pieChart->removeAllSeries();
    QPieSeries *pieSeries = new QPieSeries();
    q.exec("SELECT nature_juridique, COUNT(*) FROM Organisations GROUP BY nature_juridique");
    while (q.next()) {
        QString nature = q.value(0).toString();
        int count = q.value(1).toInt();
        if(nature.isEmpty()) nature = "Autre";
        pieSeries->append(nature + QString(" (%1)").arg(count), count);
    }
    pieChart->addSeries(pieSeries);
    
    // Bar Chart: Autorisations par mois
    barChart->removeAllSeries();
    QBarSeries *barSeries = new QBarSeries();
    QBarSet *set0 = new QBarSet("Autorisations");
    
    QMap<QString, int> authParMois;
    q.exec("SELECT date_sortie FROM AutorisationSortie");
    while(q.next()) {
        QString d = q.value(0).toString();
        if(d.length() >= 10) {
            QString mois = d.mid(3, 7); // MM/yyyy
            authParMois[mois]++;
        }
    }
    
    QStringList categories;
    for(auto it = authParMois.begin(); it != authParMois.end(); ++it) {
        categories << it.key();
        *set0 << it.value();
    }
    
    if (categories.isEmpty()) {
        categories << "Aucune";
        *set0 << 0;
    }
    
    barSeries->append(set0);
    barChart->addSeries(barSeries);
    
    for(auto axis : barChart->axes()) barChart->removeAxis(axis);
    
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    barChart->addAxis(axisX, Qt::AlignBottom);
    barSeries->attachAxis(axisX);
    
    QValueAxis *axisY = new QValueAxis();
    barChart->addAxis(axisY, Qt::AlignLeft);
    barSeries->attachAxis(axisY);
}

QWidget* MainWindow::createRhPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *title = new QLabel("<h2>Ressources Humaines - Liste des Agents</h2>", page);
    layout->addWidget(title);

    // Barre de recherche
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLineEdit *searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Rechercher un agent par nom ou matricule...");
    searchLayout->addWidget(new QLabel("Recherche :"));
    searchLayout->addWidget(searchEdit);
    layout->addLayout(searchLayout);

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
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers); // Pas d'édition directe
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    layout->addWidget(tableView);
    
    // Filtrage dynamique
    connect(searchEdit, &QLineEdit::textChanged, [this](const QString &text) {
        if(text.isEmpty()) {
            agentModel->setFilter("");
        } else {
            agentModel->setFilter(QString("nom LIKE '%%1%' OR matricule LIKE '%%1%'").arg(text));
        }
        agentModel->select();
    });

    // Ouverture par double clic
    connect(tableView, &QTableView::doubleClicked, [this, tableView](const QModelIndex &index) {
        int row = index.row();
        int agentId = agentModel->record(row).value("id").toInt();
        FicheAgentDialog dialog(agentId, this);
        if(dialog.exec() == QDialog::Accepted) {
            agentModel->select();
        }
    });

    // Boutons d'action
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("Nouveau Dossier Agent", page);
    QPushButton *btnEdit = new QPushButton("Voir/Éditer Fiche", page);
    QPushButton *btnDelete = new QPushButton("Supprimer l'agent", page);
    
    btnAdd->setStyleSheet("padding: 8px; background-color: #27ae60; color: white; border-radius: 4px;");
    btnEdit->setStyleSheet("padding: 8px; background-color: #f39c12; color: white; border-radius: 4px;");
    btnDelete->setStyleSheet("padding: 8px; background-color: #e74c3c; color: white; border-radius: 4px;");
    
    connect(btnAdd, &QPushButton::clicked, [this]() {
        FicheAgentDialog dialog(this);
        if(dialog.exec() == QDialog::Accepted) {
            agentModel->select();
            loadPresenceAgents();
        }
    });
    
    connect(btnEdit, &QPushButton::clicked, [this, tableView]() {
        int row = tableView->currentIndex().row();
        if(row >= 0) {
            int agentId = agentModel->record(row).value("id").toInt();
            FicheAgentDialog dialog(agentId, this);
            if(dialog.exec() == QDialog::Accepted) {
                agentModel->select();
            }
        }
    });

    connect(btnDelete, &QPushButton::clicked, [this, tableView]() {
        int row = tableView->currentIndex().row();
        if(row >= 0) {
            auto reply = QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer cet agent ?");
            if (reply == QMessageBox::Yes) {
                agentModel->removeRow(row);
                agentModel->submitAll();
                agentModel->select();
                loadPresenceAgents();
            }
        }
    });

    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnEdit);
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
    
    // Tableau des organisations
    orgModel = new QSqlTableModel(this);
    orgModel->setTable("Organisations");
    orgModel->select();
    
    QTableView *tableView = new QTableView(page);
    tableView->setModel(orgModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    
    layout->addWidget(tableView);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    
    QPushButton *btnNewIdent = new QPushButton("Nouvelle Demande d'Identification");
    btnNewIdent->setStyleSheet("padding: 10px; background-color: #f39c12; color: white; border-radius: 4px;");
    
    QPushButton *btnMission = new QPushButton("Rapport de Mission");
    btnMission->setStyleSheet("padding: 10px; background-color: #3498db; color: white; border-radius: 4px;");
    
    QPushButton *btnCertif = new QPushButton("Générer Certificat");
    btnCertif->setStyleSheet("padding: 10px; background-color: #27ae60; color: white; border-radius: 4px;");
    
    connect(btnNewIdent, &QPushButton::clicked, this, [this]() {
        IdentificationDialog dialog(this);
        dialog.exec();
        orgModel->select(); // Refresh
    });
    
    connect(btnMission, &QPushButton::clicked, this, [this, tableView]() {
        int row = tableView->currentIndex().row();
        if (row < 0) {
            QMessageBox::warning(this, "Attention", "Veuillez sélectionner une organisation.");
            return;
        }
        int orgId = orgModel->record(row).value("id").toInt();
        QString nomOrg = orgModel->record(row).value("denomination").toString();
        
        MissionVerificationDialog dialog(orgId, nomOrg, this);
        dialog.exec();
    });
    
    connect(btnCertif, &QPushButton::clicked, this, [this, tableView]() {
        int row = tableView->currentIndex().row();
        if (row < 0) {
            QMessageBox::warning(this, "Attention", "Veuillez sélectionner une organisation.");
            return;
        }
        int orgId = orgModel->record(row).value("id").toInt();
        QString nomOrg = orgModel->record(row).value("denomination").toString();
        QString decision = orgModel->record(row).value("decision").toString();
        
        // Vérifier si la mission est validée
        QSqlQuery qCheck;
        qCheck.prepare("SELECT conclusion FROM MissionsVerification WHERE organisation_id = ?");
        qCheck.addBindValue(orgId);
        qCheck.exec();
        
        bool missionValidee = false;
        if(qCheck.next()) {
            if(qCheck.value(0).toString().contains("Favorable", Qt::CaseInsensitive)) {
                missionValidee = true;
            }
        }
        
        if (decision != "Enregistrement Accordé" || !missionValidee) {
            QMessageBox::warning(this, "Refus", "Le certificat ne peut être généré que si la décision est 'Enregistrement Accordé' ET que le rapport de mission est Favorable.");
            return;
        }
        
        // Générer Certificat
        QString html = R"(
            <h2 align="center">REPUBLIQUE DEMOCRATIQUE DU CONGO</h2>
            <h3 align="center">PROVINCE DU KWILU</h3>
            <hr>
            <h1 align="center" style="color:green;">CERTIFICAT D'ENREGISTREMENT</h1>
            <br>
            <p align="center">Il est certifié que l'organisation <b>%1</b> a rempli toutes les conditions requises pour exercer ses activités dans la province du Kwilu, après vérification favorable de la mission de contrôle sur le terrain.</p>
            <p align="center">Ce certificat est délivré pour servir et valoir ce que de droit.</p>
            <br><br><br>
            <p align="right">Fait à Bandundu, le %2</p>
        )";
        html = html.arg(nomOrg, QDate::currentDate().toString("dd/MM/yyyy"));
        
        QTextDocument document;
        document.setHtml(html);
        QString fileName = QFileDialog::getSaveFileName(this, "Sauvegarder le Certificat", "Certificat_" + nomOrg.replace(" ", "_") + ".pdf", "*.pdf");
        if (!fileName.isEmpty()) {
            QPrinter printer(QPrinter::HighResolution);
            printer.setPageOrientation(QPageLayout::Landscape);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(fileName);
            document.print(&printer);
            QMessageBox::information(this, "Succès", "Certificat généré !");
        }
    });
    
    btnLayout->addWidget(btnNewIdent);
    btnLayout->addWidget(btnMission);
    btnLayout->addWidget(btnCertif);
    btnLayout->addStretch();
    
    layout->addLayout(btnLayout);
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

QWidget* MainWindow::createSecteursProductifsPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Header
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *title = new QLabel("<h2>Secteurs Productifs - Entreprises & Coopératives</h2>");
    headerLayout->addWidget(title);
    headerLayout->addStretch();
    
    // Stats rapides
    QFrame *statFrame = new QFrame();
    statFrame->setStyleSheet("background-color: #f39c12; color: white; border-radius: 8px; padding: 10px;");
    QHBoxLayout *statLayout = new QHBoxLayout(statFrame);
    statLayout->addWidget(new QLabel("<b>Agréments délivrés ce mois :</b> 12"));
    statLayout->addSpacing(20);
    statLayout->addWidget(new QLabel("<b>Recettes estimées :</b> 4,500 $"));
    headerLayout->addWidget(statFrame);
    
    layout->addLayout(headerLayout);
    
    // Table
    prodModel = new QSqlTableModel(this);
    prodModel->setTable("Entreprises");
    prodModel->select();
    
    prodModel->setHeaderData(1, Qt::Horizontal, "Nom de l'Entité");
    prodModel->setHeaderData(2, Qt::Horizontal, "Catégorie (PME, Coopérative...)");
    prodModel->setHeaderData(3, Qt::Horizontal, "Numéro RCCM");
    prodModel->setHeaderData(4, Qt::Horizontal, "Statut Fiscal");
    
    QTableView *tableView = new QTableView();
    tableView->setModel(prodModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(tableView);
    
    // Boutons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("Enregistrer une nouvelle Entité");
    QPushButton *btnLicence = new QPushButton("Délivrer une Licence d'Exploitation");
    
    btnAdd->setStyleSheet("padding: 8px; background-color: #27ae60; color: white; border-radius: 4px;");
    btnLicence->setStyleSheet("padding: 8px; background-color: #2980b9; color: white; border-radius: 4px;");
    
    connect(btnAdd, &QPushButton::clicked, [this]() {
        int row = prodModel->rowCount();
        prodModel->insertRow(row);
    });
    
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnLicence);
    btnLayout->addStretch();
    
    layout->addLayout(btnLayout);
    
    return page;
}

class ProgressBarDelegate : public QStyledItemDelegate {
public:
    ProgressBarDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        int progress = index.data().toInt();
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = option.rect.adjusted(4, 4, -4, -4);
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = progress;
        progressBarOption.text = QString::number(progress) + "%";
        progressBarOption.textVisible = true;
        
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    }
};

QWidget* MainWindow::createInfrastructuresPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *title = new QLabel("<h2>Infrastructures & Travaux Publics</h2>");
    layout->addWidget(title);
    
    // Cartes de KPIs
    QHBoxLayout *kpiLayout = new QHBoxLayout();
    
    QFrame *card1 = new QFrame();
    card1->setStyleSheet("background-color: #8e44ad; color: white; border-radius: 8px; padding: 15px;");
    QVBoxLayout *cL1 = new QVBoxLayout(card1);
    cL1->addWidget(new QLabel("<b>Projets Actifs</b>"));
    cL1->addWidget(new QLabel("<h2>8</h2>"));
    kpiLayout->addWidget(card1);
    
    QFrame *card2 = new QFrame();
    card2->setStyleSheet("background-color: #c0392b; color: white; border-radius: 8px; padding: 15px;");
    QVBoxLayout *cL2 = new QVBoxLayout(card2);
    cL2->addWidget(new QLabel("<b>Budget Provincial Alloué</b>"));
    cL2->addWidget(new QLabel("<h2>1,200,500 $</h2>"));
    kpiLayout->addWidget(card2);
    
    layout->addLayout(kpiLayout);
    
    // Table
    infraModel = new QSqlTableModel(this);
    infraModel->setTable("ProjetsInfra");
    infraModel->select();
    
    infraModel->setHeaderData(1, Qt::Horizontal, "Nom du Projet");
    infraModel->setHeaderData(2, Qt::Horizontal, "Type (Route, Pont...)");
    infraModel->setHeaderData(3, Qt::Horizontal, "Localisation");
    infraModel->setHeaderData(4, Qt::Horizontal, "Budget Alloué");
    infraModel->setHeaderData(5, Qt::Horizontal, "Avancement (%)");
    
    QTableView *tableView = new QTableView();
    tableView->setModel(infraModel);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->setItemDelegateForColumn(5, new ProgressBarDelegate(this));
    
    layout->addWidget(tableView);
    
    // Boutons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("Lancer un Nouveau Projet");
    QPushButton *btnUpdate = new QPushButton("Sauvegarder l'Avancement");
    
    btnAdd->setStyleSheet("padding: 8px; background-color: #27ae60; color: white; border-radius: 4px;");
    btnUpdate->setStyleSheet("padding: 8px; background-color: #34495e; color: white; border-radius: 4px;");
    
    connect(btnAdd, &QPushButton::clicked, [this]() {
        int row = infraModel->rowCount();
        infraModel->insertRow(row);
        infraModel->setData(infraModel->index(row, 5), 0); // avancement à 0
    });

    connect(btnUpdate, &QPushButton::clicked, [this]() {
        infraModel->submitAll();
    });
    
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnUpdate);
    btnLayout->addStretch();
    
    layout->addLayout(btnLayout);
    
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
