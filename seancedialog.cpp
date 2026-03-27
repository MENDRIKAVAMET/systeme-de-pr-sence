#include "seancedialog.h"
#include "ui_seancedialog.h"

//constructeur de SeanceDialog
SeanceDialog::SeanceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SeanceDialog)
{
    ui->setupUi(this);
    //id de la seance à modifier, par défaut, spécifié à -1 parce que le fenêtre s'ouvre par défaut en mode ajout
    m_id = -1;
    //mode ajout par défaut lors de l'ouvrture de la fenêtre
    m_ajout = true;
    chargerMatieres();
    chargerClasses();
    chargerEnseignants(); // chargement des enseignants dans le listWidget
}

//destructeur de SeanceDialog
SeanceDialog::~SeanceDialog()
{
    delete ui;
}

//Chargement des matières dans la base de données pour la combobox matières
void SeanceDialog::chargerMatieres()
{
    //on éface les données dans la combobox
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

//Chargement des classes depuis la base de données à afficher dans le combobox de classe
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

// Chargement des enseignants depuis la base de données dans le listWidget avec cases à cocher
void SeanceDialog::chargerEnseignants()
{
    ui->listWidget_enseignants->clear();
    QSqlQuery query;
    query.exec("SELECT id, nom, prenoms FROM enseignants ORDER BY nom");
    while (query.next()) {
        QString affichage = query.value("nom").toString() + " " + query.value("prenoms").toString();
        QListWidgetItem *item = new QListWidgetItem(affichage);
        // stocker l'id de l'enseignant dans l'item (caché)
        item->setData(Qt::UserRole, query.value("id").toInt());
        // case à cocher non cochée par défaut
        item->setCheckState(Qt::Unchecked);
        ui->listWidget_enseignants->addItem(item);
    }
}

// Sauvegarde des enseignants assignés à la séance dans seance_enseignant
void SeanceDialog::sauvegarderEnseignants(int idSeance)
{
    // Supprimer les anciens enseignants de la séance
    QSqlQuery queryDelete;
    queryDelete.prepare("DELETE FROM seance_enseignant WHERE id_seance = :id");
    queryDelete.bindValue(":id", idSeance);
    queryDelete.exec();

    // Insérer les enseignants cochés
    for (int i = 0; i < ui->listWidget_enseignants->count(); i++) {
        QListWidgetItem *item = ui->listWidget_enseignants->item(i);
        if (item->checkState() == Qt::Checked) {
            int idEnseignant = item->data(Qt::UserRole).toInt();
            QSqlQuery queryInsert;
            queryInsert.prepare("INSERT INTO seance_enseignant (id_seance, id_enseignant) "
                                "VALUES (:idSeance, :idEnseignant)");
            queryInsert.bindValue(":idSeance", idSeance);
            queryInsert.bindValue(":idEnseignant", idEnseignant);
            queryInsert.exec();
        }
    }
}

void SeanceDialog::setModeAjout()
{
    ui->label_titre->setText("Ajouter une séance");
    ui->comboBox_matiere->setCurrentIndex(0);
    ui->comboBox_classe->setCurrentIndex(0);
    ui->dateEdit_date->setDate(QDate::currentDate());
    ui->timeEdit_debut->setTime(QTime(8, 0));
    ui->timeEdit_fin->setTime(QTime(10, 0));

    // Décocher tous les enseignants
    for (int i = 0; i < ui->listWidget_enseignants->count(); i++) {
        ui->listWidget_enseignants->item(i)->setCheckState(Qt::Unchecked);
    }
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

    // Cocher les enseignants déjà assignés à cette séance
    QSqlQuery query;
    query.prepare("SELECT id_enseignant FROM seance_enseignant WHERE id_seance = :id");
    query.bindValue(":id", m_id);
    query.exec();

    QList<int> enseignantsAssignes;
    while (query.next()) {
        enseignantsAssignes.append(query.value("id_enseignant").toInt());
    }

    for (int i = 0; i < ui->listWidget_enseignants->count(); i++) {
        QListWidgetItem *item = ui->listWidget_enseignants->item(i);
        int idEnseignant = item->data(Qt::UserRole).toInt();
        if (enseignantsAssignes.contains(idEnseignant)) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
    }
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

    // Vérifier qu'au moins un enseignant est coché
    bool auMoinsUnEnseignant = false;
    for (int i = 0; i < ui->listWidget_enseignants->count(); i++) {
        if (ui->listWidget_enseignants->item(i)->checkState() == Qt::Checked) {
            auMoinsUnEnseignant = true;
            break;
        }
    }

    if (!auMoinsUnEnseignant) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner au moins un enseignant !");
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
        // Récupérer l'id de la séance insérée ou modifiée
        int idSeance = m_ajout ? query.lastInsertId().toInt() : m_id;

        // Sauvegarder les enseignants assignés
        sauvegarderEnseignants(idSeance);

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
