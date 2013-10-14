#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

namespace Ui {
class MainWindow;
}

// the maximum value of the simulation speed dial as a float.
const float speeddialmax = 10.0f;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_createPlanet_PushButton_clicked();
    void on_actionDelete_triggered();
    void on_actionClear_Velocity_triggered();

    void on_speed_Dial_valueChanged(int value);
    void on_PauseResume_Button_clicked();
    void on_FastForward_Button_clicked();

    void on_actionDraw_Paths_triggered();
    void on_actionPlanet_Colors_triggered();

    void on_actionNew_Simulation_triggered();
    void on_actionOpen_Simulation_triggered();
    void on_actionSave_Simulation_triggered();

    void on_followPlanetPushButton_clicked();
    void on_clearFollowPushButton_clicked();
    void on_followPlainAveragePushButton_clicked();
    void on_followWeightedAveragePushButton_clicked();

    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();

    void on_stepsPerFrameSpinBox_valueChanged(int value);
    void on_trailLengthSpinBox_valueChanged(int value);

    void on_firingVelocityDoubleSpinBox_valueChanged(double value);
    void on_firingMassSpinBox_valueChanged(int value);

    void on_actionGrid_toggled(bool value);

private:
    Ui::MainWindow *ui;

    QLabel *planetCountLabel;
    QLabel *fpsLabel;
    QLabel *averagefpsLabel;

    void closeEvent(QCloseEvent *e);
};

#endif // MAINWINDOW_H
