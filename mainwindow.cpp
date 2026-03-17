#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loginwindow.h"
#include <QHeaderView>
#include <QMessageBox>
#include "etudiantdialog.h"
#include "enseignantdialog.h"
#include "matieredialog.h"
#include "seancedialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //afficher le tableau de bord par défaut
    ui->stackedWidget->setCurrentIndex(1);
    chargerDashboard();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_dashboard_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    chargerDashboard();
}

void MainWindow::on_btn_etudiants_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    chargerEtudiants();
}

void MainWindow::on_btn_enseignants_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    chargerEnseignants();
}

void MainWindow::on_btn_matieres_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);
    chargerMatieres();
}

void MainWindow::on_btn_seances_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    chargerSeances();
}

void MainWindow::on_btn_rapports_clicked()
{
    ui->stackedWidget->setCurrentIndex(4);
}

void MainWindow::on_btn_deconnexion_clicked()
{
    //Fermer MainWindow et rouvrir LoginWindow
    LoginWindow *login = new LoginWindow();
    login->show();
    this->close();
}

void MainWindow::chargerDashboard()
{
    QSqlDatabase db = QSqlDatabase::database();
    if(!db.isOpen()){
        QMessageBox::warning(this, "Erreur", "Base de données non connectée !");
        return;
    }

    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM etudiants");

    if(query.next())
    {
        ui->label_nb_etudiants->setText(query.value(0).toString());
    }

    query.exec("SELECT COUNT(*) FROM enseignants");
    if(query.next())
    {
        ui->label_nb_enseignants->setText(query.value(0).toString());
    }

    query.exec("SELECT COUNT(*) FROM matieres");
    if(query.next())
    {
        ui->label_nb_matieres->setText(query.value(0).toString());
    }

    query.exec("SELECT COUNT(*) FROM seances");
    if(query.next())
    {
        ui->label_nb_seances->setText(query.value(0).toString());
    }
}

void MainWindow::chargerEtudiants()
{
    ui->table_etudiants->setAlternatingRowColors(true);
    ui->table_etudiants->clearContents();
    ui->table_etudiants->setRowCount(0);

    ui->table_etudiants->setColumnCount(3);
    ui->table_etudiants->setHorizontalHeaderLabels(
        QStringList() << "N° Inscription" << "Nom" << "Prénoms"
        );

    QSqlQuery query;
    query.exec("SELECT numero_inscription, nom, prenoms FROM etudiants ORDER BY nom, prenoms");

    int row = 0;
    while(query.next()){
        ui->table_etudiants->insertRow(row);
        ui->table_etudiants->setItem(row, 0, new QTableWidgetItem(query.value("numero_inscription").toString()));
        ui->table_etudiants->setItem(row, 1, new QTableWidgetItem(query.value("nom").toString()));
        ui->table_etudiants->setItem(row, 2, new QTableWidgetItem(query.value("prenoms").toString()));
        row++;
    }
    ui->table_etudiants->horizontalHeader()->setStretchLastSection(true);
    ui->table_etudiants->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_etudiants->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void MainWindow::on_btn_supprimer_etudiant_clicked()
{
    // Vérifier qu'une ligne est sélectionnée
    int row = ui->table_etudiants->currentRow();
    if (row == -1) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un étudiant !");
        return;
    }

    // Récupérer le nom de l'étudiant
    QString nom = ui->table_etudiants->item(row, 1)->text();
    QString prenom = ui->table_etudiants->item(row, 2)->text();

    // Demander confirmation
    QMessageBox::StandardButton reponse = QMessageBox::question(
        this, "Confirmation",
        "Voulez-vous supprimer l'étudiant " + nom + " " + prenom + " ?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reponse == QMessageBox::Yes) {
        QString id = ui->table_etudiants->item(row, 0)->text();
        QSqlQuery query;
        query.prepare("DELETE FROM etudiants WHERE numero_inscription = :id");
        query.bindValue(":id", id);

        if (query.exec()) {
            QMessageBox::information(this, "Succès", "Étudiant supprimé avec succès !");
            chargerEtudiants();
            chargerDashboard();
        } else {
            QMessageBox::critical(this, "Erreur", "Erreur lors de la suppression !");
        }
    }
}

