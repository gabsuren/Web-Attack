#include <QtWidgets>
#include <QtNetwork>
#include "mainwindow.h"
#include "clientwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent)
{
    hostLabel = new QLabel(tr("&Server name:"));
    portLabel = new QLabel(tr("S&erver port:"));

    // find out which IP to connect to
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();

    hostLineEdit = new QLineEdit(ipAddress);
    portLineEdit = new QLineEdit;
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    hostLabel->setBuddy(hostLineEdit);
    portLabel->setBuddy(portLineEdit);

    statusLabel = new QLabel(tr("Connect to the server"));

    statusLabel->setWordWrap(true);
    serverConnectionButton = new QPushButton(tr("Connect"));
    serverConnectionButton->setDefault(true);
    serverConnectionButton->setEnabled(false);

    quitButton = new QPushButton(tr("Quit"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(serverConnectionButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(hostLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));
    connect(portLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(enableGetFortuneButton()));
    connect(serverConnectionButton, SIGNAL(clicked()), this, SLOT(requestServerConnection()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(statusLabel, 2, 0, 1, 2);
    mainLayout->addWidget(buttonBox, 3, 0, 1, 2);
    setLayout(mainLayout);
    setWindowTitle(tr("Connect Server"));
    portLineEdit->setFocus();
}

void MainWindow::enableGetFortuneButton(){
    bool enable(!hostLineEdit->text().isEmpty() && !portLineEdit->text().isEmpty());
    serverConnectionButton->setEnabled(enable);
}

void MainWindow::requestServerConnection(){
    const int Timeout = 5 * 1000;
    serverConnectionButton->setEnabled(false);
    hostName = hostLineEdit->text();
    port = portLineEdit->text().toInt();

    QTcpSocket socket;
    socket.connectToHost(hostName, port);
    if (!socket.waitForConnected(Timeout)) {
        //emit error(socket.error(), socket.errorString());
         qDebug()<<"Error on connection \n";
        return;
    }else{
        qDebug()<<"Connection Successful \n";
        //this->hide();
        this->close();
        ClientWindow clientWindow(NULL, hostName, &socket);
        clientWindow.exec();
    }
}

MainWindow::~MainWindow()
{
    qDebug()<<"MainWindow destructor \n";
}
