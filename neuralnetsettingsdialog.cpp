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

#include "neuralnetsettingsdialog.hpp"
#include "processingsettings.hpp"

NeuralNetSettingsDialog::NeuralNetSettingsDialog(Core* _corePtr, QWidget* _parent) :
	QDialog (_parent), coreClassPtr (_corePtr) {
	tabWidget = new QTabWidget (this);
	acceptPB = new QPushButton (tr("Accept"), this);
	rejectPB = new QPushButton (tr("Reject"), this);
	buttonsLayout = new QHBoxLayout;
	mainLayout = new QVBoxLayout (this);
	buttonsLayout->addWidget(acceptPB, 0, Qt::AlignLeft);
	buttonsLayout->addWidget(rejectPB, 1, Qt::AlignRight);
	mainLayout->addWidget(tabWidget);
	mainLayout->addLayout(buttonsLayout);
	connect (acceptPB, SIGNAL (clicked(bool)), this, SLOT (accept()));
	connect (rejectPB, SIGNAL (clicked(bool)), this, SLOT (reject()));

	quint32 rowIndex;

	nnShowTab = new QWidget(tabWidget);
	currNNL = new QLabel (tr("Function:") , nnShowTab);
	currNNCB = new QComboBox (nnShowTab);
	currNumNNL = new QLabel (tr("Selected:") , nnShowTab);
	currNumNNCB = new QComboBox (nnShowTab);
	inputsL = new QLabel ("Inputs:", nnShowTab);
	inputsI = new QLabel (nnShowTab);
	totLayersL = new QLabel ("Layers:", nnShowTab);
	totLayersI = new QLabel (nnShowTab);
	totNeuronsL = new QLabel ("Neurons:", nnShowTab);
	totNeuronsI = new QLabel (nnShowTab);
	totWeightsL = new QLabel ("Weights:", nnShowTab);
	totWeightsI = new QLabel (nnShowTab);
	addNNPB = new QPushButton (tr("Add"), this);
	delNNPB = new QPushButton (tr("Delete"), this);
	saveNNPB = new QPushButton (tr("Save"), this);
	loadNNPB = new QPushButton (tr("Load"), this);
	nnShowTabGrid = new QGridLayout (nnShowTab);
	nnAddDial = new NeuralNetAddDialog (this);
	nnAddDial->setModal(true);
	nnShowTabGrid->addWidget(currNNL, 0, 0, 1, 1);
	nnShowTabGrid->addWidget(currNNCB, 0, 1, 1, 1, Qt::AlignLeft);
	nnShowTabGrid->addWidget(currNumNNL, 1, 0, 1, 1);
	nnShowTabGrid->addWidget(currNumNNCB, 1, 1, 1, 1, Qt::AlignLeft);
	nnShowTabGrid->addWidget(inputsL, 2, 0, 1, 1);
	nnShowTabGrid->addWidget(inputsI, 2, 1, 1, 1, Qt::AlignLeft);
	nnShowTabGrid->addWidget(totLayersL, 3, 0, 1, 1);
	nnShowTabGrid->addWidget(totLayersI, 3, 1, 1, 1, Qt::AlignLeft);
	nnShowTabGrid->addWidget(totNeuronsL, 4, 0, 1, 1);
	nnShowTabGrid->addWidget(totNeuronsI, 4, 1, 1, 1, Qt::AlignLeft);
	nnShowTabGrid->addWidget(totWeightsL, 5, 0, 1, 1);
	nnShowTabGrid->addWidget(totWeightsI, 5, 1, 1, 1, Qt::AlignLeft);
	nnShowTabGrid->addWidget(addNNPB, 6, 0, 1, 1);
	nnShowTabGrid->addWidget(delNNPB, 6, 1, 1, 1);
	nnShowTabGrid->addWidget(saveNNPB, 7, 0, 1, 1);
	nnShowTabGrid->addWidget(loadNNPB, 7, 1, 1, 1);
	nnShowTabGrid->setRowStretch(8, 1);
	connect(currNNCB, SIGNAL(currentIndexChanged(int)), this, SLOT(update_nn_info()));
	connect(currNumNNCB, SIGNAL(currentIndexChanged(int)), this, SLOT(update_show_widget()));
	connect(addNNPB, SIGNAL(clicked(bool)), this, SLOT(add_nn()));
	connect(delNNPB, SIGNAL(clicked(bool)), this, SLOT(del_nn()));
	connect(saveNNPB, SIGNAL(clicked(bool)), this, SLOT(save_nn()));
	connect(loadNNPB, SIGNAL(clicked(bool)), this, SLOT(load_nn()));

	currNNCB->addItem(tr(AvailableFunctions::FunctionName[AvailableFunctions::Amplitude]), QVariant(AvailableFunctions::Amplitude));
	currNNCB->addItem(tr(AvailableFunctions::FunctionName[AvailableFunctions::Time]), QVariant(AvailableFunctions::Time));
	currNNCB->addItem(tr(AvailableFunctions::FunctionName[AvailableFunctions::Discriminating]), QVariant(AvailableFunctions::Discriminating));

	teachTabWidget = new QWidget (tabWidget);
	infoLabel = new QLabel (tr("Here neural networks is teached.\n"
							   "Neural networks can be selected in Type and Neural nets box.\n"
							   "In Noize Settings dialog one can adjust noize for teaching pulses.\n"
							   "Number of cycles is a value how long neural network will be teached.\n"
							   "Teaching size is a size of teaching pulses buffer.\n"
							   "For teaching pulse shape must be defined.\n Shape is a base for teaching pulses."), this);

	functionLabel = new QLabel (tr("Type:"), teachTabWidget);
	functionCB = new QComboBox (teachTabWidget);
	functionCB->addItem(tr("Amplitude measuring"), QVariant(AmplitudeMeasuring));
	functionCB->addItem(tr("Time measuring"), QVariant(TimeMeasuring));
	functionCB->addItem(tr("Discriminating"), QVariant(Discriminating));

	availNNLabel = new QLabel (tr("Neural nets:"), this);
	availNNCB = new QComboBox (this);

	noizeSettingsPB = new QPushButton (tr("Noize settings"), this);
	teachCyclesLabel = new QLabel (tr("Number of cycles:"), teachTabWidget);
	teachCyclesSB = new QSpinBox (teachTabWidget);
	teachCyclesSB->setMinimum(1);
	teachCyclesSB->setMaximum(50);

	teachSizeLabel = new QLabel (tr("Teaching size"), teachTabWidget);
	teachSizeCB = new QComboBox (teachTabWidget);
	teachSizeCB->addItem("4k",	QVariant((quint32)0x1000));
	teachSizeCB->addItem("8k",	QVariant((quint32)0x2000));
	teachSizeCB->addItem("16k",	QVariant((quint32)0x4000));
	teachSizeCB->addItem("32k",	QVariant((quint32)0x8000));
	teachSizeCB->addItem("64k",	QVariant((quint32)0x10000));
	teachSizeCB->addItem("128k",	QVariant((quint32)0x20000));
	teachSizeCB->addItem("256k",	QVariant((quint32)0x40000));
	teachSizeCB->addItem("512k",	QVariant((quint32)0x80000));
	teachSizeCB->addItem("1M",		QVariant((quint32)0x100000));

	pulShapeL = new QLabel (tr("Status:"), teachTabWidget);
	pulShapeI = new QLabel (teachTabWidget);
	shapeStreamL = new QLabel (tr("Shape for teaching"), teachTabWidget);
	shapeStreamCB = new QComboBox (teachTabWidget);
	startTeachingPB = new QPushButton (tr("Start teaching"), teachTabWidget);

	teachLayout = new QGridLayout (teachTabWidget);
	teachLayout->addWidget(infoLabel, (rowIndex = 0), 0, 1, 2);
	teachLayout->addWidget(functionLabel, ++rowIndex, 0);
	teachLayout->addWidget(functionCB, rowIndex, 1);
	teachLayout->addWidget(availNNLabel, ++rowIndex, 0);
	teachLayout->addWidget(availNNCB, rowIndex, 1);
	teachLayout->addWidget(noizeSettingsPB, ++rowIndex, 0, 1, 2);
	teachLayout->addWidget(teachCyclesLabel, ++rowIndex, 0);
	teachLayout->addWidget(teachCyclesSB, rowIndex, 1);
	teachLayout->addWidget(teachSizeLabel, ++rowIndex, 0);
	teachLayout->addWidget(teachSizeCB, rowIndex, 1);
	teachLayout->addWidget(pulShapeL, ++rowIndex, 0);
	teachLayout->addWidget(pulShapeI, rowIndex, 1);
	teachLayout->addWidget(shapeStreamL, ++rowIndex, 0);
	teachLayout->addWidget(shapeStreamCB, rowIndex, 1);
	teachLayout->addWidget(startTeachingPB, ++rowIndex, 0, 1, 2);
	teachLayout->setRowStretch(++rowIndex, 1);

	teachingWaitDialog = new NeuralNetTeachingWaitingDialog (&nucTeachAmpl, this);
	noizSetDial = new NoizeSettingsDialog (this);

	connect (functionCB, SIGNAL(currentIndexChanged(int)), this, SLOT (func_changed()));
	connect (availNNCB, SIGNAL(currentIndexChanged(int)), this, SLOT (update_teach_widget()));
	connect (noizeSettingsPB, SIGNAL(clicked(bool)), this, SLOT (showNoizeDialog()));
	connect (startTeachingPB, SIGNAL(clicked(bool)), this, SLOT (startTeaching()));

	tabWidget->addTab(nnShowTab, tr("Show"));
	tabWidget->addTab(teachTabWidget, tr("Teach"));

	connect (tabWidget, SIGNAL(currentChanged(int)), SLOT (update_nn_info()));
	connect (tabWidget, SIGNAL(currentChanged(int)), SLOT (func_changed()));

}

