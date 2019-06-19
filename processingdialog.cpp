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

#include "processingdialog.hpp"
#include "processingsettings.hpp"

ProcessingDialog::ProcessingDialog (Core* ptr, QWidget* parent) : QDialog (parent) {
	corePtr = ptr;
	infoLabel = new QLabel(tr("Here one can select methods to detect and process pulses.\n"
							  ""), this);

	threadsL = new QLabel(tr("Selected thread"), this);
	threadsCB = new QComboBox (this);

	pulseSizeL = new QLabel(tr("Pulse size"), this);
	pulseSizeCB = new QComboBox (this);
	pulseSearchL = new QLabel(tr("Pulse search method"), this);
	pulseSearchCB = new QComboBox (this);
	pulseSearchPB = new QPushButton (tr("Settings..."), this);
	pulseDiscrL = new QLabel(tr("Discriminating method"), this);
	pulseDiscrCB = new QComboBox (this);
	pulseDiscrPB = new QPushButton (tr("Settings..."), this);
	pulseAmplL = new QLabel(tr("Amplitude measuring method"), this);
	pulseAmplCB = new QComboBox (this);
	pulseAmplPB = new QPushButton (tr("Settings..."), this);
	pulseTimeL = new QLabel(tr("Time measuring method"), this);
	pulseTimeCB = new QComboBox (this);
	pulseTimePB = new QPushButton (tr("Settings..."), this);
	specL = new QLabel (tr("Circuit settings"), this);
	specPB = new QPushButton (tr("Settings..."), this);

	shapeSmoothL = new QLabel ("Shape smoothing", this);
	shapeSmoothSB = new QSpinBox (this);
	shapeSmoothSB->setMaximum(10);
	amplCorrectionL = new QLabel ("Amplitude correction", this);
	amplCorrectionDSB = new QDoubleSpinBox (this);
	amplCorrectionDSB->setSingleStep(0.001);
	amplCorrectionDSB->setDecimals(4);
	subtractL = new QLabel (tr("Subtract shape from detected pulse"), this);
	subtractCB = new QCheckBox(this);

	pulseSizeCB->addItem("8", QVariant(0x8));
	pulseSizeCB->addItem("16", QVariant(0x10));
	pulseSizeCB->addItem("32", QVariant(0x20));
	pulseSizeCB->addItem("64", QVariant(0x40));
	pulseSizeCB->addItem("128", QVariant(0x80));
	pulseSizeCB->addItem("256", QVariant(0x100));

	pulseSearchCB->addItem(tr("Dynamic threshold"), QVariant(PulseSearching::Threshold));
	pulseSearchCB->addItem(tr("Monotonous regions"), QVariant(PulseSearching::Monoton));
	pulseSearchCB->addItem(tr("Threshold by tan"), QVariant(PulseSearching::TanThreshold));
	pulseDiscrCB->addItem(tr("By dispersion"), QVariant(PulseDiscriminator::Dispersion));
	pulseDiscrCB->addItem(tr("By neural network"), QVariant(PulseDiscriminator::NeuralNet));
	pulseAmplCB->addItem(tr("Maximum value"), QVariant(PulseAmplMeasuring::MaxVal));
	pulseAmplCB->addItem(tr("Polynomial max value"), QVariant(PulseAmplMeasuring::Polynomial));
	pulseAmplCB->addItem(tr("Neural network"), QVariant(PulseAmplMeasuring::NeuralNet));
	pulseTimeCB->addItem(tr("Maximum value"), QVariant(PulseTimeMeasuring::MaxVal));
	pulseTimeCB->addItem(tr("Neural network"), QVariant(PulseTimeMeasuring::NeuralNet));

	amplNNsPtr = &corePtr->get_ampl_nn();
	timeNNsPtr = &corePtr->get_time_nn();
	discrNNsPtr = &corePtr->get_discr_nn();

	searchDialogs.push_back(new SearchThresholdDialog(this));
	searchDialogs.push_back(new SearchMonotonousDialog(this));
	searchDialogs.push_back(new SearchTanThresholdDialog(this));
	discrDialogs.push_back(new DiscriminatorDispersionDialog(this));
	discrDialogs.push_back(new DiscriminatorNeuralNetDialog(discrNNsPtr, this));
	amplDialogs.push_back(new AmplMeasMaxDialog (this));
	amplDialogs.push_back(new AmplMeasPolyMaxDialog (this));
	amplDialogs.push_back(new AmplMeasNeuralNetDialog (amplNNsPtr, this));
	timeDialogs.push_back(new TimeMeasMaxDialog (this));
	timeDialogs.push_back(new TimeMeasNeuralNetDialog (timeNNsPtr, this));
	specDialogs.push_back(new CircuitSettingsDialog (corePtr, this));
	specDialogs.push_back(new CoincidenceSettingsDialog (corePtr, this));

	for (auto& a: searchDialogs) {
		a->setModal(true);
		connect(a, SIGNAL(accepted()), this, SLOT(settings_accepted()));
	}
	for (auto& a: discrDialogs) {
		a->setModal(true);
		connect(a, SIGNAL(accepted()), this, SLOT(settings_accepted()));
	}
	for (auto& a: amplDialogs) {
		a->setModal(true);
		connect(a, SIGNAL(accepted()), this, SLOT(settings_accepted()));
	}
	for (auto& a: timeDialogs) {
		a->setModal(true);
		connect(a, SIGNAL(accepted()), this, SLOT(settings_accepted()));
	}


	thrdLayout = new QHBoxLayout;
	widgLayout = new QGridLayout;
	butnLayout = new QHBoxLayout;
	acrjLayout = new QHBoxLayout;
	mainLayout = new QVBoxLayout (this);

	startPB = new QPushButton (tr("Start"), this);
	capturePB = new QPushButton (tr("Capture"), this);
	applyPB = new QPushButton (tr("Apply"), this);
	resetPB = new QPushButton (tr("Reset"), this);

	acceptPB = new QPushButton (tr("Accept"), this);
	rejectPB = new QPushButton (tr("Reject"), this);

	shapeWid = new PulseShapeWidget(this);
	shapeWid->setMinimumHeight(240);
	shapeUpdTim = new QTimer(this);
	shapeUpdTim->start(40);

	thrdLayout->addWidget(threadsL);
	thrdLayout->addWidget(threadsCB);
	widgLayout->addWidget(infoLabel, 0, 0, 1, 3);
	widgLayout->addWidget(pulseSizeL, 1, 0, 1, 1);
	widgLayout->addWidget(pulseSizeCB, 1, 1, 1, 1);
	widgLayout->addWidget(pulseSearchL, 2, 0, 1, 1);
	widgLayout->addWidget(pulseSearchCB, 2, 1, 1, 1);
	widgLayout->addWidget(pulseSearchPB, 2, 2, 1, 1);
	widgLayout->addWidget(pulseDiscrL, 3, 0, 1, 1);
	widgLayout->addWidget(pulseDiscrCB, 3, 1, 1, 1);
	widgLayout->addWidget(pulseDiscrPB, 3, 2, 1, 1);
	widgLayout->addWidget(pulseAmplL, 4, 0, 1, 1);
	widgLayout->addWidget(pulseAmplCB, 4, 1, 1, 1);
	widgLayout->addWidget(pulseAmplPB, 4, 2, 1, 1);
	widgLayout->addWidget(pulseTimeL, 5, 0, 1, 1);
	widgLayout->addWidget(pulseTimeCB, 5, 1, 1, 1);
	widgLayout->addWidget(pulseTimePB, 5, 2, 1, 1);
	widgLayout->addWidget(specL, 6, 0, 1, 1);
	widgLayout->addWidget(specPB, 6, 2, 1, 1);
	widgLayout->addWidget(shapeSmoothL, 7, 0, 1, 2);
	widgLayout->addWidget(shapeSmoothSB, 7, 2, 1, 1);
	widgLayout->addWidget(amplCorrectionL, 8, 0, 1, 2);
	widgLayout->addWidget(amplCorrectionDSB, 8, 2, 1, 1);
	widgLayout->addWidget(subtractL, 9, 0, 1, 2);
	widgLayout->addWidget(subtractCB, 9, 2, 1, 1);
	butnLayout->addWidget(startPB);
	butnLayout->addWidget(capturePB);
	butnLayout->addWidget(applyPB);
	butnLayout->addWidget(resetPB);
	acrjLayout->addWidget(acceptPB, 1, Qt::AlignLeft);
	acrjLayout->addWidget(rejectPB, 1, Qt::AlignRight);

	mainLayout->addLayout(thrdLayout);
	mainLayout->addLayout(widgLayout);
	mainLayout->addLayout(butnLayout);
	mainLayout->addWidget(shapeWid);
	mainLayout->addLayout(acrjLayout);

	connect (pulseSearchPB, SIGNAL(clicked(bool)), this, SLOT(show_search()));
	connect (pulseDiscrPB, SIGNAL(clicked(bool)), this, SLOT(show_discr()));
	connect (pulseAmplPB, SIGNAL(clicked(bool)), this, SLOT(show_ampl()));
	connect (pulseTimePB, SIGNAL(clicked(bool)), this, SLOT(show_time()));
	connect (specPB, SIGNAL(clicked(bool)), this, SLOT(show_spec()));
	connect (startPB, SIGNAL(clicked(bool)), this, SLOT(start()));
	connect (capturePB, SIGNAL(clicked(bool)), this, SLOT(capture()));
	connect (applyPB, SIGNAL(clicked(bool)), this, SLOT(apply()));
	connect (resetPB, SIGNAL(clicked(bool)), this, SLOT(reset()));
	connect (acceptPB, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect (rejectPB, SIGNAL(clicked(bool)), this, SLOT(reject()));
	connect (shapeUpdTim, SIGNAL(timeout()), shapeWid, SLOT(draw_enable()));
	connect (corePtr, SIGNAL(finished()), this, SLOT(processing_finished()));
}

void ProcessingDialog::showEvent(QShowEvent *) {
	disconnect (threadsCB, SIGNAL(currentIndexChanged(int)), this, SLOT(thread_changed()));
	corePtr->set_setup_mode (true);
	threadsCB->clear();
	curSettings.clear();
	oldSettings.clear();
	for (quint32 i = 0, ie = corePtr->get_process_threads(); i < ie; ++i)
		threadsCB->addItem(corePtr->get_process_name(i));
	for (quint32 i = 0, ie = corePtr->get_process_threads(); i < ie; ++i) {
		switch (corePtr->get_process_settings(i)->get_settings_id()) {
			case ProcessingThread::StandartCircuit:
				curSettings.push_back(std::shared_ptr<ProcessingThread::Settings>
						(new ProcessingStandartCircuit::StanCircuitSettings(*corePtr->get_process_settings(i))));
				oldSettings.push_back(std::shared_ptr<ProcessingThread::Settings>
						(new ProcessingStandartCircuit::StanCircuitSettings(*corePtr->get_process_settings(i))));
				break;
			case ProcessingThread::CoincidenceCircuit:
				curSettings.push_back(std::shared_ptr<ProcessingThread::Settings>
						(new ProcessingCoincidenceCircuit::CoinCircuitSettings(*corePtr->get_process_settings(i))));
				oldSettings.push_back(std::shared_ptr<ProcessingThread::Settings>
						(new ProcessingCoincidenceCircuit::CoinCircuitSettings(*corePtr->get_process_settings(i))));
				break;
			default:
				assert(false);
		}
	}
	load_to_dial_settings(threadsCB->currentIndex());
	connect (threadsCB, SIGNAL(currentIndexChanged(int)), this, SLOT(thread_changed()));
	thread_changed();
}

void ProcessingDialog::load_to_dial_settings(int index) {
	pulseSearchCB->setCurrentIndex(pulseSearchCB->findData(curSettings[index]->sSet->get_s_settings_id()));
	pulseDiscrCB->setCurrentIndex(pulseDiscrCB->findData(curSettings[index]->dSet->get_d_settings_id()));
	pulseAmplCB->setCurrentIndex(pulseAmplCB->findData(curSettings[index]->aSet->get_a_settings_id()));
	pulseTimeCB->setCurrentIndex(pulseTimeCB->findData(curSettings[index]->tSet->get_t_settings_id()));

	searchDialogs[pulseSearchCB->currentData().toUInt()]->set_settings(curSettings[index]);
	discrDialogs[pulseDiscrCB->currentData().toUInt()]->set_settings(curSettings[index]);
	amplDialogs[pulseAmplCB->currentData().toUInt()]->set_settings(curSettings[index]);
	timeDialogs[pulseTimeCB->currentData().toUInt()]->set_settings(curSettings[index]);
	specDialogs[curSettings[index]->get_settings_id()]->set_settings(curSettings[index]);

}

void ProcessingDialog::update_sub_widgets() {
	if (curSettings[threadsCB->currentIndex()]->enableSub) subtractCB->setCheckState(Qt::Checked);
	else subtractCB->setCheckState(Qt::Unchecked);
	if (curSettings[threadsCB->currentIndex()]->shape.size() == curSettings[threadsCB->currentIndex()]->pulseSize) {
		subtractL->setEnabled(true);
		subtractCB->setEnabled(true);
	} else {
		subtractL->setEnabled(false);
		subtractCB->setEnabled(false);
	}
}

void ProcessingDialog::set_spec_dial() {
	specDialogs[curSettings[threadsCB->currentIndex()]->get_settings_id()]->set_current_thread(threadsCB->currentIndex());
	specPB->setProperty("", QVariant(curSettings[threadsCB->currentIndex()]->get_settings_id()));
}

bool ProcessingDialog::check_settings() {
	bool success = true;

	if (!searchDialogs[pulseSearchCB->currentData().toUInt()]->check_settings(curSettings[threadsCB->currentIndex()])) success = false;
	if (!discrDialogs[pulseDiscrCB->currentData().toUInt()]->check_settings(curSettings[threadsCB->currentIndex()])) success = false;
	if (!amplDialogs[pulseAmplCB->currentData().toUInt()]->check_settings(curSettings[threadsCB->currentIndex()])) success = false;
	if (!timeDialogs[pulseTimeCB->currentData().toUInt()]->check_settings(curSettings[threadsCB->currentIndex()])) success = false;
	if (!specDialogs[curSettings[threadsCB->currentIndex()]->get_settings_id()]->check_settings(curSettings[threadsCB->currentIndex()])) success = false;

	return success;
}

void ProcessingDialog::thread_changed() {
	disconnect (pulseSizeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(size_changed()));
	disconnect (pulseSearchCB, SIGNAL(currentIndexChanged(int)), this, SLOT(method_changed()));
	disconnect (pulseDiscrCB, SIGNAL(currentIndexChanged(int)), this, SLOT(method_changed()));
	disconnect (pulseAmplCB, SIGNAL(currentIndexChanged(int)), this, SLOT(method_changed()));
	disconnect (pulseTimeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(method_changed()));

	corePtr->set_setup_mode_process(threadsCB->currentIndex());
	pulseSizeCB->setCurrentIndex(pulseSizeCB->findData(QVariant(curSettings[threadsCB->currentIndex()]->pulseSize)));
	pulseSearchCB->setCurrentIndex(pulseSearchCB->findData(QVariant(curSettings[threadsCB->currentIndex()]->sSet->get_s_settings_id())));
	pulseDiscrCB->setCurrentIndex(pulseDiscrCB->findData(QVariant(curSettings[threadsCB->currentIndex()]->dSet->get_d_settings_id())));
	pulseAmplCB->setCurrentIndex(pulseAmplCB->findData(QVariant(curSettings[threadsCB->currentIndex()]->aSet->get_a_settings_id())));
	pulseTimeCB->setCurrentIndex(pulseTimeCB->findData(QVariant(curSettings[threadsCB->currentIndex()]->tSet->get_t_settings_id())));
	shapeWid->set_shape(curSettings[threadsCB->currentIndex()]->shape);
	amplCorrectionDSB->setValue(curSettings[threadsCB->currentIndex()]->amplCorrection);

	switch (curSettings[threadsCB->currentIndex()]->get_settings_id()) {
		case ProcessingThread::StandartCircuit:
			break;
		case ProcessingThread::CoincidenceCircuit:

			break;
		default:
			assert(false);
	}
	load_to_dial_settings(threadsCB->currentIndex());
	update_sub_widgets();
	set_spec_dial();

	connect (pulseSizeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(size_changed()));
	connect (pulseSearchCB, SIGNAL(currentIndexChanged(int)), this, SLOT(method_changed()));
	connect (pulseDiscrCB, SIGNAL(currentIndexChanged(int)), this, SLOT(method_changed()));
	connect (pulseAmplCB, SIGNAL(currentIndexChanged(int)), this, SLOT(method_changed()));
	connect (pulseTimeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(method_changed()));
}

void ProcessingDialog::size_changed() {
	curSettings[threadsCB->currentIndex()]->pulseSize = pulseSizeCB->currentData().toUInt();

	searchDialogs[pulseSearchCB->currentData().toUInt()]->update_settings();
	discrDialogs[pulseDiscrCB->currentData().toUInt()]->update_settings();
	amplDialogs[pulseAmplCB->currentData().toUInt()]->update_settings();
	timeDialogs[pulseTimeCB->currentData().toUInt()]->update_settings();
	specDialogs[curSettings[threadsCB->currentIndex()]->get_settings_id()]->update_settings();

	bool success = check_settings();
	applyPB->setEnabled(success);
	acceptPB->setEnabled(success);
	threadsCB->setEnabled(success);
}

void ProcessingDialog::method_changed() {
	quint32 tmp = pulseSearchCB->currentData().toUInt();
	if(tmp != curSettings[threadsCB->currentIndex()]->sSet->get_s_settings_id())
		curSettings[threadsCB->currentIndex()]->sSet = PulseSearching::SearchSettings::get_new(tmp);

	tmp = pulseDiscrCB->currentData().toUInt();
	if(tmp != curSettings[threadsCB->currentIndex()]->dSet->get_d_settings_id())
		curSettings[threadsCB->currentIndex()]->dSet = PulseDiscriminator::DiscSettings::get_new(tmp);

	tmp = pulseAmplCB->currentData().toUInt();
	if(tmp != curSettings[threadsCB->currentIndex()]->aSet->get_a_settings_id())
		curSettings[threadsCB->currentIndex()]->aSet = PulseAmplMeasuring::AmplSettings::get_new(tmp);

	tmp = pulseTimeCB->currentData().toUInt();
	if(tmp != curSettings[threadsCB->currentIndex()]->tSet->get_t_settings_id()) {
		curSettings[threadsCB->currentIndex()]->tSet = PulseTimeMeasuring::TimeSettings::get_new(tmp);
	}
	load_to_dial_settings(threadsCB->currentIndex());
	settings_accepted();
}

void ProcessingDialog::settings_accepted() {
	bool success = check_settings();
	applyPB->setEnabled(success);
	acceptPB->setEnabled(success);
	threadsCB->setEnabled(success);
}

void ProcessingDialog::show_search() {
	quint32 tmp = pulseSearchCB->currentData().toUInt();
	searchDialogs[tmp]->show();
	searchDialogs[tmp]->raise();
	searchDialogs[tmp]->activateWindow();
}

void ProcessingDialog::show_discr() {
	quint32 tmp = pulseDiscrCB->currentData().toUInt();
	discrDialogs[tmp]->show();
	discrDialogs[tmp]->raise();
	discrDialogs[tmp]->activateWindow();
}

void ProcessingDialog::show_ampl() {
	quint32 tmp = pulseAmplCB->currentData().toUInt();
	amplDialogs[tmp]->show();
	amplDialogs[tmp]->raise();
	amplDialogs[tmp]->activateWindow();
}

void ProcessingDialog::show_time() {
	quint32 tmp = pulseTimeCB->currentData().toUInt();
	timeDialogs[tmp]->show();
	timeDialogs[tmp]->raise();
	timeDialogs[tmp]->activateWindow();

}

void ProcessingDialog::show_spec() {
	quint32 tmp = corePtr->get_process_settings(threadsCB->currentIndex())->get_settings_id();
	specDialogs[tmp]->set_settings(curSettings[threadsCB->currentIndex()]);
	specDialogs[tmp]->show();
	specDialogs[tmp]->raise();
	specDialogs[tmp]->activateWindow();
}

void ProcessingDialog::start() {
	corePtr->toggle_state();
}

void ProcessingDialog::capture() {
	shapeWid->capture();
	std::shared_ptr<ProcessingThread::Settings> tmp = ProcessingStandartCircuit::StanCircuitSettings::get_copy(*corePtr->get_process_settings(threadsCB->currentIndex()));
	tmp->shape = shapeWid->get_shape();
	curSettings[threadsCB->currentIndex()]->shape = shapeWid->get_shape();
	corePtr->set_process_settings(tmp, threadsCB->currentIndex());
	update_sub_widgets();
}

void ProcessingDialog::apply() {
	curSettings[threadsCB->currentIndex()]->pulseSize = pulseSizeCB->currentData().toUInt();
	curSettings[threadsCB->currentIndex()]->amplCorrection = amplCorrectionDSB->value();
	if (subtractCB->checkState() == Qt::Checked) curSettings[threadsCB->currentIndex()]->enableSub = true;
	else curSettings[threadsCB->currentIndex()]->enableSub = false;

	if (curSettings[threadsCB->currentIndex()]->pulseSize != curSettings[threadsCB->currentIndex()]->shape.size()
		&& curSettings[threadsCB->currentIndex()]->shape.size()) curSettings[threadsCB->currentIndex()]->shape.resize(curSettings[threadsCB->currentIndex()]->pulseSize, 0.f);
	corePtr->set_process_settings(curSettings[threadsCB->currentIndex()], threadsCB->currentIndex());

	shapeWid->set_smoothing(1.f - 1.f/std::pow(2, shapeSmoothSB->value()));
	update_sub_widgets();
}

void ProcessingDialog::reset() {
	curSettings[threadsCB->currentIndex()] = std::shared_ptr<ProcessingThread::Settings> (new ProcessingStandartCircuit::StanCircuitSettings);
	load_to_dial_settings(threadsCB->currentIndex());
	corePtr->set_process_settings(curSettings[threadsCB->currentIndex()], threadsCB->currentIndex());
	update_sub_widgets();
}

void ProcessingDialog::accept() {
	for (quint32 i = 0, ie = curSettings.size(); i < ie; ++i) {
		corePtr->set_process_settings(curSettings[i], i);
	}
	corePtr->stop();
	corePtr->set_setup_mode (false);
	QDialog::accept();
}

void ProcessingDialog::reject() {
	for (quint32 i = 0, ie = oldSettings.size(); i < ie; ++i) {
		corePtr->set_process_settings(oldSettings[i], i);
	}
	corePtr->stop();
	corePtr->set_setup_mode (false);
	QDialog::reject();
}

void ProcessingDialog::processing_finished() {
	std::vector<std::vector<float>> const* pulses = corePtr->get_setup_pulses();
	for (quint32 i = 0; i < pulses->size(); i++) shapeWid->set_pulse((*pulses)[i]);
}

void ProcessingDialog::state_changed(bool newState) {
	if (newState) startPB->setText("Stop");
	else startPB->setText("Start");
}

SettingsDialog::SettingsDialog(ProcessingDialog* parent) : QDialog (parent) {
	setInfo = new QLabel (this);
	acceptPB = new QPushButton (tr("Accept"), this);
	rejectPB = new QPushButton (tr("Reject"), this);
	mainLayout = new QGridLayout (this);

	mainLayout->addWidget(setInfo, 0, 0, 1, 2);
	connect (acceptPB, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect (rejectPB, SIGNAL(clicked(bool)), this, SLOT(reject()));
}

SearchSettingsDialog::SearchSettingsDialog(ProcessingDialog* parent) : SettingsDialog (parent) {
	skipSamplesL = new QLabel (tr("Skip samples"), this);
	skipSamplesSB = new QSpinBox(this);
}

SearchThresholdDialog::SearchThresholdDialog(ProcessingDialog* parent) : SearchSettingsDialog (parent) {
	setInfo->setText(tr("Pulse detected when (average of x[0...b]) - x[b + c] > t/n"
						"Here b - baseline samples, c - sample for comparsion position, t - threshold\n"
						"After detection search position shifts by Skip samples value."));
	baselineSamplesL = new QLabel (tr("Baseline samples"), this);
	baselineSamplesSB = new QSpinBox (this);
	detectPosL = new QLabel (tr("Compare sample pos"), this);
	detectPosSB = new QSpinBox (this);
	thresholdL = new QLabel(tr("Threshold"));
	thresholdDSB = new QDoubleSpinBox (this);

	thresholdDSB->setMaximum(1.);
	thresholdDSB->setSingleStep(.01);

	mainLayout->addWidget(skipSamplesL, 1, 0, 1, 1);
	mainLayout->addWidget(skipSamplesSB, 1, 1, 1, 1);
	mainLayout->addWidget(baselineSamplesL, 2, 0, 1, 1);
	mainLayout->addWidget(baselineSamplesSB, 2, 1, 1, 1);
	mainLayout->addWidget(detectPosL, 3, 0, 1, 1);
	mainLayout->addWidget(detectPosSB, 3, 1, 1, 1);
	mainLayout->addWidget(thresholdL, 4, 0, 1, 1);
	mainLayout->addWidget(thresholdDSB, 4, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 5, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 5, 1, 1, 1, Qt::AlignRight);
}

void SearchThresholdDialog::showEvent(QShowEvent *) {

	assert(settingsPtr->sSet->get_s_settings_id() == get_search_type());
	skipSamplesSB->setMaximum(settingsPtr->pulseSize);
	baselineSamplesSB->setMaximum(settingsPtr->pulseSize/2 - 1);
	detectPosSB->setMaximum(settingsPtr->pulseSize/2 - 1);

	PulseSearchingThreshold::SearchThresholdSettings* tmp = (PulseSearchingThreshold::SearchThresholdSettings*)settingsPtr->sSet.get();
	skipSamplesSB->setValue(tmp->skipSamples);
	baselineSamplesSB->setValue(tmp->detectBaselineSamples);
	detectPosSB->setValue(tmp->detectPos);
	thresholdDSB->setValue(tmp->threshold);
}

void SearchThresholdDialog::update_settings() {
	PulseSearchingThreshold::SearchThresholdSettings* tmp = (PulseSearchingThreshold::SearchThresholdSettings*)settingsPtr->sSet.get();
	if (settingsPtr->pulseSize < tmp->skipSamples) tmp->skipSamples = settingsPtr->pulseSize;
	if (settingsPtr->pulseSize/2 <= tmp->detectBaselineSamples) tmp->detectBaselineSamples = settingsPtr->pulseSize/2 - 1;
	if (settingsPtr->pulseSize/2 <= tmp->detectPos) tmp->detectPos = settingsPtr->pulseSize/2 - 1;
}

void SearchThresholdDialog::accept() {
	PulseSearchingThreshold::SearchThresholdSettings* tmp = (PulseSearchingThreshold::SearchThresholdSettings*)settingsPtr->sSet.get();
	tmp->skipSamples = skipSamplesSB->value();
	tmp->detectBaselineSamples = baselineSamplesSB->value();
	tmp->detectPos = detectPosSB->value();
	tmp->threshold = thresholdDSB->value();
	QDialog::accept();
}

bool SearchThresholdDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	PulseSearchingThreshold::SearchThresholdSettings* tmp = (PulseSearchingThreshold::SearchThresholdSettings*)sett->sSet.get();
	if (tmp->detectBaselineSamples + tmp->detectPos >= sett->pulseSize
		|| tmp->skipSamples > sett->pulseSize) return false;
	else return true;
}

SearchMonotonousDialog::SearchMonotonousDialog(ProcessingDialog* parent) : SearchSettingsDialog (parent) {
	setInfo->setText(tr("Pulse detected when x[i] < x[i+1] for i = 0...R.\n"
						"Also if F > 0 condition x[i] > x[i+1] for i = R+I...R+I+F takes place.\n"
						"Here R - rising samples, I - indifferent samples, F - falling samples.\n"
						"After detection search position shifts by Skip samples value."));
	risingSamplesL = new QLabel (tr("Rising samples"), this);
	risingSamplesSB = new QSpinBox (this);
	indiffSamplesL = new QLabel (tr("Indifferent samples"), this);
	indiffSamplesSB = new QSpinBox (this);
	fallingSamplesL = new QLabel (tr("Falling samples"), this);
	fallingSamplesSB = new QSpinBox (this);

	mainLayout->addWidget(risingSamplesL, 1, 0, 1, 1);
	mainLayout->addWidget(risingSamplesSB, 1, 1, 1, 1);
	mainLayout->addWidget(indiffSamplesL, 2, 0, 1, 1);
	mainLayout->addWidget(indiffSamplesSB, 2, 1, 1, 1);
	mainLayout->addWidget(fallingSamplesL, 3, 0, 1, 1);
	mainLayout->addWidget(fallingSamplesSB, 3, 1, 1, 1);
	mainLayout->addWidget(skipSamplesL, 4, 0, 1, 1);
	mainLayout->addWidget(skipSamplesSB, 4, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 5, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 5, 1, 1, 1, Qt::AlignRight);

	risingSamplesSB->setMinimum(1);
	skipSamplesSB->setMinimum(1);

	connect (risingSamplesSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
	connect (indiffSamplesSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
	connect (fallingSamplesSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
}

void SearchMonotonousDialog::showEvent(QShowEvent *) {

	assert(settingsPtr->sSet->get_s_settings_id() == get_search_type());
	skipSamplesSB->setMaximum(settingsPtr->pulseSize);
	risingSamplesSB->setMaximum(settingsPtr->pulseSize/2 - 1);
	indiffSamplesSB->setMaximum(settingsPtr->pulseSize/2 - 1);
	fallingSamplesSB->setMaximum(settingsPtr->pulseSize/2 - 1);

	PulseSearchingMonoton::SearchMonotonSettings* tmp = (PulseSearchingMonoton::SearchMonotonSettings*)settingsPtr->sSet.get();
	skipSamplesSB->setValue(tmp->skipSamples);
	risingSamplesSB->setValue(tmp->risingSamples);
	indiffSamplesSB->setValue(tmp->indiffSamples);
	fallingSamplesSB->setValue(tmp->fallingSamples);
}

void SearchMonotonousDialog::update_settings() {
	PulseSearchingMonoton::SearchMonotonSettings* tmp = (PulseSearchingMonoton::SearchMonotonSettings*)settingsPtr->sSet.get();

	if (settingsPtr->pulseSize < tmp->skipSamples) tmp->skipSamples = settingsPtr->pulseSize;
	if (settingsPtr->pulseSize <= tmp->risingSamples + tmp->fallingSamples + tmp->indiffSamples + 1)
		tmp->fallingSamples = settingsPtr->pulseSize - tmp->risingSamples - tmp->fallingSamples - 1;
}

void SearchMonotonousDialog::accept() {
	PulseSearchingMonoton::SearchMonotonSettings* tmp = (PulseSearchingMonoton::SearchMonotonSettings*)settingsPtr->sSet.get();

	tmp->skipSamples = skipSamplesSB->value();
	tmp->risingSamples = risingSamplesSB->value();
	tmp->fallingSamples = indiffSamplesSB->value();
	tmp->indiffSamples = fallingSamplesSB->value();
	QDialog::accept();
}

void SearchMonotonousDialog::check_values() {
	if (risingSamplesSB->value() + indiffSamplesSB->value() + fallingSamplesSB->value() + 1 >= (int)settingsPtr->pulseSize)
		fallingSamplesSB->setValue(settingsPtr->pulseSize - risingSamplesSB->value() - indiffSamplesSB->value() - 1);
}

bool SearchMonotonousDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	PulseSearchingMonoton::SearchMonotonSettings* tmp = (PulseSearchingMonoton::SearchMonotonSettings*)sett->sSet.get();

	if (sett->pulseSize <= tmp->risingSamples + tmp->fallingSamples + tmp->indiffSamples + 1
		|| tmp->skipSamples > sett->pulseSize) return false;
	else return true;
}

SearchTanThresholdDialog::SearchTanThresholdDialog(ProcessingDialog* parent) : SearchSettingsDialog (parent) {
	setInfo->setText(tr("Pulse detected when slope (first differential) of x[0..R] larger than Rising slope.\n"
						"Also if F > 0 condition slope (x[R+I...R+I+F]) < Falling slope takes place.\n"
						"Here R - rising samples, I - indifferent samples, F - falling samples.\n"
						"After detection search position shifts by Skip samples value."));
	risingSamplesL = new QLabel (tr("Rising samples"), this);
	risingSamplesSB = new QSpinBox (this);
	indiffSamplesL = new QLabel (tr("Indifferent samples"), this);
	indiffSamplesSB = new QSpinBox (this);
	fallingSamplesL = new QLabel (tr("Falling samples"), this);
	fallingSamplesSB = new QSpinBox (this);
	risingTanL = new QLabel (tr("Rising slope"), this);
	risingTanDSB = new QDoubleSpinBox (this);
	fallingTanL = new QLabel (tr("Falling slope"), this);
	fallingTanDSB = new QDoubleSpinBox (this);

	mainLayout->addWidget(risingSamplesL, 1, 0, 1, 1);
	mainLayout->addWidget(risingSamplesSB, 1, 1, 1, 1);
	mainLayout->addWidget(indiffSamplesL, 2, 0, 1, 1);
	mainLayout->addWidget(indiffSamplesSB, 2, 1, 1, 1);
	mainLayout->addWidget(fallingSamplesL, 3, 0, 1, 1);
	mainLayout->addWidget(fallingSamplesSB, 3, 1, 1, 1);
	mainLayout->addWidget(risingTanL, 4, 0, 1, 1);
	mainLayout->addWidget(risingTanDSB, 4, 1, 1, 1);
	mainLayout->addWidget(fallingTanL, 5, 0, 1, 1);
	mainLayout->addWidget(fallingTanDSB, 5, 1, 1, 1);
	mainLayout->addWidget(skipSamplesL, 6, 0, 1, 1);
	mainLayout->addWidget(skipSamplesSB, 6, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 7, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 7, 1, 1, 1, Qt::AlignRight);

	risingSamplesSB->setMinimum(1);
	skipSamplesSB->setMinimum(1);
	risingTanDSB->setMaximum(1.);
	risingTanDSB->setMinimum(0.001);
	risingTanDSB->setSingleStep(0.01);
	risingTanDSB->setDecimals(3);
	fallingTanDSB->setMinimum(-1.);
	fallingTanDSB->setMaximum(0.);
	fallingTanDSB->setSingleStep(0.01);
	fallingTanDSB->setDecimals(3);

	connect (risingSamplesSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
	connect (indiffSamplesSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
	connect (fallingSamplesSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
}

void SearchTanThresholdDialog::showEvent(QShowEvent *) {

	assert(settingsPtr->sSet->get_s_settings_id() == get_search_type());
	skipSamplesSB->setMaximum(settingsPtr->pulseSize);
	risingSamplesSB->setMaximum(settingsPtr->pulseSize/2 - 1);
	indiffSamplesSB->setMaximum(settingsPtr->pulseSize/2 - 1);
	fallingSamplesSB->setMaximum(settingsPtr->pulseSize/2 - 1);

	PulseSearchingTanThreshold::SearchTanThresholdSettings* tmp = (PulseSearchingTanThreshold::SearchTanThresholdSettings*)settingsPtr->sSet.get();
	skipSamplesSB->setValue(tmp->skipSamples);
	risingSamplesSB->setValue(tmp->risingSamples);
	indiffSamplesSB->setValue(tmp->indiffSamples);
	fallingSamplesSB->setValue(tmp->fallingSamples);
	risingTanDSB->setValue(tmp->risingTan);
	fallingTanDSB->setValue(tmp->fallingTan);
}

void SearchTanThresholdDialog::update_settings() {
	PulseSearchingTanThreshold::SearchTanThresholdSettings* tmp = (PulseSearchingTanThreshold::SearchTanThresholdSettings*)settingsPtr->sSet.get();

	if (settingsPtr->pulseSize < tmp->skipSamples) tmp->skipSamples = settingsPtr->pulseSize;
	if (settingsPtr->pulseSize <= tmp->risingSamples + tmp->fallingSamples + tmp->indiffSamples + 1)
		tmp->fallingSamples = settingsPtr->pulseSize - tmp->risingSamples - tmp->fallingSamples - 1;
}

void SearchTanThresholdDialog::accept() {
	PulseSearchingTanThreshold::SearchTanThresholdSettings* tmp = (PulseSearchingTanThreshold::SearchTanThresholdSettings*)settingsPtr->sSet.get();

	tmp->skipSamples = skipSamplesSB->value();
	tmp->risingSamples = risingSamplesSB->value();
	tmp->indiffSamples = indiffSamplesSB->value();
	tmp->fallingSamples = fallingSamplesSB->value();
	tmp->risingTan = risingTanDSB->value();
	tmp->fallingTan = fallingTanDSB->value();
	QDialog::accept();
}

void SearchTanThresholdDialog::check_values() {
	if (risingSamplesSB->value() + indiffSamplesSB->value() + fallingSamplesSB->value() + 1 >= (int)settingsPtr->pulseSize)
		fallingSamplesSB->setValue(settingsPtr->pulseSize - risingSamplesSB->value() - indiffSamplesSB->value() - 1);
}

bool SearchTanThresholdDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	PulseSearchingTanThreshold::SearchTanThresholdSettings* tmp = (PulseSearchingTanThreshold::SearchTanThresholdSettings*)sett->sSet.get();

	if (sett->pulseSize <= tmp->risingSamples + tmp->fallingSamples + tmp->indiffSamples + 1
		|| tmp->skipSamples > sett->pulseSize) return false;
	else return true;
}

DiscriminatorDialog::DiscriminatorDialog(ProcessingDialog* parent) : SettingsDialog (parent) {
	discEnabledL = new QLabel(tr("Discriminator enabled"), this);
	discEnabledCB = new QCheckBox (this);
}

DiscriminatorDispersionDialog::DiscriminatorDispersionDialog (ProcessingDialog* parent) : DiscriminatorDialog (parent) {
	setInfo->setText(tr("This discriminator rejects all pulses if dispersion between\n"
						"detected pulse and shape larger than Max dispersion value\n"
						"Shape must be determined."));
	maxDisperL = new QLabel (tr("Max dispersion"), this);
	maxDisperDSB = new QDoubleSpinBox (this);

	maxDisperDSB->setMaximum(1.);
	maxDisperDSB->setSingleStep(.01);

	mainLayout->addWidget(discEnabledL, 1, 0, 1, 1);
	mainLayout->addWidget(discEnabledCB, 1, 1, 1, 1);
	mainLayout->addWidget(maxDisperL, 2, 0, 1, 1);
	mainLayout->addWidget(maxDisperDSB, 2, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 3, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 3, 1, 1, 1, Qt::AlignRight);

	connect(discEnabledCB, SIGNAL(stateChanged(int)), this, SLOT(check_values()));
}

void DiscriminatorDispersionDialog::showEvent(QShowEvent *) {
	assert(settingsPtr->dSet->get_d_settings_id() == get_disc_type());
	PulseDiscriminatorByDispersion::DispDiscSettings* tmp = (PulseDiscriminatorByDispersion::DispDiscSettings*)settingsPtr->dSet.get();
	if (tmp->enabled) discEnabledCB->setCheckState(Qt::Checked);
	else discEnabledCB->setCheckState(Qt::Unchecked);
	maxDisperDSB->setValue(tmp->maxDispersion);
}

void DiscriminatorDispersionDialog::check_values() {
	if (discEnabledCB->checkState() == Qt::Checked
		&& settingsPtr->pulseSize != settingsPtr->shape.size()) acceptPB->setEnabled(false);
	else acceptPB->setEnabled(true);
}

void DiscriminatorDispersionDialog::update_settings() {
	PulseDiscriminatorByDispersion::DispDiscSettings* tmp = (PulseDiscriminatorByDispersion::DispDiscSettings*)settingsPtr->dSet.get();
	if (settingsPtr->pulseSize != settingsPtr->shape.size()) tmp->enabled = false;
}

void DiscriminatorDispersionDialog::accept() {
	PulseDiscriminatorByDispersion::DispDiscSettings* tmp = (PulseDiscriminatorByDispersion::DispDiscSettings*)settingsPtr->dSet.get();
	if (discEnabledCB->checkState() == Qt::Checked) tmp->enabled = true;
	else tmp->enabled = false;
	tmp->maxDispersion = maxDisperDSB->value();
	QDialog::accept();
}

bool DiscriminatorDispersionDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	if (sett->dSet->enabled
		&& sett->pulseSize != sett->shape.size()) return false;
	else return true;
}

DiscriminatorNeuralNetDialog::DiscriminatorNeuralNetDialog (std::vector<Neural_Network::NuclearPhysicsNeuralNet>* discr, ProcessingDialog* parent) : DiscriminatorDialog (parent) {
	setInfo->setText(tr("This discriminator rejects all pulses if result of neural\n"
						"network is false."));
	discrPtr = discr;

	nnListL = new QLabel (tr("Neural network:"), this);
	nnListCB = new QComboBox (this);
	nnInputsL = new QLabel (tr("Inputs:"), this);
	nnInputsI = new QLabel (this);

	mainLayout->addWidget(discEnabledL, 1, 0, 1, 1);
	mainLayout->addWidget(discEnabledCB, 1, 1, 1, 1);
	mainLayout->addWidget(nnListL, 2, 0, 1, 1);
	mainLayout->addWidget(nnListCB, 2, 1, 1, 1);
	mainLayout->addWidget(nnInputsL, 3, 0, 1, 1);
	mainLayout->addWidget(nnInputsI, 3, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 4, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 4, 1, 1, 1, Qt::AlignRight);

	connect(discEnabledCB, SIGNAL(stateChanged(int)), this, SLOT(check_values()));
}

void DiscriminatorNeuralNetDialog::showEvent(QShowEvent *) {
	disconnect (nnListCB, SIGNAL(currentIndexChanged(int)), this, SLOT(check_values()));
	assert(settingsPtr->dSet->get_d_settings_id() == get_disc_type());
	nnListCB->clear();

	if (settingsPtr->dSet->enabled) discEnabledCB->setCheckState(Qt::Checked);
	else discEnabledCB->setCheckState(Qt::Unchecked);

	for (auto a = discrPtr->begin(); a != discrPtr->end(); a++) {
		nnListCB->addItem(a->get_name());
	}
	check_values();
	connect (nnListCB, SIGNAL(currentIndexChanged(int)), this, SLOT(check_values()));
}

void DiscriminatorNeuralNetDialog::check_values() {
	if (discEnabledCB->checkState() == Qt::Checked && discrPtr->size() == 0) {
		acceptPB->setEnabled(false);
		return;
	}
	if (discEnabledCB->checkState() == Qt::Checked
		&& (*discrPtr)[nnListCB->currentIndex()].inputs() != settingsPtr->pulseSize) acceptPB->setEnabled(false);
	else acceptPB->setEnabled(true);
	std::stringstream sstr;
	sstr << (*discrPtr)[nnListCB->currentIndex()].inputs();
	nnInputsI->setText(sstr.str().c_str());
}

void DiscriminatorNeuralNetDialog::update_settings() {
}

void DiscriminatorNeuralNetDialog::accept() {
	PulseDiscriminatorByNeuralNet::DiscNNSettings* tmp = (PulseDiscriminatorByNeuralNet::DiscNNSettings*)settingsPtr->dSet.get();
	if (discEnabledCB->checkState() == Qt::Checked) settingsPtr->dSet->enabled = true;
	else settingsPtr->dSet->enabled = false;
	tmp->neuralNet = (*discrPtr)[nnListCB->currentIndex()];
	QDialog::accept();
}

bool DiscriminatorNeuralNetDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	PulseDiscriminatorByNeuralNet::DiscNNSettings* tmp = (PulseDiscriminatorByNeuralNet::DiscNNSettings*)sett->dSet.get();
	if (tmp->enabled
		&& (tmp->neuralNet.inputs() != sett->pulseSize || tmp->neuralNet.is_empty())) return false;
	else return true;
}

AmplMeasDialog::AmplMeasDialog(ProcessingDialog* parent) : SettingsDialog (parent) {
	blSamplesL = new QLabel (tr("Baseline samples"), this);
	blSamplesSB = new QSpinBox (this);
}

AmplMeasMaxDialog::AmplMeasMaxDialog(ProcessingDialog* parent) : AmplMeasDialog (parent) {
	setInfo->setText(tr("Amplitude measuring procedure based on determining of maximum value\n"
						"Amplitude is a value of maximum value minus baseline value.\n"
						"Baseline determines as an average of first bls samples.\n"
						"Maximum value is looking for between x[lr] and x[rr].\n"
						"Here: bls is Baseline samples, lr and rr - left and right restrictions."));
	maxLeftPosL = new QLabel (tr("Max value position left restriction"), this);
	maxLeftPosSB = new QSpinBox(this);
	maxRightPosL = new QLabel (tr("Max value position right restriction"), this);;
	maxRightPosSB = new QSpinBox(this);

	mainLayout->addWidget(blSamplesL, 1, 0, 1, 1);
	mainLayout->addWidget(blSamplesSB, 1, 1, 1, 1);
	mainLayout->addWidget(maxLeftPosL, 2, 0, 1, 1);
	mainLayout->addWidget(maxLeftPosSB, 2, 1, 1, 1);
	mainLayout->addWidget(maxRightPosL, 3, 0, 1, 1);
	mainLayout->addWidget(maxRightPosSB, 3, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 4, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 4, 1, 1, 1, Qt::AlignRight);

	connect(blSamplesSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
	connect(maxLeftPosSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
	connect(maxRightPosSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
}

void AmplMeasMaxDialog::showEvent(QShowEvent *) {

	assert(settingsPtr->aSet->get_a_settings_id() == get_ampl_type());
	blSamplesSB->setMaximum(settingsPtr->pulseSize/2 - 1);
	maxLeftPosSB->setMaximum(settingsPtr->pulseSize - 1);
	maxRightPosSB->setMaximum(settingsPtr->pulseSize);

	PulseAmplMeasuringByMax::AmplMaxSettings* tmp = (PulseAmplMeasuringByMax::AmplMaxSettings*)settingsPtr->aSet.get();
	blSamplesSB->setValue(tmp->processBaselineSamples);
	maxLeftPosSB->setValue(tmp->maxValIntervalLeft);
	maxRightPosSB->setValue(tmp->maxValIntervalRight);
}

void AmplMeasMaxDialog::update_settings() {
	PulseAmplMeasuringByMax::AmplMaxSettings* tmp = (PulseAmplMeasuringByMax::AmplMaxSettings*)settingsPtr->aSet.get();
	if (settingsPtr->pulseSize/2 <= tmp->processBaselineSamples) tmp->processBaselineSamples = settingsPtr->pulseSize/2 - 1;
	if (settingsPtr->pulseSize <= tmp->maxValIntervalLeft) tmp->maxValIntervalLeft = settingsPtr->pulseSize - 1;
	if (settingsPtr->pulseSize < tmp->maxValIntervalRight) tmp->maxValIntervalRight = settingsPtr->pulseSize;
}

void AmplMeasMaxDialog::accept() {
	PulseAmplMeasuringByMax::AmplMaxSettings* tmp = (PulseAmplMeasuringByMax::AmplMaxSettings*)settingsPtr->aSet.get();
	tmp->processBaselineSamples = blSamplesSB->value();
	tmp->maxValIntervalLeft = maxLeftPosSB->value();
	tmp->maxValIntervalRight = maxRightPosSB->value();
	QDialog::accept();
}

bool AmplMeasMaxDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	PulseAmplMeasuringByMax::AmplMaxSettings* tmp = (PulseAmplMeasuringByMax::AmplMaxSettings*)sett->aSet.get();
	if (tmp->processBaselineSamples > sett->pulseSize
		|| tmp->maxValIntervalRight > sett->pulseSize) return false;
	else return true;
}

void AmplMeasMaxDialog::check_values() {
	if (maxLeftPosSB->value() >= maxRightPosSB->value()) maxRightPosSB->setValue(maxLeftPosSB->value() + 1);
	if (blSamplesSB->value() >= maxLeftPosSB->value()) maxLeftPosSB->setValue(blSamplesSB->value() + 1);
}

AmplMeasPolyMaxDialog::AmplMeasPolyMaxDialog(ProcessingDialog* parent) : AmplMeasDialog (parent) {
	setInfo->setText(tr("Amplitude measuring procedure based on determining of maximum value of polynomial\n"
						"Input data X will approximate by P polynomial.\n"
						"Amplitude is a value of maximum value of P minus baseline value.\n"
						"Baseline determines as an average of first bls samples.\n"
						"Maximum value is looking for between p[lr] and p[rr].\n"
						"Here: bls is Baseline samples, lr and rr - left and right restrictions."));
	polyOrderL = new QLabel (tr("Polynomial order"), this);
	polyOrderSB = new QSpinBox(this);
	maxLeftPosL = new QLabel (tr("Max value position left restriction"), this);
	maxLeftPosSB = new QSpinBox(this);
	maxRightPosL = new QLabel (tr("Max value position right restriction"), this);;
	maxRightPosSB = new QSpinBox(this);

	polyOrderSB->setMaximum(10);

	mainLayout->addWidget(blSamplesL, 1, 0, 1, 1);
	mainLayout->addWidget(blSamplesSB, 1, 1, 1, 1);
	mainLayout->addWidget(polyOrderL, 2, 0, 1, 1);
	mainLayout->addWidget(polyOrderSB, 2, 1, 1, 1);
	mainLayout->addWidget(maxLeftPosL, 3, 0, 1, 1);
	mainLayout->addWidget(maxLeftPosSB, 3, 1, 1, 1);
	mainLayout->addWidget(maxRightPosL, 4, 0, 1, 1);
	mainLayout->addWidget(maxRightPosSB, 4, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 5, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 5, 1, 1, 1, Qt::AlignRight);

	connect(blSamplesSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
	connect(maxLeftPosSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
	connect(maxRightPosSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
}

void AmplMeasPolyMaxDialog::showEvent(QShowEvent *) {

	assert(settingsPtr->aSet->get_a_settings_id() == get_ampl_type());
	blSamplesSB->setMaximum(settingsPtr->pulseSize/2 - 1);
	maxLeftPosSB->setMaximum(settingsPtr->pulseSize - 1);
	maxRightPosSB->setMaximum(settingsPtr->pulseSize);

	PulseAmplMeasuringPolyMax::AmplPolyMaxSettings* tmp = (PulseAmplMeasuringPolyMax::AmplPolyMaxSettings*)settingsPtr->aSet.get();
	blSamplesSB->setValue(tmp->processBaselineSamples);
	polyOrderSB->setValue(tmp->polyOrder);
	maxLeftPosSB->setValue(tmp->maxValIntervalLeft);
	maxRightPosSB->setValue(tmp->maxValIntervalRight);
}

void AmplMeasPolyMaxDialog::update_settings() {
	PulseAmplMeasuringPolyMax::AmplPolyMaxSettings* tmp = (PulseAmplMeasuringPolyMax::AmplPolyMaxSettings*)settingsPtr->aSet.get();
	if (settingsPtr->pulseSize/2 <= tmp->processBaselineSamples) tmp->processBaselineSamples = settingsPtr->pulseSize/2 - 1;
	if (settingsPtr->pulseSize <= tmp->maxValIntervalLeft) tmp->maxValIntervalLeft = settingsPtr->pulseSize - 1;
	if (settingsPtr->pulseSize < tmp->maxValIntervalRight) tmp->maxValIntervalRight = settingsPtr->pulseSize;
}

void AmplMeasPolyMaxDialog::accept() {
	PulseAmplMeasuringPolyMax::AmplPolyMaxSettings* tmp = (PulseAmplMeasuringPolyMax::AmplPolyMaxSettings*)settingsPtr->aSet.get();
	tmp->processBaselineSamples = blSamplesSB->value();
	tmp->polyOrder = polyOrderSB->value();
	tmp->maxValIntervalLeft = maxLeftPosSB->value();
	tmp->maxValIntervalRight = maxRightPosSB->value();
	QDialog::accept();
}

bool AmplMeasPolyMaxDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	PulseAmplMeasuringPolyMax::AmplPolyMaxSettings* tmp = (PulseAmplMeasuringPolyMax::AmplPolyMaxSettings*)settingsPtr->aSet.get();
	if (tmp->processBaselineSamples > sett->pulseSize
		|| tmp->maxValIntervalRight > sett->pulseSize) return false;
	else return true;
}

void AmplMeasPolyMaxDialog::check_values() {
	if (maxLeftPosSB->value() >= maxRightPosSB->value()) maxRightPosSB->setValue(maxLeftPosSB->value() + 1);
	if (blSamplesSB->value() >= maxLeftPosSB->value()) maxLeftPosSB->setValue(blSamplesSB->value() + 1);
}

AmplMeasNeuralNetDialog::AmplMeasNeuralNetDialog(std::vector<Neural_Network::NuclearPhysicsNeuralNet>* ampl, ProcessingDialog* parent) : AmplMeasDialog (parent) {
	setInfo->setText(tr("Amplitude detemining based on using artifitial neural network.\n"
						"Amplitude is a value on an output of previously teached neural net."));

	nnListL = new QLabel (tr("Neural network:"), this);
	nnListCB = new QComboBox (this);
	nnInputsL = new QLabel (tr("Inputs:"), this);
	nnInputsI = new QLabel (this);

	amplPtr = ampl;

	mainLayout->addWidget(blSamplesL, 1, 0, 1, 1);
	mainLayout->addWidget(blSamplesSB, 1, 1, 1, 1);
	mainLayout->addWidget(nnListL, 2, 0, 1, 1);
	mainLayout->addWidget(nnListCB, 2, 1, 1, 1);
	mainLayout->addWidget(nnInputsL, 3, 0, 1, 1);
	mainLayout->addWidget(nnInputsI, 3, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 4, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 4, 1, 1, 1, Qt::AlignRight);
}

void AmplMeasNeuralNetDialog::showEvent(QShowEvent *) {

	assert(settingsPtr->aSet->get_a_settings_id() == get_ampl_type());
	disconnect (nnListCB, SIGNAL(currentIndexChanged(int)), this, SLOT(check_values()));
	blSamplesSB->setValue(settingsPtr->aSet->processBaselineSamples);
	nnListCB->clear();

	for (auto a = amplPtr->begin(); a != amplPtr->end(); a++) {
		nnListCB->addItem(a->get_name());
	}
	check_values();
	connect (nnListCB, SIGNAL(currentIndexChanged(int)), this, SLOT(check_values()));
}

void AmplMeasNeuralNetDialog::check_values() {
	if (amplPtr->size() == 0) {
		acceptPB->setEnabled(false);
		return;
	}
	if ((*amplPtr)[nnListCB->currentIndex()].inputs() != settingsPtr->pulseSize) acceptPB->setEnabled(false);
	else acceptPB->setEnabled(true);
	std::stringstream sstr;
	sstr << (*amplPtr)[nnListCB->currentIndex()].inputs();
	nnInputsI->setText(sstr.str().c_str());
}

void AmplMeasNeuralNetDialog::update_settings() {
}

void AmplMeasNeuralNetDialog::accept() {
	PulseAmplMeasuringNeuralNet::AmplNNSettings* tmp = (PulseAmplMeasuringNeuralNet::AmplNNSettings*)settingsPtr->aSet.get();
	settingsPtr->aSet->processBaselineSamples = blSamplesSB->value();
	tmp->neuralNet = (*amplPtr)[nnListCB->currentIndex()];
	QDialog::accept();
}

bool AmplMeasNeuralNetDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	PulseAmplMeasuringNeuralNet::AmplNNSettings* tmp = (PulseAmplMeasuringNeuralNet::AmplNNSettings*)sett->aSet.get();
	if (sett->pulseSize != tmp->neuralNet.inputs() || tmp->neuralNet.is_empty()) return false;
	else return true;

}

TimeMeasMaxDialog::TimeMeasMaxDialog(ProcessingDialog* parent) : TimeMeasDialog (parent) {
	setInfo->setText(tr("Time measuring procedure based on determining of maximum value\n"
						"Time determines as position of maximum value.\n"
						"Maximum value is looking for between x[lr] and x[rr].\n"
						"Here: lr and rr - left and right restrictions."));
	maxLeftPosL = new QLabel (tr("Max value position left restriction"), this);
	maxLeftPosSB = new QSpinBox(this);
	maxRightPosL = new QLabel (tr("Max value position right restriction"), this);;
	maxRightPosSB = new QSpinBox(this);

	mainLayout->addWidget(maxLeftPosL, 1, 0, 1, 1);
	mainLayout->addWidget(maxLeftPosSB, 1, 1, 1, 1);
	mainLayout->addWidget(maxRightPosL, 2, 0, 1, 1);
	mainLayout->addWidget(maxRightPosSB, 2, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 3, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 3, 1, 1, 1, Qt::AlignRight);

	connect(maxLeftPosSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
	connect(maxRightPosSB, SIGNAL(valueChanged(int)), this, SLOT(check_values()));
}

void TimeMeasMaxDialog::showEvent(QShowEvent *) {

	assert(settingsPtr->tSet->get_t_settings_id() == get_time_type());
	maxLeftPosSB->setMaximum(settingsPtr->pulseSize - 1);
	maxRightPosSB->setMaximum(settingsPtr->pulseSize);

	PulseTimeMeasuringByMax::TimeMaxSettings* tmp = (PulseTimeMeasuringByMax::TimeMaxSettings*)settingsPtr->tSet.get();
	maxLeftPosSB->setValue(tmp->maxValIntervalLeft);
	maxRightPosSB->setValue(tmp->maxValIntervalRight);
}

void TimeMeasMaxDialog::update_settings() {
	PulseTimeMeasuringByMax::TimeMaxSettings* tmp = (PulseTimeMeasuringByMax::TimeMaxSettings*)settingsPtr->tSet.get();
	if (settingsPtr->pulseSize <= tmp->maxValIntervalLeft) tmp->maxValIntervalLeft = settingsPtr->pulseSize - 1;
	if (settingsPtr->pulseSize < tmp->maxValIntervalRight) tmp->maxValIntervalRight = settingsPtr->pulseSize;

}

bool TimeMeasMaxDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	PulseTimeMeasuringByMax::TimeMaxSettings* tmp = (PulseTimeMeasuringByMax::TimeMaxSettings*)sett->tSet.get();
	if (tmp->maxValIntervalRight > sett->pulseSize) return false;
	else return true;
}

void TimeMeasMaxDialog::accept() {
	PulseTimeMeasuringByMax::TimeMaxSettings* tmp = (PulseTimeMeasuringByMax::TimeMaxSettings*)settingsPtr->tSet.get();
	tmp->maxValIntervalLeft = maxLeftPosSB->value();
	tmp->maxValIntervalRight = maxRightPosSB->value();
	QDialog::accept();
}

void TimeMeasMaxDialog::check_values() {
	if (maxLeftPosSB->value() >= maxRightPosSB->value()) maxRightPosSB->setValue(maxLeftPosSB->value() + 1);
}

TimeMeasNeuralNetDialog::TimeMeasNeuralNetDialog(std::vector<Neural_Network::NuclearPhysicsNeuralNet>* time, ProcessingDialog* parent) : TimeMeasDialog (parent) {
	setInfo->setText(tr("Time of detection detemining based on using artifitial neural network.\n"
						"Time is a value on an output of previously teached neural net."));

	nnListL = new QLabel (tr("Neural network:"), this);
	nnListCB = new QComboBox (this);
	nnInputsL = new QLabel (tr("Inputs:"), this);
	nnInputsI = new QLabel (this);

	timePtr = time;

	mainLayout->addWidget(nnListL, 1, 0, 1, 1);
	mainLayout->addWidget(nnListCB, 1, 1, 1, 1);
	mainLayout->addWidget(nnInputsL, 2, 0, 1, 1);
	mainLayout->addWidget(nnInputsI, 2, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 3, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 3, 1, 1, 1, Qt::AlignRight);
}

void TimeMeasNeuralNetDialog::showEvent(QShowEvent *) {

	assert(settingsPtr->tSet->get_t_settings_id() == get_time_type());
	disconnect (nnListCB, SIGNAL(currentIndexChanged(int)), this, SLOT(check_values()));
	nnListCB->clear();

	for (auto a = timePtr->begin(); a != timePtr->end(); a++) {
		nnListCB->addItem(a->get_name());
	}
	check_values();
	connect (nnListCB, SIGNAL(currentIndexChanged(int)), this, SLOT(check_values()));
}

void TimeMeasNeuralNetDialog::check_values() {
	if (timePtr->size() == 0) {
		acceptPB->setEnabled(false);
		return;
	}
	if ((*timePtr)[nnListCB->currentIndex()].inputs() != settingsPtr->pulseSize) acceptPB->setEnabled(false);
	else acceptPB->setEnabled(true);
	std::stringstream sstr;
	sstr << (*timePtr)[nnListCB->currentIndex()].inputs();
	nnInputsI->setText(sstr.str().c_str());
}

void TimeMeasNeuralNetDialog::update_settings() {
}

bool TimeMeasNeuralNetDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	PulseTimeMeasuringNeuralNet::TimeNNSettings* tmp = (PulseTimeMeasuringNeuralNet::TimeNNSettings*)sett->tSet.get();
	if (sett->pulseSize != tmp->neuralNet.inputs() || tmp->neuralNet.is_empty()) return false;
	else return true;
}

void TimeMeasNeuralNetDialog::accept() {
	PulseTimeMeasuringNeuralNet::TimeNNSettings* tmp = (PulseTimeMeasuringNeuralNet::TimeNNSettings*)settingsPtr->tSet.get();
	tmp->neuralNet = (*timePtr)[nnListCB->currentIndex()];
	QDialog::accept();
}

CircuitSettingsDialog::CircuitSettingsDialog (Core *core, ProcessingDialog *parent) : SettingsDialog(parent) {
	corePtr = core;

	setInfo->setText(tr("Data source - select input stream for this processing thread."));

	currDataStreamL = new QLabel (tr("Data source"), this);
	currDataStreamCB = new QComboBox (this);

	mainLayout->addWidget(currDataStreamL, 1, 0, 1, 1);
	mainLayout->addWidget(currDataStreamCB, 1, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 3, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 3, 1, 1, 1, Qt::AlignRight);
}

void CircuitSettingsDialog::showEvent(QShowEvent *) {
	currDataStreamCB->clear();
	if (corePtr->get_input_streams()) {
		for (quint32 i = 0, ie = corePtr->get_input_streams(); i < ie; ++i) {
			currDataStreamCB->addItem(corePtr->get_input_name(i));
		}
		currDataStreamCB->setCurrentIndex(settingsPtr->inputNum);
	}
}

void CircuitSettingsDialog::update_settings() {
	if (corePtr->get_input_streams()) acceptPB->setEnabled(true);
	else acceptPB->setEnabled(false);
}

bool CircuitSettingsDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	if (sett->inputNum < corePtr->get_input_streams()) return true;
	else return false;
}

void CircuitSettingsDialog::accept() {
	settingsPtr->inputNum = currDataStreamCB->currentIndex();
	QDialog::accept();
}

CoincidenceSettingsDialog::CoincidenceSettingsDialog(Core *core, ProcessingDialog *parent) : CircuitSettingsDialog(core, parent) {

	setInfo->setText(tr("Data source - select input stream for this processing thread.\n"
						"Here user can customize coincidence circuit.\n"
						"Coincidence with - assign thread which we look for coincidence.\n"
						"Amplitude restrictions - is an interval of amplitudes we considering.\n"
						"Time difference - maximum time between pulses in two threads."));
	coinThreadL = new QLabel(tr("Coincidence with"), this);
	coinThreadCB = new QComboBox (this);
	coinAmplMinL = new QLabel(tr("Amplitude restiction left"), this);
	coinAmplMinDSB = new QDoubleSpinBox (this);
	coinAmplMaxL = new QLabel(tr("Amplitude restiction right"), this);
	coinAmplMaxDSB = new QDoubleSpinBox (this);
	coinTimeDiffL = new QLabel(tr("Maximum time difference"), this);
	coinTimeDiffDSB = new QDoubleSpinBox (this);

	coinAmplMinDSB->setMaximum(0.999);
	coinAmplMinDSB->setDecimals(3);
	coinAmplMinDSB->setSingleStep(0.01);
	coinAmplMaxDSB->setMaximum(1.0);
	coinAmplMaxDSB->setDecimals(3);
	coinAmplMaxDSB->setSingleStep(0.01);
	coinTimeDiffDSB->setMaximum(256.);

	mainLayout->addWidget(coinThreadL, 2, 0, 1, 1);
	mainLayout->addWidget(coinThreadCB, 2, 1, 1, 1);
	mainLayout->addWidget(coinAmplMinL, 3, 0, 1, 1);
	mainLayout->addWidget(coinAmplMinDSB, 3, 1, 1, 1);
	mainLayout->addWidget(coinAmplMaxL, 4, 0, 1, 1);
	mainLayout->addWidget(coinAmplMaxDSB, 4, 1, 1, 1);
	mainLayout->addWidget(coinTimeDiffL, 5, 0, 1, 1);
	mainLayout->addWidget(coinTimeDiffDSB, 5, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 7, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 7, 1, 1, 1, Qt::AlignRight);

	connect (coinAmplMinDSB, SIGNAL(valueChanged(double)), this, SLOT(check_values()));
	connect (coinAmplMaxDSB, SIGNAL(valueChanged(double)), this, SLOT(check_values()));

}

void CoincidenceSettingsDialog::showEvent(QShowEvent *ev) {
	CircuitSettingsDialog::showEvent(ev);
	coinThreadCB->clear();
	for (quint32 i = 0, ie = corePtr->get_process_threads(); i < ie; ++i) {
		if (i != currThread) coinThreadCB->addItem(corePtr->get_process_name(i), QVariant(i));
	}

	ProcessingCoincidenceCircuit::CoinCircuitSettings* tmp = (ProcessingCoincidenceCircuit::CoinCircuitSettings*)settingsPtr.get();
	coinThreadCB->setCurrentIndex(coinThreadCB->findData(QVariant(tmp->coinIndex)));
	coinAmplMinDSB->setValue(tmp->amplitudeIntervalL);
	coinAmplMaxDSB->setValue(tmp->amplitudeIntervalR);
	coinTimeDiffDSB->setValue(tmp->maxTimeDifference);
}

void CoincidenceSettingsDialog::update_settings() {
	if (coinThreadCB->count()) acceptPB->setEnabled(true);
	else acceptPB->setEnabled(false);
}

bool CoincidenceSettingsDialog::check_settings(std::shared_ptr<ProcessingThread::Settings> sett) {
	ProcessingCoincidenceCircuit::CoinCircuitSettings* tmp = (ProcessingCoincidenceCircuit::CoinCircuitSettings*)sett.get();
	if (tmp->amplitudeIntervalL >= tmp->amplitudeIntervalR ||
		tmp->coinIndex >= corePtr->get_process_threads()) return false;
	else return true;
}

void CoincidenceSettingsDialog::check_values() {
	if (coinAmplMinDSB->value() >= coinAmplMaxDSB->value()) coinAmplMaxDSB->setValue(coinAmplMinDSB->value() + 0.001);
}

void CoincidenceSettingsDialog::accept() {
	ProcessingCoincidenceCircuit::CoinCircuitSettings* tmp = (ProcessingCoincidenceCircuit::CoinCircuitSettings*)settingsPtr.get();
	tmp->coinIndex = coinThreadCB->currentData().toUInt();
	tmp->amplitudeIntervalL = coinAmplMinDSB->value();
	tmp->amplitudeIntervalR = coinAmplMaxDSB->value();
	tmp->maxTimeDifference = coinTimeDiffDSB->value();
	CircuitSettingsDialog::accept();
}
