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

#include "nuclearphysicsperceptron.hpp"

namespace Neural_Network {

NuclearPhysicsNeuralNet::NuclearPhysicsNeuralNet(const quint32 &_inputs) : Perceptron(_inputs) {
}

NuclearPhysicsNeuralNet::NuclearPhysicsNeuralNet(const NuclearPhysicsNeuralNet &_neu) : Perceptron (_neu) {
	functions = _neu.functions;
	name = _neu.name;
}
NuclearPhysicsNeuralNet& NuclearPhysicsNeuralNet::operator= (const NuclearPhysicsNeuralNet& _nucperc) {
	Perceptron::operator =(_nucperc);
	functions = _nucperc.functions;
	name = _nucperc.name;
	return *this;
}


NuclearPhysicsNeuralNet::~NuclearPhysicsNeuralNet () {

}

void NuclearPhysicsNeuralNet::push_layer(quint32 _neurons) {
	functions.resize(_neurons, AvailableFunctions::TotalAvailable);
	Perceptron::push_layer(_neurons);
}

void NuclearPhysicsNeuralNet::pop_layer() {
	Perceptron::pop_layer();
	functions.resize(outputs(), AvailableFunctions::TotalAvailable);
}

void NuclearPhysicsNeuralNet::clear() {
	Perceptron::clear();
	functions.resize(0);
}

void NuclearPhysicsNeuralNet::save(std::ostream &saveStream) const {
	Perceptron::save(saveStream);
	saveStream.write((char*)functions.data(), 4*functions.size());
	quint32 t = name.toUtf8().size();
	saveStream.write((char*)&t, 4);
	saveStream.write(name.toUtf8().data(), name.toUtf8().size());
}

void NuclearPhysicsNeuralNet::load(std::istream &loadStream) {
	Perceptron::load(loadStream);
	functions.resize(outputs());
	loadStream.read((char*)functions.data(), 4*functions.size());
	quint32 t;
	loadStream.read((char*)&t, 4);
	QByteArray ba (t, 0);
	loadStream.read(ba.data(), t);
	name = QString(ba);
	if (loadStream.eof() || loadStream.fail()) throw std::runtime_error ("Invalid file\n");
}

}
