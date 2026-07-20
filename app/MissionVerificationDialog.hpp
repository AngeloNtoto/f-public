#ifndef MISSIONVERIFICATIONDIALOG_HPP
#define MISSIONVERIFICATIONDIALOG_HPP

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QDateEdit>

class MissionVerificationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MissionVerificationDialog(int orgId, const QString& orgName, QWidget *parent = nullptr);
    ~MissionVerificationDialog();

private slots:
    void saveAndGeneratePdf();

private:
    void setupUi();

    int m_orgId;
    QString m_orgName;

    QDateEdit *dateMissionEdit;
    QLineEdit *equipeEdit;
    QComboBox *gouvernanceCombo;
    QComboBox *rhCombo;
    QTextEdit *constatsEdit;
    QComboBox *conclusionCombo;
};

#endif // MISSIONVERIFICATIONDIALOG_HPP
