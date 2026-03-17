    #include "loginwindow.h"
    #include "ui_loginwindow.h"
    #include <QDebug>
    #include "mainwindow.h"

    LoginWindow::LoginWindow(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::LoginWindow)
    {
        ui->setupUi(this);
    }

    LoginWindow::~LoginWindow()
    {
        delete ui;
    }

    bool LoginWindow::connecterBaseDeDonnees()
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName("localhost");
        db.setDatabaseName("systeme_presence");
        db.setUserName("mendrika");
        db.setPassword("azerty");

        if(!db.open())
        {
            qDebug() << "Erreur de connexion :" << db.lastError().text();
            ui->label_erreur->setText("Erreur : " + db.lastError().text());
            return false;
        }
        qDebug() << "Connexion réussie !";
        return true;
    }

    void LoginWindow::on_btn_connexion_clicked()
    {
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
            QString role = query.value("role").toString();
            ui->label_erreur->setText("");

            if(role == "admin"){
                MainWindow *mainWindow = new MainWindow();
                mainWindow->show();
                this->close();
            } else if(role == "enseignant"){
                QMessageBox::information(this, "Connexion", "Bienvenue Enseignant !");
            } else if(role == "etudiant"){
                QMessageBox::information(this, "Connexion", "Bienvenue Etudiant !");
            }
        } else{
            ui->label_erreur->setText("Login ou mot de passe incorrect !");
        }
    }
