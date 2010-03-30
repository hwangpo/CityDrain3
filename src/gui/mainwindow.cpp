#include "mainwindow.h"
#include <simulationregistry.h>
#include <simulation.h>
#include <noderegistry.h>
#include <mapbasedmodel.h>
#include <log.h>

#include "ui_mainwindow.h"
#include "ui_newsimulationdialog.h"
#include "simulationscene.h"
#include "newsimulationdialog.h"
#include "simulationthread.h"
#include "guilogsink.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDateTimeEdit>
#include <QLabel>
#include <QSpinBox>
#include <QDate>
#include <QDateTime>
#include <boost/foreach.hpp>
#include <boost/date_time.hpp>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow), scene(0), simulation_unsaved(false) {
	ui->setupUi(this);
	ui->graphicsView->setRenderHints(QPainter::Antialiasing);

	setupTimeControls();

	sceneChanged();

	log_updater = new GuiLogSink();
	Log::init(log_updater);
	ui->logWidget->connect(log_updater, SIGNAL(newLogLine(QString)), SLOT(appendPlainText(QString)), Qt::QueuedConnection);
}

MainWindow::~MainWindow() {
	if (scene)
		delete scene;
	delete ui;
	Log::shutDown();
}

void MainWindow::setupTimeControls() {
	QLabel *startLabel = new QLabel("start:", ui->mainToolBar);
	ui->mainToolBar->addWidget(startLabel);

	start = new QDateTimeEdit(ui->mainToolBar);
	start->setDisplayFormat("d.M.yy h:mm:ss");
	start->setCalendarPopup(true);
	ui->mainToolBar->addWidget(start);
	this->connect(start, SIGNAL(dateTimeChanged(QDateTime)), SLOT(start_stop_dateTimeChanged(QDateTime)));

	QLabel *stopLabel = new QLabel("stop:", ui->mainToolBar);
	ui->mainToolBar->addWidget(stopLabel);

	stop = new QDateTimeEdit(ui->mainToolBar);
	stop->setDisplayFormat("d.M.yy h:mm:ss");
	stop->setCalendarPopup(true);
	ui->mainToolBar->addWidget(stop);
	this->connect(stop, SIGNAL(dateTimeChanged(QDateTime)), SLOT(start_stop_dateTimeChanged(QDateTime)));

	QLabel *dtLabel = new QLabel("dt:", ui->mainToolBar);
	ui->mainToolBar->addWidget(dtLabel);

	dt = new QSpinBox(ui->mainToolBar);
	dt->setRange(0, INT_MAX-1);
	dt->setSingleStep(60);
	ui->mainToolBar->addWidget(dt);
	this->connect(dt, SIGNAL(valueChanged(int)), SLOT(dt_valueChanged(int)));

	apply_time_button = new QPushButton("apply");
	apply_time_button->setEnabled(false);
	ui->mainToolBar->addWidget(apply_time_button);
	this->connect(apply_time_button, SIGNAL(clicked()), SLOT(applyTime()));
}

