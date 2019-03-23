#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>

#include <QString>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QTextEdit>
#include <QLabel>
#include <QFileDialog>
#include <QDateTime>
#include <QUrl>
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <QTextCodec>

#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QRegExp>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QDateTime *dateTime;
    inline QString CurrentTime(QString);

    QFile *histrtyFile;
    QTextStream *historyFileStream;
    bool isCreateHistoryFile;

    QString appID;
    QString srcData;
    QString salt = "1466355502";
    QString key;

    QByteArray GetSignMD5(void);

    QDir dirPath;

    QNetworkAccessManager *networkManager;

    QSettings *infoSettings;

private slots:
    void on_replyFinished(QNetworkReply *reply);

    void on_appIDLineEdit_returnPressed();

    void on_keyLineEdit_returnPressed();

    void on_translateAction_triggered();

    void on_openAction_triggered();

    void on_saveAction_triggered();
    void on_clearAction_triggered();
};


inline QString MainWindow::CurrentTime(QString font = "hh:mm:ss") { return dateTime->currentDateTime().toString(font); }

#endif // MAINWINDOW_H
