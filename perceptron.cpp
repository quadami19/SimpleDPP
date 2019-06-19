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

#include "perceptron.hpp"

namespace Neural_Network {

Perceptron::Perceptron(const Perceptron &_percep) {
	input = _percep.input;
	neuronsVec = _percep.neuronsVec;
	for (quint32 i = 0; i < neuronsVec.size(); ++i) for (quint32 n = 0; n < neuronsVec[i].size(); ++n)
		if (i) for (quint32 m = 0; m < neuronsVec[i-1].size(); ++m) neuronsVec[i][n].input(neuronsVec[i-1][m].get_output_ptr(), m);
		else for (quint32 m = 0; m < input.size(); ++m) neuronsVec[i][n].input(&(input[m]), m);
}

Perceptron& Perceptron::operator= (const Perceptron& _percep) {
	neuronsVec = _percep.neuronsVec;
	input = _percep.input;
	for (quint32 i = 0; i < neuronsVec.size(); ++i) for (quint32 n = 0; n < neuronsVec[i].size(); ++n)
		if (i) for (quint32 m = 0; m < neuronsVec[i-1].size(); ++m) neuronsVec[i][n].input(neuronsVec[i-1][m].get_output_ptr(), m);
		else for (quint32 m = 0; m < input.size(); ++m) neuronsVec[i][n].input(&(input[m]), m);
	return *this;
}

void Perceptron::inputs(quint32 _inputs) {
	assert (_inputs);
	input.resize(_inputs);
	if (neuronsVec.size()) for (quint32 i = 0; i < neuronsVec[0].size(); ++i) for (quint32 n = 0; n < _inputs; ++n) {
		neuronsVec[0][i].input(&(input[n]), n);
	}
}

void Perceptron::set_weight(const std::vector<float> &_weights, quint32 _layer, quint32 _neuron_index) {
	assert(_layer<neuronsVec.size()&&_neuron_index<neuronsVec[_layer].size());
	neuronsVec[_layer][_neuron_index].set_weight (_weights);
}

std::vector<float> Perceptron::get_error(const std::vector<float>& _inputVec, const std::vector<float>& _outputVec) {
	assert (_outputVec.size() == neuronsVec[neuronsVec.size()-1].size());
	work (_inputVec);
	std::vector<float> tmpVec (_outputVec.size());
	for (quint32 i = 0; i < tmpVec.size(); ++i) {
		tmpVec[i] = _outputVec[i] - neuronsVec[neuronsVec.size()-1][i];
	}
	return tmpVec;
}

quint32 Perceptron::total_neurons() const {
	quint32 t (0);
	for (quint32 l = 0; l < neuronsVec.size(); ++l) t += neuronsVec[l].size();
	return t;
}

quint32 Perceptron::total_weights() const {
	quint32 t (0);
	for (quint32 l = 0; l < neuronsVec.size(); ++l) for (quint32 n = 0; n < neuronsVec[l].size(); ++n)
		t += neuronsVec[l][n].get_weights().size();
	return t;
}

void Perceptron::push_layer(quint32 _neurons) {
	assert (_neurons);
	std::vector<neuron_base> neuronsLayer;
	for (quint32 i = 0; i < _neurons; ++i) {
		neuron_base tNeu;
		if (neuronsVec.size()) {
			tNeu.inputs(neuronsVec[neuronsVec.size()-1].size());
			for (quint32 n = 0; n < neuronsVec[neuronsVec.size()-1].size(); ++n)
				tNeu.input(neuronsVec[neuronsVec.size()-1][n].get_output_ptr(), n);
		} else {
			tNeu.inputs(input.size());
			for (quint32 n = 0; n < input.size(); ++n)
				tNeu.input(&(input[n]), n);
		}
		neuronsLayer.push_back(tNeu);
	}
	neuronsVec.push_back(neuronsLayer);
}

void Perceptron::work(const std::vector<float>& _inputVec) {
	assert (_inputVec.size() == input.size());
	input = _inputVec;
	for (quint32 i = 0; i < neuronsVec.size(); ++i) for (quint32 n = 0; n < neuronsVec[i].size(); ++n)
		neuronsVec[i][n].calculate();
}

void Perceptron::work(std::vector<float>::const_iterator begin, std::vector<float>::const_iterator end) {
	assert ((std::vector<float>::size_type)(end - begin) == input.size());
	input = std::vector<float> (begin, end);
	for (quint32 i = 0; i < neuronsVec.size(); ++i) for (quint32 n = 0; n < neuronsVec[i].size(); ++n)
		neuronsVec[i][n].calculate();
}

void Perceptron::save(std::ostream &saveStream) const {
	saveStream.write("NPRC", 4);
	float tmp;
	quint32 itmp = 12 + 4 * layers() + 4 * total_weights() + 8 * total_neurons(); //inputs + layers + func + size
	if (is_empty()) {
		itmp = 0;
		saveStream.write((char*)&itmp, 4);
		return;
	}
	saveStream.write((char*)&itmp, 4);
	saveStream.write((char*)&(itmp = inputs()), 4);
	saveStream.write((char*)&(itmp = layers()), 4);
	for (quint32 l = 0; l < layers(); ++l) saveStream.write((char*)&(itmp = neurons(l)), 4);
	itmp = 4 * total_weights() + 8 * total_neurons();
	saveStream.write((char*)&itmp, 4);
	for (quint32 l = 0; l < layers(); ++l) for (quint32 n = 0; n < neurons(l); ++n) {
		saveStream.write((char*)(get_weights(l, n).data()), 4 * weights(l, n));
		saveStream.write((char*)(&(tmp = get_const (l, n))), 4);
		saveStream.write((char*)(&(tmp = get_func_id(l, n))), 4);
	}
}

void Perceptron::load(std::istream &loadStream) {
	char header[8];
	loadStream.read(header, 8);
	std::runtime_error except ("Invalid file\n");
	if (loadStream.fail()) throw except;
	quint32 size = *(quint32*)(header+4), inputs, layers, funcID, infoSize, weiSize;
	clear();
	if (!size) return;
	float tmp;
	if (std::strncmp("NPRC", header, 4)) throw except;
	char* data = new char [size];
	loadStream.read(data, size);
	if (loadStream.eof()) throw except;
	inputs = *(quint32*)(data);
	layers = *(quint32*)(data + 4);
	if (layers == 0 || inputs == 0) throw except;
	std::vector<quint32> neuronsNum (layers);
	infoSize = 12 + 4 * layers;
	for (quint32 i = 0; i < layers; ++i)
		neuronsNum[i] = *(quint32*)(data + 8 + i * 4);
	weiSize = inputs * neuronsNum[0];
	weiSize += 2*neuronsNum[0];
	for (quint32 i = 1; i < layers; ++i) {
		weiSize += neuronsNum[i] * neuronsNum[i-1];
		weiSize += 2*neuronsNum[i];
	}
	quint32 itmp;
	if ((weiSize != (size - infoSize)/4 ) || (weiSize != (itmp = *(quint32*)(data + infoSize - 4)) / 4)) throw except;
	Perceptron::inputs(inputs);
	for (quint32 i = 0; i < layers; ++i)
		push_layer(neuronsNum[i]);
	quint32 i = 0;
	for (quint32 l = 0; l < layers; ++l) for (quint32 n = 0; n < neuronsNum[l]; ++n) {
		if (l) for (quint32 w = 0; w < neuronsNum[l-1]; ++w)
			set_weight((tmp = *(float*)(data + infoSize + 4 * i++)), l, n, w);
		else for (quint32 w = 0; w < inputs; ++w)
			set_weight((tmp = *(float*)(data + infoSize + 4 * i++)), l, n, w);
		set_const((tmp = *(float*)(data + infoSize + 4 * i++)), l, n);
		funcID = *(quint32*)(data + infoSize + 4 * i++);
		neuronsVec[l][n].set_activation_function(funcID);
	}
	delete data;
}

}
