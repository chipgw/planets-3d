#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include <functional>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QMimeData>
#include <QUrl>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), speedDialMemory(0),
    settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName()) {
    /* Set up the UI from the .ui file. */
    ui->setupUi(this);
    ui->PauseResume_Button->setFocus();

    /*Set the limits defined in the universe. */
    ui->newMass_SpinBox->setMinimum(ui->centralwidget->universe.min_mass);
    ui->newMass_SpinBox->setMaximum(ui->centralwidget->universe.max_mass);
    ui->firingMassSpinBox->setMinimum(ui->centralwidget->universe.min_mass);
    ui->firingMassSpinBox->setMaximum(ui->centralwidget->universe.max_mass);

    connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
    connect(ui->actionAbout_Qt, &QAction::triggered, QApplication::instance(), &QApplication::aboutQt);

    connect(ui->actionInteractive_Planet_Placement,     &QAction::triggered,    ui->toggleFiringModePushButton, &QPushButton::setChecked);
    connect(ui->actionInteractive_Orbital_Placement,    &QAction::triggered,    ui->toggleFiringModePushButton, &QPushButton::setChecked);
    connect(ui->actionInteractive_Planet_Placement,     &QAction::triggered,    ui->centralwidget, &PlanetsWidget::beginInteractiveCreation);
    connect(ui->actionInteractive_Orbital_Placement,    &QAction::triggered,    ui->centralwidget, &PlanetsWidget::beginOrbitalCreation);
    connect(ui->toggleFiringModePushButton,             &QPushButton::toggled,  ui->centralwidget, &PlanetsWidget::enableFiringMode);
    connect(ui->actionTake_Screenshot,                  &QAction::triggered,    ui->centralwidget, &PlanetsWidget::takeScreenshot);
    connect(ui->actionPrevious_Planet,                  &QAction::triggered,    ui->centralwidget, &PlanetsWidget::followPrevious);
    connect(ui->actionNext_Planet,                      &QAction::triggered,    ui->centralwidget, &PlanetsWidget::followNext);
    connect(ui->actionFollow_Selection,                 &QAction::triggered,    ui->centralwidget, &PlanetsWidget::followSelection);
    connect(ui->actionClear_Follow,                     &QAction::triggered,    ui->centralwidget, &PlanetsWidget::clearFollow);
    connect(ui->actionPlain_Average,                    &QAction::triggered,    ui->centralwidget, &PlanetsWidget::followPlainAverage);
    connect(ui->actionWeighted_Average,                 &QAction::triggered,    ui->centralwidget, &PlanetsWidget::followWeightedAverage);

    connect(ui->actionDelete,           &QAction::triggered, std::bind(&PlanetsUniverse::deleteSelected,    &ui->centralwidget->universe));
    connect(ui->actionCenter_All,       &QAction::triggered, std::bind(&PlanetsUniverse::centerAll,         &ui->centralwidget->universe));
    connect(ui->actionDelete_Escapees,  &QAction::triggered, std::bind(&PlanetsUniverse::deleteEscapees,    &ui->centralwidget->universe));

    connect(ui->gridRangeSpinBox, SIGNAL(valueChanged(int)), ui->centralwidget, SLOT(setGridRange(int)));

    /* Set up the statusbar labels. */
    ui->statusbar->addPermanentWidget(planetCountLabel = new QLabel(ui->statusbar));
    ui->statusbar->addPermanentWidget(fpsLabel = new QLabel(ui->statusbar));
    ui->statusbar->addPermanentWidget(averagefpsLabel = new QLabel(ui->statusbar));
    fpsLabel->setFixedWidth(120);
    planetCountLabel->setFixedWidth(120);
    averagefpsLabel->setFixedWidth(160);

    /* Connect the statusbar labels to the correct signals. */
    connect(ui->centralwidget, &PlanetsWidget::updateFPSStatusMessage,          fpsLabel,           &QLabel::setText);
    connect(ui->centralwidget, &PlanetsWidget::updateAverageFPSStatusMessage,   averagefpsLabel,    &QLabel::setText);
    connect(ui->centralwidget, &PlanetsWidget::frameSwapped,                    this,               &MainWindow::frameUpdate);
    connect(ui->centralwidget, &PlanetsWidget::statusBarMessage,                ui->statusbar,      &QStatusBar::showMessage);

    /* Add the actions in the tools toolbar to the menubar. */
    ui->menubar->insertMenu(ui->menuHelp->menuAction(), createPopupMenu())->setText(tr("Tools"));

    ui->firingSettings_DockWidget->hide();
    ui->randomSettings_DockWidget->hide();

    /* Try loading file from command line arguments... */
    for (const QString& argument : QApplication::arguments()) {
        try {
            ui->centralwidget->universe.load(argument.toStdString());
            break;
        } catch (...) { /* We don't care if there are errors, just ignore them. */ }
    }

    /* Load the saved window geometry. */
    settings.beginGroup(categoryMainWindow);
    restoreGeometry(settings.value(settingGeometry).toByteArray());
    restoreState(settings.value(settingState).toByteArray());
    settings.endGroup();

    /* Start Graphics settings. */
    settings.beginGroup(categoryGraphics);
    ui->actionGrid->setChecked(settings.value(settingDrawGrid).toBool());
    ui->actionDraw_Paths->setChecked(settings.value(settingDrawPaths).toBool());

    /* These aren't auto-saved, so for the moment they must be manually added to the config file. */
    if (settings.contains(settingGridDimensions))
        ui->gridRangeSpinBox->setValue(settings.value(settingGridDimensions).toInt());

    if (settings.contains(settingTrailLength))
        ui->trailLengthSpinBox->setValue(settings.value(settingTrailLength).toInt());

    if (settings.contains(settingTrailDelta))
        ui->trailRecordDistanceDoubleSpinBox->setValue(settings.value(settingTrailDelta).toDouble());

    settings.endGroup();
    /* End Graphics settings. */

    if (settings.contains(settingStepsPerFrame))
        ui->stepsPerFrameSpinBox->setValue(settings.value(settingStepsPerFrame).toInt());

    setAcceptDrops(true);

    updateRecentFileActions();
}

