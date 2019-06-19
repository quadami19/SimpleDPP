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

#include "serialport.hpp"

SerialPort::SerialPort (QObject *parent, quint32 _buffsize) : QObject (parent) {
	thisPort.setReadBufferSize(buffsize);
	buffsize = _buffsize;
}

bool SerialPort::set_port_settings (const SerialPortInfo &settings) {
	sPInfo = settings;
	return (thisPort.setBaudRate(sPInfo.portSettings.baudRate)
			&& thisPort.setDataBits(sPInfo.portSettings.dataBits)
			&& thisPort.setParity(sPInfo.portSettings.parity)
			&& thisPort.setStopBits(sPInfo.portSettings.stopBits)
			&& thisPort.setFlowControl(sPInfo.portSettings.flowControl));
}

void SerialPort::read_data() {
	if (buffsize <= (quint32)thisPort.bytesAvailable()) {
		QByteArray data (thisPort.read(buffsize));
		data_ready(data);
	}
}

void SerialPort::open_port() {
	if (thisPort.isOpen()) thisPort.close();
	stop_port();
	thisPort.setPortName(sPInfo.portName);
	if (thisPort.open(QIODevice::ReadWrite)) {
		if (thisPort.setBaudRate(sPInfo.portSettings.baudRate)
			&& thisPort.setDataBits(sPInfo.portSettings.dataBits)
			&& thisPort.setParity(sPInfo.portSettings.parity)
			&& thisPort.setStopBits(sPInfo.portSettings.stopBits)
			&& thisPort.setFlowControl(sPInfo.portSettings.flowControl)) {
			if (thisPort.isOpen()) {
				std::cout << ("Port " + sPInfo.portName + " opened").toUtf8().data() << std::endl;
			} else {
				thisPort.close();
				std::cout << ("Can't set up port" + sPInfo.portName).toUtf8().data() << std::endl;
			}
		} else {
			thisPort.close();
			std::cout << thisPort.errorString().toLocal8Bit().data() << std::endl;
		}
	} else {
		thisPort.close();
		std::cout << thisPort.errorString().toLocal8Bit().data() << std::endl;
	}
}

void SerialPort::close_port() {
	if (thisPort.isOpen()) {
		thisPort.close();
		std::cout << "Port closed." << std::endl;
	}
}

void SerialPort::start_port() {
	connect(&thisPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handle_error(QSerialPort::SerialPortError)));
	connect(&thisPort, SIGNAL(readyRead()), this, SLOT(read_data()));
	state = true;
	change_state(true);
}

void SerialPort::stop_port() {
	disconnect(&thisPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handle_error(QSerialPort::SerialPortError)));
	disconnect(&thisPort, SIGNAL(readyRead()), this, SLOT(read_data()));
	state = false;
	change_state(false);
}

void SerialPort::handle_error(QSerialPort::SerialPortError error) {
	if ((thisPort.isOpen()) && (error == QSerialPort::ResourceError)) {
		std::cout << thisPort.errorString().toUtf8().data() << std::endl;
		//close_port();
		stop_port();
	}
}

SerialPortDevice::SerialPortDevice (QObject *parent, quint32 datasize) :  QObject(parent) {
	port = new SerialPort (0x0, datasize);
	data.resize(1);
	data[0].resize(datasize);
	thread = new QThread;
	port->moveToThread(thread);
	port->thisPort.moveToThread(thread);
	connect(this, SIGNAL(open_port()), port, SLOT(open_port()));
	connect(this, SIGNAL(close_port()), port, SLOT(close_port()));
	connect(this, SIGNAL(start_port()), port, SLOT(start_port()));
	connect(this, SIGNAL(stop_port()), port, SLOT(stop_port()));
	connect(port, SIGNAL(data_ready(QByteArray)), this, SLOT(receive_data(QByteArray)));
	connect(port, SIGNAL(finished_port()), thread, SLOT(quit()));
	connect(thread, SIGNAL(finished()), port, SLOT(deleteLater()));
	connect(port, SIGNAL(finished_port()), thread, SLOT(deleteLater()));
	connect(port, SIGNAL(change_state(bool)), this, SLOT(state_changed(bool)));
	thread->start();
}

void SerialPortDevice::update_buffsize() {
	quint32 newsize = get_data_size()*get_channels();
	if (datumType == Datum8UBits || datumType == Datum8SBits) newsize *= 1;
	else if (datumType == Datum16UBits || datumType == Datum16SBits) newsize *= 2;
	else if (datumType == DatumDouble) newsize *= 8;
	else newsize *= 4;
	port->set_buffsize(newsize);
}

