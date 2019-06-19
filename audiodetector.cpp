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

#include "audiodetector.hpp"

AudioDetector::AudioDetector (quint32 dataSize) {

	input_settings.setSampleRate(44100);
	input_settings.setSampleSize(32);
	input_settings.setChannelCount(1);
	input_settings.setSampleType(QAudioFormat::SignedInt);
	input_settings.setByteOrder(QAudioFormat::LittleEndian);
	input_settings.setCodec("audio/pcm");

	if (!currDev.isFormatSupported(input_settings)) {
		std::cout << "Selected format don't supported. Trying to use nearest.\n";
		input_settings = currDev.nearestFormat(input_settings);
	}
	audioInput = new QAudioInput (currDev, input_settings, this);
	audioInputDevice = new AudioInputDevice (this, audioInput, dataSize*4);
	audioInput->start(audioInputDevice);
	audioInput->suspend();
	change_state(false);

	data.push_back(std::vector<float> (dataSize));
	connect (audioInputDevice, SIGNAL(data_ready(QByteArray)), this, SLOT(receive_data(QByteArray)));
}

void AudioDetector::update_settings() {
	disconnect (audioInputDevice, SIGNAL(data_ready(QByteArray)), this, SLOT(receive_data(QByteArray)));
	audioInput->stop();
	audioInputDevice->stop();
	delete audioInput;
	delete audioInputDevice;

	if (!currDev.isFormatSupported(input_settings)) {
		std::cout << "Selected format don't supported. Trying to use nearest.\n";
		input_settings = currDev.nearestFormat(input_settings);
	}
	audioInput = new QAudioInput (currDev, input_settings, this);
	audioInputDevice = new AudioInputDevice (this, audioInput, get_buffer_size());
	audioInput->start(audioInputDevice);
	audioInput->suspend();
	change_state(false);
	connect (audioInputDevice, SIGNAL(data_ready(QByteArray)), this, SLOT(receive_data(QByteArray)));
}

quint32 AudioDetector::get_buffer_size() {
	quint32 bufSize = data[0].size()*data.size();
	if (datumType == Datum32UBits || datumType == Datum32SBits || datumType == DatumFloat) bufSize *= 4;
	else if (datumType == Datum16UBits || datumType == Datum16SBits) bufSize *= 2;
	else if (datumType == Datum8UBits || datumType == Datum8SBits) bufSize *= 1;
	else if (datumType == DatumDouble) bufSize *= 8;
	else assert(false);
	return bufSize;
}

AudioDetector::~AudioDetector() {
	delete audioInput;
	delete audioInputDevice;
}

void AudioDetector::start() {
	audioInput->resume();
	audioInputDevice->start();
	change_state(true);
}

void AudioDetector::stop() {
	audioInput->suspend();
	audioInputDevice->stop();
	change_state(false);
}

void AudioDetector::set_sample_rate(quint32 sample_rate) {
	if ((quint32)input_settings.sampleRate() == sample_rate) return;
	input_settings.setSampleRate(sample_rate);
	update_settings();
}

void AudioDetector::set_channels(quint32 channels) {
	if ((quint32)input_settings.channelCount() == channels) return;
	if (channels == 0) assert(false);
	input_settings.setChannelCount(channels);
	while (channels < data.size()) {
		data.pop_back();
	}
	while (channels > data.size()) {
		data.push_back(std::vector<float> (data[0].size(), 0.f));
	}

	update_settings();
}

void AudioDetector::set_datum_type(quint32 _datumType) {
	if (datumType == _datumType) return;
	datumType = _datumType;
	if (datumType == Datum8UBits || datumType == Datum16UBits || datumType == Datum32UBits) input_settings.setSampleType(QAudioFormat::UnSignedInt);
	else if (datumType == Datum8SBits || datumType == Datum16SBits || datumType == Datum32SBits) input_settings.setSampleType(QAudioFormat::SignedInt);
	else if (datumType == DatumFloat || datumType == DatumDouble) input_settings.setSampleType(QAudioFormat::Float);
	else assert(false);
	if (datumType == Datum32UBits || datumType == Datum32SBits || datumType == DatumFloat) input_settings.setSampleSize(32);
	else if (datumType == Datum16UBits || datumType == Datum16SBits) input_settings.setSampleSize(16);
	else if (datumType == Datum8UBits || datumType == Datum8SBits) input_settings.setSampleSize(8);
	else if (datumType == DatumDouble) input_settings.setSampleSize(64);
	else assert(false);
	update_settings();
	if (input_settings.sampleType() == QAudioFormat::UnSignedInt) {
		if (input_settings.sampleSize() == 32) datumType = Datum32UBits;
		else if (input_settings.sampleSize() == 16) datumType = Datum16UBits;
		else if (input_settings.sampleSize() == 8) datumType = Datum8UBits;
		else assert(false);
	} else if (input_settings.sampleType() == QAudioFormat::SignedInt) {
		if (input_settings.sampleSize() == 32) datumType = Datum32SBits;
		else if (input_settings.sampleSize() == 16) datumType = Datum16SBits;
		else if (input_settings.sampleSize() == 8) datumType = Datum8SBits;
		else assert(false);
	} else if (input_settings.sampleType() == QAudioFormat::Float) {
		if (input_settings.sampleSize() == 64) datumType = DatumDouble;
		else if (input_settings.sampleSize() == 32) datumType = DatumFloat;
		else assert(false);
	} else assert(false);
}

