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

#include "interpolator.hpp"

Interpolator::Interpolator() {
}

void Interpolator::set_settings(InterpolatorSettings sett) {
	buffer.resize(2*sett.precision);
	settings = sett;
}

void Interpolator::artifact_reduction() {
	std::vector<float> tmpVec (output.size(), 0.f);
	float ftmp = settings.pointsMult + 1.f;
	for (uint32_t i = settings.pointsMult/2, ie = output.size() - settings.pointsMult/2; i < ie; i++) {
		for (int32_t n = i - settings.pointsMult/2, ne = i + settings.pointsMult/2 + 1; n < ne; n++) {
			tmpVec[i] += output[n];
		}
		tmpVec[i] /= ftmp;
	}
	memcpy(output.data() + settings.pointsMult/2, tmpVec.data() + settings.pointsMult/2, sizeof(float)*(output.size()-settings.pointsMult));
}

void Interpolator::save_settings(std::ostream &os) const {
	os.write((char*)&settings, sizeof(settings));
}

void Interpolator::load_settings(std::istream &is) {
	InterpolatorSettings t;
	is.read((char*)&t, sizeof(t));
	set_settings(t);
}

WhitShanInterpolator::WhitShanInterpolator () {
	set_settings(InterpolatorSettings());
}

void WhitShanInterpolator::tabulate_sinc() {
	sinc_tab.resize(settings.pointsMult*buffer.size());
	for (int32_t i = 0; i < (int32_t)sinc_tab.size(); i++)
		sinc_tab[i] = sinc((float)(i-(int32_t)sinc_tab.size()/2)/settings.pointsMult);
}

void WhitShanInterpolator::set_settings(InterpolatorSettings sett) {
	Interpolator::set_settings(sett);
	tabulate_sinc();
}

void WhitShanInterpolator::interpolate() {
	output.resize(input->size()*settings.pointsMult, 0.f);
	for (uint32_t i = 0, ie = buffer.size(); i < ie; i++) {
		uint32_t itmp = (i+1)*settings.pointsMult - 1;
		for (uint32_t n = 0; n < settings.pointsMult; n++) {
			output[itmp - n] = 0.f;
			for (int32_t l = i, le = i + buffer.size(); l < le; l++) {
				assert (itmp - n < output.size());
				if ((uint32_t)l < buffer.size()) output[itmp - n] += buffer[l] * sinc_tab[n + (l-i)*settings.pointsMult];
				else output[itmp-n] += (*input)[l-buffer.size()] * sinc_tab[n + (l-i)*settings.pointsMult];
			}
		}
	}
	for (uint32_t i = buffer.size(), ie = input->size(); i < ie; i++) {
		uint32_t itmp = (i+1)*settings.pointsMult - 1;
		for (uint32_t n = 0; n < settings.pointsMult; n++) {
			output[itmp - n] = 0.f;
			for (int32_t l = i, le = i + buffer.size(); l < le; l++) {
				assert (itmp - n < output.size());
				assert (l-buffer.size() < input->size());
				output[itmp-n] += (*input)[l-buffer.size()] * sinc_tab[n + (l-i)*settings.pointsMult];

			}
		}
	}
	if (settings.artifactRedution) {
		artifact_reduction();
	}
	memcpy(buffer.data(), input->data() + input->size() - buffer.size(), sizeof(float)*buffer.size());
}

void InterpolationThread::run() {
	inter.get()->interpolate();
}

InterpolationClass::InterpolationClass() : QObject() {
	thisPool = std::shared_ptr<QThreadPool> (new QThreadPool);
	interpolators.resize(1);
}

InterpolationClass::~InterpolationClass() {
}

void InterpolationClass::start() {
	if (interEnabled) {
		mutex.lock();
		bool running = false;
		for (quint32 i = 0, ie = interpolators.size(); i < ie; i++) {
			thisPool->start(&(interpolators[i]));
			running = true;
		}
		if (running) thisPool->waitForDone();
		mutex.unlock();
	}
	finished();
}

void InterpolationClass::set_inter_type(quint32 type) {
	if (type == interType || interpolators.size() == 0) return;
	mutex.lock();
	interType = type;
	InterpolatorSettings s (interpolators[0].inter.get()->get_settings());
	for (quint32 i = 0, ie = interpolators.size(); i < ie; ++i) {
		interpolators[i].inter.reset();
		switch (type) {
			case WhittakerShannon: {
				std::shared_ptr<Interpolator> t (new WhitShanInterpolator);
				t.get()->set_settings(s);
				interpolators[i].inter = t;
				break;
			} default:
				assert(false);
				break;
		}
	}
	mutex.unlock();
}

void InterpolationClass::set_inputs (quint32 inputs) {
	mutex.lock();
	assert(inputs);
	while (inputs < interpolators.size()) {
		interpolators.pop_back();
	}
	while (inputs > interpolators.size()) {
		switch (interType) {
			case WhittakerShannon: {
				InterpolationThread t;
				t.inter = std::shared_ptr<Interpolator> (new WhitShanInterpolator);
				interpolators.push_back(t);
				break;
			} default:
				assert(false);
				break;
		}
	}
	mutex.unlock();
}

void InterpolationClass::save_settings(std::ostream &os) const {
	quint32 tmp = interpolators.size();
	char t = interEnabled;
	os.write("INPL", 4);
	os.write((char*)&tmp, 4);
	os.write(&t, 1);
	for (const InterpolationThread& a: interpolators)
		a.inter.get()->save_settings(os);
}

void InterpolationClass::load_settings(std::istream &is) {
	mutex.lock();
	mutex.unlock();
	char h[9];
	is.read(h, 9);
	if (strncmp(h, "INPL", 4) || is.fail()) throw std::runtime_error ("");
	set_inputs(*(quint32*)(h+4));
	set_inter_enabled(*(char*)(h+8));
	for (InterpolationThread& a: interpolators)
		a.inter.get()->load_settings(is);
}
