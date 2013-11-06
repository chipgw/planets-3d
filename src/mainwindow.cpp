#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->PauseResume_Button->setFocus();

    connect(ui->actionExit,     SIGNAL(triggered()), this,                      SLOT(close()));
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), QApplication::instance(),  SLOT(aboutQt()));

    connect(ui->actionCenter_All,                       SIGNAL(triggered()),        &ui->centralwidget->universe,   SLOT(centerAll()));
    connect(ui->actionInteractive_Planet_Placement,     SIGNAL(triggered()),        ui->centralwidget,              SLOT(beginInteractiveCreation()));
    connect(ui->actionInteractive_Orbital_Placement,    SIGNAL(triggered()),        ui->centralwidget,              SLOT(beginOrbitalCreation()));
    connect(ui->actionToggle_Firing_Mode,               SIGNAL(toggled(bool)),      ui->centralwidget,              SLOT(enableFiringMode(bool)));
    connect(ui->actionTake_Screenshot,                  SIGNAL(triggered()),        ui->centralwidget,              SLOT(takeScreenshot()));
    connect(ui->gridRangeSpinBox,                       SIGNAL(valueChanged(int)),  ui->centralwidget,              SLOT(setGridRange(int)));

    planetCountLabel = new QLabel(ui->statusbar);
    planetCountLabel->setFixedWidth(120);
    ui->statusbar->addPermanentWidget(planetCountLabel);
    fpsLabel = new QLabel(ui->statusbar);
    fpsLabel->setFixedWidth(120);
    ui->statusbar->addPermanentWidget(fpsLabel);
    averagefpsLabel = new QLabel(ui->statusbar);
    averagefpsLabel->setFixedWidth(160);
    ui->statusbar->addPermanentWidget(averagefpsLabel);

    connect(&ui->centralwidget->universe,   SIGNAL(updatePlanetCountMessage(QString)),      planetCountLabel, SLOT(setText(QString)));
    connect(ui->centralwidget,              SIGNAL(updateFPSStatusMessage(QString)),        fpsLabel,         SLOT(setText(QString)));
    connect(ui->centralwidget,              SIGNAL(updateAverageFPSStatusMessage(QString)), averagefpsLabel,  SLOT(setText(QString)));

    connect(ui->actionNew_Planet,           SIGNAL(toggled(bool)),              ui->newPlanet_DockWidget,       SLOT(setVisible(bool)));
    connect(ui->newPlanet_DockWidget,       SIGNAL(visibilityChanged(bool)),    ui->actionNew_Planet,           SLOT(setChecked(bool)));
    connect(ui->actionSpeed_Control,        SIGNAL(toggled(bool)),              ui->speedControl_DockWidget,    SLOT(setVisible(bool)));
    connect(ui->speedControl_DockWidget,    SIGNAL(visibilityChanged(bool)),    ui->actionSpeed_Control,        SLOT(setChecked(bool)));
    connect(ui->actionCamera_Location,      SIGNAL(toggled(bool)),              ui->camera_DockWidget,          SLOT(setVisible(bool)));
    connect(ui->camera_DockWidget,          SIGNAL(visibilityChanged(bool)),    ui->actionCamera_Location,      SLOT(setChecked(bool)));
    connect(ui->actionView_Settings,        SIGNAL(toggled(bool)),              ui->viewSettings_DockWidget,    SLOT(setVisible(bool)));
    connect(ui->viewSettings_DockWidget,    SIGNAL(visibilityChanged(bool)),    ui->actionView_Settings,        SLOT(setChecked(bool)));
    connect(ui->actionFiring_Mode_Settings, SIGNAL(toggled(bool)),              ui->firingSettings_DockWidget,  SLOT(setVisible(bool)));
    connect(ui->firingSettings_DockWidget,  SIGNAL(visibilityChanged(bool)),    ui->actionFiring_Mode_Settings, SLOT(setChecked(bool)));
}

MainWindow::~MainWindow(){
    delete ui;
    delete planetCountLabel;
    delete averagefpsLabel;
    delete fpsLabel;
}

