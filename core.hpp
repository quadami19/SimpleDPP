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

#ifndef CORE_HPP
#define CORE_HPP

#include "audiodetector.hpp"
#include "serialport.hpp"
#include "interpolator.hpp"
#include "filtering.hpp"
#include "processing.hpp"

class MainWindow;

class Core : public QObject {

		Q_OBJECT

		AudioDetector * ADClass;
		SerialPortDevice * SPort;
		FilteringProcessor* filtProc;
		InterpolationClass * IPolation;
		PulseProcessing* pulProc;

		MainWindow * mainWinPtr;

		std::shared_ptr<QThread> thisThread;

		std::vector<std::vector<float> const*> rawData;
		std::vector<std::vector<float> const*> filterData;
		std::vector<std::vector<float> const*> outputData;
		std::vector<QString> inputsName;

		quint32 dataBufferSize;
		quint32 currentDevice = AudioDevice;
		quint64 totalPulsesDetected = 0;

		std::vector<Neural_Network::NuclearPhysicsNeuralNet> availableDiscrNNs;
		std::vector<Neural_Network::NuclearPhysicsNeuralNet> availableAmplNNs;
		std::vector<Neural_Network::NuclearPhysicsNeuralNet> availableTimeNNs;

		Core () = delete;
		Core (const Core& _core) = delete;

		void update_channels();

	public:

		enum InputDevice {
			AudioDevice,
			UARTDevice
		};

		explicit Core(quint32 dataSize, quint32 spectrumSize = 0x400, MainWindow* _parent = 0x0);
		virtual ~Core ();
		std::vector<float> const* get_data (quint32 stream) const { return outputData[stream]; }
		void start();
		void stop();

		void set_sample_rate (quint32 sampleRate)
			{ ADClass->set_sample_rate(sampleRate); }
		quint32 get_sample_rate () const
			{ return ADClass->get_sample_rate(); }

		void set_buffer_size (quint32 size);

		quint32 get_buffer_size () const
			{ return dataBufferSize; }

		void set_spectrum_size (quint32 size);

		quint32 get_spectrum_size () const
			{ return pulProc->get_spectrum(0)->size(); }

		void set_freq_collect_enable (bool state, quint32 stream);
		bool is_working () const;

		void set_input_device (quint32 dev);
		quint32 get_input_device ()
			{ return currentDevice; }

		quint32 get_input_streams ()
			{ return rawData.size(); }


		const QString& get_input_name (quint32 input) const
			{ return inputsName[input]; }
		void set_input_name (const QString& name, quint32 input)
			{ inputsName[input] = name; }

		std::vector<float> const* get_raw_data (quint32 input) const
			{ return rawData[input]; }
		std::vector<float> const* get_filter_data (quint32 input) const
			{ return filterData[input]; }
		std::vector<float> const* get_output_data (quint32 input) const
			{ return outputData[input]; }

		/*   AUDIO   */

		void set_audio_device (const QAudioDeviceInfo& dev)
			{ ADClass->set_audio_device(dev); }
		const QAudioDeviceInfo& get_audio_device ()
			{ return ADClass->get_curr_audev(); }

		void set_audio_channels (quint32 channels)
			{ ADClass->set_channels(channels); update_channels(); }
		quint32 get_audio_channels () const
			{ return ADClass->get_channels(); }

		void set_audio_software_gain (float _gain)
			{ ADClass->set_software_gain(_gain); }
		float get_audio_software_gain () const
			{ return ADClass->get_software_gain(); }

		void set_audio_datum_type (quint32 type)
			{ return ADClass->set_datum_type(type); }
		quint32 get_audio_datum_type () const
			{ return ADClass->get_datum_type(); }

		void set_audio_datum_align (quint32 align)
			{ return ADClass->set_datum_align(align); }
		quint32 get_audio_datum_align () const
			{ return ADClass->get_datum_align(); }
		/*   UART   */

		void open_uart_device ()
			{ SPort->open(); }
		void close_uart_device ()
			{ SPort->close(); }
		bool uart_device_is_opened () const
			{ return SPort->is_open(); }

		void set_uart_channels (quint32 channels)
			{ SPort->set_channels(channels); update_channels(); }
		quint32 get_uart_channels () const
			{ return SPort->get_channels(); }

		bool set_uart_settings (const SerialPortInfo& set)
			{ return SPort->set_port_settings(set); }
		const SerialPortInfo& get_uart_settings () const
			{ return SPort->get_port_settings(); }

		void set_uart_datum_type (quint32 type)
			{ return SPort->set_datum_type(type); }
		quint32 get_uart_datum_type () const
			{ return SPort->get_datum_type(); }

		void set_uart_datum_align (quint32 align)
			{ return SPort->set_datum_align(align); }
		quint32 get_uart_datum_align () const
			{ return SPort->get_datum_align(); }

