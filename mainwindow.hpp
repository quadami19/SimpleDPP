/*

	Copyright (C) 2019 Gostev Roman

	This file is part of SimpleDPP.

	SimpleDPP is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	SimpleDPP is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with SimpleDPP.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "core.hpp"
#include "inputdevicesetdialog.hpp"
#include "filteringdialog.hpp"
#include "interpolatingdialog.hpp"
#include "processingdialog.hpp"
#include "streamsmanagerdialog.hpp"
#include "neuralnetsettingsdialog.hpp"
#include "debugmenu.hpp"
#include "autosavedialog.hpp"
#include "qcustomplot/qcustomplot.h"
#include <QMainWindow>
#include <QStatusBar>
#include <QTextStream>
#include <QMenu>
#include <QMenuBar>
#include <QDir>
#include <utility>
#include <ctime>

class RightGraphicInfoPanel;
class GraphsInfoPanel;

class MainWindow : public QMainWindow {

		Q_OBJECT

		AutosaveDialog* asDial;
		InputSetDialog* ISetDial;
		InterpolatingDialog * sigIntDial;
		FilteringDialog* filtDial;
		ProcessingDialog* pulProcDial;
		StreamsManagerDialog* strManDial;
		NeuralNetSettingsDialog* nnSetDial;
		DebugMenu* dbgMenuDial;
		std::vector<QLabel*> sBStreamsLabels;
		QLabel* filenameLabel = 0x0;
		QFileDialog* openDial;
		QFileDialog* saveDial;
		QFileDialog* exprDial;

		QMenu* fileMenu;
		QMenu* settingsMenu;
		QMenu* helpMenu;
		QToolBar* fileToolBar;
		QToolBar* procToolBar;
		QPushButton* toolbarOpenButton;
		QPushButton* toolbarSaveButton;
		QPushButton* toolbarStartButton;
		QPushButton* toolbarResetButton;
		QDockWidget* rightDockWidget;
		QWidget* rightDockWidChild;
		QComboBox* rightDockGraphicsSelectCB;
		QLabel* rightDockXLogLabel;
		QCheckBox* rightDockXLogCB;
		QLabel* rightDockYLogLabel;
		QCheckBox* rightDockYLogCB;
		QLabel* rightDockSmoothingLabel;
		QDoubleSpinBox* rightDockSmoothingDSB;
		QLabel* rightDockGraphName;
		QLabel* rightDockGraphColor;
		QLabel* rightDockGraphEnable;
		std::vector<QLabel*> rightDockGraphColorLabels;
		std::vector<QPushButton*> rightDockGraphColorPBs;
		std::vector<QCheckBox*> rightDockGraphEnableCBs;
		QGridLayout* rightDockInfoLayout;
		QVBoxLayout* rightDockLayout;
		QCustomPlot* graphicsPlot;
		std::vector<std::vector<QColor>> graphicsColorsVec;

		std::vector<QVector<double>> graphicFourierBuffer;
		quint32 updateIntervalMs = 100;
		QTimer* updateTimerPerSecond;
		QTimer* updateGraphicsTimer;
		QString currentFile;
		QCommonStyle style;
		Core* CoreClass;

		friend class AutosaveDialog;
		quint32 intervalASave = 60;
		std::string filenameASave = "Experiment";
		time_t lastASave;
		bool enableASave = false;


		enum GraphicView {
			Spectrum,
			Signal
		};

		void graphics_info_update(quint32 newCount);

	protected:

	public:

		explicit MainWindow(QWidget *parent = 0);
		~MainWindow() {}
		void set_autosave (quint32 inteval, const std::string& filename, bool enable);

	private slots:

		void file_open_slot();
		void file_save_slot();
		void file_save_as_slot();
		void file_autosave_slot();
		void file_export_slot();
		void proc_reset_slot();
		void exit_selected ();

		void show_set_id_dialog();
		void show_set_sf_dialog();
		void show_set_si_dialog();
		void show_set_pp_dialog();
		void show_str_man_dialog();
		void show_neu_net_dialog();

		void show_dbg_dialog();

		void main_timer_timeout();
		void graphics_update_slot(bool check = true);
		void graphic_view_update_slot();
		void graphic_color_change();
		void graphic_enable_slot();
		void state_changed(bool _newstate);
		void update_streams ();
};

#endif // MAINWINDOW_HPP
