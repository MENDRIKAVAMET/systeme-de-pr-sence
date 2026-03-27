#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loginwindow.h"
#include <QHeaderView>
#include <QMessageBox>
#include "etudiantdialog.h"
#include "enseignantdialog.h"
#include "matieredialog.h"
#include "seancedialog.h"
#include <QBrush>
#include <QColor>

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

    ui->table_enseignants->setColumnCount(6);
    ui->table_enseignants->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Nom" << "Prénoms" << "Email" << "Téléphone" << "Matières"
        );

    QSqlQuery query;
    query.exec("SELECT e.id, e.nom, e.prenoms, e.email, e.telephone, "
               "GROUP_CONCAT(m.nom SEPARATOR ', ') as matieres "
               "FROM enseignants e "
               "LEFT JOIN matiere_enseignant me ON e.id = me.id_enseignant "
               "LEFT JOIN matieres m ON me.id_matiere = m.id "
               "GROUP BY e.id "
               "ORDER BY e.nom");

    int row = 0;
    while (query.next()) {
        ui->table_enseignants->insertRow(row);
        ui->table_enseignants->setItem(row, 0, new QTableWidgetItem(query.value("id").toString()));
        ui->table_enseignants->setItem(row, 1, new QTableWidgetItem(query.value("nom").toString()));
        ui->table_enseignants->setItem(row, 2, new QTableWidgetItem(query.value("prenoms").toString()));
        ui->table_enseignants->setItem(row, 3, new QTableWidgetItem(query.value("email").toString()));
        ui->table_enseignants->setItem(row, 4, new QTableWidgetItem(query.value("telephone").toString()));
        ui->table_enseignants->setItem(row, 5, new QTableWidgetItem(query.value("matieres").toString()));
        row++;
    }

    ui->table_enseignants->hideColumn(0);
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

    ui->table_matieres->setColumnCount(6);
    ui->table_matieres->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Code" << "Nom" << "Volume horaire" << "Classe" << "Enseignants"
        );

    QSqlQuery query;
    query.exec("SELECT m.id, m.code, m.nom, m.volume_horaire, c.nom as classe, "
               "GROUP_CONCAT(e.nom, ' ', e.prenoms SEPARATOR ', ') as enseignants "
               "FROM matieres m "
               "JOIN classes c ON m.id_classe = c.id "
               "LEFT JOIN matiere_enseignant me ON m.id = me.id_matiere "
               "LEFT JOIN enseignants e ON me.id_enseignant = e.id "
               "GROUP BY m.id "
               "ORDER BY m.nom");

    int row = 0;
    while (query.next()) {
        ui->table_matieres->insertRow(row);
        ui->table_matieres->setItem(row, 0, new QTableWidgetItem(query.value("id").toString()));
        ui->table_matieres->setItem(row, 1, new QTableWidgetItem(query.value("code").toString()));
        ui->table_matieres->setItem(row, 2, new QTableWidgetItem(query.value("nom").toString()));
        ui->table_matieres->setItem(row, 3, new QTableWidgetItem(query.value("volume_horaire").toString() + " h"));
        ui->table_matieres->setItem(row, 4, new QTableWidgetItem(query.value("classe").toString()));
        ui->table_matieres->setItem(row, 5, new QTableWidgetItem(query.value("enseignants").toString()));
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

    ui->table_seances->setColumnCount(8);
    ui->table_seances->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Matière" << "Classe" << "Date"
                      << "Heure début" << "Heure fin" << "Validée" << "Enseignants"
        );

    QSqlQuery query;
    query.exec("SELECT s.id, m.nom as matiere, c.nom as classe, "
               "s.date_seance, s.heure_debut, s.heure_fin, s.validee, "
               "GROUP_CONCAT(e.nom, ' ', e.prenoms SEPARATOR ', ') as enseignants "
               "FROM seances s "
               "JOIN matieres m ON s.id_matiere = m.id "
               "JOIN classes c ON s.id_classe = c.id "
               "LEFT JOIN seance_enseignant se ON s.id = se.id_seance "
               "LEFT JOIN enseignants e ON se.id_enseignant = e.id "
               "GROUP BY s.id "
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
                                               query.value("validee").toInt() == 1 ? "✓ Oui" : "✗ Non"
                                               ));
        ui->table_seances->setItem(row, 7, new QTableWidgetItem(query.value("enseignants").toString()));
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

