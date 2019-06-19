
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
#include "processing.hpp"


void PulseSearching::search(std::vector<float>::iterator _begin, std::vector<float>::iterator _end, quint32 _begPos) {
	begin = _begin;
	end = _end;
	begPos = _begPos;
	std::vector<float>::iterator currIter (begin);
	while (currIter < end) {
		if (check_pulse(currIter)) {
			pulsePos = begPos + (currIter - begin);
			detectCallback();
			if (isSingle) return;
			currIter += settings->sSet->skipSamples;
		}
		++currIter;
	}
}

std::shared_ptr<PulseSearching> PulseSearching::get_new (quint32 id) {
	switch (id) {
		case PulseSearching::Threshold:
			return std::shared_ptr<PulseSearching> (new PulseSearchingThreshold);
			break;
		case PulseSearching::Monoton:
			return std::shared_ptr<PulseSearching> (new PulseSearchingMonoton);
			break;
		case PulseSearching::TanThreshold:
			return std::shared_ptr<PulseSearching> (new PulseSearchingTanThreshold);
			break;
		default: assert(false);
	}
	return std::shared_ptr<PulseSearching> (new PulseSearchingThreshold);
}

void PulseSearching::set (std::shared_ptr<ProcessingThread::Settings> s) {
	assert(get_search_type() == s->sSet->get_s_settings_id());
	settings = s;
	update_settings();
}

PulseSearchingThreshold::PulseSearchingThreshold () : PulseSearching() {
}

quint8 PulseSearchingThreshold::check_pulse(std::vector<float>::iterator curr) {
	float sum = 0;
	SearchThresholdSettings* tmp = (SearchThresholdSettings*)settings->sSet.get();
	if (tmp->detectBaselineSamples) {
		for (auto beg = curr, end = curr + tmp->detectBaselineSamples; beg != end; ++beg)
			sum += *beg;
		sum /= tmp->detectBaselineSamples;
	}
	if (*(curr + tmp->detectBaselineSamples + tmp->detectPos) - sum > tmp->threshold)
		return 1;
	else return 0;
}

void PulseSearchingThreshold::save(std::ostream &os) const {
	SearchThresholdSettings& tmp = *(SearchThresholdSettings*)settings->sSet.get();
	os.write((char*)&tmp.detectBaselineSamples, 4);
	os.write((char*)&tmp.detectPos, 4);
	os.write((char*)&tmp.skipSamples, 4);
	os.write((char*)&tmp.threshold, 4);
}

void PulseSearchingThreshold::load(std::istream &is) {
	SearchThresholdSettings& tmp = *(SearchThresholdSettings*)settings->sSet.get();
	is.read((char*)&tmp.detectBaselineSamples, 4);
	is.read((char*)&tmp.detectPos, 4);
	is.read((char*)&tmp.skipSamples, 4);
	is.read((char*)&tmp.threshold, 4);
	if (is.fail()) throw std::runtime_error ("");
}

PulseSearchingMonoton::PulseSearchingMonoton () : PulseSearching () {
}

void PulseSearchingMonoton::save(std::ostream &os) const {
	SearchMonotonSettings& tmp = *(SearchMonotonSettings*)settings->sSet.get();
	os.write((char*)&tmp.risingSamples, 4);
	os.write((char*)&tmp.indiffSamples, 4);
	os.write((char*)&tmp.fallingSamples, 4);
	os.write((char*)&tmp.skipSamples, 4);
}

void PulseSearchingMonoton::load(std::istream &is) {
	SearchMonotonSettings& tmp = *(SearchMonotonSettings*)settings->sSet.get();
	is.read((char*)&tmp.risingSamples, 4);
	is.read((char*)&tmp.indiffSamples, 4);
	is.read((char*)&tmp.fallingSamples, 4);
	is.read((char*)&tmp.skipSamples, 4);
	if (is.fail()) throw std::runtime_error ("");
}

quint8 PulseSearchingMonoton::check_pulse(std::vector<float>::iterator curr) {
	SearchMonotonSettings* tmp = (SearchMonotonSettings*)settings->sSet.get();
	for (auto beg = curr, end = curr + tmp->risingSamples; beg != end; ++beg)
		if (*beg > *(beg+1)) return 0;
	for (auto beg = curr + tmp->indiffSamples + tmp->risingSamples, end = beg + tmp->fallingSamples; beg != end; ++beg)
		if (*beg < *(beg+1)) return 0;
	return 1;
}

PulseSearchingTanThreshold::PulseSearchingTanThreshold () : PulseSearching () {
}

void PulseSearchingTanThreshold::update_settings() {
	SearchTanThresholdSettings& tmp = *(SearchTanThresholdSettings*)settings->sSet.get();
	Eigen::Matrix2f matrTmp = Eigen::Matrix2f::Zero (2, 2);
	if (tmp.risingSamples) {
		rise0OrderPoly.resize(tmp.risingSamples);
		rise1OrderPoly.resize(tmp.risingSamples);
		rise0OrderPoly(0) = 1.0f;
		rise1OrderPoly(0) = 0.0f;
		for (quint16 i = 1; i < tmp.risingSamples; ++i) {
			rise0OrderPoly(i) = 1.0f;
			rise1OrderPoly(i) = rise1OrderPoly(i-1) + 1.0f;
		}
		matrTmp(0,0) = rise0OrderPoly.transpose() * rise0OrderPoly;
		matrTmp(1,0) = rise1OrderPoly.transpose() * rise0OrderPoly;
		matrTmp(0,1) = matrTmp(1,0);
		matrTmp(1,1) = rise1OrderPoly.transpose() * rise1OrderPoly;
		riseMatrix = matrTmp.inverse();
	}
	if (tmp.fallingSamples) {
		matrTmp = Eigen::Matrix2f::Zero (2, 2);
		fall0OrderPoly.resize(tmp.fallingSamples);
		fall1OrderPoly.resize(tmp.fallingSamples);
		fall0OrderPoly(0) = 1.0f;
		fall1OrderPoly(0) = 0.0f;
		for (quint16 i = 1; i < tmp.fallingSamples; ++i) {
			fall0OrderPoly(i) = 1.0f;
			fall1OrderPoly(i) = fall1OrderPoly(i-1) + 1.0f;
		}
		matrTmp(0,0) = fall0OrderPoly.transpose() * fall0OrderPoly;
		matrTmp(1,0) = fall1OrderPoly.transpose() * fall0OrderPoly;
		matrTmp(0,1) = matrTmp(1,0);
		matrTmp(1,1) = fall1OrderPoly.transpose() * fall1OrderPoly;
		fallMatrix = matrTmp.inverse();
	}
}

