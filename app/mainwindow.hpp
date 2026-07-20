#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QSqlTableModel>
#include <QTableView>

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
};

#endif // MAINWINDOW_HPP
