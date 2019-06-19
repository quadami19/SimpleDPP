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

#include "filtering.hpp"

void Shaper::process() {
	quint32 sz = shape.size();
	for(quint32 i = 0; i < sz; i++) {
		output[i] = 0;
		for (quint32 n = i; n < sz; n++) output[i] += buffer[n]*shape[n - i];
		for (quint32 n = 0; n < i; n++) output[i] += (*inputPtr)[n]*shape[n + sz - i];
	}
	for(int i = sz, ie = output.size(); i < ie; i++) {
		output[i] = 0;
		for (quint32 n = 0; n < sz; n++) output[i] += (*inputPtr)[n + i - sz]*shape[n];
	}
	memcpy (buffer.data(), (*inputPtr).data() + (*inputPtr).size() - sz, sizeof(float)*(sz));
}

void Delay::set(std::shared_ptr<FilterSettings> settings) {
	delayTime = ((Settings*)settings.get())->time;
	buffer.resize(delayTime, 0.f);
}

std::shared_ptr<Filter::FilterSettings> Delay::get() {
	std::shared_ptr<FilterSettings> t (new Settings);
	((Settings*)t.get())->time = delayTime;
	return t;
}

quint32 Delay::get_filter_id () const {
	return FilteringProcessor::Delay;
}

void Delay::save_settings(std::ostream &os) const {
	os.write((char*)&delayTime, 4);
}

void Delay::load_settings(std::istream &is) {
	is.read((char*)&delayTime, 4);
	if (is.fail()) throw std::runtime_error ("");
	buffer.resize(delayTime, 0.f);
}

void Delay::process() {
	memcpy (output.data(), inputPtr->data(), sizeof(float)*output.size());
	quint32 ie = delayTime, i;
	for (i = 0; i < ie; i++) {
		output[i] = buffer[i];
	}
	quint32 b = ie;
	ie = output.size() - delayTime;
	for (i = 0; i < ie; i++) {
		output[i + b] = (*inputPtr)[i];
	}
	memcpy(buffer.data(), inputPtr->data() + ie, sizeof(float)*b);
}

Delay::Settings & Delay::Settings::operator= (const FilterSettings& a) {
	time = ((Delay::Settings*)(&a))->time;
	return *this;
}

void DelayLine::set(std::shared_ptr<FilterSettings> settings) {
	delayAmplit = ((Settings*)settings.get())->ampl;
	delayTime = ((Settings*)settings.get())->time;
	buffer.resize(delayTime, 0.f);
}

std::shared_ptr<Filter::FilterSettings> DelayLine::get() {
	std::shared_ptr<FilterSettings> t (new Settings);
	((Settings*)t.get())->ampl = delayAmplit;
	((Settings*)t.get())->time = delayTime;
	return t;
}

quint32 DelayLine::get_filter_id () const {
	return FilteringProcessor::DelayLine;
}

void DelayLine::save_settings(std::ostream &os) const {
	os.write((char*)&delayTime, 4);
	os.write((char*)&delayAmplit, 4);
}

void DelayLine::load_settings(std::istream &is) {
	is.read((char*)&delayTime, 4);
	if (is.fail()) throw std::runtime_error ("");
	buffer.resize(delayTime, 0.f);
	is.read((char*)&delayAmplit, 4);
	if (is.fail()) throw std::runtime_error ("");
}

void DelayLine::process() {
	memcpy (output.data(), inputPtr->data(), sizeof(float)*output.size());
	quint32 ie = delayTime, i;
	for (i = 0; i < ie; i++) {
		output[i] -= delayAmplit * buffer[i];
	}
	quint32 b = ie;
	ie = output.size() - delayTime;
	for (i = 0; i < ie; i++) {
		output[i + b] -= delayAmplit * (*inputPtr)[i];
	}
	memcpy(buffer.data(), inputPtr->data() + ie, sizeof(float)*b);
}

DelayLine::Settings & DelayLine::Settings::operator= (const FilterSettings& a) {
	time = ((DelayLine::Settings*)(&a))->time;
	ampl = ((DelayLine::Settings*)(&a))->ampl;
	return *this;
}