void MainWindow::on_btn_rapports_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_rapports);
    chargerRapports();
}

void MainWindow::chargerRapports()
{
    // Charger les combobox
    // ComboBox séances
    ui->comboBox_seances->clear();
    QSqlQuery querySeances;
    querySeances.exec("SELECT s.id, m.nom as matiere, c.nom as classe, "
                      "s.date_seance, s.heure_debut "
                      "FROM seances s "
                      "JOIN matieres m ON s.id_matiere = m.id "
                      "JOIN classes c ON s.id_classe = c.id "
                      "ORDER BY s.date_seance DESC");
    while (querySeances.next()) {
        QString affichage = querySeances.value("date_seance").toString() + " - " +
                            querySeances.value("matiere").toString() + " - " +
                            querySeances.value("classe").toString();
        ui->comboBox_seances->addItem(affichage, querySeances.value("id").toInt());
    }

    // ComboBox étudiants
    ui->comboBox_etudiants->clear();
    QSqlQuery queryEtudiants;
    queryEtudiants.exec("SELECT id, nom, prenoms, numero_inscription "
                        "FROM etudiants ORDER BY nom");
    while (queryEtudiants.next()) {
        QString affichage = queryEtudiants.value("nom").toString() + " " +
                            queryEtudiants.value("prenoms").toString() +
                            " (" + queryEtudiants.value("numero_inscription").toString() + ")";
        ui->comboBox_etudiants->addItem(affichage, queryEtudiants.value("id").toInt());
    }

    // ComboBox matières
    ui->comboBox_matieres->clear();
    QSqlQuery queryMatieres;
    queryMatieres.exec("SELECT id, nom, code FROM matieres ORDER BY nom");
    while (queryMatieres.next()) {
        QString affichage = queryMatieres.value("code").toString() + " - " +
                            queryMatieres.value("nom").toString();
        ui->comboBox_matieres->addItem(affichage, queryMatieres.value("id").toInt());
    }

    // Charger les rapports
    chargerRapportParSeance();
    chargerRapportParEtudiant();
    chargerRapportParMatiere();
}

void MainWindow::chargerRapportParSeance()
{
    int idSeance = ui->comboBox_seances->currentData().toInt();
    if (idSeance == 0) return;

    // Infos de la séance
    QSqlQuery queryInfo;
    queryInfo.prepare("SELECT m.nom as matiere, c.nom as classe, "
                      "s.date_seance, s.heure_debut, s.heure_fin "
                      "FROM seances s "
                      "JOIN matieres m ON s.id_matiere = m.id "
                      "JOIN classes c ON s.id_classe = c.id "
                      "WHERE s.id = :id");
    queryInfo.bindValue(":id", idSeance);
    queryInfo.exec();

    if (queryInfo.next()) {
        ui->label_info_seance->setText(
            queryInfo.value("matiere").toString() + " | " +
            queryInfo.value("classe").toString() + " | " +
            queryInfo.value("date_seance").toString() + " | " +
            queryInfo.value("heure_debut").toString() + " - " +
            queryInfo.value("heure_fin").toString()
            );
    }

    // Tableau présences
    ui->table_rapport_seance->clearContents();
    ui->table_rapport_seance->setRowCount(0);
    ui->table_rapport_seance->setColumnCount(4);
    ui->table_rapport_seance->setHorizontalHeaderLabels(
        QStringList() << "N°" << "Nom" << "Prénoms" << "Statut"
        );

    // Tous les étudiants de la classe
    QSqlQuery query;
    query.prepare("SELECT e.id, e.nom, e.prenoms FROM etudiants e "
                  "JOIN classes c ON e.id_classe = c.id "
                  "JOIN seances s ON s.id_classe = c.id "
                  "WHERE s.id = :idSeance "
                  "ORDER BY e.nom");
    query.bindValue(":idSeance", idSeance);
    query.exec();

    int row = 0;
    int nbPresents = 0;
    int nbAbsents = 0;

    while (query.next()) {
        int idEtudiant = query.value("id").toInt();

        // Vérifier si présent
        QSqlQuery queryPresence;
        queryPresence.prepare("SELECT statut FROM presences "
                              "WHERE id_seance = :idSeance "
                              "AND id_etudiant = :idEtudiant");
        queryPresence.bindValue(":idSeance", idSeance);
        queryPresence.bindValue(":idEtudiant", idEtudiant);
        queryPresence.exec();

        QString statut = "Absent";
        if (queryPresence.next()) {
            statut = "Présent";
            nbPresents++;
        } else {
            nbAbsents++;
        }

        ui->table_rapport_seance->insertRow(row);
        ui->table_rapport_seance->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        ui->table_rapport_seance->setItem(row, 1, new QTableWidgetItem(query.value("nom").toString()));
        ui->table_rapport_seance->setItem(row, 2, new QTableWidgetItem(query.value("prenoms").toString()));

        QTableWidgetItem *itemStatut = new QTableWidgetItem(statut);
        if (statut == "Présent") {
            itemStatut->setForeground(QBrush(QColor("#28A745")));
        } else {
            itemStatut->setForeground(QBrush(QColor("#DC3545")));
        }
        ui->table_rapport_seance->setItem(row, 3, itemStatut);
        row++;
    }

    ui->table_rapport_seance->horizontalHeader()->setStretchLastSection(true);
    ui->table_rapport_seance->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_rapport_seance->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_rapport_seance->setAlternatingRowColors(true);

    int total = nbPresents + nbAbsents;
    int taux = total > 0 ? (nbPresents * 100 / total) : 0;
    ui->label_stats_seance->setText(
        QString("✅ %1 présents | ❌ %2 absents | 📊 Taux : %3%")
            .arg(nbPresents).arg(nbAbsents).arg(taux)
        );
}

