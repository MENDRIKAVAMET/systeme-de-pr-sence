    #include "loginwindow.h"
    #include "ui_loginwindow.h"
    #include <QDebug>
    #include "mainwindow.h"
    #include "enseignantwindow.h"

    //constructeur de loginWindow
    LoginWindow::LoginWindow(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::LoginWindow)
    {
        ui->setupUi(this);
    }

    //destructeur de loginWindow
    LoginWindow::~LoginWindow()
    {
        delete ui;
    }

    //connexion à la base de données
    bool LoginWindow::connecterBaseDeDonnees()
    {
        //spécification du base de données utilisé ici Mysql et la base de données à utiliser
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName("localhost");
        db.setDatabaseName("systeme_presence");
        db.setUserName("mendrika");
        db.setPassword("azerty");

        if(!db.open())
        {
            //debug pour les erreurs de connexions de la base de données
            qDebug() << "Erreur de connexion :" << db.lastError().text();
            ui->label_erreur->setText("Erreur : " + db.lastError().text());
            return false;
        }
        return true;
    }

    //lorsque le bouton connexion est cliqué
    void LoginWindow::on_btn_connexion_clicked()
    {
        //on enlève les éspaces du login et du password
        QString login = ui->lineEdit_login->text().trimmed();
        QString mdp = ui->lineEdit_password->text().trimmed();
        if(login.isEmpty() || mdp.isEmpty()){
            ui->label_erreur->setText("Veuillez remplir tous les champs !");
            return;
        }
        if(!connecterBaseDeDonnees()){
            ui->label_erreur->setText("Erreur de connexion à la base de données !");
            return;
        }
        QSqlQuery query;
        query.prepare("SELECT id, role FROM utilisateurs WHERE login = :login AND mot_de_passe = :mdp AND actif=1");
        query.bindValue(":login", login);
        query.bindValue(":mdp", mdp);

        if(query.exec() && query.next()){
            //on récupère le rôle de l'utilisateur connecté dans la base de données
            QString role = query.value("role").toString();
            ui->label_erreur->setText("");

            //Si le rôle de l'utilisateur est de type admin
            if(role == "admin"){
                //on ouvre le fenêtre de l'admin
                MainWindow *mainWindow = new MainWindow();
                mainWindow->show();
                this->close();

            //Si le rôle de l'utilisateur est de type enseignant
            } else if(role == "enseignant"){
                int idEnseignant = query.value("id").toInt();

                // Récupérer le nom de l'enseignant
                QSqlQuery queryEns;
                queryEns.prepare("SELECT nom, prenoms FROM enseignants "
                                 "WHERE id_utilisateur = :id");
                queryEns.bindValue(":id", idEnseignant);
                queryEns.exec();

                QString nomEnseignant = "";
                if (queryEns.next()) {
                    nomEnseignant = queryEns.value("nom").toString() + " " +
                                    queryEns.value("prenoms").toString();
                }

                EnseignantWindow *ensegnantWindow = new EnseignantWindow(idEnseignant, nomEnseignant);
                ensegnantWindow->show();
                this->close();

            //Si le rôle de l'utilisateur est de type étudiant
            } else if(role == "etudiant"){
                QMessageBox::information(this, "Connexion", "Bienvenue Etudiant !");
            }
        } else{
            ui->label_erreur->setText("Login ou mot de passe incorrect !");
        }
    }
