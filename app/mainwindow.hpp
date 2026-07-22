#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QDialog>
#include <QStackedWidget>
#include <QListWidget>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QTableView>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSet>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QSortFilterProxyModel>
#include <QRegularExpression>

class AgentFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit AgentFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent), m_targetColumn(-1) {}

    void updateFilter() {
        invalidate();
    }

    void setTargetColumn(int col) {
        m_targetColumn = col;
        updateFilter();
    }

    void setFilterMinistere(const QString &min) {
        m_ministereFilter = min;
        updateFilter();
    }

    void setFilterSexe(const QString &sexe) {
        m_sexeFilter = sexe;
        updateFilter();
    }

    void setFilterRecensement(const QString &rec) {
        m_recensementFilter = rec;
        updateFilter();
    }

    void setColumnValueFilter(int col, const QStringList &allowedValues) {
        if (allowedValues.isEmpty()) {
            m_columnValueFilters.remove(col);
        } else {
            m_columnValueFilters[col] = allowedValues;
        }
        updateFilter();
    }

    void clearColumnFilter(int col) {
        m_columnValueFilters.remove(col);
        updateFilter();
    }

    bool hasColumnFilter(int col) const {
        return m_columnValueFilters.contains(col);
    }

    QStringList columnFilterValues(int col) const {
        return m_columnValueFilters.value(col);
    }

    void resetAllFilters() {
        m_targetColumn = -1;
        m_ministereFilter.clear();
        m_sexeFilter.clear();
        m_recensementFilter.clear();
        m_columnValueFilters.clear();
        setFilterRegularExpression("");
        updateFilter();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        QAbstractItemModel *model = sourceModel();
        if (!model) return true;

        QSqlTableModel *sqlModel = qobject_cast<QSqlTableModel*>(model);

        // 1. Filtrage Recherche Texte
        QRegularExpression regex = filterRegularExpression();
        if (!regex.pattern().isEmpty()) {
            bool textMatches = false;
            if (m_targetColumn >= 0) {
                QModelIndex index = model->index(sourceRow, m_targetColumn, sourceParent);
                if (model->data(index).toString().contains(regex)) {
                    textMatches = true;
                }
            } else {
                int colCount = model->columnCount(sourceParent);
                for (int c = 0; c < colCount; ++c) {
                    QModelIndex index = model->index(sourceRow, c, sourceParent);
                    if (model->data(index).toString().contains(regex)) {
                        textMatches = true;
                        break;
                    }
                }
            }
            if (!textMatches) return false;
        }

        // 2. Filtre Ministère (Détermination dynamique du champ BDD)
        if (!m_ministereFilter.isEmpty() && m_ministereFilter != "Tous les Ministères") {
            int minCol = sqlModel ? sqlModel->fieldIndex("ministere") : -1;
            if (minCol != -1) {
                QModelIndex idx = model->index(sourceRow, minCol, sourceParent);
                if (model->data(idx).toString().compare(m_ministereFilter, Qt::CaseInsensitive) != 0) {
                    return false;
                }
            }
        }

        // 3. Filtre Sexe (Détermination dynamique du champ BDD)
        if (!m_sexeFilter.isEmpty() && m_sexeFilter != "Tous les sexes") {
            int sexeCol = sqlModel ? sqlModel->fieldIndex("sexe") : -1;
            if (sexeCol != -1) {
                QModelIndex idx = model->index(sourceRow, sexeCol, sourceParent);
                if (model->data(idx).toString().compare(m_sexeFilter, Qt::CaseInsensitive) != 0) {
                    return false;
                }
            }
        }

        // 4. Filtre Recensement (Détermination dynamique du champ BDD)
        if (!m_recensementFilter.isEmpty() && m_recensementFilter != "Tous les statuts") {
            int recCol = sqlModel ? sqlModel->fieldIndex("statut_recensement") : -1;
            if (recCol != -1) {
                QModelIndex idx = model->index(sourceRow, recCol, sourceParent);
                if (model->data(idx).toString().compare(m_recensementFilter, Qt::CaseInsensitive) != 0) {
                    return false;
                }
            }
        }

        // 5. Filtres par valeurs spécifiques de colonne (via Clic/Double-Clic En-tête)
        for (auto it = m_columnValueFilters.begin(); it != m_columnValueFilters.end(); ++it) {
            int col = it.key();
            const QStringList &allowed = it.value();
            if (!allowed.isEmpty()) {
                QModelIndex idx = model->index(sourceRow, col, sourceParent);
                QString val = model->data(idx).toString();
                bool match = false;
                for (const QString &allowedVal : allowed) {
                    if (val.compare(allowedVal, Qt::CaseInsensitive) == 0) {
                        match = true;
                        break;
                    }
                }
                if (!match) return false;
            }
        }

        return true;
    }

