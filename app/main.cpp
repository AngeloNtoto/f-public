#include "mainwindow.hpp"
#include "DatabaseManager.hpp"
#include "LoginDialog.hpp"

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

    LoginDialog login;
    if (login.exec() == QDialog::Accepted) {
        MainWindow w(login.getUserRole(), login.getUserName());
        w.show();
        return QApplication::exec();
    }
    
    return 0;
}