MainWindow::~MainWindow() {
    /* Save the window geometry for loading next time. */
    settings.beginGroup(categoryMainWindow);
    settings.setValue(settingGeometry, saveGeometry());
    settings.setValue(settingState, saveState());
    settings.endGroup();

    /* The auto-saved graphics settings. */
    settings.beginGroup(categoryGraphics);
    settings.setValue(settingDrawGrid, ui->actionGrid->isChecked());
    settings.setValue(settingDrawPaths, ui->actionDraw_Paths->isChecked());
    settings.endGroup();

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* e) {
    if (!ui->centralwidget->universe.isEmpty()) {
        int result = QMessageBox::warning(this, tr("Are You Sure?"), tr("Are you sure you wish to exit? (universe will not be saved...)"),
                                          QMessageBox::Yes | QMessageBox::Save | QMessageBox::No, QMessageBox::Yes);

        /* Ignore close event if user cancels or saving doesn't complete properly. */
        if (result == QMessageBox::No || (result == QMessageBox::Save && !on_actionSave_Simulation_triggered()))
            return e->ignore();
    }
    e->accept();
}

void MainWindow::on_createPlanet_PushButton_clicked() {
    ui->centralwidget->universe.selected = ui->centralwidget->universe.addPlanet(
                Planet(glm::vec3(ui->newPosX_SpinBox->value(),      ui->newPosY_SpinBox->value(),      ui->newPosZ_SpinBox->value()),
                       glm::vec3(ui->newVelocityX_SpinBox->value(), ui->newVelocityY_SpinBox->value(), ui->newVelocityZ_SpinBox->value()) * ui->centralwidget->universe.velocityfac,
                       ui->newMass_SpinBox->value()));
}

void MainWindow::on_actionClear_Velocity_triggered() {
    if (ui->centralwidget->universe.isSelectedValid())
        ui->centralwidget->universe.getSelected().velocity = glm::vec3();
}