void PulseSearchingTanThreshold::save(std::ostream &os) const {
	SearchTanThresholdSettings& tmp = *(SearchTanThresholdSettings*)settings->sSet.get();
	os.write((char*)&tmp.risingSamples, 4);
	os.write((char*)&tmp.indiffSamples, 4);
	os.write((char*)&tmp.fallingSamples, 4);
	os.write((char*)&tmp.risingTan, 4);
	os.write((char*)&tmp.fallingTan, 4);
	os.write((char*)&tmp.skipSamples, 4);
}

void PulseSearchingTanThreshold::load(std::istream &is) {
	SearchTanThresholdSettings & tmp = *(SearchTanThresholdSettings*)settings->sSet.get();
	is.read((char*)&tmp.risingSamples, 4);
	is.read((char*)&tmp.indiffSamples, 4);
	is.read((char*)&tmp.fallingSamples, 4);
	is.read((char*)&tmp.risingTan, 4);
	is.read((char*)&tmp.fallingTan, 4);
	is.read((char*)&tmp.skipSamples, 4);
	if (is.fail()) throw std::runtime_error ("");
}

quint8 PulseSearchingTanThreshold::check_pulse(std::vector<float>::iterator curr) {
	SearchTanThresholdSettings* tmp = (SearchTanThresholdSettings*)settings->sSet.get();
	float f1 = 0, f2 = 0, angle;
	quint32 j = 0;
	auto t1 = curr, t2 = curr + tmp->risingSamples;
	while (t1 < t2) {
		f1 += *(t1  ) * rise0OrderPoly(j  );
		f2 += *(t1++) * rise1OrderPoly(j++);
	}
	angle = riseMatrix(1,0) * f1 + riseMatrix(1,1) * f2;
	if (angle > tmp->risingTan) {
		if (!tmp->fallingSamples) return 1;
		f1 = 0; f2 = 0; j = 0;
		t1 = curr + tmp->risingSamples + tmp->indiffSamples;
		t2 = t1 + tmp->fallingSamples;
		while (t1 < t2) {
			f1 += *(t1  ) * fall0OrderPoly(j  );
			f2 += *(t1++) * fall1OrderPoly(j++);
		}
		angle = fallMatrix(1,0) * f1 + fallMatrix(1,1) * f2;
		if (angle < tmp->fallingTan) return 1;
	}
	return 0;
}

void PulseDiscriminator::set (std::shared_ptr<ProcessingThread::Settings> d) {
	assert(get_disc_type() == d->dSet->get_d_settings_id());
	settings = d;
	update_settings();
}

std::shared_ptr<PulseDiscriminator> PulseDiscriminator::get_new (quint32 id) {
	switch (id) {
		case PulseDiscriminator::Dispersion:
			return std::shared_ptr<PulseDiscriminator> (new PulseDiscriminatorByDispersion);
			break;
		case PulseDiscriminator::NeuralNet:
			return std::shared_ptr<PulseDiscriminator> (new PulseDiscriminatorByNeuralNet);
			break;
		default: assert(false);
	}
}

PulseDiscriminatorByDispersion::PulseDiscriminatorByDispersion () : PulseDiscriminator() {
}

quint8 PulseDiscriminatorByDispersion::discriminate(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse) {
	DispDiscSettings* tmp = (DispDiscSettings*)settings->dSet.get();
	assert ((std::vector<float>::size_type)(endPulse - begPulse) == settings->shape.size());
	float tDisp (0.0f), fTmp;
	for (auto curr = begPulse; curr != endPulse; ++curr) {
		fTmp = *curr - settings->shape[curr-begPulse];
		tDisp += fTmp*fTmp;
	}
	tDisp /= static_cast<float> (settings->shape.size());
	tDisp = std::sqrt (tDisp);
	if (tDisp < tmp->maxDispersion) return 1;
	else return 0;
}

void PulseDiscriminatorByDispersion::save(std::ostream &os) const {
	DispDiscSettings& tmp = *(DispDiscSettings*)settings->dSet.get();
	quint8 t = tmp.enabled;
	os.write((char*)&t, 1);
	os.write((char*)&tmp.maxDispersion, 4);
}

void PulseDiscriminatorByDispersion::load(std::istream &is) {
	DispDiscSettings& tmp = *(DispDiscSettings*)settings->dSet.get();
	quint8 t;
	is.read((char*)&t, 1);
	tmp.enabled = t;
	is.read((char*)&tmp.maxDispersion, 4);
	if (is.fail()) throw std::runtime_error ("");
}

PulseDiscriminatorByNeuralNet::PulseDiscriminatorByNeuralNet () : PulseDiscriminator() {
}

