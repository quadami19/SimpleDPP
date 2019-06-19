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

#include "processingsettings.hpp"

ProcessingThread::Settings::Settings () {
	sSet = std::shared_ptr<PulseSearching::SearchSettings> (new PulseSearchingThreshold::SearchThresholdSettings);
	dSet = std::shared_ptr<PulseDiscriminator::DiscSettings> (new PulseDiscriminatorByDispersion::DispDiscSettings);
	aSet = std::shared_ptr<PulseAmplMeasuring::AmplSettings> (new PulseAmplMeasuringByMax::AmplMaxSettings);
	tSet = std::shared_ptr<PulseTimeMeasuring::TimeSettings> (new PulseTimeMeasuringByMax::TimeMaxSettings);
}

ProcessingThread::Settings::Settings (const Settings &s) {
	sSet = PulseSearching::SearchSettings::get_copy(s.sSet);
	dSet = PulseDiscriminator::DiscSettings::get_copy(s.dSet);
	aSet = PulseAmplMeasuring::AmplSettings::get_copy(s.aSet);
	tSet = PulseTimeMeasuring::TimeSettings::get_copy(s.tSet);
	shape = s.shape;
	pulseSize = s.pulseSize;
	amplCorrection = s.amplCorrection;
	enableSub = s.enableSub;
}

ProcessingThread::Settings& ProcessingThread::Settings::operator = (const ProcessingThread::Settings& s) {
	sSet = PulseSearching::SearchSettings::get_copy(s.sSet);
	dSet = PulseDiscriminator::DiscSettings::get_copy(s.dSet);
	aSet = PulseAmplMeasuring::AmplSettings::get_copy(s.aSet);
	tSet = PulseTimeMeasuring::TimeSettings::get_copy(s.tSet);
	shape = s.shape;
	pulseSize = s.pulseSize;
	amplCorrection = s.amplCorrection;
	inputNum = s.inputNum;
	enableSub = s.enableSub;
	return *this;
}

ProcessingStandartCircuit::StanCircuitSettings::StanCircuitSettings (const Settings & s) : Settings (s) {
	assert (s.get_settings_id() == ProcessingThread::StandartCircuit);
	*this = (StanCircuitSettings&)s;
}

ProcessingThread::Settings& ProcessingStandartCircuit::StanCircuitSettings::operator = (const ProcessingThread::Settings& s) {
	return Settings::operator =(s);
}

ProcessingCoincidenceCircuit::CoinCircuitSettings::CoinCircuitSettings (const Settings & s) : Settings (s) {
	assert (s.get_settings_id() == ProcessingThread::CoincidenceCircuit);
	*this = (CoinCircuitSettings&)s;
}

ProcessingThread::Settings& ProcessingCoincidenceCircuit::CoinCircuitSettings::operator = (const ProcessingThread::Settings& s) {
	const CoinCircuitSettings& tmp = (const CoinCircuitSettings&)s;
	coinIndex = tmp.coinIndex;
	amplitudeIntervalL = tmp.amplitudeIntervalL;
	amplitudeIntervalR = tmp.amplitudeIntervalR;
	maxTimeDifference = tmp.maxTimeDifference;
	return Settings::operator =(s);
}

std::shared_ptr<ProcessingThread::Settings> ProcessingThread::Settings::get_copy(const Settings& s) {
	switch (s.get_settings_id()) {
		case StandartCircuit:
			return std::shared_ptr<Settings> (new ProcessingStandartCircuit::StanCircuitSettings(s));
		case CoincidenceCircuit:
			return std::shared_ptr<Settings> (new ProcessingCoincidenceCircuit::CoinCircuitSettings(s));
		default:
			assert(false);
	}
}

