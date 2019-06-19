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

#include "neuron_base.hpp"

using namespace Neural_Network;

Sigmoid sigmoid;
Linear linear;
Hiperbolic_Tan hip_tan;

neuron_base::neuron_base(quint32 _inputs) {
	assert (_inputs);
	inputData.resize (_inputs);
	weights.resize (_inputs);
	actfunc = &sigmoid;
}

neuron_base::neuron_base(const std::vector<float> &_weights) {
	assert (_weights.size());
	inputData.resize (_weights.size());
	weights = _weights;
}

void neuron_base::set_activation_function(quint32 id) {
	switch (id) {
		case SIGMOID_ID:
			set_activation_function(&sigmoid);
			break;
		case LINEAR_ID:
			set_activation_function(&linear);
			break;
		case HIPERBOLIC_TAN_ID:
			set_activation_function(&hip_tan);
			break;
		default:
			break;
	}
}

void neuron_base::set_weight(const std::vector<float> &_weights) {
	assert (_weights.size());
	inputData.resize (_weights.size());
	weights = _weights;
}
void neuron_base::calculate() {
	float ftmp (constWeight);
	for (quint32 i = 0, iend = inputData.size(); i < iend; i++) ftmp += (*(inputData[i])) * weights[i];
	outputData = (*actfunc)(ftmp);
}
