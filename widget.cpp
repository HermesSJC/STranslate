#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    fileData.clear();
    texteditData.clear();
    isCreateFile = false;

    dateTime = new QDateTime();

    ui->fromComboBox->addItem("English","en");
    ui->fromComboBox->addItem("Chinese","zh");
    ui->fromComboBox->addItem("Auto","auto");
    ui->fromComboBox->setCurrentIndex(0);

    ui->toComboBox->addItem("English","en");
    ui->toComboBox->addItem("Chinese","zh");
    ui->toComboBox->setCurrentIndex(1);

    networkManager = new QNetworkAccessManager(this);
    this->connect(networkManager,SIGNAL(finished(QNetworkReply *)),this,SLOT(on_replyFinished(QNetworkReply *)));


    if(!dirPath.exists("Translate"))
    {
        dirPath.mkdir("Translate");
    }

    translateFile = new QFile(dirPath.path().append("/Translate/history.txt"));
    if(translateFile->open(QIODevice::Text | QIODevice::Append))
    {
        translateTextStream = new QTextStream(translateFile);
        isCreateFile = true;
    }
    else
    {
        ui->statusLabel->setText(tr("%1 - Translate file create failed").arg(CurrentTime()));
    }
}

Widget::~Widget()
{
    translateFile->close();

    delete ui;
}

void Widget::on_chooseFileButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,QString("Open"),dirPath.path(),"* txt");
    if(filePath.isEmpty())
    {
        ui->statusLabel->setText(tr("%1 - Not choose any file!").arg(CurrentTime()));
        return ;
    }

    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly))
    {
        fileData.clear();
        fileData.append(file.readAll());
    }
    else
    {
        ui->statusLabel->setText(tr("%1 - Open file failed!").arg(CurrentTime()));
    }
}

void Widget::on_translateButton_clicked()
{
    srcData.clear();
    texteditData.clear();
    texteditData = ui->srcTextEdit->toPlainText();
    if(!texteditData.isEmpty())
    {
        ui->translateTextEdit->clear();
        ui->translateTextEdit->document()->clear();

        texteditData.replace("\n", " ");

        srcData.append(texteditData);
        texteditData.clear();

    }
    else if(!fileData.isEmpty())
    {
        ui->translateTextEdit->clear();
        ui->translateTextEdit->document()->clear();

        fileData.replace("\r\n", " ");

        srcData.append(fileData);

        fileData.clear();
    }
    else
    {
        ui->statusLabel->setText(tr("%1 - There is no translate data!").arg(CurrentTime()));
        return;
    }

    QString from = ui->fromComboBox->currentData().toString();
    QString to = ui->toComboBox->currentData().toString();
    QString md5 = GetSignMD5().toHex();

    QString status = QString("q=%1&from=%2&to=%3&appid=%4&salt=%5&sign=%6").arg(srcData).arg(from).arg(to).arg(SSign.appID).arg(SSign.salt).arg(md5);

    QNetworkRequest request;
    request.setUrl(QUrl("http://api.fanyi.baidu.com/api/trans/vip/translate?"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::ContentLengthHeader,status.length());
    networkManager->post(request,status.toUtf8());
}

void Widget::on_clearButton_clicked()
{
    ui->srcTextEdit->clear();
    ui->srcTextEdit->document()->clear();

    ui->translateTextEdit->clear();
    ui->translateTextEdit->document()->clear();

    fileData.clear();
    texteditData.clear();
}

void Widget::on_historyButton_clicked()
{
    //保存文件 可以打开历史txt
    translateFile->close();
    translateFile->open(QIODevice::Text | QIODevice::Append);
}

void Widget::on_replyFinished(QNetworkReply *reply)
{

    QVariant rep = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray data = reply->readAll();

        int errorIndex = data.indexOf("error_code");
        if(errorIndex != -1)
        {
            QRegExp rx("(\\d+)");
            QString list;
            int pos = 0;
            while( (pos = rx.indexIn(data, pos)) != -1  )
            {
                list.append(rx.cap(1));
                pos += rx.matchedLength();
            }
            ui->statusLabel->setText(tr("%1 - Error code : %2").arg(CurrentTime()).arg(list));
            list.clear();
            return;
        }

        QJsonObject jsData(QJsonDocument::fromJson(data).object());
        QString string = jsData["trans_result"].toArray()[0].toObject()["dst"].toString();

        ui->translateTextEdit->insertPlainText(string);
        if(isCreateFile)
        {
            *translateTextStream << string << "\r\n";
        }

        ui->statusLabel->setText(tr("%1 - Request success!").arg(CurrentTime()));
    }
    else
    {
        ui->statusLabel->setText(tr("%1 - Request error!").arg(CurrentTime()));
    }
}


QByteArray Widget::GetSignMD5()
{
    QString sign;
    SSign.q = srcData;
    sign = SSign.appID + SSign.q + SSign.salt + SSign.appPassword;
    QByteArray md5 = QCryptographicHash::hash(sign.toUtf8(), QCryptographicHash::Md5);
    return md5;
}

void Widget::on_keyLineEdit_returnPressed()
{
    if(ui->keyLineEdit->text().isEmpty())
    {
        ui->statusLabel->setText(tr("%1 - Please input effective key!").arg(CurrentTime()));
        return;
    }

    SSign.appPassword = ui->keyLineEdit->text();
    ui->statusLabel->setText(tr("%1 - Enter key successful!").arg(CurrentTime()));

}

void Widget::on_appIDLineEdit_returnPressed()
{
    if(ui->appIDLineEdit->text().isEmpty())
    {
        ui->statusLabel->setText(tr("%1 - Please input effective appid!").arg(CurrentTime()));
        return;
    }

    SSign.appID = ui->appIDLineEdit->text();
    ui->statusLabel->setText(tr("%1 - Enter appid successful!").arg(CurrentTime()));
}
