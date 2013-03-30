#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_actionExit_triggered();

    void on_createPlanet_PushButton_clicked();

    void on_speed_Dial_valueChanged(int value);

    void on_PauseResume_Button_clicked();

    void on_actionTake_Screenshot_triggered();

    void on_actionDelete_triggered();

    void on_actionClear_Velocity_triggered();

    void on_actionNew_Simulation_triggered();

    void on_actionCenter_All_triggered();

    void on_actionInteractive_Planet_Placement_triggered();

    void on_actionOff_triggered();

    void on_actionLines_triggered();

    void on_actionPoints_triggered();

    void on_actionOpen_Simulation_triggered();

    void on_actionSave_Simulation_triggered();

    void on_actionDraw_Paths_triggered();

    void on_actionMotion_Blur_triggered();

    void on_followPlanetPushButton_clicked();

    void on_clearFollowPushButton_clicked();

    void on_actionAbout_triggered();

    void on_actionAbout_Qt_triggered();

    void on_followPlainAveragePushButton_clicked();

    void on_followWeightedAveragePushButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
