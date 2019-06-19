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

#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

	CoreClass = new Core (0x800, 0x200, this);
	asDial = new AutosaveDialog (this);
	ISetDial = new InputSetDialog (CoreClass, this);
	ISetDial->setModal(true);
	sigIntDial = new InterpolatingDialog (CoreClass, this);
	filtDial = new FilteringDialog (CoreClass, this);
	pulProcDial = new ProcessingDialog (CoreClass, this);
	pulProcDial->setModal(true);
	strManDial = new StreamsManagerDialog (CoreClass, this);
	strManDial->setModal(true);
	nnSetDial = new NeuralNetSettingsDialog (CoreClass, this);
	dbgMenuDial = new DebugMenu (CoreClass, this);
	updateTimerPerSecond = new QTimer (this);
	updateTimerPerSecond->start(1000);

	fileMenu = menuBar()->addMenu("&File");
	fileMenu->addAction(tr("Open"), this, SLOT(file_open_slot()), tr("Ctrl+O"));
	fileMenu->addAction(tr("Save"), this, SLOT(file_save_slot()), tr("Ctrl+S"));
	fileMenu->addAction(tr("Save As"), this, SLOT(file_save_as_slot()), tr("Ctrl+Shift+S"));
	fileMenu->addAction(tr("Autosave"), this, SLOT(file_autosave_slot()));
	fileMenu->addAction(tr("Export"), this, SLOT(file_export_slot()));
	settingsMenu = menuBar()->addMenu("Se&ttings");
	settingsMenu->addAction("Input device", this, SLOT(show_set_id_dialog()));
	settingsMenu->addAction("FIR/IIR Filters", this, SLOT(show_set_sf_dialog()));
	settingsMenu->addAction("Interpolating", this, SLOT(show_set_si_dialog()));
	settingsMenu->addAction("Pulse processing", this, SLOT(show_set_pp_dialog()));
	settingsMenu->addAction("Streams manager", this, SLOT(show_str_man_dialog()));
	settingsMenu->addAction("Neural network", this, SLOT(show_neu_net_dialog()));
	helpMenu = menuBar()->addMenu("&Help");
	helpMenu->addAction("Debug menu", this, SLOT(show_dbg_dialog()));

	toolbarOpenButton = new QPushButton(style.standardIcon(QStyle::SP_DialogOpenButton), "Open", this);
	toolbarSaveButton = new QPushButton(style.standardIcon(QStyle::SP_DialogSaveButton), "Save", this);
	toolbarStartButton = new QPushButton(style.standardIcon(QStyle::SP_MediaPlay), "&Start", this);
	toolbarResetButton = new QPushButton(style.standardIcon(QStyle::SP_DialogResetButton), "&Reset", this);

	fileToolBar = addToolBar("File");
	fileToolBar->addWidget(toolbarOpenButton);
	fileToolBar->addWidget(toolbarSaveButton);
	procToolBar = addToolBar("Procesing");
	procToolBar->addWidget(toolbarStartButton);
	procToolBar->addWidget(toolbarResetButton);
	connect(toolbarOpenButton, SIGNAL(clicked(bool)), this, SLOT(file_open_slot()));
	connect(toolbarSaveButton, SIGNAL(clicked(bool)), this, SLOT(file_save_slot()));
	connect(toolbarStartButton, SIGNAL(clicked(bool)), CoreClass, SLOT(toggle_state()));
	connect(toolbarResetButton, SIGNAL(clicked(bool)), this, SLOT(proc_reset_slot()));

	graphicsPlot = new QCustomPlot (this);
	setCentralWidget(graphicsPlot);
	graphicsPlot->legend->setVisible(true);

	rightDockWidget = new QDockWidget (tr("View Mode"), this);
	rightDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
	rightDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
	addDockWidget(Qt::RightDockWidgetArea, rightDockWidget);
	rightDockWidChild = new QWidget;
	rightDockWidget->setWidget(rightDockWidChild);
	rightDockGraphicsSelectCB = new QComboBox (rightDockWidChild);
	rightDockGraphicsSelectCB->addItem(tr("Spectrum"), QVariant((quint32)Spectrum));
	rightDockGraphicsSelectCB->addItem(tr("Signal"), QVariant((quint32)Signal));
	rightDockXLogLabel = new QLabel (tr("Use logarithm X axis"), rightDockWidChild);
	rightDockXLogCB = new QCheckBox (rightDockWidChild);
	rightDockYLogLabel = new QLabel (tr("Use logarithm Y axis"), rightDockWidChild);
	rightDockYLogCB = new QCheckBox (rightDockWidChild);
	rightDockInfoLayout = new QGridLayout;
	rightDockSmoothingLabel = new QLabel (tr("Smoothing") ,rightDockWidChild);
	rightDockSmoothingDSB = new QDoubleSpinBox (rightDockWidChild);
	rightDockGraphName = new QLabel (tr("Stream"), rightDockWidChild);
	rightDockGraphColor = new QLabel (tr("Color"), rightDockWidChild);
	rightDockGraphEnable = new QLabel (tr("Show"), rightDockWidChild);
	graphicsColorsVec.resize(2);
	graphicsColorsVec[0].push_back(Qt::blue);
	graphicsColorsVec[0].push_back(Qt::black);
	graphicsColorsVec[0].push_back(Qt::red);
	graphicsColorsVec[1].push_back(Qt::blue);
	graphicsColorsVec[1].push_back(Qt::black);
	graphicsColorsVec[1].push_back(Qt::red);
	rightDockSmoothingDSB->setMaximum(1.);
	rightDockSmoothingDSB->setMinimum(0.);
	rightDockSmoothingDSB->setDecimals(2);
	rightDockSmoothingDSB->setSingleStep(0.01);
	rightDockInfoLayout->addWidget(rightDockXLogLabel, 0, 0, 1, 4);
	rightDockInfoLayout->addWidget(rightDockXLogCB, 0, 4, 1, 1);
	rightDockInfoLayout->addWidget(rightDockYLogLabel, 1, 0, 1, 4);
	rightDockInfoLayout->addWidget(rightDockYLogCB, 1, 4, 1, 1);
	rightDockInfoLayout->addWidget(rightDockSmoothingLabel, 2, 0, 1, 2);
	rightDockInfoLayout->addWidget(rightDockSmoothingDSB, 2, 2, 1, 3);
	rightDockInfoLayout->addWidget(rightDockGraphName, 3, 0, 1, 2);
	rightDockInfoLayout->addWidget(rightDockGraphColor, 3, 2, 1, 2);
	rightDockInfoLayout->addWidget(rightDockGraphEnable, 3, 4, 1, 1);
	rightDockInfoLayout->setRowStretch(5, 1);
	rightDockLayout = new QVBoxLayout (rightDockWidChild);
	rightDockLayout->addWidget(rightDockGraphicsSelectCB, 0, Qt::AlignTop);
	rightDockLayout->addLayout(rightDockInfoLayout);
	filenameLabel = new QLabel (this);
	graphic_view_update_slot();
	update_streams();
	updateGraphicsTimer = new QTimer (this);
	updateGraphicsTimer->start(200);
	graphicsPlot->setNoAntialiasingOnDrag(true);
	connect (updateGraphicsTimer, SIGNAL(timeout()), this, SLOT(graphics_update_slot()));
	connect (rightDockGraphicsSelectCB, SIGNAL(currentIndexChanged(int)), this, SLOT(graphic_view_update_slot()));
	connect (strManDial, SIGNAL(accepted()), this, SLOT(graphic_view_update_slot()));
	connect (ISetDial, SIGNAL(accepted()), this, SLOT(graphic_view_update_slot()));
	connect (rightDockXLogCB, SIGNAL(stateChanged(int)), this, SLOT(graphic_view_update_slot()));
	connect (rightDockYLogCB, SIGNAL(stateChanged(int)), this, SLOT(graphic_view_update_slot()));

	connect (CoreClass, SIGNAL (state_changed(bool)), this, SLOT(state_changed(bool)));
	connect (CoreClass, SIGNAL (state_changed(bool)), pulProcDial, SLOT(state_changed(bool)));
	connect (CoreClass, SIGNAL(buff_size_changed()), sigIntDial, SLOT(accept()));
	connect (CoreClass, SIGNAL(finished()), pulProcDial, SLOT(processing_finished()));
	connect (updateTimerPerSecond, SIGNAL(timeout()), this, SLOT(main_timer_timeout()));
	connect (updateTimerPerSecond, SIGNAL(timeout()), CoreClass, SLOT(sec_timer_update()));
	connect (strManDial, SIGNAL(accepted()), this, SLOT(update_streams()));

	setMinimumWidth(640);
	setMinimumHeight(480);
}