quint8 PulseDiscriminatorByNeuralNet::discriminate(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse) {
	DiscNNSettings* tmp = (DiscNNSettings*)settings->dSet.get();
	assert ((std::vector<float>::size_type)(endPulse - begPulse) == tmp->neuralNet.inputs());

	float bl = 0.f;
	if (settings->aSet->processBaselineSamples) {
		for (auto curr = begPulse, end = begPulse + settings->aSet->processBaselineSamples; curr != end; ++curr) bl += *curr;
		bl /= static_cast<float> (settings->aSet->processBaselineSamples);
	}

	std::vector<float> tmpv (begPulse, endPulse);
	for (auto a = tmpv.begin(), b = tmpv.end(); a != b; ++a) *a -= bl;
	float maxVal = *std::max_element(begPulse, endPulse);
	for (auto a = tmpv.begin(), b = tmpv.end(); a != b; ++a) *a /= maxVal;

	tmp->neuralNet.work(tmpv);
	if (tmp->neuralNet.get_output(0) > 0.5)	return 1;
	else return 0;
}

void PulseDiscriminatorByNeuralNet::save(std::ostream &os) const {
	DiscNNSettings& tmp = *(DiscNNSettings*)settings->dSet.get();
	quint8 t = tmp.enabled;
	os.write((char*)&t, 1);
	tmp.neuralNet.save(os);
}

void PulseDiscriminatorByNeuralNet::load(std::istream &is) {
	DiscNNSettings& tmp = *(DiscNNSettings*)settings->dSet.get();
	quint8 t;
	is.read((char*)&t, 1);
	tmp.enabled = t;
	tmp.neuralNet.load(is);
}

void PulseAmplMeasuring::measure (std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse) {
	pulseAmpl = find_ampl(begPulse, endPulse) * settings->amplCorrection;
}

void PulseAmplMeasuring::set (std::shared_ptr<ProcessingThread::Settings> a) {
	assert(get_ampl_type() == a->aSet->get_a_settings_id());
	settings = a;
	update_settings();
}

std::shared_ptr<PulseAmplMeasuring> PulseAmplMeasuring::get_new (quint32 id) {
	switch (id) {
		case PulseAmplMeasuring::MaxVal:
			return std::shared_ptr<PulseAmplMeasuring> (new PulseAmplMeasuringByMax);
			break;
		case PulseAmplMeasuring::Polynomial:
			return std::shared_ptr<PulseAmplMeasuring> (new PulseAmplMeasuringPolyMax);
			break;
		case PulseAmplMeasuring::NeuralNet:
			return std::shared_ptr<PulseAmplMeasuring> (new PulseAmplMeasuringNeuralNet);
			break;
		default: assert(false);
	}
}

PulseAmplMeasuringByMax::PulseAmplMeasuringByMax() {
}

float PulseAmplMeasuringByMax::find_ampl(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse) {
	AmplMaxSettings* tmp = (AmplMaxSettings*)settings->aSet.get();
	assert (begPulse + tmp->maxValIntervalLeft <= endPulse);
	assert (begPulse + tmp->maxValIntervalRight <= endPulse);

	float bl = 0.f;
	if (settings->aSet->processBaselineSamples) {
		for (auto curr = begPulse, end = begPulse + tmp->processBaselineSamples; curr != end; ++curr) bl += *curr;
		bl /= static_cast<float> (tmp->processBaselineSamples);
	}

	return *std::max_element(begPulse + tmp->maxValIntervalLeft, begPulse + tmp->maxValIntervalRight) - bl;
}

void PulseAmplMeasuringByMax::update_settings() {
	AmplMaxSettings* tmp = (AmplMaxSettings*)settings->aSet.get();
	assert (tmp->maxValIntervalLeft < tmp->maxValIntervalRight);
	assert (tmp->processBaselineSamples <= tmp->maxValIntervalLeft);
}

void PulseAmplMeasuringByMax::save(std::ostream &os) const {
	AmplMaxSettings& tmp = *(AmplMaxSettings*)settings->aSet.get();
	os.write((char*)&tmp.processBaselineSamples, 4);
	os.write((char*)&tmp.maxValIntervalLeft, 4);
	os.write((char*)&tmp.maxValIntervalRight, 4);
}

void PulseAmplMeasuringByMax::load(std::istream &is) {
	AmplMaxSettings& tmp = *(AmplMaxSettings*)settings->aSet.get();
	is.read((char*)&tmp.processBaselineSamples, 4);
	is.read((char*)&tmp.maxValIntervalLeft, 4);
	is.read((char*)&tmp.maxValIntervalRight, 4);
	if (is.fail()) throw std::runtime_error ("");
}

PulseAmplMeasuringPolyMax::PulseAmplMeasuringPolyMax() {
}

float PulseAmplMeasuringPolyMax::find_ampl(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse) {
	AmplPolyMaxSettings* tmp = (AmplPolyMaxSettings*)settings->aSet.get();
	assert (begPulse + tmp->maxValIntervalLeft <= endPulse);
	assert (begPulse + tmp->maxValIntervalRight <= endPulse);
	assert (endPulse - begPulse == settings->pulseSize);

	Eigen::VectorXf vecTmp (tmp->polyOrder);
	for (quint32 i = 0; i < tmp->polyOrder; ++i) {
		vecTmp(i) = 0.f;
		for (quint32 j = 0; j < settings->pulseSize; ++j)
			vecTmp(i) += (*polyVectors[i])(j) * *(begPulse + j);
	}
	polyValues = polyMatrix * vecTmp;
	polyRestoredPulse = Eigen::VectorXf::Zero(settings->pulseSize);
	for (quint32 i = 0; i < tmp->polyOrder; ++i)
		polyRestoredPulse += polyValues(i) * (*polyVectors[i]);

	float bl = 0.f;
	if (settings->aSet->processBaselineSamples) {
		for (auto curr = begPulse, end = begPulse + tmp->processBaselineSamples; curr != end; ++curr) bl += *curr;
		bl /= static_cast<float> (tmp->processBaselineSamples);
	}

	return *std::max_element(polyRestoredPulse.data() + tmp->maxValIntervalLeft, polyRestoredPulse.data() + tmp->maxValIntervalRight) - bl;
}

