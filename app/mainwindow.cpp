#include "mainwindow.hpp"
#include "AutorisationSortieDialog.hpp"
#include "FicheAgentDialog.hpp"
#include "IdentificationDialog.hpp"
#include "MissionVerificationDialog.hpp"
#include <QApplication>
#include <QComboBox>
#include <QDateEdit>
#include <QDebug>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QProgressBar>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlRelation>
#include <QSqlRelationalDelegate>
#include <QSqlRelationalTableModel>
#include <QStyle>
#include <QStyleOptionProgressBar>
#include <QStyledItemDelegate>
#include <QTabWidget>
#include <QTextDocument>
#include <QTime>
#include <QVBoxLayout>

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

class ProjectTrackingDialog : public QDialog {
public:
    ProjectTrackingDialog(int orgId, const QString &orgName, QWidget *parent = nullptr)
        : QDialog(parent), m_orgId(orgId), m_orgName(orgName) {
        setWindowTitle("Suivi des Projets - " + m_orgName);
        resize(500, 450);
        setupUi();
    }

private:
    int m_orgId;
    QString m_orgName;
    
    QLineEdit *titreEdit;
    QLineEdit *localisationEdit;
    QLineEdit *bailleurEdit;
    QLineEdit *budgetEdit;
    QLineEdit *dureeEdit;
    QLineEdit *beneficiaireEdit;
    QComboBox *avancementCombo;
    QTextEdit *observationsEdit;
    
    void setupUi() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        
        QLabel *header = new QLabel(QString("<h3>Fiche de suivi de projet pour : %1</h3>").arg(m_orgName));
        mainLayout->addWidget(header);
        
        QFormLayout *formLayout = new QFormLayout();
        titreEdit = new QLineEdit();
        localisationEdit = new QLineEdit();
        bailleurEdit = new QLineEdit();
        budgetEdit = new QLineEdit();
        dureeEdit = new QLineEdit();
        beneficiaireEdit = new QLineEdit();
        
        avancementCombo = new QComboBox();
        avancementCombo->addItems({"Non démarré", "En cours", "Finalisé", "Suspendu"});
        
        observationsEdit = new QTextEdit();
        observationsEdit->setMaximumHeight(80);
        
        formLayout->addRow("Titre du projet * :", titreEdit);
        formLayout->addRow("Localisation :", localisationEdit);
        formLayout->addRow("Bailleur :", bailleurEdit);
        formLayout->addRow("Budget :", budgetEdit);
        formLayout->addRow("Durée :", dureeEdit);
        formLayout->addRow("Bénéficiaires :", beneficiaireEdit);
        formLayout->addRow("État d'avancement :", avancementCombo);
        formLayout->addRow("Observations :", observationsEdit);
        
        mainLayout->addLayout(formLayout);
        
        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *btnSave = new QPushButton("Enregistrer le Suivi");
        btnSave->setStyleSheet("padding: 8px; background-color: #27ae60; color: white; font-weight: bold; border-radius: 4px;");
        QPushButton *btnCancel = new QPushButton("Annuler");
        btnCancel->setStyleSheet("padding: 8px; background-color: #7f8c8d; color: white; border-radius: 4px;");
        
        connect(btnSave, &QPushButton::clicked, this, &ProjectTrackingDialog::saveProject);
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
        
        btnLayout->addStretch();
        btnLayout->addWidget(btnCancel);
        btnLayout->addWidget(btnSave);
        mainLayout->addLayout(btnLayout);
    }
    
    void saveProject() {
        if (titreEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Le titre du projet est obligatoire.");
            return;
        }
        
        QSqlQuery q;
        q.prepare("INSERT INTO Projets (organisation_id, nom, intitule, localisation, budget, bailleur, duree, beneficiaire, etat_avancement, observations) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        q.addBindValue(m_orgId);
        q.addBindValue(titreEdit->text());
        q.addBindValue(titreEdit->text());
        q.addBindValue(localisationEdit->text());
        q.addBindValue(budgetEdit->text());
        q.addBindValue(bailleurEdit->text());
        q.addBindValue(dureeEdit->text());
        q.addBindValue(beneficiaireEdit->text());
        q.addBindValue(avancementCombo->currentText());
        q.addBindValue(observationsEdit->toPlainText());
        
        if (q.exec()) {
            QMessageBox::information(this, "Succès", "Fiche de suivi de projet enregistrée.");
            accept();
        } else {
            QMessageBox::critical(this, "Erreur", "Erreur lors de l'enregistrement : " + q.lastError().text());
        }
    }
};