NeuralNetSettingsDialog::~NeuralNetSettingsDialog() {

}

Neural_Network::NuclearPhysicsNeuralNet& NeuralNetSettingsDialog::get_selected(QComboBox *type, QComboBox *num) {
	switch (type->currentData().toUInt()) {
		case AvailableFunctions::Amplitude: return nnAmplVec[num->currentIndex()];
		case AvailableFunctions::Time: return nnTimeVec[num->currentIndex()];
		case AvailableFunctions::Discriminating: return nnDiscrVec[num->currentIndex()];
		default: assert(false);
	}
}

std::vector<Neural_Network::NuclearPhysicsNeuralNet>* NeuralNetSettingsDialog::get_nnvector(QComboBox *type) {
	switch (type->currentData().toUInt()) {
		case AvailableFunctions::Amplitude: return &nnAmplVec;
		case AvailableFunctions::Time: return &nnTimeVec;
		case AvailableFunctions::Discriminating: return &nnDiscrVec;
		default: assert(false);
	}
	return 0;
}

void NeuralNetSettingsDialog::showEvent(QShowEvent *_event) {
	Q_UNUSED(_event);
	shapeStreamCB->clear();
	for (quint32 n = 0; n < coreClassPtr->get_process_threads(); n++) {
		shapeStreamCB->addItem(coreClassPtr->get_process_name(n));
	}
	nnDiscrVec = coreClassPtr->get_discr_nn();
	nnAmplVec = coreClassPtr->get_ampl_nn();
	nnTimeVec = coreClassPtr->get_time_nn();

	update_nn_info();
	func_changed();
}

