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

#ifndef PROCESSINGSETTINGS
#define PROCESSINGSETTINGS

#include "processing.hpp"

class ProcessingThread::Settings {
	public:
		Settings();
		Settings(const Settings&);
		~Settings() {}
		virtual Settings& operator= (const Settings&);
		static std::shared_ptr<Settings> get_copy(const Settings&);

		std::shared_ptr<PulseSearching::SearchSettings> sSet;
		std::shared_ptr<PulseDiscriminator::DiscSettings> dSet;
		std::shared_ptr<PulseAmplMeasuring::AmplSettings> aSet;
		std::shared_ptr<PulseTimeMeasuring::TimeSettings> tSet;
		std::vector<float> shape;
		quint32 pulseSize = 32;
		float amplCorrection = 1.f;
		quint32 inputNum = 0;
		bool enableSub = false;
		virtual quint32 get_settings_id () const = 0;
};

class ProcessingStandartCircuit::StanCircuitSettings : public Settings {
	public:
		StanCircuitSettings () : Settings() {}
		StanCircuitSettings (const Settings & s);
		virtual Settings& operator= (const Settings&);
		quint32 get_settings_id() const { return ProcessingThread::StandartCircuit; }
};

class ProcessingCoincidenceCircuit::CoinCircuitSettings : public Settings {
	public:
		CoinCircuitSettings () : Settings() {}
		CoinCircuitSettings (const Settings & s);
		virtual Settings& operator= (const Settings&);
		quint32 get_settings_id() const { return ProcessingThread::CoincidenceCircuit; }
		quint32 coinIndex = 0;
		float amplitudeIntervalL = 0.f;
		float amplitudeIntervalR = 1.f;
		float maxTimeDifference = 5.f;

};

class PulseSearching::SearchSettings {
	public:
		quint32 skipSamples = 32;
		quint32 virtual get_s_settings_id() const = 0;
		static std::shared_ptr<SearchSettings> get_copy (const std::shared_ptr<SearchSettings>& search);
		static std::shared_ptr<SearchSettings> get_new (quint32 num);
};

class PulseSearchingThreshold::SearchThresholdSettings : public SearchSettings {
	public:
		SearchThresholdSettings() : SearchSettings() {}
		SearchThresholdSettings(const SearchSettings&);

		float threshold = 0.05f;
		quint32 detectBaselineSamples = 3;
		quint32 detectPos = 7;
		quint32 get_s_settings_id() const { return PulseSearching::Threshold; }
};

class PulseSearchingMonoton::SearchMonotonSettings : public SearchSettings {
	public:
		SearchMonotonSettings() : SearchSettings() {}
		SearchMonotonSettings(const SearchSettings&);

		quint32 risingSamples = 8;
		quint32 indiffSamples = 0;
		quint32 fallingSamples = 0;
		quint32 get_s_settings_id() const { return PulseSearching::Monoton; }
};

class PulseSearchingTanThreshold::SearchTanThresholdSettings : public SearchSettings {
	public:
		SearchTanThresholdSettings() : SearchSettings() {}
		SearchTanThresholdSettings(const SearchSettings&);

		quint32 risingSamples = 8;
		quint32 indiffSamples = 0;
		quint32 fallingSamples = 0;
		float risingTan = .05f;
		float fallingTan = .05f;
		quint32 get_s_settings_id() const { return PulseSearching::TanThreshold; }
};

class PulseDiscriminator::DiscSettings {
	public:
		bool enabled = false;
		quint32 virtual get_d_settings_id() const = 0;
		static std::shared_ptr<DiscSettings> get_copy (const std::shared_ptr<DiscSettings>& discr);
		static std::shared_ptr<DiscSettings> get_new (quint32 num);
};

class PulseDiscriminatorByDispersion::DispDiscSettings : public DiscSettings {
	public:
		DispDiscSettings() : DiscSettings() { }
		DispDiscSettings(const DiscSettings&);

		float maxDispersion = 1.f;
		quint32 get_d_settings_id() const { return PulseDiscriminator::Dispersion; }
};

class PulseDiscriminatorByNeuralNet::DiscNNSettings : public DiscSettings {
	public:
		DiscNNSettings() : DiscSettings() {}
		DiscNNSettings(const DiscSettings&);

		Neural_Network::NuclearPhysicsNeuralNet neuralNet;
		quint32 get_d_settings_id() const { return PulseDiscriminator::NeuralNet; }
};

class PulseAmplMeasuring::AmplSettings {
	public:
		quint32 processBaselineSamples = 6;
		quint32 virtual get_a_settings_id() const = 0;
		static std::shared_ptr<AmplSettings> get_copy (const std::shared_ptr<AmplSettings>& ampl);
		static std::shared_ptr<AmplSettings> get_new (quint32 num);
};

class PulseAmplMeasuringByMax::AmplMaxSettings : public AmplSettings {
	public:
		AmplMaxSettings() : AmplSettings() {}
		AmplMaxSettings(const AmplSettings&);

		quint32 maxValIntervalLeft = 7;
		quint32 maxValIntervalRight = 32;
		quint32 get_a_settings_id() const { return PulseAmplMeasuring::MaxVal; }
};

class PulseAmplMeasuringPolyMax::AmplPolyMaxSettings : public AmplSettings {
	public:
		AmplPolyMaxSettings() : AmplSettings() {}
		AmplPolyMaxSettings(const AmplSettings& a);

		quint32 maxValIntervalLeft = 7;
		quint32 maxValIntervalRight = 32;
		quint32 polyOrder = 2;
		quint32 get_a_settings_id() const { return PulseAmplMeasuring::Polynomial; }
};

class PulseAmplMeasuringNeuralNet::AmplNNSettings : public AmplSettings {
	public:
		AmplNNSettings() : AmplSettings() {}
		AmplNNSettings(const AmplSettings&a);

		Neural_Network::NuclearPhysicsNeuralNet neuralNet;
		quint32 get_a_settings_id() const { return PulseAmplMeasuring::NeuralNet; }
};

class PulseTimeMeasuring::TimeSettings {
	public:
		quint32 virtual get_t_settings_id() const = 0;
		static std::shared_ptr<TimeSettings> get_copy (const std::shared_ptr<TimeSettings>& discr);
		static std::shared_ptr<TimeSettings> get_new (quint32 num);
};

class PulseTimeMeasuringByMax::TimeMaxSettings : public TimeSettings {
	public:
		TimeMaxSettings() : TimeSettings() {}
		TimeMaxSettings(const TimeSettings& t);

		quint32 maxValIntervalLeft = 0;
		quint32 maxValIntervalRight = 32;
		quint32 get_t_settings_id() const { return PulseTimeMeasuringByMax::MaxVal; }
};

class PulseTimeMeasuringNeuralNet::TimeNNSettings : public TimeSettings {
	public:
		TimeNNSettings() : TimeSettings() {}
		TimeNNSettings(const TimeSettings& t);

		Neural_Network::NuclearPhysicsNeuralNet neuralNet;
		quint32 get_t_settings_id() const { return PulseTimeMeasuringNeuralNet::NeuralNet; }
};

#endif // PROCESSINGSETTINGS

