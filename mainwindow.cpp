#include <QMessageBox>
#include <QRandomGenerator>

#include "mainwindow.h"
#include "ui_mainwindow.h"

QString generateNick()
{
    QString result {};
    QStringList _nicks = QString(NICKNAMES_RAW).split('\n');
    if(!_nicks.isEmpty())
    {
        auto glob = QRandomGenerator::global();
        int i = static_cast<int>(glob->bounded(50) < 25) + 1;
        while(i-- > 0)
        {
            result += _nicks[glob->bounded(_nicks.size() - 1)];
        }
    }
    else
    {
        result = ("Unknown");
    }
    return result;
}

std::pair<bool, PixelStats> readFromSettings(QSettings *settings)
{
    PixelStats p {};
    bool res = settings != nullptr || settings->contains("ID") && settings->contains("NAME");
    if(res)
    {
        p.id = settings->value("ID").toInt();
        p.name = settings->value("NAME").toString();
    }
    return {res, p};
}

void writeToSettings(QSettings *settings, const PixelStats &pb)
{
    if(settings != nullptr)
    {
        settings->setValue("ID", pb.id);
        settings->setValue("NAME", pb.name);
    }
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), currentAccount {}, anyUsers {}
{
    settings = new QSettings("badcast", "Pixel Blast", this);

    pxbModule = new PixelBlast(this);

    ui->setupUi(this);
    ui->overlay->addWidget(pxbModule);

    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(500);
    QObject::connect(timer, &QTimer::timeout, this, &MainWindow::updateWindow);
    timer->start();

    QObject::connect(pxbModule, &PixelBlast::endOfGame, this, &MainWindow::endOfGame);

    writeLog("Инициализация...");
    auto result = readFromSettings(settings);
    if(result.first)
    {
        currentAccount = std::make_shared<PixelStats>(result.second);
        currentAccount->maxPoints = -1;
        ui->textUserName->setText(result.second.name);
    }
    setOnlineMode(false);
    writeLog("Игра запущена.");
    setWindowTitle("Pixel Blast Game");

    updateWindow();
}

MainWindow::~MainWindow()
{
    if(currentAccount)
    {
        writeToSettings(settings, *currentAccount);
    }
    else
    {
        settings->remove("ID");
        settings->remove("NAME");
    }
    delete ui;
}

bool MainWindow::isOnline()
{
    return pxbModule != nullptr && pxbModule->network != nullptr;
}

void MainWindow::setOnlineMode(bool value)
{
    ui->checkedOnlineMode->blockSignals(true);
    ui->checkedOnlineMode->setChecked(value);
    ui->checkedOnlineMode->blockSignals(false);
    pxbModule->setOnlineMode(value);
    anyUsers.reset();
    if(value)
    {
        QObject::connect(pxbModule->network, &PixelNetwork::callbackCurrent, this, &MainWindow::receiveCurrent);
        QObject::connect(pxbModule->network, &PixelNetwork::callbackStats, this, &MainWindow::receiveStats);
        if(currentAccount)
        {
            if(currentAccount->maxPoints > -1)
                pxbModule->network->updateStats(*currentAccount);
            else
                pxbModule->network->readStats();
            interactableUI(false);
        }
        else
            pxbModule->network->readStats();
    }
    else
    {
        writeLog("Вы в состояний оффлайн");
    }
    showLoadPage(value);

    pxbModule->startGame();
}

void MainWindow::showLoadPage(bool value)
{
    ui->loadingText->setVisible(value);
    pxbModule->setVisible(!value);
}

void MainWindow::writeLog(QString log)
{
    qDebug() << log;
    ui->statusBar->setText(log);
}

void MainWindow::interactableUI(bool value)
{
    ui->loginIdBut->setEnabled(value);
    ui->textUserName->setEnabled(value);
    ui->checkedOnlineMode->setEnabled(value);
    ui->resetIDBut->setEnabled(value);
    ui->genNameBut->setEnabled(value);
}

