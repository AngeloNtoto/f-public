#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QTableView>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &userRole = "Admin", const QString &userName = "Admin", QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void changePage(int index);

private:
    void setupUi();
    QWidget* createDashboardPage();
    QWidget* createRhPage();
    QWidget* createSecteursSociauxPage();
    QWidget* createSecretariatPage();

    QListWidget *sidebar;
    QStackedWidget *stackedWidget;
    
    // Modèles de données
    QSqlTableModel *agentModel;
    QSqlRelationalTableModel *presencesModel;
    QSqlTableModel *orgModel;

    // UI Présences
    QComboBox *agentPresenceCombo;
    QDateEdit *datePresenceEdit;

    void loadPresenceAgents();
    void pointerArrivee();
    void pointerDepart();
    
    // Dashboard
    QLabel *lblTotalAgents;
    QLabel *lblPresenceRate;
    QChart *barChart;
    QChart *pieChart;
    void refreshDashboard();
};

#endif // MAINWINDOW_HPP
