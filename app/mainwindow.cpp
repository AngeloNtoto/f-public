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
#include <QRandomGenerator>

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
    
    QHBoxLayout *topDashLayout = new QHBoxLayout();
    topDashLayout->addWidget(new QLabel("<h2>Tableau de Bord</h2>", page));
    topDashLayout->addStretch();
    
    QPushButton *btnMock = new QPushButton("Générer Données Mocks (Développement)", page);
    btnMock->setStyleSheet("padding: 8px 16px; background-color: #d35400; color: white; border-radius: 4px; font-weight: bold;");
    topDashLayout->addWidget(btnMock);
    connect(btnMock, &QPushButton::clicked, this, &MainWindow::generateMockData);
    
    layout->addLayout(topDashLayout);
    
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
    layout->setSpacing(12);
    
    QLabel *title = new QLabel("<h2>Ressources Humaines - Liste & Gestion des Agents</h2>", page);
    layout->addWidget(title);

    // Initialisation du modèle SQL
    agentModel = new QSqlTableModel(this);
    agentModel->setTable("Agents");
    agentModel->select();

    // Initialisation du Proxy de Tri/Filtrage Avancé
    agentProxyModel = new AgentFilterProxyModel(this);
    agentProxyModel->setSourceModel(agentModel);
    agentProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    // En-têtes du modèle SQL par nom de champ
    QMap<QString, QString> headers = {
        {"id", "ID"},
        {"matricule", "Matricule"},
        {"nom", "Nom"},
        {"postnom", "Post-nom"},
        {"prenom", "Prénom"},
        {"sexe", "Sexe"},
        {"etat_civil", "État Civil"},
        {"date_naissance", "Date de Naissance"},
        {"telephone", "Téléphone"},
        {"email", "Email"},
        {"adresse", "Adresse"},
        {"ministere", "Ministère"},
        {"secretariat_general", "Secrétariat Général"},
        {"direction", "Direction"},
        {"division", "Division"},
        {"bureau", "Bureau"},
        {"service", "Service"},
        {"niveau_affectation", "Niveau Affectation"},
        {"province", "Province"},
        {"lieu_affectation", "Lieu Affectation"},
        {"grade", "Grade"},
        {"fonction", "Fonction"},
        {"domaine", "Domaine"},
        {"date_engagement", "Date Engagement"},
        {"position_admin", "Position Admin"},
        {"statut_recensement", "Statut Recensement"},
        {"ref_engagement", "Réf. Engagement"},
        {"ref_nomination", "Réf. Nomination"},
        {"ref_commissionnement", "Réf. Commissionnement"},
        {"niveau_etude", "Niveau Étude"},
        {"filiere", "Filière"},
        {"salaire", "Salaire ($)"},
        {"primes", "Primes ($)"}
    };

    for (auto it = headers.begin(); it != headers.end(); ++it) {
        int idx = agentModel->fieldIndex(it.key());
        if (idx != -1) {
            agentModel->setHeaderData(idx, Qt::Horizontal, it.value());
        }
    }

    // ==========================================
    // BARRE DE FILTRAGE & RECHERCHE AVANCÉE (PRO)
    // ==========================================
    QFrame *filterFrame = new QFrame(page);
    filterFrame->setStyleSheet("QFrame { background: #ffffff; border: 1px solid #e2e8f0; border-radius: 6px; padding: 10px; }");
    QVBoxLayout *filterFrameLayout = new QVBoxLayout(filterFrame);
    filterFrameLayout->setSpacing(8);

    // Ligne 1 : Recherche Texte & Cible de colonne
    QHBoxLayout *searchRow = new QHBoxLayout();
    
    QLineEdit *searchEdit = new QLineEdit(filterFrame);
    searchEdit->setPlaceholderText("Filtrer les agents...");
    searchEdit->setClearButtonEnabled(true);
    searchEdit->setStyleSheet("padding: 6px 12px; border: 1px solid #cbd5e1; border-radius: 4px; font-size: 13px;");

    QComboBox *targetColCombo = new QComboBox(filterFrame);
    targetColCombo->addItem("Tous les champs", -1);
    for (int i = 0; i < agentModel->columnCount(); ++i) {
        QString headerText = agentModel->headerData(i, Qt::Horizontal).toString();
        if (!headerText.isEmpty()) {
            targetColCombo->addItem("Champ : " + headerText, i);
        }
    }
    targetColCombo->setStyleSheet("padding: 6px; border: 1px solid #cbd5e1; border-radius: 4px;");

    searchRow->addWidget(new QLabel("<b>Recherche :</b>"));
    searchRow->addWidget(searchEdit, 1);
    searchRow->addWidget(new QLabel("<b>Cible :</b>"));
    searchRow->addWidget(targetColCombo);

    // Ligne 2 : Filtres combinés (Ministère, Sexe, Recensement) + Reset
    QHBoxLayout *combosRow = new QHBoxLayout();

    QComboBox *filterMinistereCombo = new QComboBox(filterFrame);
    filterMinistereCombo->addItem("Tous les Ministères");
    QSqlQuery qMin("SELECT DISTINCT valeur FROM RefOptions WHERE categorie='ministere' UNION SELECT DISTINCT ministere FROM Agents WHERE ministere IS NOT NULL AND ministere != '' ORDER BY valeur ASC");
    while(qMin.next()) filterMinistereCombo->addItem(qMin.value(0).toString());

    QComboBox *filterSexeCombo = new QComboBox(filterFrame);
    filterSexeCombo->addItems({"Tous les sexes", "Féminin", "Masculin"});

    QComboBox *filterRecensementCombo = new QComboBox(filterFrame);
    filterRecensementCombo->addItems({"Tous les statuts", "Oui", "Non"});

    QPushButton *btnResetFilters = new QPushButton(style()->standardIcon(QStyle::SP_BrowserReload), "Réinitialiser", filterFrame);
    btnResetFilters->setCursor(Qt::PointingHandCursor);
    btnResetFilters->setStyleSheet("padding: 6px 12px; background: #64748b; color: white; border-radius: 4px; font-weight: bold;");

    combosRow->addWidget(new QLabel("Ministère :"));
    combosRow->addWidget(filterMinistereCombo);
    combosRow->addWidget(new QLabel("Sexe :"));
    combosRow->addWidget(filterSexeCombo);
    combosRow->addWidget(new QLabel("Recensé :"));
    combosRow->addWidget(filterRecensementCombo);
    combosRow->addStretch();
    combosRow->addWidget(btnResetFilters);

    filterFrameLayout->addLayout(searchRow);
    filterFrameLayout->addLayout(combosRow);

    layout->addWidget(filterFrame);

    // Compteur dynamique d'agents
    QLabel *lblCounter = new QLabel(page);
    lblCounter->setStyleSheet("color: #475569; font-weight: bold; font-size: 13px; margin-top: 4px;");

    auto updateCounter = [this, lblCounter]() {
        int count = agentProxyModel->rowCount();
        int total = agentModel->rowCount();
        lblCounter->setText(QString("Agents affichés : <b>%1</b> / %2").arg(count).arg(total));
    };

    // Connexions du Proxy
    connect(searchEdit, &QLineEdit::textChanged, [this, updateCounter](const QString &text) {
        agentProxyModel->setFilterRegularExpression(QRegularExpression(QRegularExpression::escape(text), QRegularExpression::CaseInsensitiveOption));
        updateCounter();
    });

    connect(targetColCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, targetColCombo, updateCounter](int) {
        int col = targetColCombo->currentData().toInt();
        agentProxyModel->setTargetColumn(col);
        updateCounter();
    });

    connect(filterMinistereCombo, &QComboBox::currentTextChanged, [this, updateCounter](const QString &min) {
        agentProxyModel->setFilterMinistere(min);
        updateCounter();
    });

    connect(filterSexeCombo, &QComboBox::currentTextChanged, [this, updateCounter](const QString &sexe) {
        agentProxyModel->setFilterSexe(sexe);
        updateCounter();
    });

    connect(filterRecensementCombo, &QComboBox::currentTextChanged, [this, updateCounter](const QString &rec) {
        agentProxyModel->setFilterRecensement(rec);
        updateCounter();
    });

    connect(btnResetFilters, &QPushButton::clicked, [this, searchEdit, targetColCombo, filterMinistereCombo, filterSexeCombo, filterRecensementCombo, updateCounter]() {
        searchEdit->clear();
        targetColCombo->setCurrentIndex(0);
        filterMinistereCombo->setCurrentIndex(0);
        filterSexeCombo->setCurrentIndex(0);
        filterRecensementCombo->setCurrentIndex(0);
        agentProxyModel->resetAllFilters();
        updateCounter();
    });

    // ==========================================
    // TABLE VIEW AVEC TRI INTERACTIF SUR TOUTES LES COLONNES
    // ==========================================
    QTableView *tableView = new QTableView(page);
    tableView->setModel(agentProxyModel); // Proxy activé !
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setSortingEnabled(true); // Tri interactif par clic sur en-tête !
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setSortIndicatorShown(true);
    tableView->sortByColumn(2, Qt::AscendingOrder); // Tri par défaut sur la colonne Nom

    layout->addWidget(lblCounter);
    layout->addWidget(tableView);

    updateCounter();

    // Connexion du double-clic sur l'en-tête de colonne pour ouvrir le menu filtrage & tri de la colonne (Style Excel)
    connect(tableView->horizontalHeader(), &QHeaderView::sectionDoubleClicked, [this, tableView, updateCounter](int logicalIndex) {
        QString colName = agentModel->headerData(logicalIndex, Qt::Horizontal).toString();
        QStringList activeFilters = agentProxyModel->columnFilterValues(logicalIndex);

        ColumnFilterDialog dlg(logicalIndex, colName, agentModel, activeFilters, this);
        if (dlg.exec() == QDialog::Accepted) {
            if (dlg.clearFilterRequested()) {
                agentProxyModel->clearColumnFilter(logicalIndex);
            } else {
                if (dlg.sortOrder() == 1) {
                    tableView->sortByColumn(logicalIndex, Qt::AscendingOrder);
                } else if (dlg.sortOrder() == 2) {
                    tableView->sortByColumn(logicalIndex, Qt::DescendingOrder);
                }
                agentProxyModel->setColumnValueFilter(logicalIndex, dlg.selectedValues());
            }
            updateCounter();
        }
    });

    // Ouverture par double clic avec mapping Proxy -> Source
    connect(tableView, &QTableView::doubleClicked, [this, tableView, updateCounter](const QModelIndex &proxyIndex) {
        QModelIndex sourceIndex = agentProxyModel->mapToSource(proxyIndex);
        int row = sourceIndex.row();
        int agentId = agentModel->record(row).value("id").toInt();
        FicheAgentDialog dialog(agentId, this);
        if(dialog.exec() == QDialog::Accepted) {
            agentModel->select();
            updateCounter();
        }
    });

    // Boutons d'action principaux
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogNewFolder), "Nouveau Dossier Agent", page);
    QPushButton *btnEdit = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogDetailedView), "Voir/Éditer Fiche", page);
    QPushButton *btnDelete = new QPushButton(style()->standardIcon(QStyle::SP_TrashIcon), "Supprimer l'agent", page);
    
    btnAdd->setStyleSheet("padding: 8px 16px; background-color: #27ae60; color: white; border-radius: 4px; font-weight: bold;");
    btnEdit->setStyleSheet("padding: 8px 16px; background-color: #f39c12; color: white; border-radius: 4px; font-weight: bold;");
    btnDelete->setStyleSheet("padding: 8px 16px; background-color: #e74c3c; color: white; border-radius: 4px; font-weight: bold;");
    
    connect(btnAdd, &QPushButton::clicked, [this, updateCounter]() {
        FicheAgentDialog dialog(this);
        if(dialog.exec() == QDialog::Accepted) {
            agentModel->select();
            loadPresenceAgents();
            updateCounter();
        }
    });
    
    connect(btnEdit, &QPushButton::clicked, [this, tableView, updateCounter]() {
        QModelIndex proxyIndex = tableView->currentIndex();
        if(proxyIndex.isValid()) {
            QModelIndex sourceIndex = agentProxyModel->mapToSource(proxyIndex);
            int row = sourceIndex.row();
            int agentId = agentModel->record(row).value("id").toInt();
            FicheAgentDialog dialog(agentId, this);
            if(dialog.exec() == QDialog::Accepted) {
                agentModel->select();
                updateCounter();
            }
        } else {
            QMessageBox::warning(this, "Attention", "Veuillez sélectionner un agent dans la liste.");
        }
    });

    connect(btnDelete, &QPushButton::clicked, [this, tableView, updateCounter]() {
        QModelIndex proxyIndex = tableView->currentIndex();
        if(proxyIndex.isValid()) {
            QModelIndex sourceIndex = agentProxyModel->mapToSource(proxyIndex);
            int row = sourceIndex.row();
            auto reply = QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer cet agent ?");
            if (reply == QMessageBox::Yes) {
                agentModel->removeRow(row);
                agentModel->submitAll();
                agentModel->select();
                loadPresenceAgents();
                updateCounter();
            }
        } else {
            QMessageBox::warning(this, "Attention", "Veuillez sélectionner un agent à supprimer.");
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

void MainWindow::generateMockData()
{
    auto reply = QMessageBox::question(this, "Mocks", "Voulez-vous générer des données de test (Agents, Présences, Congés) ?\nCela ne supprime pas les données existantes.");
    if (reply != QMessageBox::Yes) return;
    
    QSqlQuery q;
    
    // Génération de 15 agents
    for (int i = 1; i <= 15; ++i) {
        QString matricule = QString("MAT-%1").arg(QRandomGenerator::global()->bounded(1000, 9999));
        q.prepare("INSERT INTO Agents (matricule, nom, postnom, prenom, sexe, grade, fonction, service, ministere, date_engagement) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        q.addBindValue(matricule);
        q.addBindValue(QString("NomMock%1").arg(i));
        q.addBindValue(QString("PostnomMock%1").arg(i));
        q.addBindValue(QString("Prenom%1").arg(i));
        q.addBindValue(i % 2 == 0 ? "Féminin" : "Masculin");
        q.addBindValue(i % 3 == 0 ? "Directeur" : "Attaché d'Administration");
        q.addBindValue("Chef de Bureau");
        q.addBindValue(i % 2 == 0 ? "Ressources Humaines" : "Secrétariat");
        q.addBindValue("Plan et Coordination de l'Aide au Développement");
        q.addBindValue(QDate::currentDate().addYears(- (i % 10)).toString("dd/MM/yyyy"));
        q.exec();
    }
    
    // Génération de quelques congés / absences
    q.exec("SELECT id FROM Agents LIMIT 5");
    while (q.next()) {
        int agentId = q.value(0).toInt();
        QSqlQuery qC;
        qC.prepare("INSERT INTO Conges (agent_id, type_conge, date_debut, date_fin, duree_jours, mois_annee, motif) "
                   "VALUES (?, ?, ?, ?, ?, ?, ?)");
        qC.addBindValue(agentId);
        qC.addBindValue("Congé Annuel");
        qC.addBindValue(QDate::currentDate().addDays(-10).toString("dd/MM/yyyy"));
        qC.addBindValue(QDate::currentDate().addDays(5).toString("dd/MM/yyyy"));
        qC.addBindValue(15);
        qC.addBindValue(QDate::currentDate().toString("MM/yyyy"));
        qC.addBindValue("Congé de reconstitution");
        qC.exec();
    }

    QMessageBox::information(this, "Succès", "Données de test générées avec succès !");
    refreshDashboard();
    
    // Refresh models if they are already initialized
    if(agentModel) agentModel->select();
    if(presencesModel) presencesModel->select();
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
