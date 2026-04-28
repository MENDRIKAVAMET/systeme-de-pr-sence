#include "enseignantwindow.h"
#include "ui_enseignantwindow.h"
#include "loginwindow.h"
#include "enseignantwindow.h"
#include "ui_enseignantwindow.h"
#include "loginwindow.h"
#include <QDate>
#include <QTime>
#include <QProcess>
#include <QTcpSocket>
#include <QNetworkInterface>

EnseignantWindow::EnseignantWindow(int idEnseignant, QString nomEnseignant, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::EnseignantWindow)
    , m_idEnseignant(idEnseignant)
    , m_nomEnseignant(nomEnseignant)
    , m_idSeanceEnCours(-1)
    , m_serveur(nullptr)
{
    ui->setupUi(this);
    ui->label_nom_enseignant->setText("Bonjour, " + m_nomEnseignant + " !");
    ui->stackedWidget->setCurrentWidget(ui->page_seances_jour);
    chargerSeancesJour();

    nodeProcess = new QProcess(this);

    if(isServerRunning()){
        qDebug()<< "Serveur deja lance";
    } else {
        qDebug() << "Demarrage du serveur Node...";

        if(!nodeProcess){
            nodeProcess = new QProcess(this);
        }

        connect(nodeProcess, &QProcess::readyReadStandardError, [this]() {
            qDebug() << "NODE ERROR:" << nodeProcess->readAllStandardError();
        });

        connect(nodeProcess, &QProcess::readyReadStandardOutput, [this]() {
            qDebug() << "NODE OUT:" << nodeProcess->readAllStandardOutput();
        });
        connect(nodeProcess, &QProcess::started, this, [=]() {
            nodeRunning = true;
            qDebug() << "Node STARTED";
        });

        connect(nodeProcess,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this,
                [=]() {
                    nodeRunning = false;
                    qDebug() << "Node STOPPED";
                });
        nodeProcess->start("/home/mendrika/.nvm/versions/node/v20.20.0/bin/node", QStringList() << "server.js");

        if(!nodeProcess->waitForStarted()){
            qDebug() << "ERROR START:" << nodeProcess->errorString();
        }
    }
}

QString EnseignantWindow::getLocalIP()
{
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            address != QHostAddress::LocalHost) {
            return address.toString();
        }
    }
    return "127.0.0.1";
}

bool EnseignantWindow::isServerRunning(){
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 8080);

    return socket.waitForConnected(500);
}

EnseignantWindow::~EnseignantWindow()
{
    arreterServeur();
    if(nodeProcess){
        nodeProcess->kill();
        nodeProcess->waitForFinished();
    }
    delete ui;
}

void EnseignantWindow::chargerSeancesJour()
{
    ui->table_seances_jour->clearContents();
    ui->table_seances_jour->setRowCount(0);

    ui->table_seances_jour->setColumnCount(5);
    ui->table_seances_jour->setHorizontalHeaderLabels(
        QStringList() << "ID" << "Matière" << "Classe" << "Heure début" << "Heure fin"
        );

    QSqlQuery query;
    query.prepare("SELECT s.id, m.nom as matiere, c.nom as classe, "
                  "s.heure_debut, s.heure_fin "
                  "FROM seances s "
                  "JOIN matieres m ON s.id_matiere = m.id "
                  "JOIN classes c ON s.id_classe = c.id "
                  "JOIN seance_enseignant se ON s.id = se.id_seance "
                  "WHERE se.id_enseignant = :idEnseignant "
                  "AND s.date_seance = :date "
                  "ORDER BY s.heure_debut");
    query.bindValue(":idEnseignant", m_idEnseignant);
    query.bindValue(":date", QDate::currentDate().toString("yyyy-MM-dd"));
    query.exec();

    int row = 0;
    while (query.next()) {
        ui->table_seances_jour->insertRow(row);
        ui->table_seances_jour->setItem(row, 0, new QTableWidgetItem(query.value("id").toString()));
        ui->table_seances_jour->setItem(row, 1, new QTableWidgetItem(query.value("matiere").toString()));
        ui->table_seances_jour->setItem(row, 2, new QTableWidgetItem(query.value("classe").toString()));
        ui->table_seances_jour->setItem(row, 3, new QTableWidgetItem(query.value("heure_debut").toString()));
        ui->table_seances_jour->setItem(row, 4, new QTableWidgetItem(query.value("heure_fin").toString()));
        row++;
    }

    ui->table_seances_jour->hideColumn(0);
    ui->table_seances_jour->horizontalHeader()->setStretchLastSection(true);
    ui->table_seances_jour->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_seances_jour->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_seances_jour->setAlternatingRowColors(true);
}

