#include <QMessageBox>
#include <QRandomGenerator>

#include "mainwindow.h"
#include "ui_mainwindow.h"

// TODO: Make Game Over custom.

#ifndef NICKNAMES_RAW
#define NICKNAMES_RAW "Name not defined"
#endif

QString generateNick()
{
    QString result {};
    QStringList _nicks = QString(NICKNAMES_RAW).split('\n', Qt::SkipEmptyParts);
    if(!_nicks.isEmpty())
    {
        QRandomGenerator *glob = QRandomGenerator::global();
        int i = static_cast<int>(glob->bounded(50) < 25) + 1;
        while(i-- > 0)
        {
            result += _nicks[glob->bounded(_nicks.size())].trimmed();
        }
    }
    else
    {
        result = "Unknown";
    }
    return result;
}

std::pair<bool, PixelStats> readFromSettings(QSettings *settings)
{
    PixelStats p {};
    bool res = settings != nullptr && settings->contains("ID") && settings->contains("NAME");
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
void resetIDSettings(QSettings *settings)
{
    if(settings)
    {
        settings->remove("ID");
        settings->remove("NAME");
    }
}
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), currentAccount {}, anyUsers {}, network(nullptr), _lineFlag(0)
{
    int x;
    ui->setupUi(this);

    writeLog("Инициализация...");

    settings = new QSettings("badcast", "Pixel Blast", this);

    mPages.resize(3);
    mPages[0] = ui->tabs->widget(0);
    mPages[1] = ui->tabs->widget(1);
    mPages[2] = ui->tabs->widget(2);

    pxbModule = new PixelBlast();
    ui->pxbContent->layout()->addWidget(pxbModule);
    QObject::connect(pxbModule, &PixelBlast::endOfGame, this, &MainWindow::endOfGame);

    ui->tabs->deleteLater();
    ui->overlay->addWidget(mPages[0]);
    ui->overlay->addWidget(mPages[1]);
    ui->overlay->addWidget(mPages[2]);

    auto result = readFromSettings(settings);
    if(result.first && !result.second.name.isEmpty())
    {
        currentAccount = std::make_shared<PixelStats>(result.second);
        currentAccount->maxPoints = -1;
    }
    else
    {
        resetIDSettings(settings);
        ui->textUserName->setText(generateNick());
    }

    setOnlineMode(false);
    writeLog("Игра запущена.");
    setWindowTitle("Pixel Blast Game");

    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(500);
    QObject::connect(timer, &QTimer::timeout, this, &MainWindow::updateWindow);
    timer->start();

    showPage(GAMEPAGE);
    updateWindow();
}

MainWindow::~MainWindow()
{
    if(currentAccount)
    {

        if(isOnline())
            network->updateStats(*currentAccount);
        writeToSettings(settings, *currentAccount);
    }
    else
    {
        resetIDSettings(settings);
    }
    delete ui;
}

bool MainWindow::isOnline()
{
    return pxbModule != nullptr && network != nullptr;
}

void MainWindow::setOnlineMode(bool value)
{
    anyUsers.reset();
    _lineFlag = 0;

    if(value)
    {
        network = new PixelNetwork(this);
        QObject::connect(network, &PixelNetwork::callbackCurrent, this, &MainWindow::receiveCurrent);
        QObject::connect(network, &PixelNetwork::callbackUsers, this, &MainWindow::receiveStats);
        network->readUsers();
        pxbModule->resetGame();
        if(currentAccount)
        {
            currentAccount->maxPoints = 0;
            network->updateStats(*currentAccount);
        }
        _lineFlag |= 4;
    }
    else
    {
        logOut();
    }
    pxbModule->startGame();
}

void MainWindow::writeLog(QString log)
{
    qDebug() << log;
    ui->statusBar->setText(log);
}

void MainWindow::showPage(PagesX page)
{
    if(page < 0 || page >= mPages.size())
        return;
    for(int x = 0; x < mPages.size(); ++x)
    {
        mPages[x]->setVisible(x == static_cast<int>(page));
    }
}

