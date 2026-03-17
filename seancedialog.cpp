#include "seancedialog.h"
#include "ui_seancedialog.h"

SeanceDialog::SeanceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SeanceDialog)
{
    ui->setupUi(this);
    m_id = -1;
    m_ajout = true;
    chargerMatieres();
    chargerClasses();
}

SeanceDialog::~SeanceDialog()
{
    delete ui;
}

void SeanceDialog::chargerMatieres()
{
    ui->comboBox_matiere->clear();
    QSqlQuery query;
    query.exec("SELECT id, nom, code FROM matieres ORDER BY nom");
    while (query.next()) {
        QString affichage = query.value("code").toString() + " - " + query.value("nom").toString();
        ui->comboBox_matiere->addItem(
            affichage,
            query.value("id").toInt()
            );
    }
}

void SeanceDialog::chargerClasses()
{
    ui->comboBox_classe->clear();
    QSqlQuery query;
    query.exec("SELECT id, nom, niveau FROM classes ORDER BY nom");
    while (query.next()) {
        QString affichage = query.value("nom").toString() + " - " + query.value("niveau").toString();
        ui->comboBox_classe->addItem(
            affichage,
            query.value("id").toInt()
            );
    }
}

void SeanceDialog::setModeAjout()
{
    m_ajout = true;
    ui->label_titre->setText("Ajouter une séance");
    ui->comboBox_matiere->setCurrentIndex(0);
    ui->comboBox_classe->setCurrentIndex(0);
    ui->dateEdit_date->setDate(QDate::currentDate());
    ui->timeEdit_debut->setTime(QTime(8, 0));
    ui->timeEdit_fin->setTime(QTime(10, 0));
}

void SeanceDialog::setModeModification(int id, int idMatiere, int idClasse,
                                       QDate date, QTime heureDebut, QTime heureFin)
{
    m_ajout = false;
    m_id = id;
    ui->label_titre->setText("Modifier une séance");

    // Sélectionner la bonne matière
    for (int i = 0; i < ui->comboBox_matiere->count(); i++) {
        if (ui->comboBox_matiere->itemData(i).toInt() == idMatiere) {
            ui->comboBox_matiere->setCurrentIndex(i);
            break;
        }
    }

    // Sélectionner la bonne classe
    for (int i = 0; i < ui->comboBox_classe->count(); i++) {
        if (ui->comboBox_classe->itemData(i).toInt() == idClasse) {
            ui->comboBox_classe->setCurrentIndex(i);
            break;
        }
    }

    ui->dateEdit_date->setDate(date);
    ui->timeEdit_debut->setTime(heureDebut);
    ui->timeEdit_fin->setTime(heureFin);
}

void SeanceDialog::on_btn_enregistrer_clicked()
{
    int idMatiere = ui->comboBox_matiere->currentData().toInt();
    int idClasse = ui->comboBox_classe->currentData().toInt();
    QDate date = ui->dateEdit_date->date();
    QTime heureDebut = ui->timeEdit_debut->time();
    QTime heureFin = ui->timeEdit_fin->time();

    if (idMatiere == 0 || idClasse == 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une matière et une classe !");
        return;
    }

    if (heureDebut >= heureFin) {
        QMessageBox::warning(this, "Attention", "L'heure de fin doit être après l'heure de début !");
        return;
    }

    QSqlQuery query;

    if (m_ajout) {
        query.prepare("INSERT INTO seances (id_matiere, id_classe, date_seance, heure_debut, heure_fin) "
                      "VALUES (:idMatiere, :idClasse, :date, :debut, :fin)");
    } else {
        query.prepare("UPDATE seances SET id_matiere = :idMatiere, id_classe = :idClasse, "
                      "date_seance = :date, heure_debut = :debut, heure_fin = :fin "
                      "WHERE id = :id");
        query.bindValue(":id", m_id);
    }

    query.bindValue(":idMatiere", idMatiere);
    query.bindValue(":idClasse", idClasse);
    query.bindValue(":date", date.toString("yyyy-MM-dd"));
    query.bindValue(":debut", heureDebut.toString("HH:mm:ss"));
    query.bindValue(":fin", heureFin.toString("HH:mm:ss"));

    if (query.exec()) {
        QMessageBox::information(this, "Succès",
                                 m_ajout ? "Séance ajoutée avec succès !" : "Séance modifiée avec succès !");
        accept();
    } else {
        QMessageBox::critical(this, "Erreur", "Erreur : " + query.lastError().text());
    }
}

void SeanceDialog::on_btn_annuler_clicked()
{
    reject();
}