void MainWindow::receiveCurrent(const PixelStats &stat, bool ok)
{
    // hide load page
    showLoadPage(false);
    currentAccount->maxPoints = -1;
    interactableUI(true);
    if(!ok)
    {
        writeLog("Ошибка подключения к интернету или серверная ошибка.");
        return;
    }
    writeLog("Успешно подключен к серверу.");
    currentAccount = std::make_shared<PixelStats>(stat);
    ui->textUserName->setText(currentAccount->name);
    // write id
    writeToSettings(settings, stat);
    pxbModule->network->readStats();
}

void MainWindow::receiveStats(const QList<PixelStats> &stats, bool ok)
{
    if(!ok)
    {
        writeLog("Ошибка подключения к интернету или серверная ошибка.");
        return;
    }
    writeLog("Успешно подключен к серверу. Имена получены.");
    anyUsers = std::make_shared<QList<PixelStats>>(stats);
    std::sort(std::begin(*anyUsers), std::end(*anyUsers), [](auto &lhs, auto &rhs) { return lhs.maxPoints > rhs.maxPoints; });
    if(currentAccount)
    {
        for(int i = 0; i < anyUsers->size(); ++i)
        {
            if((*anyUsers)[i].id == currentAccount->id)
            {
                currentAccount->rankPos = i + 1;
                if(currentAccount->maxPoints == -1)
                {
                    currentAccount->maxPoints = qMax(currentAccount->maxPoints, (*anyUsers)[i].maxPoints);
                    pxbModule->network->updateStats(*currentAccount);
                }
                break;
            }
        }
    }
}

void MainWindow::updateWindow()
{
    int x = 0, y = 0;

    if(!pxbModule->isPlaying())
        return;

    if(isOnline() && anyUsers && currentAccount)
    {
        if(currentAccount->maxPoints > -1 && pxbModule->getFrames() % 60 * 3 == 0)
        {
            // Send update data
            pxbModule->network->updateStats(*currentAccount);
        }
        y = currentAccount->rankPos;
    }
    // get max scores offline or online
    x = pxbModule->getScores();
    if(currentAccount && currentAccount->maxPoints > -1)
    {
        x = currentAccount->maxPoints = qMax(currentAccount->maxPoints, x);
    }
    ui->maxScoresText->setText(QString("Макс Очко: %1, В топе: %2").arg(x).arg(y));
}

void MainWindow::endOfGame()
{
    if(isOnline() && currentAccount)
    {
        pxbModule->network->updateStats(*currentAccount);
    }

    writeLog("Конец игры. Перезапустите игру (нажать снова ВХОД)");

    int result = QMessageBox::question(this, "Конец игры.", "Перезапустить игру?", QMessageBox::Yes, QMessageBox::No);
    if(result == QMessageBox::Yes)
    {
        pxbModule->startGame();
    }
}

void MainWindow::on_genNameBut_clicked()
{
    ui->textUserName->setText(generateNick());
}

void MainWindow::on_loginIdBut_clicked()
{
    writeLog("Вход на сервер...");
    if(pxbModule->network == nullptr)
    {
        writeLog("Состояние оффлайн. Включите ONLINE режим.");
        return;
    }

    QString str = ui->textUserName->text();
    if(str.size() < 4)
    {
        QMessageBox::warning(this, "Имя пользователя", "Имя пользователя не может быть меньше 4 символов");
        return;
    }

    interactableUI(false);
    pxbModule->resetGame();
    showLoadPage(true);

    if(currentAccount)
    {
        currentAccount->name = str;
        if(currentAccount->maxPoints > -1)
            pxbModule->network->updateStats(*currentAccount);
        else
            pxbModule->network->readStats();
    }
    else
    {
        pxbModule->network->newClient(str);
    }
}

void MainWindow::on_checkedOnlineMode_checkStateChanged(const Qt::CheckState &arg1)
{
    setOnlineMode(arg1 == Qt::CheckState::Checked);
}

void MainWindow::on_resetIDBut_clicked()
{
    int result = QMessageBox::question(this, "Сброс данных", "Это привидет к сбросу вашего статуса и ID. Вы уверены?", QMessageBox::Yes, QMessageBox::No);
    if(result == QMessageBox::Yes)
    {
        anyUsers.reset();
        currentAccount.reset();
        pxbModule->startGame();
        setOnlineMode(false);
    }
}