std::shared_ptr<PulseSearching::SearchSettings> PulseSearching::SearchSettings::get_copy(const std::shared_ptr<SearchSettings> &search) {
	switch (search->get_s_settings_id()) {
		case PulseSearching::Threshold:
			return std::shared_ptr<SearchSettings> (new PulseSearchingThreshold::SearchThresholdSettings (*search));
			break;
		case PulseSearching::Monoton:
			return std::shared_ptr<SearchSettings> (new PulseSearchingMonoton::SearchMonotonSettings (*search));
			break;
		case PulseSearching::TanThreshold:
			return std::shared_ptr<SearchSettings> (new PulseSearchingTanThreshold::SearchTanThresholdSettings (*search));
			break;
		default:
			assert(false);
			break;
	}
}

std::shared_ptr<PulseSearching::SearchSettings> PulseSearching::SearchSettings::get_new(quint32 num) {
	switch (num) {
		case PulseSearching::Threshold:
			return std::shared_ptr<SearchSettings> (new PulseSearchingThreshold::SearchThresholdSettings);
			break;
		case PulseSearching::Monoton:
			return std::shared_ptr<SearchSettings> (new PulseSearchingMonoton::SearchMonotonSettings);
			break;
		case PulseSearching::TanThreshold:
			return std::shared_ptr<SearchSettings> (new PulseSearchingTanThreshold::SearchTanThresholdSettings);
			break;
		default:
			assert(false);
			break;
	}
}

PulseSearchingThreshold::SearchThresholdSettings::SearchThresholdSettings (const SearchSettings &s) : SearchSettings () {
	assert (s.get_s_settings_id() == Threshold);
	*this = (const SearchThresholdSettings&)s;
}

PulseSearchingMonoton::SearchMonotonSettings::SearchMonotonSettings (const SearchSettings &s) : SearchSettings () {
	assert (s.get_s_settings_id() == Monoton);
	*this = (const SearchMonotonSettings&)s;
}

PulseSearchingTanThreshold::SearchTanThresholdSettings::SearchTanThresholdSettings (const SearchSettings &s) : SearchSettings () {
	assert (s.get_s_settings_id() == TanThreshold);
	*this = (const SearchTanThresholdSettings&)s;
}

PulseDiscriminatorByDispersion::DispDiscSettings::DispDiscSettings (const DiscSettings & s) : DiscSettings () {
	assert(s.get_d_settings_id() == Dispersion);
	*this = (const DispDiscSettings&)s;
}

PulseDiscriminatorByNeuralNet::DiscNNSettings::DiscNNSettings (const DiscSettings & s) : DiscSettings () {
	assert(s.get_d_settings_id() == NeuralNet);
	*this = (const DiscNNSettings&)s;
}

std::shared_ptr<PulseDiscriminator::DiscSettings> PulseDiscriminator::DiscSettings::get_copy(const std::shared_ptr<DiscSettings> &discr) {
	switch (discr->get_d_settings_id()) {
		case PulseDiscriminator::Dispersion:
			return std::shared_ptr<DiscSettings> (new PulseDiscriminatorByDispersion::DispDiscSettings (*discr));
			break;
		case PulseDiscriminator::NeuralNet:
			return std::shared_ptr<DiscSettings> (new PulseDiscriminatorByNeuralNet::DiscNNSettings (*discr));
			break;
		default:
			assert(false);
			break;
	}
}

std::shared_ptr<PulseDiscriminator::DiscSettings> PulseDiscriminator::DiscSettings::get_new(quint32 num) {
	switch (num) {
		case PulseDiscriminator::Dispersion:
			return std::shared_ptr<DiscSettings> (new PulseDiscriminatorByDispersion::DispDiscSettings);
			break;
		case PulseDiscriminator::NeuralNet:
			return std::shared_ptr<DiscSettings> (new PulseDiscriminatorByNeuralNet::DiscNNSettings);
			break;
		default:
			assert(false);
			break;
	}
}