MainWindow::MainWindow(const QString &userRole, const QString &userName,
                       QWidget *parent)
    : QMainWindow(parent) {
  setWindowTitle(QString("Fonction Publique - Province du Kwilu [%1 : %2]")
                     .arg(userRole, userName));
  resize(1024, 768);
  setupUi();

  // Application des droits d'accès
  if (userRole == "Secrétaire") {
    sidebar->item(1)->setHidden(true); // RH
    sidebar->item(2)->setHidden(true); // Sociaux
    sidebar->item(3)->setHidden(true); // Productifs
    sidebar->item(4)->setHidden(true); // Infra
    sidebar->setCurrentRow(5);         // Focus direct sur Secrétariat
  } else if (userRole == "Agent RH") {
    sidebar->item(2)->setHidden(true);
    sidebar->item(3)->setHidden(true);
    sidebar->item(4)->setHidden(true);
    sidebar->item(5)->setHidden(true);
    sidebar->setCurrentRow(1); // Focus direct sur RH
  }
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // Sidebar
  sidebar = new QListWidget(this);
  sidebar->setFixedWidth(250);
  sidebar->setStyleSheet(
      "QListWidget { background-color: #2c3e50; color: white; border: none; "
      "font-size: 16px; }"
      "QListWidget::item { padding: 15px; border-bottom: 1px solid #34495e; }"
      "QListWidget::item:selected { background-color: #2980b9; }");

  struct ModuleDef {
    QString name;
    QStyle::StandardPixmap icon;
  };
  QList<ModuleDef> modules = {
      {"Tableau de Bord", QStyle::SP_ComputerIcon},
      {"Ressources Humaines", QStyle::SP_DirIcon},
      {"Secteurs Sociaux", QStyle::SP_FileDialogDetailedView},
      {"Secteurs Productifs", QStyle::SP_FileDialogListView},
      {"Infrastructures", QStyle::SP_DirHomeIcon},
      {"Secrétariat", QStyle::SP_FileIcon}};

  for (const auto &mod : modules) {
    QListWidgetItem *item =
        new QListWidgetItem(style()->standardIcon(mod.icon), mod.name);
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

  connect(sidebar, &QListWidget::currentRowChanged, this,
          &MainWindow::changePage);

  // Sélection par défaut
  sidebar->setCurrentRow(0);
}

void MainWindow::changePage(int index) {
  stackedWidget->setCurrentIndex(index);
  if (index == 0) {
    refreshDashboard();
  }
}

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

QWidget *MainWindow::createDashboardPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(20);

  QHBoxLayout *topDashLayout = new QHBoxLayout();
  topDashLayout->addWidget(new QLabel("<h2>Tableau de Bord</h2>", page));
  topDashLayout->addStretch();

  QPushButton *btnMock =
      new QPushButton("Générer Données Mocks (Développement)", page);
  btnMock->setStyleSheet("padding: 8px 16px; background-color: #d35400; color: "
                         "white; border-radius: 4px; font-weight: bold;");
  topDashLayout->addWidget(btnMock);
  connect(btnMock, &QPushButton::clicked, this, &MainWindow::generateMockData);

  layout->addLayout(topDashLayout);

  // KPIs
  QHBoxLayout *kpiLayout = new QHBoxLayout();

  QFrame *kpiFrame1 = new QFrame();
  kpiFrame1->setFrameStyle(QFrame::StyledPanel);
  kpiFrame1->setStyleSheet("background-color: #2980b9; color: white; "
                           "border-radius: 8px; padding: 10px;");
  QVBoxLayout *kL1 = new QVBoxLayout(kpiFrame1);
  lblTotalAgents = new QLabel("<h3>0</h3>");
  kL1->addWidget(new QLabel("<b>Total Agents</b>"));
  kL1->addWidget(lblTotalAgents);

  QFrame *kpiFrame2 = new QFrame();
  kpiFrame2->setFrameStyle(QFrame::StyledPanel);
  kpiFrame2->setStyleSheet("background-color: #27ae60; color: white; "
                           "border-radius: 8px; padding: 10px;");
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

void MainWindow::refreshDashboard() {
  // KPI 1: Total Agents
  QSqlQuery q;
  q.exec("SELECT COUNT(*) FROM Agents");
  int totalAgents = 0;
  if (q.next())
    totalAgents = q.value(0).toInt();
  lblTotalAgents->setText(QString("<h3>%1</h3>").arg(totalAgents));

  // KPI 2: Présences
  q.prepare("SELECT COUNT(*) FROM Presences WHERE date = ?");
  q.addBindValue(QDate::currentDate().toString("dd/MM/yyyy"));
  q.exec();
  int presents = 0;
  if (q.next())
    presents = q.value(0).toInt();

  int rate = totalAgents > 0 ? (presents * 100) / totalAgents : 0;
  lblPresenceRate->setText(
      QString("<h3>%1 % (%2 présents)</h3>").arg(rate).arg(presents));

  // Pie Chart: Organisations
  pieChart->removeAllSeries();
  QPieSeries *pieSeries = new QPieSeries();
  q.exec("SELECT nature_juridique, COUNT(*) FROM Organisations GROUP BY "
         "nature_juridique");
  while (q.next()) {
    QString nature = q.value(0).toString();
    int count = q.value(1).toInt();
    if (nature.isEmpty())
      nature = "Autre";
    pieSeries->append(nature + QString(" (%1)").arg(count), count);
  }
  pieChart->addSeries(pieSeries);

  // Bar Chart: Autorisations par mois
  barChart->removeAllSeries();
  QBarSeries *barSeries = new QBarSeries();
  QBarSet *set0 = new QBarSet("Autorisations");

  QMap<QString, int> authParMois;
  q.exec("SELECT date_sortie FROM AutorisationSortie");
  while (q.next()) {
    QString d = q.value(0).toString();
    if (d.length() >= 10) {
      QString mois = d.mid(3, 7); // MM/yyyy
      authParMois[mois]++;
    }
  }

  QStringList categories;
  for (auto it = authParMois.begin(); it != authParMois.end(); ++it) {
    categories << it.key();
    *set0 << it.value();
  }

  if (categories.isEmpty()) {
    categories << "Aucune";
    *set0 << 0;
  }

  barSeries->append(set0);
  barChart->addSeries(barSeries);

  for (auto axis : barChart->axes())
    barChart->removeAxis(axis);

  QBarCategoryAxis *axisX = new QBarCategoryAxis();
  axisX->append(categories);
  barChart->addAxis(axisX, Qt::AlignBottom);
  barSeries->attachAxis(axisX);

  QValueAxis *axisY = new QValueAxis();
  barChart->addAxis(axisY, Qt::AlignLeft);
  barSeries->attachAxis(axisY);
}

QWidget *MainWindow::createRhPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(12);

  QLabel *title = new QLabel(
      "<h2>Ressources Humaines - Liste & Gestion des Agents</h2>", page);
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
      {"primes", "Primes ($)"}};

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
  filterFrame->setStyleSheet("QFrame { background: #ffffff; border: 1px solid "
                             "#e2e8f0; border-radius: 6px; padding: 10px; }");
  QVBoxLayout *filterFrameLayout = new QVBoxLayout(filterFrame);
  filterFrameLayout->setSpacing(8);

  // Ligne 1 : Recherche Texte & Cible de colonne
  QHBoxLayout *searchRow = new QHBoxLayout();

  QLineEdit *searchEdit = new QLineEdit(filterFrame);
  searchEdit->setPlaceholderText("Filtrer les agents...");
  searchEdit->setClearButtonEnabled(true);
  searchEdit->setStyleSheet("padding: 6px 12px; border: 1px solid #cbd5e1; "
                            "border-radius: 4px; font-size: 13px;");

  QComboBox *targetColCombo = new QComboBox(filterFrame);
  targetColCombo->addItem("Tous les champs", -1);
  for (int i = 0; i < agentModel->columnCount(); ++i) {
    QString headerText = agentModel->headerData(i, Qt::Horizontal).toString();
    if (!headerText.isEmpty()) {
      targetColCombo->addItem("Champ : " + headerText, i);
    }
  }
  targetColCombo->setStyleSheet(
      "padding: 6px; border: 1px solid #cbd5e1; border-radius: 4px;");

  searchRow->addWidget(new QLabel("<b>Recherche :</b>"));
  searchRow->addWidget(searchEdit, 1);
  searchRow->addWidget(new QLabel("<b>Cible :</b>"));
  searchRow->addWidget(targetColCombo);

  // Ligne 2 : Filtres combinés (Ministère, Sexe, Recensement) + Reset
  QHBoxLayout *combosRow = new QHBoxLayout();

  QComboBox *filterMinistereCombo = new QComboBox(filterFrame);
  filterMinistereCombo->addItem("Tous les Ministères");
  QSqlQuery qMin(
      "SELECT DISTINCT valeur FROM RefOptions WHERE categorie='ministere' "
      "UNION SELECT DISTINCT ministere FROM Agents WHERE ministere IS NOT NULL "
      "AND ministere != '' ORDER BY valeur ASC");
  while (qMin.next())
    filterMinistereCombo->addItem(qMin.value(0).toString());

  QComboBox *filterSexeCombo = new QComboBox(filterFrame);
  filterSexeCombo->addItems({"Tous les sexes", "Féminin", "Masculin"});

  QComboBox *filterRecensementCombo = new QComboBox(filterFrame);
  filterRecensementCombo->addItems({"Tous les statuts", "Oui", "Non"});

  QPushButton *btnResetFilters =
      new QPushButton("🔄 Réinitialiser", filterFrame);
  btnResetFilters->setCursor(Qt::PointingHandCursor);
  btnResetFilters->setStyleSheet(
      "padding: 6px 12px; background: #64748b; color: white; border-radius: "
      "4px; font-weight: bold;");

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
  lblCounter->setStyleSheet(
      "color: #475569; font-weight: bold; font-size: 13px; margin-top: 4px;");

  auto updateCounter = [this, lblCounter]() {
    int count = agentProxyModel->rowCount();
    int total = agentModel->rowCount();
    lblCounter->setText(
        QString("Agents affichés : <b>%1</b> / %2").arg(count).arg(total));
  };

  // Connexions du Proxy
  connect(searchEdit, &QLineEdit::textChanged,
          [this, updateCounter](const QString &text) {
            agentProxyModel->setFilterRegularExpression(
                QRegularExpression(QRegularExpression::escape(text),
                                   QRegularExpression::CaseInsensitiveOption));
            updateCounter();
          });

  connect(targetColCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          [this, targetColCombo, updateCounter](int) {
            int col = targetColCombo->currentData().toInt();
            agentProxyModel->setTargetColumn(col);
            updateCounter();
          });

  connect(filterMinistereCombo, &QComboBox::currentTextChanged,
          [this, updateCounter](const QString &min) {
            agentProxyModel->setFilterMinistere(min);
            updateCounter();
          });

  connect(filterSexeCombo, &QComboBox::currentTextChanged,
          [this, updateCounter](const QString &sexe) {
            agentProxyModel->setFilterSexe(sexe);
            updateCounter();
          });

  connect(filterRecensementCombo, &QComboBox::currentTextChanged,
          [this, updateCounter](const QString &rec) {
            agentProxyModel->setFilterRecensement(rec);
            updateCounter();
          });

  connect(btnResetFilters, &QPushButton::clicked,
          [this, searchEdit, targetColCombo, filterMinistereCombo,
           filterSexeCombo, filterRecensementCombo, updateCounter]() {
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
  tableView->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  tableView->horizontalHeader()->setSortIndicatorShown(true);
  tableView->sortByColumn(
      2, Qt::AscendingOrder); // Tri par défaut sur la colonne Nom

  layout->addWidget(lblCounter);
  layout->addWidget(tableView);

  updateCounter();

  // Connexion du double-clic sur l'en-tête de colonne pour ouvrir le menu
  // filtrage & tri de la colonne (Style Excel)
  connect(tableView->horizontalHeader(), &QHeaderView::sectionDoubleClicked,
          [this, tableView, updateCounter](int logicalIndex) {
            QString colName =
                agentModel->headerData(logicalIndex, Qt::Horizontal).toString();
            QStringList activeFilters =
                agentProxyModel->columnFilterValues(logicalIndex);

            ColumnFilterDialog dlg(logicalIndex, colName, agentModel,
                                   activeFilters, this);
            if (dlg.exec() == QDialog::Accepted) {
              if (dlg.clearFilterRequested()) {
                agentProxyModel->clearColumnFilter(logicalIndex);
              } else {
                if (dlg.sortOrder() == 1) {
                  tableView->sortByColumn(logicalIndex, Qt::AscendingOrder);
                } else if (dlg.sortOrder() == 2) {
                  tableView->sortByColumn(logicalIndex, Qt::DescendingOrder);
                }
                agentProxyModel->setColumnValueFilter(logicalIndex,
                                                      dlg.selectedValues());
              }
              updateCounter();
            }
          });

  // Ouverture par double clic avec mapping Proxy -> Source
  connect(tableView, &QTableView::doubleClicked,
          [this, tableView, updateCounter](const QModelIndex &proxyIndex) {
            QModelIndex sourceIndex = agentProxyModel->mapToSource(proxyIndex);
            int row = sourceIndex.row();
            int agentId = agentModel->record(row).value("id").toInt();
            FicheAgentDialog dialog(agentId, this);
            if (dialog.exec() == QDialog::Accepted) {
              agentModel->select();
              updateCounter();
            }
          });

  // Boutons d'action principaux
  QHBoxLayout *btnLayout = new QHBoxLayout();
  QPushButton *btnAdd = new QPushButton("➕ Nouveau Dossier Agent", page);
  QPushButton *btnEdit = new QPushButton("👁️ Voir/Éditer Fiche", page);
  QPushButton *btnDelete = new QPushButton("🗑️ Supprimer l'agent", page);

  btnAdd->setStyleSheet("padding: 8px 16px; background-color: #27ae60; color: "
                        "white; border-radius: 4px; font-weight: bold;");
  btnEdit->setStyleSheet("padding: 8px 16px; background-color: #f39c12; color: "
                         "white; border-radius: 4px; font-weight: bold;");
  btnDelete->setStyleSheet(
      "padding: 8px 16px; background-color: #e74c3c; color: white; "
      "border-radius: 4px; font-weight: bold;");

  connect(btnAdd, &QPushButton::clicked, [this, updateCounter]() {
    FicheAgentDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
      agentModel->select();
      loadPresenceAgents();
      updateCounter();
    }
  });

  connect(btnEdit, &QPushButton::clicked, [this, tableView, updateCounter]() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (proxyIndex.isValid()) {
      QModelIndex sourceIndex = agentProxyModel->mapToSource(proxyIndex);
      int row = sourceIndex.row();
      int agentId = agentModel->record(row).value("id").toInt();
      FicheAgentDialog dialog(agentId, this);
      if (dialog.exec() == QDialog::Accepted) {
        agentModel->select();
        updateCounter();
      }
    } else {
      QMessageBox::warning(this, "Attention",
                           "Veuillez sélectionner un agent dans la liste.");
    }
  });

  connect(btnDelete, &QPushButton::clicked, [this, tableView, updateCounter]() {
    QModelIndex proxyIndex = tableView->currentIndex();
    if (proxyIndex.isValid()) {
      QModelIndex sourceIndex = agentProxyModel->mapToSource(proxyIndex);
      int row = sourceIndex.row();
      auto reply = QMessageBox::question(
          this, "Confirmation", "Voulez-vous vraiment supprimer cet agent ?");
      if (reply == QMessageBox::Yes) {
        agentModel->removeRow(row);
        agentModel->submitAll();
        agentModel->select();
        loadPresenceAgents();
        updateCounter();
      }
    } else {
      QMessageBox::warning(this, "Attention",
                           "Veuillez sélectionner un agent à supprimer.");
    }
  });

  btnLayout->addWidget(btnAdd);
  btnLayout->addWidget(btnEdit);
  btnLayout->addWidget(btnDelete);
  btnLayout->addStretch();

  layout->addLayout(btnLayout);

  return page;
}