void NeuralNetSettingsDialog::update_show_widget() {
	std::stringstream tStrs;

	auto curr = get_nnvector(currNNCB);

	if (curr->size() == 0) {
		inputsI->setText("--");
		totLayersI->setText("--");
		totNeuronsI->setText("--");
		totWeightsI->setText("--");
		saveNNPB->setEnabled(false);
		delNNPB->setEnabled(false);
		return;
	} else {
		delNNPB->setEnabled(true);
	}

	Neural_Network::NuclearPhysicsNeuralNet& nn = get_selected(currNNCB, currNumNNCB);
	if (nn.is_empty()) {
		inputsI->setText("--");
		totLayersI->setText("--");
		totNeuronsI->setText("--");
		totWeightsI->setText("--");
		saveNNPB->setEnabled(false);
	} else {
		tStrs << nn.inputs();
		inputsI->setText(QString::fromUtf8(tStrs.str().c_str()));
		tStrs.str(std::string());
		tStrs << nn.layers();
		totLayersI->setText(QString::fromUtf8(tStrs.str().c_str()));
		tStrs.str(std::string());
		tStrs << nn.total_neurons();
		totNeuronsI->setText(QString::fromUtf8(tStrs.str().c_str()));
		tStrs.str(std::string());
		tStrs << nn.total_weights();
		totWeightsI->setText(QString::fromUtf8(tStrs.str().c_str()));
		tStrs.str(std::string());
		saveNNPB->setEnabled(true);
	}
}