		void set_uart_software_gain (float _gain)
			{ SPort->set_software_gain(_gain); }
		float get_uart_software_gain () const
			{ return SPort->get_software_gain(); }

		/*   Interpolation   */

		void set_inter_settings (InterpolatorSettings set)
			{ IPolation->set_settings(set); }
		InterpolatorSettings get_inter_settings () const
			{ return IPolation->get_settings(); }

		void set_inter_type (quint32 type)
			{ IPolation->set_inter_type(type); }
		quint32 get_inter_type ()
			{ return IPolation->get_inter_type(); }

		bool get_inter_enabled () const
			{ return IPolation->get_inter_enabled(); }
		void set_inter_enabled (bool enable)
			{ IPolation->set_inter_enabled(enable); }

		/*   FILTERING   */

		quint32 get_filters_count (quint32 input) const
			{ return filtProc->get_filter_count(input); }

		void set_filter_enabled (bool state, quint32 input)
			{ filtProc->set_enabled(state, input); }

		bool get_filter_enabled (quint32 input) const
			{ return filtProc->get_enabled(input); }

		void add_filter (quint32 filter_id, quint32 input)
			{ filtProc->add_filter(filter_id, input); }

		void del_filter (quint32 filter, quint32 input)
			{ filtProc->del_filter(input, filter); }

		void set_filter_id (quint32 filter_id, quint32 input, quint32 filter)
			{ filtProc->set_filter_id(filter_id, input, filter); }
		quint32 get_filter_id (quint32 input, quint32 filter) const
			{ return filtProc->get_filter_id(input, filter); }

		void filter_set (std::shared_ptr<Filter::FilterSettings> settings, quint32 input, quint32 filter)
			{ filtProc->set(settings, input, filter); }
		std::shared_ptr<Filter::FilterSettings> filter_get (quint32 input, quint32 filter)
			{ return filtProc->get(input, filter); }

		/*   PROCESSING   */

		void set_process_threads (quint32 threads)
			{ pulProc->set_threads(threads); }
		quint32 get_process_threads () const
			{ return pulProc->get_threads(); }


		//void set_process_enabled (bool value, quint32 stream, quint32 stage)
		//	{ pulProc->set_enabled(value, stream, stage); }
		//bool get_process_enabled (quint32 stream, quint32 stage) const
		//	{ return pulProc->get_enabled(stream, stage); }

		void set_process_name (QString name, quint32 thread)
			{ pulProc->set_process_name(name, thread); }
		QString get_process_name (quint32 thread) const
			{ return pulProc->get_process_name(thread); }
/*
		void set_process_input (quint32 input, quint32 thread)
			{ pulProc->set_input_index(input, thread); }
		quint32 get_process_input (quint32 thread) const
			{ return pulProc->get_input_index(thread); }
*/
		void set_process_settings (std::shared_ptr<ProcessingThread::Settings> sett, quint32 thread)
			{ pulProc->set_settings(sett, thread); }
		ProcessingThread::Settings const* get_process_settings (quint32 thread)
			{ return pulProc->get_settings(thread); }
		void set_process_type (quint32 type, quint32 thread)
			{ pulProc->set_process_type(type, thread); }

		std::vector<quint32> const* get_spectrum (quint32 thread) const
			{ return pulProc->get_spectrum(thread); }

		void set_setup_mode (bool state)
			{ pulProc->set_setup_mode(state); }
		void set_setup_mode_process (quint32 thread)
			{ pulProc->set_setup_thread(thread); }

		std::vector<std::vector<float>> const* get_setup_pulses () const
			{ return pulProc->get_setup_pulses(); }

		std::vector<float> debug_get_proc (quint32 thread) const
			{ return pulProc->get_processed(thread); }

		void reset_spectrum (quint32 thread)
			{ pulProc->reset_spectrum(thread); }

		quint32 get_count_rate (quint32 thread) { return pulProc->get_count_rate(thread); }

		std::vector<Neural_Network::NuclearPhysicsNeuralNet>& get_discr_nn ()
			{ return availableDiscrNNs; }
		std::vector<Neural_Network::NuclearPhysicsNeuralNet>& get_ampl_nn ()
			{ return availableAmplNNs; }
		std::vector<Neural_Network::NuclearPhysicsNeuralNet>& get_time_nn ()
			{ return availableTimeNNs; }


		void save_settings (std::ostream& os);
		void load_settings (std::istream& is);

	signals:
		void state_changed (bool _newstate);
		void start_sig();
		void stop_sig();
		void buff_size_changed ();
		void filter();
		void interpolate();
		void process ();
		void finished ();
		/*
*/
	public slots:
		void toggle_state();
		void receive_data ();
		void filt_finished ();
		void inter_finished();
		void proc_finished();

		void sec_timer_update ()
			{ pulProc->sec_timer_update(); }
};

int get_index (int index);

#endif // CORE_HPP
