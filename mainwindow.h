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

enum PagesX
{
    LOADPAGE,
    GAMEPAGE,
    STATPAGE
};

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
    void writeLog(QString log);
    void showPage(PagesX page);
    void interactableUI(bool enabled);

private slots:
    void updateWindow();

    void endOfGame();

    void receiveCurrent(const PixelStats &stat, NetworkResultFlags ok);

    void receiveStats(const QList<PixelStats> &stats, NetworkResultFlags ok);

    void logOut();

    void on_genNameBut_clicked();

    void on_loginIdBut_clicked();

    void on_checkedOnlineMode_checkStateChanged(const Qt::CheckState &arg1);

    void on_resetIDBut_clicked();

private:
    int _lineFlag;
    QSettings *settings;
    Ui::MainWindow *ui;
    QList<QWidget*> mPages;

    std::shared_ptr<PixelStats> currentAccount;
    std::shared_ptr<QList<PixelStats>> anyUsers;
    PixelNetwork *network;

    QTimer *timer;
};

#endif // MAINWINDOW_H
