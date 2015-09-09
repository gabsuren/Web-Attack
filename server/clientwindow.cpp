#include <QtWidgets>
#include <QtNetwork>
#include "clientwindow.h"

ClientWindow::ClientWindow(QDialog *parent, QString hostname, QTcpSocket *socket) :
    QDialog(parent), hostName(hostname), connectionSocket(socket) {

    loginsFileButton = new QPushButton(tr("logins file"));
    passwordsFileButton = new QPushButton(tr("passwords file"));
    startAttackButton = new QPushButton(tr("START"));
    cancellAttackButton = new QPushButton(tr("CANCEL"));
    loginsLineEdit = new QLineEdit;
    passwordsLineEdit = new QLineEdit;
    hostLineEdit = new QLineEdit;
    hostLabel = new QLabel("Enter host name which should be attacked");
    buttonBox = new QDialogButtonBox;

    buttonBox->addButton(startAttackButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(cancellAttackButton, QDialogButtonBox::RejectRole);

    connect(loginsFileButton, SIGNAL(clicked()), this, SLOT(loginsFileDialog()));
    connect(passwordsFileButton, SIGNAL(clicked()), this, SLOT(passwordsFileDialog()));
    connect(startAttackButton, SIGNAL(clicked()), this, SLOT(startAttack()));
    connect(startAttackButton, SIGNAL(clicked()), this, SLOT(cancelAttack()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(loginsFileButton, 1, 0);
    mainLayout->addWidget(loginsLineEdit, 1, 1);
    mainLayout->addWidget(passwordsFileButton, 2, 0);
    mainLayout->addWidget(passwordsLineEdit, 2, 1);
    mainLayout->addWidget(buttonBox, 3, 0, 1, 2);
    setLayout(mainLayout);

    setWindowTitle("Connected to " + hostName);
}

ClientWindow::ClientWindow() : connectionSocket(NULL) {
    ClientWindow(NULL, "", connectionSocket);
}

void ClientWindow::loginsFileDialog(){
    QStringList fileList = QFileDialog::getOpenFileNames(this, tr("Open File"), QDir::currentPath());
    if(!fileList.empty()){
        qDebug()<<fileList.at(0);
        loginsFileName = fileList.at(0);
        loginsLineEdit->setText(loginsFileName);
    }
}

void ClientWindow::passwordsFileDialog(){
    QStringList fileList = QFileDialog::getOpenFileNames(this, tr("Open File"), QDir::currentPath());
    if(!fileList.empty()){
        qDebug()<<fileList.at(0);
        passwordsFileName = fileList.at(0);
        passwordsLineEdit->setText(passwordsFileName);
    }
}

void ClientWindow::startAttack(){

    if(hostLineEdit->text().isEmpty()){
        qWarning()<<"Error: Please Enter host name";
        return;
    }

    if(loginsFileName.isEmpty()){
        qWarning()<<"Please Enter logins file";
        return;
    }

    if(passwordsFileName.isEmpty()){
        qWarning()<<"Please Enter passwords file";
        return;
    }

    QFile loginsFile(loginsFileName);
    if(!loginsFile.open(QIODevice::ReadOnly)){
        qWarning() << "Error logins file can't be opened ! !";
        return;
    }

    QFile passwordsFile(passwordsFileName);
    if(!passwordsFile.open(QIODevice::ReadOnly)){
        qWarning() << "Error passwords file can't be opened ! !";
        return;
    }

    connectionSocket->write("<HOST>");
    connectionSocket->write(hostLineEdit->text().toStdString().c_str());
    connectionSocket->write("</HOST>");
    connectionSocket->write("<LOGINS>");
    connectionSocket->write(loginsFile.readAll());
    connectionSocket->write("</LOGINS>");
    connectionSocket->write("<PASSWORDS>");
    connectionSocket->write(passwordsFile.readAll());
    connectionSocket->write("</PASSWORDS>");

    connectionSocket->flush();
    loginsFile.close();
    passwordsFile.close();
}

void ClientWindow::cancelAttack(){

}

ClientWindow::~ClientWindow(){
    delete loginsFileButton;
    delete passwordsFileButton;
    delete startAttackButton;
    delete cancellAttackButton;
    delete buttonBox;
    delete hostLabel;
    delete hostLineEdit;
    delete loginsLineEdit;
    delete passwordsLineEdit;
    delete connectionSocket;

    qDebug()<<"ClientWindow destructor \n";
}