void OverrunLine::set(std::shared_ptr<FilterSettings> settings) {
	overrunAmplit = ((Settings*)settings.get())->ampl;
	overrunTime = ((Settings*)settings.get())->time;
	buffer.resize(overrunTime, 0.f);
}

std::shared_ptr<Filter::FilterSettings> OverrunLine::get() {
	std::shared_ptr<FilterSettings> t (new Settings);
	((Settings*)t.get())->ampl = overrunAmplit;
	((Settings*)t.get())->time = overrunTime;
	return t;
}

quint32 OverrunLine::get_filter_id () const {
	return FilteringProcessor::OverrunningLine;
}

void OverrunLine::save_settings(std::ostream &os) const {
	os.write((char*)&overrunTime, 4);
	os.write((char*)&overrunAmplit, 4);
}

void OverrunLine::load_settings(std::istream &is) {
	is.read((char*)&overrunTime, 4);
	if (is.fail()) throw std::runtime_error ("");
	buffer.resize(overrunTime, 0.f);
	is.read((char*)&overrunAmplit, 4);
	if (is.fail()) throw std::runtime_error ("");
}

void OverrunLine::process() {
	quint32 ie = output.size(), i;
	for (i = 0; i < overrunTime; i++) {
		output[i] = buffer[i] - overrunAmplit * (*inputPtr)[i];
	}
	for (i = overrunTime; i < ie; i++) {
		output[i] = (*inputPtr)[i - overrunTime] - overrunAmplit * (*inputPtr)[i];
	}
	memcpy(buffer.data(), inputPtr->data() + ie - overrunTime, sizeof(float)*overrunTime);
}

OverrunLine::Settings & OverrunLine::Settings::operator= (const FilterSettings& a) {
	time = ((OverrunLine::Settings*)(&a))->time;
	ampl = ((OverrunLine::Settings*)(&a))->ampl;
	return *this;
}

void R_C_Filter::set(std::shared_ptr<FilterSettings> settings) {
	alpha = ((RC_Settings*)settings.get())->alpha;
}

R_C_Filter::RC_Settings & R_C_Filter::RC_Settings::operator= (const FilterSettings& a) {
	alpha = ((R_C_Filter::RC_Settings*)(&a))->alpha;
	return *this;
}

void R_C_Filter::save_settings(std::ostream &os) const {
	os.write((char*)&alpha, 4);
}

void R_C_Filter::load_settings(std::istream &is) {
	is.read((char*)&alpha, 4);
	if (is.fail()) throw std::runtime_error ("");
}

std::shared_ptr<Filter::FilterSettings> RC_IIR_Filter::get() {
	std::shared_ptr<FilterSettings> t (new Settings);
	((Settings*)t.get())->alpha = alpha;
	return t;
}

quint32 RC_IIR_Filter::get_filter_id () const {
	return FilteringProcessor::RC_IIR;
}

void RC_IIR_Filter::process () {
	float tmp = 1.f - alpha;
	output[0] = alpha*(*inputPtr)[0] + tmp * buffer[0];
	for (quint32 i = 1, ie = output.size(); i < ie; i++)
		output[i] = alpha*(*inputPtr)[i] + tmp * output[i-1];
	buffer[0] = output[output.size()-1];
}

std::shared_ptr<Filter::FilterSettings> CR_IIR_Filter::get() {
	std::shared_ptr<FilterSettings> t (new Settings);
	((Settings*)t.get())->alpha = alpha;
	return t;
}

quint32 CR_IIR_Filter::get_filter_id () const {
	return FilteringProcessor::CR_IIR;
}

void CR_IIR_Filter::process () {
	output[0] = alpha*((*inputPtr)[0] + buffer[0]);
	for (quint32 i = 1, ie = output.size(); i < ie; i++)
		output[i] = alpha*((*inputPtr)[i] - (*inputPtr)[i-1] + output[i-1]);
	buffer[0] = output[output.size()-1] - (*inputPtr)[(*inputPtr).size()-1];
}

