#include "mainwindow.hpp"
#include "DatabaseManager.hpp"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialisation de la base de données
    if (!DatabaseManager::instance().init()) {
        QMessageBox::critical(nullptr, "Erreur", "Impossible d'initialiser la base de données.");
        return -1;
    }

    MainWindow w;
    w.show();
    return QApplication::exec();
}
