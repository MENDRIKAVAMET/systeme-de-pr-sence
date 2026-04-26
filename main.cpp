#include "loginwindow.h"
#include <QProcess>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Démarrage automatique du serveur node.js
    QProcess::startDetached("node", QStringList() << "server.js");

    LoginWindow w;
    w.show();
    return a.exec();
}
