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

#include "nuclteachingclass.hpp"

namespace Neural_Network {

NuclTeachingClass::NuclTeachingClass() : TeachingClass(), fft(128) {
	gauChance = new std::normal_distribution<float> (0.f, 1.f);
	linChance = new std::uniform_real_distribution<float> (0.f, 1.f);
	generator.seed(std::time(0));

	sincTab.resize(interMult*interPrec);
	for (int32_t i = 0; i < (int32_t)sincTab.size(); i++) {
		float t = 3.1415f*(float)(i-(int32_t)sincTab.size()/2)/interMult;
		if (t*t > 0.01) sincTab[i] = std::sin(t)/t;
		else sincTab[i] = 1.f;
	}
}

NuclTeachingClass::~NuclTeachingClass () {
	delete gauChance;
	delete linChance;
}

std::vector<float> NuclTeachingClass::get_random(quint32 size) {
	std::vector<float> out(size);
	for (quint32 i = 0; i < size; i++) out[i] = (*gauChance)(generator);
	return out;
}

void NuclTeachingClass::init_noizes() {
	assert(pulse.size());
	noizeAFCs.clear();
	for (auto a = ni.begin(); a != ni.end(); a++) {
		std::vector<float> noize(pulse.size()/2);
		if (a->order == 0) for (quint32 i = 0, ie = noize.size(); i < ie; i++) noize[i] = a->magnitude;
		else if (a->order < 0) for (quint32 i = 0, ie = noize.size(); i < ie; i++) noize[i] = a->magnitude/(a->diff + std::pow((float)i/(float)ie, -a->order) + 0.01);
		else if (a->order > 0) for (quint32 i = 0, ie = noize.size(); i < ie; i++) noize[i] = a->magnitude*(a->diff + std::pow((float)i/(float)ie, a->order));
		noizeAFCs.push_back(noize);
	}
}

void NuclTeachingClass::add_noize(std::vector<float>::iterator iter, quint32 size) {
	fft.set_size(size);
	std::vector<float> noizeBuff(size, 0.f);
	for (auto a = noizeAFCs.begin(); a != noizeAFCs.end(); a++) {
		fft.set_time_data(get_random(size));
		fft.go(false);
		std::vector<float> tmpFilter (*a);
		float tmpAmpl = (*gauChance)(generator);
		for (auto &b: tmpFilter) b *= tmpAmpl;
		fft.filtering(tmpFilter);
		fft.go(true);
		for (quint32 i = 0, ie = noizeBuff.size(); i < ie; i++) noizeBuff[i] += fft.get_time_data()[i].real();
	}
	overlay(iter, noizeBuff);
}
/*
void NuclTeachingClass::add_noize_shaped(std::vector<float>::iterator iter, quint32 size) {
	fft.set_size(size);
	std::vector<float> noizeBuff(size, 0.f);
	for (auto a = noizeAFCs.begin(); a != noizeAFCs.end(); a++) {
		fft.set_time_data(get_random(size));
		fft.go(false);
		std::vector<float> tmpFilter (*a);
		float tmpAmpl = (*gauChance)(generator);
		for (auto &b: tmpFilter) b *= tmpAmpl;
		fft.filtering(tmpFilter);
		fft.go(true);
		for (quint32 i = 0, ie = noizeBuff.size(); i < ie; i++) noizeBuff[i] += fft.get_time_data()[i].real();
	}
	std::vector<float> noizeBuff2(size, 0.f);
	float tmp = std::sqrt(pulse.size());
	for (qint32 i = 0, ie = size; i < ie; ++i) {
		for (qint32 n = -pulse.size()/2, ne = pulse.size()/2; n < ne; ++n) {
			if (n + i < 0) continue;
			else {
				noizeBuff2[i] += pulse[pulse.size()/2-n]*noizeBuff[n + i]/tmp;
			}
		}
	}

	overlay(iter, noizeBuff2);
}
*/
void NuclTeachingClass::overlay(std::vector<float>::iterator iter, const std::vector<float>& input, float magnitude) {
	for (quint32 i = 0, ie = input.size(); i < ie; ++i) {
		*(iter+i) += magnitude*input[i];
	}
}

std::vector<float> NuclTeachingClass::interpolate(const std::vector<float> &input) {
	std::vector<float> output (input.size()*interMult, 0.f);
	std::vector<float> tmp (input.size() + interPrec);

	for (quint32 i = 0; i < interPrec/2; ++i) tmp[i] = input[0];
	for (quint32 i = interPrec/2, ie = input.size() + interPrec/2; i < ie; ++i) tmp[i] = input[i-interPrec/2];
	for (quint32 i = input.size() + interPrec/2, ie = input.size() + interPrec; i < ie; ++i) tmp[i] = input[input.size()-1];

	for (uint32_t i = 0, ie = input.size(); i < ie; i++) {
		uint32_t itmp = (i+1)*interMult - 1;
		for (uint32_t n = 0; n < interMult; n++) {
			output[itmp-n] = 0.f;
			for (int32_t l = 0, le = interPrec; l < le; l++) {
				output[itmp-n] += tmp[i+l] * sincTab[l*interMult+n];

			}
		}
	}
	return output;
}

std::vector<float> NuclTeachingClass::deinterpolate(const std::vector<float> &input, quint32 displ) {
	assert(displ < interMult);
	std::vector<float> output (input.size()/interMult);
	for (quint32 i = 0, ie = output.size(); i < ie; i++) output[i] = input[i*interMult + displ];
	return output;
}

void NuclTeachingClass::set_noize(qint32 order, float magn, float diff) {
	for (auto a = ni.begin(); a != ni.end(); a++) {
		if (a->order == order) {
			a->magnitude = magn;
			a->diff = diff;
			return;
		}
	}
	NoizeInfo n;
	n.order = order;
	n.magnitude = magn;
	n.diff = diff;
	ni.push_back(n);
}

void NuclTeachingClass::set_noize(NoizeInfo n) {
	for (auto a = ni.begin(); a != ni.end(); a++) {
		if (a->order == n.order) {
			*a = n;
			return;
		}
	}
	ni.push_back(n);
}

NoizeInfo NuclTeachingClass::get_noize(qint32 order) {
	for (auto a = ni.begin(); a != ni.end(); a++) {
		if (a->order == order) {
			return *a;
		}
	}
	return NoizeInfo();
}

void NuclTeachingClass::reset_noizes() {
	ni.clear();
}

void NuclTeachingClass::set_pulse(const std::vector<float> &_pulse) {
	assert(!_pulse.empty());
	pulse = _pulse;
	interpolated = interpolate(pulse);
	float max = *std::max_element(interpolated.data(), interpolated.data() + interpolated.size());
	pulMaxTime = (float)(std::max_element(interpolated.data(), interpolated.data() + interpolated.size()) - interpolated.data())/(float)interpolated.size();
	for (quint32 i = 0, ie = pulse.size(); i < ie; ++i) pulse[i] /= max;
	for (quint32 i = 0, ie = interpolated.size(); i < ie; ++i) interpolated[i] /= max;
}

double NuclTeachingClass::start(NuclearPhysicsNeuralNet *_percep, quint32 iterations) {
	assert_nn(_percep);

	teachData.resize(iterations);
	for (auto& a: teachData) {
		a.first.resize(pulse.size());
		a.second.resize(_percep->outputs());
	}
	init_noizes();
	for (quint32 i = 0; i < iterations; ++i) {
		teachData[i] = generate_pulse();
	}
	return TeachingClass::start(_percep);
}

void NuclAmplTeachingClass::assert_nn(NuclearPhysicsNeuralNet *_percep) {
	assert(_percep->outputs() == 1 && _percep->get_function(0) == AvailableFunctions::Amplitude);
}

std::pair<std::vector<float>, std::vector<float>> NuclAmplTeachingClass::generate_pulse() {

	float baseline = .05f*(*gauChance)(generator);
	float baselineLinDrift = 0.05*(*gauChance)(generator)/interpolated.size();

	std::vector<float> a (interpolated.size(), baseline);
	std::vector<float> b (3*interpolated.size(), 0.f);
	std::vector<float> out;
	for (quint32 i = 0, ie = interpolated.size(); i < ie; i++) {
		a[i] += baselineLinDrift*i;
	}

	for (quint32 i = 0, ie = b.size()/3; i < ie; ++i) b[i] = interpolated[0];
	for (quint32 i = 2*b.size()/3, ie = b.size(); i < ie; ++i) b[i] = interpolated[interpolated.size()-1];
	memcpy(b.data() + b.size()/3, interpolated.data(), 4*interpolated.size());

	if ((*linChance)(generator) < 0.35) {
		quint32 overlayPos = b.size()/3 + interpolated.size()*((*linChance)(generator) + 0.25f)/1.25f;
		float overlayMagn = (0.2f + 1.7*(*linChance)(generator));
		overlay(b.begin() + overlayPos, interpolated, overlayMagn);
	}
	/*if ((*linChance)(generator) < 0.25) {
		quint32 overlayPos = interpolated.size()*(*linChance)(generator)/1.25;
		float overlayMagn = (-0.4f + 0.8*(*linChance)(generator));
		overlay(b.begin() + overlayPos, interpolated, overlayMagn);
	}*/

	qint32 pulsePos;
	if (pulMaxTime > 0.25f) pulsePos = interpolated.size()*((*linChance)(generator) - .5f)/2.f;
	else pulsePos = pulMaxTime*interpolated.size()*(2.f*(*linChance)(generator) - 1.f);
	for (quint32 i = 0, ie = a.size(); i < ie; ++i) a[i] += b[i + b.size()/3 - pulsePos];

	float resultedAmpl1 = *std::max_element(a.begin(), a.end());
	for (quint32 i = 0, ie = a.size(); i < ie; ++i) a[i] /= resultedAmpl1;

	out = deinterpolate(a, interMult*(*linChance)(generator));
	add_noize(out.begin(), out.size());
	float resultedAmpl2 = *std::max_element(out.begin(), out.end());
	for (quint32 i = 0, ie = out.size(); i < ie; ++i) out[i] /= resultedAmpl2;

	return std::pair<std::vector<float>, std::vector<float>> (out, std::vector<float> (1, .5f/(resultedAmpl1*resultedAmpl2)));
}

void NuclTimeTeachingClass::assert_nn(NuclearPhysicsNeuralNet *_percep) {
	assert(_percep->outputs() == 1 && _percep->get_function(0) == AvailableFunctions::Time);
}

std::pair<std::vector<float>, std::vector<float>> NuclTimeTeachingClass::generate_pulse() {

	float baseline = .05f*(*gauChance)(generator);
	float baselineLinDrift = 0.05*(*gauChance)(generator)/interpolated.size();

	std::vector<float> a (interpolated.size(), baseline);
	std::vector<float> b (3*interpolated.size(), 0.f);
	std::vector<float> out;
	for (quint32 i = 0, ie = interpolated.size(); i < ie; i++) {
		a[i] += baselineLinDrift*i;
	}

	for (quint32 i = 0, ie = b.size()/3; i < ie; ++i) b[i] = interpolated[0];
	for (quint32 i = 2*b.size()/3, ie = b.size(); i < ie; ++i) b[i] = interpolated[interpolated.size()-1];
	memcpy(b.data() + b.size()/3, interpolated.data(), 4*interpolated.size());

	if ((*linChance)(generator) < 0.35) {
		quint32 overlayPos = b.size()/3 + interpolated.size()*((*linChance)(generator) + 0.25f)/1.25f;
		float overlayMagn = (0.2f + 1.7*(*linChance)(generator));
		overlay(b.begin() + overlayPos, interpolated, overlayMagn);
	}
	/*if ((*linChance)(generator) < 0.25) {
		quint32 overlayPos = interpolated.size()*(*linChance)(generator)/1.25;
		float overlayMagn = (-0.4f + 0.8*(*linChance)(generator));
		overlay(b.begin() + overlayPos, interpolated, overlayMagn);
	}*/

	qint32 pulsePos;
	if (pulMaxTime > 0.25f) pulsePos = interpolated.size()*((*linChance)(generator) - .5f)/2.f;
	else pulsePos = pulMaxTime*interpolated.size()*(2.f*(*linChance)(generator) - 1.f);
	for (quint32 i = 0, ie = a.size(); i < ie; ++i) a[i] += b[i + b.size()/3 - pulsePos];

	float resultedAmpl1 = *std::max_element(a.begin(), a.end());
	for (quint32 i = 0, ie = a.size(); i < ie; ++i) a[i] /= resultedAmpl1;

	out = deinterpolate(a, interMult*(*linChance)(generator));
	add_noize(out.begin(), out.size());
	float resultedAmpl2 = *std::max_element(out.begin(), out.end());
	for (quint32 i = 0, ie = out.size(); i < ie; ++i) out[i] /= resultedAmpl2;

	return std::pair<std::vector<float>, std::vector<float>> (out, std::vector<float> (1, pulMaxTime + (float)pulsePos/(float)interpolated.size()));
}

void NuclDiscrTeachingClass::assert_nn(NuclearPhysicsNeuralNet *_percep) {
	assert(_percep->outputs() == 1 && _percep->get_function(0) == AvailableFunctions::Discriminating);
}

std::pair<std::vector<float>, std::vector<float>> NuclDiscrTeachingClass::generate_pulse() {

	float baseline = .05f*(*gauChance)(generator);
	float baselineLinDrift = 0.05*(*gauChance)(generator)/interpolated.size();

	std::vector<float> a (interpolated.size(), baseline);
	std::vector<float> b (3*interpolated.size(), 0.f);
	std::vector<float> out;
	for (quint32 i = 0, ie = interpolated.size(); i < ie; i++) {
		a[i] += baselineLinDrift*i;
	}

	if ((*linChance)(generator) > 0.5f) {

		for (quint32 i = 0, ie = b.size()/3; i < ie; ++i) b[i] = interpolated[0];
		for (quint32 i = 2*b.size()/3, ie = b.size(); i < ie; ++i) b[i] = interpolated[interpolated.size()-1];
		memcpy(b.data() + b.size()/3, interpolated.data(), 4*interpolated.size());

		if ((*linChance)(generator) < 0.35) {
			quint32 overlayPos = b.size()/3 + interpolated.size()*((*linChance)(generator) + 0.25f)/1.25f;
			float overlayMagn = (0.2f + 1.7*(*linChance)(generator));
			overlay(b.begin() + overlayPos, interpolated, overlayMagn);
		}
		/*if ((*linChance)(generator) < 0.25) {
			quint32 overlayPos = interpolated.size()*(*linChance)(generator)/1.25;
			float overlayMagn = (-0.4f + 0.8*(*linChance)(generator));
			overlay(b.begin() + overlayPos, interpolated, overlayMagn);
		}*/

		qint32 pulsePos;
		if (pulMaxTime > 0.25f) pulsePos = interpolated.size()*((*linChance)(generator) - .5f)/2.f;
		else pulsePos = pulMaxTime*interpolated.size()*(2.f*(*linChance)(generator) - 1.f);
		for (quint32 i = 0, ie = a.size(); i < ie; ++i) a[i] += b[i + b.size()/3 - pulsePos];

		float resultedAmpl1 = *std::max_element(a.begin(), a.end());
		for (quint32 i = 0, ie = a.size(); i < ie; ++i) a[i] /= resultedAmpl1;


		out = deinterpolate(a, interMult*(*linChance)(generator));
		add_noize(out.begin(), out.size());
		float resultedAmpl2 = *std::max_element(out.begin(), out.end());
		for (quint32 i = 0, ie = out.size(); i < ie; ++i) out[i] /= resultedAmpl2;

		return std::pair<std::vector<float>, std::vector<float>> (out, std::vector<float> (1, 0.95));
	} else {

		out = deinterpolate(a, interMult*(*linChance)(generator));
		add_noize(out.begin(), out.size());
		float parabMagn = .05f*(*gauChance)(generator);
		if (parabMagn < 0.) parabMagn = -parabMagn;

		if ((*linChance)(generator) > 0.5f) for (quint32 i = 0, ie = out.size(); i < ie; ++i) {
			float t = (2.f*i - ie)/ie;
			out[i] += parabMagn * (1.f - 2*t*t);
		}

		float resultedAmpl2 = *std::max_element(out.begin(), out.end());
		if (resultedAmpl2 < 0.f) {
			for (quint32 i = 0, ie = out.size(); i < ie; ++i) out[i] -= 1.25*resultedAmpl2;
			resultedAmpl2 = *std::max_element(out.begin(), out.end());
		}

		for (quint32 i = 0, ie = out.size(); i < ie; ++i) out[i] /= resultedAmpl2;
		return std::pair<std::vector<float>, std::vector<float>> (out, std::vector<float> (1, 0.05));
	}
}

}
