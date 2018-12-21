#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QString>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QTextEdit>
#include <QLabel>
#include <QFileDialog>
#include <QDateTime>
#include <QUrl>

#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QRegExp>


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_chooseFileButton_clicked();

    void on_translateButton_clicked();

    void on_clearButton_clicked();

    void on_historyButton_clicked();

private:

    struct STranslateSign
    {

        QString appID = "";
        QString q;
        QString salt = "1466355502";
        QString appPassword = "";
    } SSign;

    QByteArray GetSignMD5();


    Ui::Widget *ui;

    QString fileData;
    QString texteditData;
    QString srcData;

    QDir dirPath;
    QFile *translateFile;
    QTextStream *translateTextStream;

    bool isCreateFile;

    QDateTime *dateTime;
    inline QString CurrentTime(QString);

    QNetworkAccessManager *networkManager;

private slots:
    void on_replyFinished(QNetworkReply *reply);

    void on_keyLineEdit_returnPressed();
    void on_appIDLineEdit_returnPressed();
};

inline QString Widget::CurrentTime(QString font = "hh:mm:ss") { return dateTime->currentDateTime().toString(font); }

#endif // WIDGET_H
