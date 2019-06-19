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

#ifndef PROCESSING_HPP
#define PROCESSING_HPP

#include <QtGlobal>
#include <QObject>
#include <QMutex>
#include <functional>
#include <cassert>
#include <fstream>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <memory>
#include <forward_list>
#include "Eigen/Core"
#include "Eigen/LU"
#include "nuclearphysicsperceptron.hpp"

class PulseSearching;
class PulseDiscriminator;
class PulseAmplMeasuring;
class PulseTimeMeasuring;

class ProcessingThread : public QObject{
	Q_OBJECT


	public:
		void run();

		class PulseInfo {
			public:
				quint32 pos;
				float ampl;
				float time;
		};

		class Settings;

		ProcessingThread (quint32 specSize);
		virtual ~ProcessingThread () { deleted();}
		void set_spectrum_size(quint32 size) { spectrum.resize(size, 0); }
		std::vector<quint32> const* get_spectrum () const { return &spectrum; }
		void reset_spectrum () { spectrum = std::vector<quint32> (spectrum.size(), 0); }

		void set_input (std::vector<float> const* inp);
		std::vector<float> get_processed () const;

		void set_settings (std::shared_ptr<Settings> s);
		Settings const* get_settings () const { return settings.get(); }
		void set_name (QString str) { name = str; }
		QString get_name () const { return name; }
		virtual quint32 get_process_type () const = 0;

		void set_pulse_collect (bool mode) { isPulseCollect = mode; }
		void set_spect_collect (bool mode) { isSpectCollect = mode; }
		std::vector<std::vector<float>> const* get_setup_pulses() { return &setupDetectedPulses; }
		void reset_setup_pulses () { setupDetectedPulses.clear(); }

		void sec_tim_update() { countRate = detectedLastSec; detectedLastSec = 0; }
		quint32 get_count_rate () { return countRate; }

		enum ProcessType {
			StandartCircuit,
			CoincidenceCircuit
		};

		virtual void save (std::ostream& os) const;
		virtual void load (std::istream& is);

	signals:
		void pulse_detected (PulseInfo p);
		void deleted();

	protected:
		QMutex mutex;
		std::vector<quint32> spectrum;
		std::vector<float> input;
		QString name;

		std::shared_ptr<PulseSearching> pulSearch;
		std::shared_ptr<PulseDiscriminator> pulDisc;
		std::shared_ptr<PulseAmplMeasuring> pulAmpl;
		std::shared_ptr<PulseTimeMeasuring> pulTime;

		std::shared_ptr<Settings> settings;
		PulseInfo pulShapeInfo;

		PulseInfo lastDetectInfo;

		bool isPulseCollect = false;
		bool isSpectCollect = true;
		std::vector<std::vector<float>> setupDetectedPulses;

		std::function<void ()> callback;

		quint32 detectedLastSec = 0;
		quint32 countRate = 0;

		virtual void update_settings() = 0;
		virtual void process() = 0;
		void subtract (std::vector<float>::iterator begPulse);

		void add_to_spectrum ();


};

class ProcessingStandartCircuit : public ProcessingThread {

		void update_settings() {}

		void process();

	public:
		ProcessingStandartCircuit(quint32 specSize);

		quint32 get_process_type() const { return StandartCircuit; }

		class StanCircuitSettings;
};

class ProcessingCoincidenceCircuit : public ProcessingThread {
		Q_OBJECT

		ProcessingThread* coincidenceWith;
		PulseInfo coinPulse;
		bool sourceNDeleted = true;

		void update_settings();

		void process() {}

	public:
		ProcessingCoincidenceCircuit(quint32 specSize);
		void set_coincidence (ProcessingThread* coincidence);
		ProcessingThread* get_coincidence ()
			{ return coincidenceWith; }

		quint32 get_process_type() const { return CoincidenceCircuit; }

		class CoinCircuitSettings;

		virtual void save(std::ostream &os) const;
		virtual void load(std::istream &is);

	public slots:
		void detect_event(PulseInfo p);
		void source_deleted();
};

class PulseSearching {

	public:
		PulseSearching () {}
		virtual ~PulseSearching () {}
		void search (std::vector<float>::iterator _begin, std::vector<float>::iterator _end, quint32 _begPos);
		void set_callback (std::function<void ()> call) { detectCallback = call; }
		void set_single (bool single) { isSingle = single; }
		quint32 get_pos () const { return pulsePos; }
		std::vector<float>::iterator get_iter () const { return (begin + pulsePos) - begPos; }

		static std::shared_ptr<PulseSearching> get_new (quint32 id);

		virtual void save (std::ostream& os) const = 0;
		virtual void load (std::istream& is) = 0;

		class SearchSettings;

		enum SearchTypes {
			Threshold,
			Monoton,
			TanThreshold
		};

		void set (std::shared_ptr<ProcessingThread::Settings> s);
		ProcessingThread::Settings const* get () const { return settings.get(); }
		virtual quint32 get_search_type () const = 0;