QWidget *MainWindow::createSecteursSociauxPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(15);

  layout->addWidget(new QLabel("<h2>Secteurs Sociaux - Contrôle, Agrément & Suivi (ONG / ASBL / EUP)</h2>", page));

  // --- Zone des indicateurs clés (KPIs) ---
  QFrame *kpiFrame = new QFrame(page);
  kpiFrame->setStyleSheet("QFrame { background-color: #f8fafc; border: 1px solid #e2e8f0; border-radius: 8px; padding: 12px; }");
  QHBoxLayout *kpiLayout = new QHBoxLayout(kpiFrame);

  auto createKpiCard = [](const QString &title, const QString &color) {
      QFrame *card = new QFrame();
      card->setStyleSheet(QString("background-color: %1; color: white; border-radius: 6px; padding: 8px;").arg(color));
      QVBoxLayout *l = new QVBoxLayout(card);
      l->setContentsMargins(5, 5, 5, 5);
      l->setSpacing(2);
      QLabel *tLbl = new QLabel(QString("<b>%1</b>").arg(title));
      tLbl->setStyleSheet("font-size: 10px;");
      QLabel *vLbl = new QLabel("<h2>-</h2>");
      l->addWidget(tLbl);
      l->addWidget(vLbl);
      return qMakePair(card, vLbl);
  };

  auto kpiCertif = createKpiCard("Entités Certifiées", "#27ae60");
  auto kpiValid = createKpiCard("Enregistrements Validés", "#2980b9");
  auto kpiMissions = createKpiCard("Missions Réalisées", "#8e44ad");
  auto kpiProjets = createKpiCard("Projets Suivis", "#d35400");
  auto kpiPerf = createKpiCard("Conformité EUP", "#2c3e50");

  kpiLayout->addWidget(kpiCertif.first);
  kpiLayout->addWidget(kpiValid.first);
  kpiLayout->addWidget(kpiMissions.first);
  kpiLayout->addWidget(kpiProjets.first);
  kpiLayout->addWidget(kpiPerf.first);
  layout->addWidget(kpiFrame);

  // Fonction de mise à jour dynamique des KPIs
  auto updateSocialKpis = [kpiCertif, kpiValid, kpiMissions, kpiProjets, kpiPerf]() {
      QSqlQuery q;
      
      // 1. Entités Certifiées (Enregistrement accordé + mission favorable)
      q.exec("SELECT COUNT(*) FROM Organisations o "
             "INNER JOIN MissionsVerification m ON o.id = m.organisation_id "
             "WHERE o.decision = 'Enregistrement accordé' AND m.conclusion != 'Ne remplit pas les conditions'");
      if(q.next()) kpiCertif.second->setText(QString("<h2>%1</h2>").arg(q.value(0).toInt()));

      // 2. Enregistrements Validés
      q.exec("SELECT COUNT(*) FROM Organisations WHERE decision IN ('Enregistrement accordé', 'Dossier confirmé')");
      if(q.next()) kpiValid.second->setText(QString("<h2>%1</h2>").arg(q.value(0).toInt()));

      // 3. Missions réalisées
      q.exec("SELECT COUNT(*) FROM MissionsVerification");
      if(q.next()) kpiMissions.second->setText(QString("<h2>%1</h2>").arg(q.value(0).toInt()));

      // 4. Projets suivis
      q.exec("SELECT COUNT(*) FROM Projets");
      if(q.next()) kpiProjets.second->setText(QString("<h2>%1</h2>").arg(q.value(0).toInt()));

      // 5. Score de performance
      q.exec("SELECT "
             "COUNT(CASE WHEN conclusion = 'Existe et exerce' THEN 1 END) * 100 / NULLIF(COUNT(*), 0) "
             "FROM MissionsVerification");
      if(q.next()) {
          int score = q.value(0).isNull() ? 0 : q.value(0).toInt();
          kpiPerf.second->setText(QString("<h2>%1 %</h2>").arg(score));
      }
  };

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

  // --- Grille de boutons d'action professionnels ---
  QGridLayout *gridButtons = new QGridLayout();
  gridButtons->setSpacing(10);

  QPushButton *btnNewIdent = new QPushButton(style()->standardIcon(QStyle::SP_FileIcon), "Nouvelle Demande d'Identification");
  btnNewIdent->setStyleSheet("padding: 10px; background-color: #f39c12; color: white; border-radius: 4px; font-weight: bold;");
  
  QPushButton *btnMission = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogContentsView), "Rapport de Mission sur Terrain");
  btnMission->setStyleSheet("padding: 10px; background-color: #3498db; color: white; border-radius: 4px; font-weight: bold;");
  
  QPushButton *btnSuiviProjets = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogListView), "Fiche de Suivi des Projets");
  btnSuiviProjets->setStyleSheet("padding: 10px; background-color: #e67e22; color: white; border-radius: 4px; font-weight: bold;");
  
  QPushButton *btnEvalPerf = new QPushButton(style()->standardIcon(QStyle::SP_DialogApplyButton), "Évaluation Performance & Conformité");
  btnEvalPerf->setStyleSheet("padding: 10px; background-color: #9b59b6; color: white; border-radius: 4px; font-weight: bold;");
  
  QPushButton *btnPubliRapport = new QPushButton(style()->standardIcon(QStyle::SP_FileDialogToParent), "Publication du Rapport par Entité");
  btnPubliRapport->setStyleSheet("padding: 10px; background-color: #8e44ad; color: white; border-radius: 4px; font-weight: bold;");
  
  QPushButton *btnCertif = new QPushButton(style()->standardIcon(QStyle::SP_DialogSaveButton), "Générer Certificat d'Enregistrement");
  btnCertif->setStyleSheet("padding: 10px; background-color: #27ae60; color: white; border-radius: 4px; font-weight: bold;");

  gridButtons->addWidget(btnNewIdent, 0, 0);
  gridButtons->addWidget(btnMission, 0, 1);
  gridButtons->addWidget(btnSuiviProjets, 0, 2);
  gridButtons->addWidget(btnEvalPerf, 1, 0);
  gridButtons->addWidget(btnPubliRapport, 1, 1);
  gridButtons->addWidget(btnCertif, 1, 2);
  layout->addLayout(gridButtons);

  // --- Connexions ---
  
  connect(btnNewIdent, &QPushButton::clicked, this, [this, updateSocialKpis]() {
    IdentificationDialog dialog(this);
    if(dialog.exec() == QDialog::Accepted) {
        orgModel->select();
        updateSocialKpis();
    }
  });

  connect(btnMission, &QPushButton::clicked, this, [this, tableView, updateSocialKpis]() {
    int row = tableView->currentIndex().row();
    if (row < 0) {
      QMessageBox::warning(this, "Attention", "Veuillez sélectionner une organisation dans la liste.");
      return;
    }
    int orgId = orgModel->record(row).value("id").toInt();
    QString nomOrg = orgModel->record(row).value("denomination").toString();

    MissionVerificationDialog dialog(orgId, nomOrg, this);
    if(dialog.exec() == QDialog::Accepted) {
        updateSocialKpis();
    }
  });

  connect(btnSuiviProjets, &QPushButton::clicked, this, [this, tableView, updateSocialKpis]() {
    int row = tableView->currentIndex().row();
    if (row < 0) {
      QMessageBox::warning(this, "Attention", "Veuillez sélectionner une organisation.");
      return;
    }
    int orgId = orgModel->record(row).value("id").toInt();
    QString nomOrg = orgModel->record(row).value("denomination").toString();

    ProjectTrackingDialog dialog(orgId, nomOrg, this);
    if(dialog.exec() == QDialog::Accepted) {
        updateSocialKpis();
    }
  });

  connect(btnEvalPerf, &QPushButton::clicked, this, [this, tableView]() {
    int row = tableView->currentIndex().row();
    if (row < 0) {
      QMessageBox::warning(this, "Attention", "Veuillez sélectionner une organisation.");
      return;
    }
    int orgId = orgModel->record(row).value("id").toInt();
    QString nomOrg = orgModel->record(row).value("denomination").toString();

    QSqlQuery q;
    q.prepare("SELECT date_mission, equipe, situation_admin, gouvernance, rh_materiel, "
              "verification_projets, niveau_execution, impact_communautaire, conclusion "
              "FROM MissionsVerification WHERE organisation_id = ?");
    q.addBindValue(orgId);
    q.exec();

    if (q.next()) {
        QString text = QString(
            "<h3>Fiche d'Évaluation - %1</h3>"
            "<p><b>Date inspection :</b> %2</p>"
            "<p><b>Équipe inspecteurs :</b> %3</p>"
            "<p><b>Situation Juridique :</b> %4</p>"
            "<p><b>Gouvernance institutionnelle :</b> %5</p>"
            "<p><b>Ressources Humaines & Locaux :</b> %6</p>"
            "<p><b>Vérification opérationnelle projets :</b> %7</p>"
            "<p><b>Niveau d'exécution :</b> %8</p>"
            "<p><b>Impact mesuré :</b> %9</p>"
            "<p><b>Avis final de conformité :</b> <b><font color='blue'>%10</font></b></p>"
        ).arg(nomOrg, q.value(0).toString(), q.value(1).toString(), q.value(2).toString(),
              q.value(3).toString(), q.value(4).toString(), q.value(5).toString(),
              q.value(6).toString(), q.value(7).toString(), q.value(8).toString());

        QDialog *dlg = new QDialog(this);
        dlg->setWindowTitle("Performance & Conformité");
        dlg->resize(600, 400);
        QVBoxLayout *l = new QVBoxLayout(dlg);
        QTextEdit *te = new QTextEdit();
        te->setHtml(text);
        te->setReadOnly(true);
        l->addWidget(te);
        QPushButton *closeBtn = new QPushButton("Fermer");
        connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
        l->addWidget(closeBtn);
        dlg->exec();
    } else {
        QMessageBox::information(this, "Évaluation indisponible", "Aucune mission de vérification n'a été enregistrée pour cette organisation.");
    }
  });

  connect(btnPubliRapport, &QPushButton::clicked, this, [this, tableView]() {
    int row = tableView->currentIndex().row();
    if (row < 0) {
      QMessageBox::warning(this, "Attention", "Veuillez sélectionner une organisation.");
      return;
    }
    int orgId = orgModel->record(row).value("id").toInt();
    QString nomOrg = orgModel->record(row).value("denomination").toString();
    QSqlRecord rec = orgModel->record(row);

    // Publication du rapport d'entité complet (HTML/PDF)
    QString html = QString(
        "<h1>RAPPORT GÉNÉRAL DE LA DIVISION PROVINCIALE</h1>"
        "<h2>Entité : %1 (%2)</h2>"
        "<p><b>Numéro d'enregistrement :</b> %3</p>"
        "<p><b>Nature juridique :</b> %4</p>"
        "<p><b>Adresse Kwilu :</b> %5</p>"
        "<p><b>Téléphone :</b> %6 | <b>Email :</b> %7</p>"
        "<p><b>Représentant Légal :</b> %8</p>"
        "<p><b>Domaines d'intervention :</b> %9</p>"
        "<p><b>Territoires d'intervention :</b> %10</p>"
        "<p><b>Décision statutaire :</b> %11</p>"
        "<p><b>Observations :</b> %12</p>"
    ).arg(nomOrg, rec.value("sigle").toString(), rec.value("num_enregistrement").toString(),
          rec.value("nature_juridique").toString(), rec.value("adresse_province").toString(),
          rec.value("telephone").toString(), rec.value("email").toString(),
          rec.value("representant_nom").toString(), rec.value("domaines").toString(),
          rec.value("zones").toString(), rec.value("decision").toString(),
          rec.value("observations").toString());

    // Joindre le rapport de mission
    QSqlQuery q;
    q.prepare("SELECT date_mission, equipe, conclusion, recommandations_structure, recommandations_division FROM MissionsVerification WHERE organisation_id = ?");
    q.addBindValue(orgId);
    q.exec();
    if(q.next()) {
        html += QString(
            "<hr><h3>Vérification terrain & Inspection</h3>"
            "<p><b>Date mission :</b> %1</p>"
            "<p><b>Inspecteurs :</b> %2</p>"
            "<p><b>Conclusion :</b> %3</p>"
            "<p><b>Reco structure :</b> %4</p>"
            "<p><b>Reco division :</b> %5</p>"
        ).arg(q.value(0).toString(), q.value(1).toString(), q.value(2).toString(),
              q.value(3).toString(), q.value(4).toString());
    }

    QTextDocument document;
    document.setHtml(html);
    QString fileName = QFileDialog::getSaveFileName(this, "Publier le Rapport complet", "Rapport_Complet_" + nomOrg.replace(" ", "_") + ".pdf", "*.pdf");
    if(!fileName.isEmpty()) {
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileName);
        document.print(&printer);
        QMessageBox::information(this, "Succès", "Rapport complet généré et publié !");
    }
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
    if (qCheck.next()) {
      QString conclusion = qCheck.value(0).toString();
      if (conclusion == "Existe et exerce" || conclusion == "Existe avec insuffisances") {
        missionValidee = true;
      }
    }

    if (decision != "Enregistrement accordé" || !missionValidee) {
      QMessageBox::warning(this, "Refus",
                           "Le certificat ne peut être généré que si la "
                           "décision est 'Enregistrement accordé' ET que le "
                           "rapport de mission confirme que la structure existe et exerce.");
      return;
    }

    // Générer Certificat d'enregistrement officiel
    QString html = R"(
        <div style="border: 5px double #2c3e50; padding: 20px; text-align: center;">
            <h2 align="center">RÉPUBLIQUE DÉMOCRATIQUE DU CONGO</h2>
            <h3 align="center">PROVINCE DU KWILU</h3>
            <h4 align="center">DIVISION PROVINCIALE DE LA FONCTION PUBLIQUE</h4>
            <hr width="80%">
            <br>
            <h1 align="center" style="color:#27ae60; font-family: Garamond;">CERTIFICAT D'ENREGISTREMENT</h1>
            <br>
            <p align="center" style="font-size: 16px;">Il est officiellement certifié que l'organisation :</p>
            <h2 align="center" style="color:#2c3e50;">%1</h2>
            <p align="center" style="font-size: 14px;">a été enregistrée sous le numéro d'enregistrement provincial mentionné dans ses fiches.</p>
            <p align="center" style="font-size: 14px;">Après inspection physique favorable de ses activités dans la province du Kwilu, cette structure est reconnue et autorisée à poursuivre ses projets d'intérêt social.</p>
            <br><br>
            <table width="100%">
                <tr>
                    <td align="left">Date de délivrance : <b>%2</b></td>
                    <td align="right"><b>Le Chef de Division Provinciale</b></td>
                </tr>
            </table>
        </div>
    )";
    html = html.arg(nomOrg, QDate::currentDate().toString("dd/MM/yyyy"));

    QTextDocument document;
    document.setHtml(html);
    QString fileName = QFileDialog::getSaveFileName(
        this, "Sauvegarder le Certificat",
        "Certificat_" + nomOrg.replace(" ", "_") + ".pdf", "*.pdf");
    if (!fileName.isEmpty()) {
      QPrinter printer(QPrinter::HighResolution);
      printer.setPageOrientation(QPageLayout::Landscape);
      printer.setOutputFormat(QPrinter::PdfFormat);
      printer.setOutputFileName(fileName);
      document.print(&printer);
      QMessageBox::information(this, "Succès", "Le Certificat d'Enregistrement a été généré !");
    }
  });

  updateSocialKpis(); // Charger les statistiques
  return page;
}