void MainWindow::interactableUI(bool enabled)
{
    if(ui->genNameBut) ui->genNameBut->setEnabled(enabled);
    if(ui->loginIdBut) ui->loginIdBut->setEnabled(enabled);
    if(ui->textUserName) ui->textUserName->setEnabled(enabled);
    if(ui->checkedOnlineMode) ui->checkedOnlineMode->setEnabled(enabled);
    if(ui->resetIDBut) ui->resetIDBut->setEnabled(enabled);
}

void MainWindow::receiveCurrent(const PixelStats &stat, NetworkResultFlags state)
{
    // hide load page
    if(state)
    {
        _lineFlag &= ~2;
        writeLog("Ошибка подключения к интернету или серверная ошибка.");
        // QMessageBox::warning(this, "", "Ошибка подключения. Проверьте связь.");
        return;
    }
    _lineFlag |= 2;
    currentAccount = std::make_shared<PixelStats>(stat);
    ui->textUserName->setText(currentAccount->name);
    // write id
    writeToSettings(settings, stat);
    writeLog("Успешно подключен к серверу.");
}

void MainWindow::receiveStats(const QList<PixelStats> &stats, NetworkResultFlags state)
{
    interactableUI(true);
    if(state)
    {
        _lineFlag &= ~1;
        writeLog("Ошибка подключения к интернету или серверная ошибка.");
        return;
    }
    _lineFlag |= 1;
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
                break;
            }
        }
    }
}

void MainWindow::logOut()
{
    if(network)
        delete network;
    network = nullptr;
    pxbModule->resetGame();
    writeLog("Вы в состояний оффлайн");
}

void MainWindow::updateWindow()
{
    int x = 0, y = 0;

    if(!pxbModule->isPlaying())
        return;

    if(isOnline() && currentAccount)
    {
        y = currentAccount->rankPos;

        // after init network data, show game page.
        if(_lineFlag == 0x7)
        {
            _lineFlag &= ~4;
            showPage(GAMEPAGE);
        }
    }
    // get max scores offline or online
    x = pxbModule->getScores();
    ui->maxScoresText->setText(QString("Макс Очко: %1, В топе: %2").arg(x).arg(y));
}

void MainWindow::endOfGame()
{
    if(isOnline() && currentAccount)
    {
        currentAccount->maxPoints = pxbModule->getScores();
        network->updateStats(*currentAccount);
    }

    writeLog("Конец игры. Перезапустите игру (нажать снова ВХОД)");

    int result = QMessageBox::question(this, "Конец игры.", QString("Вы закончили игру со счётом %1 очков. Перезапустить игру?").arg(pxbModule->getScores()), QMessageBox::Yes, QMessageBox::No);
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
    if(network == nullptr)
    {
        writeLog("Состояние оффлайн. Включите ONLINE режим.");
        return;
    }

    QString str = ui->textUserName->text();
    bool hasInvalidSym = std::all_of(std::begin(str), std::end(str), [](const QChar &c) { return c.isDigit() || c.isLetter() || (c == ' '); });
    if(!hasInvalidSym || str.size() < 4 || str.size() > 24)
    {
        QMessageBox::warning(this, "Имя пользователя", "Имя пользователя не может быть меньше 4 символов и не более 24 символов. Разрешено использовать только буквы и цифры с пробелом.");
        return;
    }

    interactableUI(false);
    pxbModule->resetGame();
    showPage(LOADPAGE);

    if(currentAccount)
    {
        currentAccount->name = str;
        currentAccount->maxPoints = 0;
        network->updateStats(*currentAccount);
        network->readUsers();
    }
    else
    {
        network->newClient(str);
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
        resetIDSettings(settings);
        anyUsers.reset();
        currentAccount.reset();
        pxbModule->startGame();
        setOnlineMode(false);
        ui->textUserName->setText(generateNick());
    }
}