std::shared_ptr<Filter::FilterSettings> CR_REV_IIR_Filter::get() {
	std::shared_ptr<FilterSettings> t (new Settings);
	((Settings*)t.get())->alpha = alpha;
	return t;
}

quint32 CR_REV_IIR_Filter::get_filter_id () const {
	return FilteringProcessor::CR_REV_IIR;
}

void CR_REV_IIR_Filter::process () {
	output[0] = (*inputPtr)[0]/alpha + buffer[0];
	for (quint32 i = 1, ie = output.size(); i < ie; i++)
		output[i] = (*inputPtr)[i]/alpha - (*inputPtr)[i-1] + output[i-1]*0.9999;
	buffer[0] = output[output.size()-1]*0.9999 - (*inputPtr)[(*inputPtr).size()-1];
}

void MovingAverage::set_shape() {
	shape.resize(size);
	buffer.resize(size);
	for (quint32 i = 0; i < size; i++) shape[i] = 1.f/(float)size;
}
void MovingAverage::set(std::shared_ptr<FilterSettings> settings) {
	size = ((Settings*)settings.get())->size;
	set_shape();
}

std::shared_ptr<Filter::FilterSettings> MovingAverage::get() {
	std::shared_ptr<FilterSettings> t (new Settings);
	((Settings*)t.get())->size = size;
	return t;
}

quint32 MovingAverage::get_filter_id () const {
	return FilteringProcessor::MovingAverage;
}

void MovingAverage::save_settings(std::ostream &os) const {
	os.write((char*)&size, 4);
}

void MovingAverage::load_settings(std::istream &is) {
	is.read((char*)&size, 4);
	if (is.fail()) throw std::runtime_error ("");
	set_shape();
}

MovingAverage::Settings & MovingAverage::Settings::operator= (const FilterSettings& a) {
	size = ((MovingAverage::Settings*)(&a))->size;
	return *this;
}

void TrapezoidalShaper::set_shape() {
	shape.resize(2*sideSz + flatSz);
	buffer.resize(2*sideSz + flatSz);
	for (quint32 i = 0; i < sideSz; i++) shape[i] = (float)i/(float)sideSz;
	for (quint32 i = sideSz, ie = i + flatSz; i < ie; i++) shape[i] = 1.f;
	for (quint32 i = sideSz + flatSz, ie = i + sideSz; i < ie; i++) shape[i] = (float)(ie - i)/(float)sideSz;
}
void TrapezoidalShaper::set(std::shared_ptr<FilterSettings> settings) {
	sideSz = ((Settings*)settings.get())->side;
	flatSz = ((Settings*)settings.get())->flat;
	set_shape();
}

std::shared_ptr<Filter::FilterSettings> TrapezoidalShaper::get() {
	std::shared_ptr<FilterSettings> t (new Settings);
	((Settings*)t.get())->side = sideSz;
	((Settings*)t.get())->flat = flatSz;
	return t;
}

quint32 TrapezoidalShaper::get_filter_id () const {
	return FilteringProcessor::Trapezoidal;
}

void TrapezoidalShaper::save_settings(std::ostream &os) const {
	os.write((char*)&sideSz, 4);
	os.write((char*)&flatSz, 4);
}

void TrapezoidalShaper::load_settings(std::istream &is) {
	is.read((char*)&sideSz, 4);
	if (is.fail()) throw std::runtime_error ("");
	is.read((char*)&flatSz, 4);
	if (is.fail()) throw std::runtime_error ("");
	set_shape();
}

TrapezoidalShaper::Settings & TrapezoidalShaper::Settings::operator= (const FilterSettings& a) {
	side = ((TrapezoidalShaper::Settings*)(&a))->side;
	flat = ((TrapezoidalShaper::Settings*)(&a))->flat;
	return *this;
}

void GaussianShaper::set_shape() {
	shape.resize(8*width);
	buffer.resize(8*width);
	for (qint32 i = 0, ie = shape.size(); i < ie; i++) shape[i] = std::exp(-(i - ie/2)*(i - ie/2)/(2.f*width*width));
}

