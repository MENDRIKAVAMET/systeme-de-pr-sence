#ifndef MATIEREDIALOG_H
#define MATIEREDIALOG_H

#include <QDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QListWidgetItem>


QT_BEGIN_NAMESPACE
namespace Ui {
class MatiereDialog;
}
QT_END_NAMESPACE

class MatiereDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MatiereDialog(QWidget *parent = nullptr);
    ~MatiereDialog();

    void setModeAjout();
    void setModeModification(int id, QString code, QString nom, int volumeHoraire, int idClasse);

private slots:
    void on_btn_enregistrer_clicked();
    void on_btn_annuler_clicked();

private:
    Ui::MatiereDialog *ui;
    int m_id;
    bool m_ajout;
    void chargerClasses();  // charge les classes dans le comboBox;
    void chargerEnseignants();
    void sauvegarderEnseignants(int idMatiere);
};

#endif // MATIEREDIALOG_H