std::shared_ptr<PulseAmplMeasuring::AmplSettings> PulseAmplMeasuring::AmplSettings::get_copy(const std::shared_ptr<AmplSettings> &ampl) {
	switch (ampl->get_a_settings_id()) {
		case PulseAmplMeasuring::MaxVal:
			return std::shared_ptr<AmplSettings> (new PulseAmplMeasuringByMax::AmplMaxSettings (*ampl));
			break;
		case PulseAmplMeasuring::Polynomial:
			return std::shared_ptr<AmplSettings> (new PulseAmplMeasuringPolyMax::AmplPolyMaxSettings (*ampl));
			break;
		case PulseAmplMeasuring::NeuralNet:
			return std::shared_ptr<AmplSettings> (new PulseAmplMeasuringNeuralNet::AmplNNSettings (*ampl));
			break;
		default:
			assert(false);
			break;
	}
}

std::shared_ptr<PulseAmplMeasuring::AmplSettings> PulseAmplMeasuring::AmplSettings::get_new(quint32 num) {
	switch (num) {
		case PulseAmplMeasuring::MaxVal:
			return std::shared_ptr<AmplSettings> (new PulseAmplMeasuringByMax::AmplMaxSettings);
			break;
		case PulseAmplMeasuring::Polynomial:
			return std::shared_ptr<AmplSettings> (new PulseAmplMeasuringPolyMax::AmplPolyMaxSettings);
			break;
		case PulseAmplMeasuring::NeuralNet:
			return std::shared_ptr<AmplSettings> (new PulseAmplMeasuringNeuralNet::AmplNNSettings);
			break;
		default:
			assert(false);
			break;
	}
}

PulseAmplMeasuringByMax::AmplMaxSettings::AmplMaxSettings (const AmplSettings &a) {
	assert (a.get_a_settings_id() == PulseAmplMeasuring::MaxVal);
	*this = (const AmplMaxSettings&)a;
}

PulseAmplMeasuringPolyMax::AmplPolyMaxSettings::AmplPolyMaxSettings (const AmplSettings & a) : AmplSettings () {
	assert (a.get_a_settings_id() == PulseAmplMeasuring::Polynomial);
	*this = (const AmplPolyMaxSettings&)a;
}

PulseAmplMeasuringNeuralNet::AmplNNSettings::AmplNNSettings (const AmplSettings & a) : AmplSettings () {
	assert (a.get_a_settings_id() == PulseAmplMeasuring::NeuralNet);
	*this = (const AmplNNSettings&)a;
}

std::shared_ptr<PulseTimeMeasuring::TimeSettings> PulseTimeMeasuring::TimeSettings::get_copy(const std::shared_ptr<TimeSettings> &time) {
	switch (time->get_t_settings_id()) {
		case PulseTimeMeasuring::MaxVal:
			return std::shared_ptr<TimeSettings> (new PulseTimeMeasuringByMax::TimeMaxSettings (*time));
			break;
		case PulseTimeMeasuring::NeuralNet:
			return std::shared_ptr<TimeSettings> (new PulseTimeMeasuringNeuralNet::TimeNNSettings (*time));
			break;
		default:
			assert(false);
			break;
	}
}

std::shared_ptr<PulseTimeMeasuring::TimeSettings> PulseTimeMeasuring::TimeSettings::get_new(quint32 num) {
	switch (num) {
		case PulseTimeMeasuring::MaxVal:
			return std::shared_ptr<TimeSettings> (new PulseTimeMeasuringByMax::TimeMaxSettings);
			break;
		case PulseTimeMeasuring::NeuralNet:
			return std::shared_ptr<TimeSettings> (new PulseTimeMeasuringNeuralNet::TimeNNSettings);
			break;
		default:
			assert(false);
			break;
	}
}

PulseTimeMeasuringByMax::TimeMaxSettings::TimeMaxSettings (const TimeSettings & t) : TimeSettings () {
	assert (t.get_t_settings_id() == PulseTimeMeasuring::MaxVal);
	*this = (const TimeMaxSettings&)t;
}

PulseTimeMeasuringNeuralNet::TimeNNSettings::TimeNNSettings (const TimeSettings & t) : TimeSettings () {
	assert (t.get_t_settings_id() == PulseTimeMeasuring::NeuralNet);
	*this = (const TimeNNSettings&)t;
}
