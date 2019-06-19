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

#ifndef AUDIODETECTOR_HPP
#define AUDIODETECTOR_HPP

#include <iostream>
#include <limits>
#include <cassert>
#include <QIODevice>
#include <QObject>
#include <QAudioInput>
#include "datum_types.hpp"

class AudioDetector;

class AudioInputDevice : public QIODevice {
	Q_OBJECT

		AudioDetector* m_parent;
		QByteArray buffer;
		quint32 buffsize;

	protected:

		virtual qint64 readData(char *data, qint64 maxlen);
		virtual qint64 writeData(const char *data, qint64 len);

	public:

		AudioInputDevice(QObject* _parent, QAudioInput* _device, quint32 _buffsize);
		~AudioInputDevice();

		bool get_state () { return QIODevice::isOpen(); }
		void start();
		void stop();


	signals:
		void data_ready(QByteArray data);


};

class AudioDetector : public QObject {
		Q_OBJECT

		friend class AudioInputDevice;
		std::vector<std::vector<float>> data;
		QAudioInput* audioInput = 0x0;
		AudioInputDevice* audioInputDevice = 0x0;
		QAudioFormat input_settings;
		QAudioDeviceInfo currDev = QAudioDeviceInfo::defaultInputDevice();

		//quint32 currBufferPos = 0;
		float softwareGain = 1.f;
		quint32 datumType = Datum32SBits;
		quint32 datumAlign = LittleEndian;

		void set_data (const char* dt, qint64 len);
		void update_settings();

		quint32 get_buffer_size();

	public:

		AudioDetector(quint32 dataSize);
		//void start ();
		//void stop ();
		void set_sample_rate (quint32 sample_rate);
		quint32 get_sample_rate () const { return input_settings.sampleRate(); }
		void set_channels (quint32 channels);
        quint32 get_channels () const { if (input_settings.channelCount() > 0) return input_settings.channelCount(); else return 0; }
		void set_software_gain (float sGain) { softwareGain = sGain; }
		void set_datum_type (quint32 _datum_type);
		quint32 get_datum_type () const;
		void set_datum_align (quint32 _datum_align);
		quint32 get_datum_align () const;
		float get_software_gain () { return softwareGain; }
		void set_data_size (quint32 _dataSize);
		void set_audio_device (const QAudioDeviceInfo& dev);
		std::vector<float> const* get_data (quint32 channel) const { return &data[channel]; }
		bool get_state () const { return audioInputDevice->get_state(); }
		const QAudioDeviceInfo& get_curr_audev () { return currDev; }
		void save_settings (std::ostream& os) const;
		void load_settings (std::istream& is);

		~AudioDetector();

	signals:

		void data_ready ();
		void change_state (bool newstate);

	public slots:

		void start();
		void stop();

	private slots:
		void receive_data(QByteArray buffer);


};

#endif // AUDIODETECTOR_HPP
