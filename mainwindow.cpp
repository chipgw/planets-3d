#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::on_actionExit_triggered(){
    this->close();
}

void MainWindow::on_createPlanet_PushButton_clicked(){
    ui->centralwidget->selected = ui->centralwidget->createPlanet(glm::vec3(ui->newPosX_SpinBox->value(),ui->newPosY_SpinBox->value(),ui->newPosZ_SpinBox->value()),
                                                                  glm::vec3(ui->newVelocityX_SpinBox->value(),ui->newVelocityY_SpinBox->value(),ui->newVelocityZ_SpinBox->value()),
                                                                  ui->newMass_SpinBox->value());
}

void MainWindow::on_speed_Dial_valueChanged(int value){
    ui->centralwidget->simspeed = (value*5.0)/ui->speed_Dial->maximum();
    ui->speedDisplay_lcdNumber->display(ui->centralwidget->simspeed);
    if(ui->centralwidget->simspeed <= 0){
        ui->PauseResume_Button->setText(tr("Resume"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/icons/silk/control_play_blue.png"));
    }
    else{
        ui->PauseResume_Button->setText(tr("Pause"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/icons/silk/control_pause_blue.png"));
    }
}

void MainWindow::on_PauseResume_Button_clicked(){
    if(ui->centralwidget->simspeed <= 0){
        ui->centralwidget->simspeed = 1;
        ui->PauseResume_Button->setText(tr("Pause"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/icons/silk/control_pause_blue.png"));
    }
    else{
        ui->centralwidget->simspeed = 0;
        ui->PauseResume_Button->setText(tr("Resume"));
        ui->PauseResume_Button->setIcon(QIcon(":/icons/silk/icons/silk/control_play_blue.png"));
    }
    ui->speedDisplay_lcdNumber->display(ui->centralwidget->simspeed);
    ui->speed_Dial->setValue((ui->centralwidget->simspeed/5)*ui->speed_Dial->maximum());
}

void MainWindow::on_actionTake_Screenshot_triggered(){
    ui->centralwidget->doScreenshot = true;
}

void MainWindow::on_actionDelete_triggered(){
    if(ui->centralwidget->selected){
        ui->centralwidget->planets.removeAll(ui->centralwidget->selected);
        delete ui->centralwidget->selected;
    }
}

void MainWindow::on_actionClear_Velocity_triggered(){
    if(ui->centralwidget->selected){
        ui->centralwidget->selected->velocity = glm::vec3(0,0,0);
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

void MainWindow::on_actionCenter_All_triggered(){
    if(ui->centralwidget){
        ui->centralwidget->centerAll();
    }
}

void MainWindow::on_actionInteractive_Planet_Placement_triggered(){
    if(ui->centralwidget){
        ui->centralwidget->beginInteractiveCreation();
    }
}

void MainWindow::on_actionOff_triggered(){
    ui->centralwidget->gridMode = PlanetsWidget::Off;
}

void MainWindow::on_actionLines_triggered(){
    ui->centralwidget->gridMode = PlanetsWidget::SolidLine;
}

void MainWindow::on_actionPoints_triggered(){
    ui->centralwidget->gridMode = PlanetsWidget::Points;
}

void MainWindow::on_actionDraw_Paths_toggled(bool val){
    ui->centralwidget->drawPaths = val;

    QMutableListIterator<Planet*> i(ui->centralwidget->planets);
    Planet* planet;
    while (i.hasNext()) {
        planet = i.next();
        planet->path.clear();
    }
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