private:
    int m_targetColumn;
    QString m_ministereFilter;
    QString m_sexeFilter;
    QString m_recensementFilter;
    QMap<int, QStringList> m_columnValueFilters;
};

/**
 * @brief Dialogue de Filtrage et Tri Spécifique à une Colonne (Style Tableur Pro / Excel)
 * 
 * Cette boîte de dialogue s'ouvre lors d'un double-clic sur l'en-tête d'une colonne du tableau.
 * Elle permet d'ordonner la colonne (croissant / décroissant) et de filtrer individuellement
 * par sélection parmi l'ensemble des valeurs uniques actuellement enregistrées.
 */
class ColumnFilterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ColumnFilterDialog(int colIndex, const QString &colName, QAbstractItemModel *model, const QStringList &initialActiveFilters, QWidget *parent = nullptr)
        : QDialog(parent), m_colIndex(colIndex), m_sortOrder(0), m_clearFilter(false)
    {
        setWindowTitle("Filtre & Tri - Colonne : " + colName);
        resize(380, 500);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(12);

        // --- Section 1 : Ordre de Tri (Croissant / Décroissant) ---
        QGroupBox *groupSort = new QGroupBox("Tri de la colonne", this);
        QHBoxLayout *sortLayout = new QHBoxLayout(groupSort);

        QPushButton *btnAsc = new QPushButton("▲ Tri Croissant (A-Z)", groupSort);
        QPushButton *btnDesc = new QPushButton("▼ Tri Décroissant (Z-A)", groupSort);

        btnAsc->setCursor(Qt::PointingHandCursor);
        btnDesc->setCursor(Qt::PointingHandCursor);
        btnAsc->setStyleSheet("padding: 8px; background-color: #3498db; color: white; border-radius: 4px; font-weight: bold;");
        btnDesc->setStyleSheet("padding: 8px; background-color: #e67e22; color: white; border-radius: 4px; font-weight: bold;");

        // Action de sélection de tri
        connect(btnAsc, &QPushButton::clicked, this, [this]() { m_sortOrder = 1; accept(); });
        connect(btnDesc, &QPushButton::clicked, this, [this]() { m_sortOrder = 2; accept(); });

        sortLayout->addWidget(btnAsc);
        sortLayout->addWidget(btnDesc);
        mainLayout->addWidget(groupSort);

        // --- Section 2 : Liste des Valeurs Uniques avec Cases à Cocher ---
        QGroupBox *groupFilter = new QGroupBox("Valeurs présentes (" + colName + ")", this);
        QVBoxLayout *filterLayout = new QVBoxLayout(groupFilter);

        // Champ de recherche rapide dans la liste des valeurs
        QLineEdit *searchEdit = new QLineEdit(groupFilter);
        searchEdit->setPlaceholderText("Chercher une valeur dans cette liste...");
        searchEdit->setClearButtonEnabled(true);
        filterLayout->addWidget(searchEdit);

        listWidget = new QListWidget(groupFilter);
        listWidget->setSelectionMode(QAbstractItemView::NoSelection);

        // Extraction et dédoublonnage des valeurs uniques de cette colonne
        QSet<QString> uniqueVals;
        for (int r = 0; r < model->rowCount(); ++r) {
            QString val = model->data(model->index(r, colIndex)).toString().trimmed();
            if (!val.isEmpty()) {
                uniqueVals.insert(val);
            } else {
                uniqueVals.insert("(Vide)");
            }
        }

        // Tri alphabétique des valeurs dans la liste du filtre
        QStringList sortedVals = uniqueVals.values();
        sortedVals.sort(Qt::CaseInsensitive);

        // Remplissage de la liste avec état de coche
        for (const QString &val : sortedVals) {
            QListWidgetItem *item = new QListWidgetItem(val, listWidget);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            bool isChecked = initialActiveFilters.isEmpty() || initialActiveFilters.contains(val, Qt::CaseInsensitive);
            item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
        }

        filterLayout->addWidget(listWidget);

        // Boutons de sélection rapide (Tout cocher / Tout décocher)
        QHBoxLayout *selBtnLayout = new QHBoxLayout();
        QPushButton *btnSelectAll = new QPushButton("Tout Sélectionner", groupFilter);
        QPushButton *btnUnselectAll = new QPushButton("Tout Décocher", groupFilter);

        selBtnLayout->addWidget(btnSelectAll);
        selBtnLayout->addWidget(btnUnselectAll);
        filterLayout->addLayout(selBtnLayout);

        // Connexion des boutons Tout Sélectionner / Tout Décocher
        connect(btnSelectAll, &QPushButton::clicked, [this]() {
            for (int i = 0; i < listWidget->count(); ++i) {
                if (!listWidget->item(i)->isHidden()) {
                    listWidget->item(i)->setCheckState(Qt::Checked);
                }
            }
        });
        connect(btnUnselectAll, &QPushButton::clicked, [this]() {
            for (int i = 0; i < listWidget->count(); ++i) {
                if (!listWidget->item(i)->isHidden()) {
                    listWidget->item(i)->setCheckState(Qt::Unchecked);
                }
            }
        });

        // Recherche en temps réel dans la liste des valeurs
        connect(searchEdit, &QLineEdit::textChanged, [this](const QString &txt) {
            for (int i = 0; i < listWidget->count(); ++i) {
                QListWidgetItem *item = listWidget->item(i);
                item->setHidden(!item->text().contains(txt, Qt::CaseInsensitive));
            }
        });

        mainLayout->addWidget(groupFilter);

        // --- Section 3 : Boutons d'Action de Validation ---
        QHBoxLayout *actionLayout = new QHBoxLayout();
        QPushButton *btnApply = new QPushButton("Appliquer le filtre", this);
        QPushButton *btnClear = new QPushButton("Réinitialiser cette colonne", this);

        btnApply->setCursor(Qt::PointingHandCursor);
        btnClear->setCursor(Qt::PointingHandCursor);
        btnApply->setStyleSheet("padding: 8px 16px; background-color: #27ae60; color: white; font-weight: bold; border-radius: 4px;");
        btnClear->setStyleSheet("padding: 8px 16px; background-color: #7f8c8d; color: white; border-radius: 4px;");

        connect(btnApply, &QPushButton::clicked, this, &QDialog::accept);
        connect(btnClear, &QPushButton::clicked, this, [this]() {
            m_clearFilter = true;
            accept();
        });

        actionLayout->addWidget(btnClear);
        actionLayout->addWidget(btnApply);
        mainLayout->addLayout(actionLayout);
    }

    int sortOrder() const { return m_sortOrder; } // 0: Aucun, 1: Ascendant, 2: Descendant
    bool clearFilterRequested() const { return m_clearFilter; }

    /**
     * @brief Récupère la liste des valeurs sélectionnées par l'utilisateur
     */
    QStringList selectedValues() const {
        QStringList result;
        for (int i = 0; i < listWidget->count(); ++i) {
            QListWidgetItem *item = listWidget->item(i);
            if (item->checkState() == Qt::Checked) {
                result.append(item->text());
            }
        }
        return result;
    }

