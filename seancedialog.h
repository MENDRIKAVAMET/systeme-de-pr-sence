#ifndef SEANCEDIALOG_H
#define SEANCEDIALOG_H

#include <QDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class SeanceDialog;
}
QT_END_NAMESPACE

class SeanceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SeanceDialog(QWidget *parent = nullptr);
    ~SeanceDialog();

    void setModeAjout();
    void setModeModification(int id, int idMatiere, int idClasse,
                             QDate date, QTime heureDebut, QTime heureFin);

private slots:
    void on_btn_enregistrer_clicked();
    void on_btn_annuler_clicked();

private:
    Ui::SeanceDialog *ui;
    int m_id;
    bool m_ajout;
    void chargerMatieres();
    void chargerClasses();
    void chargerEnseignants();
    void sauvegarderEnseignants(int idSeance);
};

#endif // SEANCEDIALOG_H
