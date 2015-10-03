#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QDialog>
#include <QDebug>

class QLineEdit;
class QString;
class QLabel;
class QDialogButtonBox;
class QTcpSocket;
class QTextEdit;

class ClientWindow : public QDialog
{
    Q_OBJECT

private slots:
    void loginsFileDialog();
    void passwordsFileDialog();
    void readData();
    void startAttack();
    void cancelAttack();

public:
    explicit ClientWindow(QDialog *parent, QString hostname, QTcpSocket *socket);
    explicit ClientWindow();
    ~ClientWindow();
    
private:
    QDialogButtonBox *buttonBox;
    QPushButton *passwordsFileButton;
    QPushButton *cancellAttackButton;
    QPushButton *startAttackButton;
    QPushButton *loginsFileButton;
    QLineEdit *passwordsLineEdit;
    QLineEdit *loginsLineEdit;
    QLineEdit *hostLineEdit;
    QTcpSocket *tcpSocket;
    QTextEdit *textEdit; 
    QLabel    *hostLabel;
    QString passwordsFileName;
    QString loginsFileName;
    QString hostName;
    quint16 port;
};

#endif // CLIENTWINDOW_H
