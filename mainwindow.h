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

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