void SerialPortDevice::receive_data(QByteArray buffer) {
	float k = softwareGain;
	quint32 channels = data.size();
	switch (datumType) {
		case Datum8UBits:
			k /= (float)std::numeric_limits<quint8>::max();
			for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++) data[n][i] = ((*(quint8*)(buffer.data() + i*channels+n))*k - .5f) * 2;
			break;
		case Datum8SBits:
			k /= (float)std::numeric_limits<qint8>::max();
			for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++) data[n][i] = (*(qint8*)(buffer.data() + i*channels+n))*k;
			break;
		case Datum16UBits:
			k /= (float)std::numeric_limits<quint16>::max();
			if (datumAlign == LittleEndian)
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					data[n][i] = ((*(quint16*)(buffer.data() + 2*(i*channels+n)))*k - .5f) * 2;
			else
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					data[n][i] = (((quint16)(buffer.data()[2*(i*channels+n) + 1] << 8) + (quint16)buffer.data()[2*(i*channels+n)])*k - .5f) * 2;
			break;
		case Datum16SBits:
			k /= (float)std::numeric_limits<qint16>::max();
			if (datumAlign == LittleEndian)
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					data[n][i] = (*(qint16*)(buffer.data() + 2*(i*channels+n)))*k;
			else
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					data[n][i] = (((qint16)buffer.data()[2*(i*channels+n) + 1] << 8) + (qint16)buffer.data()[2*(i*channels+n)])*k;
			break;
		case Datum32UBits:
			k /= (float)std::numeric_limits<quint32>::max();
			if (datumAlign == LittleEndian)
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					data[n][i] = ((*(quint32*)(buffer.data() + 4*(i*channels+n)))*k - .5f) * 2;
			else
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					data[n][i] = ((((quint32)buffer.data()[4*(i*channels+n) + 3] << 24) + ((quint32)buffer.data()[4*(i*channels+n) + 2] << 16)
							+ ((quint32)buffer.data()[4*(i*channels+n) + 1] << 8) + (quint32)buffer.data()[4*(i*channels+n)])*k - .5f) * 2;
			break;
		case Datum32SBits:
			k /= (float)std::numeric_limits<qint32>::max();
			if (datumAlign == LittleEndian)
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					data[n][i] = (*(qint32*)(buffer.data() + 4*(i*channels+n)))*k;
			else
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					data[n][i] = (((qint32)buffer.data()[4*(i*channels+n) + 3] << 24) + ((qint32)buffer.data()[4*(i*channels+n) + 2] << 16)
							+ ((qint32)buffer.data()[4*(i*channels+n) + 1] << 8) + (qint32)buffer.data()[4*(i*channels+n)])*k;
			break;
		case DatumFloat:
			if (datumAlign == LittleEndian)
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					data[n][i] = *(float*)(buffer.data() + 4*(i*channels+n));
			else
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					*(quint32*)(data[n].data() + i) = ((quint32)buffer.data()[4*(i*channels+n) + 3] << 24) + ((quint32)buffer.data()[4*(i*channels+n) + 2] << 16)
							+ ((quint32)buffer.data()[4*(i*channels+n) + 1] << 8) + (quint32)buffer.data()[4*(i*channels+n)];
			break;
		case DatumDouble:
			if (datumAlign == LittleEndian)
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					data[n][i] = *(double*)(buffer.data() + 8*(i*channels+n));
			else
				for (quint32 i = 0, ie = data[0].size(); i < ie; i++) for (quint32 n = 0; n < channels; n++)
					*(quint64*)(data[n].data() + i) = ((quint64)buffer.data()[8*(i*channels+n) + 7] << 56) + ((quint64)buffer.data()[8*(i*channels+n) + 6] << 48)
						+ ((quint64)buffer.data()[8*(i*channels+n) + 5] << 40) + ((quint64)buffer.data()[8*(i*channels+n) + 4] << 32) + ((quint64)buffer.data()[8*(i*channels+n) + 3] << 24)
						+ ((quint64)buffer.data()[8*(i*channels+n) + 2] << 16) + ((quint64)buffer.data()[8*(i*channels+n) + 1] << 8) + (quint64)buffer.data()[8*(i*channels+n)];
			break;
		default:
			assert(false);
	}
	data_ready();
}

bool SerialPortDevice::is_open() const {
	return port->thisPort.isOpen();
}

void SerialPortDevice::save_settings(std::ostream &os) const {
	os.write("SPDV", 4);
	quint32 tmp;
	bool state;
	os.write((char*)&datumType, 4);
	os.write((char*)&datumAlign, 4);
	os.write((char*)&(tmp = get_data_size()), 4);
	os.write((char*)&(tmp = get_channels()), 4);
	os.write((char*)&softwareGain, 4);
	os.write((char*)&(tmp = port->get_port_settings().portName.toUtf8().size()), 4);
	os.write((char*)&(tmp = sizeof(port->get_port_settings().portSettings)), 4);
	os.write((char*)&(state = is_open()), 1);
	os.write(port->get_port_settings().portName.toUtf8().data(), port->get_port_settings().portName.toUtf8().size());
	os.write((char*)&(port->get_port_settings().portSettings), sizeof(port->get_port_settings().portSettings));
}


void SerialPortDevice::load_settings(std::istream &is) {
	char header[33];
	SerialPortInfo tmpSet;
	is.read(header, 33);
	if(strncmp(header, "SPDV", 4) || is.fail()) throw std::runtime_error ("");
	datumType = *(quint32*)(header+4);
	datumAlign = *(quint32*)(header+8);
	set_data_size(*(quint32*)(header+12));
	set_channels(*(quint32*)(header+16));
	softwareGain = *(float*)(header+20);
	char* stmp = new char [*(quint32*)(header+24)];
	is.read(stmp, *(quint32*)(header+24));
	tmpSet.portName = QByteArray (stmp, *(quint32*)(header+24));
	delete stmp;
	stmp = new char [*(quint32*)(header+28)];
	is.read(stmp, *(quint32*)(header+28));
	tmpSet.portSettings = *(SerialPortSettings*)stmp;
	delete stmp;
	port->set_port_settings(tmpSet);
	if (*(bool*)(header+32)) open_port();
}

