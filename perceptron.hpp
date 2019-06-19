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

#ifndef PERCEPTRON_HPP
#define PERCEPTRON_HPP

#include "neuron_base.hpp"
#include "teachingclass.hpp"
#include <cstring>

namespace Neural_Network {

class TeachingClass;

class Perceptron {

		friend class TeachingClass;

	protected:

		std::vector<std::vector<neuron_base>> neuronsVec;
		std::vector<float> input;

	public:

		explicit Perceptron(quint32 _inputs = 32) { assert(_inputs); input.resize(_inputs); }
		Perceptron(const Perceptron& _percep);
		Perceptron& operator= (const Perceptron& _percep);
		void set_weight (float _weight, quint32 _layer, quint32 _neuron_index, quint32 _weight_index) { neuronsVec[_layer][_neuron_index].set_weight (_weight, _weight_index); }
		void set_weight (const std::vector<float>& _weights, quint32 _layer, quint32 _neuron_index);
		void set_const (float _const, quint32 _layer, quint32 _neuron) { neuronsVec[_layer][_neuron].set_const(_const); }
		void set_activation_function (quint32 _layer, quint32 _neuron, Activation_Function* _actfunc) { neuronsVec[_layer][_neuron].set_activation_function(_actfunc); }
		void set_activation_function (quint32 _layer, quint32 _neuron, quint32 _actfunc_id) { neuronsVec[_layer][_neuron].set_activation_function(_actfunc_id); }
		std::vector<float> get_error (const std::vector<float>& _inputVec, const std::vector<float>& _outputVec);
		const std::vector<float>& get_weights (quint32 layer, quint32 neuron) const { return neuronsVec[layer][neuron].get_weights(); }
		quint32 neurons (quint32 _layer) const { return neuronsVec[_layer].size(); }
		quint32 layers () const { return neuronsVec.size(); }
		quint32 weights (quint32 _layer, quint32 _neuron) const { return neuronsVec[_layer][_neuron].inputs(); }
		float get_const (quint32 _layer, quint32 _neuron) const { return neuronsVec[_layer][_neuron].get_const(); }
		float get_func_diff (quint32 _layer, quint32 _neuron) const { return neuronsVec[_layer][_neuron].get_func_diff(); }
		float get_func_id (quint32 _layer, quint32 _neuron) const { return neuronsVec[_layer][_neuron].get_func_id(); }
		float get_output (quint32 _index) { if (neuronsVec.empty()) return 0.f; else return neuronsVec[neuronsVec.size()-1][_index]; }
		quint32 total_neurons () const;
		quint32 total_weights () const;
		void inputs (quint32 _inputs);
		quint32 inputs () const { return input.size(); }
		quint32 outputs () const { if (neuronsVec.empty()) return 0; else return  neuronsVec[neuronsVec.size() - 1].size(); }
		bool is_empty () const { return neuronsVec.empty(); }
		void push_layer (quint32 _neurons);
		void pop_layer () { neuronsVec.pop_back(); }
		void clear () { neuronsVec.clear(); input.clear(); }
		void work (const std::vector<float>& _inputVec);
		void work (std::vector<float>::const_iterator begin, std::vector<float>::const_iterator end);
		void save (std::ostream& saveStream) const;
		void load (std::istream& loadStream);
};

}

#endif // PERCEPTRON_HPP
