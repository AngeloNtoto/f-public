#ifndef LOGINDIALOG_HPP
#define LOGINDIALOG_HPP

#include <QDialog>
#include <QLineEdit>

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    QString getUserRole() const { return m_role; }
    QString getUserName() const { return m_username; }

private slots:
    void attemptLogin();

private:
    QLineEdit *userEdit;
    QLineEdit *passEdit;
    QString m_role;
    QString m_username;
};

#endif // LOGINDIALOG_HPP
