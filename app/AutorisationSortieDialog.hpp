#ifndef AUTORISATIONSORTIEDIALOG_HPP
#define AUTORISATIONSORTIEDIALOG_HPP

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>

class AutorisationSortieDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AutorisationSortieDialog(QWidget *parent = nullptr);
    ~AutorisationSortieDialog();

private slots:
    void saveAndGeneratePdf();

private:
    void setupUi();
    void loadAgents();
    void generatePdf(int autorisationId);

    QComboBox *agentCombo;
    QLineEdit *destinationEdit;
    QLineEdit *motifEdit;
    QDateEdit *dateDepartEdit;
    QDateEdit *dateRetourEdit;
    QComboBox *transportCombo;
    QLineEdit *mentionsEdit;

    QPushButton *btnSave;
    QPushButton *btnCancel;
};

#endif // AUTORISATIONSORTIEDIALOG_HPP