void MainWindow::set_autosave(quint32 interval, const std::string& filename, bool enable) {
	lastASave = std::time(0);
	intervalASave = interval;
	filenameASave = filename;
	enableASave = enable;
}

void MainWindow::graphics_info_update(quint32 newCount) {
	while (rightDockGraphColorLabels.size() < newCount) {
		rightDockGraphColorLabels.push_back(new QLabel (rightDockWidChild));
		rightDockGraphColorPBs.push_back(new QPushButton (tr("Select") ,rightDockWidChild));
		rightDockGraphColorPBs[rightDockGraphColorPBs.size()-1]->setProperty("", QVariant((quint32)rightDockGraphColorPBs.size()-1));
		rightDockGraphEnableCBs.push_back(new QCheckBox (rightDockWidChild));
		rightDockGraphEnableCBs[rightDockGraphEnableCBs.size()-1]->setProperty("", QVariant((quint32)rightDockGraphEnableCBs.size()-1));
		rightDockInfoLayout->addWidget(rightDockGraphColorLabels[rightDockGraphColorPBs.size()-1], 3+rightDockGraphColorPBs.size(), 0, 1, 2);
		rightDockInfoLayout->addWidget(rightDockGraphColorPBs[rightDockGraphColorPBs.size()-1], 3+rightDockGraphColorPBs.size(), 2, 1, 2);
		rightDockInfoLayout->addWidget(rightDockGraphEnableCBs[rightDockGraphEnableCBs.size()-1], 3+rightDockGraphColorPBs.size(), 4, 1, 1);
		connect (rightDockGraphColorPBs[rightDockGraphColorPBs.size()-1], SIGNAL(clicked(bool)), this, SLOT(graphic_color_change()));
		connect (rightDockGraphEnableCBs[rightDockGraphEnableCBs.size()-1], SIGNAL(stateChanged(int)), this, SLOT(graphic_enable_slot()));
	}
	rightDockInfoLayout->setRowStretch(4+newCount, 1);
	while (rightDockGraphColorLabels.size() > newCount) {
		delete rightDockGraphColorLabels[rightDockGraphColorLabels.size()-1];
		delete rightDockGraphColorPBs[rightDockGraphColorPBs.size()-1];
		delete rightDockGraphEnableCBs[rightDockGraphEnableCBs.size()-1];
		rightDockGraphColorLabels.pop_back();
		rightDockGraphColorPBs.pop_back();
		rightDockGraphEnableCBs.pop_back();
	}
	for (quint32 i = 0; i < newCount; i++)
		rightDockGraphEnableCBs[i]->setCheckState(Qt::Checked);
}