void MainWindow::on_btn_ajouter_etudiant_clicked()
{
    EtudiantDialog dialog(this);
    dialog.setModeAjout();

    if (dialog.exec() == QDialog::Accepted) {
        chargerEtudiants();
        chargerDashboard();
    }
}

void MainWindow::on_btn_modifier_etudiant_clicked()
{
    int row = ui->table_etudiants->currentRow();

    if (row < 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un étudiant !");
        return;
    }

    // Récupérer les données de la ligne sélectionnée
    QString numInsc = ui->table_etudiants->item(row, 0)->text();
    QString nom = ui->table_etudiants->item(row, 1)->text();
    QString prenoms = ui->table_etudiants->item(row, 2)->text();

    EtudiantDialog dialog(this);
    dialog.setModeModification(numInsc, nom, prenoms);

    if (dialog.exec() == QDialog::Accepted) {
        chargerEtudiants();
        chargerDashboard();
    }
}

void MainWindow::chargerEnseignants()
{
    ui->table_enseignants->clearContents();
    ui->table_enseignants->setRowCount(0);

    ui->table_enseignants->setColumnCount(5);
    ui->table_enseignants->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Nom" << "Prénoms" << "Email" << "Téléphone"
        );

    QSqlQuery query;
    query.exec("SELECT id, nom, prenoms, email, telephone FROM enseignants ORDER BY nom, prenoms");

    int row = 0;
    while (query.next()) {
        ui->table_enseignants->insertRow(row);
        ui->table_enseignants->setItem(row, 0, new QTableWidgetItem(query.value("id").toString()));
        ui->table_enseignants->setItem(row, 1, new QTableWidgetItem(query.value("nom").toString()));
        ui->table_enseignants->setItem(row, 2, new QTableWidgetItem(query.value("prenoms").toString()));
        ui->table_enseignants->setItem(row, 3, new QTableWidgetItem(query.value("email").toString()));
        ui->table_enseignants->setItem(row, 4, new QTableWidgetItem(query.value("telephone").toString()));
        row++;
    }

    ui->table_enseignants->horizontalHeader()->setStretchLastSection(true);
    ui->table_enseignants->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_enseignants->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_enseignants->setAlternatingRowColors(true);
}

void MainWindow::on_btn_ajouter_enseignant_clicked()
{
    EnseignantDialog dialog(this);
    dialog.setModeAjout();

    if (dialog.exec() == QDialog::Accepted) {
        chargerEnseignants();
        chargerDashboard();
    }
}

void MainWindow::on_btn_modifier_enseignant_clicked()
{
    int row = ui->table_enseignants->currentRow();

    if (row < 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un enseignant !");
        return;
    }

    int id = ui->table_enseignants->item(row, 0)->text().toInt();
    QString nom = ui->table_enseignants->item(row, 1)->text();
    QString prenoms = ui->table_enseignants->item(row, 2)->text();
    QString email = ui->table_enseignants->item(row, 3)->text();
    QString telephone = ui->table_enseignants->item(row, 4)->text();

    EnseignantDialog dialog(this);
    dialog.setModeModification(id, nom, prenoms, email, telephone);

    if (dialog.exec() == QDialog::Accepted) {
        chargerEnseignants();
        chargerDashboard();
    }
}

void MainWindow::on_btn_supprimer_enseignant_clicked()
{
    int row = ui->table_enseignants->currentRow();

    if (row < 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner un enseignant !");
        return;
    }

    QString nom = ui->table_enseignants->item(row, 1)->text();
    QString prenoms = ui->table_enseignants->item(row, 2)->text();

    QMessageBox::StandardButton reponse = QMessageBox::question(
        this, "Confirmation",
        "Voulez-vous supprimer l'enseignant " + nom + " " + prenoms + " ?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reponse == QMessageBox::Yes) {
        QString id = ui->table_enseignants->item(row, 0)->text();
        QSqlQuery query;
        query.prepare("DELETE FROM enseignants WHERE id = :id");
        query.bindValue(":id", id);

        if (query.exec()) {
            QMessageBox::information(this, "Succès", "Enseignant supprimé avec succès !");
            chargerEnseignants();
            chargerDashboard();
        } else {
            QMessageBox::critical(this, "Erreur", "Erreur : " + query.lastError().text());
        }
    }
}