	protected:
		std::shared_ptr<ProcessingThread::Settings> settings;
		quint32 begPos = 0;
		quint32 pulsePos = 0;
		std::vector<float>::iterator begin;
		std::vector<float>::iterator end;
		std::function<void ()> detectCallback;
		bool isSingle = false;
		virtual void update_settings () = 0;
		virtual quint8 check_pulse (std::vector<float>::iterator curr) = 0;


};

class PulseSearchingThreshold : public PulseSearching {

		void update_settings() {}
		quint8 check_pulse(std::vector<float>::iterator curr);

	public:

		PulseSearchingThreshold();
		virtual ~PulseSearchingThreshold() {}

		void save (std::ostream &os) const;
		void load (std::istream &is);

		quint32 get_search_type () const { return PulseSearching::Threshold; }

		class SearchThresholdSettings;
};

class PulseSearchingMonoton : public PulseSearching {

		void update_settings() {}
		quint8 check_pulse(std::vector<float>::iterator curr);

	public:

		PulseSearchingMonoton();
		virtual ~PulseSearchingMonoton() {}

		void save (std::ostream &os) const;
		void load (std::istream &is);

		quint32 get_search_type () const { return PulseSearching::Monoton; }

		class SearchMonotonSettings;
};

class PulseSearchingTanThreshold : public PulseSearching {


		Eigen::Matrix2f riseMatrix;
		Eigen::VectorXf rise0OrderPoly;
		Eigen::VectorXf rise1OrderPoly;
		Eigen::Matrix2f fallMatrix;
		Eigen::VectorXf fall0OrderPoly;
		Eigen::VectorXf fall1OrderPoly;


		void update_settings();
		quint8 check_pulse(std::vector<float>::iterator curr);

	public:

		PulseSearchingTanThreshold();
		virtual ~PulseSearchingTanThreshold() {}

		void save (std::ostream &os) const;
		void load (std::istream &is);

		quint32 get_search_type () const { return PulseSearching::TanThreshold; }

		class SearchTanThresholdSettings;
};

class PulseDiscriminator {

	public:
		PulseDiscriminator () {}
		virtual ~PulseDiscriminator () {}
		virtual quint8 discriminate (std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse) = 0;

		virtual void save (std::ostream& os) const = 0;
		virtual void load (std::istream& is) = 0;

		class DiscSettings;

		enum DiscTypes {
			Dispersion,
			NeuralNet
		};

		void set (std::shared_ptr<ProcessingThread::Settings> d);
		static std::shared_ptr<PulseDiscriminator> get_new (quint32 id);
		ProcessingThread::Settings const* get () const { return settings.get(); }
		virtual quint32 get_disc_type () const = 0;

	protected:
		std::shared_ptr<ProcessingThread::Settings> settings;
		virtual void update_settings () = 0;

};

class PulseDiscriminatorByDispersion : public PulseDiscriminator {

		void update_settings() {}

	public:

		PulseDiscriminatorByDispersion();
		virtual ~PulseDiscriminatorByDispersion() {}

		void save(std::ostream &os) const;
		void load(std::istream &is);

		quint32 get_disc_type() const { return PulseDiscriminator::Dispersion; }

		quint8 discriminate(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse);

		class DispDiscSettings;

};

class PulseDiscriminatorByNeuralNet : public PulseDiscriminator {

		void update_settings() {}

	public:

		PulseDiscriminatorByNeuralNet();
		virtual ~PulseDiscriminatorByNeuralNet() {}

		void save(std::ostream &os) const;
		void load(std::istream &is);

		quint32 get_disc_type() const { return PulseDiscriminator::NeuralNet; }

		quint8 discriminate(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse);

		class DiscNNSettings;

};

class PulseAmplMeasuring {

	public:

		PulseAmplMeasuring() {}
		virtual ~PulseAmplMeasuring() {}

		void measure (std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse);
		float get_ampl() const { return pulseAmpl; }

		virtual void save (std::ostream& os) const = 0;
		virtual void load (std::istream& is) = 0;

		class AmplSettings;

		enum AmplTypes {
			MaxVal,
			Polynomial,
			NeuralNet
		};

		void set (std::shared_ptr<ProcessingThread::Settings> a);
		static std::shared_ptr<PulseAmplMeasuring> get_new (quint32 id);
		ProcessingThread::Settings const* get () const { return settings.get(); }
		virtual quint32 get_ampl_type () const = 0;

	protected:
		float pulseAmpl = 0.;
		std::shared_ptr<ProcessingThread::Settings> settings;
		virtual float find_ampl(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse) = 0;
		virtual void update_settings () = 0;

};

class PulseAmplMeasuringByMax : public PulseAmplMeasuring {

		float find_ampl(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse);
		void update_settings();

	public:

		PulseAmplMeasuringByMax ();
		virtual ~PulseAmplMeasuringByMax () {}

