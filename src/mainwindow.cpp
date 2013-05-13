#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    universe = &ui->centralwidget->universe;

    planetCountLabel = new QLabel(ui->statusbar);
    planetCountLabel->setFixedWidth(120);
    ui->statusbar->addPermanentWidget(planetCountLabel);
    fpsLabel = new QLabel(ui->statusbar);
    fpsLabel->setFixedWidth(120);
    ui->statusbar->addPermanentWidget(fpsLabel);
    averagefpsLabel = new QLabel(ui->statusbar);
    averagefpsLabel->setFixedWidth(160);
    ui->statusbar->addPermanentWidget(averagefpsLabel);

    connect(ui->actionCenter_All,                   SIGNAL(triggered()), universe, SLOT(centerAll()));
    connect(ui->actionInteractive_Planet_Placement, SIGNAL(triggered()), ui->centralwidget, SLOT(beginInteractiveCreation()));

    connect(ui->centralwidget, SIGNAL(updatePlanetCountStatusMessage(QString)), planetCountLabel, SLOT(setText(QString)));
    connect(ui->centralwidget, SIGNAL(updateFPSStatusMessage(QString)),         fpsLabel,         SLOT(setText(QString)));
    connect(ui->centralwidget, SIGNAL(updateAverageFPSStatusMessage(QString)),  averagefpsLabel,  SLOT(setText(QString)));
}

MainWindow::~MainWindow(){
    delete ui;
    delete averagefpsLabel;
    delete fpsLabel;
}

void MainWindow::on_actionExit_triggered(){
    float tmpsimspeed = universe->simspeed;
    universe->simspeed = 0.0f;

    QMessageBox areYouSureMsgbox(this);
    areYouSureMsgbox.setText(tr("Are you sure you wish to exit? (universe will not be saved...)"));
    areYouSureMsgbox.setWindowTitle(tr("Are You Sure?"));
    areYouSureMsgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    areYouSureMsgbox.setDefaultButton(QMessageBox::Yes);
    areYouSureMsgbox.setIcon(QMessageBox::Warning);

    int input = areYouSureMsgbox.exec();

    if(input == QMessageBox::Yes){
        this->close();
    }
    universe->simspeed = tmpsimspeed;
}

void MainWindow::on_createPlanet_PushButton_clicked(){
    universe->selected = universe->createPlanet(QVector3D(ui->newPosX_SpinBox->value(),      ui->newPosY_SpinBox->value(),      ui->newPosZ_SpinBox->value()),
                                                QVector3D(ui->newVelocityX_SpinBox->value(), ui->newVelocityY_SpinBox->value(), ui->newVelocityZ_SpinBox->value()) * velocityfac,
                                                ui->newMass_SpinBox->value());
}

