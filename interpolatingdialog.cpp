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

#include "interpolatingdialog.hpp"

InterpolatingDialog::InterpolatingDialog(Core* _core, QWidget* parent) : QDialog (parent) {
	CoreClassPtr = _core;

	interEnabledLab = new QLabel (tr("Interpolaion enabled"), this);
	interEnabledCB = new QCheckBox(this);
	interTypeLab = new QLabel (tr("Interpolaion type"), this);
	interTypeCB = new QComboBox (this);
	interTypeCB->addItem(tr("Whittaker-Shannon"), InterpolationClass::WhittakerShannon);
	interMultLab = new QLabel (tr("Points multiplication"), this);
	interMultCB = new QComboBox (this);
	interMultCB->addItem(tr("2"), QVariant(2u));
	interMultCB->addItem(tr("4"), QVariant(4u));
	interMultCB->addItem(tr("8"), QVariant(8u));
	interMultCB->addItem(tr("16"), QVariant(16u));
	interPrecLab = new QLabel (tr("Interpolaion precision"), this);
	interPrecCB = new QComboBox (this);
	interPrecCB->addItem(tr("4"), QVariant(4u));
	interPrecCB->addItem(tr("8"), QVariant(8u));
	interPrecCB->addItem(tr("16"), QVariant(16u));
	interPrecCB->addItem(tr("32"), QVariant(32u));
	interPrecCB->addItem(tr("64"), QVariant(64u));
	interPrecCB->addItem(tr("128"), QVariant(128u));
	artReducLab = new QLabel (tr("Artifact reduction"), this);
	artReducCB = new QCheckBox (this);

	acceptPB = new QPushButton (tr("Accept"), this);
	rejectPB = new QPushButton (tr("Reject"), this);
	buttonsLayout = new QHBoxLayout;
	buttonsLayout->addWidget(acceptPB, 0, Qt::AlignLeft);
	buttonsLayout->addWidget(rejectPB, 0, Qt::AlignRight);
	settingsLayout = new QGridLayout;
	settingsLayout->addWidget(interEnabledLab, 0, 0);
	settingsLayout->addWidget(interEnabledCB, 0, 1);
	settingsLayout->addWidget(interTypeLab, 1, 0);
	settingsLayout->addWidget(interTypeCB, 1, 1);
	settingsLayout->addWidget(interMultLab, 2, 0);
	settingsLayout->addWidget(interMultCB, 2, 1);
	settingsLayout->addWidget(interPrecLab, 3, 0);
	settingsLayout->addWidget(interPrecCB, 3, 1);
	settingsLayout->addWidget(artReducLab, 4, 0);
	settingsLayout->addWidget(artReducCB, 4, 1);

	mainLayout = new QVBoxLayout (this);
	mainLayout->addLayout(settingsLayout);
	mainLayout->addLayout(buttonsLayout);

	connect(acceptPB, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect(rejectPB, SIGNAL(clicked(bool)), this, SLOT(reject()));
}

InterpolatingDialog::~InterpolatingDialog () {

}

void InterpolatingDialog::reload_settings() {
	InterpolatorSettings t = CoreClassPtr->get_inter_settings();
	if (CoreClassPtr->get_inter_enabled()) interEnabledCB->setCheckState(Qt::Checked);
	else interEnabledCB->setCheckState(Qt::Unchecked);
	interTypeCB->setCurrentIndex(get_index(interTypeCB->findData(QVariant(CoreClassPtr->get_inter_type()))));
	interMultCB->setCurrentIndex(get_index(interMultCB->findData(QVariant(t.pointsMult))));
	interPrecCB->setCurrentIndex(get_index(interPrecCB->findData(QVariant(t.precision))));
	if (t.artifactRedution) artReducCB->setCheckState(Qt::Checked);
	else artReducCB->setCheckState(Qt::Unchecked);
}

void InterpolatingDialog::showEvent(QShowEvent *_event) {
	Q_UNUSED(_event);
	reload_settings();
}

void InterpolatingDialog::accept() {
	InterpolatorSettings t;
	CoreClassPtr->set_inter_enabled(interEnabledCB->checkState());
	CoreClassPtr->set_inter_type(interTypeCB->currentData().toUInt());
	t.pointsMult = interMultCB->currentData().toUInt();
	t.precision = interPrecCB->currentData().toUInt();
	t.artifactRedution = artReducCB->checkState();
	CoreClassPtr->set_inter_settings(t);
	QDialog::accept();
}

void InterpolatingDialog::reject() {
	QDialog::reject();
}
