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

#include "debugmenu.hpp"
#include "processingsettings.hpp"

DebugMenu::DebugMenu(Core* ptr, QWidget* parent) : QDialog (parent) {
	availDataLabel = new QLabel (tr("Data:"), this);
	availDataCB = new QComboBox (this);
	headLayout = new QHBoxLayout;
	mainLayout = new QVBoxLayout (this);
	closePB = new QPushButton (tr("Close"), this);
	availDataCB->addItem(tr("Input buffer"), QVariant (InputBuffer));
	availDataCB->addItem(tr("Process buffer"), QVariant (ProcesBuffer));
	availDataCB->addItem(tr("Pulse"), QVariant (NeuralPulse));

	dbgInpData = new DebugInputWidget (ptr, this);
	dbgProcData = new DebugProcWidget (ptr, this);
	dbgPulData = new DebugPulseWidget (ptr, this);

	headLayout->addWidget(availDataLabel);
	headLayout->addWidget(availDataCB);
	mainLayout->addLayout(headLayout, 0);
	mainLayout->insertWidget(2, closePB);
	connect (availDataCB, SIGNAL(currentIndexChanged(int)), this, SLOT(replace_widgets()));
	connect (closePB, SIGNAL(clicked(bool)), this, SLOT(reject()));
}

void DebugMenu::showEvent(QShowEvent *) {
	replace_widgets();
}

void DebugMenu::replace_widgets() {
	switch (availDataCB->currentData().toUInt()) {
		case InputBuffer:
			dbgProcData->hide();
			dbgPulData->hide();
			dbgInpData->show();
			mainLayout->insertWidget(1, dbgInpData);
			break;
		case ProcesBuffer:
			dbgInpData->hide();
			dbgPulData->hide();
			dbgProcData->show();
			mainLayout->insertWidget(1, dbgProcData);
			break;
		case NeuralPulse:
			dbgInpData->hide();
			dbgProcData->hide();
			dbgPulData->show();
			mainLayout->insertWidget(1, dbgPulData);
			break;
		default:
			assert(false);
	}
}

DebugWidget::DebugWidget (Core *ptr, QWidget *parent) : QWidget (parent) {
	corePtr = ptr;
	plot = new QCustomPlot (this);
	dataSelect = new QComboBox (this);
	dataGet = new QPushButton (tr(("Get")), this);
	streamLabel = new QLabel (tr("Stream"), this);
	streamCB = new QComboBox (this);
	xLeftLabel = new QLabel (tr("X min"), this);
	xLeftDSB = new QDoubleSpinBox (this);
	xRightLabel = new QLabel (tr("X max"), this);
	xRightDSB = new QDoubleSpinBox (this);
	yDownLabel = new QLabel (tr("Y min"), this);
	yDownDSB = new QDoubleSpinBox (this);
	yUpLabel = new QLabel (tr("Y max"), this);
	yUpDSB = new QDoubleSpinBox (this);
	rescalePB = new QPushButton (tr("Rescale"), this);
	clearPB = new QPushButton (tr("Clear"), this);
	scaleLayout = new QGridLayout;
	customLayout = new QGridLayout;
	butnLayout = new QGridLayout;
	widgLayout = new QVBoxLayout;

	mainLayout = new QHBoxLayout (this);

	xLeftDSB->setSingleStep(1.);
	xLeftDSB->setDecimals(0);
	xRightDSB->setSingleStep(1.);
	xRightDSB->setDecimals(0);

	plot->setMinimumSize(480, 360);
	plot->addGraph();


	quint32 index = 0;
	scaleLayout->addWidget(dataSelect, index, 0);
	scaleLayout->addWidget(dataGet, index, 1);
	scaleLayout->addWidget(streamLabel, ++index, 0);
	scaleLayout->addWidget(streamCB, index, 1);
	scaleLayout->addWidget(xLeftLabel, ++index, 0);
	scaleLayout->addWidget(xLeftDSB, index, 1);
	scaleLayout->addWidget(xRightLabel, ++index, 0);
	scaleLayout->addWidget(xRightDSB, index, 1);
	scaleLayout->addWidget(yUpLabel, ++index, 0);
	scaleLayout->addWidget(yUpDSB, index, 1);
	scaleLayout->addWidget(yDownLabel, ++index, 0);
	scaleLayout->addWidget(yDownDSB, index, 1);
	butnLayout->addWidget(rescalePB, 0, 1, 1, 1, Qt::AlignTop);
	butnLayout->addWidget(clearPB, 1, 1, 1, 1, Qt::AlignTop);

	widgLayout->addLayout(scaleLayout);
	widgLayout->addLayout(customLayout);
	widgLayout->addLayout(butnLayout);

	mainLayout->addWidget(plot, 2);
	mainLayout->addLayout(widgLayout);

	connect (xLeftDSB, SIGNAL(valueChanged(double)), this, SLOT(scale_val_update()));
	connect (xRightDSB, SIGNAL(valueChanged(double)), this, SLOT(scale_val_update()));
	connect (yDownDSB, SIGNAL(valueChanged(double)), this, SLOT(scale_val_update()));
	connect (yUpDSB, SIGNAL(valueChanged(double)), this, SLOT(scale_val_update()));
	connect (dataGet, SIGNAL(clicked(bool)), this, SLOT(get_data()));
	connect (rescalePB, SIGNAL(clicked(bool)), this, SLOT(rescale()));
	connect (clearPB, SIGNAL(clicked(bool)), this, SLOT(clear_rescdsb()));
}