void MainWindow::on_speed_Dial_valueChanged(int value){
    universe->simspeed = (value*  speeddialmax) / ui->speed_Dial->maximum();
    ui->speedDisplay_lcdNumber->display(universe->simspeed);
    if(universe->simspeed <= 0.0f){
        ui->PauseResume_Button->setText(tr("Resume"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/control_play_blue.png"));
    }
    else{
        ui->PauseResume_Button->setText(tr("Pause"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/control_pause_blue.png"));
    }
}

void MainWindow::on_PauseResume_Button_clicked(){
    if(universe->simspeed <= 0.0f){
        universe->simspeed = 1.0f;
        ui->PauseResume_Button->setText(tr("Pause"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/control_pause_blue.png"));
    }
    else{
        universe->simspeed = 0.0f;
        ui->PauseResume_Button->setText(tr("Resume"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/control_play_blue.png"));
    }
    ui->speedDisplay_lcdNumber->display(universe->simspeed);
    ui->speed_Dial->setValue((universe->simspeed / speeddialmax) * ui->speed_Dial->maximum());
}

void MainWindow::on_actionTake_Screenshot_triggered(){
    ui->centralwidget->doScreenshot = true;
}

void MainWindow::on_actionDelete_triggered(){
    if(universe->selected){
        universe->planets.remove(universe->selected);
    }
}

void MainWindow::on_actionClear_Velocity_triggered(){
    if(universe->planets.contains(universe->selected)){
        universe->planets[universe->selected].velocity = QVector3D();
    }
}

void MainWindow::on_actionNew_Simulation_triggered(){
    float tmpsimspeed = universe->simspeed;
    universe->simspeed = 0.0f;

    QMessageBox areYouSureMsgbox(this);
    areYouSureMsgbox.setText(tr("Are you sure you wish to destroy the universe? (i.e. delete all planets.)"));
    areYouSureMsgbox.setWindowTitle(tr("Are You Sure?"));
    areYouSureMsgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    areYouSureMsgbox.setDefaultButton(QMessageBox::Yes);
    areYouSureMsgbox.setIcon(QMessageBox::Warning);

    int input = areYouSureMsgbox.exec();

    if(input == QMessageBox::Yes){
       universe->deleteAll();
    }
    universe->simspeed = tmpsimspeed;
}

void MainWindow::on_actionOff_triggered(){
    ui->centralwidget->displaysettings &= ~PlanetsWidget::SolidLineGrid;
    ui->centralwidget->displaysettings &= ~PlanetsWidget::PointGrid;
    ui->actionLines->setChecked(ui->centralwidget->displaysettings & PlanetsWidget::SolidLineGrid);
    ui->actionPoints->setChecked(ui->centralwidget->displaysettings & PlanetsWidget::PointGrid);
}

void MainWindow::on_actionLines_triggered(){
    ui->centralwidget->displaysettings |= PlanetsWidget::SolidLineGrid;
    ui->centralwidget->displaysettings &= ~PlanetsWidget::PointGrid;
    ui->actionLines->setChecked(ui->centralwidget->displaysettings & PlanetsWidget::SolidLineGrid);
    ui->actionPoints->setChecked(ui->centralwidget->displaysettings & PlanetsWidget::PointGrid);
}

void MainWindow::on_actionPoints_triggered(){
    ui->centralwidget->displaysettings &= ~PlanetsWidget::SolidLineGrid;
    ui->centralwidget->displaysettings |= PlanetsWidget::PointGrid;
    ui->actionLines->setChecked(ui->centralwidget->displaysettings & PlanetsWidget::SolidLineGrid);
    ui->actionPoints->setChecked(ui->centralwidget->displaysettings & PlanetsWidget::PointGrid);
}

void MainWindow::on_actionOpen_Simulation_triggered(){
    float tmpsimspeed = universe->simspeed;
    universe->simspeed = 0;

    QString filename = QFileDialog::getOpenFileName(this,tr("Open Simulation"), "", tr("Simulation files (*.uni *.xml)"));
    universe->load(filename);

    universe->simspeed = tmpsimspeed;
}

void MainWindow::on_actionSave_Simulation_triggered(){
    float tmpsimspeed = universe->simspeed;
    universe->simspeed = 0;

    QString filename = QFileDialog::getSaveFileName(this,tr("Save Simulation"), "", tr("Simulation files (*.uni *.xml)"));
    universe->save(filename);

    universe->simspeed = tmpsimspeed;
}

void MainWindow::on_actionDraw_Paths_triggered(){
    ui->centralwidget->displaysettings ^= PlanetsWidget::PlanetTrails;

    ui->actionDraw_Paths->setChecked(ui->centralwidget->displaysettings & PlanetsWidget::PlanetTrails);
}

void MainWindow::on_actionMotion_Blur_triggered(){
    ui->centralwidget->displaysettings ^= PlanetsWidget::MotionBlur;

    ui->actionMotion_Blur->setChecked(ui->centralwidget->displaysettings & PlanetsWidget::MotionBlur);
}

void MainWindow::on_followPlanetPushButton_clicked(){
    ui->centralwidget->following = universe->selected;
    ui->centralwidget->followState = PlanetsWidget::Single;

}

void MainWindow::on_clearFollowPushButton_clicked(){
    ui->centralwidget->following = 0;
    ui->centralwidget->followState = PlanetsWidget::FollowNone;
}

void MainWindow::on_actionAbout_triggered(){
    QMessageBox::about(this, "About Planets3D",
                       tr("<html><head/><body>"
                       "<p>Planets3D is a 3D gravitational simulator, inspired by <a href=\"http://planets.homedns.org/\">Planets</a></p>"
                       "<p>Website: <a href=\"https://github.com/chipgw/planets-3d\">github.com/chipgw/planets-3d</a></p>"
                       "<p>Git sha1: %1</p>"
                       "<p>Build type: %2</p>"
                       "</body></html>").arg(version::git_revision).arg(version::build_type));
}

void MainWindow::on_actionAbout_Qt_triggered(){
    QMessageBox::aboutQt(this);
}

void MainWindow::on_followPlainAveragePushButton_clicked(){
    ui->centralwidget->followState = PlanetsWidget::PlainAverage;
}

void MainWindow::on_followWeightedAveragePushButton_clicked(){
    ui->centralwidget->followState = PlanetsWidget::WeightedAverage;
}

void MainWindow::on_stepsPerFrameSpinBox_valueChanged(int value){
    ui->centralwidget->stepsPerFrame = value;
}

void MainWindow::on_trailLengthSpinBox_valueChanged(int value){
    Planet::pathLength = value;
}

void MainWindow::on_gridRangeSpinBox_valueChanged(int value){
    ui->centralwidget->gridRange = (value - 1) / 2;
}
