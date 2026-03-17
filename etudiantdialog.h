#ifndef ETUDIANTDIALOG_H
#define ETUDIANTDIALOG_H

#include <QDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class EtudiantDialog;
}
QT_END_NAMESPACE

class EtudiantDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EtudiantDialog(QWidget *parent = nullptr);
    ~EtudiantDialog();

    // Mode ajout ou modification
    void setModeAjout();
    void setModeModification(QString numInsc, QString nom, QString prenoms);

private slots:
    void on_btn_enregistrer_clicked();
    void on_btn_annuler_clicked();

private:
    Ui::EtudiantDialog *ui;
    int m_id;        // id de l'étudiant en cours de modification
    bool m_ajout;    // true = ajout, false = modification
};

#endif // ETUDIANTDIALOG_H