void MainWindow::chargerMatieres()
{
    ui->table_matieres->clearContents();
    ui->table_matieres->setRowCount(0);

    ui->table_matieres->setColumnCount(5);
    ui->table_matieres->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Code" << "Nom" << "Volume horaire" << "Classe"
        );

    QSqlQuery query;
    query.exec("SELECT m.id, m.code, m.nom, m.volume_horaire, c.nom as classe "
               "FROM matieres m "
               "JOIN classes c ON m.id_classe = c.id "
               "ORDER BY m.nom");

    int row = 0;
    while (query.next()) {
        ui->table_matieres->insertRow(row);
        ui->table_matieres->setItem(row, 0, new QTableWidgetItem(query.value("id").toString()));
        ui->table_matieres->setItem(row, 1, new QTableWidgetItem(query.value("code").toString()));
        ui->table_matieres->setItem(row, 2, new QTableWidgetItem(query.value("nom").toString()));
        ui->table_matieres->setItem(row, 3, new QTableWidgetItem(query.value("volume_horaire").toString() + " h"));
        ui->table_matieres->setItem(row, 4, new QTableWidgetItem(query.value("classe").toString()));
        row++;
    }

    ui->table_matieres->hideColumn(0);
    ui->table_matieres->horizontalHeader()->setStretchLastSection(true);
    ui->table_matieres->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_matieres->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_matieres->setAlternatingRowColors(true);
}

void MainWindow::on_btn_ajouter_matiere_clicked()
{
    MatiereDialog dialog(this);
    dialog.setModeAjout();

    if (dialog.exec() == QDialog::Accepted) {
        chargerMatieres();
        chargerDashboard();
    }
}

void MainWindow::on_btn_modifier_matiere_clicked()
{
    int row = ui->table_matieres->currentRow();

    if (row < 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une matière !");
        return;
    }

    int id = ui->table_matieres->item(row, 0)->text().toInt();
    QString code = ui->table_matieres->item(row, 1)->text();
    QString nom = ui->table_matieres->item(row, 2)->text();
    int volume = ui->table_matieres->item(row, 3)->text().replace(" h", "").toInt();

    // Récupérer l'id de la classe
    QSqlQuery query;
    query.prepare("SELECT id_classe FROM matieres WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();
    int idClasse = 0;
    if (query.next())
        idClasse = query.value("id_classe").toInt();

    MatiereDialog dialog(this);
    dialog.setModeModification(id, code, nom, volume, idClasse);

    if (dialog.exec() == QDialog::Accepted) {
        chargerMatieres();
        chargerDashboard();
    }
}

void MainWindow::on_btn_supprimer_matiere_clicked()
{
    int row = ui->table_matieres->currentRow();

    if (row < 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une matière !");
        return;
    }

    QString nom = ui->table_matieres->item(row, 2)->text();

    QMessageBox::StandardButton reponse = QMessageBox::question(
        this, "Confirmation",
        "Voulez-vous supprimer la matière " + nom + " ?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reponse == QMessageBox::Yes) {
        QString id = ui->table_matieres->item(row, 0)->text();
        QSqlQuery query;
        query.prepare("DELETE FROM matieres WHERE id = :id");
        query.bindValue(":id", id);

        if (query.exec()) {
            QMessageBox::information(this, "Succès", "Matière supprimée avec succès !");
            chargerMatieres();
            chargerDashboard();
        } else {
            QMessageBox::critical(this, "Erreur", "Erreur : " + query.lastError().text());
        }
    }
}

void MainWindow::chargerSeances()
{
    ui->table_seances->clearContents();
    ui->table_seances->setRowCount(0);

    ui->table_seances->setColumnCount(7);
    ui->table_seances->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Matière" << "Classe" << "Date" << "Heure début" << "Heure fin" << "Validée"
        );

    QSqlQuery query;
    query.exec("SELECT s.id, m.nom as matiere, c.nom as classe, "
               "s.date_seance, s.heure_debut, s.heure_fin, s.validee, "
               "s.id_matiere, s.id_classe "
               "FROM seances s "
               "JOIN matieres m ON s.id_matiere = m.id "
               "JOIN classes c ON s.id_classe = c.id "
               "ORDER BY s.date_seance DESC");

    int row = 0;
    while (query.next()) {
        ui->table_seances->insertRow(row);
        ui->table_seances->setItem(row, 0, new QTableWidgetItem(query.value("id").toString()));
        ui->table_seances->setItem(row, 1, new QTableWidgetItem(query.value("matiere").toString()));
        ui->table_seances->setItem(row, 2, new QTableWidgetItem(query.value("classe").toString()));
        ui->table_seances->setItem(row, 3, new QTableWidgetItem(query.value("date_seance").toString()));
        ui->table_seances->setItem(row, 4, new QTableWidgetItem(query.value("heure_debut").toString()));
        ui->table_seances->setItem(row, 5, new QTableWidgetItem(query.value("heure_fin").toString()));
        ui->table_seances->setItem(row, 6, new QTableWidgetItem(
                                               query.value("validee").toInt() == 1 ? "Oui" : "Non"
                                               ));
        row++;
    }

    ui->table_seances->hideColumn(0);
    ui->table_seances->horizontalHeader()->setStretchLastSection(true);
    ui->table_seances->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_seances->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_seances->setAlternatingRowColors(true);
}