void PulseAmplMeasuringPolyMax::update_settings() {
	AmplPolyMaxSettings* tmp = (AmplPolyMaxSettings*)settings->aSet.get();
	assert (tmp->maxValIntervalLeft < tmp->maxValIntervalRight);
	assert (tmp->processBaselineSamples <= tmp->maxValIntervalLeft);

	for (quint16 i = 0, iend = polyVectors.size(); i < iend; ++i)
		delete polyVectors[i];
	polyVectors.resize(0);
	Eigen::VectorXf* vecTmp;
	Eigen::MatrixXf matrTmp (tmp->polyOrder, tmp->polyOrder);
	for (quint16 i = 0; i < tmp->polyOrder; ++i) {
		vecTmp = new Eigen::VectorXf (settings->pulseSize);
		for (quint16 j = 0; j < settings->pulseSize; ++j)
			(*vecTmp)(j) = std::pow (static_cast<float> (j), i);
		polyVectors.push_back(vecTmp);
	}
	for (quint16 i = 0; i < tmp->polyOrder; ++i) {
		matrTmp(0, i) = (*polyVectors[0]).transpose() * (*polyVectors[i]);
		for (quint16 j = 0; j < i; ++j)
			matrTmp (i - j, j) = matrTmp(0, i);
	}
	for (quint16 i = 1; i < tmp->polyOrder; ++i) {
		matrTmp(i, tmp->polyOrder - 1) = (*polyVectors[i]).transpose() * (*polyVectors[tmp->polyOrder - 1]);
		for (quint16 j = 1; i + j < tmp->polyOrder; ++j)
			matrTmp (i + j, tmp->polyOrder - 1 - j) = matrTmp(i, tmp->polyOrder - 1);
	}
	polyMatrix = matrTmp.inverse();
	polyValues.resize(tmp->polyOrder);
}

void PulseAmplMeasuringPolyMax::save(std::ostream &os) const {
	AmplPolyMaxSettings& tmp = *(AmplPolyMaxSettings*)settings->aSet.get();
	os.write((char*)&tmp.processBaselineSamples, 4);
	os.write((char*)&tmp.maxValIntervalLeft, 4);
	os.write((char*)&tmp.maxValIntervalRight, 4);
	os.write((char*)&tmp.polyOrder, 4);
}

void PulseAmplMeasuringPolyMax::load(std::istream &is) {
	AmplPolyMaxSettings& tmp = *(AmplPolyMaxSettings*)settings->aSet.get();
	is.read((char*)&tmp.processBaselineSamples, 4);
	is.read((char*)&tmp.maxValIntervalLeft, 4);
	is.read((char*)&tmp.maxValIntervalRight, 4);
	is.read((char*)&tmp.polyOrder, 4);
	if (is.fail()) throw std::runtime_error ("");
}

PulseAmplMeasuringNeuralNet::PulseAmplMeasuringNeuralNet() {
}

float PulseAmplMeasuringNeuralNet::find_ampl(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse) {
	AmplNNSettings* tmp = (AmplNNSettings*)settings->aSet.get();
	assert ((std::vector<float>::size_type)(endPulse - begPulse) == tmp->neuralNet.inputs());

	float bl = 0.f;
	if (settings->aSet->processBaselineSamples) {
		for (auto curr = begPulse, end = begPulse + tmp->processBaselineSamples; curr != end; ++curr) bl += *curr;
		bl /= static_cast<float> (tmp->processBaselineSamples);
	}

	std::vector<float> tmpv (begPulse, endPulse);
	for (auto a = tmpv.begin(), b = tmpv.end(); a != b; ++a) *a -= bl;
	float maxVal = *std::max_element(begPulse, endPulse);
	for (auto a = tmpv.begin(), b = tmpv.end(); a != b; ++a) *a /= maxVal;

	tmp->neuralNet.work(tmpv);
	return tmp->neuralNet.get_output(0)*2*maxVal;
}

void PulseAmplMeasuringNeuralNet::save(std::ostream &os) const {
	AmplNNSettings& tmp = *(AmplNNSettings*)settings->aSet.get();
	tmp.neuralNet.save(os);
}

void PulseAmplMeasuringNeuralNet::load(std::istream &is) {
	AmplNNSettings& tmp = *(AmplNNSettings*)settings->aSet.get();
	tmp.neuralNet.load(is);
}

void PulseTimeMeasuring::set (std::shared_ptr<ProcessingThread::Settings> t) {
	assert(get_time_type() == t->tSet->get_t_settings_id());
	settings = t;
	update_settings();
}

std::shared_ptr<PulseTimeMeasuring> PulseTimeMeasuring::get_new (quint32 id) {
	switch (id) {
		case PulseTimeMeasuring::MaxVal:
			return std::shared_ptr<PulseTimeMeasuring> (new PulseTimeMeasuringByMax);
			break;
		case PulseTimeMeasuring::NeuralNet:
			return std::shared_ptr<PulseTimeMeasuring> (new PulseTimeMeasuringNeuralNet);
			break;
		default: assert(false);
	}
	return std::shared_ptr<PulseTimeMeasuring> (new PulseTimeMeasuringByMax);
}

PulseTimeMeasuringByMax::PulseTimeMeasuringByMax () {
}

float PulseTimeMeasuringByMax::find_time(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse) {
	TimeMaxSettings* tmp = (TimeMaxSettings*)settings->tSet.get();
	assert (begPulse + tmp->maxValIntervalLeft <= endPulse);
	assert (begPulse + tmp->maxValIntervalRight <= endPulse);

	return std::max_element(begPulse + tmp->maxValIntervalLeft, begPulse + tmp->maxValIntervalRight) - begPulse;
}

void PulseTimeMeasuringByMax::update_settings() {
	TimeMaxSettings* tmp = (TimeMaxSettings*)settings->tSet.get();
	assert (tmp->maxValIntervalLeft < tmp->maxValIntervalRight);
}

