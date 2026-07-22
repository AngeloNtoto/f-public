#ifndef IDENTIFICATIONDIALOG_HPP
#define IDENTIFICATIONDIALOG_HPP

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTextEdit>
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
    void addEffectifRow();
    void addPartenaireRow();
    void addProjetRow();

private:
    void setupUi();
    
    // Tab 1 : Identification & Représentant Légal
    QLineEdit *numEnregistrementEdit;
    QDateEdit *dateReceptionEdit;
    QLineEdit *nomEdit;
    QLineEdit *sigleEdit;
    QComboBox *natureCombo;
    QDateEdit *dateCreationEdit;
    QDateEdit *dateDebutProvinceEdit;
    QLineEdit *numPersoJuridiqueEdit;
    QLineEdit *autoriteDelivranceEdit;
    QLineEdit *adresseSiegeEdit;
    QLineEdit *adresseProvinceEdit;
    QLineEdit *telephoneEdit;
    QLineEdit *emailEdit;
    QLineEdit *siteWebEdit;

    QLineEdit *repNomEdit;
    QLineEdit *repFonctionEdit;
    QLineEdit *repNationaliteEdit;
    QLineEdit *repPhoneEdit;
    QLineEdit *repEmailEdit;
    QLineEdit *repAdresseEdit;

    // Tab 2 : Dirigeants & Effectifs
    QTableWidget *dirigeantsTable;
    QTableWidget *effectifsTable;

    // Tab 3 : Domaines & Zones d'Intervention
    QList<QCheckBox*> domainesChecks;
    QList<QCheckBox*> zonesChecks;

    // Tab 4 : Partenaires & Projets
    QTableWidget *partenairesTable;
    QTableWidget *projetsTable;

    // Tab 5 : Documents Annexes
    QList<QCheckBox*> annexesChecks;

    // Tab 6 : Déclarations & Décision
    QTextEdit *observationsEdit;
    QComboBox *decisionCombo;

    QPushButton *btnSave;
    QPushButton *btnCancel;
};

#endif // IDENTIFICATIONDIALOG_HPP