private:
    int m_colIndex;
    int m_sortOrder;
    bool m_clearFilter;
    QListWidget *listWidget;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &userRole = "Admin", const QString &userName = "Admin", QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void changePage(int index);
    void generateMockData();

private:
    void setupUi();
    QWidget* createDashboardPage();
    QWidget* createRhPage();
    QWidget* createSecteursSociauxPage();
    QWidget* createSecteursProductifsPage();
    QWidget* createInfrastructuresPage();
    QWidget* createSecretariatPage();

    QListWidget *sidebar;
    QStackedWidget *stackedWidget;
    
    // Modèles de données
    QSqlTableModel *agentModel;
    AgentFilterProxyModel *agentProxyModel;
    QSqlQueryModel *presencesQueryModel;
    QSqlTableModel *orgModel;
    QSqlTableModel *prodModel;
    QSqlTableModel *infraModel;

    // UI Présences
    QComboBox *agentPresenceCombo;
    QDateEdit *datePresenceEdit;

    void loadPresenceAgents();
    void pointerArrivee();
    void pointerDepart();
    void refreshPresencesTable(const QDate &date);
    
    // Dashboard
    QLabel *lblTotalAgents;
    QLabel *lblPresenceRate;
    QChart *barChart;
    QChart *pieChart;
    void refreshDashboard();

    // Contrôle d'accès
    QString m_userRole;
};

#endif // MAINWINDOW_HPP