void MainWindow::file_open_slot() {
	QString loadFileName = QFileDialog::getOpenFileName(this,
							QString::fromUtf8("Open file"),
							QDir::currentPath(),
							"Experiment data files (*.ned);;All files (*)");
	if (loadFileName.isEmpty()) return;
	else {
		std::ifstream istr (loadFileName.toStdString());
		std::stringstream tmp;
		CoreClass->save_settings(tmp);
		try {
			CoreClass->load_settings(istr);
			update_streams();
			graphic_view_update_slot();
			graphics_update_slot(false);
		}
		catch (std::exception& ba) {
			QErrorMessage msg;
			msg.setModal(true);
			msg.showMessage("Invalid experiment data file");
			msg.exec();
			CoreClass->load_settings(tmp);
			update_streams();
			graphic_view_update_slot();
			graphics_update_slot(false);
			return;
		}
	}
	currentFile = loadFileName;
	filenameLabel->setText(currentFile);
}

void MainWindow::file_save_slot() {
	if (currentFile.isEmpty()) file_save_as_slot();
	else {
		std::ofstream ostr (currentFile.toStdString());
		CoreClass->save_settings(ostr);
	}
}

void MainWindow::file_save_as_slot() {
	QString saveFileName = QFileDialog::getSaveFileName(this,
							QString::fromUtf8("Save file"),
							QDir::currentPath(),
							tr("Experiment data files (*.ned);;All files (*)"));
	if (saveFileName.isEmpty()) return;
	QFileInfo file(saveFileName);
	if (file.suffix().isEmpty()) saveFileName += ".ned";
	currentFile = saveFileName;
	file_save_slot();
	filenameLabel->setText(currentFile);
}

