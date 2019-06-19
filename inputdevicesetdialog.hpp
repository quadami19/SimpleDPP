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

#ifndef INPUTDEVICESETDIALOG_HPP
#define INPUTDEVICESETDIALOG_HPP

#include <QtWidgets>
#include <sstream>
#include "core.hpp"

class AudioDeviceSettings;
class UARTDeviceSettings;

class InputSetDialog : public QDialog {

		Q_OBJECT
		Core* coreClass;

		QLabel* inputDevLabel;
		QComboBox* inputDevSelect;
		QLabel* audioSetLabel;
		QPushButton* audioSetPB;
		AudioDeviceSettings* audioSetDial;
		QList<QAudioDeviceInfo> audioDevices;
		quint32 audioDeviceIndex = 0;
		quint32 audioSampleRate = 44100;
		quint32 audioChannelCount = 1;

		QLabel* uartSetLabel;
		QPushButton* uartSetPB;
		UARTDeviceSettings* uartSetDial;
		QList<QSerialPortInfo> uartPorts;
		SerialPortSettings uartSettings;
		quint32 uartPortIndex = 0;
		quint32 uartChannels = 1;

		QLabel* dataTypeLabel;
		QComboBox* dataTypeCB;
		QLabel* endianLabel;
		QComboBox* endianCB;

		QLabel* softGainLabel;
		QDoubleSpinBox* softGainDSBox;
		QLabel* bufferSizeLabel;
		QComboBox* bufferSizeCB;
		QLabel* spectrumSizeLabel;
		QComboBox* spectrumSizeCB;
		QPushButton* acceptButton;
		QPushButton* cancelButton;

		QGridLayout* mainLayout;
		void load_devs();
		void load_settings();

	protected:

		void showEvent(QShowEvent *);

	public:
		InputSetDialog(Core* _core, QWidget* _parent = 0);
		virtual ~InputSetDialog();

	private slots:
		void accept();
		void inp_dev_changed();
		void chk_values ();
		void aud_set_show();
		void uart_set_show();
};

class DeviceSettingsDialog : public QDialog {
		Q_OBJECT

	protected:
		QPushButton* acceptPB;
		QPushButton* rejectPB;
		QGridLayout* mainLayout;

	public:
		DeviceSettingsDialog (QWidget* parent = 0x0);
		~DeviceSettingsDialog () {}

	public slots:
		virtual void accept() = 0;
};

class AudioDeviceSettings : public DeviceSettingsDialog {
		Q_OBJECT
		QLabel* currAuDevLabel;
		QComboBox* currAuDevCB;
		QLabel* availSampleRatesLabel;
		QComboBox* availSampleRatesCB;
		QLabel* availChanCountLabel;
		QComboBox* availChanCountCB;
		QList<QAudioDeviceInfo>* devs;
		quint32* devIndex;
		quint32* sampleRate;
		quint32* chanCount;

	public:
		AudioDeviceSettings (QWidget* parent = 0x0);
		~AudioDeviceSettings() {}

		void set_curr_settings (QList<QAudioDeviceInfo>* _devs, quint32* _devIndex, quint32* _sampleRate, quint32* _chanCount);

	private slots:
		void accept();
		void device_changed();
};

class UARTDeviceSettings : public DeviceSettingsDialog {
		Q_OBJECT
		QLabel* devLabel;
		QComboBox* devCB;
		QLabel* baudLabel;
		QComboBox* baudCB;
		QLabel* dataBitsLabel;
		QComboBox* dataBitsCB;
		QLabel* parityLabel;
		QComboBox* parityCB;
		QLabel* stopBitsLabel;
		QComboBox* stopBitsCB;
		QLabel* flowControlLabel;
		QComboBox* flowControlCB;
		QLabel* channelsLabel;
		QSpinBox* channelsSB;
		QLabel* portStatus;
		QPushButton* openPortPB;
		QList<QSerialPortInfo>* devs;
		SerialPortSettings* settings;
		quint32* devIndex;
		quint32* channels;
		Core* corePtr;

		void showEvent(QShowEvent *);
		void update_status ();
		void enable (bool enable);
		SerialPortInfo read_widgets();
		QString read_port()
			{ return devCB->currentText(); }


	public:
		UARTDeviceSettings (Core* _core, QWidget* parent = 0x0);
		~UARTDeviceSettings() {}

		void set_curr_settings (QList<QSerialPortInfo>* _devs, SerialPortSettings* _settings, quint32* _devIndex, quint32* _channels);

	private slots:
		void accept();
		void device_changed();
		void open_port();
};


#endif // INPUTDEVICESETDIALOG_HPP
