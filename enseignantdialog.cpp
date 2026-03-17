#include "enseignantdialog.h"
#include "ui_enseignantdialog.h"

EnseignantDialog::EnseignantDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EnseignantDialog)
{
    ui->setupUi(this);
    m_id = -1;
    m_ajout = true;
}

EnseignantDialog::~EnseignantDialog()
{
    delete ui;
}

void EnseignantDialog::setModeAjout()
{
    m_ajout = true;
    ui->label_titre->setText("Ajouter un enseignant");
    ui->lineEdit_nom->clear();
    ui->lineEdit_prenoms->clear();
    ui->lineEdit_email->clear();
    ui->lineEdit_telephone->clear();
}

void EnseignantDialog::setModeModification(int id, QString nom, QString prenoms, QString email, QString telephone)
{
    m_ajout = false;
    m_id = id;
    ui->label_titre->setText("Modifier un enseignant");
    ui->lineEdit_nom->setText(nom);
    ui->lineEdit_prenoms->setText(prenoms);
    ui->lineEdit_email->setText(email);
    ui->lineEdit_telephone->setText(telephone);
}

void EnseignantDialog::on_btn_enregistrer_clicked()
{
    QString nom = ui->lineEdit_nom->text().trimmed().toUpper();
    QString prenoms = ui->lineEdit_prenoms->text().trimmed();
    QString email = ui->lineEdit_email->text().trimmed();
    QString telephone = ui->lineEdit_telephone->text().trimmed();

    if (nom.isEmpty() || prenoms.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Nom et prénoms sont obligatoires !");
        return;
    }

    QSqlQuery query;

    if (m_ajout) {
        query.prepare("INSERT INTO enseignants (nom, prenoms, email, telephone) "
                      "VALUES (:nom, :prenoms, :email, :telephone)");
    } else {
        query.prepare("UPDATE enseignants SET nom = :nom, prenoms = :prenoms, "
                      "email = :email, telephone = :telephone "
                      "WHERE id = :id");
        query.bindValue(":id", m_id);
    }

    query.bindValue(":nom", nom);
    query.bindValue(":prenoms", prenoms);
    query.bindValue(":email", email);
    query.bindValue(":telephone", telephone);

    if (query.exec()) {
        QMessageBox::information(this, "Succès",
                                 m_ajout ? "Enseignant ajouté avec succès !" : "Enseignant modifié avec succès !");
        accept();
    } else {
        QMessageBox::critical(this, "Erreur", "Erreur : " + query.lastError().text());
    }
}

void EnseignantDialog::on_btn_annuler_clicked()
{
    reject();
}
