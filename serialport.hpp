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

#ifndef SERIALPORT_HPP
#define SERIALPORT_HPP

#include <QtGlobal>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include "datum_types.hpp"

struct SerialPortSettings {
	quint32 baudRate = QSerialPort::Baud9600;
	QSerialPort::DataBits dataBits = QSerialPort::Data8;
	QSerialPort::Parity parity = QSerialPort::NoParity;
	QSerialPort::StopBits stopBits = QSerialPort::OneStop;
	QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
};

struct SerialPortInfo {
	QString portName;
	SerialPortSettings portSettings;
};

class SerialPortDevice;

class SerialPort : public QObject {
		Q_OBJECT
		friend class SerialPortDevice;

		QSerialPort thisPort;
		SerialPortInfo sPInfo;
		quint32 buffsize;
		bool state = false;


	public:
		SerialPort(QObject* parent = 0x0, quint32 _buffsize = 0x800);
		~SerialPort() { emit finished_port(); }
		QString get_port_name ()
			{ return thisPort.portName(); }
		bool set_port_settings (const SerialPortInfo& settings);
		const SerialPortInfo& get_port_settings ()
			{ return sPInfo; }
		void set_buffsize(quint32 bsize)
			{ assert (bsize); buffsize = bsize; }
		quint32 get_buffsize () const
			{ return buffsize; }

	signals:
		void data_ready (QByteArray);
		void finished_port();
		void change_state (bool newstate);

	public slots:
		void read_data();
		void open_port();
		void close_port();
		void start_port();
		void stop_port ();
		void handle_error(QSerialPort::SerialPortError error);

};

class SerialPortDevice : public QObject {
		Q_OBJECT
		std::vector<std::vector<float>> data;
		QByteArray buffer;

		float softwareGain = 1.;
		quint32 datumType = Datum8UBits;
		quint32 datumAlign = LittleEndian;
		SerialPort* port;
		QThread* thread;

		void update_buffsize();

	public:
		explicit SerialPortDevice(QObject *parent = 0, quint32 datasize = 0x800);
		void open() { open_port(); }
		void close() { close_port(); }
		bool get_state () const { return port->state; }


		void set_datum_type (quint32 type)
			{ datumType = type; update_buffsize(); }
		quint32 get_datum_type () const
			{ return datumType; }

		void set_datum_align (quint32 align)
			{ datumAlign = align; }
		quint32 get_datum_align () const
			{ return datumAlign; }

		void set_data_size (quint32 size)
			{ if (data[0].size() != size) for (auto &a: data) a.resize(size); update_buffsize(); }
		quint32 get_data_size () const
			{ return data[0].size(); }

		void set_channels (quint32 channels)
			{ assert (channels); if (data.size() != channels) data.resize(channels, std::vector<float> (data[0].size())); update_buffsize(); }
		quint32 get_channels () const
			{ return data.size(); }

		std::vector<float> const* get_data(quint32 channel) const
			{ return &(data[channel]); }

		bool set_port_settings (const SerialPortInfo& settings)
			{ return port->set_port_settings(settings); }
		const SerialPortInfo& get_port_settings () const
			{ return port->sPInfo; }

		void set_software_gain (float gain)
			{ assert(gain > 0.99); softwareGain = gain; }
		float get_software_gain () const
			{ return softwareGain; }

		bool is_open() const;

		void save_settings (std::ostream& os) const;
		void load_settings (std::istream& is);

	signals:
		void open_port();
		void close_port();
		void start_port();
		void stop_port();
		void data_ready();
		void change_state (bool newstate);

	public slots:

		void start() { start_port(); }
		void stop() { stop_port(); }

	private slots:
		void state_changed(bool newstate)
			{ change_state(newstate); }
		void receive_data (QByteArray dt);

};


#endif // SERIALPORT_HPP
