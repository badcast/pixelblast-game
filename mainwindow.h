#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QMainWindow>
#include <QSettings>
#include <QTimer>

#include "PixelBlastGame.h"

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

    PixelBlast *m_PixelBlast;

private slots:
    void updateWindow();

    void receiveCurrent(const PixelStats &stat, bool ok);

    void receiveStats(const QList<PixelStats> &stats, bool ok);

    void on_genNameBut_clicked();

    void on_resetIdBut_clicked();

    void on_loginIdBut_clicked();

    void on_checkedOnlineMode_checkStateChanged(const Qt::CheckState &arg1);

private:
    QSettings *settings;
    Ui::MainWindow *ui;

    std::shared_ptr<PixelStats> currentAccount;
    std::shared_ptr<QList<PixelStats>> anyUsers;

    QTimer *timer;
};

#endif // MAINWINDOW_H
