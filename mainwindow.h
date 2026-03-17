#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btn_dashboard_clicked();
    void on_btn_etudiants_clicked();
    void on_btn_enseignants_clicked();
    void on_btn_matieres_clicked();
    void on_btn_seances_clicked();
    void on_btn_rapports_clicked();
    void on_btn_deconnexion_clicked();
    void on_btn_ajouter_etudiant_clicked();
    void on_btn_modifier_etudiant_clicked();
    void on_btn_supprimer_etudiant_clicked();
    void on_btn_ajouter_enseignant_clicked();
    void on_btn_modifier_enseignant_clicked();
    void on_btn_supprimer_enseignant_clicked();
    void on_btn_ajouter_matiere_clicked();
    void on_btn_modifier_matiere_clicked();
    void on_btn_supprimer_matiere_clicked();
    void on_btn_ajouter_seance_clicked();
    void on_btn_modifier_seance_clicked();
    void on_btn_supprimer_seance_clicked();

private:
    Ui::MainWindow *ui;
    void chargerDashboard();
    void chargerEtudiants();
    void chargerEnseignants();
    void chargerMatieres();
    void chargerSeances();

};

#endif // MAINWINDOW_H