void GaussianShaper::set(std::shared_ptr<FilterSettings> settings) {
	width = ((Settings*)settings.get())->width;
	set_shape();
}

std::shared_ptr<Filter::FilterSettings> GaussianShaper::get() {
	std::shared_ptr<FilterSettings> t (new Settings);
	((Settings*)t.get())->width = width;
	return t;
}

quint32 GaussianShaper::get_filter_id () const {
	return FilteringProcessor::Gaussian;
}

void GaussianShaper::save_settings(std::ostream &os) const {
	os.write((char*)&width, 4);
}

void GaussianShaper::load_settings(std::istream &is) {
	is.read((char*)&width, 4);
	if (is.fail()) throw std::runtime_error ("");
	set_shape();
}

GaussianShaper::Settings & GaussianShaper::Settings::operator= (const FilterSettings& a) {
	width = ((GaussianShaper::Settings*)(&a))->width;
	return *this;
}

void CuspShaper::set_shape() {
	qint32 sideSz = 8 * width;
	shape.resize(2*sideSz + flatSz);
	buffer.resize(2*sideSz + flatSz);
	for (qint32 i = 0; i < sideSz; i++) shape[i] = std::exp((i-sideSz)/width);
	for (qint32 i = sideSz, ie = i + flatSz; i < ie; i++) shape[i] = 1.f;
	for (qint32 i = sideSz + flatSz, ie = i + sideSz; i < ie; i++) shape[i] = std::exp((ie - i - sideSz)/width);
}

void CuspShaper::set(std::shared_ptr<FilterSettings> settings) {
	width = ((Settings*)settings.get())->width;
	flatSz = ((Settings*)settings.get())->flat;
	set_shape();
}

std::shared_ptr<Filter::FilterSettings> CuspShaper::get() {
	std::shared_ptr<FilterSettings> t (new Settings);
	((Settings*)t.get())->width = width;
	((Settings*)t.get())->flat = flatSz;
	return t;
}

quint32 CuspShaper::get_filter_id () const {
	return FilteringProcessor::Cusp;
}

void CuspShaper::save_settings(std::ostream &os) const {
	os.write((char*)&width, 4);
	os.write((char*)&flatSz, 4);
}

void CuspShaper::load_settings(std::istream &is) {
	is.read((char*)&width, 4);
	if (is.fail()) throw std::runtime_error ("");
	is.read((char*)&flatSz, 4);
	if (is.fail()) throw std::runtime_error ("");
	set_shape();
}

CuspShaper::Settings & CuspShaper::Settings::operator= (const FilterSettings& a) {
	width = ((CuspShaper::Settings*)(&a))->width;
	flat = ((CuspShaper::Settings*)(&a))->flat;
	return *this;
}

FilteringThread::FilteringThread (quint32 dataSize) : size(dataSize) {
	setAutoDelete(false);
}

FilteringThread::~FilteringThread () {
}

void FilteringThread::run () {
	if (filters.empty() || !isEnabled) return;
	filters[0]->set_data(inputPtr);
	filters[0]->process();
	for (quint32 i = 1, ie = filters.size(); i < ie; i++) {
		filters[i]->set_data(filters[i-1]->get_data());
		filters[i]->process();
	}
}

void FilteringThread::set_size(quint32 _size) {
	size = _size;
	for (auto& a: filters)
		a->set_size(size);
}

void FilteringThread::set_input(std::vector<float> const* input) {
	inputPtr = input;
}

std::vector<float> const* FilteringThread::get_output() const {
	if (filters.size() && isEnabled) return filters[filters.size()-1]->get_data();
	else return inputPtr;
}

