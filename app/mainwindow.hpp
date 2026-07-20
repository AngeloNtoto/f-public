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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
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

    // UI Présences
    QComboBox *agentPresenceCombo;
    QDateEdit *datePresenceEdit;

    void loadPresenceAgents();
    void pointerArrivee();
    void pointerDepart();
};

#endif // MAINWINDOW_HPP