void PulseTimeMeasuringByMax::save(std::ostream &os) const {
	TimeMaxSettings& tmp = *(TimeMaxSettings*)settings->tSet.get();
	os.write((char*)&tmp.maxValIntervalLeft, 4);
	os.write((char*)&tmp.maxValIntervalRight, 4);
}

void PulseTimeMeasuringByMax::load(std::istream &is) {
	TimeMaxSettings& tmp = *(TimeMaxSettings*)settings->tSet.get();
	is.read((char*)&tmp.maxValIntervalLeft, 4);
	is.read((char*)&tmp.maxValIntervalRight, 4);
	if (is.fail()) throw std::runtime_error ("");
}

PulseTimeMeasuringNeuralNet::PulseTimeMeasuringNeuralNet() {
}

float PulseTimeMeasuringNeuralNet::find_time(std::vector<float>::iterator begPulse, std::vector<float>::iterator endPulse) {
	TimeNNSettings* tmp = (TimeNNSettings*)settings->tSet.get();
	assert ((std::vector<float>::size_type)(endPulse - begPulse) == tmp->neuralNet.inputs());

	float bl = 0.f;
	if (settings->aSet->processBaselineSamples) {
		for (auto curr = begPulse, end = begPulse + settings->aSet->processBaselineSamples; curr != end; ++curr) bl += *curr;
		bl /= static_cast<float> (settings->aSet->processBaselineSamples);
	}

	std::vector<float> tmpv (begPulse, endPulse);
	for (auto a = tmpv.begin(), b = tmpv.end(); a != b; ++a) *a -= bl;
	float maxVal = *std::max_element(begPulse, endPulse);
	for (auto a = tmpv.begin(), b = tmpv.end(); a != b; ++a) *a /= maxVal;

	tmp->neuralNet.work(tmpv);
	return tmp->neuralNet.get_output(0)*settings->pulseSize;
}

void PulseTimeMeasuringNeuralNet::save(std::ostream &os) const {
	TimeNNSettings& tmp = *(TimeNNSettings*)settings->tSet.get();
	tmp.neuralNet.save(os);
}

void PulseTimeMeasuringNeuralNet::load(std::istream &is) {
	TimeNNSettings& tmp = *(TimeNNSettings*)settings->tSet.get();
	tmp.neuralNet.load(is);
}

ProcessingThread::ProcessingThread(quint32 specSize) {
	spectrum.resize(specSize, 0);
	qRegisterMetaType<PulseInfo>("PulseInfo");
	callback = [&] () {
				std::vector<float>::iterator pos = pulSearch->get_iter();

				if (!settings->dSet->enabled ||
					(settings->dSet->enabled && pulDisc->discriminate(pos, pos + settings->pulseSize))) {
					++detectedLastSec;
					pulAmpl->measure(pos, pos + settings->pulseSize);
					pulTime->measure(pos, pos + settings->pulseSize);

					if (isPulseCollect) {
						std::vector<float> a (pos, pos + settings->pulseSize);
						if (settings->aSet->processBaselineSamples) {
							float bl = 0.f;
							for (auto b = a.begin(), be = b + settings->aSet->processBaselineSamples; b != be; ++b)
								bl += *b;
							bl /= settings->aSet->processBaselineSamples;
							for (auto &b: a) b -= bl;
						}
						setupDetectedPulses.push_back(a);
					}
					if (isSpectCollect) add_to_spectrum();

					lastDetectInfo.pos = pulSearch->get_pos();
					lastDetectInfo.ampl = pulAmpl->get_ampl();
					lastDetectInfo.time = pulTime->get_time();
					if (settings->enableSub) subtract(pos);
					pulse_detected(lastDetectInfo);
				}
			};

}

void ProcessingThread::add_to_spectrum () {
	if (pulAmpl->get_ampl() > 0.f && pulAmpl->get_ampl() < 0.999f)
		spectrum[pulAmpl->get_ampl()*spectrum.size()]++;
}

void ProcessingThread::subtract(std::vector<float>::iterator begPulse) {
	float difff = lastDetectInfo.time - pulShapeInfo.time;
	qint32 diff;
	if (difff < 0) diff = difff - 0.5f;
	else diff = difff + 0.5f;
	if (begPulse + diff < input.end() - settings->pulseSize && begPulse + diff > input.begin())
		for (quint32 i = 0, ie = settings->pulseSize; i < ie; ++i) {
			*(begPulse + i + diff) -= settings->shape[i]*lastDetectInfo.ampl/pulShapeInfo.ampl;
		}
}

void ProcessingThread::run() {
	mutex.lock();
	setupDetectedPulses.clear();
	process();
	mutex.unlock();
}

void ProcessingThread::set_settings (std::shared_ptr<Settings> s) {
	assert(s->get_settings_id() == get_process_type());
	*settings = *s;
	if (pulSearch->get_search_type() != settings->sSet->get_s_settings_id()) {
		pulSearch = PulseSearching::get_new(settings->sSet->get_s_settings_id());
	}
	if (pulDisc->get_disc_type() != settings->dSet->get_d_settings_id()) {
		pulDisc = PulseDiscriminator::get_new(settings->dSet->get_d_settings_id());
	}
	if (pulAmpl->get_ampl_type() != settings->aSet->get_a_settings_id()) {
		pulAmpl = PulseAmplMeasuring::get_new(settings->aSet->get_a_settings_id());
	}
	if (pulTime->get_time_type() != settings->tSet->get_t_settings_id()) {
		pulTime = PulseTimeMeasuring::get_new(settings->tSet->get_t_settings_id());
	}

	pulSearch->set (settings);
	pulDisc->set (settings);
	pulAmpl->set (settings);
	pulTime->set (settings);

	pulSearch->set_callback(callback);

	update_settings();

	if (settings->shape.size() == settings->pulseSize) {
		pulAmpl->measure(settings->shape.begin(), settings->shape.end());
		pulTime->measure(settings->shape.begin(), settings->shape.end());

		pulShapeInfo.ampl = pulAmpl->get_ampl();
		pulShapeInfo.time = pulTime->get_time();
	} else if (settings->enableSub) settings->enableSub = false;
}

