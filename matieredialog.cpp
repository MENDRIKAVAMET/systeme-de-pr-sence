#include "matieredialog.h"
#include "ui_matieredialog.h"

MatiereDialog::MatiereDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MatiereDialog)
{
    ui->setupUi(this);
    m_id = -1;
    m_ajout = true;
    chargerClasses();
}

MatiereDialog::~MatiereDialog()
{
    delete ui;
}

void MatiereDialog::chargerClasses()
{
    ui->comboBox_classe->clear();
    QSqlQuery query;
    query.exec("SELECT id, nom, niveau FROM classes ORDER BY nom");
    while (query.next()) {
        // Affiche "nom - niveau" mais stocke l'id caché
        QString affichage = query.value("nom").toString() + " - " + query.value("niveau").toString();
        ui->comboBox_classe->addItem(
            affichage,
            query.value("id").toInt()  // id caché dans itemData
            );
    }
}
void MatiereDialog::setModeAjout()
{
    m_ajout = true;
    ui->label_titre->setText("Ajouter une matière");
    ui->lineEdit_code->clear();
    ui->lineEdit_nom->clear();
    ui->lineEdit_volume->clear();
    ui->comboBox_classe->setCurrentIndex(0);

    // Code modifiable en mode ajout
    ui->lineEdit_code->setReadOnly(false);
    ui->lineEdit_code->setStyleSheet("");
}

void MatiereDialog::setModeModification(int id, QString code, QString nom, int volumeHoraire, int idClasse)
{
    m_ajout = false;
    m_id = id;
    ui->label_titre->setText("Modifier une matière");
    ui->lineEdit_code->setText(code);
    ui->lineEdit_nom->setText(nom);
    ui->lineEdit_volume->setText(QString::number(volumeHoraire));

    // Code non modifiable en mode modification
    ui->lineEdit_code->setReadOnly(true);
    ui->lineEdit_code->setStyleSheet("background-color: #E0E0E0; border: 2px solid #AAAAAA;");

    // Sélectionner la bonne classe dans le comboBox
    for (int i = 0; i < ui->comboBox_classe->count(); i++) {
        if (ui->comboBox_classe->itemData(i).toInt() == idClasse) {
            ui->comboBox_classe->setCurrentIndex(i);
            break;
        }
    }
}

void MatiereDialog::on_btn_enregistrer_clicked()
{
    QString code = ui->lineEdit_code->text().trimmed().toUpper();
    QString nom = ui->lineEdit_nom->text().trimmed();
    QString volume = ui->lineEdit_volume->text().trimmed();
    int idClasse = ui->comboBox_classe->currentData().toInt();

    if (code.isEmpty() || nom.isEmpty() || volume.isEmpty()) {
        QMessageBox::warning(this, "Attention", "Veuillez remplir tous les champs !");
        return;
    }

    if (idClasse == 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une classe !");
        return;
    }

    QSqlQuery query;

    if (m_ajout) {
        query.prepare("INSERT INTO matieres (code, nom, volume_horaire, id_classe) "
                      "VALUES (:code, :nom, :volume, :idClasse)");
    } else {
        query.prepare("UPDATE matieres SET nom = :nom, "
                      "volume_horaire = :volume, id_classe = :idClasse "
                      "WHERE id = :id");
        query.bindValue(":id", m_id);
    }

    query.bindValue(":code", code);
    query.bindValue(":nom", nom);
    query.bindValue(":volume", volume.toInt());
    query.bindValue(":idClasse", idClasse);

    if (query.exec()) {
        QMessageBox::information(this, "Succès",
                                 m_ajout ? "Matière ajoutée avec succès !" : "Matière modifiée avec succès !");
        accept();
    } else {
        QMessageBox::critical(this, "Erreur", "Erreur : " + query.lastError().text());
    }
}

void MatiereDialog::on_btn_annuler_clicked()
{
    reject();
}