void DebugWidget::clear_rescdsb() {
	xLeftDSB->setMinimum(0.);
	xLeftDSB->setValue(0.);
	xRightDSB->setMaximum(dataX.size());
	xRightDSB->setValue(dataX.size());
	yUpDSB->setMaximum(1.);
	yUpDSB->setValue(1.);
	yDownDSB->setMinimum(-1.);
	yDownDSB->setValue(-1.);
}

void DebugWidget::scale_val_update() {
	xLeftDSB->setMaximum(xRightDSB->value());
	xRightDSB->setMinimum(xLeftDSB->value());
	yDownDSB->setMaximum(yUpDSB->value());
	yUpDSB->setMinimum(yDownDSB->value());
}

void DebugWidget::rescale() {
	plot->xAxis->setRange(xLeftDSB->value(), xRightDSB->value());
	plot->yAxis->setRange(yDownDSB->value(), yUpDSB->value());
	plot->replot();
}

void DebugWidget::showEvent(QShowEvent *) {
	update_streams();
	get_data();
	clear_rescdsb();
	rescale();
}

DebugInputWidget::DebugInputWidget(Core *ptr, QWidget* parent) : DebugWidget (ptr, parent) {
	dataSelect->addItem(tr("Raw"), QVariant (Raw));
	dataSelect->addItem(tr("Filtered"), QVariant (Filtered));
	dataSelect->addItem(tr("Interpolated"), QVariant (Interpolated));
}

void DebugInputWidget::update_streams() {
	streamCB->clear();
	for (quint32 i = 0, ie = corePtr->get_input_streams(); i < ie; i++) {
		streamCB->addItem(corePtr->get_input_name(i));
	}
}

void DebugInputWidget::get_data() {
    if (corePtr->get_input_streams() == 0) return;
	std::vector<float> const* tmp;
	switch (dataSelect->currentData().toUInt()) {
		case Raw:
			tmp = corePtr->get_raw_data(streamCB->currentIndex());
			break;
		case Filtered:
			tmp = corePtr->get_filter_data(streamCB->currentIndex());
			break;
		case Interpolated:
			tmp = corePtr->get_output_data(streamCB->currentIndex());
			break;
		default:
			assert(false);
	}
	dataY.resize(tmp->size());
	dataX.resize(tmp->size());
	for (quint32 i = 0, ie = dataX.size(); i < ie; i++) {
		dataX[i] = i;
		dataY[i] = (*tmp)[i];
	}
	clear_rescdsb();
	plot->graph(0)->setData(dataX, dataY);
	rescale();
}

DebugProcWidget::DebugProcWidget (Core *ptr, QWidget *parent) : DebugWidget (ptr, parent) {
	dataSelect->addItem(tr("Processed"), QVariant (Processed));
}

void DebugProcWidget::showEvent(QShowEvent * event) {
	DebugWidget::showEvent(event);
}

void DebugProcWidget::update_streams() {
	streamCB->clear();
	for (quint32 i = 0, ie = corePtr->get_process_threads(); i < ie; i++) {
		streamCB->addItem(corePtr->get_process_name(i));
	}
}

void DebugProcWidget::get_data() {
	std::vector<float> tmp;
	switch (dataSelect->currentData().toUInt()) {
		case Processed:
			tmp = corePtr->debug_get_proc(streamCB->currentIndex());
			break;
		default:
			assert(false);
	}
	dataY.resize(tmp.size());
	dataX.resize(tmp.size());
	for (quint32 i = 0, ie = dataX.size(); i < ie; i++) {
		dataX[i] = i;
		dataY[i] = tmp[i];
	}
	plot->graph(0)->setData(dataX, dataY);
	rescale();
}

