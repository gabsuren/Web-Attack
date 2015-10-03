#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QAction;
QT_END_NAMESPACE

//namespace Ui {
//class MainWindow;
//}

class MainWindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void enableGetFortuneButton();
    void requestServerConnection();

private:
    QLabel *hostLabel;
    QLabel *portLabel;
    QLineEdit *hostLineEdit;
    QLineEdit *portLineEdit;
    QLabel *statusLabel;
    QPushButton *serverConnectionButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
    QString hostName;
    quint16 port;
};

#endif // MAINWINDOW_H
