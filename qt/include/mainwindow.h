#pragma once

#include <QMainWindow>
#include <QSettings>

class QLabel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
    
private slots:
    void on_createPlanet_PushButton_clicked();

    void on_actionClear_Velocity_triggered();

    void on_speed_Dial_valueChanged(int value);
    void on_PauseResume_Button_clicked();
    void on_FastForward_Button_clicked();

    void on_actionNew_Simulation_triggered();
    void on_actionOpen_Simulation_triggered();
    void on_actionAppend_Simulation_triggered();
    bool on_actionSave_Simulation_triggered();
    void on_actionAbout_triggered();

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
    void on_actionDraw_Planar_Circles_toggled(bool value);

    void on_randomOrbitalCheckBox_toggled(bool checked);
    void on_generateRandomPushButton_clicked();

    void openRecentFile();

    void on_actionClear_triggered();

    /* Called on GL frame swap to update status bar messages and any other controls that may need it. */
    void frameUpdate();

private:
    /* String keys for settings. */
    const static QString categoryMainWindow;
    const static QString categoryGraphics;
    const static QString settingGeometry;
    const static QString settingState;
    const static QString settingDrawGrid;
    const static QString settingDrawPaths;
    const static QString settingGridDimensions;
    const static QString settingTrailLength;
    const static QString settingTrailDelta;
    const static QString settingStepsPerFrame;

    Ui::MainWindow* ui;

    /* the maximum value of the simulation speed dial. */
    static const int speeddialmax;
    int speedDialMemory;

    QSettings settings;

    /* These labels go in the statusbar. */
    QLabel* planetCountLabel;
    QLabel* fpsLabel;
    QLabel* averagefpsLabel;

    /* Read recent file list from settings. */
    QStringList getRecentFiles();

    /* Add a new file to recent file list in settings. */
    void addRecentFile(const QString& filename);

    /* Update the "Recent Files" menu. */
    void updateRecentFileActions();

    /* These actions are for individual recent file entries, the tooltips contain the full path. */
    QList<QAction*> recentFileActions;

    void closeEvent(QCloseEvent* e);

    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

    bool event(QEvent* event);
};
