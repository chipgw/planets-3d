#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->PauseResume_Button->setFocus();

    planetCountLabel = new QLabel(ui->statusbar);
    planetCountLabel->setFixedWidth(120);
    ui->statusbar->addPermanentWidget(planetCountLabel);
    fpsLabel = new QLabel(ui->statusbar);
    fpsLabel->setFixedWidth(120);
    ui->statusbar->addPermanentWidget(fpsLabel);
    averagefpsLabel = new QLabel(ui->statusbar);
    averagefpsLabel->setFixedWidth(160);
    ui->statusbar->addPermanentWidget(averagefpsLabel);

    connect(ui->actionCenter_All,                   SIGNAL(triggered()), &ui->centralwidget->universe,  SLOT(centerAll()));
    connect(ui->actionInteractive_Planet_Placement, SIGNAL(triggered()), ui->centralwidget,             SLOT(beginInteractiveCreation()));

    connect(ui->centralwidget, SIGNAL(updatePlanetCountStatusMessage(QString)), planetCountLabel, SLOT(setText(QString)));
    connect(ui->centralwidget, SIGNAL(updateFPSStatusMessage(QString)),         fpsLabel,         SLOT(setText(QString)));
    connect(ui->centralwidget, SIGNAL(updateAverageFPSStatusMessage(QString)),  averagefpsLabel,  SLOT(setText(QString)));

    connect(ui->actionExit, SIGNAL(triggered()),  this,  SLOT(close()));

#ifndef GL_ACCUM
    ui->actionMotion_Blur->setEnabled(false);
#endif
}

MainWindow::~MainWindow(){
    delete ui;
    delete planetCountLabel;
    delete averagefpsLabel;
    delete fpsLabel;
}

void MainWindow::closeEvent(QCloseEvent *e){
    float tmpsimspeed = ui->centralwidget->universe.simspeed;
    ui->centralwidget->universe.simspeed = 0.0f;

    if(QMessageBox::warning(this, tr("Are You Sure?"), tr("Are you sure you wish to exit? (universe will not be saved...)"),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes){
        e->accept();
    }else{
        ui->centralwidget->universe.simspeed = tmpsimspeed;
        e->ignore();
    }
}

void MainWindow::on_createPlanet_PushButton_clicked(){
    ui->centralwidget->universe.selected = ui->centralwidget->universe.addPlanet(
                Planet(QVector3D(ui->newPosX_SpinBox->value(),      ui->newPosY_SpinBox->value(),      ui->newPosZ_SpinBox->value()),
                       QVector3D(ui->newVelocityX_SpinBox->value(), ui->newVelocityY_SpinBox->value(), ui->newVelocityZ_SpinBox->value()) * velocityfac,
                       ui->newMass_SpinBox->value()));
}

void MainWindow::on_actionDelete_triggered(){
    ui->centralwidget->universe.planets.remove(ui->centralwidget->universe.selected);
}

void MainWindow::on_actionClear_Velocity_triggered(){
    if(ui->centralwidget->universe.planets.contains(ui->centralwidget->universe.selected)){
        ui->centralwidget->universe.planets[ui->centralwidget->universe.selected].velocity = QVector3D();
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

void MainWindow::on_actionTake_Screenshot_triggered(){
    ui->centralwidget->doScreenshot = true;
}

void MainWindow::on_actionGridOff_triggered(){
    ui->centralwidget->displaysettings &= ~PlanetsWidget::SolidLineGrid;
    ui->centralwidget->displaysettings &= ~PlanetsWidget::PointGrid;
    ui->actionGridLines->setChecked(false);
    ui->actionGridPoints->setChecked(false);
    ui->centralwidget->gridPoints.clear();
}

void MainWindow::on_actionGridLines_triggered(){
    ui->centralwidget->displaysettings |= PlanetsWidget::SolidLineGrid;
    ui->centralwidget->displaysettings &= ~PlanetsWidget::PointGrid;
    ui->actionGridLines->setChecked(true);
    ui->actionGridPoints->setChecked(false);
    ui->centralwidget->gridPoints.clear();
}

void MainWindow::on_actionGridPoints_triggered(){
    ui->centralwidget->displaysettings &= ~PlanetsWidget::SolidLineGrid;
    ui->centralwidget->displaysettings |= PlanetsWidget::PointGrid;
    ui->actionGridLines->setChecked(false);
    ui->actionGridPoints->setChecked(true);
    ui->centralwidget->gridPoints.clear();
}

void MainWindow::on_actionDraw_Paths_triggered(){
    ui->centralwidget->displaysettings ^= PlanetsWidget::PlanetTrails;

    ui->actionDraw_Paths->setChecked(ui->centralwidget->displaysettings & PlanetsWidget::PlanetTrails);
}

void MainWindow::on_actionMotion_Blur_triggered(){
    ui->centralwidget->displaysettings ^= PlanetsWidget::MotionBlur;

    ui->actionMotion_Blur->setChecked(ui->centralwidget->displaysettings & PlanetsWidget::MotionBlur);
}

void MainWindow::on_actionPlanet_Colors_triggered(){
    ui->centralwidget->displaysettings ^= PlanetsWidget::PlanetColors;

    ui->actionPlanet_Colors->setChecked(ui->centralwidget->displaysettings & PlanetsWidget::PlanetColors);
}

void MainWindow::on_actionNew_Simulation_triggered(){
    float tmpsimspeed = ui->centralwidget->universe.simspeed;
    ui->centralwidget->universe.simspeed = 0.0f;

    if(QMessageBox::warning(this, tr("Are You Sure?"), tr("Are you sure you wish to destroy the universe? (i.e. delete all planets.)"),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes){
       ui->centralwidget->universe.deleteAll();
    }
    ui->centralwidget->universe.simspeed = tmpsimspeed;
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

    QString filename = QFileDialog::getSaveFileName(this, tr("Save Simulation"), "", tr("Simulation files (*.xml)"));
    ui->centralwidget->universe.save(filename);

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

void MainWindow::on_actionAbout_Qt_triggered(){
    QMessageBox::aboutQt(this);
}

void MainWindow::on_stepsPerFrameSpinBox_valueChanged(int value){
    ui->centralwidget->universe.stepsPerFrame = value;
}

void MainWindow::on_trailLengthSpinBox_valueChanged(int value){
    Planet::pathLength = value;
}

void MainWindow::on_gridRangeSpinBox_valueChanged(int value){
    ui->centralwidget->gridRange = value;
}