void FilteringThread::add_filter(quint32 filter_id) {
	switch (filter_id) {
		case FilteringProcessor::Delay:
			filters.push_back(std::shared_ptr<Filter>(new Delay (size)));
			break;
		case FilteringProcessor::DelayLine:
			filters.push_back(std::shared_ptr<Filter>(new DelayLine (size)));
			break;
		case FilteringProcessor::OverrunningLine:
			filters.push_back(std::shared_ptr<Filter>(new OverrunLine (size)));
			break;
		case FilteringProcessor::RC_IIR:
			filters.push_back(std::shared_ptr<Filter>(new RC_IIR_Filter (size)));
			break;
		case FilteringProcessor::CR_IIR:
			filters.push_back(std::shared_ptr<Filter>(new CR_IIR_Filter (size)));
			break;
		case FilteringProcessor::CR_REV_IIR:
			filters.push_back(std::shared_ptr<Filter>(new CR_REV_IIR_Filter (size)));
			break;
		case FilteringProcessor::MovingAverage:
			filters.push_back(std::shared_ptr<Filter>(new MovingAverage (size)));
			break;
		case FilteringProcessor::Trapezoidal:
			filters.push_back(std::shared_ptr<Filter>(new TrapezoidalShaper (size)));
			break;
		case FilteringProcessor::Gaussian:
			filters.push_back(std::shared_ptr<Filter>(new GaussianShaper (size)));
			break;
		case FilteringProcessor::Cusp:
			filters.push_back(std::shared_ptr<Filter>(new CuspShaper (size)));
			break;
		default:
			assert(false);
	}
}

void FilteringThread::del_filter(quint32 filter_num) {
	assert (filter_num < filters.size());
	filters.erase(filters.begin() + filter_num);
}

quint32 FilteringThread::get_filter_count() const {
	return filters.size();
}

void FilteringThread::set_filter_id(quint32 filter_id, quint32 filter_num) {
	if (filters[filter_num]->get_filter_id() == filter_id) return;
	switch (filter_id) {
		case FilteringProcessor::Delay:
			filters[filter_num] = std::shared_ptr<Filter>(new Delay (size));
			break;
		case FilteringProcessor::DelayLine:
			filters[filter_num] = std::shared_ptr<Filter>(new DelayLine (size));
			break;
		case FilteringProcessor::OverrunningLine:
			filters.push_back(std::shared_ptr<Filter>(new OverrunLine (size)));
			break;
		case FilteringProcessor::RC_IIR:
			filters[filter_num] = std::shared_ptr<Filter>(new RC_IIR_Filter (size));
			break;
		case FilteringProcessor::CR_IIR:
			filters[filter_num] = std::shared_ptr<Filter>(new CR_IIR_Filter (size));
			break;
		case FilteringProcessor::CR_REV_IIR:
			filters[filter_num] = std::shared_ptr<Filter>(new CR_REV_IIR_Filter (size));
			break;
		case FilteringProcessor::MovingAverage:
			filters[filter_num] = std::shared_ptr<Filter>(new MovingAverage (size));
			break;
		case FilteringProcessor::Trapezoidal:
			filters[filter_num] = std::shared_ptr<Filter>(new TrapezoidalShaper (size));
			break;
		case FilteringProcessor::Gaussian:
			filters[filter_num] = std::shared_ptr<Filter>(new GaussianShaper (size));
			break;
		case FilteringProcessor::Cusp:
			filters[filter_num] = std::shared_ptr<Filter>(new CuspShaper (size));
			break;
		default:
			assert(false);
	}
}

quint32 FilteringThread::get_filter_id(quint32 filter_num) const {
	assert (filter_num < filters.size());
	return filters[filter_num]->get_filter_id();
}

void FilteringThread::save_settings(std::ostream &os) const {
	char a = isEnabled;
	os.write(&a, 1);
	quint32 sz = filters.size();
	os.write((char*)&sz, 4);
	for (quint32 i = 0, ie = sz; i < ie; i++) {
		sz = filters[i]->get_filter_id();
		os.write((char*)&sz, 4);
		filters[i]->save_settings(os);
	}
}