QWidget *MainWindow::createSecretariatPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  layout->addWidget(new QLabel("<h2>Secrétariat</h2>", page));

  QTabWidget *tabWidget = new QTabWidget(page);

  // Onglet Autorisations
  QWidget *tabAuto = new QWidget();
  QVBoxLayout *layoutAuto = new QVBoxLayout(tabAuto);

  QHBoxLayout *btnLayoutAuto = new QHBoxLayout();
  QPushButton *btnAutoSortie =
      new QPushButton("Générer une Autorisation de Sortie", tabAuto);
  btnAutoSortie->setStyleSheet(
      "padding: 10px; background-color: #8e44ad; color: white; border-radius: "
      "4px; font-weight: bold;");

  QPushButton *btnVoirAgentAuto =
      new QPushButton(style()->standardIcon(QStyle::SP_FileDialogContentsView),
                      "Voir le Dossier de l'Agent", tabAuto);
  btnVoirAgentAuto->setStyleSheet(
      "padding: 10px; background-color: #2980b9; color: white; border-radius: "
      "4px; font-weight: bold;");

  QPushButton *btnMockData =
      new QPushButton(style()->standardIcon(QStyle::SP_FileDialogToParent),
                      "Générer Données Mock (Dev)", tabAuto);
  btnMockData->setStyleSheet("padding: 10px; background-color: #e74c3c; color: "
                             "white; border-radius: 4px; font-weight: bold;");

  btnLayoutAuto->addWidget(btnAutoSortie);
  btnLayoutAuto->addWidget(btnVoirAgentAuto);
  btnLayoutAuto->addWidget(btnMockData);
  btnLayoutAuto->addStretch();
  layoutAuto->addLayout(btnLayoutAuto);

  // Modèle SQL Personnalisé avec Jointure pour afficher les 13 colonnes exactes
  QSqlQueryModel *autoQueryModel = new QSqlQueryModel(this);
  auto refreshAutoQuery = [autoQueryModel]() {
    autoQueryModel->setQuery(
        "SELECT "
        "a.id AS \"N° Autorisation\", "
        "ag.nom || ' ' || ag.postnom || ' ' || COALESCE(ag.prenom, '') AS "
        "\"Noms de l'agent\", "
        "ag.grade AS \"Grade\", "
        "ag.matricule AS \"Matricule\", "
        "ag.fonction AS \"Fonction\", "
        "ag.service AS \"Service\", "
        "a.destination AS \"Destination / Trajet\", "
        "a.motif AS \"Motif de déplacement\", "
        "a.duree AS \"Durée\", "
        "a.date_depart AS \"Date départ prévue\", "
        "a.date_retour AS \"Date retour probable\", "
        "a.transport AS \"Mode de transport\", "
        "COALESCE(a.mentions, '') AS \"Mentions spéciales\" "
        "FROM AutorisationSortie a "
        "LEFT JOIN Agents ag ON a.agent_id = ag.id "
        "ORDER BY a.id DESC");
  };
  refreshAutoQuery();

  QTableView *tvAuto = new QTableView(tabAuto);
  tvAuto->setModel(autoQueryModel);
  tvAuto->setSelectionBehavior(QAbstractItemView::SelectRows);
  tvAuto->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvAuto->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  tvAuto->horizontalHeader()->setStretchLastSection(true);
  layoutAuto->addWidget(tvAuto);

  // Consultation du dossier de l'agent depuis les autorisations
  auto openAgentFromAuto = [this, tvAuto, autoQueryModel]() {
    int row = tvAuto->currentIndex().row();
    if (row < 0) {
      QMessageBox::warning(this, "Attention",
                           "Veuillez sélectionner une autorisation.");
      return;
    }
    int autoId = autoQueryModel->data(autoQueryModel->index(row, 0)).toInt();
    QSqlQuery q;
    q.prepare("SELECT agent_id FROM AutorisationSortie WHERE id = ?");
    q.addBindValue(autoId);
    if (q.exec() && q.next()) {
      int agentId = q.value(0).toInt();
      FicheAgentDialog dialog(agentId, this);
      dialog.exec();
    }
  };
  connect(tvAuto, &QTableView::doubleClicked, this, openAgentFromAuto);
  connect(btnVoirAgentAuto, &QPushButton::clicked, this, openAgentFromAuto);

  connect(btnAutoSortie, &QPushButton::clicked, this,
          [this, refreshAutoQuery]() {
            AutorisationSortieDialog dialog(this);
            dialog.exec();
            refreshAutoQuery(); // Rafraîchit la liste des autorisations
          });

  connect(btnMockData, &QPushButton::clicked, this, [this, refreshAutoQuery]() {
    int ret = QMessageBox::question(
        this, "Génération Mock",
        "Voulez-vous générer des agents, présences et autorisations fictifs "
        "pour tester l'application ?");
    if (ret == QMessageBox::Yes) {
      QSqlQuery q;
      // 5 Agents
      q.exec(
          "INSERT INTO Agents (matricule, nom, postnom, prenom, sexe, "
          "ministere, grade, fonction, service, date_engagement) VALUES "
          "('M1001', 'KABILA', 'MUKENDI', 'Joseph', 'Masculin', 'Fonction "
          "Publique', 'Directeur', 'Directeur Général', 'RH', '01/01/2010')");
      q.exec(
          "INSERT INTO Agents (matricule, nom, postnom, prenom, sexe, "
          "ministere, grade, fonction, service, date_engagement) VALUES "
          "('M1002', 'LUMUMBA', 'OMALOKAMBA', 'Patrice', 'Masculin', 'Plan et "
          "Coordination de l''Aide au Développement', 'Chef de Division', "
          "'Chef de Division Suivi', 'Planification', '15/06/2015')");
      q.exec("INSERT INTO Agents (matricule, nom, postnom, prenom, sexe, "
             "ministere, grade, fonction, service, date_engagement) VALUES "
             "('M1003', 'TSHISEKEDI', 'ILUNGA', 'Félix', 'Masculin', 'Santé "
             "Publique', 'Chef de Bureau', 'Chef de Bureau Courrier', "
             "'Secrétariat', '20/08/2018')");
      q.exec("INSERT INTO Agents (matricule, nom, postnom, prenom, sexe, "
             "ministere, grade, fonction, service, date_engagement) VALUES "
             "('M1004', 'KIMBANGU', 'DIANGIENDA', 'Simon', 'Masculin', "
             "'Finances', 'Attaché d''Administration', 'Comptable', "
             "'Finances', '05/03/2020')");
      q.exec("INSERT INTO Agents (matricule, nom, postnom, prenom, sexe, "
             "ministere, grade, fonction, service, date_engagement) VALUES "
             "('M1005', 'MWIYA', 'MANGROU', 'Marie', 'Féminin', 'Budget', "
             "'Agent d''Appoint', 'Secrétaire', 'Réception', '10/11/2022')");

      // Présences aujourd'hui
      QString today = QDate::currentDate().toString("dd/MM/yyyy");
      q.exec("INSERT INTO Presences (agent_id, date, heure_arrivee, signature) "
             "VALUES (1, '" +
             today + "', '07:45', '')");
      q.exec("INSERT INTO Presences (agent_id, date, heure_arrivee, signature) "
             "VALUES (2, '" +
             today + "', '08:10', '')");
      q.exec("INSERT INTO Presences (agent_id, date, heure_arrivee, signature) "
             "VALUES (5, '" +
             today + "', '07:55', '')");

      // Autorisations avec mentions spéciales
      q.exec("INSERT INTO AutorisationSortie (agent_id, destination, motif, "
             "duree, date_depart, date_retour, transport, mentions) VALUES (3, "
             "'Kinshasa', 'Mission Officielle de contrôle', '5 Jours', "
             "'10/08/2026', '15/08/2026', 'Avion', 'Urgent - Prise en charge "
             "provinciale')");
      q.exec("INSERT INTO AutorisationSortie (agent_id, destination, motif, "
             "duree, date_depart, date_retour, transport, mentions) VALUES (4, "
             "'Kikwit', 'Deuil familial', '3 Jours', '12/08/2026', "
             "'15/08/2026', 'Bus', 'Congé de circonstance accordé')");

      // Congés
      q.exec("INSERT INTO Conges (agent_id, type_conge, date_debut, date_fin, "
             "duree_jours, mois_annee, motif) VALUES (3, 'Mission', "
             "'10/08/2026', '15/08/2026', 5, '08/2026', 'Mission Officielle')");

      refreshAutoQuery();
      refreshPresencesTable(datePresenceEdit->date());
      if (agentModel)
        agentModel->select();
      refreshDashboard();
      QMessageBox::information(this, "Succès",
                               "Données mock générées avec succès !");
    }
  });

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
  QPushButton *btnVoirAgentPresence =
      new QPushButton(style()->standardIcon(QStyle::SP_FileDialogContentsView),
                      "Voir le Dossier de l'Agent", tabPresence);

  btnArrivee->setStyleSheet("padding: 8px; background-color: #27ae60; color: "
                            "white; border-radius: 4px;");
  btnDepart->setStyleSheet("padding: 8px; background-color: #e67e22; color: "
                           "white; border-radius: 4px;");
  btnVoirAgentPresence->setStyleSheet(
      "padding: 8px; background-color: #2980b9; color: white; border-radius: "
      "4px;");

  connect(btnArrivee, &QPushButton::clicked, this, &MainWindow::pointerArrivee);
  connect(btnDepart, &QPushButton::clicked, this, &MainWindow::pointerDepart);

  pointageLayout->addWidget(new QLabel("Date :"));
  pointageLayout->addWidget(datePresenceEdit);
  pointageLayout->addWidget(new QLabel("Agent :"));
  pointageLayout->addWidget(agentPresenceCombo);
  pointageLayout->addWidget(btnArrivee);
  pointageLayout->addWidget(btnDepart);
  pointageLayout->addWidget(btnVoirAgentPresence);
  pointageLayout->addStretch();

  layoutPresence->addLayout(pointageLayout);

  // Modèle SQL avec Jointure pour afficher les 10 colonnes complètes des
  // présences
  presencesQueryModel = new QSqlQueryModel(this);
  refreshPresencesTable(datePresenceEdit->date());

  QTableView *tableView = new QTableView(tabPresence);
  tableView->setModel(presencesQueryModel);
  tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tableView->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  tableView->horizontalHeader()->setStretchLastSection(true);

  // Consultation du dossier de l'agent depuis les présences
  auto openAgentFromPresence = [this, tableView]() {
    int row = tableView->currentIndex().row();
    if (row < 0) {
      QMessageBox::warning(this, "Attention",
                           "Veuillez sélectionner une ligne de présence.");
      return;
    }
    int presId =
        presencesQueryModel->data(presencesQueryModel->index(row, 0)).toInt();
    QSqlQuery q;
    q.prepare("SELECT agent_id FROM Presences WHERE id = ?");
    q.addBindValue(presId);
    if (q.exec() && q.next()) {
      int agentId = q.value(0).toInt();
      FicheAgentDialog dialog(agentId, this);
      dialog.exec();
    }
  };
  connect(tableView, &QTableView::doubleClicked, this, openAgentFromPresence);
  connect(btnVoirAgentPresence, &QPushButton::clicked, this,
          openAgentFromPresence);

  // Rafraîchir le tableau si on change la date
  connect(datePresenceEdit, &QDateEdit::dateChanged, this,
          &MainWindow::refreshPresencesTable);

  layoutPresence->addWidget(tableView);
  tabWidget->addTab(tabPresence, "Gestion des Présences");

  layout->addWidget(tabWidget);
  return page;
}