void MainWindow::on_btn_ajouter_seance_clicked()
{
    SeanceDialog dialog(this);
    dialog.setModeAjout();

    if (dialog.exec() == QDialog::Accepted) {
        chargerSeances();
        chargerDashboard();
    }
}

void MainWindow::on_btn_modifier_seance_clicked()
{
    int row = ui->table_seances->currentRow();

    if (row < 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une séance !");
        return;
    }

    int id = ui->table_seances->item(row, 0)->text().toInt();

    // Récupérer les données complètes depuis MySQL
    QSqlQuery query;
    query.prepare("SELECT * FROM seances WHERE id = :id");
    query.bindValue(":id", id);
    query.exec();

    if (query.next()) {
        int idMatiere = query.value("id_matiere").toInt();
        int idClasse = query.value("id_classe").toInt();
        QDate date = QDate::fromString(query.value("date_seance").toString(), "yyyy-MM-dd");
        QTime heureDebut = QTime::fromString(query.value("heure_debut").toString(), "HH:mm:ss");
        QTime heureFin = QTime::fromString(query.value("heure_fin").toString(), "HH:mm:ss");

        SeanceDialog dialog(this);
        dialog.setModeModification(id, idMatiere, idClasse, date, heureDebut, heureFin);

        if (dialog.exec() == QDialog::Accepted) {
            chargerSeances();
            chargerDashboard();
        }
    }
}

void MainWindow::on_btn_supprimer_seance_clicked()
{
    int row = ui->table_seances->currentRow();

    if (row < 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une séance !");
        return;
    }

    QString matiere = ui->table_seances->item(row, 1)->text();
    QString date = ui->table_seances->item(row, 3)->text();

    QMessageBox::StandardButton reponse = QMessageBox::question(
        this, "Confirmation",
        "Voulez-vous supprimer la séance de " + matiere + " du " + date + " ?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reponse == QMessageBox::Yes) {
        QString id = ui->table_seances->item(row, 0)->text();
        QSqlQuery query;
        query.prepare("DELETE FROM seances WHERE id = :id");
        query.bindValue(":id", id);

        if (query.exec()) {
            QMessageBox::information(this, "Succès", "Séance supprimée avec succès !");
            chargerSeances();
            chargerDashboard();
        } else {
            QMessageBox::critical(this, "Erreur", "Erreur : " + query.lastError().text());
        }
    }
}
