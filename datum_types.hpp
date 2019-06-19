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

#ifndef DATUM_TYPES_HPP
#define DATUM_TYPES_HPP

enum DatumType {
	Datum8UBits = 0,
	Datum16UBits,
	Datum32UBits,
	Datum8SBits,
	Datum16SBits,
	Datum32SBits,
	DatumFloat,
	DatumDouble,
	MaxDatumType
};

enum DatumAlign {
	LittleEndian = 0,
	BigEndian,
	MaxEndian
};

#endif // DATUM_TYPES_HPP