void NeuralNetSettingsDialog::update_teach_widget() {
	ProcessingThread::Settings const* tmp = coreClassPtr->get_process_settings(shapeStreamCB->currentIndex());

	auto vec = get_nnvector(functionCB);

	bool isValid = true;
	if (vec->size()) {
		Neural_Network::NuclearPhysicsNeuralNet& nn = get_selected(functionCB, availNNCB);

		if (tmp->shape.size() != tmp->pulseSize) {
			pulShapeI->setText("not loaded.");
			isValid = false;
		} else if (tmp->shape.size() != nn.inputs()) {
			pulShapeI->setText("loaded, but size not equal to inputs.");
			isValid = false;
		} else {
			pulShapeI->setText("loaded.");
		}
		if (nn.is_empty()) isValid = false;
	} else {
		pulShapeI->setText("no neural net defined.");
		isValid = false;
	}

	startTeachingPB->setEnabled(isValid);
}

void NeuralNetSettingsDialog::update_nn_info () {
	disconnect (currNumNNCB, SIGNAL(currentIndexChanged(int)), this, SLOT(update_show_widget()));

	std::vector<Neural_Network::NuclearPhysicsNeuralNet>* nnVecPtr = get_nnvector(currNNCB);
	if (nnVecPtr->size() == 0) delNNPB->setEnabled(false);
	else delNNPB->setEnabled(true);

	currNumNNCB->clear();
	for (quint32 i = 0, ie = nnVecPtr->size(); i < ie; ++i) {
		currNumNNCB->addItem((*nnVecPtr)[i].get_name());
	}
	update_show_widget();

	connect (currNumNNCB, SIGNAL(currentIndexChanged(int)), this, SLOT(update_show_widget()));
}

void NeuralNetSettingsDialog::func_changed() {
	disconnect (availNNCB, SIGNAL(currentIndexChanged(int)), this, SLOT (update_teach_widget()));

	auto nnVecPtr = get_nnvector(functionCB);
	availNNCB->clear();
	for (quint32 i = 0, ie = nnVecPtr->size(); i < ie; ++i) {
		availNNCB->addItem((*nnVecPtr)[i].get_name());
	}
	update_teach_widget();

	connect (availNNCB, SIGNAL(currentIndexChanged(int)), this, SLOT (update_teach_widget()));
}

void NeuralNetSettingsDialog::update_values() {
}

void NeuralNetSettingsDialog::add_nn() {

	nnAddDial->show();
	nnAddDial->raise();
	nnAddDial->activateWindow();

	nnAddDial->exec();
	if (nnAddDial->get_status()) {
		auto tmp = nnAddDial->get_nn();

		switch (tmp.get_function(0)) {
			case AvailableFunctions::Amplitude:
				nnAmplVec.push_back(Neural_Network::NuclearPhysicsNeuralNet(tmp));
				break;
			case AvailableFunctions::Time:
				nnTimeVec.push_back(Neural_Network::NuclearPhysicsNeuralNet(tmp));
				break;
			case AvailableFunctions::Discriminating:
				nnDiscrVec.push_back(Neural_Network::NuclearPhysicsNeuralNet(tmp));
				break;
			default:
				assert(false);
		}
		update_nn_info();
	}
}

void NeuralNetSettingsDialog::del_nn() {
	switch (currNNCB->currentData().toUInt()) {
		case AvailableFunctions::Amplitude:
			nnAmplVec.erase(nnAmplVec.begin() + currNumNNCB->currentIndex());
			break;
		case AvailableFunctions::Time:
			nnTimeVec.erase(nnTimeVec.begin() + currNumNNCB->currentIndex());
			break;
		case AvailableFunctions::Discriminating:
			nnDiscrVec.erase(nnDiscrVec.begin() + currNumNNCB->currentIndex());
			break;
		default:
			assert(false);
	}
	update_nn_info();
}

void NeuralNetSettingsDialog::save_nn() {
	QString NeuralNetworkFileName = QFileDialog::getSaveFileName(this,
							QString::fromUtf8("Save file"),
							QDir::currentPath(),
							tr("Neural network files (*.nnf);;All files (*)"));
	if (NeuralNetworkFileName.isEmpty()) return;
	QFileInfo file(NeuralNetworkFileName);
	if (file.suffix().isEmpty()) NeuralNetworkFileName += ".nnf";
	std::ofstream ostr (NeuralNetworkFileName.toStdString());
	switch (currNNCB->currentData().toUInt()) {
		case AvailableFunctions::Amplitude:
			nnAmplVec[currNumNNCB->currentIndex()].save(ostr);
			break;
		case AvailableFunctions::Time:
			nnTimeVec[currNumNNCB->currentIndex()].save(ostr);
			break;
		case AvailableFunctions::Discriminating:
			nnDiscrVec[currNumNNCB->currentIndex()].save(ostr);
			break;
		default:
			assert(false);
	}
	update_nn_info();
}

