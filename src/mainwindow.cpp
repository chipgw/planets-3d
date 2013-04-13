#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    simspeedLabel = new QLabel(ui->statusbar);
    simspeedLabel->setFixedWidth(160);
    ui->statusbar->addPermanentWidget(simspeedLabel);
    fpsLabel = new QLabel(ui->statusbar);
    fpsLabel->setFixedWidth(120);
    ui->statusbar->addPermanentWidget(fpsLabel);
    averagefpsLabel = new QLabel(ui->statusbar);
    averagefpsLabel->setFixedWidth(160);
    ui->statusbar->addPermanentWidget(averagefpsLabel);

    connect(ui->actionCenter_All,                   SIGNAL(triggered()), ui->centralwidget, SLOT(centerAll()));
    connect(ui->actionInteractive_Planet_Placement, SIGNAL(triggered()), ui->centralwidget, SLOT(beginInteractiveCreation()));

    connect(ui->centralwidget, SIGNAL(updateSimspeedStatusMessage(QString)),   simspeedLabel,   SLOT(setText(QString)));
    connect(ui->centralwidget, SIGNAL(updateFPSStatusMessage(QString)),        fpsLabel,        SLOT(setText(QString)));
    connect(ui->centralwidget, SIGNAL(updateAverageFPSStatusMessage(QString)), averagefpsLabel, SLOT(setText(QString)));
}

MainWindow::~MainWindow(){
    delete ui;
    delete simspeedLabel;
    delete averagefpsLabel;
    delete fpsLabel;
}

void MainWindow::on_actionExit_triggered(){
    // TODO - either make confirmation dialog or connect triggered() signal directly to close() slot.
    this->close();
}

void MainWindow::on_createPlanet_PushButton_clicked(){
    ui->centralwidget->selected = &ui->centralwidget->createPlanet(glm::vec3(ui->newPosX_SpinBox->value(),ui->newPosY_SpinBox->value(),ui->newPosZ_SpinBox->value()),
                                                                  glm::vec3(ui->newVelocityX_SpinBox->value(),ui->newVelocityY_SpinBox->value(),ui->newVelocityZ_SpinBox->value()) * velocityfac,
                                                                  ui->newMass_SpinBox->value());
}

void MainWindow::on_speed_Dial_valueChanged(int value){
    ui->centralwidget->simspeed = (value*speeddialmax)/ui->speed_Dial->maximum();
    ui->speedDisplay_lcdNumber->display(ui->centralwidget->simspeed);
    if(ui->centralwidget->simspeed <= 0){
        ui->PauseResume_Button->setText(tr("Resume"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/control_play_blue.png"));
    }
    else{
        ui->PauseResume_Button->setText(tr("Pause"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/control_pause_blue.png"));
    }
}

void MainWindow::on_PauseResume_Button_clicked(){
    if(ui->centralwidget->simspeed <= 0){
        ui->centralwidget->simspeed = 1;
        ui->PauseResume_Button->setText(tr("Pause"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/control_pause_blue.png"));
    }
    else{
        ui->centralwidget->simspeed = 0;
        ui->PauseResume_Button->setText(tr("Resume"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/control_play_blue.png"));
    }
    ui->speedDisplay_lcdNumber->display(ui->centralwidget->simspeed);
    ui->speed_Dial->setValue((ui->centralwidget->simspeed/speeddialmax)*ui->speed_Dial->maximum());
}

void MainWindow::on_actionTake_Screenshot_triggered(){
    ui->centralwidget->doScreenshot = true;
}

void MainWindow::on_actionDelete_triggered(){
    if(ui->centralwidget->selected){
        ui->centralwidget->planets.removeAll(*ui->centralwidget->selected);
    }
}

void MainWindow::on_actionClear_Velocity_triggered(){
    if(ui->centralwidget->selected){
        ui->centralwidget->selected->velocity = glm::vec3(0.0f);
    }
}

void MainWindow::on_actionNew_Simulation_triggered(){
    float tmpsimspeed = ui->centralwidget->simspeed;
    ui->centralwidget->simspeed = 0;

    QMessageBox areYouSureMsgbox(this);
    areYouSureMsgbox.setText(tr("Are you sure you wish to destroy the universe? (i.e. delete all planets.)"));
    areYouSureMsgbox.setWindowTitle(tr("Are You Sure?"));
    areYouSureMsgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    areYouSureMsgbox.setDefaultButton(QMessageBox::Yes);
    areYouSureMsgbox.setIcon(QMessageBox::Warning);

    int input = areYouSureMsgbox.exec();

    if(input == QMessageBox::Yes){
        ui->centralwidget->deleteAll();
    }
    ui->centralwidget->simspeed = tmpsimspeed;
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
    float tmpsimspeed = ui->centralwidget->simspeed;
    ui->centralwidget->simspeed = 0;

    QString filename = QFileDialog::getOpenFileName(this,tr("Open Simulation"), "", tr("Simulation files (*.uni *.xml)"));
    ui->centralwidget->load(filename);

    ui->centralwidget->simspeed = tmpsimspeed;
}

void MainWindow::on_actionSave_Simulation_triggered(){
    float tmpsimspeed = ui->centralwidget->simspeed;
    ui->centralwidget->simspeed = 0;

    QString filename = QFileDialog::getSaveFileName(this,tr("Save Simulation"), "", tr("Simulation files (*.uni *.xml)"));
    ui->centralwidget->save(filename);

    ui->centralwidget->simspeed = tmpsimspeed;
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
    ui->centralwidget->following = ui->centralwidget->selected;
    ui->centralwidget->followState = PlanetsWidget::Single;

}

void MainWindow::on_clearFollowPushButton_clicked(){
    ui->centralwidget->following = NULL;
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
