#ifndef ENSEIGNANTDIALOG_H
#define ENSEIGNANTDIALOG_H

#include <QDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class EnseignantDialog;
}
QT_END_NAMESPACE

class EnseignantDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnseignantDialog(QWidget *parent = nullptr);
    ~EnseignantDialog();

    void setModeAjout();
    void setModeModification(int id, QString nom, QString prenoms, QString email, QString telephone);

private slots:
    void on_btn_enregistrer_clicked();
    void on_btn_annuler_clicked();

private:
    Ui::EnseignantDialog *ui;
    int m_id;
    bool m_ajout;
};

#endif // ENSEIGNANTDIALOG_H
