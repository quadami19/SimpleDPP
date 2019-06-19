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

#include "teachingclass.hpp"

namespace Neural_Network {

static std::uniform_real_distribution<float> randEvent (0.f, 1.f);

void TeachingClass::get_error() {
	double dtmp (.0);
	std::vector<float> errorVec;
	for (quint32 i = 0; i < teachData.size(); ++i) {
		errorVec = currentPerceptron->get_error(teachData[i].first, teachData[i].second);
		for (quint32 n = 0; n < teachingWeights.size(); ++n) dtmp += errorVec[n] * errorVec[n] * teachingWeights[n];
	}
	currentResult = std::sqrt(dtmp/teachData.size());
}

void TeachingClass::save_current(bool replace) {
	if (currentResult < bestResult) {
		*teachingPerceptron_ptr = *currentPerceptron;
		bestResult = currentResult;
	} else if (replace) {
		*currentPerceptron = *teachingPerceptron_ptr;
		currentResult = bestResult;
	}
	get_error();
}

void TeachingClass::teach_back_propagation() {
	float temp;
	double dtmp = currentResult;
	quint32 l2, n2, w2, continues, count = 0;
	while (1) {
		for (l2 = 0; l2 < backPropagationWeightsBufferVec.size(); ++l2)
			for (n2 = 0; n2 < backPropagationWeightsBufferVec[l2].size(); ++n2)
				for (w2 = 0; w2 < backPropagationWeightsBufferVec[l2][n2].size(); ++w2)
					backPropagationWeightsBufferVec[l2][n2][w2] = .0f;
		for (quint32 i = 0; i < teachData.size(); ++i) {
			currentPerceptron->work(teachData[i].first);
			for (l2 = 0; l2 < teachingWeights.size(); ++l2) {
				neuron_base& a = currentPerceptron->neuronsVec[currentPerceptron->layers() - 1][l2];
				backPropagationErrorsVec[currentPerceptron->neuronsVec.size()-1][l2] = a.get_func_diff() * (teachData[i].second[l2] - a);
			}
			for (l2 = currentPerceptron->neuronsVec.size() - 1; l2 != 0; --l2) for (n2 = 0; n2 < teachingPerceptron_ptr->neuronsVec[l2-1].size(); ++n2) {
				temp = .0f;
				for (w2 = 0; w2 < currentPerceptron->neuronsVec[l2].size(); ++w2) temp += backPropagationErrorsVec[l2][w2] * (currentPerceptron->neuronsVec[l2][w2]).get_weights()[n2];
				backPropagationErrorsVec[l2-1][n2] = currentPerceptron->neuronsVec[l2-1][n2].get_func_diff() * temp;
			}
			for (n2 = 0; n2 < currentPerceptron->neuronsVec[0].size(); ++n2) for (w2 = 0; w2 < currentPerceptron->input.size(); ++w2)
				backPropagationWeightsBufferVec[0][n2][w2] += currSpeed * backPropagationErrorsVec [0][n2] * teachData[i].first[w2];
			for (l2 = 1; l2 < currentPerceptron->neuronsVec.size(); ++l2) for (n2 = 0; n2 < currentPerceptron->neuronsVec[l2].size(); ++n2) for (w2 = 0; w2 < currentPerceptron->neuronsVec[l2-1].size(); ++w2)
				backPropagationWeightsBufferVec[l2][n2][w2] += currSpeed * backPropagationErrorsVec [l2][n2] * currentPerceptron->neuronsVec[l2-1][w2];
		}
		continues = 0;
		do {
			if (!isEnabled) return;
			dtmp = currentResult;
			for (l2 = 0; l2 < currentPerceptron->neuronsVec.size(); ++l2) for (n2 = 0; n2 < currentPerceptron->neuronsVec[l2].size(); ++n2)
				if (l2) for (w2 = 0; w2 < currentPerceptron->neuronsVec[l2-1].size(); ++w2)
					currentPerceptron->neuronsVec[l2][n2].drift_weight
							((backPropagationWeightsBufferVec[l2][n2][w2] *= backPropagationSpeedCorrection)/(float)teachData.size(), w2);
				else for (w2 = 0; w2 < currentPerceptron->input.size(); ++w2)
					currentPerceptron->neuronsVec[l2][n2].drift_weight
							((backPropagationWeightsBufferVec[l2][n2][w2] *= backPropagationSpeedCorrection)/(float)teachData.size(), w2);
			get_error();
			currSpeed *= backPropagationSpeedCorrection;
			//std::cout << "bprp " << continues << "\t" << currentResult << "\t" << currSpeed << std::endl;
			continues++;
		} while (dtmp > currentResult);
		currSpeed /= std::pow(backPropagationSpeedCorrection, 8);
		for (l2 = 0; l2 < currentPerceptron->neuronsVec.size(); ++l2) for (n2 = 0; n2 < currentPerceptron->neuronsVec[l2].size(); ++n2)
			if (l2) for (w2 = 0; w2 < currentPerceptron->neuronsVec[l2-1].size(); ++w2)
				currentPerceptron->neuronsVec[l2][n2].drift_weight (-backPropagationWeightsBufferVec[l2][n2][w2]/(float)teachData.size(), w2);
			else for (w2 = 0; w2 < currentPerceptron->input.size(); ++w2)
				currentPerceptron->neuronsVec[l2][n2].drift_weight (-backPropagationWeightsBufferVec[l2][n2][w2]/(float)teachData.size(), w2);
		currentResult = dtmp;
		if (continues < 2 || count++ > backPropagationMaxRepeats) return;
	}
}

void TeachingClass::teach_heat_imitation(quint32 l, quint32 n, quint32 w) {
	std::normal_distribution<float> jumpGet (0.f, heatImitationJumpRadius);
	std::default_random_engine generator;
	generator.seed(time(0));
	double dTmp;
	float temperature (currInitTemp), weight, dweight, con, dcon;
	weight = currentPerceptron->neuronsVec[l][n].get_weights()[w];
	if (weight < 0.f) weight = -weight;
	con = currentPerceptron->neuronsVec[l][n].get_const();
	if (con < 0.f) con = -con;
	con += 1.f;
	weight += 1.f;
	get_error();
	for (quint32 i = 0; i < heatImitationNumberOfJumps; ++i) {
		if (!isEnabled) return;
		dTmp = currentResult;
		dweight = jumpGet(generator) * weight;
		dcon = jumpGet(generator) * con * heatImitationConstWeightMult;
		currentPerceptron->neuronsVec[l][n].drift_weight (dweight, w);
		currentPerceptron->neuronsVec[l][n].drift_const (dcon);
		get_error();
		dTmp = (dTmp - currentResult);
		if ((dTmp < 0.) && (randEvent(generator) > std::exp(dTmp/temperature/currentResult))) {
			currentPerceptron->neuronsVec[l][n].drift_weight (-dweight, w);
			currentPerceptron->neuronsVec[l][n].drift_const (-dcon);
			currentResult += dTmp;
			currInitTemp *= std::pow(heatImitationTemperatureCorrection, 2);
		} else if (dTmp > 0.) currInitTemp /= heatImitationTemperatureCorrection;
		temperature *= heatImitationTemperatureReductionPerJump;
		save_current(false);
		//std::cout << "heat\t" << currentResult << "\tD=" << -dTmp << "\t" << currInitTemp << std::endl;
		if (dTmp < 0.) dTmp = -dTmp;
		if (dTmp/currentResult < 1.e-9) return;
	}
}

TeachingClass::TeachingClass() {
}

double TeachingClass::start(Perceptron *_percep) {
	teachingPerceptron_ptr = _percep;
	currentPerceptron = new Perceptron (*_percep);
	get_error();
	bestResult = currentResult;
	currSpeed = backPropagationSpeed;
	currInitTemp = heatImitationInitialTemperature;
	quint32 t, n, m;
	qint32 i;
	backPropagationWeightsBufferVec.resize(teachingPerceptron_ptr->neuronsVec.size());
	backPropagationErrorsVec.resize(teachingPerceptron_ptr->neuronsVec.size());
	for (i = 0; i < (qint32)teachingPerceptron_ptr->neuronsVec.size(); ++i) {
		backPropagationWeightsBufferVec[i].resize(teachingPerceptron_ptr->neuronsVec[i].size());
		backPropagationErrorsVec[i].resize(teachingPerceptron_ptr->neuronsVec[i].size());
		for (n = 0; n < teachingPerceptron_ptr->neuronsVec[i].size(); ++n)
			if (i) backPropagationWeightsBufferVec[i][n].resize(teachingPerceptron_ptr->neuronsVec[i-1].size(), .0f);
			else backPropagationWeightsBufferVec[i][n].resize(teachingPerceptron_ptr->input.size(), .0f);
	}
	teachingState = true;
	isEnabled = true;
	std::cout.precision(10);
	for (t = 0; t < totalIterations; ++t) {
		for (i = teachingPerceptron_ptr->neuronsVec.size()-1; i >= 0; --i)
			for (n = 0; n < teachingPerceptron_ptr->neuronsVec[i].size(); ++n) {
				if (i) for (m = 0; m < teachingPerceptron_ptr->neuronsVec[i-1].size(); ++m) {
					if (isEnabled) {
						teach_heat_imitation(i, n, m);
						teach_back_propagation();
						++iProgress;
						save_current(true);
					} else {
						t = totalIterations;
						break;
					}
				} else for (m = 0; m < teachingPerceptron_ptr->inputs(); ++m) {
					if (isEnabled) {
						teach_heat_imitation(i, n, m);
						teach_back_propagation();
						++iProgress;
						save_current(true);
					} else {
						t = totalIterations;
						break;
					}
				}
			}
	}
	delete currentPerceptron;
	iProgress = 0;
	teachingState = false;
	return currentResult;
}

double TeachingClass::start(Perceptron *_percep, const std::vector<std::pair<std::vector<float>, std::vector<float> > >& _teachDataInOut) {
	if (_percep->is_empty()) throw std::invalid_argument ("Error! Empty perceptron.");
	if (_teachDataInOut.empty()) throw std::invalid_argument ("Error! Empty teaching data.");
	if (_teachDataInOut[0].first.size() != _percep->input.size()) throw std::invalid_argument ("Error! Input size is wrong.");
	if (_percep->neuronsVec[_percep->neuronsVec.size() - 1].size() != _teachDataInOut[0].second.size()) throw std::invalid_argument ("Error! Teach data output size is wrong.");
	if (teachingWeights.size() != _teachDataInOut[0].second.size())  throw std::invalid_argument ("Error! Weights vector size is wrong.");
	teachData = _teachDataInOut;
	return start(_percep);
}

void TeachingClass::set_output_teach_weight(quint32 outp, float weight) {
	if (outp > teachingWeights.size()) throw std::invalid_argument ("Error! Invalid output index.\n");
	if (weight < 0.f || weight > 1.f) throw std::invalid_argument ("Error! Invalid output teaching weight.\n");
	teachingWeights[outp] = weight;
}

float TeachingClass::current_progress()  {
	if (iProgress == teachingPerceptron_ptr->total_weights()*totalIterations) return 1.f;
	else return static_cast<float> (iProgress)/static_cast<float> (teachingPerceptron_ptr->total_weights()*totalIterations);
}

}