void EnseignantWindow::demarrerServeur()
{
    m_serveur = new QTcpServer(this);
    connect(m_serveur, &QTcpServer::newConnection, this, &EnseignantWindow::nouveauClient);

    if (m_serveur->listen(QHostAddress::Any, 12345)) {
        qDebug() << "Serveur démarré sur le port 12345";
    } else {
        QMessageBox::critical(this, "Erreur", "Impossible de démarrer le serveur !");
    }
}

void EnseignantWindow::arreterServeur()
{
    if (m_serveur) {
        if(m_serveur->isListening()){
            m_serveur->close();
        }
        delete m_serveur;
        m_serveur = nullptr;
    }
}

void EnseignantWindow::nouveauClient()
{
    QTcpSocket *socket = m_serveur->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        QString numeroInscription = QString(socket->readAll()).trimmed();
        qDebug() << "QR Code reçu :" << numeroInscription;

        QString reponse = traiterQRCode(numeroInscription);

        // Renvoyer la réponse à Node.js
        socket->write(reponse.toUtf8());
        socket->flush();
        socket->disconnectFromHost();
    });
}

void EnseignantWindow::nouvelleDonneeRecue()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QString numeroInscription = QString(socket->readAll()).trimmed();
    qDebug() << "QR Code reçu :" << numeroInscription;
    traiterQRCode(numeroInscription);
}

QString EnseignantWindow::traiterQRCode(QString numeroInscription)
{
    QSqlQuery query;
    query.prepare("SELECT e.id, e.nom, e.prenoms FROM etudiants e "
                  "JOIN classes c ON e.id_classe = c.id "
                  "JOIN seances s ON s.id_classe = c.id "
                  "WHERE e.numero_inscription = :num "
                  "AND s.id = :idSeance");
    query.bindValue(":num", numeroInscription);
    query.bindValue(":idSeance", m_idSeanceEnCours);
    query.exec();

    if (query.next()) {
        int idEtudiant = query.value("id").toInt();
        QString nom = query.value("nom").toString();
        QString prenoms = query.value("prenoms").toString();

        // Vérifier si déjà présent
        QSqlQuery queryPresence;
        queryPresence.prepare("SELECT id FROM presences "
                              "WHERE id_seance = :idSeance "
                              "AND id_etudiant = :idEtudiant");
        queryPresence.bindValue(":idSeance", m_idSeanceEnCours);
        queryPresence.bindValue(":idEtudiant", idEtudiant);
        queryPresence.exec();

        if (queryPresence.next()) {
            // Déjà présent
            ui->label_statut->setStyleSheet("color: #F0A500; border: 2px solid #F0A500; border-radius: 5px; padding: 10px;");
            ui->label_statut->setText("⚠ Déjà marqué présent !");
            ui->label_nom_etudiant->setText(nom + " " + prenoms);
            return "DEJA_PRESENT:" + nom + " " + prenoms;
        } else {
            // Marquer présent
            QSqlQuery queryInsert;
            queryInsert.prepare("INSERT INTO presences "
                                "(id_seance, id_etudiant, statut, heure_scan) "
                                "VALUES (:idSeance, :idEtudiant, 'present', :heure)");
            queryInsert.bindValue(":idSeance", m_idSeanceEnCours);
            queryInsert.bindValue(":idEtudiant", idEtudiant);
            queryInsert.bindValue(":heure", QTime::currentTime().toString("HH:mm:ss"));
            queryInsert.exec();

            // Afficher succès
            ui->label_statut->setStyleSheet("color: #28A745; border: 2px solid #28A745; border-radius: 5px; padding: 10px;");
            ui->label_statut->setText("✓ Présent !");
            ui->label_nom_etudiant->setText(nom + " " + prenoms);
            ui->label_nom_etudiant->setStyleSheet("color: #28A745; font-size: 22px; font-weight: bold;");

            // Ajouter dans le tableau
            int row = ui->table_presents->rowCount();
            ui->table_presents->insertRow(row);
            ui->table_presents->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
            ui->table_presents->setItem(row, 1, new QTableWidgetItem(nom));
            ui->table_presents->setItem(row, 2, new QTableWidgetItem(prenoms));
            ui->table_presents->setItem(row, 3, new QTableWidgetItem(
                                                    QTime::currentTime().toString("HH:mm:ss")
                                                    ));

            return "PRESENT:" + nom + " " + prenoms;
        }
    } else {
        // Étudiant non reconnu
        ui->label_statut->setStyleSheet("color: #DC3545; border: 2px solid #DC3545; border-radius: 5px; padding: 10px;");
        ui->label_statut->setText("✗ Étudiant non reconnu !");
        ui->label_nom_etudiant->setText("");
        ui->label_nom_etudiant->setStyleSheet("");
        return "INCONNU";
    }
}

