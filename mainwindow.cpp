#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化时间
    dateTime = new QDateTime();

    //清空变量 防止初始化时候给了乱七八糟的值
    srcData.clear();
    appID.clear();
    key.clear();

    //combobox添加翻译的类型
    ui->fromComboBox->addItem("English","en");
    ui->fromComboBox->addItem("Chinese","zh");
    ui->fromComboBox->addItem("Auto","auto");
    ui->fromComboBox->setCurrentIndex(2);

    ui->toComboBox->addItem("English","en");
    ui->toComboBox->addItem("Chinese","zh");
    ui->toComboBox->setCurrentIndex(0);

    //链接网络
    networkManager = new QNetworkAccessManager(this);
    this->connect(networkManager,SIGNAL(finished(QNetworkReply *)),this,SLOT(on_replyFinished(QNetworkReply *)));

    //初始化ini文件
    QFile file("info.ini");
    //如果ini文件存在就修改文件
    if(file.exists())
    {
        infoSettings = new QSettings("info.ini",QSettings::IniFormat);

        ui->appIDLineEdit->setText(infoSettings->value("info/appid").toString());
        appID = ui->appIDLineEdit->text();
        ui->keyLineEdit->setText(infoSettings->value("info/key").toString());
        key = ui->keyLineEdit->text();
    }
    //如果ini文件不存在则创建一个
    else
    {
        //尝试创建一个文件 如果失败则提示并退出
        if(file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            file.close();
            //设置ini文件数据
            infoSettings = new QSettings("info.ini",QSettings::IniFormat);
            infoSettings->beginGroup("info");

            infoSettings->setValue("appid","00000000000000000");
            appID = "00000000000000000";
            ui->appIDLineEdit->setText("00000000000000000");

            infoSettings->setValue("key","00000000000000000000");
            key = "00000000000000000000";
            ui->keyLineEdit->setText("00000000000000000000");

            infoSettings->endGroup();
        }
        else
        {
            ui->statusBar->showMessage(tr("%1 - inifo.ini create failed!").arg(CurrentTime()), 1000);
        }
    }

    //初始化本地保存文件
    histrtyFile = new QFile("history.txt");
    //初始化成功
    if(histrtyFile->open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
    {
        historyFileStream = new QTextStream(histrtyFile);
        isCreateHistoryFile = true;
    }
    //初始化失败
    else
    {
        ui->statusBar->showMessage(tr("%1 - history file create failed").arg(CurrentTime()), 1000);
        isCreateHistoryFile = false;
    }
}

MainWindow::~MainWindow()
{
    //保存文件
    histrtyFile->close();

    delete ui;
}

void MainWindow::on_replyFinished(QNetworkReply *reply)
{
    //获得接受数据的形式
    QVariant rep = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    //如果没有接受错误
    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray data = reply->readAll();

        qDebug() << "data = " << data;

        //如果有错误的序列号
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
            //显示序列号
            ui->statusBar->showMessage(tr("%1 - Error code : %2").arg(CurrentTime()).arg(list), 1000);
            list.clear();
            return;
        }

        //用一个js类型变量获得返回值的内容
        QJsonObject jsData(QJsonDocument::fromJson(data).object());

        qDebug() << jsData;

        QString string = jsData["trans_result"].toArray()[0].toObject()["dst"].toString();

        //清除原先的信息
        ui->translateTextEdit->clear();
        //显示新的信息
        ui->translateTextEdit->insertPlainText(string);
        //保存到本地
        if(isCreateHistoryFile)
        {
            *historyFileStream << string << "\r\n";
        }
        //提示
        ui->statusBar->showMessage(tr("%1 - Request success!").arg(CurrentTime()), 1000);
    }
    //接受错误则直接提示
    else
    {
        ui->statusBar->showMessage(tr("%1 - Request error!").arg(CurrentTime()), 1000);
    }
}

