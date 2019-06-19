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

#ifndef NEWNEURON_HPP
#define NEWNEURON_HPP

#include <QtGlobal>
#include <vector>
#include <cmath>
#include <iostream>
#include <cassert>


namespace Neural_Network {

enum Activation_Functions_IDs {
	SIGMOID_ID = (quint32)0,
	LINEAR_ID,
	HIPERBOLIC_TAN_ID
};

class Activation_Function {

	public:
		Activation_Function() {}
		~Activation_Function() {}
		virtual float operator() (float f) = 0;
		virtual float diff (float f) const = 0;
		virtual quint32 get_id () const = 0;
};

class Sigmoid : public Activation_Function {

	public:
		Sigmoid() {}
		~Sigmoid() {}
		float operator() (float f) { return 1.f/(1.f + std::exp(f)); }
		float diff (float f) const { return f*(f-1.f); }
		quint32 get_id() const { return SIGMOID_ID; }
};

class Linear : public Activation_Function {
	public:
		Linear() {}
		~Linear() {}
		float operator() (float f) { return f; }
		float diff (float f) const { Q_UNUSED(f); return 1.f; }
		quint32 get_id() const { return LINEAR_ID; }
};

class Hiperbolic_Tan : public Activation_Function {
	public:
		Hiperbolic_Tan() {}
		~Hiperbolic_Tan() {}
		float operator() (float f) { return std::tanh(f); }
		float diff (float f) const { return 1.f - f*f; }
		quint32 get_id() const { return HIPERBOLIC_TAN_ID; }
};

class neuron_base {
		std::vector<float*> inputData;
		std::vector<float> weights;
		float constWeight = 0.f;
		float outputData = 0.f;
		Activation_Function* actfunc;

	public:

		neuron_base(quint32 _inputs = 32);
		neuron_base(const std::vector<float>& _weights);
		operator float() const { return outputData; }
		void set_weight (const std::vector<float>& _weights);
		void set_weight (float _weight, quint32 _index) { assert(_index<weights.size()); weights[_index] = _weight; }
		void set_const (float _const) { constWeight = _const; }
		void input (float* _ptr, quint32 _index) { assert(_index<inputData.size()); inputData[_index] = _ptr; }
		float* input (quint32 _index) const { assert(_index<inputData.size()); return inputData[_index]; }
		void inputs (quint32 _inputs) { assert (_inputs); inputData.resize (_inputs); weights.resize (_inputs); }
		quint32 inputs () const { return inputData.size(); }
		void set_activation_function (Activation_Function* _actfunc) { actfunc = _actfunc; }
		void set_activation_function (quint32 id);
		const std::vector<float>& get_weights () const { return weights; }
		float get_const () const { return constWeight; }
		float* get_output_ptr() { return &outputData; }
		float get_func_diff () const { return (*actfunc).diff(outputData); }
		float get_func_id () const { return (*actfunc).get_id(); }
		void calculate ();
		void drift_weight (float _driftVal, quint32 _index) { assert(_index<weights.size()); weights[_index] += _driftVal; }
		void drift_const (float _driftVal) { constWeight += _driftVal; }
};

}

#endif // NEWNEURON_HPP