void MainWindow::chargerRapportParEtudiant()
{
    int idEtudiant = ui->comboBox_etudiants->currentData().toInt();
    if (idEtudiant == 0) return;

    // Infos étudiant
    QSqlQuery queryInfo;
    queryInfo.prepare("SELECT nom, prenoms, numero_inscription FROM etudiants WHERE id = :id");
    queryInfo.bindValue(":id", idEtudiant);
    queryInfo.exec();

    if (queryInfo.next()) {
        ui->label_info_etudiant->setText(
            queryInfo.value("nom").toString() + " " +
            queryInfo.value("prenoms").toString() +
            " | " + queryInfo.value("numero_inscription").toString()
            );
    }

    // Tableau par matière
    ui->table_rapport_etudiant->clearContents();
    ui->table_rapport_etudiant->setRowCount(0);
    ui->table_rapport_etudiant->setColumnCount(4);
    ui->table_rapport_etudiant->setHorizontalHeaderLabels(
        QStringList() << "Matière" << "Séances" << "Présences" << "Taux"
        );

    QSqlQuery query;
    query.prepare("SELECT m.nom as matiere, "
                  "COUNT(DISTINCT s.id) as total_seances, "
                  "COUNT(DISTINCT p.id_seance) as total_presents "
                  "FROM matieres m "
                  "JOIN seances s ON s.id_matiere = m.id "
                  "JOIN classes c ON s.id_classe = c.id "
                  "JOIN etudiants e ON e.id_classe = c.id "
                  "LEFT JOIN presences p ON p.id_seance = s.id "
                  "AND p.id_etudiant = e.id "
                  "WHERE e.id = :idEtudiant "
                  "GROUP BY m.id "
                  "ORDER BY m.nom");
    query.bindValue(":idEtudiant", idEtudiant);
    query.exec();

    int row = 0;
    int totalSeances = 0;
    int totalPresents = 0;

    while (query.next()) {
        int seances = query.value("total_seances").toInt();
        int presents = query.value("total_presents").toInt();
        int taux = seances > 0 ? (presents * 100 / seances) : 0;

        totalSeances += seances;
        totalPresents += presents;

        ui->table_rapport_etudiant->insertRow(row);
        ui->table_rapport_etudiant->setItem(row, 0, new QTableWidgetItem(query.value("matiere").toString()));
        ui->table_rapport_etudiant->setItem(row, 1, new QTableWidgetItem(QString::number(seances)));
        ui->table_rapport_etudiant->setItem(row, 2, new QTableWidgetItem(QString::number(presents)));

        QTableWidgetItem *itemTaux = new QTableWidgetItem(QString::number(taux) + "%");
        if (taux >= 75) {
            itemTaux->setForeground(QBrush(QColor("#28A745")));
        } else if (taux >= 50) {
            itemTaux->setForeground(QBrush(QColor("#F0A500")));
        } else {
            itemTaux->setForeground(QBrush(QColor("#DC3545")));
        }
        ui->table_rapport_etudiant->setItem(row, 3, itemTaux);
        row++;
    }

    ui->table_rapport_etudiant->horizontalHeader()->setStretchLastSection(true);
    ui->table_rapport_etudiant->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_rapport_etudiant->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_rapport_etudiant->setAlternatingRowColors(true);

    int tauxGlobal = totalSeances > 0 ? (totalPresents * 100 / totalSeances) : 0;
    ui->label_status_etudiant->setText(
        QString("1 séances total | présences | Taux global : %3%")
            .arg(totalSeances).arg(totalPresents).arg(tauxGlobal)
        );
}

