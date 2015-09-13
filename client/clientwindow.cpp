#include <QtWidgets>
#include <QtNetwork>
#include "clientwindow.h"

ClientWindow::ClientWindow(QDialog *parent, QString hostname, QTcpSocket *socket) :
    QDialog(parent), hostName(hostname), tcpSocket(socket) {

    loginsFileButton = new QPushButton(tr("logins file"));
    passwordsFileButton = new QPushButton(tr("passwords file"));
    startAttackButton = new QPushButton(tr("START"));
    cancellAttackButton = new QPushButton(tr("CANCEL"));
    loginsLineEdit = new QLineEdit;
    passwordsLineEdit = new QLineEdit;
    hostLineEdit = new QLineEdit;
    hostLabel = new QLabel("Enter host name which should be attack");
    buttonBox = new QDialogButtonBox;

    buttonBox->addButton(startAttackButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(cancellAttackButton, QDialogButtonBox::RejectRole);

    connect(loginsFileButton, SIGNAL(clicked()), this, SLOT(loginsFileDialog()));
    connect(passwordsFileButton, SIGNAL(clicked()), this, SLOT(passwordsFileDialog()));
    connect(startAttackButton, SIGNAL(clicked()), this, SLOT(startAttack()));
    connect(cancellAttackButton, SIGNAL(clicked()), this, SLOT(cancelAttack()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readData()));

    //textEdit = new QTextEdit("Connection is successful \n");
    textEdit = new QTextEdit("");
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(loginsFileButton, 1, 0);
    mainLayout->addWidget(loginsLineEdit, 1, 1);
    mainLayout->addWidget(passwordsFileButton, 2, 0);
    mainLayout->addWidget(passwordsLineEdit, 2, 1);
    mainLayout->addWidget(textEdit, 3, 0, 1, 2);
    mainLayout->addWidget(buttonBox, 4, 0, 1, 2);
    setLayout(mainLayout);

    setWindowTitle("Connected to " + hostName);
}

ClientWindow::ClientWindow() : tcpSocket(NULL) {
    ClientWindow(NULL, "", tcpSocket);
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
    qDebug()<<"loginsFileName"<<loginsFileName<<endl;
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

    tcpSocket->write("<HOST>");
    tcpSocket->write(hostLineEdit->text().toStdString().c_str());
    tcpSocket->write("</HOST>");
    tcpSocket->write("<LOGINS>");
    tcpSocket->write(loginsFile.readAll());
    tcpSocket->waitForBytesWritten();
    tcpSocket->write("</LOGINS>");
    tcpSocket->write("<PASSWORDS>");
    tcpSocket->write(passwordsFile.readAll());
    tcpSocket->waitForBytesWritten();
    tcpSocket->write("</PASSWORDS>");
    tcpSocket->flush();
    loginsFile.close();
    passwordsFile.close();
}

void ClientWindow::cancelAttack(){

}

void ClientWindow::readData(){
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    QString buffer;
    int blockSize = 0;
    in.setVersion(QDataStream::Qt_4_0);

//    if (blockSize == 0) {
//       if (tcpSocket->bytesAvailable() < (int)sizeof(quint16))
//            return;
//        in >> blockSize;
//    }

    //if (tcpSocket->bytesAvailable() < blockSize)
    //    return;

    while (tcpSocket->bytesAvailable()){
	textEdit->moveCursor(QTextCursor::Down);
        buffer.append(tcpSocket->readAll());
	
        textEdit->insertPlainText(buffer);
    }

    qDebug()<<buffer<<endl;

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
    //delete tcpSocket;
    qDebug()<<"ClientWindow destructor \n";
}
