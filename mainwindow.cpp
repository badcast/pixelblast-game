#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), currentAccount {}, anyUsers {}
{
    settings = new QSettings("badcast", "Pixel Blast", this);

    m_PixelBlast = new PixelBlast(this);

    ui->setupUi(this);
    ui->overlay->addWidget(m_PixelBlast);

    timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(500);
    QObject::connect(timer, &QTimer::timeout, this, &MainWindow::updateWindow);
    timer->start();

    updateWindow();

    m_PixelBlast->setOnlineMode(false);
    m_PixelBlast->startGame();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateWindow()
{
    int i = 0;
    if(currentAccount)
    {
        i = currentAccount->maxPoints = qMax(currentAccount->maxPoints, m_PixelBlast->getScores());
    }
    ui->maxScoresText->setText(QString("Макс Очко: %1").arg(i));
}

void MainWindow::receiveCurrent(const PixelStats &stat, bool ok)
{
    if(!ok)
    {
        QMessageBox::warning(this, "Ошибка", "Ошибка подключения к интернету или серверная ошибка.");
        return;
    }
    currentAccount = std::make_shared<PixelStats>(stat);
}

void MainWindow::receiveStats(const QList<PixelStats> &stats, bool ok)
{
    if(!ok)
    {
        QMessageBox::warning(this, "Ошибка", "Ошибка подключения к интернету или серверная ошибка.");
        return;
    }
    anyUsers = std::make_shared<QList<PixelStats>>(stats);
}

void MainWindow::on_genNameBut_clicked()
{
    ui->textUserName->setText("Александр");
}

void MainWindow::on_resetIdBut_clicked()
{
    m_PixelBlast->resetGame();
}

void MainWindow::on_loginIdBut_clicked()
{
    m_PixelBlast->resetGame();

    if(m_PixelBlast->network == nullptr)
    {
        QMessageBox::warning("Состояние оффлайн", "Включите ONLINE режим.");
        return;
    }

    QString str = ui->textUserName->text();
    if(str.size() < 4)
    {
        QMessageBox::warning("Имя пользователя", "Имя пользователя не может быть меньше 4 символов");
        return;
    }

    if(currentAccount == nullptr)
    m_PixelBlast->network->newClient(str);
}

void MainWindow::on_checkedOnlineMode_checkStateChanged(const Qt::CheckState &arg1)
{
    m_PixelBlast->setOnlineMode(arg1 == Qt::CheckState::Checked);
}