void MainWindow::on_speed_Dial_valueChanged(int value) {
    ui->centralwidget->universe.simspeed = float(value * speeddialmax) / ui->speed_Dial->maximum();
    ui->speedDisplay_lcdNumber->display(ui->centralwidget->universe.simspeed);

    ui->PauseResume_Button->setText(value == 0 ? tr("Resume") : tr("Pause"));
    ui->PauseResume_Button->setIcon(QIcon(value == 0 ? ":/icons/silk/control_play_blue.png" : ":/icons/silk/control_pause_blue.png"));
}

void MainWindow::on_PauseResume_Button_clicked() {
    if (ui->speed_Dial->value() == 0) {
        ui->speed_Dial->setValue(speedDialMemory);
    } else {
        /* Store the curent value for resuming. */
        speedDialMemory = ui->speed_Dial->value();
        ui->speed_Dial->setValue(0);
    }
}

void MainWindow::on_FastForward_Button_clicked() {
    /* If it's at the max or zero, set to the value that gives a simspeed of 1. */
    if (ui->speed_Dial->value() == ui->speed_Dial->maximum() || ui->speed_Dial->value() == 0)
        ui->speed_Dial->setValue(ui->speed_Dial->maximum() / speeddialmax);
    /* Otherwise just double it, the max limit is handled by the widget. */
    else
        ui->speed_Dial->setValue(ui->speed_Dial->value() * 2);
}

void MainWindow::on_actionNew_Simulation_triggered() {
    if (!ui->centralwidget->universe.isEmpty() && QMessageBox::warning(this, tr("Are You Sure?"), tr("Are you sure you wish to destroy the universe? (i.e. delete all planets.)"),
                                                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
        ui->centralwidget->universe.deleteAll();
}

void MainWindow::on_actionOpen_Simulation_triggered() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Simulation"), "", tr("Simulation files (*.xml);;All Files (*.*)"));

    if (!filename.isEmpty()) {
        try {
            int loaded = ui->centralwidget->universe.load(filename.toStdString());
            ui->statusbar->showMessage(("Loaded %1 planets from \"" + filename + '"').arg(loaded), 8000);
            addRecentFile(filename);
        } catch (const std::exception& err) {
            QMessageBox::warning(this, tr("Error loading simulation!"), err.what());
        }
    }
}

void MainWindow::on_actionAppend_Simulation_triggered() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Append Simulation"), "", tr("Simulation files (*.xml);;All Files (*.*)"));

    if (!filename.isEmpty()) {
        try {
            int loaded = ui->centralwidget->universe.load(filename.toStdString(), false);
            ui->statusbar->showMessage(("Loaded %1 planets from \"" + filename + '"').arg(loaded), 8000);
            addRecentFile(filename);
        } catch (const std::exception& err) {
            QMessageBox::warning(this, tr("Error loading simulation!"), err.what());
        }
    }
}

bool MainWindow::on_actionSave_Simulation_triggered() {
    if (!ui->centralwidget->universe.isEmpty()) {
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Simulation"), "", tr("Simulation files (*.xml)"));

        if (!filename.isEmpty()) {
            try {
                ui->centralwidget->universe.save(filename.toStdString());
                ui->statusbar->showMessage("Simulation saved to \"" + filename + '"', 8000);
                addRecentFile(filename);
                return true;
            } catch (const std::exception& err) {
                QMessageBox::warning(this, tr("Error Saving Simulation."), err.what());
            }
        }
    } else {
        QMessageBox::warning(this, tr("Error Saving Simulation."), tr("No planets to save!"));
    }
    return false;
}

void MainWindow::on_actionAbout_triggered() {
    QMessageBox::about(this, tr("About Planets3D"),
                       tr("<html><head/><body>"
                          "<p>Planets3D is a simple 3D gravitational simulator</p>"
                          "<p>Website: <a href=\"https://github.com/chipgw/planets-3d\">github.com/chipgw/planets-3d</a></p>"
                          "<p>Build Info:</p><ul>"
                          "<li>Git sha1: %2</li>"
                          "<li>Build type: %3</li>"
                          "<li>Compiler: %4</li>"
                          "<li>CMake Version: %5</li>"
                          "</ul></body></html>")
                       .arg(version::git_revision)
                       .arg(version::build_type)
                       .arg(version::compiler)
                       .arg(version::cmake_version));
}