void ProcessingThread::set_input(const std::vector<float> *inp) {
	if (inp->size() + settings->pulseSize != input.size()) {
		input = std::vector<float> (inp->size() + settings->pulseSize, 0.f);
	}
	memcpy(input.data(), input.data() + inp->size(), 4*settings->pulseSize);
	memcpy(input.data() + settings->pulseSize, inp->data(), 4*inp->size());
}

std::vector<float> ProcessingThread::get_processed () const {
	if (input.size()) return std::vector<float> (input.data() + settings->pulseSize, input.data() + input.size());
	else return std::vector<float> (0);
}

void ProcessingThread::save (std::ostream& os) const {
	quint32 t;
	quint8 c;
	float f;

	os.write((char*)&(t = settings->pulseSize), 4);
	os.write((char*)&(t = settings->shape.size()), 4);
	os.write((char*)&(f = settings->amplCorrection), 4);
	os.write((char*)&(t = settings->inputNum), 4);
	os.write((char*)&(c = settings->enableSub), 1);
	os.write((char*)settings->shape.data(), 4*settings->shape.size());

	os.write((char*)&(t = name.toUtf8().size()), 4);
	os.write(name.toUtf8().data(), name.toUtf8().size());
	os.write((char*)spectrum.data(), 4*spectrum.size());

	os.write((char*)&(t = pulSearch->get_search_type()), 4);
	pulSearch->save(os);
	os.write((char*)&(t = pulDisc->get_disc_type()), 4);
	pulDisc->save(os);
	os.write((char*)&(t = pulAmpl->get_ampl_type()), 4);
	pulAmpl->save(os);
	os.write((char*)&(t = pulTime->get_time_type()), 4);
	pulTime->save(os);
}

void ProcessingThread::load (std::istream& is) {
	quint32 t;
	quint8 c;
	float f;

	is.read((char*)&t, 4);
	settings->pulseSize = t;
	is.read((char*)&t, 4);
	settings->shape.resize(t);
	is.read((char*)&f, 4);
	settings->amplCorrection = f;
	is.read((char*)&t, 4);
	settings->inputNum = t;
	is.read((char*)&c, 1);
	settings->enableSub = c;
	is.read((char*)settings->shape.data(), 4*settings->shape.size());

	is.read((char*)&t, 4);
	QByteArray bAtmp (t, 0);
	is.read(bAtmp.data(), t);
	name = QString (bAtmp);

	is.read((char*)spectrum.data(), 4*spectrum.size());


	is.read((char*)&t, 4);
	if (t != pulSearch->get_search_type()) switch (t) {
		case PulseSearching::Threshold:
			pulSearch = std::shared_ptr<PulseSearching> (new PulseSearchingThreshold);
			break;
		case PulseSearching::Monoton:
			pulSearch = std::shared_ptr<PulseSearching> (new PulseSearchingMonoton);
			break;
		case PulseSearching::TanThreshold:
			pulSearch = std::shared_ptr<PulseSearching> (new PulseSearchingTanThreshold);
			break;
		default: throw std::runtime_error("");
	}
	if (settings->sSet->get_s_settings_id() != t) settings->sSet = PulseSearching::SearchSettings::get_new(t);
	pulSearch->set(settings);
	pulSearch->load(is);

	is.read((char*)&t, 4);
	if (t != pulDisc->get_disc_type()) switch (t) {
		case PulseDiscriminator::Dispersion:
			pulDisc = std::shared_ptr<PulseDiscriminator> (new PulseDiscriminatorByDispersion);
			break;
		case PulseDiscriminator::NeuralNet:
			pulDisc = std::shared_ptr<PulseDiscriminator> (new PulseDiscriminatorByNeuralNet);
			break;
		default: throw std::runtime_error("");
	}
	if (settings->dSet->get_d_settings_id() != t) settings->dSet = PulseDiscriminator::DiscSettings::get_new(t);
	pulDisc->set(settings);
	pulDisc->load(is);

	is.read((char*)&t, 4);
	if (t != pulAmpl->get_ampl_type()) switch (t) {
		case PulseAmplMeasuring::MaxVal:
			pulAmpl = std::shared_ptr<PulseAmplMeasuring> (new PulseAmplMeasuringByMax);
			break;
		case PulseAmplMeasuring::Polynomial:
			pulAmpl = std::shared_ptr<PulseAmplMeasuring> (new PulseAmplMeasuringPolyMax);
			break;
		case PulseAmplMeasuring::NeuralNet:
			pulAmpl = std::shared_ptr<PulseAmplMeasuring> (new PulseAmplMeasuringNeuralNet);
			break;
		default: throw std::runtime_error("");
	}
	if (settings->aSet->get_a_settings_id() != t) settings->aSet = PulseAmplMeasuring::AmplSettings::get_new(t);
	pulAmpl->set(settings);
	pulAmpl->load(is);

	is.read((char*)&t, 4);
	if (t != pulTime->get_time_type()) switch (t) {
		case PulseTimeMeasuring::MaxVal:
			pulTime = std::shared_ptr<PulseTimeMeasuring> (new PulseTimeMeasuringByMax);
			break;
		case PulseTimeMeasuring::NeuralNet:
			pulTime = std::shared_ptr<PulseTimeMeasuring> (new PulseTimeMeasuringNeuralNet);
			break;
		default: throw std::runtime_error("");
	}
	if (settings->tSet->get_t_settings_id() != t) settings->tSet = PulseTimeMeasuring::TimeSettings::get_new(t);
	pulTime->set(settings);
	pulTime->load(is);
}