void MainWindow::file_autosave_slot() {
	asDial->show();
	asDial->raise();
	asDial->activateWindow();
}

void MainWindow::file_export_slot() {
	QString exportFileName = QFileDialog::getSaveFileName(this,
							QString::fromUtf8("Export data"),
							QDir::currentPath(),
							tr("Text files (*.txt);;All files (*)"));
	if (exportFileName.isEmpty()) return;
	QFileInfo file(exportFileName);
	if (file.suffix().isEmpty()) exportFileName += ".txt";
	std::ofstream ostr (exportFileName.toStdString());
	ostr << "Channels\t";
	for (quint32 i = 0, ie = CoreClass->get_process_threads(); i < ie; i++)
		ostr << CoreClass->get_process_name(i).toStdString() << "\t";
	ostr << "\n";
	for (quint32 l = 0, le = CoreClass->get_spectrum_size(); l < le; ++l) {
		ostr << l << "\t";
		for (quint32 i = 0, ie = CoreClass->get_process_threads(); i < ie; i++)
			ostr << (*CoreClass->get_spectrum(i))[l] << "\t";
		ostr << "\n";
	}
}

void MainWindow::proc_reset_slot() {
	QMessageBox msg;
	msg.setText("All colected data will be deleted.");
	msg.setInformativeText("Do you want to delete it?");
	msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	switch (msg.exec()) {
		case QMessageBox::Yes:
			for (quint32 n = 0; n < CoreClass->get_process_threads(); n++)
				CoreClass->reset_spectrum(n);
			break;
		case QMessageBox::No:
			break;
		default:
			break;
	}
	graphics_update_slot(false);
}

void MainWindow::main_timer_timeout() {
	quint32 l = 0;
	for (quint32 n = 0; n < CoreClass->get_process_threads(); n++) {
			std::stringstream tStrs;
			tStrs << " c/r: " << CoreClass->get_count_rate(n);
			sBStreamsLabels[l++]->setText(CoreClass->get_process_name(n) + QString::fromUtf8(tStrs.str().c_str()));
	}
	if (std::time(0) - lastASave >= intervalASave && enableASave) {
		time_t tmp = std::time(0);
		std::string str = filenameASave + " " + std::asctime(std::localtime(&tmp));
		str.pop_back();
		str += ".ned";
		std::ofstream ostr (str);
		CoreClass->save_settings(ostr);
		std::cout << "Autosave: " << str << std::endl;
		lastASave = tmp;
	}
}

void MainWindow::graphics_update_slot(bool check) {
	if (!CoreClass->is_working() && check) return;
	switch (rightDockGraphicsSelectCB->currentData().toUInt()) {
		case Spectrum: {
			QVector<double> energy (CoreClass->get_spectrum(0)->size());
			for (qint32 i = 0; i < energy.size(); ++i)
				energy[i] = i;
			graphicsPlot->xAxis->setRange(0, energy.size());
			QVector<double> spectrum (energy.size());
			double max1, max(0.);
			quint32 total = 0;
			for (quint32 n = 0; n < CoreClass->get_process_threads(); n++) {
				const std::vector<quint32> vec = *CoreClass->get_spectrum(n);
				for (qint32 l = 0; l < energy.size(); ++l)
					spectrum[l] = vec[l];
				graphicsPlot->graph(total)->setData(energy, spectrum);
				if (rightDockGraphEnableCBs[total]->checkState() == Qt::Checked) {
					max1 = *std::max_element (spectrum.begin(), spectrum.end());
					if (max < max1) max = max1;
				}
				total++;
			}
			graphicsPlot->yAxis->setRange(0, max);
			graphicsPlot->replot();
			break;
		} case Signal: {
			QVector<double> channel (CoreClass->get_output_data(0)->size());
			QVector<double> data (channel.size());
			for (qint32 i = 0; i < channel.size(); ++i) channel[i] = i;
			graphicsPlot->xAxis->setRange(0, channel.size());
			graphicsPlot->yAxis->setRange(-1., 1.);
			for (quint32 n = 0; n < CoreClass->get_input_streams(); n++) {
				std::vector<float> const* vec = CoreClass->get_output_data(n);
				for (qint32 i = 0; i < channel.size(); ++i)
					data[i] = (*vec)[i];
				graphicsPlot->graph(n)->setData(channel, data);
			}
			graphicsPlot->replot();
			break;
		} default:
			assert(false);
			break;
	}
}