void MainWindow::closeEvent(QCloseEvent *e){
    if(!ui->centralwidget->universe.isEmpty()){
        float tmpsimspeed = ui->centralwidget->universe.simspeed;
        ui->centralwidget->universe.simspeed = 0.0f;

        if(QMessageBox::warning(this, tr("Are You Sure?"), tr("Are you sure you wish to exit? (universe will not be saved...)"),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No){
            ui->centralwidget->universe.simspeed = tmpsimspeed;
            e->ignore();
            return;
        }
    }
    e->accept();
}

void MainWindow::on_createPlanet_PushButton_clicked(){
    ui->centralwidget->universe.selected = ui->centralwidget->universe.addPlanet(
                Planet(QVector3D(ui->newPosX_SpinBox->value(),      ui->newPosY_SpinBox->value(),      ui->newPosZ_SpinBox->value()),
                       QVector3D(ui->newVelocityX_SpinBox->value(), ui->newVelocityY_SpinBox->value(), ui->newVelocityZ_SpinBox->value()) * velocityfac,
                       ui->newMass_SpinBox->value()));
}

void MainWindow::on_actionDelete_triggered(){
    ui->centralwidget->universe.remove(ui->centralwidget->universe.selected);
}

void MainWindow::on_actionClear_Velocity_triggered(){
    if(ui->centralwidget->universe.isValid(ui->centralwidget->universe.selected)){
        ui->centralwidget->universe[ui->centralwidget->universe.selected].velocity = QVector3D();
    }
}

void MainWindow::on_speed_Dial_valueChanged(int value){
    ui->centralwidget->universe.simspeed = (value * speeddialmax) / ui->speed_Dial->maximum();
    ui->speedDisplay_lcdNumber->display(ui->centralwidget->universe.simspeed);
    if(ui->centralwidget->universe.simspeed <= 0.0f){
        ui->PauseResume_Button->setText(tr("Resume"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/control_play_blue.png"));
    }else{
        ui->PauseResume_Button->setText(tr("Pause"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/control_pause_blue.png"));
    }
}

void MainWindow::on_PauseResume_Button_clicked(){
    if(ui->speed_Dial->value() == 0){
        ui->speed_Dial->setValue(ui->speed_Dial->maximum() / speeddialmax);
    }else{
        ui->speed_Dial->setValue(0);
    }
}

void MainWindow::on_FastForward_Button_clicked(){
    if(ui->speed_Dial->value() >= ui->speed_Dial->maximum() * 3 / 4){
        ui->speed_Dial->setValue(ui->speed_Dial->maximum() / speeddialmax);
    }else if(ui->speed_Dial->value() <= ui->speed_Dial->maximum() / 4){
        ui->speed_Dial->setValue(ui->speed_Dial->maximum() / 2);
    }else{
        ui->speed_Dial->setValue(ui->speed_Dial->maximum());
    }
}

void MainWindow::on_actionNew_Simulation_triggered(){
    if(!ui->centralwidget->universe.isEmpty()){
        float tmpsimspeed = ui->centralwidget->universe.simspeed;
        ui->centralwidget->universe.simspeed = 0.0f;

        if(QMessageBox::warning(this, tr("Are You Sure?"), tr("Are you sure you wish to destroy the universe? (i.e. delete all planets.)"),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes){
            ui->centralwidget->universe.deleteAll();
        }
        ui->centralwidget->universe.simspeed = tmpsimspeed;
    }
}

void MainWindow::on_actionOpen_Simulation_triggered(){
    float tmpsimspeed = ui->centralwidget->universe.simspeed;
    ui->centralwidget->universe.simspeed = 0.0f;

    QString filename = QFileDialog::getOpenFileName(this, tr("Open Simulation"), "", tr("Simulation files (*.xml)"));
    ui->centralwidget->universe.load(filename);

    ui->centralwidget->universe.simspeed = tmpsimspeed;
}

void MainWindow::on_actionSave_Simulation_triggered(){
    float tmpsimspeed = ui->centralwidget->universe.simspeed;
    ui->centralwidget->universe.simspeed = 0.0f;

    if(!ui->centralwidget->universe.isEmpty()){
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Simulation"), "", tr("Simulation files (*.xml)"));
        ui->centralwidget->universe.save(filename);
    }else{
        QMessageBox::warning(this, tr("Error Saving Simulation."), tr("No planets to save!"));
    }

    ui->centralwidget->universe.simspeed = tmpsimspeed;
}

void MainWindow::on_followPlanetPushButton_clicked(){
    ui->centralwidget->following = ui->centralwidget->universe.selected;
    ui->centralwidget->followState = PlanetsWidget::Single;
}

void MainWindow::on_clearFollowPushButton_clicked(){
    ui->centralwidget->following = 0;
    ui->centralwidget->followState = PlanetsWidget::FollowNone;
}

void MainWindow::on_followPlainAveragePushButton_clicked(){
    ui->centralwidget->followState = PlanetsWidget::PlainAverage;
}

void MainWindow::on_followWeightedAveragePushButton_clicked(){
    ui->centralwidget->followState = PlanetsWidget::WeightedAverage;
}

void MainWindow::on_actionAbout_triggered(){
    QMessageBox::about(this, tr("About Planets3D"),
                       tr("<html><head/><body>"
                          "<h1>Planets3D %1</h1>"
                          "<p>Planets3D is a simple 3D gravitational simulator</p>"
                          "<p>Website: <a href=\"https://github.com/chipgw/planets-3d\">github.com/chipgw/planets-3d</a></p>"
                          "<p>Build Info:</p><ul>"
                          "<li>Git sha1: %2</li>"
                          "<li>Build type: %3</li>"
                          "</ul></body></html>").arg(version::getVersionString()).arg(version::git_revision).arg(version::build_type));
}

void MainWindow::on_stepsPerFrameSpinBox_valueChanged(int value){
    ui->centralwidget->universe.stepsPerFrame = value;
}

void MainWindow::on_trailLengthSpinBox_valueChanged(int value){
    Planet::pathLength = value;
}

void MainWindow::on_firingVelocityDoubleSpinBox_valueChanged(double value){
    ui->centralwidget->firingSpeed = value * velocityfac;
}

void MainWindow::on_firingMassSpinBox_valueChanged(int value){
    ui->centralwidget->firingMass = value;
}

void MainWindow::on_actionGrid_toggled(bool value){
    ui->centralwidget->drawGrid = value;
}

void MainWindow::on_actionDraw_Paths_toggled(bool value){
    ui->centralwidget->drawPlanetTrails = value;
}

void MainWindow::on_actionPlanet_Colors_toggled(bool value){
    ui->centralwidget->drawPlanetColors = value;
}
