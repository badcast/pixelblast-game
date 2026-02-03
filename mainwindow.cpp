#include <QMessageBox>
#include <QRandomGenerator>

#include "mainwindow.h"
#include "ui_mainwindow.h"

// TODO: Make Game Over custom.

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
void resetIDSettings(QSettings *settings)
{
    if(settings)
    {
        settings->remove("ID");
        settings->remove("NAME");
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
    if(result.first && !result.second.name.isEmpty())
    {
        currentAccount = std::make_shared<PixelStats>(result.second);
        currentAccount->maxPoints = -1;
        ui->textUserName->setText(result.second.name);
    }
    else
    {
        resetIDSettings(settings);
        ui->textUserName->setText(generateNick());
    }
    showLoadPage(false);
    setOnlineMode(false);
    writeLog("Игра запущена.");
    setWindowTitle("Pixel Blast Game");

    updateWindow();
}

MainWindow::~MainWindow()
{
    if(currentAccount)
    {
        if(isOnline())
            pxbModule->network->updateStats(*currentAccount);
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
        pxbModule->network->readStats();
        if(currentAccount)
        {
            currentAccount->maxPoints = 0;
            pxbModule->network->updateStats(*currentAccount);
        }
    }
    else
    {
        writeLog("Вы в состояний оффлайн");
    }
    interactableUI(true);
    showableUI(value);

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

void MainWindow::showableUI(bool value)
{
    ui->label->setVisible(value);
    ui->loginIdBut->setVisible(value);
    ui->textUserName->setVisible(value);
    ui->resetIDBut->setVisible(value);
    ui->genNameBut->setVisible(value);
}

void MainWindow::receiveCurrent(const PixelStats &stat, bool ok)
{
    // hide load page
    showLoadPage(false);
    interactableUI(true);
    if(currentAccount)
        currentAccount->maxPoints = -1;
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
    showLoadPage(false);
    interactableUI(true);
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
                break;
            }
        }
    }
    else
    {
        showLoadPage(false);
    }
}

void MainWindow::updateWindow()
{
    int x = 0, y = 0;

    if(!pxbModule->isPlaying())
        return;

    if(isOnline() && currentAccount)
    {
        y = currentAccount->rankPos;
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
        pxbModule->network->updateStats(*currentAccount);
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
    if(pxbModule->network == nullptr)
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
    showLoadPage(true);

    if(currentAccount)
    {
        currentAccount->name = str;
        currentAccount->maxPoints = 0;
        pxbModule->network->updateStats(*currentAccount);
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
        resetIDSettings(settings);
        anyUsers.reset();
        currentAccount.reset();
        pxbModule->startGame();
        setOnlineMode(false);
        ui->textUserName->setText(generateNick());
    }
}
