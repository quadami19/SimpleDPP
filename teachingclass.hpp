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

#ifndef TEACHINGCLASS_HPP
#define TEACHINGCLASS_HPP

#include <QObject>
#include <ctime>
#include <random>
#include <limits>
#include <iostream>
#include <ios>
#include "perceptron.hpp"
namespace Neural_Network {

class Perceptron;

class TeachingClass : public QObject {
		Q_OBJECT
		Perceptron* teachingPerceptron_ptr;
		Perceptron* currentPerceptron;
		quint32 totalIterations = 2;
		quint32 backPropagationMaxRepeats = 5;
		quint32 heatImitationNumberOfJumps = 20;
		double backPropagationSpeed = 1;
		double backPropagationSpeedCorrection = 1.1;
		double heatImitationInitialTemperature = 1.e-5;
		double heatImitationTemperatureReductionPerJump = .75;
		double heatImitationJumpRadius = 0.2;
		double heatImitationTemperatureCorrection = 1.1;
		double heatImitationConstWeightMult = 0.14;
		std::vector<float> teachingWeights;
		std::default_random_engine generator;
		std::vector<std::vector<float>> backPropagationErrorsVec;
		std::vector<std::vector<std::vector<float>>> backPropagationWeightsBufferVec;

		double currentResult = 1.;
		double bestResult = 1.;
		double currSpeed;
		double currInitTemp;
		quint32 iProgress = 0;
		bool teachingState = false;
		bool isEnabled = true;

		void teach_back_propagation ();
		void teach_heat_imitation (quint32 l, quint32 n, quint32 w);
		void get_error ();
		void save_current (bool replace = true);

	protected:

		std::vector<std::pair<std::vector<float>, std::vector<float>>> teachData;
		double start (Perceptron* _percep);

	public:

		explicit TeachingClass();
		virtual ~TeachingClass() {}
		double start (Perceptron* _percep, const std::vector<std::pair<std::vector<float>, std::vector<float> > >& _teachDataInOut);
		void set_teaching_outputs (quint32 outp) { teachingWeights.resize(outp); }
		void set_output_teach_weight (quint32 outp, float weight);

		void total_iterations (quint32 _iter) { totalIterations = _iter; }
		quint32 total_iterations() { return totalIterations; }
		void back_propagation_speed (double _speed) { backPropagationSpeed = _speed; }
		double back_propagation_speed() { return backPropagationSpeed; }
		void back_propagation_speed_corr (double _corr) { backPropagationSpeedCorrection = _corr; }
		double back_propagation_speed_corr () { return backPropagationSpeedCorrection; }
		void heat_imitation_initial_temperature (double _temp) { heatImitationInitialTemperature = _temp; }
		double heat_imitation_initial_temperature() { return heatImitationInitialTemperature; }
		void heat_imitation_temperature_reduction (double _red) { heatImitationTemperatureReductionPerJump = _red; }
		double heat_imitation_temperature_reduction () { return heatImitationTemperatureReductionPerJump; }
		void heat_imitation_jump_radius (double _rad) { heatImitationJumpRadius = _rad; }
		double heat_imitation_jump_radius() { return heatImitationJumpRadius; }
		void heat_imitation_temp_correction (double _corr) { heatImitationTemperatureCorrection = _corr; }
		double heat_imitation_temp_correction () { return heatImitationTemperatureCorrection; }
		void heat_imitation_const_wei_mult (double _mult) { heatImitationConstWeightMult = _mult; }
		double heat_imitation_const_wei_mult () { return heatImitationConstWeightMult; }
		void heat_imitation_jumps (quint32 jumps) { heatImitationNumberOfJumps = jumps; }
		quint32 heat_imitation_jumps () { return heatImitationNumberOfJumps; }
		double current_STD () { if (bestResult < currentResult) return bestResult; else return currentResult; }
		float current_progress ();
		bool running () { return teachingState; }
		bool enabled () { return isEnabled; }
		void enable (bool _value) { isEnabled = _value; }

	signals:

	public slots:
};

}

#endif // TEACHINGCLASS_HPP