ProcessingStandartCircuit::ProcessingStandartCircuit(quint32 specSize) : ProcessingThread (specSize) {
	settings = std::shared_ptr<Settings> (new StanCircuitSettings);
	pulSearch = std::shared_ptr<PulseSearching> (new PulseSearchingThreshold);
	pulDisc = std::shared_ptr<PulseDiscriminator> (new PulseDiscriminatorByDispersion);
	pulAmpl = std::shared_ptr<PulseAmplMeasuring> (new PulseAmplMeasuringByMax);
	pulTime = std::shared_ptr<PulseTimeMeasuringByMax> (new PulseTimeMeasuringByMax);
	pulSearch->set(settings);
	pulDisc->set(settings);
	pulAmpl->set(settings);
	pulTime->set(settings);

	pulSearch->set_callback(callback);
}

void ProcessingStandartCircuit::process() {
	pulSearch->search(input.begin(), input.end() - settings->pulseSize, 0);
}

ProcessingCoincidenceCircuit::ProcessingCoincidenceCircuit(quint32 specSize) : ProcessingThread (specSize) {
	settings = std::shared_ptr<Settings> (new CoinCircuitSettings);
	pulSearch = std::shared_ptr<PulseSearching> (new PulseSearchingThreshold);
	pulDisc = std::shared_ptr<PulseDiscriminator> (new PulseDiscriminatorByDispersion);
	pulAmpl = std::shared_ptr<PulseAmplMeasuring> (new PulseAmplMeasuringByMax);
	pulTime = std::shared_ptr<PulseTimeMeasuringByMax> (new PulseTimeMeasuringByMax);
	pulSearch->set(settings);
	pulDisc->set(settings);
	pulAmpl->set(settings);
	pulTime->set(settings);

	callback = [&] () {
				std::vector<float>::iterator pos = pulSearch->get_iter();

				if (!settings->dSet->enabled ||
					(settings->dSet->enabled && pulDisc->discriminate(pos, pos + settings->pulseSize))) {
					++detectedLastSec;
					pulAmpl->measure(pos, pos + settings->pulseSize);
					pulTime->measure(pos, pos + settings->pulseSize);

					float timeDiff = pulSearch->get_pos() + pulTime->get_time() - coinPulse.pos - coinPulse.time;

					CoinCircuitSettings* tmp = (CoinCircuitSettings*)settings.get();
					if (timeDiff*timeDiff > tmp->maxTimeDifference*tmp->maxTimeDifference) return;

					if (isPulseCollect) {
						std::vector<float> a (pos, pos + settings->pulseSize);
						if (settings->aSet->processBaselineSamples) {
							float bl = 0.f;
							for (auto b = a.begin(), be = b + settings->aSet->processBaselineSamples; b != be; ++b)
								bl += *b;
							bl /= settings->aSet->processBaselineSamples;
							for (auto &b: a) b -= bl;
						}
						setupDetectedPulses.push_back(a);
					}
					if (isSpectCollect) add_to_spectrum();

					lastDetectInfo.pos = pulSearch->get_pos();
					lastDetectInfo.ampl = pulAmpl->get_ampl();
					lastDetectInfo.time = pulTime->get_time();
					if (settings->enableSub) subtract(pos);
					pulse_detected(lastDetectInfo);
				}
			};
	pulSearch->set_callback(callback);
	pulSearch->set_single(true);

	coincidenceWith = this;
	connect (coincidenceWith, SIGNAL(pulse_detected(PulseInfo)), this, SLOT(detect_event(PulseInfo)));
}

void ProcessingCoincidenceCircuit::update_settings() {
	pulSearch->set_single(true);
}

void ProcessingCoincidenceCircuit::set_coincidence(ProcessingThread *coincidence) {

	if (sourceNDeleted) {
		disconnect (coincidenceWith, SIGNAL(pulse_detected(PulseInfo)), this, SLOT(detect_event(PulseInfo)));
		disconnect (coincidenceWith, SIGNAL(deleted()), this, SLOT(source_deleted()));
	}
	coincidenceWith = coincidence;
	sourceNDeleted = true;
	connect (coincidenceWith, SIGNAL(pulse_detected(PulseInfo)), this, SLOT(detect_event(PulseInfo)));
	connect (coincidenceWith, SIGNAL(deleted()), this, SLOT(source_deleted()));
}

void ProcessingCoincidenceCircuit::detect_event(PulseInfo p) {
	CoinCircuitSettings* tmp = (CoinCircuitSettings*)settings.get();
	std::vector<float>::iterator beg, end;

	if (p.ampl < tmp->amplitudeIntervalL || p.ampl > tmp->amplitudeIntervalR) return;

	if (p.pos > tmp->maxTimeDifference*2) {
		beg = input.begin() + (p.pos - tmp->maxTimeDifference*2);
	}
	else beg = input.begin();
	if (input.begin() + p.pos + 2*tmp->maxTimeDifference + 2*settings->pulseSize < input.end()) {
		end = input.begin() + (p.pos + tmp->maxTimeDifference*2 + settings->pulseSize);
	}
	else end = input.end() - settings->pulseSize;
	coinPulse = p;
	assert(input.end() - end >= settings->pulseSize);
	assert(beg - input.begin() >= 0);
	pulSearch->search(beg, end, beg - input.begin());
}

void ProcessingCoincidenceCircuit::source_deleted() {
	sourceNDeleted = false;
}

void ProcessingCoincidenceCircuit::save(std::ostream &os) const {
	ProcessingThread::save(os);
	CoinCircuitSettings* tmp = (CoinCircuitSettings*)settings.get();

	os.write((char*)&tmp->coinIndex, 4);
	os.write((char*)&tmp->amplitudeIntervalL, 4);
	os.write((char*)&tmp->amplitudeIntervalR, 4);
	os.write((char*)&tmp->maxTimeDifference, 4);
}