void FilteringThread::load_settings(std::istream &is) {
	char a;
	is.read(&a, 1);
	if (is.fail()) throw std::runtime_error ("");
	isEnabled = a;
	quint32 sz;
	is.read((char*)&sz, 4);
	if (is.fail()) throw std::runtime_error ("");
	filters.resize(0);
	for (quint32 i = 0, ie = sz; i < ie; i++) {
		is.read((char*)&sz, 4);
		if (is.fail()) throw std::runtime_error ("");
		switch (sz) {
			case FilteringProcessor::Delay:
				filters.push_back(std::shared_ptr<Filter>(new Delay (size)));
				break;
			case FilteringProcessor::DelayLine:
				filters.push_back(std::shared_ptr<Filter>(new DelayLine (size)));
				break;
			case FilteringProcessor::OverrunningLine:
				filters.push_back(std::shared_ptr<Filter>(new OverrunLine (size)));
				break;
			case FilteringProcessor::RC_IIR:
				filters.push_back(std::shared_ptr<Filter>(new RC_IIR_Filter (size)));
				break;
			case FilteringProcessor::CR_IIR:
				filters.push_back(std::shared_ptr<Filter>(new CR_IIR_Filter (size)));
				break;
			case FilteringProcessor::CR_REV_IIR:
				filters.push_back(std::shared_ptr<Filter>(new CR_REV_IIR_Filter (size)));
				break;
			case FilteringProcessor::MovingAverage:
				filters.push_back(std::shared_ptr<Filter>(new MovingAverage (size)));
				break;
			case FilteringProcessor::Trapezoidal:
				filters.push_back(std::shared_ptr<Filter>(new TrapezoidalShaper (size)));
				break;
			case FilteringProcessor::Gaussian:
				filters.push_back(std::shared_ptr<Filter>(new GaussianShaper (size)));
				break;
			case FilteringProcessor::Cusp:
				filters.push_back(std::shared_ptr<Filter>(new CuspShaper (size)));
				break;
			default:
				assert (false);
		}
		filters[i]->load_settings(is);
	}
}

FilteringProcessor::FilteringProcessor (quint32 size) : QObject(), dataSize (size) {
	thisPool = std::shared_ptr<QThreadPool> (new QThreadPool);
}

FilteringProcessor::~FilteringProcessor () {
}

void FilteringProcessor::process() {
	mutex.lock();
	bool running = false;
	for (quint32 i = 0, ie = filterStreams.size(); i < ie; i++) {
		filterStreams[i]->set_input(inputPtrs[i]);
		if (filterStreams[i]->get_filter_count() && filterStreams[i]->get_enabled()) {
			thisPool->start(filterStreams[i]);
			running = true;
		}
	}
	if (running) thisPool->waitForDone();
	finished();
	mutex.unlock();
}

void FilteringProcessor::set_streams(quint32 streams) {
	mutex.lock();
	while (filterStreams.size() < streams) {
		filterStreams.push_back(new FilteringThread (dataSize));
	}
	while (filterStreams.size() > streams) {
		delete filterStreams[filterStreams.size()-1];
		filterStreams.pop_back();
	}
	inputPtrs.resize(streams, 0x0);
	mutex.unlock();
}

void FilteringProcessor::set_size(quint32 size) {
	mutex.lock();
	dataSize = size;
	for (quint32 i = 0, ie = filterStreams.size(); i < ie; i++)
		for (quint32 n = 0, ne = filterStreams[i]->get_filter_count(); n < ne; n++) {
			filterStreams[i]->set_size(size);
		}
	mutex.unlock();
}

void FilteringProcessor::save_settings(std::ostream &os) const {
	quint32 tmp = filterStreams.size();
	os.write("SHPR", 4);
	os.write((char*)&dataSize, 4);
	os.write((char*)&tmp, 4);
	for (FilteringThread* a: filterStreams)
		a->save_settings(os);
}

void FilteringProcessor::load_settings(std::istream &is) {
	mutex.lock();
	mutex.unlock();
	char h[12];
	is.read(h, 12);
	if (strncmp(h, "SHPR", 4) || is.fail()) throw std::runtime_error ("");
	set_streams(*(quint32*)(h + 8));
	set_size(*(quint32*)(h + 4));
	for (FilteringThread* a: filterStreams)
		a->load_settings(is);
}