void NeuralNetSettingsDialog::load_nn() {
	QString NeuralNetworkFileName = QFileDialog::getOpenFileName(this,
							QString::fromUtf8("Open file"),
							QDir::currentPath(),
							"Neural network files (*.nnf);;All files (*)");
	if (NeuralNetworkFileName.isEmpty()) return;
	else {
		std::ifstream istr (NeuralNetworkFileName.toStdString());
		Neural_Network::NuclearPhysicsNeuralNet nntmp;
		try {
			nntmp.load(istr);
			if (nntmp.outputs() != 1) throw std::exception ();
		}
		catch (std::exception& ba) {
			QErrorMessage msg;
			msg.setModal(true);
			msg.showMessage("Invalid Neural network file");
			msg.exec();
			return;
		}
		switch (nntmp.get_function(0)) {
			case AvailableFunctions::Amplitude:
				nnAmplVec.push_back(nntmp);
				break;
			case AvailableFunctions::Time:
				nnTimeVec.push_back(nntmp);
				break;
			case AvailableFunctions::Discriminating:
				nnDiscrVec.push_back(nntmp);
				break;
			default:
				assert(false);
		}
		update_nn_info();
	}
}

void NeuralNetSettingsDialog::accept() {
	coreClassPtr->get_ampl_nn() = nnAmplVec;
	coreClassPtr->get_time_nn() = nnTimeVec;
	coreClassPtr->get_discr_nn() = nnDiscrVec;
	QDialog::accept();
}

void NeuralNetSettingsDialog::reject() {
	QDialog::reject();
}

void NeuralNetSettingsDialog::showNoizeDialog() {
	noizSetDial->show();
	noizSetDial->raise();
	noizSetDial->activateWindow();
}

void NeuralNetSettingsDialog::startTeaching() {
	Neural_Network::NuclearPhysicsNeuralNet* NuclearPercep;
	Neural_Network::NuclTeachingClass* currTeach;
	switch (functionCB->currentData().toUInt()) {
		case AvailableFunctions::Amplitude:
			NuclearPercep = &nnAmplVec[availNNCB->currentIndex()];
			currTeach = &nucTeachAmpl;
			break;
		case AvailableFunctions::Time:
			NuclearPercep = &nnTimeVec[availNNCB->currentIndex()];
			currTeach = &nucTeachTime;
			break;
		case AvailableFunctions::Discriminating:
			NuclearPercep = &nnDiscrVec[availNNCB->currentIndex()];
			currTeach = &nucTeachDiscr;
			break;
		default:
			assert(false);
	}

	currTeach->total_iterations(teachCyclesSB->value());

	currTeach->set_teaching_outputs(1);
	NuclearPercep->set_function(functionCB->currentData().toUInt(), 0);
	currTeach->set_output_teach_weight(0, 1.f);

	currTeach->set_noize(noizSetDial->get_noize(0));
	currTeach->set_noize(noizSetDial->get_noize(-2));
	currTeach->set_noize(noizSetDial->get_noize(-1));
	currTeach->set_noize(noizSetDial->get_noize(1));
	currTeach->set_noize(noizSetDial->get_noize(2));

	teachingWaitDialog->set_teaching_class(currTeach);

	QObject context;
	QEventLoop loop;
	teachingWaitDialog->setModal(true);
	teachingWaitDialog->show();
	context.moveToThread(&teachThread);
	connect(&teachThread, &QThread::started, &context,
	[&]() {
		currTeach->set_pulse(coreClassPtr->get_process_settings(shapeStreamCB->currentIndex())->shape);
		currTeach->start(NuclearPercep, teachSizeCB->currentData().toUInt());
		loop.quit();
	});
	teachThread.start();
	loop.exec();
	teachThread.quit();
	teachThread.wait();
	teachingWaitDialog->hide();

	QMessageBox msg (this);
	std::stringstream strs;
	strs << 100. * currTeach->current_STD();
	msg.setText(tr("Teaching finished with result: ") + QString::fromUtf8(strs.str().c_str()));
	msg.setModal(true);
	msg.exec();
	update_teach_widget();
}