void MainWindow::on_stepsPerFrameSpinBox_valueChanged(int value) {
    ui->centralwidget->universe.stepsPerFrame = value;
}

void MainWindow::on_trailLengthSpinBox_valueChanged(int value) {
    ui->centralwidget->universe.pathLength = value;
}

void MainWindow::on_planetScaleDoubleSpinBox_valueChanged(double value) {
    ui->centralwidget->drawScale = value;
}

void MainWindow::on_trailRecordDistanceDoubleSpinBox_valueChanged(double value) {
    ui->centralwidget->universe.pathRecordDistance = value * value;
}

void MainWindow::on_firingVelocityDoubleSpinBox_valueChanged(double value) {
    ui->centralwidget->placing.firingSpeed = value * ui->centralwidget->universe.velocityfac;
}

void MainWindow::on_firingMassSpinBox_valueChanged(int value) {
    ui->centralwidget->placing.firingMass = value;
}

void MainWindow::on_actionGrid_toggled(bool value) {
    ui->centralwidget->grid.draw = value;
}

void MainWindow::on_actionDraw_Paths_toggled(bool value) {
    ui->centralwidget->drawPlanetTrails = value;
}

void MainWindow::on_actionPlanet_Colors_toggled(bool value) {
    ui->centralwidget->drawPlanetColors = value;
}

void MainWindow::on_actionHide_Planets_toggled(bool value) {
    ui->centralwidget->hidePlanets = value;

    /* Automatically enable drawing paths when hiding planets. */
    if (value)
        ui->actionDraw_Paths->setChecked(true);
}

void MainWindow::on_randomOrbitalCheckBox_toggled(bool checked) {
    /* All these boxes are unused for orbital mode. */
    ui->randomRangeDoubleSpinBox->setEnabled(!checked);
    ui->randomMassDoubleSpinBox->setEnabled(!checked);
    ui->randomSpeedDoubleSpinBox->setEnabled(!checked);
}

void MainWindow::on_generateRandomPushButton_clicked() {
    if (!ui->randomOrbitalCheckBox->isChecked())
        ui->centralwidget->universe.generateRandom(ui->randomAmountSpinBox->value(), ui->randomRangeDoubleSpinBox->value(),
                                                   ui->randomSpeedDoubleSpinBox->value() * ui->centralwidget->universe.velocityfac,
                                                   ui->randomMassDoubleSpinBox->value());
    else if (ui->centralwidget->universe.isEmpty())
        QMessageBox::warning(this, tr("Can't generate planets!"), tr("Nothing for new planets to orbit around!"));
    else
        ui->centralwidget->universe.generateRandomOrbital(ui->randomAmountSpinBox->value(), ui->centralwidget->universe.selected);
}

void MainWindow::on_actionClear_triggered() {
    settings.remove("Recent");
    updateRecentFileActions();
}

void MainWindow::addRecentFile(const QString& filename) {
    if (QFile::exists(filename)) {
        /* Get the existing list. */
        QStringList recent = settings.value("Recent").toStringList();

        /* Front = first on the list, which is where the newest entry goes. */
        recent.push_front(filename);

        /* No duplicates allowed. */
        recent.removeDuplicates();

        /* Save them and update the menu. */
        settings.setValue("Recent", recent);
        updateRecentFileActions();
    }
}

void MainWindow::updateRecentFileActions() {
    /* Remove any existing actions from the menu. */
    for (QAction* action : recentFileActions) {
        ui->menuRecent_Files->removeAction(action);
        delete action;
    }
    recentFileActions.clear();

    for (const QString& filename : settings.value("Recent").toStringList()) {
        QFileInfo file(filename);
        /* The action text itself is just the file name without the path. */
        QAction* action = new QAction(file.fileName(), this);

        /* Store the full path in the tooltip. */
        action->setToolTip(file.absoluteFilePath());

        /* Connect it to our generic slot for recent file actions. */
        connect(action, &QAction::triggered, this, &MainWindow::openRecentFile);

        /* Throw it into the list. */
        recentFileActions.append(action);
    }
    /* Put them before the first item in the menu, which would be the separator. */
    ui->menuRecent_Files->insertActions(ui->menuRecent_Files->actions()[0], recentFileActions);
}

