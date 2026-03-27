#ifndef ENSEIGNANTWINDOW_H
#define ENSEIGNANTWINDOW_H

#include <QMainWindow>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class EnseignantWindow;
}
QT_END_NAMESPACE

class EnseignantWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EnseignantWindow(int idEnseignant, QString nomEnseignant, QWidget *parent = nullptr);
    ~EnseignantWindow();

private slots:
    void on_btn_deconnexion_clicked();
    void on_btn_demarrer_presence_clicked();
    void on_btn_terminer_clicked();
    void nouvelleDonneeRecue();
    void nouveauClient();
    void on_btn_historique_clicked();
    void on_btn_retour_historique_clicked();
    void on_comboBox_seances_hist_currentIndexChanged(int index);

private:
    Ui::EnseignantWindow *ui;
    int m_idEnseignant;
    QString m_nomEnseignant;
    int m_idSeanceEnCours;

    QTcpServer *m_serveur;

    void chargerSeancesJour();
    void demarrerServeur();
    void arreterServeur();
    QString traiterQRCode(QString numeroInscription);
    void chargerHistorique();
    void chargerSeancesHistorique();
};

#endif // ENSEIGNANTWINDOW_H