QWidget *MainWindow::createSecteursProductifsPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  // Header
  QHBoxLayout *headerLayout = new QHBoxLayout();
  QLabel *title =
      new QLabel("<h2>Secteurs Productifs - Entreprises & Coopératives</h2>");
  headerLayout->addWidget(title);
  headerLayout->addStretch();

  // Stats rapides
  QFrame *statFrame = new QFrame();
  statFrame->setStyleSheet("background-color: #f39c12; color: white; "
                           "border-radius: 8px; padding: 10px;");
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
  prodModel->setHeaderData(2, Qt::Horizontal,
                           "Catégorie (PME, Coopérative...)");
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
  QPushButton *btnLicence =
      new QPushButton("Délivrer une Licence d'Exploitation");

  btnAdd->setStyleSheet("padding: 8px; background-color: #27ae60; color: "
                        "white; border-radius: 4px;");
  btnLicence->setStyleSheet("padding: 8px; background-color: #2980b9; color: "
                            "white; border-radius: 4px;");

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
  ProgressBarDelegate(QObject *parent = nullptr)
      : QStyledItemDelegate(parent) {}
  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    int progress = index.data().toInt();
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.rect = option.rect.adjusted(4, 4, -4, -4);
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.progress = progress;
    progressBarOption.text = QString::number(progress) + "%";
    progressBarOption.textVisible = true;

    QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                       &progressBarOption, painter);
  }
};

