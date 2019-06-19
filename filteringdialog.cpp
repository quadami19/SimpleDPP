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

#include "filteringdialog.hpp"

FilteringDialog::FilteringDialog(Core* core, QWidget* parent) : QDialog (parent) {

	coreClassPtr = core;
	streamCB = new QComboBox (this);
	applyPB = new QPushButton (tr("Apply (current stream)"), this);
	enableLabel = new QLabel (tr("Enable filtering"), this);
	enableCB = new QCheckBox (this);
	filterAddCB = new QComboBox (this);
	filterAddPB = new QPushButton (tr("Add filter"), this);
	acceptPB = new QPushButton (tr("Accept"), this);
	rejectPB = new QPushButton (tr("Reject"), this);
	filterAddCB->addItem(tr("Delay"), QVariant((quint32)FilteringProcessor::Delay));
	filterAddCB->addItem(tr("Delay line"), QVariant((quint32)FilteringProcessor::DelayLine));
	filterAddCB->addItem(tr("Overrunning line"), QVariant((quint32)FilteringProcessor::OverrunningLine));
	filterAddCB->addItem(tr("RC IIR Filter"), QVariant((quint32)FilteringProcessor::RC_IIR));
	filterAddCB->addItem(tr("CR IIR Filter"), QVariant((quint32)FilteringProcessor::CR_IIR));
	filterAddCB->addItem(tr("CR Reversed IIR Filter"), QVariant((quint32)FilteringProcessor::CR_REV_IIR));
	filterAddCB->addItem(tr("Moving average filter"), QVariant((quint32)FilteringProcessor::MovingAverage));
	filterAddCB->addItem(tr("Trapezoidal shaper"), QVariant((quint32)FilteringProcessor::Trapezoidal));
	filterAddCB->addItem(tr("Gaussian shaper"), QVariant((quint32)FilteringProcessor::Gaussian));
	filterAddCB->addItem(tr("Cusp shaper"), QVariant((quint32)FilteringProcessor::Cusp));

	mainLayout = new QVBoxLayout (this);
	strmLayout = new QHBoxLayout;
	listLayout = new QGridLayout;
	addSLayout = new QHBoxLayout;
	butnLayout = new QHBoxLayout;
	strmLayout->addWidget(streamCB);
	strmLayout->addWidget(applyPB);
	listLayout->addWidget(enableLabel, 0, 0);
	listLayout->addWidget(enableCB, 0, 1);
	addSLayout->addWidget(filterAddCB);
	addSLayout->addWidget(filterAddPB);
	butnLayout->addWidget(acceptPB);
	butnLayout->addWidget(rejectPB);
	mainLayout->addLayout(strmLayout);
	mainLayout->addLayout(listLayout);
	mainLayout->addLayout(addSLayout);
	mainLayout->addLayout(butnLayout);

	filDialogs.push_back(std::shared_ptr<ShaperDialog> (new DelayDialog (this)));
	filDialogs.push_back(std::shared_ptr<ShaperDialog> (new DelayLineDialog (this)));
	filDialogs.push_back(std::shared_ptr<ShaperDialog> (new OverrunLineDialog (this)));
	filDialogs.push_back(std::shared_ptr<ShaperDialog> (new RCIIRDialog (this)));
	filDialogs.push_back(std::shared_ptr<ShaperDialog> (new CRIIRDialog (this)));
	filDialogs.push_back(std::shared_ptr<ShaperDialog> (new CRREVIIRDialog (this)));
	filDialogs.push_back(std::shared_ptr<ShaperDialog> (new MovingAverageDialog (this)));
	filDialogs.push_back(std::shared_ptr<ShaperDialog> (new TrapezoidalDialog (this)));
	filDialogs.push_back(std::shared_ptr<ShaperDialog> (new GaussianDialog (this)));
	filDialogs.push_back(std::shared_ptr<ShaperDialog> (new CuspDialog (this)));
	for (auto& a: filDialogs) a->setModal(true);

	connect (acceptPB, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect (rejectPB, SIGNAL(clicked(bool)), this, SLOT(reject()));
	connect (applyPB, SIGNAL(clicked(bool)), this, SLOT(apply()));
	connect (filterAddPB, SIGNAL(clicked(bool)), this, SLOT(add()));
	connect (enableCB, SIGNAL(stateChanged(int)), this, SLOT(chb_changed()));
}

void FilteringDialog::showEvent(QShowEvent *) {
	update_streams();
	reload_settings();
	update_widgets();
}

void FilteringDialog::update_streams() {
	disconnect (streamCB, SIGNAL(currentIndexChanged(int)), this, SLOT(cb_stream_changed()));
	streamCB->clear();
	for (quint32 i = 0, ie = coreClassPtr->get_input_streams(); i < ie; i++) {
		streamCB->addItem(coreClassPtr->get_input_name(i), QVariant((quint32)i));
	}
	connect (streamCB, SIGNAL(currentIndexChanged(int)), this, SLOT(cb_stream_changed()));
}

void FilteringDialog::reload_settings() {
	filterSettings.resize(0);
	filterSettings.resize(coreClassPtr->get_input_streams());
	filterEnabled.resize(coreClassPtr->get_input_streams());
	for (quint32 i = 0, ie = coreClassPtr->get_input_streams(); i < ie; i++) {
		filterEnabled[i] = coreClassPtr->get_filter_enabled(i);
		quint32 ne = coreClassPtr->get_filters_count(i);
		std::vector<std::shared_ptr<Filter::FilterSettings>>& ref = filterSettings[i];
		while (ref.size() < ne) {
			switch (coreClassPtr->get_filter_id(i, ref.size())) {
				case FilteringProcessor::Delay:
					ref.push_back(std::shared_ptr<Filter::FilterSettings> (new DelayLine::Settings));
					break;
				case FilteringProcessor::DelayLine:
					ref.push_back(std::shared_ptr<Filter::FilterSettings> (new DelayLine::Settings));
					break;
				case FilteringProcessor::OverrunningLine:
					ref.push_back(std::shared_ptr<Filter::FilterSettings> (new OverrunLine::Settings));
					break;
				case FilteringProcessor::RC_IIR:
					ref.push_back(std::shared_ptr<Filter::FilterSettings> (new RC_IIR_Filter::Settings));
					break;
				case FilteringProcessor::CR_IIR:
					ref.push_back(std::shared_ptr<Filter::FilterSettings> (new CR_IIR_Filter::Settings));
					break;
				case FilteringProcessor::CR_REV_IIR:
					ref.push_back(std::shared_ptr<Filter::FilterSettings> (new CR_REV_IIR_Filter::Settings));
					break;
				case FilteringProcessor::MovingAverage:
					ref.push_back(std::shared_ptr<Filter::FilterSettings> (new MovingAverage::Settings));
					break;
				case FilteringProcessor::Trapezoidal:
					ref.push_back(std::shared_ptr<Filter::FilterSettings> (new TrapezoidalShaper::Settings));
					break;
				case FilteringProcessor::Gaussian:
					ref.push_back(std::shared_ptr<Filter::FilterSettings> (new GaussianShaper::Settings));
					break;
				case FilteringProcessor::Cusp:
					ref.push_back(std::shared_ptr<Filter::FilterSettings> (new CuspShaper::Settings));
					break;
				default:
					assert(false);
			}
		}
		for (quint32 n = 0; n < ne; n++) *(ref[n].get()) = *coreClassPtr->filter_get(i, n).get();
	}
}

void FilteringDialog::update_widgets() {
    if (streamCB->count() == 0) return;
	std::vector<std::shared_ptr<Filter::FilterSettings>>& ref = filterSettings[streamCB->currentData().toUInt()];
	while (filterTypeLabel.size() < ref.size()) {
		quint32 sz = filterTypeLabel.size();
		filterTypeLabel.push_back(new QLabel (this));
		filterSetPB.push_back(new QPushButton(tr("Settings"), this));
		filterDelPB.push_back(new QPushButton(tr("Delete"), this));
		listLayout->addWidget(filterTypeLabel[sz], sz+1, 0, 1, 1);
		listLayout->addWidget(filterSetPB[sz], sz+1, 1, 1, 1);
		listLayout->addWidget(filterDelPB[sz], sz+1, 2, 1, 1);
		filterSetPB[sz]->setProperty("", QVariant(sz));
		filterDelPB[sz]->setProperty("", QVariant(sz));
		connect (filterSetPB[sz], SIGNAL(clicked(bool)), this, SLOT(set_filter()));
		connect (filterDelPB[sz], SIGNAL(clicked(bool)), this, SLOT(del_filter()));
	}
	while (filterTypeLabel.size() > ref.size()) {
		quint32 sz = filterTypeLabel.size() - 1;
		delete filterTypeLabel[sz];
		delete filterSetPB[sz];
		delete filterDelPB[sz];
		filterTypeLabel.pop_back();
		filterSetPB.pop_back();
		filterDelPB.pop_back();
	}
	for (quint32 i = 0, ie = filterTypeLabel.size(); i < ie; i++) {
		switch (filterSettings[streamCB->currentData().toUInt()][i]->get_filter_id()) {
			case FilteringProcessor::Delay:
				filterTypeLabel[i]->setText(tr("Delay"));
				break;
			case FilteringProcessor::DelayLine:
				filterTypeLabel[i]->setText(tr("Delay line"));
				break;
			case FilteringProcessor::OverrunningLine:
				filterTypeLabel[i]->setText(tr("Overrun line"));
				break;
			case FilteringProcessor::RC_IIR:
				filterTypeLabel[i]->setText(tr("RC IIR Filter"));
				break;
			case FilteringProcessor::CR_IIR:
				filterTypeLabel[i]->setText(tr("CR IIR Filter"));
				break;
			case FilteringProcessor::CR_REV_IIR:
				filterTypeLabel[i]->setText(tr("CR Reversed IIR Filter"));
				break;
			case FilteringProcessor::MovingAverage:
				filterTypeLabel[i]->setText(tr("Moving average filter"));
				break;
			case FilteringProcessor::Trapezoidal:
				filterTypeLabel[i]->setText(tr("Trapezoidal shaper"));
				break;
			case FilteringProcessor::Gaussian:
				filterTypeLabel[i]->setText(tr("Gaussian shaper"));
				break;
			case FilteringProcessor::Cusp:
				filterTypeLabel[i]->setText(tr("Cusp shaper"));
				break;
			default:
				assert(false);
		}
	}
	if (filterEnabled[streamCB->currentData().toUInt()]) enableCB->setCheckState(Qt::Checked);
	else enableCB->setCheckState(Qt::Unchecked);
}

void FilteringDialog::accept() {
	for (quint32 i = 0, ie = filterSettings.size(); i < ie; i++) {
		while (coreClassPtr->get_filters_count(i) < filterSettings[i].size()) {
			coreClassPtr->add_filter(filterSettings[i][coreClassPtr->get_filters_count(i)]->get_filter_id(), i);
		}
		while (coreClassPtr->get_filters_count(i) > filterSettings[i].size()) {
			coreClassPtr->del_filter (coreClassPtr->get_filters_count(i) - 1, i);
		}
		for (quint32 n = 0, ne = filterSettings[i].size(); n < ne; n++) {
			coreClassPtr->set_filter_id(filterSettings[i][n]->get_filter_id(), i, n);
			coreClassPtr->filter_set(filterSettings[i][n], i, n);
		}
		coreClassPtr->set_filter_enabled(filterEnabled[i], i);
	}
	QDialog::accept();
}

void FilteringDialog::apply() {
	quint32 i = streamCB->currentData().toUInt();
	while (coreClassPtr->get_filters_count(i) < filterSettings[i].size()) {
		coreClassPtr->add_filter(filterSettings[i][coreClassPtr->get_filters_count(i)]->get_filter_id(), i);
	}
	while (coreClassPtr->get_filters_count(i) > filterSettings[i].size()) {
		coreClassPtr->del_filter (coreClassPtr->get_filters_count(i) - 1, i);
	}
	for (quint32 n = 0, ne = filterSettings[i].size(); n < ne; n++) {
		coreClassPtr->set_filter_id(filterSettings[i][n]->get_filter_id(), i, n);
		coreClassPtr->filter_set(filterSettings[i][n], i, n);
	}
	coreClassPtr->set_filter_enabled(filterEnabled[i], i);
}

void FilteringDialog::add() {
	switch (filterAddCB->currentData().toUInt()) {
		case FilteringProcessor::Delay: {
			filterSettings[streamCB->currentData().toUInt()].push_back (std::shared_ptr<Filter::FilterSettings> (new Delay::Settings));
			break;
		} case FilteringProcessor::DelayLine: {
			filterSettings[streamCB->currentData().toUInt()].push_back (std::shared_ptr<Filter::FilterSettings> (new DelayLine::Settings));
			break;
		} case FilteringProcessor::OverrunningLine: {
			filterSettings[streamCB->currentData().toUInt()].push_back (std::shared_ptr<Filter::FilterSettings> (new OverrunLine::Settings));
			break;
		} case FilteringProcessor::RC_IIR: {
			filterSettings[streamCB->currentData().toUInt()].push_back (std::shared_ptr<Filter::FilterSettings> (new RC_IIR_Filter::Settings));
			break;
		} case FilteringProcessor::CR_IIR: {
			filterSettings[streamCB->currentData().toUInt()].push_back (std::shared_ptr<Filter::FilterSettings> (new CR_IIR_Filter::Settings));
			break;
		} case FilteringProcessor::CR_REV_IIR: {
			filterSettings[streamCB->currentData().toUInt()].push_back (std::shared_ptr<Filter::FilterSettings> (new CR_REV_IIR_Filter::Settings));
			break;
		} case FilteringProcessor::MovingAverage: {
			filterSettings[streamCB->currentData().toUInt()].push_back (std::shared_ptr<Filter::FilterSettings> (new MovingAverage::Settings));
			break;
		} case FilteringProcessor::Trapezoidal: {
			filterSettings[streamCB->currentData().toUInt()].push_back (std::shared_ptr<Filter::FilterSettings> (new TrapezoidalShaper::Settings));
			break;
		} case FilteringProcessor::Gaussian: {
			filterSettings[streamCB->currentData().toUInt()].push_back (std::shared_ptr<Filter::FilterSettings> (new GaussianShaper::Settings));
			break;
		} case FilteringProcessor::Cusp: {
			filterSettings[streamCB->currentData().toUInt()].push_back (std::shared_ptr<Filter::FilterSettings> (new CuspShaper::Settings));
			break;
		} default:
			assert(false);
	}
	update_widgets();
}

void FilteringDialog::set_filter() {
	quint32 i = streamCB->currentData().toUInt();
	quint32 n = sender()->property("").toUInt();
	filDialogs[filterSettings[i][n]->get_filter_id()]->set_settings(filterSettings[i][n]);
	filDialogs[filterSettings[i][n]->get_filter_id()]->show();
	filDialogs[filterSettings[i][n]->get_filter_id()]->raise();
	filDialogs[filterSettings[i][n]->get_filter_id()]->activateWindow();
}

void FilteringDialog::del_filter() {
	quint32 i = streamCB->currentData().toUInt();
	quint32 n = sender()->property("").toUInt();
	filterSettings[i].erase(filterSettings[i].begin() + n);
	update_widgets();
}

void FilteringDialog::cb_stream_changed() {
	update_widgets();
}

void FilteringDialog::chb_changed() {
	if (enableCB->checkState() == Qt::Checked) filterEnabled[streamCB->currentData().toUInt()] = true;
	else filterEnabled[streamCB->currentData().toUInt()] = false;
}

ShaperDialog::ShaperDialog (QWidget* parent) : QDialog (parent) {
	infoLabel = new QLabel (this);
	acceptPB = new QPushButton (tr("Accept"), this);
	rejectPB = new QPushButton (tr("Reject"), this);
	mainLayout = new QGridLayout (this);
	connect (acceptPB, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect (rejectPB, SIGNAL(clicked(bool)), this, SLOT(reject()));
}

DelayDialog::DelayDialog (QWidget* parent) : ShaperDialog(parent) {
	infoLabel->setText("Simple delay.");
	delayerTimeLabel = new QLabel (tr("Delay samples"), this);
	delayerTime = new QSpinBox (this);
	delayerTime->setMinimum(1);
	delayerTime->setMaximum(128);
	mainLayout->addWidget(infoLabel, 0, 0, 1, 2);
	mainLayout->addWidget(delayerTimeLabel, 1, 0, 1, 1);
	mainLayout->addWidget(delayerTime, 1, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 3, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 3, 1, 1, 1, Qt::AlignRight);
}

void DelayDialog::update_values() {
	delayerTime->setValue(((Delay::Settings*)settings.get())->time);
}

void DelayDialog::accept() {
	((Delay::Settings*)settings.get())->time = delayerTime->value();
	QDialog::accept();
}

DelayLineDialog::DelayLineDialog (QWidget* parent) : ShaperDialog(parent) {
	infoLabel->setText("This is FIR filter. Impulse responce:<br><img src=:/images/dl.png/>");
	delayerTimeLabel = new QLabel (tr("N"), this);
	delayerAmplLabel = new QLabel (tr("a"), this);
	delayerTime = new QSpinBox (this);
	delayerAmpl = new QDoubleSpinBox (this);
	delayerTime->setMinimum(1);
	delayerTime->setMaximum(32);
	delayerAmpl->setMinimum(-1.);
	delayerAmpl->setMaximum(1.);
	delayerAmpl->setSingleStep(0.01);
	delayerAmpl->setDecimals(3);
	mainLayout->addWidget(infoLabel, 0, 0, 1, 2);
	mainLayout->addWidget(delayerTimeLabel, 1, 0, 1, 1);
	mainLayout->addWidget(delayerTime, 1, 1, 1, 1);
	mainLayout->addWidget(delayerAmplLabel, 2, 0, 1, 1);
	mainLayout->addWidget(delayerAmpl, 2, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 4, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 4, 1, 1, 1, Qt::AlignRight);
}

void DelayLineDialog::update_values() {
	delayerTime->setValue(((DelayLine::Settings*)settings.get())->time);
	delayerAmpl->setValue(((DelayLine::Settings*)settings.get())->ampl);
}

void DelayLineDialog::accept() {
	((DelayLine::Settings*)settings.get())->time = delayerTime->value();
	((DelayLine::Settings*)settings.get())->ampl = delayerAmpl->value();
	QDialog::accept();
}

OverrunLineDialog::OverrunLineDialog (QWidget* parent) : ShaperDialog(parent) {
	infoLabel->setText("This is FIR filter. Impulse responce:<br><img src=:/images/ol.png/>");
	overrunTimeLabel = new QLabel (tr("N"), this);
	overrunAmplLabel = new QLabel (tr("a"), this);
	overrunTime = new QSpinBox (this);
	overrunAmpl = new QDoubleSpinBox (this);
	overrunTime->setMinimum(1);
	overrunTime->setMaximum(32);
	overrunAmpl->setMinimum(-1.);
	overrunAmpl->setMaximum(1.);
	overrunAmpl->setSingleStep(0.01);
	overrunAmpl->setDecimals(3);
	mainLayout->addWidget(infoLabel, 0, 0, 1, 2);
	mainLayout->addWidget(overrunTimeLabel, 1, 0, 1, 1);
	mainLayout->addWidget(overrunTime, 1, 1, 1, 1);
	mainLayout->addWidget(overrunAmplLabel, 2, 0, 1, 1);
	mainLayout->addWidget(overrunAmpl, 2, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 4, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 4, 1, 1, 1, Qt::AlignRight);
}

void OverrunLineDialog::update_values() {
	overrunTime->setValue(((OverrunLine::Settings*)settings.get())->time);
	overrunAmpl->setValue(((OverrunLine::Settings*)settings.get())->ampl);
}

void OverrunLineDialog::accept() {
	((OverrunLine::Settings*)settings.get())->time = overrunTime->value();
	((OverrunLine::Settings*)settings.get())->ampl = overrunAmpl->value();
	QDialog::accept();
}

RC_CR_Dialog::RC_CR_Dialog (QWidget *parent) : ShaperDialog (parent) {
	tauConstLabel = new QLabel (tr("Alpha"), this);
	tauConstDSB = new QDoubleSpinBox(this);
	tauConstDSB->setMinimum(0.001);
	tauConstDSB->setMaximum(1.);
	tauConstDSB->setDecimals(3);
	tauConstDSB->setSingleStep(0.001);
	mainLayout->addWidget(infoLabel, 0, 0, 1, 2);
	mainLayout->addWidget(tauConstLabel, 1, 0, 1, 1);
	mainLayout->addWidget(tauConstDSB, 1, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 3, 0, 1, 1);
	mainLayout->addWidget(rejectPB, 3, 1, 1, 1);
}

RCIIRDialog::RCIIRDialog(QWidget* parent) : RC_CR_Dialog (parent) {
	infoLabel->setText("This is IIR filter. Impulse responce:<br><img src=:/images/rc.png/><br>t - time between samples");
}

void RCIIRDialog::update_values() {
	tauConstDSB->setValue(((RC_IIR_Filter::Settings*)settings.get())->alpha);
}

void RCIIRDialog::accept() {
	((RC_IIR_Filter::Settings*)settings.get())->alpha = tauConstDSB->value();
	QDialog::accept();
}

CRIIRDialog::CRIIRDialog(QWidget* parent) : RC_CR_Dialog (parent) {
	infoLabel->setText("This is IIR filter. Impulse responce:<br><img src=:/images/cr.png/><br>t - time between samples");
}

void CRIIRDialog::update_values() {
	tauConstDSB->setValue(((CR_IIR_Filter::Settings*)settings.get())->alpha);
}

void CRIIRDialog::accept() {
	((CR_IIR_Filter::Settings*)settings.get())->alpha = tauConstDSB->value();
	QDialog::accept();
}

CRREVIIRDialog::CRREVIIRDialog(QWidget* parent) : RC_CR_Dialog (parent) {
	infoLabel->setText("This is IIR filter. Restores signal after CR filter.\nAlpha is simular to alpha in CR filter.");
}

void CRREVIIRDialog::update_values() {
	tauConstDSB->setValue(((CR_REV_IIR_Filter::Settings*)settings.get())->alpha);
}

void CRREVIIRDialog::accept() {
	((CR_REV_IIR_Filter::Settings*)settings.get())->alpha = tauConstDSB->value();
	QDialog::accept();
}

MovingAverageDialog::MovingAverageDialog (QWidget* parent) : ShaperDialog (parent) {
	infoLabel->setText("This is FIR filter. Impulse responce:<br><img src=:/images/ma.png/>");
	sizeLabel = new QLabel (tr("N"), this);
	size = new QSpinBox (this);
	size->setMinimum(1);
	size->setMaximum(32);
	mainLayout->addWidget(infoLabel, 0, 0, 1, 2);
	mainLayout->addWidget(sizeLabel, 1, 0, 1, 1);
	mainLayout->addWidget(size, 1, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 4, 0, 1, 1);
	mainLayout->addWidget(rejectPB, 4, 1, 1, 1);
}

void MovingAverageDialog::update_values() {
	size->setValue(((MovingAverage::Settings*)settings.get())->size);
}

void MovingAverageDialog::accept() {
	((MovingAverage::Settings*)settings.get())->size = size->value();
	QDialog::accept();
}

TrapezoidalDialog::TrapezoidalDialog (QWidget* parent) : ShaperDialog (parent) {
	infoLabel->setText("This is FIR filter. Impulse responce:<br><img src=:/images/trz.png/>");
	sideLabel = new QLabel (tr("N"), this);
	flatLabel = new QLabel (tr("M"), this);
	side = new QSpinBox (this);
	flat = new QSpinBox (this);
	side->setMinimum(1);
	side->setMaximum(32);
	flat->setMinimum(0);
	flat->setMaximum(64);
	mainLayout->addWidget(infoLabel, 0, 0, 1, 2);
	mainLayout->addWidget(sideLabel, 1, 0, 1, 1);
	mainLayout->addWidget(side, 1, 1, 1, 1);
	mainLayout->addWidget(flatLabel, 2, 0, 1, 1);
	mainLayout->addWidget(flat, 2, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 4, 0, 1, 1);
	mainLayout->addWidget(rejectPB, 4, 1, 1, 1);
}

void TrapezoidalDialog::update_values() {
	side->setValue(((TrapezoidalShaper::Settings*)settings.get())->side);
	flat->setValue(((TrapezoidalShaper::Settings*)settings.get())->flat);
}

void TrapezoidalDialog::accept() {
	((TrapezoidalShaper::Settings*)settings.get())->side = side->value();
	((TrapezoidalShaper::Settings*)settings.get())->flat = flat->value();
	QDialog::accept();
}

GaussianDialog::GaussianDialog (QWidget* parent) : ShaperDialog (parent) {
	infoLabel->setText("This is FIR filter. Impulse responce:<br><img src=:/images/gauss.png/>");
	widthLabel = new QLabel (tr("W"), this);
	width = new QDoubleSpinBox (this);
	width->setMinimum(0.5);
	width->setMaximum(20);
	mainLayout->addWidget(infoLabel, 0, 0, 1, 2);
	mainLayout->addWidget(widthLabel, 1, 0, 1, 1);
	mainLayout->addWidget(width, 1, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 4, 0, 1, 1);
	mainLayout->addWidget(rejectPB, 4, 1, 1, 1);
}

void GaussianDialog::update_values() {
	width->setValue(((GaussianShaper::Settings*)settings.get())->width);
}

void GaussianDialog::accept() {
	((GaussianShaper::Settings*)settings.get())->width = width->value();
	QDialog::accept();
}

CuspDialog::CuspDialog (QWidget* parent) : ShaperDialog (parent) {
	infoLabel->setText("This is FIR filter. Impulse responce:<br><img src=:/images/cusp.png/>");
	widthLabel = new QLabel (tr("W"), this);
	flatLabel = new QLabel (tr("M"), this);
	width = new QDoubleSpinBox (this);
	flat = new QSpinBox (this);
	width->setMinimum(0.5);
	width->setMaximum(20.);
	flat->setMinimum(0);
	flat->setMaximum(64);
	mainLayout->addWidget(infoLabel, 0, 0, 1, 2);
	mainLayout->addWidget(widthLabel, 1, 0, 1, 1);
	mainLayout->addWidget(width, 1, 1, 1, 1);
	mainLayout->addWidget(flatLabel, 2, 0, 1, 1);
	mainLayout->addWidget(flat, 2, 1, 1, 1);
	mainLayout->addWidget(acceptPB, 4, 0, 1, 1);
	mainLayout->addWidget(rejectPB, 4, 1, 1, 1);
}

void CuspDialog::update_values() {
	width->setValue(((CuspShaper::Settings*)settings.get())->width);
	flat->setValue(((CuspShaper::Settings*)settings.get())->flat);
}

void CuspDialog::accept() {
	((CuspShaper::Settings*)settings.get())->width = width->value();
	((CuspShaper::Settings*)settings.get())->flat = flat->value();
	QDialog::accept();
}