void MainWindow::openRecentFile() {
    /* Getthe QAction that sent the signal. If it wasn't a QAction, just ignore it. */
    if (QAction* action = qobject_cast<QAction*>(sender())) {
        /* tooltip = full path to the file. */
        QString path = action->toolTip();

        try {
            int loaded = ui->centralwidget->universe.load(path.toStdString());

            ui->statusbar->showMessage(("Loaded %1 planets from \"" + path + '"').arg(loaded), 8000);

            /* Even though it is already in recent we add it, so it will be on top. */
            addRecentFile(path);
        } catch (const std::exception& err) {
            QMessageBox::warning(this, tr("Error loading simulation!"), err.what());
        }
    }
}


void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    for (const QUrl& url : event->mimeData()->urls())
        /* Only accept local files. */
        if (QFile::exists(url.toLocalFile()))
            /* TODO - perhaps this should filter to XML files... */
            return event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event) {
    for (const QUrl& url : event->mimeData()->urls()) {
        try {
            int loaded = ui->centralwidget->universe.load(url.toLocalFile().toStdString());

            ui->statusbar->showMessage(("Loaded %1 planets from \"" + url.toLocalFile() + '"').arg(loaded), 8000);

            addRecentFile(url.toLocalFile());
            return event->acceptProposedAction();
        } catch (const std::exception& err) {
            QMessageBox::warning(this, tr("Error loading simulation!"), err.what());
        }
    }
}

bool MainWindow::event(QEvent* event) {
    /* Pause/Resume when window looses/gains focus. */
    switch (event->type()) {
    case QEvent::WindowActivate:
        /* If paused, resume. */
        if (ui->centralwidget->universe.simspeed <= 0.0f)
            on_PauseResume_Button_clicked();
        break;
    case QEvent::WindowDeactivate:
        /* If running, pause. */
        if (ui->centralwidget->universe.simspeed > 0.0f)
            on_PauseResume_Button_clicked();
        break;
    default: break;
    }
    return QMainWindow::event(event);
}

void MainWindow::frameUpdate() {
    if (ui->centralwidget->universe.size() == 1)
        planetCountLabel->setText(tr("1 planet"));
    else
        planetCountLabel->setText(tr("%1 planets").arg(ui->centralwidget->universe.size()));

    if (int(ui->centralwidget->universe.simspeed * ui->speed_Dial->maximum() / speeddialmax) != ui->speed_Dial->value())
        ui->speed_Dial->setValue(int(ui->centralwidget->universe.simspeed * ui->speed_Dial->maximum() / speeddialmax));

    if (ui->centralwidget->universe.isSelectedValid()) {
        const Planet& selected = ui->centralwidget->universe.getSelected();

        glm::vec3 velocity = selected.velocity / ui->centralwidget->universe.velocityfac;

        ui->positionLabel->setText(QString("x: %0, y: %1, z: %2").arg(selected.position.x).arg(selected.position.y).arg(selected.position.z));
        ui->velocityLabel->setText(QString("x: %0, y: %1, z: %2").arg(velocity.x).arg(velocity.y).arg(velocity.z));
        ui->massLabel->setNum(selected.mass());
        ui->colorLabel->setText(QColor(QRgb(ui->centralwidget->universe.selected)).name());
    } else {
        ui->positionLabel->setText("");
        ui->velocityLabel->setText("");
        ui->massLabel->setText("");
        ui->colorLabel->setText("");
    }
}

const int MainWindow::speeddialmax = 64;

const QString MainWindow::categoryMainWindow =      "MainWindow";
const QString MainWindow::categoryGraphics =        "Graphics";
const QString MainWindow::settingGeometry =         "geometry";
const QString MainWindow::settingState =            "state";
const QString MainWindow::settingDrawGrid =         "DrawGrid";
const QString MainWindow::settingDrawPaths =        "DrawPaths";
const QString MainWindow::settingGridDimensions =   "GridDimensions";
const QString MainWindow::settingTrailLength =      "TrailLength";
const QString MainWindow::settingTrailDelta =       "TrailDelta";
const QString MainWindow::settingStepsPerFrame =    "StepsPerFrame";