void MainWindow::chargerRapportParMatiere()
{
    int idMatiere = ui->comboBox_matieres->currentData().toInt();
    if (idMatiere == 0) return;

    // Infos matière
    QSqlQuery queryInfo;
    queryInfo.prepare("SELECT nom, code FROM matieres WHERE id = :id");
    queryInfo.bindValue(":id", idMatiere);
    queryInfo.exec();

    if (queryInfo.next()) {
        ui->label_info_matiere->setText(
            queryInfo.value("code").toString() + " | " +
            queryInfo.value("nom").toString()
            );
    }

    // Tableau par séance
    ui->table_rapport_matiere->clearContents();
    ui->table_rapport_matiere->setRowCount(0);
    ui->table_rapport_matiere->setColumnCount(4);
    ui->table_rapport_matiere->setHorizontalHeaderLabels(
        QStringList() << "Date" << "Total étudiants" << "Présents" << "Taux"
        );

    QSqlQuery query;
    query.prepare("SELECT s.date_seance, s.heure_debut, "
                  "COUNT(DISTINCT e.id) as total_etudiants, "
                  "COUNT(DISTINCT p.id_etudiant) as total_presents "
                  "FROM seances s "
                  "JOIN classes c ON s.id_classe = c.id "
                  "JOIN etudiants e ON e.id_classe = c.id "
                  "LEFT JOIN presences p ON p.id_seance = s.id "
                  "WHERE s.id_matiere = :idMatiere "
                  "GROUP BY s.id "
                  "ORDER BY s.date_seance DESC");
    query.bindValue(":idMatiere", idMatiere);
    query.exec();

    int row = 0;
    int totalTaux = 0;

    while (query.next()) {
        int total = query.value("total_etudiants").toInt();
        int presents = query.value("total_presents").toInt();
        int taux = total > 0 ? (presents * 100 / total) : 0;
        totalTaux += taux;

        QString dateAffichage = query.value("date_seance").toString() +
                                " (" + query.value("heure_debut").toString() + ")";

        ui->table_rapport_matiere->insertRow(row);
        ui->table_rapport_matiere->setItem(row, 0, new QTableWidgetItem(dateAffichage));
        ui->table_rapport_matiere->setItem(row, 1, new QTableWidgetItem(QString::number(total)));
        ui->table_rapport_matiere->setItem(row, 2, new QTableWidgetItem(QString::number(presents)));

        QTableWidgetItem *itemTaux = new QTableWidgetItem(QString::number(taux) + "%");
        if (taux >= 75) {
            itemTaux->setForeground(QBrush(QColor("#28A745")));
        } else if (taux >= 50) {
            itemTaux->setForeground(QBrush(QColor("#F0A500")));
        } else {
            itemTaux->setForeground(QBrush(QColor("#DC3545")));
        }
        ui->table_rapport_matiere->setItem(row, 3, itemTaux);
        row++;
    }

    ui->table_rapport_matiere->horizontalHeader()->setStretchLastSection(true);
    ui->table_rapport_matiere->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_rapport_matiere->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_rapport_matiere->setAlternatingRowColors(true);

    int moyenneTaux = row > 0 ? (totalTaux / row) : 0;
    ui->label_stats_matiere->setText(
        QString("%1 séances | Taux moyen de présence : %2%")
            .arg(row).arg(moyenneTaux)
        );
}

void MainWindow::on_comboBox_seances_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    chargerRapportParSeance();
}

void MainWindow::on_comboBox_etudiants_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    chargerRapportParEtudiant();
}

void MainWindow::on_comboBox_matieres_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    chargerRapportParMatiere();
}