void MainWindow::graphic_view_update_slot() {
	QPen pen;
	switch (rightDockGraphicsSelectCB->currentData().toUInt()) {
		case Spectrum: {
			graphicsPlot->clearGraphs();
			graphicsPlot->xAxis->setLabel(tr("Channels"));
			graphicsPlot->yAxis->setLabel(tr("Counts"));
			if (graphicsColorsVec.size() < CoreClass->get_process_threads()) {
				std::uniform_int_distribution<quint32> dist (0, 255);
				std::default_random_engine gen;
				gen.seed(std::time(0));
				while (graphicsColorsVec[0].size() < CoreClass->get_process_threads()) {
					QColor col;
					col.setBlue(dist(gen));
					col.setRed(dist(gen));
					col.setGreen(dist(gen));
					col.setAlpha(dist(gen));
					graphicsColorsVec[0].push_back(col);
				}
			}
			for (quint32 n = 0; n < CoreClass->get_process_threads(); n++) {
				graphicsPlot->addGraph();
				graphicsPlot->graph(n)->setName(CoreClass->get_process_name(n));
				pen.setColor(graphicsColorsVec[0][n]);
				graphicsPlot->graph(n)->setPen(pen);
			}
			graphics_info_update(CoreClass->get_process_threads());
			for (quint32 n = 0; n < CoreClass->get_process_threads(); n++) {
				rightDockGraphColorLabels[n]->setText(CoreClass->get_process_name(n));
			}
			rightDockXLogLabel->hide();
			rightDockXLogCB->hide();
			rightDockYLogLabel->show();
			rightDockYLogCB->show();
			rightDockSmoothingDSB->hide();
			rightDockSmoothingLabel->hide();
			graphicsPlot->xAxis->setScaleType(QCPAxis::stLinear);
			if (rightDockYLogCB->checkState() == Qt::Checked) {
				graphicsPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
			}
			else {
				graphicsPlot->yAxis->setScaleType(QCPAxis::stLinear);
			}
			break;
		} case Signal: {
			graphicsPlot->clearGraphs();
			graphicsPlot->xAxis->setLabel(tr("Channels"));
			graphicsPlot->yAxis->setLabel(tr("Value"));
			if (graphicsColorsVec[1].size() != CoreClass->get_input_streams()) {
				std::uniform_int_distribution<quint32> dist (0, 255);
				std::default_random_engine gen;
				gen.seed(std::time(0));
				while (graphicsColorsVec[1].size() < CoreClass->get_input_streams()) {
					QColor col;
					col.setBlue(dist(gen));
					col.setRed(dist(gen));
					col.setGreen(dist(gen));
					col.setAlpha(dist(gen));
					graphicsColorsVec[1].push_back(col);
				}
			}
			for (quint32 n = 0; n < CoreClass->get_input_streams(); n++) {
				graphicsPlot->addGraph();
				pen.setColor(graphicsColorsVec[1][n]);
				graphicsPlot->graph(n)->setPen(pen);
			}
			graphics_info_update(CoreClass->get_input_streams());
			for (quint32 n = 0; n < CoreClass->get_input_streams(); n++) {
				graphicsPlot->graph(n)->setName(CoreClass->get_input_name(n));
				rightDockGraphColorLabels[n]->setText(CoreClass->get_input_name(n));
			}
			rightDockXLogLabel->hide();
			rightDockXLogCB->hide();
			rightDockYLogLabel->hide();
			rightDockYLogCB->hide();
			rightDockSmoothingDSB->hide();
			rightDockSmoothingLabel->hide();
			graphicsPlot->xAxis->setScaleType(QCPAxis::stLinear);
			graphicsPlot->yAxis->setScaleType(QCPAxis::stLinear);
			//QSharedPointer<QCPAxisTickerFixed> a (new QCPAxisTickerFixed);
			//QSharedPointer<QCPAxisTickerFixed> b (new QCPAxisTickerFixed);
			//graphicsPlot->xAxis->setTicker(a);
			//graphicsPlot->yAxis->setTicker(b);
			break;
		} default:
			throw std::invalid_argument ("Error! Invalid graphic mode index.\n");
			break;
	}
	graphics_update_slot(false);
}