QWidget *MainWindow::createInfrastructuresPage() {
  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(20, 20, 20, 20);

  QLabel *title = new QLabel("<h2>Infrastructures & Travaux Publics</h2>");
  layout->addWidget(title);

  // Cartes de KPIs
  QHBoxLayout *kpiLayout = new QHBoxLayout();

  QFrame *card1 = new QFrame();
  card1->setStyleSheet("background-color: #8e44ad; color: white; "
                       "border-radius: 8px; padding: 15px;");
  QVBoxLayout *cL1 = new QVBoxLayout(card1);
  cL1->addWidget(new QLabel("<b>Projets Actifs</b>"));
  cL1->addWidget(new QLabel("<h2>8</h2>"));
  kpiLayout->addWidget(card1);

  QFrame *card2 = new QFrame();
  card2->setStyleSheet("background-color: #c0392b; color: white; "
                       "border-radius: 8px; padding: 15px;");
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

  btnAdd->setStyleSheet("padding: 8px; background-color: #27ae60; color: "
                        "white; border-radius: 4px;");
  btnUpdate->setStyleSheet("padding: 8px; background-color: #34495e; color: "
                           "white; border-radius: 4px;");

  connect(btnAdd, &QPushButton::clicked, [this]() {
    int row = infraModel->rowCount();
    infraModel->insertRow(row);
    infraModel->setData(infraModel->index(row, 5), 0); // avancement à 0
  });

  connect(btnUpdate, &QPushButton::clicked,
          [this]() { infraModel->submitAll(); });

  btnLayout->addWidget(btnAdd);
  btnLayout->addWidget(btnUpdate);
  btnLayout->addStretch();

  layout->addLayout(btnLayout);

  return page;
}

