#ifndef MISSIONVERIFICATIONDIALOG_HPP
#define MISSIONVERIFICATIONDIALOG_HPP

// Dialogue de rapport de mission de vérification sur terrain
// Secteurs Sociaux - Province du Kwilu

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QTableWidget>

class MissionVerificationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MissionVerificationDialog(int orgId, const QString& orgName, QWidget *parent = nullptr);
    ~MissionVerificationDialog();

private slots:
    void saveAndGeneratePdf();
    void addEquipeRow();

private:
    void setupUi();

    int m_orgId;
    QString m_orgName;

    // Équipe de mission
    QTableWidget *equipeTable;

    // Situation administrative et juridique
    QTextEdit *situationAdminEdit;

    // Gouvernance et fonctionnement institutionnel
    QComboBox *agFonctionnelleCombo;
    QComboBox *caCombo;
    QComboBox *comiteGestionCombo;
    QComboBox *coordExecCombo;

    // Ressources humaines et matérielles
    QTextEdit *rhMaterielEdit;

    // Vérification des projets et activités
    QTextEdit *verificationProjetsEdit;
    QComboBox *niveauExecutionCombo;

    // Impact communautaire
    QTextEdit *impactEdit;

    // Contraintes et insuffisances
    QTextEdit *contraintesEdit;

    // Conclusion
    QComboBox *conclusionCombo;

    // Recommandations
    QTextEdit *recoStructureEdit;
    QTextEdit *recoDivisionEdit;
};

#endif // MISSIONVERIFICATIONDIALOG_HPP