void EnseignantWindow::on_btn_demarrer_presence_clicked()
{
    int row = ui->table_seances_jour->currentRow();

    if (row < 0) {
        QMessageBox::warning(this, "Attention", "Veuillez sélectionner une séance !");
        return;
    }

    m_idSeanceEnCours = ui->table_seances_jour->item(row, 0)->text().toInt();
    QString matiere = ui->table_seances_jour->item(row, 1)->text();
    QString classe = ui->table_seances_jour->item(row, 2)->text();

    // Configurer le tableau des présents
    ui->table_presents->clearContents();
    ui->table_presents->setRowCount(0);
    ui->table_presents->setColumnCount(4);
    ui->table_presents->setHorizontalHeaderLabels(
        QStringList() << "N°" << "Nom" << "Prénoms" << "Heure scan"
        );
    ui->table_presents->horizontalHeader()->setStretchLastSection(true);
    ui->table_presents->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_presents->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_presents->setAlternatingRowColors(true);

    // Afficher les infos de la séance
    ui->label_info_seance->setText(matiere + " - " + classe + " - " +
                                   QDate::currentDate().toString("dd/MM/yyyy"));
    ui->label_statut->setText("En attente de scan...");
    ui->label_statut->setStyleSheet("color: #2D2D2D; border: 2px solid #E0E0E0; border-radius: 5px; padding: 10px;");
    ui->label_nom_etudiant->setText("");

    QString ip = getLocalIP();
    QString url = "https://" + ip + ":8080";
    ui->label_url->setText(url);

    // Démarrer le serveur TCP
    demarrerServeur();

    // Aller sur la page séance en cours
    ui->stackedWidget->setCurrentWidget(ui->page_seance_cours);
}

void EnseignantWindow::on_btn_terminer_clicked()
{
    // Valider la séance
    QSqlQuery query;
    query.prepare("UPDATE seances SET validee = 1 WHERE id = :id");
    query.bindValue(":id", m_idSeanceEnCours);
    query.exec();

    // Arrêter le serveur
    arreterServeur();
    m_idSeanceEnCours = -1;

    // Retourner à la liste des séances
    ui->stackedWidget->setCurrentWidget(ui->page_seances_jour);
    chargerSeancesJour();

    QMessageBox::information(this, "Succès", "Séance terminée et validée !");
}

void EnseignantWindow::on_btn_deconnexion_clicked()
{
    arreterServeur();
    LoginWindow *login = new LoginWindow();
    login->show();
    this->close();
}

void EnseignantWindow::on_btn_historique_clicked()
{
    chargerSeancesHistorique();
    ui->stackedWidget->setCurrentWidget(ui->page_historique);
}