DebugPulseWidget::DebugPulseWidget (Core *ptr, QWidget *parent) : DebugWidget (ptr, parent) {
	wNLabel = new QLabel (tr("White noize (n = 0) a ="), this);
	pNLabel = new QLabel (tr("Pink noize (n = -1) a, b ="), this);
	rNLabel = new QLabel (tr("Red noize (n = -2) a, b ="), this);
	bNLabel = new QLabel (tr("Blue noize (n = 1) a, b ="), this);
	vNLabel = new QLabel (tr("Violet noize (n = 2) a, b ="), this);
	wNDSBa = new QDoubleSpinBox (this);
	pNDSBa = new QDoubleSpinBox (this);
	rNDSBa = new QDoubleSpinBox (this);
	bNDSBa = new QDoubleSpinBox (this);
	vNDSBa = new QDoubleSpinBox (this);
	pNDSBb = new QDoubleSpinBox (this);
	rNDSBb = new QDoubleSpinBox (this);
	bNDSBb = new QDoubleSpinBox (this);
	vNDSBb = new QDoubleSpinBox (this);
	wNDSBa->setSingleStep(0.01);
	pNDSBa->setSingleStep(0.01);
	rNDSBa->setSingleStep(0.01);
	bNDSBa->setSingleStep(0.01);
	vNDSBa->setSingleStep(0.01);
	pNDSBb->setSingleStep(0.01);
	rNDSBb->setSingleStep(0.01);
	bNDSBb->setSingleStep(0.01);
	vNDSBb->setSingleStep(0.01);
	selTypeLabel = new QLabel (tr("Show for"), this);
	selTypeCB = new QComboBox (this);

	for (quint32 i = 0; i < AvailableFunctions::TotalAvailable; ++i)
		selTypeCB->addItem(AvailableFunctions::FunctionName[i], QVariant(i));

	quint32 index = 0;
	customLayout->addWidget(wNLabel, index, 0, 1, 2);
	customLayout->addWidget(wNDSBa, index, 2);
	customLayout->addWidget(pNLabel, ++index, 0, 1, 2);
	customLayout->addWidget(pNDSBa, index, 2);
	customLayout->addWidget(pNDSBb, index, 3);
	customLayout->addWidget(rNLabel, ++index, 0, 1, 2);
	customLayout->addWidget(rNDSBa, index, 2);
	customLayout->addWidget(rNDSBb, index, 3);
	customLayout->addWidget(bNLabel, ++index, 0, 1, 2);
	customLayout->addWidget(bNDSBa, index, 2);
	customLayout->addWidget(bNDSBb, index, 3);
	customLayout->addWidget(vNLabel, ++index, 0, 1, 2);
	customLayout->addWidget(vNDSBa, index, 2);
	customLayout->addWidget(vNDSBb, index, 3);
	customLayout->addWidget(selTypeLabel, ++index, 0, 1, 2);
	customLayout->addWidget(selTypeCB, index, 2, 1, 1, Qt::AlignLeft);

	dataSelect->addItem(tr("Teaching pulse"), NeuralNet);
}

void DebugPulseWidget::showEvent(QShowEvent *event) {
	DebugWidget::showEvent(event);
}

void DebugPulseWidget::clear_rescdsb() {
	xLeftDSB->setMinimum(0.);
	xLeftDSB->setValue(0.);
	xRightDSB->setMaximum(dataX.size());
	xRightDSB->setValue(dataX.size());
	yUpDSB->setMaximum(1.5);
	yUpDSB->setValue(1.5);
	yDownDSB->setMinimum(-0.5);
	yDownDSB->setValue(-0.5);
}

void DebugPulseWidget::update_streams() {
	streamCB->clear();
	for (quint32 i = 0, ie = corePtr->get_process_threads(); i < ie; i++) {
		streamCB->addItem(corePtr->get_process_name(i));
	}
}

void DebugPulseWidget::get_data() {

	Neural_Network::NuclTeachingClass* teach;
	switch (selTypeCB->currentData().toUInt()) {
		case AvailableFunctions::Amplitude:
			teach = &teachA;
			break;
		case AvailableFunctions::Time:
			teach = &teachT;
			break;
		case AvailableFunctions::Discriminating:
			teach = &teachD;
			break;
		default:
			assert(false);
	}

	teach->set_noize(0, wNDSBa->value(), 0.);
	teach->set_noize(-1, pNDSBa->value(), pNDSBb->value());
	teach->set_noize(-2, rNDSBa->value(), rNDSBb->value());
	teach->set_noize(1, bNDSBa->value(), bNDSBb->value());
	teach->set_noize(2, vNDSBa->value(), vNDSBb->value());
	std::vector<float> tmp = corePtr->get_process_settings(streamCB->currentIndex())->shape;
	if (tmp.size() == 0) return;
	teach->set_pulse(tmp);
	teach->init_noizes();
	tmp = teach->generate_pulse().first;

	dataY.resize(tmp.size());
	dataX.resize(tmp.size());
	for (quint32 i = 0, ie = dataX.size(); i < ie; i++) {
		dataX[i] = i;
		dataY[i] = tmp[i];
	}
	plot->graph(0)->setData(dataX, dataY);
	rescale();
}
