#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QDialog>
#include <QDebug>

class QLineEdit;
class QString;
class QLabel;
class QDialogButtonBox;
class QTcpSocket;

class ClientWindow : public QDialog
{
    Q_OBJECT

private slots:
    void loginsFileDialog();
    void passwordsFileDialog();
    void startAttack();
    void cancelAttack();

public:
    explicit ClientWindow(QDialog *parent, QString hostname, QTcpSocket *socket);
    explicit ClientWindow();
    ~ClientWindow();
    
private:
    QPushButton *loginsFileButton;
    QPushButton *passwordsFileButton;
    QPushButton *startAttackButton;
    QPushButton *cancellAttackButton;
    QDialogButtonBox *buttonBox;
    QLabel *hostLabel;
    QLineEdit *hostLineEdit;
    QLineEdit *loginsLineEdit;
    QLineEdit *passwordsLineEdit;
    QString loginsFileName;
    QString passwordsFileName;
    QString hostName;
    QTcpSocket *connectionSocket;
    quint16 port;
};

#endif // CLIENTWINDOW_H