void ProcessingCoincidenceCircuit::load(std::istream &is) {
	ProcessingThread::load(is);
	CoinCircuitSettings* tmp = (CoinCircuitSettings*)settings.get();

	is.read((char*)&tmp->coinIndex, 4);
	is.read((char*)&tmp->amplitudeIntervalL, 4);
	is.read((char*)&tmp->amplitudeIntervalR, 4);
	is.read((char*)&tmp->maxTimeDifference, 4);
}

PulseProcessing::PulseProcessing(quint32 specSize) : QObject () {
	threads.push_back(std::shared_ptr<ProcessingThread> (new ProcessingStandartCircuit (specSize)));
	threads[0]->set_name("Spectrum_0");
}

PulseProcessing::~PulseProcessing() {
	mutex.lock();
	mutex.unlock();
}

void PulseProcessing::set_inputs(std::vector<std::vector<float> const*> _inputs) {
	mutex.lock();
	inputs = _inputs;
	mutex.unlock();
}

void PulseProcessing::set_settings(std::shared_ptr<ProcessingThread::Settings> set, quint32 index) {
	mutex.lock();
	quint32 specSize = get_spectrum(0)->size();
	assert (index < threads.size());
	if (threads[index]->get_process_type() != set->get_settings_id()) {
		switch (set->get_settings_id()) {
			case ProcessingThread::StandartCircuit:
				threads[index] = std::shared_ptr<ProcessingThread> (new ProcessingStandartCircuit (specSize));
				break;
			case ProcessingThread::CoincidenceCircuit:
				threads[index] = std::shared_ptr<ProcessingThread> (new ProcessingCoincidenceCircuit (specSize));
				break;
			default:
				assert(false);
				break;
		}
	}
	threads[index]->set_settings(set);
	update_settings();
	mutex.unlock();
}

ProcessingThread::Settings const* PulseProcessing::get_settings(quint32 index) const {
	assert (index < threads.size());
	return threads[index]->get_settings();
}

void PulseProcessing::set_threads(quint32 size) {
	mutex.lock();
	quint32 specSize = get_spectrum(0)->size();
	assert(size);
	while (threads.size() < size) threads.push_back(std::shared_ptr<ProcessingThread> (new ProcessingStandartCircuit (specSize)));
	while (threads.size() > size) threads.pop_back();
	inputs.resize(threads.size(), 0);
	mutex.unlock();
}

void PulseProcessing::set_process_type(quint32 type, quint32 thread) {
	mutex.lock();
	quint32 specSize = get_spectrum(0)->size();
	if (threads[thread]->get_process_type() != type) {
		switch (type) {
			case ProcessingThread::StandartCircuit:
				threads[thread] = std::shared_ptr<ProcessingThread> (new ProcessingStandartCircuit (specSize));
				break;
			case ProcessingThread::CoincidenceCircuit:
				threads[thread] = std::shared_ptr<ProcessingThread> (new ProcessingCoincidenceCircuit (specSize));
				break;
			default:
				assert(false);
		}
	}
	update_settings();
	mutex.unlock();
}

void PulseProcessing::set_setup_mode(bool mode) {
	mutex.lock();
	setupMode = mode;
	for (auto& a: threads) a->set_spect_collect(!mode);
	threads[setupStream]->set_pulse_collect(mode);
	mutex.unlock();
}

void PulseProcessing::set_setup_thread(quint32 thread) {
	mutex.lock();
	assert (thread < threads.size());
	if (setupMode) {
		threads[setupStream]->set_pulse_collect(false);
		threads[thread]->set_pulse_collect(true);
	}
	setupStream = thread;
	mutex.unlock();
}

void PulseProcessing::process() {
	mutex.lock();
	for (quint32 i = 0, ie = threads.size(); i < ie; ++i) threads[i]->set_input(inputs[threads[i]->get_settings()->inputNum]);
	for (quint32 i = 0, ie = threads.size(); i < ie; ++i) threads[i].get()->run();
	finished();
	mutex.unlock();
}

void PulseProcessing::update_settings() {
	for (auto& a: threads) {
		switch (a->get_process_type()) {
			case ProcessingThread::StandartCircuit:
				break;
			case ProcessingThread::CoincidenceCircuit: {
				ProcessingCoincidenceCircuit* b = (ProcessingCoincidenceCircuit*) a.get();
				ProcessingCoincidenceCircuit::CoinCircuitSettings* tmp = (ProcessingCoincidenceCircuit::CoinCircuitSettings*) a->get_settings();
				b->set_coincidence(threads[tmp->coinIndex].get());
				break;
			} default:
				break;
		}
	}
}

std::vector<std::vector<float>> const* PulseProcessing::get_setup_pulses() {
	return threads[setupStream]->get_setup_pulses();
}

void PulseProcessing::save(std::ostream &os) const {
	quint32 t;
	os.write((char*)&(t = threads.size()), 4);
	os.write((char*)&(t = threads[0]->get_spectrum()->size()), 4);
	for (auto& a : threads) {
		os.write((char*)&(t = a->get_process_type()), 4);
		a->save(os);
	}
}

void PulseProcessing::load(std::istream &is) {
	mutex.lock();
	quint32 t;
	quint32 specSize;
	is.read((char*)&t, 4);
	is.read((char*)&specSize, 4);
	mutex.unlock();
	set_threads(t);
	mutex.lock();
	for (auto& a : threads) {
		is.read((char*)&t, 4);
		switch (t) {
			case ProcessingThread::StandartCircuit: { a = std::shared_ptr<ProcessingThread> (new ProcessingStandartCircuit(specSize)); break; }
			case ProcessingThread::CoincidenceCircuit: { a = std::shared_ptr<ProcessingThread> (new ProcessingCoincidenceCircuit(specSize)); break; }
			default: assert(false);
		}
		a->load(is);
	}
	update_settings();
	mutex.unlock();
}
