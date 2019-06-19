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

#include "streamsmanagerdialog.hpp"
#include "processingsettings.hpp"

StreamsManagerDialog::StreamsManagerDialog(Core* _ptr, QWidget* _parent) : QDialog(_parent) {
	corePtr = _ptr;

	inptLayout = new QVBoxLayout;
	procLayout = new QGridLayout;
	widgLayout = new QHBoxLayout;
	butnLayout = new QHBoxLayout;
	mainLayout = new QVBoxLayout(this);

	inputNamesLabel = new QLabel (tr("Input name"), this);
	procTypeLabel = new QLabel (tr("Process type"), this);
	procNameLabel = new QLabel (tr("Process name"), this);

	acceptPB = new QPushButton (tr("Accept"), this);
	rejectPB = new QPushButton (tr("Reject"), this);

	addPB = new QPushButton (tr("Add"), this);
	delPB = new QPushButton (tr("Delete"), this);

	butnLayout->addWidget(acceptPB, 0, Qt::AlignLeft);
	butnLayout->addWidget(rejectPB, 0, Qt::AlignRight);

	widgLayout->addLayout(inptLayout);
	widgLayout->addLayout(procLayout);
	mainLayout->addLayout(widgLayout);
	mainLayout->addLayout(butnLayout);

	inptLayout->addWidget(inputNamesLabel, 0, Qt::AlignCenter | Qt::AlignTop);
	procLayout->addWidget(procTypeLabel, 0, 0, 1, 1, Qt::AlignCenter | Qt::AlignTop);
	procLayout->addWidget(procNameLabel, 0, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);

	connect (acceptPB, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect (rejectPB, SIGNAL(clicked(bool)), this, SLOT(reject()));
	connect (addPB, SIGNAL(clicked(bool)), this, SLOT(add_proc()));
	connect (delPB, SIGNAL(clicked(bool)), this, SLOT(del_proc()));
}

void StreamsManagerDialog::update_input_streams() {
	for (auto& a : inputsNames) delete a;
	inputsNames.resize(corePtr->get_input_streams());
	for (quint32 i = 0, ie = corePtr->get_input_streams(); i < ie; ++i) {
		inputsNames[i] = new QLineEdit (corePtr->get_input_name(i), this);
		inptLayout->addWidget(inputsNames[i], 1, Qt::AlignTop);
	}
	inptLayout->setStretch(inputsNames.size(), 1);
}

void StreamsManagerDialog::update_processing_streams() {
	for (auto& a : procTypeWidgets) delete a;
	for (auto& a : procNameWidgets) delete a;
	procTypeWidgets.clear();
	procNameWidgets.clear();
	for (quint32 i = 0, ie = corePtr->get_process_threads(); i < ie; ++i) {
		procTypeWidgets.push_back(new QComboBox(this));
		procNameWidgets.push_back(new QLineEdit(corePtr->get_process_name(i), this));
		procLayout->addWidget(procTypeWidgets[i], i+1, 0, 1, 1, Qt::AlignTop);
		procLayout->addWidget(procNameWidgets[i], i+1, 1, 1, 1, Qt::AlignTop);
		cb_add_items(procTypeWidgets[i]);
		procTypeWidgets[i]->setCurrentIndex(procTypeWidgets[i]->findData(QVariant(corePtr->get_process_settings(i)->get_settings_id())));
	}
	procLayout->addWidget(addPB, procTypeWidgets.size()+1, 0);
	procLayout->addWidget(delPB, procTypeWidgets.size()+1, 1);
	procLayout->setRowStretch(procTypeWidgets.size()+2, 1);
}

void StreamsManagerDialog::cb_add_items(QComboBox* box) {
	box->addItem(tr("Standart circuit"), QVariant(ProcessingThread::StandartCircuit));
	box->addItem(tr("Coincidence circuit"), QVariant(ProcessingThread::CoincidenceCircuit));
}

void StreamsManagerDialog::showEvent(QShowEvent *) {
	update_input_streams();
	update_processing_streams();
}

void StreamsManagerDialog::accept() {
	for (quint32 i = 0, ie = inputsNames.size(); i < ie; ++i)
		corePtr->set_input_name(inputsNames[i]->text(), i);
	corePtr->set_process_threads(procNameWidgets.size());
	for (quint32 i = 0, ie = procNameWidgets.size(); i < ie; ++i) {
		if (corePtr->get_process_settings(i)->get_settings_id() != procTypeWidgets[i]->currentData().toUInt())
			corePtr->set_process_type(procTypeWidgets[i]->currentData().toUInt(), i);
		corePtr->set_process_name(procNameWidgets[i]->text(), i);
	}
	QDialog::accept();
}

void StreamsManagerDialog::reject() {
	QDialog::reject();
}

void StreamsManagerDialog::add_proc() {
	std::stringstream sstr;
	quint32 i = procNameWidgets.size();
	sstr << "Spectrum_" << i;
	procTypeWidgets.push_back(new QComboBox(this));
	procNameWidgets.push_back(new QLineEdit((sstr.str().c_str()), this));
	procLayout->addWidget(addPB, procTypeWidgets.size()+1, 0);
	procLayout->addWidget(delPB, procTypeWidgets.size()+1, 1);
	procLayout->addWidget(procTypeWidgets[i], i+1, 0, 1, 1, Qt::AlignTop);
	procLayout->addWidget(procNameWidgets[i], i+1, 1, 1, 1, Qt::AlignTop);
	procLayout->setRowStretch(i+2, 1);
	cb_add_items(procTypeWidgets[i]);
}

void StreamsManagerDialog::del_proc() {
	quint32 i = procNameWidgets.size()-1;
	delete procTypeWidgets[i];
	delete procNameWidgets[i];
	procTypeWidgets.pop_back();
	procNameWidgets.pop_back();
	procLayout->addWidget(addPB, procTypeWidgets.size()+1, 0);
	procLayout->addWidget(delPB, procTypeWidgets.size()+1, 1);
	procLayout->setRowStretch(procTypeWidgets.size(), 1);
}

