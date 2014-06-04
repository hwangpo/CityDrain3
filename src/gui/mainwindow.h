/**
 * CityDrain3 is an open source software for modelling and simulating integrated 
 * urban drainage systems.
 * 
 * Copyright (C) 2012 Gregor Burger
 * 
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software 
 * Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with 
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin 
 * Street, Fifth Floor, Boston, MA 02110-1301, USA.
 **/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QUndoStack>
class QTreeWidgetItem;

namespace Ui {
	class MainWindow;
	class TimeControls;
}

class SimulationScene;
class SimulationThread;
class QDateTimeEdit;
class QSpinBox;
class QDateTime;
class QPushButton;
class GuiLogSink;
class QStateMachine;

class MainWindow : public QMainWindow {
Q_OBJECT
Q_PROPERTY(bool unsaved_changes READ getUnsavedChanges WRITE setUnsavedChanges);
public:
	MainWindow(QWidget *parent = 0);
	virtual ~MainWindow();
	bool getUnsavedChanges() const { return unsaved_changes; }
	void setUnsavedChanges(bool unsaved) { unsaved_changes = unsaved; }
	Ui::MainWindow *ui;

protected:
	void changeEvent(QEvent *e);
	void closeEvent(QCloseEvent *);

	void setupTimeControls();
	bool unload();
private:
	SimulationScene *scene;
	SimulationThread *current_thread;
	bool unsaved_changes;
	Ui::TimeControls *time_controls;
	QWidget *tc_widget;
	GuiLogSink *log_updater;
	QStateMachine *state_machine;
	QUndoStack undo_stack;

	void setupStateMachine();
	void incVerticalDistance(double inc);
	void incHorizontalDistance(double inc);

public Q_SLOTS:
    void on_actionAdd_Plugin_triggered();
    void on_actionNewSimulation_triggered();
	void on_actionSave_Simulation_activated();
	void on_actionSave_as_activated();
	void on_runButton_clicked();
	void on_stopButton_clicked();
	void on_actionAdd_Python_Module_activated();
	void on_action_exit_activated();
	void on_action_open_activated();
	void on_actionClose_activated();
	void on_actionZoom_in_activated();
	void on_actionZoom_out_activated();
	void on_actionZoom_reset_activated();
	void on_actionExport_to_pdf_activated();
	void on_actionSelect_all_activated();

	//edit stuff
	void on_actionFind_node_activated();
	void on_actionPaste_activated();
	void on_actionCopy_activated();
	void on_action_delete_activated();
	void on_actionRedo_activated();
	void on_actionUndo_activated();
	void on_actionRename_activated();
	void on_actionIncrease_distance_activated();
	void on_actionDecrease_distance_activated();
	void on_actionAuto_layout_activated();

	//align stuff
	void on_actionHorizontal_center_align_activated();
	void on_actionAlign_vertically_center_activated();

	//help stuff
	void on_action_About_activated();

	void start_stop_dateTimeChanged(const QDateTime &date);
	void dt_valueChanged(int value);
	void applyTime();

	void checkThreadOk();

	void zoomIn(int times = 1);
	void zoomOut(int times = 1);
	void pluginsAdded();
	void simulationChanged(QUndoCommand *);
	void simulationSaved();
	void updateTimeControls();
};

#endif // MAINWINDOW_H
