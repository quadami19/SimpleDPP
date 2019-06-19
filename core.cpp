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

#include "core.hpp"
#include "mainwindow.hpp"

int get_index(int index)
	{ if (index < 0) index = 0; return index; }

Core::Core(quint32 dataSize, quint32 spectrumSize, MainWindow* _parent) : QObject () {

	mainWinPtr = _parent;
	dataBufferSize = dataSize;

	ADClass = new AudioDetector (dataSize);
	SPort = new SerialPortDevice(nullptr, dataSize);
	IPolation = new InterpolationClass ();
	filtProc = new FilteringProcessor (dataSize);
	pulProc = new PulseProcessing (spectrumSize);

	set_audio_channels(1);
	set_uart_channels(1);
	update_channels();
	set_buffer_size(dataBufferSize);
	connect (this, SIGNAL (start_sig()), ADClass, SLOT(start()));
	connect (this, SIGNAL (stop_sig()), ADClass, SLOT(stop()));
	connect (ADClass, SIGNAL (change_state(bool)), mainWinPtr, SLOT(state_changed(bool)));
	connect (SPort, SIGNAL (change_state(bool)), mainWinPtr, SLOT(state_changed(bool)));
	connect (ADClass, SIGNAL (data_ready()), this, SLOT(receive_data()));
	connect (this, SIGNAL(filter()), filtProc, SLOT(process()));
	//connect (filtProc, SIGNAL(finished()), this, SLOT(filt_finished()));
	connect (this, SIGNAL(interpolate()), IPolation, SLOT(start()));
	//connect (IPolation, SIGNAL(finished()), this, SLOT(inter_finished()));
	connect (this, SIGNAL(process()), pulProc, SLOT(process()));
	//connect (pulProc, SIGNAL(finished()), this, SLOT(proc_finished()));

	thisThread = std::shared_ptr<QThread> (new QThread);
	this->moveToThread(thisThread.get());
	thisThread->start();
	thisThread->setPriority(QThread::HighestPriority);

}

Core::~Core() {
	thisThread->exit();
	thisThread->wait();
	delete ADClass;
	delete SPort;
	delete filtProc;
	delete IPolation;
	delete pulProc;
}

void Core::start() {
	start_sig();
	emit state_changed(true);
}

void Core::stop() {
	stop_sig();
	emit state_changed(false);
}

void Core::toggle_state() {
	switch (currentDevice) {
		case AudioDevice:
			if (ADClass->get_state()) {
				stop_sig();
				emit state_changed(false);
			} else {
				start_sig();
				emit state_changed(true);
			}
			break;
		case UARTDevice:
			if (SPort->get_state()) {
				stop_sig();
				emit state_changed(false);
			} else {
				start_sig();
				emit state_changed(true);
			}
			break;
		default:
			assert(false);
	}
}

void Core::update_channels() {
	quint32 num = 0;
	switch (currentDevice) {
		case AudioDevice:
			num = ADClass->get_channels();
			break;
		case UARTDevice:
			num = SPort->get_channels();
			break;
		default:
			assert(false);
	}
    if (num == 0) return;

	rawData.resize(num);
	filterData.resize(num);
	outputData.resize(num);
	switch (currentDevice) {
		case AudioDevice:
			for (quint32 n = 0; n < num; n++) outputData[n] = filterData[n] = rawData[n] = ADClass->get_data(n);
			break;
		case UARTDevice:
			for (quint32 n = 0; n < num; n++) outputData[n] = filterData[n] = rawData[n] = SPort->get_data(n);
			break;
		default:
			assert(false);
	}
	while (inputsName.size() < num) {
		std::stringstream sstr;
		sstr << "Input_" << inputsName.size();
		inputsName.push_back(QString(sstr.str().c_str()));
	}
	while (inputsName.size() > num) inputsName.pop_back();
	filtProc->set_streams(num);
	IPolation->set_inputs(num);
}

void Core::set_buffer_size(quint32 size) {
	if (dataBufferSize == size) return;
	dataBufferSize = size;
	ADClass->set_data_size(size);
	SPort->set_data_size(size);
	filtProc->set_size(size);
	emit buff_size_changed();
}

void Core::set_spectrum_size(quint32 size) {
	pulProc->set_spectrum_size(size);
}

