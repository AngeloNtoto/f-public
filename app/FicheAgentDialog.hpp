#ifndef FICHEAGENTDIALOG_HPP
#define FICHEAGENTDIALOG_HPP

#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>

#include <QTableWidget>

class FicheAgentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FicheAgentDialog(QWidget *parent = nullptr);
    explicit FicheAgentDialog(int agentId, QWidget *parent = nullptr);
    ~FicheAgentDialog();

private slots:
    void saveToDatabase();
    void ajouterConge();

private:
    void setupUi();
    void loadAgentData();
    void loadCongesData();
    QWidget* createDynamicField(QComboBox *combo, const QString &category, const QString &labelTitle);
    void loadDynamicOptions(QComboBox *combo, const QString &category);
    void addOptionInteractively(QComboBox *combo, const QString &category, const QString &labelTitle);
    void removeOptionInteractively(QComboBox *combo, const QString &category, const QString &labelTitle);

    int m_agentId; // -1 for new

    // --- Champs D (Dynamiques : QComboBox + Bouton +) ---
    QComboBox *ministereCombo;
    QComboBox *secretariatGeneralCombo;
    QComboBox *directionCombo;
    QComboBox *divisionCombo;
    QComboBox *bureauCombo;
    QComboBox *fonctionCombo;
    QComboBox *niveauEtudeCombo;
    QComboBox *filiereCombo;
    QComboBox *positionAdminCombo;
    QComboBox *lieuAffectationCombo;

    // --- Champs M (Manuels : QLineEdit / QDateEdit) ---
    QLineEdit *nomEdit;
    QLineEdit *postnomEdit;
    QLineEdit *prenomEdit;
    QDateEdit *dateNaissanceEdit;
    QLineEdit *matriculeEdit;
    QLineEdit *gradeEdit;
    QDateEdit *dateEngagementEdit;
    QLineEdit *refEngagementEdit;
    QLineEdit *refNominationEdit;
    QLineEdit *refCommissionnementEdit;
    QLineEdit *telephoneEdit;
    QLineEdit *emailEdit;
    QLineEdit *adresseEdit;

    // --- Champs O (Optionnels / Liste Fixe : QComboBox) ---
    QComboBox *sexeCombo;
    QComboBox *etatCivilCombo;
    QComboBox *salaireCombo;
    QComboBox *primeCombo;
    QComboBox *statutRecensementCombo;
    QComboBox *serviceCombo;
    QComboBox *carteBiometriqueCombo;
    QComboBox *dossierPhysiqueCombo;
    QComboBox *statutCarriereCombo;

    // --- Champs C (Constants : QLineEdit en lecture seule / préremplis) ---
    QLineEdit *domaineEdit;
    QLineEdit *niveauAffectationEdit;
    QLineEdit *provinceEdit;

    // --- Module Congés et Absences ---
    QComboBox *typeCongeCombo;
    QDateEdit *dateDebutCongeEdit;
    QDateEdit *dateFinCongeEdit;
    QLineEdit *motifCongeEdit;
    QTableWidget *congesTable;
};

#endif // FICHEAGENTDIALOG_HPP