void MainWindow::generateMockData() {
  auto reply = QMessageBox::question(
      this, "Mocks",
      "Voulez-vous générer des données de test (Agents, Présences, Congés) "
      "?\nCela ne supprime pas les données existantes.");
  if (reply != QMessageBox::Yes)
    return;

  QSqlQuery q;

  // Génération de 15 agents
  for (int i = 1; i <= 15; ++i) {
    QString matricule =
        QString("MAT-%1").arg(QRandomGenerator::global()->bounded(1000, 9999));
    q.prepare("INSERT INTO Agents (matricule, nom, postnom, prenom, sexe, "
              "grade, fonction, service, ministere, date_engagement) "
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
    q.addBindValue(
        QDate::currentDate().addYears(-(i % 10)).toString("dd/MM/yyyy"));
    q.exec();
  }

  // Génération de quelques congés / absences
  q.exec("SELECT id FROM Agents LIMIT 5");
  while (q.next()) {
    int agentId = q.value(0).toInt();
    QSqlQuery qC;
    qC.prepare("INSERT INTO Conges (agent_id, type_conge, date_debut, "
               "date_fin, duree_jours, mois_annee, motif) "
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

  QMessageBox::information(this, "Succès",
                           "Données de test générées avec succès !");
  refreshDashboard();

  // Refresh models if they are already initialized
  if (agentModel)
    agentModel->select();
  if (datePresenceEdit)
    refreshPresencesTable(datePresenceEdit->date());
}

void MainWindow::loadPresenceAgents() {
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

void MainWindow::pointerArrivee() {
  int agentId = agentPresenceCombo->currentData().toInt();
  if (agentId == 0) {
    QMessageBox::warning(this, "Erreur", "Veuillez sélectionner un agent.");
    return;
  }

  QString dateStr = datePresenceEdit->date().toString("dd/MM/yyyy");

  // Vérifier si l'agent a déjà pointé aujourd'hui
  QSqlQuery checkQuery;
  checkQuery.prepare(
      "SELECT id FROM Presences WHERE agent_id = :agent_id AND date = :date");
  checkQuery.bindValue(":agent_id", agentId);
  checkQuery.bindValue(":date", dateStr);
  checkQuery.exec();

  if (checkQuery.next()) {
    QMessageBox::information(
        this, "Info", "Cet agent a déjà une ligne de présence pour ce jour.");
    return;
  }

  QString heureActuelle = QTime::currentTime().toString("HH:mm");

  QSqlQuery query;
  query.prepare("INSERT INTO Presences (agent_id, date, heure_arrivee, "
                "heure_depart, signature) "
                "VALUES (:agent_id, :date, :heure_arrivee, '', '')");
  query.bindValue(":agent_id", agentId);
  query.bindValue(":date", dateStr);
  query.bindValue(":heure_arrivee", heureActuelle);

  if (!query.exec()) {
    QMessageBox::critical(this, "Erreur", query.lastError().text());
  } else {
    refreshPresencesTable(datePresenceEdit->date());
  }
}

void MainWindow::pointerDepart() {
  int agentId = agentPresenceCombo->currentData().toInt();
  if (agentId == 0)
    return;

  QString dateStr = datePresenceEdit->date().toString("dd/MM/yyyy");
  QString heureActuelle = QTime::currentTime().toString("HH:mm");

  QSqlQuery query;
  // On met à jour l'heure de départ de la ligne existante pour aujourd'hui
  query.prepare("UPDATE Presences SET heure_depart = :heure_depart "
                "WHERE agent_id = :agent_id AND date = :date");
  query.bindValue(":heure_depart", heureActuelle);
  query.bindValue(":agent_id", agentId);
  query.bindValue(":date", dateStr);

  if (!query.exec()) {
    QMessageBox::critical(this, "Erreur", query.lastError().text());
  } else {
    if (query.numRowsAffected() == 0) {
      QMessageBox::warning(
          this, "Attention",
          "Aucune arrivée n'a été enregistrée pour cet agent aujourd'hui.");
    }
    refreshPresencesTable(datePresenceEdit->date());
  }
}

void MainWindow::refreshPresencesTable(const QDate &date) {
  if (!presencesQueryModel)
    return;

  QString dateStr = date.toString("dd/MM/yyyy");

  QString sql = QString("SELECT "
                        "p.id AS \"Numéro\", "
                        "ag.nom AS \"Nom\", "
                        "ag.postnom AS \"Post-nom\", "
                        "ag.matricule AS \"Matricule\", "
                        "ag.grade AS \"Grade\", "
                        "p.heure_arrivee AS \"Heure d'arrivée\", "
                        "COALESCE(p.heure_depart, '') AS \"Heure de départ\", "
                        "p.date AS \"Jour / Date\", "
                        "SUBSTR(p.date, 4, 7) AS \"Mois\", "
                        "COALESCE(p.signature, '') AS \"Signature\" "
                        "FROM Presences p "
                        "LEFT JOIN Agents ag ON p.agent_id = ag.id "
                        "WHERE p.date = '%1' "
                        "ORDER BY p.id DESC")
                    .arg(dateStr);

  presencesQueryModel->setQuery(sql);
}