void MainWindow::graphic_color_change() {
	quint32 tmp = rightDockGraphicsSelectCB->currentIndex();
	QColor color = QColorDialog::getColor(graphicsColorsVec[tmp][sender()->property("").toUInt()]);
	if (color.isValid() ) {
		QPen pen;
		pen.setColor(color);
		graphicsPlot->graph(sender()->property("").toUInt())->setPen(pen);
		graphicsColorsVec[tmp][sender()->property("").toUInt()] = color;
	}
}

void MainWindow::graphic_enable_slot() {
	quint32 tmp = sender()->property("").toUInt();
	graphicsPlot->graph(tmp)->setVisible(rightDockGraphEnableCBs[tmp]->checkState() == Qt::Checked);
	graphicsPlot->replot();
}

void MainWindow::state_changed(bool _newstate) {
	if (_newstate) {
		toolbarStartButton->setText(tr("&Stop"));
		toolbarStartButton->setIcon(style.standardIcon(QStyle::SP_MediaStop));
	} else {
		toolbarStartButton->setText(tr("&Start"));
		toolbarStartButton->setIcon(style.standardIcon(QStyle::SP_MediaPlay));
	}
}

void MainWindow::update_streams() {
	graphic_view_update_slot();
	while (sBStreamsLabels.size() < CoreClass->get_process_threads()) {
		QLabel* tmp = new QLabel (this);
		statusBar()->addWidget(tmp);
		sBStreamsLabels.push_back (tmp);
	}
	while (sBStreamsLabels.size() > CoreClass->get_process_threads()) {
		delete sBStreamsLabels[sBStreamsLabels.size()-1];
		sBStreamsLabels.pop_back();
	}
	if (currentFile.size()) filenameLabel->setText(currentFile);
	else filenameLabel->setText("File not selected.");
	statusBar()->addWidget(filenameLabel);
	for (quint32 n = 0; n < CoreClass->get_process_threads(); n++) {
		std::stringstream tStrs;
		tStrs << " c/r: " << CoreClass->get_count_rate(n);
		sBStreamsLabels[n]->setText(CoreClass->get_process_name(n) + QString::fromUtf8(tStrs.str().c_str()));
	}
}

void MainWindow::exit_selected() {
	close();
}

void MainWindow::show_set_id_dialog() {
	CoreClass->stop();
	ISetDial->show();
	ISetDial->raise();
	ISetDial->activateWindow();
}

void MainWindow::show_set_sf_dialog() {
	CoreClass->stop();
	filtDial->show();
	filtDial->raise();
	filtDial->activateWindow();
}

void MainWindow::show_set_si_dialog() {
	CoreClass->stop();
	sigIntDial->show();
	sigIntDial->raise();
	sigIntDial->activateWindow();
}

void MainWindow::show_set_pp_dialog() {
	CoreClass->stop();
	pulProcDial->show();
	pulProcDial->raise();
	pulProcDial->activateWindow();
}

void MainWindow::show_str_man_dialog() {
	CoreClass->stop();
	strManDial->show();
	strManDial->raise();
	strManDial->activateWindow();
}

void MainWindow::show_neu_net_dialog() {
	CoreClass->stop();
	nnSetDial->show();
	nnSetDial->raise();
	nnSetDial->activateWindow();
}

void MainWindow::show_dbg_dialog() {
	dbgMenuDial->show();
	dbgMenuDial->raise();
	dbgMenuDial->activateWindow();
}
