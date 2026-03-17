#include "etudiantdialog.h"
#include "ui_etudiantdialog.h"

EtudiantDialog::EtudiantDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EtudiantDialog)
{
    ui->setupUi(this);
    m_id = -1;
    m_ajout = true;
}

EtudiantDialog::~EtudiantDialog()
{
    delete ui;
}

void EtudiantDialog::setModeAjout()
{
    m_ajout = true;
    ui->label_titre->setText("Ajouter un étudiant");
    ui->lineEdit_num_insc->clear();
    ui->lineEdit_nom->clear();
    ui->lineEdit_prenom->clear();

    ui->lineEdit_num_insc->setReadOnly(false);
    ui->lineEdit_num_insc->setStyleSheet("");
}

void EtudiantDialog::setModeModification(QString numInsc, QString nom, QString prenoms)
{
    m_ajout = false;
    ui->label_titre->setText("Modifier un étudiant");
    ui->lineEdit_num_insc->setText(numInsc);
    ui->lineEdit_nom->setText(nom);
    ui->lineEdit_prenom->setText(prenoms);

    ui->lineEdit_num_insc->setReadOnly(true);
    ui->lineEdit_num_insc->setStyleSheet("background-color: #E0E0E0; border: 2px solid #AAAAAA");
}

void EtudiantDialog::on_btn_enregistrer_clicked()
{
    // Récupérer les valeurs
    QString numInsc = ui->lineEdit_num_insc->text().trimmed();
    QString nom = ui->lineEdit_nom->text().trimmed().toUpper();
    QString prenoms = ui->lineEdit_prenom->text().trimmed();

    // Vérifier que les champs ne sont pas vides
    if (numInsc.isEmpty() || nom.isEmpty() || prenoms.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez remplir tous les champs !");
        return;
    }

    QSqlQuery query;

    if (m_ajout) {
        // Ajout
        query.prepare("INSERT INTO etudiants (numero_inscription, nom, prenoms, id_classe) "
                      "VALUES (:num, :nom, :prenoms, 1)");
    } else {
        // Modification
        query.prepare("UPDATE etudiants SET "
                      "nom = :nom, prenoms = :prenoms "
                      "WHERE numero_inscription = :num");
        query.bindValue(":id", m_id);
    }

    query.bindValue(":num", numInsc);
    query.bindValue(":nom", nom);
    query.bindValue(":prenoms", prenoms);

    if (query.exec()) {
        QMessageBox::information(this, "Succès",
                                 m_ajout ? "Étudiant ajouté avec succès !" : "Étudiant modifié avec succès !");
        accept();  // ferme le dialog avec succès
    } else {
        QMessageBox::critical(this, "Erreur", "Erreur : " + query.lastError().text());
    }
}

void EtudiantDialog::on_btn_annuler_clicked()
{
    reject();  // ferme le dialog sans sauvegarder
}