void EnseignantWindow::on_btn_retour_historique_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_seances_jour);
}

void EnseignantWindow::chargerSeancesHistorique()
{
    ui->comboBox_seances_hist->clear();

    QSqlQuery query;
    query.prepare("SELECT s.id, m.nom as matiere, c.nom as classe, "
                  "s.date_seance, s.heure_debut, s.heure_fin "
                  "FROM seances s "
                  "JOIN matieres m ON s.id_matiere = m.id "
                  "JOIN classes c ON s.id_classe = c.id "
                  "JOIN seance_enseignant se ON s.id = se.id_seance "
                  "WHERE se.id_enseignant = :idEnseignant "
                  "ORDER BY s.date_seance DESC, s.heure_debut DESC");
    query.bindValue(":idEnseignant", m_idEnseignant);
    query.exec();

    while (query.next()) {
        QString affichage = query.value("date_seance").toString() + " - " +
                            query.value("matiere").toString() + " - " +
                            query.value("classe").toString() + " (" +
                            query.value("heure_debut").toString() + ")";
        ui->comboBox_seances_hist->addItem(
            affichage,
            query.value("id").toInt()
            );
    }

    // Charger automatiquement la première séance
    if (ui->comboBox_seances_hist->count() > 0) {
        chargerHistorique();
    }
}

void EnseignantWindow::chargerHistorique()
{
    int idSeance = ui->comboBox_seances_hist->currentData().toInt();
    if (idSeance == 0) return;

    // Afficher les infos de la séance
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
        ui->label_info_hist->setText(
            queryInfo.value("matiere").toString() + " | " +
            queryInfo.value("classe").toString() + " | " +
            queryInfo.value("date_seance").toString() + " | " +
            queryInfo.value("heure_debut").toString() + " - " +
            queryInfo.value("heure_fin").toString()
            );
    }

    // Charger les présences
    ui->table_historique->clearContents();
    ui->table_historique->setRowCount(0);
    ui->table_historique->setColumnCount(4);
    ui->table_historique->setHorizontalHeaderLabels(
        QStringList() << "N°" << "Nom" << "Prénoms" << "Heure scan"
        );

    QSqlQuery query;
    query.prepare("SELECT e.nom, e.prenoms, p.heure_scan "
                  "FROM presences p "
                  "JOIN etudiants e ON p.id_etudiant = e.id "
                  "WHERE p.id_seance = :idSeance "
                  "AND p.statut = 'present' "
                  "ORDER BY p.heure_scan");
    query.bindValue(":idSeance", idSeance);
    query.exec();

    int row = 0;
    while (query.next()) {
        ui->table_historique->insertRow(row);
        ui->table_historique->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        ui->table_historique->setItem(row, 1, new QTableWidgetItem(query.value("nom").toString()));
        ui->table_historique->setItem(row, 2, new QTableWidgetItem(query.value("prenoms").toString()));
        ui->table_historique->setItem(row, 3, new QTableWidgetItem(query.value("heure_scan").toString()));
        row++;
    }

    ui->table_historique->horizontalHeader()->setStretchLastSection(true);
    ui->table_historique->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_historique->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_historique->setAlternatingRowColors(true);

    // Compter total étudiants dans la classe
    QSqlQuery queryTotal;
    queryTotal.prepare("SELECT COUNT(*) FROM etudiants e "
                       "JOIN classes c ON e.id_classe = c.id "
                       "JOIN seances s ON s.id_classe = c.id "
                       "WHERE s.id = :idSeance");
    queryTotal.bindValue(":idSeance", idSeance);
    queryTotal.exec();
    int total = 0;
    if (queryTotal.next()) total = queryTotal.value(0).toInt();

    ui->label_stats_hist->setText(
        QString("✅ %1 présents / %2 étudiants | ❌ %3 absents")
            .arg(row)
            .arg(total)
            .arg(total - row)
        );
}

void EnseignantWindow::on_comboBox_seances_hist_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    chargerHistorique();
}
