#ifndef IDENTIFICATIONDIALOG_HPP
#define IDENTIFICATIONDIALOG_HPP

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTableWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>

class IdentificationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IdentificationDialog(QWidget *parent = nullptr);
    ~IdentificationDialog();

private slots:
    void saveToDatabase();
    void addDirigeantRow();
    void addProjetRow();

private:
    void setupUi();
    
    // Tab 1
    QLineEdit *nomEdit;
    QLineEdit *sigleEdit;
    QComboBox *natureCombo;
    QDateEdit *dateCreationEdit;
    QLineEdit *adresseEdit;
    QLineEdit *telephoneEdit;
    QLineEdit *emailEdit;
    QLineEdit *representantEdit;

    // Tab 2
    QTableWidget *dirigeantsTable;

    // Tab 3
    QList<QCheckBox*> domainesChecks;
    QList<QCheckBox*> zonesChecks;

    // Tab 4
    QTableWidget *projetsTable;

    // Tab 5
    QCheckBox *docStatutsCheck;
    QCheckBox *docF3Check;
    QComboBox *decisionCombo;

    QPushButton *btnSave;
    QPushButton *btnCancel;
};

#endif // IDENTIFICATIONDIALOG_HPP
