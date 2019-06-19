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

#include "autosavedialog.hpp"
#include "mainwindow.hpp"

AutosaveDialog::AutosaveDialog(MainWindow* parent) : QDialog (parent) {
	mainPtr = parent;
	enableLabel = new QLabel(tr("Enable"), this);
	enableChB = new QCheckBox (this);
	intervalLabel = new QLabel (tr("Interval (min)"), this);
	intervalSB = new QSpinBox (this);
	filenameLabel = new QLabel (tr("Name template"), this);
	filenameEdit = new QLineEdit (this);
	acceptPB = new QPushButton (tr("Accept"), this);
	rejectPB = new QPushButton (tr("Reject"), this);
	mainLayout = new QGridLayout (this);
	intervalSB->setMinimum(1);
	intervalSB->setMaximum(24*60);

	mainLayout->addWidget(enableLabel, 0, 0);
	mainLayout->addWidget(enableChB, 0, 1, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(intervalLabel, 1, 0);
	mainLayout->addWidget(intervalSB, 1, 1);
	mainLayout->addWidget(filenameLabel, 2, 0);
	mainLayout->addWidget(filenameEdit, 2, 1);
	mainLayout->addWidget(acceptPB, 3, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 3, 1, 1, 1, Qt::AlignRight);

	connect (acceptPB, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect (rejectPB, SIGNAL(clicked(bool)), this, SLOT(reject()));

}

void AutosaveDialog::showEvent(QShowEvent *) {
	if (mainPtr->enableASave) enableChB->setCheckState(Qt::Checked);
	else enableChB->setCheckState(Qt::Unchecked);
	intervalSB->setValue(mainPtr->intervalASave/60);
	filenameEdit->setText(mainPtr->filenameASave.c_str());
}

void AutosaveDialog::accept() {
	if (enableChB->checkState() == Qt::Checked)
		mainPtr->set_autosave(intervalSB->value()*60, filenameEdit->text().toStdString(), true);
	else
		mainPtr->set_autosave(intervalSB->value()*60, filenameEdit->text().toStdString(), false);
	QDialog::accept();
}