void MainWindow::changeEvent(QEvent *e) {
	QMainWindow::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void MainWindow::on_actionAdd_Plugin_activated() {
	QString plugin = QFileDialog::getOpenFileName(this, "select plugin", ".", "*.so, *.dll");
	if (plugin == "")
		return;
	scene->addPlugin(plugin);
	pluginsAdded();
}

void MainWindow::keyPressEvent(QKeyEvent *e) {
	if (e->key() == Qt::Key_Plus) {
		zoomIn();
		return;
	}
	if (e->key() == Qt::Key_Minus) {
		zoomOut();
		return;
	}
	QMainWindow::keyPressEvent(e);
}

void MainWindow::zoomIn(int times) {
	ui->graphicsView->scale(times * 1.2, times * 1.2);
}

void MainWindow::zoomOut(int times) {
	ui->graphicsView->scale(1/(times * 1.2), 1/(times * 1.2));
}

void MainWindow::on_actionZoom_in_activated() {
	zoomIn(1);
}

void MainWindow::on_actionZoom_out_activated() {
	zoomOut(1);
}

void MainWindow::on_actionZoom_reset_activated() {
	ui->graphicsView->setTransform(QTransform());
}

void MainWindow::pluginsAdded() {
	ui->treeWidget->clear();
	QTreeWidgetItem *nodes = new QTreeWidgetItem(QStringList("nodes"));

	BOOST_FOREACH(std::string node_name, scene->getNodeRegistry()->getRegisteredNames()) {
		QString qnode_name = QString::fromStdString(node_name);
		if (qnode_name.startsWith("CycleNode"))
			continue;
		QTreeWidgetItem *item = new QTreeWidgetItem(nodes);
		item->setText(0, qnode_name);
	}
	ui->treeWidget->insertTopLevelItem(0, nodes);

	nodes->setExpanded(true);
}

void MainWindow::on_runButton_clicked() {
	current_thread = new SimulationThread(scene->getSimulation());
	QObject::connect(current_thread->handler, SIGNAL(progress(int)),
					 ui->simProgressBar, SLOT(setValue(int)), Qt::QueuedConnection);
	connect(current_thread, SIGNAL(finished()),
			SLOT(simulationThreadFinished()), Qt::QueuedConnection);
	connect(current_thread, SIGNAL(started()),
			SLOT(simulationThreadStarted()), Qt::QueuedConnection);
	current_thread->start();
}

void MainWindow::on_stopButton_clicked() {
	current_thread->getSimulation()->stop();
}

void MainWindow::simulationThreadFinished() {
	delete current_thread;
	ui->dockWidget->setEnabled(true);
	ui->menuBar->setEnabled(true);
	ui->mainToolBar->setEnabled(true);
	ui->runButton->setEnabled(true);
	ui->stopButton->setEnabled(false);
}

void MainWindow::simulationThreadStarted() {
	//this->setEnabled(false);
	ui->dockWidget->setEnabled(false);
	ui->menuBar->setEnabled(false);
	ui->mainToolBar->setEnabled(false);
	ui->runButton->setEnabled(false);
	ui->stopButton->setEnabled(true);
}

void MainWindow::on_actionNewSimulation_activated() {
	if (scene) {
		QMessageBox::warning(this, "New Simulation", "A simulation is already defined");
		return;
	}
	NewSimulationDialog ns(this);
	if (ns.exec()) {
		scene = ns.createSimulationScene();
		sceneChanged();
		ui->graphicsView->setScene(scene);
		ui->actionSave_Simulation->setEnabled(true);
		ui->actionSave_as->setEnabled(true);
		ui->runButton->setEnabled(true);
		pluginsAdded();
		ui->actionAdd_Plugin->setEnabled(true);
		ui->actionAdd_Python_Module->setEnabled(true);
	}
}

QDateTime pttoqt(const ptime &dt) {
	time_duration td = dt.time_of_day();
	QTime t(td.hours(), td.minutes(), td.seconds());
	gregorian::date dtd = dt.date();
	QDate d(dtd.year(), dtd.month(), dtd.day());
	return QDateTime(d, t);
}

void MainWindow::sceneChanged() {
	if (!scene) {
		start->setEnabled(false);
		stop->setEnabled(false);
		dt->setEnabled(false);
		return;
	}

	start->setEnabled(true);
	stop->setEnabled(true);
	dt->setEnabled(true);

	SimulationParameters sp = scene->getSimulation()->getSimulationParameters();
	start->setDateTime(pttoqt(sp.start));
	stop->setDateTime(pttoqt(sp.stop));
	dt->setValue(sp.dt);

	apply_time_button->setEnabled(false);
	this->connect(scene, SIGNAL(unsavedChanged(bool)), SLOT(simulationUnsavedChanged(bool)));
}

void MainWindow::on_actionSave_Simulation_activated() {
	if (scene->getModelFileName() == "") {
		on_actionSave_as_activated();
	}
	scene->save();
}

void MainWindow::on_actionAdd_Python_Module_activated() {
	QString plugin = QFileDialog::getOpenFileName(this,
												  "select python module",
												  "./data/scripts", "*.py");
	if (plugin == "")
		return;
	scene->addPythonModule(plugin);
	pluginsAdded();
}

void MainWindow::on_action_exit_activated() {
	this->close();
}

void MainWindow::on_action_open_activated() {
	if (scene) {
		//TODO show dialog
		return;
	}

	QString path = QFileDialog::getOpenFileName(this,
												"Open Model File",
												".",
												"XML Files (*.xml)");
	if (path == "")
		return;
	setWindowTitle(QString("CityDrain3 (%1)").arg(path));
	scene = new SimulationScene(path);
	sceneChanged();
	ui->graphicsView->setScene(scene);
	ui->actionSave_Simulation->setEnabled(true);
	ui->actionSave_as->setEnabled(true);
	ui->runButton->setEnabled(true);
	ui->actionAdd_Plugin->setEnabled(true);
	ui->actionAdd_Python_Module->setEnabled(true);
	pluginsAdded();
}

void MainWindow::start_stop_dateTimeChanged(const QDateTime &date) {
	if (start->dateTime() == date) {
		stop->setMinimumDateTime(start->dateTime().addSecs(+dt->value()));
	}
	if (stop->dateTime() == date) {
		start->setMaximumDateTime(stop->dateTime().addSecs(-dt->value()));
	}
	apply_time_button->setEnabled(true);
}

void MainWindow::dt_valueChanged(int value) {
	apply_time_button->setEnabled(true);
}

void MainWindow::applyTime() {
	SimulationParameters p = scene->getSimulation()->getSimulationParameters();
	p.start = qttopt(start->dateTime());
	p.stop = qttopt(stop->dateTime());
	p.dt = dt->value();

	scene->getModel()->deinitNodes();
	if (!scene->getModel()->initNodes(p)) {
		QMessageBox::critical(this, "failed to init nodes",
							  "some nodes aren't happy with"
							  "your new choice of simulation time.\n"
							  "See log for more informations.");
		return;
	}
	scene->getSimulation()->setSimulationParameters(p);
	apply_time_button->setEnabled(false);
}

void MainWindow::closeEvent(QCloseEvent *event) {
	if (!simulation_unsaved) {
		event->accept();
		return;
	}
	if (QMessageBox::question(this, "Quit",
							  "There are unsaved changes. Do you really wan't to Quit?",
							  QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
		event->accept();
	} else {
		event->ignore();
	}
}

void MainWindow::on_actionSave_as_activated() {
	QString fileName = QFileDialog::getSaveFileName(this, "Save Model File", ".", "XML Files (*.xml)");
	if (fileName == "")
		return;
	if (!fileName.endsWith(".xml", Qt::CaseInsensitive)) {
		fileName = fileName + ".xml";
	}
	scene->setModelFileName(fileName);
	scene->save();
	setWindowTitle(QString("CityDrain3 (%1)").arg(fileName));
}

void MainWindow::simulationUnsavedChanged(bool unsaved) {
	if (unsaved && !windowTitle().endsWith(" *")) {
		setWindowTitle(windowTitle() + " *");
	}
	if (!unsaved && windowTitle().endsWith(" *")) {
		setWindowTitle(windowTitle().left(windowTitle().length()-2));
	}
	simulation_unsaved = unsaved;
}