NeuralNetAddDialog::NeuralNetAddDialog(QWidget *parent) : QDialog (parent) {
	typeL = new QLabel (tr("Type"), this);
	typeCB = new QComboBox(this);
	inputsL = new QLabel (tr("Inputs"), this);
	inputsCB = new QComboBox(this);
	nameL = new QLabel (tr("Name"), this);
	nameLE = new QLineEdit (this);
	layersL = new QLabel (tr("Layers"), this);
	layersSB = new QSpinBox (this);
	settLayout = new QGridLayout;
	lastLayerL = new QLabel (this);
	lastLayerI = new QLabel (this);
	layersLayout = new QGridLayout;
	addPB = new QPushButton (tr("Add"), this);
	rejPB = new QPushButton (tr("Reject"), this);
	buttonLayout = new QHBoxLayout;
	mainLayout = new QVBoxLayout (this);

	typeCB->addItem("Amplitude", QVariant(AvailableFunctions::Amplitude));
	typeCB->addItem("Time", QVariant(AvailableFunctions::Time));
	typeCB->addItem("Discriminating", QVariant(AvailableFunctions::Discriminating));

	layersSB->setMinimum(2);
	layersSB->setMaximum(6);

	inputsCB->addItem("8", QVariant(0x8));
	inputsCB->addItem("16", QVariant(0x10));
	inputsCB->addItem("32", QVariant(0x20));
	inputsCB->addItem("64", QVariant(0x40));
	inputsCB->addItem("128", QVariant(0x80));
	inputsCB->addItem("256", QVariant(0x100));

	settLayout->addWidget(typeL, 0, 0);
	settLayout->addWidget(typeCB, 0, 1);
	settLayout->addWidget(inputsL, 1, 0);
	settLayout->addWidget(inputsCB, 1, 1);
	settLayout->addWidget(nameL, 2, 0);
	settLayout->addWidget(nameLE, 2, 1);
	settLayout->addWidget(layersL, 3, 0);
	settLayout->addWidget(layersSB, 3, 1);
	settLayout->setRowStretch(4, 1);

	buttonLayout->addWidget(addPB);
	buttonLayout->addWidget(rejPB);

	mainLayout->addLayout(settLayout);
	mainLayout->addLayout(layersLayout);
	mainLayout->addLayout(buttonLayout);

	connect (layersSB, SIGNAL(valueChanged(int)), this, SLOT(update_layers()));
	connect (addPB, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect (rejPB, SIGNAL(clicked(bool)), this, SLOT(reject()));
}

void NeuralNetAddDialog::showEvent(QShowEvent *) {
	update_layers();
}

Neural_Network::NuclearPhysicsNeuralNet NeuralNetAddDialog::get_nn() {
	Neural_Network::NuclearPhysicsNeuralNet tmp (inputsCB->currentData().toUInt());
	for (auto a: NeuralsSBs) tmp.push_layer(a->value());
	tmp.push_layer(1);
	tmp.set_function(typeCB->currentData().toUInt(), 0);
	tmp.set_name(nameLE->text());
	return tmp;
}

void NeuralNetAddDialog::accept() {
	status = true;
	QDialog::accept();
}

void NeuralNetAddDialog::reject() {
	status = false;
	QDialog::reject();
}

void NeuralNetAddDialog::update_layers() {
	while (NeuralsLs.size()+1 < (quint32)layersSB->value()) {
		quint32 i = NeuralsLs.size();
		std::stringstream sstr;
		sstr << "Layer " << i + 1;
		NeuralsLs.push_back(new QLabel (sstr.str().c_str(), this));
		NeuralsSBs.push_back(new QSpinBox (this));
		NeuralsSBs[i]->setMinimum(2);
		NeuralsSBs[i]->setMaximum(32);
		layersLayout->addWidget(NeuralsLs[i], i, 0);
		layersLayout->addWidget(NeuralsSBs[i], i, 1);
	}
	while (NeuralsLs.size() + 1 > (quint32)layersSB->value()) {
		quint32 i = NeuralsLs.size() - 1;
		delete NeuralsLs[i];
		delete NeuralsSBs[i];
		NeuralsLs.pop_back();
		NeuralsSBs.pop_back();
	}
	std::stringstream sstr;
	sstr << "Layer " << NeuralsLs.size() + 1;
	lastLayerL->setText(tr(sstr.str().c_str()));
	lastLayerI->setText(tr("1"));
	layersLayout->addWidget(lastLayerL, NeuralsLs.size(), 0);
	layersLayout->addWidget(lastLayerI, NeuralsSBs.size(), 1);
}

NeuralNetTeachingWaitingDialog::NeuralNetTeachingWaitingDialog(Neural_Network::TeachingClass* _teachingClass, QWidget* _parent) : QDialog (_parent, Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint) {
	teachClass = _teachingClass;
	teachingProgressBar = new QProgressBar (this);
	teachingProgressBar->setMaximum(10000);
	teachingProgressBar->setMinimum(0);
	statusLabel = new QLabel (this);
	currentSTDLabel = new QLabel (this);
	totalTimeLabel = new QLabel (this);
	stopPB = new QPushButton(this);
	stopPB->setText(tr("Stop"));
	mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(teachingProgressBar, 0);
	mainLayout->addWidget(statusLabel, 1, Qt::AlignLeft);
	mainLayout->addWidget(currentSTDLabel, 2, Qt::AlignLeft);
	mainLayout->addWidget(totalTimeLabel, 3, Qt::AlignLeft);
	mainLayout->addWidget(stopPB, 4);
	installEventFilter(this);
	connect (stopPB, SIGNAL(clicked(bool)), this, SLOT(stopClicked()));
}

void NeuralNetTeachingWaitingDialog::showEvent(QShowEvent *_event) {
	Q_UNUSED(_event);
	currStatus = Initializating;
	statusLabel->setText(tr("Initiaizing"));
	currentSTDLabel->setText("");
	updateTimer = new QTimer(this);
	connect (updateTimer, SIGNAL(timeout()), this, SLOT(timerUpdated()));
	updateTimer->start(1000);
	teachingProgressBar->setValue(0);
	totalSecs = 0;
	timerUpdated();
}

void NeuralNetTeachingWaitingDialog::hideEvent(QHideEvent *_event) {
	Q_UNUSED(_event);
	updateTimer->stop();
	disconnect (updateTimer, SIGNAL(timeout()), this, SLOT(timerUpdated()));
	delete updateTimer;
}

void NeuralNetTeachingWaitingDialog::closeEvent(QCloseEvent *_event) {
	Q_UNUSED(_event);
	stopClicked();
	_event->ignore();
	stopClicked();
}

bool NeuralNetTeachingWaitingDialog::eventFilter(QObject *dist, QEvent *event) {
	Q_UNUSED (dist)
	if ( event->type() == QEvent::KeyPress ) {
		  QKeyEvent *keyEvent = static_cast<QKeyEvent*>( event );
		  if(keyEvent->key() == Qt::Key_Escape)
			return true;
	 }
	 return false;
}

NeuralNetTeachingWaitingDialog::~NeuralNetTeachingWaitingDialog () {
	disconnect (stopPB, SIGNAL(clicked(bool)), this, SLOT(stopClicked()));

	delete teachingProgressBar;
	delete statusLabel;
	delete currentSTDLabel;
	delete totalTimeLabel;
	delete stopPB;
	delete mainLayout;
}

void NeuralNetTeachingWaitingDialog::timerUpdated() {
	std::stringstream strs;
	if (!teachClass->running()) {
		currStatus = Initializating;
		statusLabel->setText(tr("Initiaizing"));
		strs << totalSecs;
		totalTimeLabel->setText(tr("Total time: ") + QString::fromUtf8(strs.str().c_str()));
		totalSecs++;
	} else {
		if (currStatus == Initializating) {
			currStatus = Teaching;
			statusLabel->setText(tr("Teaching"));
		} else if (currStatus == Finishing) statusLabel->setText(tr("Finishing"));
		if (totalSecs >= 3600) strs << totalSecs / 3600 << "h " << (totalSecs % 3600) / 60 << "m " << (totalSecs % 3600) % 60 << "s";
		else if (totalSecs >= 60) strs << totalSecs / 60 << "m " << totalSecs % 60 << "s";
		else strs << totalSecs << "s";
		totalTimeLabel->setText(tr("Total time: ") + QString::fromUtf8(strs.str().c_str()));
		strs.str(std::string());
		strs << 100. * teachClass->current_STD();
		currentSTDLabel->setText(tr("Current STD: ") + QString::fromUtf8(strs.str().c_str()));
		totalSecs++;
		float currProg = teachClass->current_progress();
		teachingProgressBar->setValue(10000.f*currProg);
	}
}

void NeuralNetTeachingWaitingDialog::stopClicked() {
	currStatus = Finishing;
	teachClass->enable(false);
}

NoizeSettingsDialog::NoizeSettingsDialog(QWidget *parent) : QDialog (parent) {
	infoLabel = new QLabel (tr("<html>S(f) = a for white noize<br>"
									   "S(f) = a/(b + f<sup>|n|</sup>) for n lesser than 0 <br>"
									   "S(f) = a(b + f<sup>n</sup>) for n bigger than 0 <br>"
									   "f changes from 0 (const) to 1 (max frequency)</html>"), this);
	white = new QLabel (tr("n = 0 (white), a:"), this);
	whiteMagn = new QDoubleSpinBox(this);
	pinkM = new QLabel (tr("n = -1 (pink), a:"), this);
	pinkMagn = new QDoubleSpinBox(this);
	pinkD = new QLabel (tr("b:"), this);
	pinkDiff = new QDoubleSpinBox(this);
	redM = new QLabel (tr("n = -2 (red), a:"), this);
	redMagn = new QDoubleSpinBox(this);
	redD = new QLabel (tr("b:"), this);
	redDiff = new QDoubleSpinBox(this);
	blueM = new QLabel (tr("n = 1 (blue), a:"), this);
	blueMagn = new QDoubleSpinBox(this);
	blueD = new QLabel (tr("b:"), this);
	blueDiff = new QDoubleSpinBox(this);
	violetM = new QLabel (tr("n = 2 (violet), a:"), this);
	violetMagn = new QDoubleSpinBox(this);
	violetD = new QLabel (tr("b:"), this);
	violetDiff = new QDoubleSpinBox(this);
	whiteMagn->setSingleStep(0.01);
	pinkMagn->setSingleStep(0.01);
	redMagn->setSingleStep(0.01);
	blueMagn->setSingleStep(0.01);
	violetMagn->setSingleStep(0.01);
	pinkDiff->setSingleStep(0.01);
	redDiff->setSingleStep(0.01);
	blueDiff->setSingleStep(0.01);
	violetDiff->setSingleStep(0.01);
	acceptPB = new QPushButton (tr("Accept"), this);
	mainLayout = new QGridLayout (this);
	mainLayout->addWidget(infoLabel, 0, 0, 1, 4);
	mainLayout->addWidget(white, 1, 0, 1, 1);
	mainLayout->addWidget(whiteMagn, 1, 1, 1, 1);
	mainLayout->addWidget(pinkM, 2, 0, 1, 1);
	mainLayout->addWidget(pinkMagn, 2, 1, 1, 1);
	mainLayout->addWidget(pinkD, 2, 2, 1, 1);
	mainLayout->addWidget(pinkDiff, 2, 3, 1, 1);
	mainLayout->addWidget(redM, 3, 0, 1, 1);
	mainLayout->addWidget(redMagn, 3, 1, 1, 1);
	mainLayout->addWidget(redD, 3, 2, 1, 1);
	mainLayout->addWidget(redDiff, 3, 3, 1, 1);
	mainLayout->addWidget(blueM, 4, 0, 1, 1);
	mainLayout->addWidget(blueMagn, 4, 1, 1, 1);
	mainLayout->addWidget(blueD, 4, 2, 1, 1);
	mainLayout->addWidget(blueDiff, 4, 3, 1, 1);
	mainLayout->addWidget(violetM, 5, 0, 1, 1);
	mainLayout->addWidget(violetMagn, 5, 1, 1, 1);
	mainLayout->addWidget(violetD, 5, 2, 1, 1);
	mainLayout->addWidget(violetDiff, 5, 3, 1, 1);
	mainLayout->addWidget(acceptPB, 6, 0, 1, 4);
	connect (acceptPB, SIGNAL(clicked(bool)), this, SLOT(accept()));
}

Neural_Network::NoizeInfo NoizeSettingsDialog::get_noize(qint32 order) {
	Neural_Network::NoizeInfo n;
	n.order = order;
	switch (order) {
		case -2:
			n.magnitude = redMagn->value();
			n.diff = redDiff->value();
			break;
		case -1:
			n.magnitude = pinkMagn->value();
			n.diff = pinkDiff->value();
			break;
		case 0:
			n.magnitude = whiteMagn->value();
			break;
		case 1:
			n.magnitude = blueMagn->value();
			n.diff = blueDiff->value();
			break;
		case 2:
			n.magnitude = violetMagn->value();
			n.diff = violetDiff->value();
			break;
		default:
			assert(false);
	}
	return n;
}
