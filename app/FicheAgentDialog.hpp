#ifndef FICHEAGENTDIALOG_HPP
#define FICHEAGENTDIALOG_HPP

#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>

class FicheAgentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FicheAgentDialog(QWidget *parent = nullptr);
    explicit FicheAgentDialog(int agentId, QWidget *parent = nullptr);
    ~FicheAgentDialog();

private slots:
    void saveToDatabase();

private:
    void setupUi();
    void loadAgentData();

    int m_agentId; // -1 for new
    
    QLineEdit *matriculeEdit;
    QLineEdit *nomEdit;
    QLineEdit *postnomEdit;
    QLineEdit *gradeEdit;
    QLineEdit *fonctionEdit;
    QComboBox *serviceCombo;
    QLineEdit *ministereEdit;
    QLineEdit *directionEdit;
    QDateEdit *dateEngagementEdit;
    QLineEdit *salaireEdit;
};

#endif // FICHEAGENTDIALOG_HPP
