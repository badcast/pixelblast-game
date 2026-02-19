#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QMainWindow>
#include <QSettings>
#include <QTimer>

#include "PixelBegin.h"
#include "PixelBlastGame.h"
#include "PixelNetwork.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    PixelBlast *pxbModule;

    bool isOnline();
    void setOnlineMode(bool value);


private:
    void showLoadPage(bool value);
    void writeLog(QString log);
    void interactableUI(bool value);
    void showableUI(bool value);

private slots:
    void updateWindow();

    void endOfGame();

    void receiveCurrent(const PixelStats &stat, bool ok);

    void receiveStats(const QList<PixelStats> &stats, bool ok);

    void on_genNameBut_clicked();

    void on_loginIdBut_clicked();

    void on_checkedOnlineMode_checkStateChanged(const Qt::CheckState &arg1);

    void on_resetIDBut_clicked();

private:
    int onlineSetup;
    QSettings *settings;
    Ui::MainWindow *ui;

    std::shared_ptr<PixelStats> currentAccount;
    std::shared_ptr<QList<PixelStats>> anyUsers;
    PixelNetwork *network;

    QTimer *timer;
};

#endif // MAINWINDOW_H
