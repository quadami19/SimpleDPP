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

#ifndef NUCLEARPHYSICSNEURALNET_HPP
#define NUCLEARPHYSICSNEURALNET_HPP

#include "perceptron.hpp"
#include "fft.hpp"
#include <QtGlobal>
#include <QString>
#include <ios>
#include <fstream>
#include <random>
#include <ctime>
#include <vector>
#include <sstream>

namespace AvailableFunctions {
	enum {
		Amplitude = 0,
		Time,
		Discriminating,
		TotalAvailable
	};
	constexpr static const char* FunctionName[TotalAvailable] = {
		"Amplitude",
		"Time",
		"Discriminating"
	};
}

namespace Neural_Network {

class NuclearPhysicsNeuralNet : public Perceptron {

		std::vector<quint32> functions;
		QString name;

	public:
		explicit NuclearPhysicsNeuralNet (const quint32& _inputs = 32);
		NuclearPhysicsNeuralNet (const NuclearPhysicsNeuralNet& _neu);
		NuclearPhysicsNeuralNet& operator= (const NuclearPhysicsNeuralNet& _nucperc);
		virtual ~NuclearPhysicsNeuralNet();
		void set_function (quint32 func, quint32 num) { assert(num < functions.size()); functions[num] = func; }
		quint32 get_function (quint32 num) const { assert(num < functions.size()); return functions[num]; }
		void set_name (QString _name) { name = _name; }
		QString get_name () { return name; }
		virtual void push_layer(quint32 _neurons);
		virtual void pop_layer();
		virtual void clear();
		double teach(TeachingClass* _teachClass, std::vector<float> teachPulse, float _noizeAmplitude, quint32 _numOfMassive);
		void save (std::ostream& saveStream) const;
		void load (std::istream& loadStream);
};

}

#endif // NUCLEARPHYSICSNEURALNET_HPP