quint32 AudioDetector::get_datum_type() const {
	return datumType;
}

void AudioDetector::set_datum_align(quint32 _datumAlign) {
	if (datumAlign == _datumAlign) return;
	datumAlign = _datumAlign;
	if (datumAlign == LittleEndian) input_settings.setByteOrder(QAudioFormat::LittleEndian);
	else if (datumAlign == BigEndian) input_settings.setByteOrder(QAudioFormat::BigEndian);
	else assert(false);
	update_settings();
	if (input_settings.byteOrder() == QAudioFormat::LittleEndian) datumAlign = LittleEndian;
	else if (input_settings.byteOrder() == QAudioFormat::BigEndian) datumAlign = BigEndian;
	else assert(false);
}

quint32 AudioDetector::get_datum_align() const {
	return datumAlign;
}

void AudioDetector::set_data_size(quint32 _dataSize) {
	if (data[0].size() != _dataSize) {
		for (quint32 i = 0; i < data.size(); i++) {
			data[i].resize (_dataSize, 0.f);
		}
	}
}

void AudioDetector::set_audio_device(const QAudioDeviceInfo &dev) {
	currDev = dev;
	update_settings();
}

void AudioDetector::receive_data(QByteArray buffer) {
	float k (softwareGain);
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

void AudioDetector::save_settings(std::ostream &os) const {
	os.write("AUST", 4);
	quint32 tmp;
	os.write((char*)&(tmp = input_settings.sampleRate()), 4);
	os.write((char*)&(tmp = input_settings.channelCount()), 4);
	os.write((char*)&softwareGain, 4);
	os.write((char*)&(tmp = data[0].size()), 4);
	os.write((char*)&(tmp = currDev.deviceName().toUtf8().size()), 4);
	os.write(currDev.deviceName().toUtf8().data(), tmp);
}

void AudioDetector::load_settings(std::istream &is) {
	char header [24];
	is.read(header, 24);
	if(strncmp(header, "AUST", 4) || is.fail()) throw std::runtime_error ("");
	input_settings.setSampleRate(*(quint32*)(header+4));
	softwareGain = *(float*)(header+12);
	char* stmp = new char [*(quint32*)(header+20)];
	is.read(stmp, *(quint32*)(header+20));
	auto devs = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
	for (auto &a: devs) {
		if(!strncmp(a.deviceName().toUtf8().data(), stmp, *(quint32*)(header+20))) {
			currDev = a;
		}
	}
	if (*(quint32*)(header+8)) {
		if (data.empty()) data.resize(1);
		data[0].resize (*(quint32*)(header+16));
	}
	set_channels(*(quint32*)(header+8));
	delete [] stmp;
}

AudioInputDevice::AudioInputDevice (QObject *_parent, QAudioInput *_device, quint32 _buffsize) : QIODevice (_parent) {
	Q_UNUSED (_device);
	m_parent = (AudioDetector*) _parent;
	buffsize = _buffsize;
}

AudioInputDevice::~AudioInputDevice() {

}

qint64 AudioInputDevice::writeData(const char *data, qint64 len) {
	buffer.append(data, len);
	if ((quint32)buffer.size() >= buffsize) {
		data_ready(buffer);
		buffer = QByteArray (buffer.data() + buffsize, buffer.size() - buffsize);
	}
	return len;
}

qint64 AudioInputDevice::readData(char *data, qint64 maxlen) {
	Q_UNUSED (data);
	Q_UNUSED (maxlen);
	return maxlen;
}

void AudioInputDevice::start() {
	open (QIODevice::ReadWrite);
}

void AudioInputDevice::stop() {
	close();
}
