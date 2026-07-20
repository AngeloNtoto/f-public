#include "LoginDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSqlQuery>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Connexion - Fonction Publique");
    setFixedSize(350, 200);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QLabel *title = new QLabel("<h2>Identification requise</h2>");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    QHBoxLayout *userLayout = new QHBoxLayout();
    userLayout->addWidget(new QLabel("Identifiant :"));
    userEdit = new QLineEdit();
    userLayout->addWidget(userEdit);
    mainLayout->addLayout(userLayout);

    QHBoxLayout *passLayout = new QHBoxLayout();
    passLayout->addWidget(new QLabel("Mot de passe :"));
    passEdit = new QLineEdit();
    passEdit->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(passEdit);
    mainLayout->addLayout(passLayout);

    QPushButton *btnLogin = new QPushButton("Se Connecter");
    btnLogin->setStyleSheet("padding: 10px; background-color: #2c3e50; color: white; border-radius: 4px; font-weight: bold;");
    mainLayout->addWidget(btnLogin);

    connect(btnLogin, &QPushButton::clicked, this, &LoginDialog::attemptLogin);
}

void LoginDialog::attemptLogin()
{
    QString username = userEdit->text();
    QString password = passEdit->text();

    QSqlQuery q;
    q.prepare("SELECT role FROM Users WHERE username = ? AND password = ?");
    q.addBindValue(username);
    q.addBindValue(password);
    
    if (q.exec() && q.next()) {
        m_role = q.value(0).toString();
        m_username = username;
        accept();
    } else {
        QMessageBox::warning(this, "Erreur", "Identifiants incorrects.");
    }
}
