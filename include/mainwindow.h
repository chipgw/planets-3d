#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

class QLabel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_createPlanet_PushButton_clicked();
    void on_actionClear_Velocity_triggered();

    void on_speed_Dial_valueChanged(int value);
    void on_PauseResume_Button_clicked();
    void on_FastForward_Button_clicked();

    void on_actionNew_Simulation_triggered();
    void on_actionOpen_Simulation_triggered();
    void on_actionSave_Simulation_triggered();

    void on_actionFollow_Selection_triggered();
    void on_actionClear_Follow_triggered();
    void on_actionPlain_Average_triggered();
    void on_actionWeighted_Average_triggered();

    void on_actionAbout_triggered();
    void on_actionAbout_Qt_triggered();

    void on_stepsPerFrameSpinBox_valueChanged(int value);
    void on_trailLengthSpinBox_valueChanged(int value);
    void on_trailRecordDistanceDoubleSpinBox_valueChanged(double value);
    void on_planetScaleDoubleSpinBox_valueChanged(double value);

    void on_firingVelocityDoubleSpinBox_valueChanged(double value);
    void on_firingMassSpinBox_valueChanged(int value);

    void on_actionGrid_toggled(bool value);
    void on_actionDraw_Paths_toggled(bool value);
    void on_actionPlanet_Colors_toggled(bool value);
    void on_actionHide_Planets_toggled(bool value);

    void on_generateRandomPushButton_clicked();

private:
    // the maximum value of the simulation speed dial.
    static const int speeddialmax;

    QSettings settings;

    Ui::MainWindow *ui;

    QLabel *planetCountLabel;
    QLabel *fpsLabel;
    QLabel *averagefpsLabel;

    void closeEvent(QCloseEvent *e);
};

#endif // MAINWINDOW_H
