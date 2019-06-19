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

#include "inputdevicesetdialog.hpp"

InputSetDialog::InputSetDialog (Core* _core, QWidget *_parent) : QDialog (_parent), coreClass (_core) {

	inputDevLabel = new QLabel (tr("Input device:"), this);
	inputDevSelect = new QComboBox (this);
	inputDevSelect->addItem("Audio card", QVariant(Core::AudioDevice));
	inputDevSelect->addItem("UART", QVariant(Core::UARTDevice));

	audioSetLabel = new QLabel ("Audio settings", this);
	audioSetPB = new QPushButton ("Settings", this);
	audioSetDial = new AudioDeviceSettings (this);
	audioSetDial->setModal(true);

	uartSetLabel = new QLabel ("UART settings", this);
	uartSetPB = new QPushButton ("Settings", this);
	uartSetDial = new UARTDeviceSettings (coreClass, this);
	uartSetDial->setModal(true);

	dataTypeLabel = new QLabel (tr("Datum type"), this);
	dataTypeCB = new QComboBox (this);
	endianLabel = new QLabel (tr("Datum endian"), this);
	endianCB = new QComboBox (this);
	dataTypeCB->addItem("Unsigned int 8 bits", QVariant (Datum8UBits));
	dataTypeCB->addItem("Signed int 8 bits", QVariant (Datum8SBits));
	dataTypeCB->addItem("Unsigned int 16 bits", QVariant (Datum16UBits));
	dataTypeCB->addItem("Signed int 16 bits", QVariant (Datum16SBits));
	dataTypeCB->addItem("Unsigned int 32 bits", QVariant (Datum32UBits));
	dataTypeCB->addItem("Signed int 32 bits", QVariant (Datum32SBits));
	dataTypeCB->addItem("Float", QVariant (DatumFloat));
	dataTypeCB->addItem("Double", QVariant (DatumDouble));
	endianCB->addItem("Little endian", QVariant(LittleEndian));
	endianCB->addItem("Big endian", QVariant(BigEndian));

	softGainLabel = new QLabel (tr("Software Gain:"), this);
	softGainDSBox = new QDoubleSpinBox (this);
	softGainDSBox->setMinimum(1.0);
	softGainDSBox->setMaximum(1024.0);
	softGainDSBox->setDecimals(0);
	softGainDSBox->setSingleStep(1.0);

	bufferSizeLabel = new QLabel (tr("Buffer size"), this);
	bufferSizeCB = new QComboBox (this);
	bufferSizeCB->addItem("256", QVariant((quint32)0x100));
	bufferSizeCB->addItem("512", QVariant((quint32)0x200));
	bufferSizeCB->addItem("1024", QVariant((quint32)0x400));
	bufferSizeCB->addItem("2048", QVariant((quint32)0x800));
	bufferSizeCB->addItem("4096", QVariant((quint32)0x1000));
	bufferSizeCB->addItem("8192", QVariant((quint32)0x2000));
	bufferSizeCB->addItem("16384", QVariant((quint32)0x4000));
	bufferSizeCB->setCurrentIndex(3);
	spectrumSizeLabel = new QLabel (tr("Spectrum size"), this);
	spectrumSizeCB = new QComboBox (this);
	spectrumSizeCB->addItem("128", QVariant((quint32)0x80));
	spectrumSizeCB->addItem("256", QVariant((quint32)0x100));
	spectrumSizeCB->addItem("512", QVariant((quint32)0x200));
	spectrumSizeCB->addItem("1024", QVariant((quint32)0x400));
	spectrumSizeCB->addItem("2048", QVariant((quint32)0x800));
	spectrumSizeCB->addItem("4096", QVariant((quint32)0x1000));
	spectrumSizeCB->addItem("8192", QVariant((quint32)0x2000));
	spectrumSizeCB->setCurrentIndex(2);

	acceptButton = new QPushButton (tr("Accept"), this);
	cancelButton = new QPushButton (tr("Cancel"), this);
	connect(acceptButton, SIGNAL (clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL (clicked()), this, SLOT(reject()));
	connect(inputDevSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(inp_dev_changed()));
	connect(audioSetPB, SIGNAL(clicked(bool)), this, SLOT(aud_set_show()));
	connect(uartSetPB, SIGNAL(clicked(bool)), this, SLOT(uart_set_show()));
	connect(audioSetDial, SIGNAL(accepted()), this, SLOT(chk_values()));
	connect(uartSetDial, SIGNAL(accepted()), this, SLOT(chk_values()));

	//connect(uartDevSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(uart_dev_changed()));

	mainLayout = new QGridLayout (this);
	quint32 index = 0;
	mainLayout->addWidget(inputDevLabel, index, 0);
	mainLayout->addWidget(inputDevSelect, index++, 1);
	mainLayout->addWidget(audioSetLabel, index, 0);
	mainLayout->addWidget(audioSetPB, index, 1);
	mainLayout->addWidget(uartSetLabel, index, 0);
	mainLayout->addWidget(uartSetPB, index++, 1);
	mainLayout->addWidget(dataTypeLabel, index, 0);
	mainLayout->addWidget(dataTypeCB, index++, 1);
	mainLayout->addWidget(endianLabel, index, 0);
	mainLayout->addWidget(endianCB, index++, 1);
	mainLayout->addWidget(softGainLabel, index, 0);
	mainLayout->addWidget(softGainDSBox, index++, 1);
	mainLayout->addWidget(bufferSizeLabel, index, 0);
	mainLayout->addWidget(bufferSizeCB, index++, 1);
	mainLayout->addWidget(spectrumSizeLabel, index, 0);
	mainLayout->addWidget(spectrumSizeCB, index++, 1);
	mainLayout->addWidget(acceptButton, index, 0, Qt::AlignLeft);
	mainLayout->addWidget(cancelButton, index++, 1, Qt::AlignRight);
}

InputSetDialog::~InputSetDialog() {
}

void InputSetDialog::showEvent(QShowEvent *) {
	coreClass->stop();
	load_devs();
	load_settings();
	inp_dev_changed();
}

void InputSetDialog::load_devs() {
	audioDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	uartPorts = QSerialPortInfo::availablePorts();
	for (auto a = audioDevices.begin(); a != audioDevices.end();) {
		if (a->supportedSampleSizes().empty()) {
			a = audioDevices.erase(a);
		} else a++;
	}
	int index = -1;
	auto a = audioDevices.begin();
	for (quint32 i = 0, ie = audioDevices.size(); i < ie; i++) {
		if (coreClass->get_audio_device().deviceName() == a->deviceName()) {
			index = i;
			break;
		}
		a++;
	}
	if (index == -1) {
		std::cout << "Can't find audio device " << coreClass->get_audio_device().deviceName().toUtf8().data() << std::endl;
		audioDeviceIndex = 0;
	} else audioDeviceIndex = index;
	index = -1;
	auto b = uartPorts.begin();
	for (quint32 i = 0, ie = uartPorts.size(); i < ie; i++) {
		if (coreClass->get_uart_settings().portName == b->portName()) {
			index = i;
			break;
		}
		b++;
	}
	if (index == -1) {
		if (!uartPorts.empty()) std::cout << "Can't find port " << coreClass->get_uart_settings().portName.toUtf8().data() << std::endl;
		uartPortIndex = 0;
		if (coreClass->uart_device_is_opened()) coreClass->close_uart_device();
	} else uartPortIndex = index;
}

void InputSetDialog::load_settings() {
	switch (coreClass->get_input_device()) {
		case Core::AudioDevice:
			inputDevSelect->setCurrentIndex(0);
			softGainDSBox->setValue(coreClass->get_audio_software_gain());
			dataTypeCB->setCurrentIndex(get_index(dataTypeCB->findData(coreClass->get_audio_datum_type())));
			endianCB->setCurrentIndex(get_index(endianCB->findData(coreClass->get_audio_datum_align())));
			audioChannelCount = coreClass->get_audio_channels();
			break;
		case Core::UARTDevice:
			inputDevSelect->setCurrentIndex(1);
			softGainDSBox->setValue(coreClass->get_uart_software_gain());
			dataTypeCB->setCurrentIndex(get_index(dataTypeCB->findData(coreClass->get_uart_datum_type())));
			endianCB->setCurrentIndex(get_index(endianCB->findData(coreClass->get_uart_datum_align())));
			uartChannels = coreClass->get_uart_channels();
			break;
		default:
			assert(false);
	}
}

void InputSetDialog::chk_values() {
	switch (inputDevSelect->currentIndex()) {
		case 0:
			if ((quint32)audioDevices.size() > audioDeviceIndex) acceptButton->setEnabled(true);
			else acceptButton->setEnabled(false);
			break;
		case 1:
			if (coreClass->uart_device_is_opened() && (uartPortIndex < (quint32)uartPorts.size())) acceptButton->setEnabled(true);
			else acceptButton->setEnabled(false);
			break;
		default:
			assert(false);
	}
}

void InputSetDialog::aud_set_show() {
	audioSetDial->set_curr_settings(&audioDevices, &audioDeviceIndex, &audioSampleRate, &audioChannelCount);
	audioSetDial->show();
	audioSetDial->raise();
	audioSetDial->activateWindow();
}

void InputSetDialog::uart_set_show() {
	uartSetDial->set_curr_settings(&uartPorts, &uartSettings, &uartPortIndex, &uartChannels);
	uartSetDial->show();
	uartSetDial->raise();
	uartSetDial->activateWindow();
}

void InputSetDialog::accept () {
	coreClass->set_buffer_size((quint32)bufferSizeCB->currentData().toUInt());
	coreClass->set_spectrum_size((quint32)spectrumSizeCB->currentData().toUInt());
	switch (inputDevSelect->currentIndex()) {
		case 0:
			coreClass->set_input_device(Core::AudioDevice);
			coreClass->set_sample_rate(audioSampleRate);
			coreClass->set_audio_channels(audioChannelCount);
			coreClass->set_audio_software_gain(softGainDSBox->value());
			coreClass->set_audio_device(audioDevices[audioDeviceIndex]);
			coreClass->set_audio_datum_type(dataTypeCB->currentData().toInt());
			coreClass->set_audio_datum_align(endianCB->currentData().toInt());
			break;
		case 1:
			coreClass->set_input_device(Core::UARTDevice);
			coreClass->set_uart_datum_type(dataTypeCB->currentData().toInt());
			coreClass->set_uart_datum_align(endianCB->currentData().toInt());
			coreClass->set_uart_channels(uartChannels);
			coreClass->set_uart_software_gain(softGainDSBox->value());
			break;
	}
	QDialog::accept();
}

void InputSetDialog::inp_dev_changed() {
	switch (inputDevSelect->currentIndex()) {
		case 0:
			audioSetPB->show();
			audioSetLabel->show();
			uartSetPB->hide();
			uartSetLabel->hide();
			softGainDSBox->setValue(coreClass->get_audio_software_gain());
			dataTypeCB->setCurrentIndex(get_index(dataTypeCB->findData(QVariant(coreClass->get_audio_datum_type()))));
			endianCB->setCurrentIndex(get_index(endianCB->findData(QVariant(coreClass->get_audio_datum_align()))));
			bufferSizeCB->setCurrentIndex(get_index(bufferSizeCB->findData(QVariant(coreClass->get_buffer_size()))));
			spectrumSizeCB->setCurrentIndex(get_index(spectrumSizeCB->findData(QVariant(coreClass->get_spectrum_size()))));
			break;
		case 1:
			audioSetPB->hide();
			audioSetLabel->hide();
			uartSetPB->show();
			uartSetLabel->show();
			softGainDSBox->setValue(coreClass->get_audio_software_gain());
			dataTypeCB->setCurrentIndex(get_index(dataTypeCB->findData(QVariant(coreClass->get_uart_datum_type()))));
			endianCB->setCurrentIndex(get_index(endianCB->findData(QVariant(coreClass->get_uart_datum_align()))));
			bufferSizeCB->setCurrentIndex(get_index(bufferSizeCB->findData(QVariant(coreClass->get_buffer_size()))));
			spectrumSizeCB->setCurrentIndex(get_index(spectrumSizeCB->findData(QVariant(coreClass->get_spectrum_size()))));
			break;
		default:
			assert(false);
	}
	chk_values();
}

DeviceSettingsDialog::DeviceSettingsDialog (QWidget *parent) : QDialog (parent) {
	acceptPB = new QPushButton (tr("Accept"), this);
	rejectPB = new QPushButton (tr("Reject"), this);
	mainLayout = new QGridLayout (this);
	connect (acceptPB, SIGNAL(clicked()), this, SLOT(accept()));
	connect (rejectPB, SIGNAL(clicked()), this, SLOT(reject()));
}

AudioDeviceSettings::AudioDeviceSettings(QWidget *parent) : DeviceSettingsDialog (parent) {
	currAuDevLabel = new QLabel (tr("Audio source"), this);
	currAuDevCB = new QComboBox (this);
	availSampleRatesLabel = new QLabel (tr("Sample rate"), this);
	availSampleRatesCB = new QComboBox (this);
	availChanCountLabel = new QLabel (tr("Channel count"), this);
	availChanCountCB = new QComboBox (this);
	mainLayout->addWidget(currAuDevLabel, 0, 0);
	mainLayout->addWidget(currAuDevCB, 0, 1);
	mainLayout->addWidget(availSampleRatesLabel, 1, 0);
	mainLayout->addWidget(availSampleRatesCB, 1, 1);
	mainLayout->addWidget(availChanCountLabel, 2, 0);
	mainLayout->addWidget(availChanCountCB, 2, 1);
	mainLayout->addWidget(acceptPB, 3, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, 3, 1, 1, 1, Qt::AlignRight);
}

void AudioDeviceSettings::set_curr_settings(QList<QAudioDeviceInfo> *_devs, quint32* _devIndex, quint32* _sampleRate, quint32* _chanCount) {
	disconnect (currAuDevCB, SIGNAL(currentIndexChanged(int)), this, SLOT(device_changed()));
	currAuDevCB->clear();
	for (auto a = _devs->begin(); a != _devs->end(); a++) {
		currAuDevCB->addItem(a->deviceName());
	}
	devs = _devs;
	devIndex = _devIndex;
	sampleRate = _sampleRate;
	chanCount = _chanCount;
	currAuDevCB->setCurrentIndex(*devIndex);
	device_changed();
	connect (currAuDevCB, SIGNAL(currentIndexChanged(int)), this, SLOT(device_changed()));
}

void AudioDeviceSettings::accept() {
	*devIndex = currAuDevCB->currentIndex();
	*sampleRate = availSampleRatesCB->currentData().toInt();
	*chanCount = availChanCountCB->currentData().toInt();
	QDialog::accept();
}

void AudioDeviceSettings::device_changed() {
	availSampleRatesCB->clear();
	availChanCountCB->clear();
    if (devs->empty()) return;
	auto a = (*devs)[*devIndex].supportedSampleRates();
	for (auto b = a.begin(); b != a.end(); b++) {
		std::stringstream sstr;
		sstr << *b;
		availSampleRatesCB->addItem(sstr.str().c_str(), QVariant(*b));
	}
	availSampleRatesCB->setCurrentIndex(get_index(availSampleRatesCB->findData(QVariant((int)*sampleRate))));
	a = (*devs)[*devIndex].supportedChannelCounts();
	for (auto b = a.begin(); b != a.end(); b++) {
		std::stringstream sstr;
		sstr << *b;
		availChanCountCB->addItem(sstr.str().c_str(), QVariant(*b));
	}
	availChanCountCB->setCurrentIndex(get_index(availChanCountCB->findData(QVariant((int)*chanCount))));
}

UARTDeviceSettings::UARTDeviceSettings (Core* _core, QWidget *parent) : DeviceSettingsDialog (parent) {
	corePtr = _core;
	devLabel = new QLabel (tr("Device"), this);
	devCB = new QComboBox (this);
	baudLabel = new QLabel (tr("Baud rate"), this);
	baudCB = new QComboBox (this);
	dataBitsLabel = new QLabel (tr("Data bits"), this);
	dataBitsCB = new QComboBox (this);
	parityLabel = new QLabel (tr("Parity"), this);
	parityCB = new QComboBox (this);
	stopBitsLabel = new QLabel (tr("Stop bits"), this);
	stopBitsCB = new QComboBox (this);
	flowControlLabel = new QLabel (tr("Flow control"), this);
	flowControlCB = new QComboBox (this);
	channelsLabel = new QLabel (tr("Channels"), this);
	channelsSB = new QSpinBox (this);
	portStatus = new QLabel (this);
	openPortPB = new QPushButton (this);

	auto a = QSerialPortInfo::standardBaudRates();
	for (auto b = a.begin(); b != a.end(); b++) {
		std::stringstream sstr;
		sstr << *b;
		baudCB->addItem(sstr.str().c_str(), QVariant(*b));
	}

	dataBitsCB->addItem("5", QVariant(QSerialPort::Data5));
	dataBitsCB->addItem("6", QVariant(QSerialPort::Data6));
	dataBitsCB->addItem("7", QVariant(QSerialPort::Data7));
	dataBitsCB->addItem("8", QVariant(QSerialPort::Data8));

	parityCB->addItem(tr("No"), QVariant(QSerialPort::NoParity));
	parityCB->addItem(tr("Even"), QVariant(QSerialPort::EvenParity));
	parityCB->addItem(tr("Odd"), QVariant(QSerialPort::OddParity));
	parityCB->addItem(tr("Space"), QVariant(QSerialPort::SpaceParity));
	parityCB->addItem(tr("Mark"), QVariant(QSerialPort::MarkParity));

	stopBitsCB->addItem("1", QVariant(QSerialPort::OneStop));
	stopBitsCB->addItem("1.5", QVariant(QSerialPort::OneAndHalfStop));
	stopBitsCB->addItem("2", QVariant(QSerialPort::TwoStop));

	flowControlCB->addItem("No", QVariant(QSerialPort::NoFlowControl));
	flowControlCB->addItem("Hardware", QVariant(QSerialPort::HardwareControl));
	flowControlCB->addItem("Sortware", QVariant(QSerialPort::SoftwareControl));
	channelsSB->setMinimum(1);
	channelsSB->setMaximum(8);

	quint32 i = 0;
	mainLayout->addWidget(devLabel, i, 0);
	mainLayout->addWidget(devCB, i, 1);
	mainLayout->addWidget(baudLabel, ++i, 0);
	mainLayout->addWidget(baudCB, i, 1);
	mainLayout->addWidget(dataBitsLabel, ++i, 0);
	mainLayout->addWidget(dataBitsCB, i, 1);
	mainLayout->addWidget(parityLabel, ++i, 0);
	mainLayout->addWidget(parityCB, i, 1);
	mainLayout->addWidget(stopBitsLabel, ++i, 0);
	mainLayout->addWidget(stopBitsCB, i, 1);
	mainLayout->addWidget(flowControlLabel, ++i, 0);
	mainLayout->addWidget(flowControlCB, i, 1);
	mainLayout->addWidget(channelsLabel, ++i, 0);
	mainLayout->addWidget(channelsSB, i, 1);
	mainLayout->addWidget(portStatus, ++i, 0);
	mainLayout->addWidget(openPortPB, i, 1);
	mainLayout->addWidget(acceptPB, ++i, 0, 1, 1, Qt::AlignLeft);
	mainLayout->addWidget(rejectPB, i, 1, 1, 1, Qt::AlignRight);
	connect (openPortPB, SIGNAL(clicked(bool)), this, SLOT(open_port()));
}

void UARTDeviceSettings::showEvent(QShowEvent *) {
	update_status();
}

void UARTDeviceSettings::update_status () {
	if (devs->empty()) {
		portStatus->setText(tr("No ports found"));
		openPortPB->setText(tr("Open port"));
		openPortPB->setEnabled(false);
		enable(false);
		acceptPB->setEnabled(false);
	} else if (corePtr->uart_device_is_opened()) {
		portStatus->setText(tr("Port opened"));
		openPortPB->setText(tr("Close port"));
		openPortPB->setEnabled(true);
		enable(false);
		acceptPB->setEnabled(true);
	} else {
		portStatus->setText(tr("Port not opened"));
		openPortPB->setText(tr("Open port"));
		openPortPB->setEnabled(true);
		enable(true);
		acceptPB->setEnabled(false);
	}
	disconnect (devCB, SIGNAL(currentIndexChanged(int)), this, SLOT(device_changed()));
	devCB->clear();
	for (auto a = devs->begin(); a != devs->end(); a++) {
		devCB->addItem(a->portName());
	}
	devCB->setCurrentIndex(*devIndex);
	connect (devCB, SIGNAL(currentIndexChanged(int)), this, SLOT(device_changed()));
}

void UARTDeviceSettings::enable(bool enable) {
	devCB->setEnabled(enable);
	baudCB->setEnabled(enable);
	dataBitsCB->setEnabled(enable);
	parityCB->setEnabled(enable);
	stopBitsCB->setEnabled(enable);
	flowControlCB->setEnabled(enable);
}

SerialPortInfo UARTDeviceSettings::read_widgets() {
	SerialPortInfo set;

	set.portSettings.baudRate = baudCB->currentData().toInt();
	set.portName = devCB->currentText();

	switch (dataBitsCB->currentData().toInt()) {
		case QSerialPort::Data5: set.portSettings.dataBits = QSerialPort::Data5; break;
		case QSerialPort::Data6: set.portSettings.dataBits = QSerialPort::Data6; break;
		case QSerialPort::Data7: set.portSettings.dataBits = QSerialPort::Data7; break;
		case QSerialPort::Data8: set.portSettings.dataBits = QSerialPort::Data8; break;
		default: assert(false);
	}
	switch (parityCB->currentData().toInt()) {
		case QSerialPort::NoParity: set.portSettings.parity = QSerialPort::NoParity; break;
		case QSerialPort::EvenParity: set.portSettings.parity = QSerialPort::EvenParity; break;
		case QSerialPort::OddParity: set.portSettings.parity = QSerialPort::OddParity; break;
		case QSerialPort::SpaceParity: set.portSettings.parity = QSerialPort::SpaceParity; break;
		case QSerialPort::MarkParity: set.portSettings.parity = QSerialPort::MarkParity; break;
		default: assert(false);
	}
	switch (stopBitsCB->currentData().toInt()) {
		case QSerialPort::OneStop: set.portSettings.stopBits = QSerialPort::OneStop; break;
		case QSerialPort::OneAndHalfStop: set.portSettings.stopBits = QSerialPort::OneAndHalfStop; break;
		case QSerialPort::QSerialPort::TwoStop: set.portSettings.stopBits = QSerialPort::TwoStop; break;
		default: assert(false);
	}
	switch (flowControlCB->currentData().toInt()) {
		case QSerialPort::NoFlowControl: set.portSettings.flowControl = QSerialPort::NoFlowControl; break;
		case QSerialPort::HardwareControl: set.portSettings.flowControl = QSerialPort::HardwareControl; break;
		case QSerialPort::SoftwareControl: set.portSettings.flowControl = QSerialPort::SoftwareControl; break;
		default: assert(false);
	}
	return set;
}

void UARTDeviceSettings::set_curr_settings(QList<QSerialPortInfo> *_devs, SerialPortSettings *_settings, quint32 * _devIndex, quint32 *_channels) {
	devs = _devs;
	settings = _settings;
	devIndex = _devIndex;
	channels = _channels;
}

void UARTDeviceSettings::accept() {
	*devIndex = devCB->currentIndex();

	settings->baudRate = baudCB->currentData().toInt();

	switch (dataBitsCB->currentData().toInt()) {
		case QSerialPort::Data5: settings->dataBits = QSerialPort::Data5; break;
		case QSerialPort::Data6: settings->dataBits = QSerialPort::Data6; break;
		case QSerialPort::Data7: settings->dataBits = QSerialPort::Data7; break;
		case QSerialPort::Data8: settings->dataBits = QSerialPort::Data8; break;
		default: assert(false);
	}
	switch (parityCB->currentData().toInt()) {
		case QSerialPort::NoParity: settings->parity = QSerialPort::NoParity; break;
		case QSerialPort::EvenParity: settings->parity = QSerialPort::EvenParity; break;
		case QSerialPort::OddParity: settings->parity = QSerialPort::OddParity; break;
		case QSerialPort::SpaceParity: settings->parity = QSerialPort::SpaceParity; break;
		case QSerialPort::MarkParity: settings->parity = QSerialPort::MarkParity; break;
		default: assert(false);
	}
	switch (stopBitsCB->currentData().toInt()) {
		case QSerialPort::OneStop: settings->stopBits = QSerialPort::OneStop; break;
		case QSerialPort::OneAndHalfStop: settings->stopBits = QSerialPort::OneAndHalfStop; break;
		case QSerialPort::QSerialPort::TwoStop: settings->stopBits = QSerialPort::TwoStop; break;
		default: assert(false);
	}
	switch (flowControlCB->currentData().toInt()) {
		case QSerialPort::NoFlowControl: settings->flowControl = QSerialPort::NoFlowControl; break;
		case QSerialPort::HardwareControl: settings->flowControl = QSerialPort::HardwareControl; break;
		case QSerialPort::SoftwareControl: settings->flowControl = QSerialPort::SoftwareControl; break;
		default: assert(false);
	}
	*channels = channelsSB->value();
	QDialog::accept();
}

void UARTDeviceSettings::device_changed() {
	baudCB->clear();
	update_status();
	baudCB->setCurrentIndex(get_index(baudCB->findData(QVariant(settings->baudRate))));
}

void UARTDeviceSettings::open_port() {
	if (corePtr->uart_device_is_opened()) {
		corePtr->close_uart_device();
		portStatus->setText(tr("Port closed"));
		openPortPB->setText(tr("Open port"));
		enable(true);
		acceptPB->setEnabled(false);
	} else {
		SerialPortInfo set (read_widgets());
		bool b2 = corePtr->set_uart_settings(set);
		corePtr->open_uart_device();
		QThread::msleep(50);
		bool b1 = corePtr->uart_device_is_opened();
		if (b1 && b2) {
			portStatus->setText(tr("Port opened"));
			openPortPB->setText(tr("Close port"));
			enable(false);
			acceptPB->setEnabled(true);
		} else if (b1 && !b2) {
			corePtr->close_uart_device();
			portStatus->setText(tr("Wrong parameters"));
			openPortPB->setText(tr("Open port"));
			enable(true);
			acceptPB->setEnabled(false);
		} else {
			portStatus->setText(tr("Can't open port"));
			openPortPB->setText(tr("Open port"));
			enable(true);
			acceptPB->setEnabled(false);
		}
	}
}