QByteArray MainWindow::GetSignMD5()
{
    //拼接签名
    QString sign = appID + srcData.toUtf8() + salt + key;

    qDebug() << "sign=" << sign;

    //生成md5
    QByteArray md5 = QCryptographicHash::hash(sign.toUtf8(), QCryptographicHash::Md5);

    return md5;
}

void MainWindow::on_translateAction_triggered()
{
    //获得文本框的内容
    srcData = ui->srcTextEdit->toPlainText();

    //把所有的换行改成空格
    srcData = srcData.replace("\n"," ");

    //非空就翻译
    if(srcData.isEmpty())
    {
        ui->statusBar->showMessage(tr("%1 - Please enter translate data!").arg(CurrentTime()), 1000);
        return;
    }

    //设置翻译类型
    QString from = ui->fromComboBox->currentData().toString();
    QString to = ui->toComboBox->currentData().toString();
    //生成md5
    QString md5 = GetSignMD5().toHex();

    QString status = "q=" + srcData.toUtf8() + "&from=" + from + "&to=" + to + "&appid=" + appID + "&salt=" + salt + "&sign=" + md5;
    qDebug() << "status=" << status;

    //设置网络 发送请求
    QNetworkRequest request;
    request.setUrl(QUrl("http://api.fanyi.baidu.com/api/trans/vip/translate?"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::ContentLengthHeader,status.length());
    networkManager->post(request,status.toUtf8());
}

void MainWindow::on_openAction_triggered()
{
    //读取文件名称 读取失败直接退出
    QString filePath = QFileDialog::getOpenFileName(this,QString("Open"),dirPath.path(),"* txt");
    if(filePath.isEmpty())
    {
        ui->statusBar->showMessage(tr("%1 - Not choose any file!").arg(CurrentTime()), 1000);
        return ;
    }


    QFile file(filePath);
    //打开文件 成功则读取内容 显示于对话框
    if(file.open(QIODevice::ReadOnly))
    {
        ui->srcTextEdit->clear();
        ui->srcTextEdit->insertPlainText(file.readAll());
    }
    //失败则提示
    else
    {
        ui->statusBar->showMessage(tr("%1 - Open file failed!").arg(CurrentTime()), 1000);
    }

}

void MainWindow::on_saveAction_triggered()
{
    //关闭文件
    histrtyFile->close();
    //重新打开 打开失败则报错
    if(histrtyFile->open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text))
    {
        isCreateHistoryFile = true;
    }
    else
    {
        ui->statusBar->showMessage(tr("%1 - history file open failed").arg(CurrentTime()), 1000);
        isCreateHistoryFile = false;
    }
}

void MainWindow::on_clearAction_triggered()
{
    //清除src内容
    srcData.clear();
    ui->srcTextEdit->clear();
    ui->srcTextEdit->document()->clear();

    //清除translate内容
    ui->translateTextEdit->clear();
    ui->translateTextEdit->document()->clear();
}

void MainWindow::on_appIDLineEdit_returnPressed()
{
    //获取当前appid 空就退出
    appID = ui->appIDLineEdit->text();
    if(appID.isEmpty())
    {
        ui->statusBar->showMessage(tr("%1 - Please input effective appid!").arg(CurrentTime()), 1000);
        return;
    }

    //appid保存到ini文件
    infoSettings->beginGroup("info");
    infoSettings->setValue("appid",appID);
    infoSettings->endGroup();
    //提示保存成功
    ui->statusBar->showMessage(tr("%1 - Enter appid successful!").arg(CurrentTime()), 1000);
}

void MainWindow::on_keyLineEdit_returnPressed()
{
    //获取当前key 空就退出
    key = ui->keyLineEdit->text();
    if(key.isEmpty())
    {
        ui->statusBar->showMessage(tr("%1 - Please input effective key!").arg(CurrentTime()), 1000);
        return;
    }

    //key保存到ini文件
    infoSettings->beginGroup("info");
    infoSettings->setValue("key",key);
    infoSettings->endGroup();
    //提示保存成功
    ui->statusBar->showMessage(tr("%1 - Enter key successful!").arg(CurrentTime()), 1000);
}