		void save(std::ostream &os) const;
		void load(std::istream &is);

		quint32 get_ampl_type() const { return PulseAmplMeasuring::MaxVal; }

		class AmplMaxSettings;
};

class PulseAmplMeasuringPolyMax : public PulseAmplMeasuring {

		float find_ampl(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse);
		void update_settings();

		std::vector<Eigen::VectorXf*> polyVectors;
		Eigen::MatrixXf polyMatrix;
		Eigen::VectorXf polyValues;
		Eigen::VectorXf polyRestoredPulse;

	public:

		PulseAmplMeasuringPolyMax ();
		virtual ~PulseAmplMeasuringPolyMax () {}

		void save(std::ostream &os) const;
		void load(std::istream &is);

		quint32 get_ampl_type() const { return PulseAmplMeasuring::Polynomial; }

		class AmplPolyMaxSettings;
};

class PulseAmplMeasuringNeuralNet : public PulseAmplMeasuring {

		float find_ampl(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse);
		void update_settings() {}

	public:

		PulseAmplMeasuringNeuralNet ();
		virtual ~PulseAmplMeasuringNeuralNet () {}

		void save(std::ostream &os) const;
		void load(std::istream &is);

		quint32 get_ampl_type() const { return PulseAmplMeasuring::NeuralNet; }

		class AmplNNSettings;
};

class PulseTimeMeasuring {

	public:
		PulseTimeMeasuring () {}
		virtual ~PulseTimeMeasuring () {}
		void measure (std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse)
			{ pulseTime = find_time(begPulse, endPulse); }
		float get_time() const { return pulseTime; }

		virtual void save (std::ostream& os) const = 0;
		virtual void load (std::istream& is) = 0;

		class TimeSettings;

		enum TimeTypes {
			MaxVal,
			NeuralNet
		};

		void set (std::shared_ptr<ProcessingThread::Settings> t);
		static std::shared_ptr<PulseTimeMeasuring> get_new (quint32 id);
		ProcessingThread::Settings const* get () const { return settings.get(); }
		virtual quint32 get_time_type () const = 0;

	protected:
		float pulseTime = 0.;
		std::shared_ptr<ProcessingThread::Settings> settings;

		virtual float find_time(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse) = 0;
		virtual void update_settings () = 0;

};

class PulseTimeMeasuringByMax : public PulseTimeMeasuring {

		float find_time(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse);
		void update_settings();

	public:

		PulseTimeMeasuringByMax();
		virtual ~PulseTimeMeasuringByMax() {}

		void save(std::ostream &os) const;
		void load(std::istream &is);

		quint32 get_time_type() const { return MaxVal; }

		class TimeMaxSettings;
};

class PulseTimeMeasuringNeuralNet : public PulseTimeMeasuring {

		float find_time(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse);
		void update_settings() {}

	public:

		PulseTimeMeasuringNeuralNet();
		virtual ~PulseTimeMeasuringNeuralNet() {}

		void save(std::ostream &os) const;
		void load(std::istream &is);

		quint32 get_time_type() const { return NeuralNet; }

		class TimeNNSettings;
};

class PulseProcessing : public QObject {
		Q_OBJECT

		QMutex mutex;
		std::vector<std::shared_ptr<ProcessingThread>> threads;

		bool setupMode = false;
		quint32 setupStream = 0;

		std::vector<std::vector<float> const*> inputs;

		void update_settings();

	public:
		PulseProcessing (quint32 specSize = 0x200);
		~PulseProcessing();
		void set_inputs (std::vector<std::vector<float> const*> _inputs);
		std::vector<float> get_processed (quint32 index) const
			{ return threads[index]->get_processed(); }
		void set_settings (std::shared_ptr<ProcessingThread::Settings> set, quint32 index);
		ProcessingThread::Settings const* get_settings (quint32 index) const;

		void set_spectrum_size (quint32 size)
			{ for (auto& a: threads) a->set_spectrum_size(size); }
		std::vector<quint32> const* get_spectrum (quint32 thread)
			{ return threads[thread]->get_spectrum(); }
		void reset_spectrum (quint32 thread)
			{ threads[thread]->reset_spectrum(); }

		void set_threads (quint32 size);
		quint32 get_threads () const { return threads.size(); }
		void set_process_type (quint32 type, quint32 thread);
		quint32 get_count_rate (quint32 thread) { return threads[thread]->get_count_rate(); }

		void set_setup_mode (bool mode);
		void set_setup_thread (quint32 thread);

		void set_process_name (QString name, quint32 thread) { threads[thread]->set_name(name); }
		QString get_process_name (quint32 thread) const { return threads[thread]->get_name(); }

		std::vector<std::vector<float>> const* get_setup_pulses();

		void save (std::ostream& os) const;
		void load (std::istream& is);


	public slots:
		void process();
		void sec_timer_update() { for (auto& a: threads) a->sec_tim_update(); }

	signals:
		void finished();

};

#endif // PROCESSING_HPP