bool Core::is_working() const {
	switch (currentDevice) {
		case AudioDevice: return ADClass->get_state();
		case UARTDevice: return SPort->get_state();
		default:
			assert(false);
	}
}

void Core::set_input_device(quint32 dev) {
	disconnect (ADClass, SIGNAL (data_ready()), this, SLOT(receive_data()));
	disconnect (SPort, SIGNAL (data_ready()), this, SLOT(receive_data()));
	disconnect (this, SIGNAL (start_sig()), ADClass, SLOT(start()));
	disconnect (this, SIGNAL (stop_sig()), ADClass, SLOT(stop()));
	disconnect (this, SIGNAL (start_sig()), SPort, SLOT(start()));
	disconnect (this, SIGNAL (stop_sig()), SPort, SLOT(stop()));
	currentDevice = dev;
	switch (currentDevice) {
		case AudioDevice:
			connect (ADClass, SIGNAL (data_ready()), this, SLOT(receive_data()));
			connect (this, SIGNAL (start_sig()), ADClass, SLOT(start()));
			connect (this, SIGNAL (stop_sig()), ADClass, SLOT(stop()));
			break;
		case UARTDevice:
			connect (SPort, SIGNAL (data_ready()), this, SLOT(receive_data()));
			connect (this, SIGNAL (start_sig()), SPort, SLOT(start()));
			connect (this, SIGNAL (stop_sig()), SPort, SLOT(stop()));
			break;
		default:
			assert(false);
	}
	update_channels();
}

void Core::save_settings (std::ostream& os) {
	ADClass->stop();
	stop();
	ADClass->save_settings(os);
	SPort->save_settings(os);
	filtProc->save_settings(os);
	IPolation->save_settings(os);
	pulProc->save(os);

	quint32 nns = availableAmplNNs.size();
	os.write((char*)&nns, 4);
	for (auto& a: availableAmplNNs) a.save(os);
	nns = availableTimeNNs.size();
	os.write((char*)&nns, 4);
	for (auto& a: availableTimeNNs) a.save(os);
	nns = availableDiscrNNs.size();
	os.write((char*)&nns, 4);
	for (auto& a: availableDiscrNNs) a.save(os);

	os.write((char*)&currentDevice, 4);
}

void Core::load_settings(std::istream &is) {
	stop();
	ADClass->load_settings(is);
	SPort->load_settings(is);
	filtProc->load_settings(is);
	IPolation->load_settings(is);
	pulProc->load(is);
	quint32 tmp;

	is.read((char*)&tmp, 4);
	availableAmplNNs.resize(tmp);
	for (auto& a: availableAmplNNs) a.load(is);
	is.read((char*)&tmp, 4);
	availableTimeNNs.resize(tmp);
	for (auto& a: availableTimeNNs) a.load(is);
	is.read((char*)&tmp, 4);
	availableDiscrNNs.resize(tmp);
	for (auto& a: availableDiscrNNs) a.load(is);

	is.read((char*)&tmp, 4);
	set_input_device(tmp);
}

void Core::receive_data() {
	switch (currentDevice) {
		case AudioDevice:
			for (quint32 n = 0; n < ADClass->get_channels(); n++) rawData[n] = ADClass->get_data(n);
			break;
		case UARTDevice:
			for (quint32 n = 0; n < SPort->get_channels(); n++) rawData[n] = SPort->get_data(n);
			break;
		default:
			assert(false);
	}
	for (quint32 n = 0; n < filtProc->get_streams(); n++)
		filtProc->set_input(rawData[n], n);
	filtProc->process();
	filt_finished();
}

void Core::filt_finished() {
	for (quint32 n = 0; n < filtProc->get_streams(); n++) {
		if (filtProc->get_enabled(n))
			filterData[n] = filtProc->get_output(n);
		else filterData[n] = rawData[n];
		IPolation->set_input(filterData[n], n);
	}
	IPolation->start();
	inter_finished();
}

void Core::inter_finished() {
	for (quint32 n = 0; n < filtProc->get_streams(); n++)
		if (IPolation->get_inter_enabled()) outputData[n] = IPolation->get_output(n);
		else outputData[n] = filterData[n];
	pulProc->set_inputs(outputData);
	pulProc->process();
	proc_finished();
}

void Core::proc_finished() {
	finished();
}
