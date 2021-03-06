/*
 * examples/blackbox/example.C
 *
 * Copyright (C) 2003 William J. Turner
 * ========LICENCE========
 * This file is part of the library LinBox.
 *
 * LinBox is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 */

/** @name examples/blackbox/ex-mat0.C
 * @author William J. Turner for the LinBox group
 *
 * @memo usage: ex-mat0 in-file out-file
 *
 * @doc
 * Run tests on Wiedemann algorithm for solving nonhomogeneous linear
 * equations
 *
 * FIXME What does it do?  I think this may be a remnant, has evolved
 * into one of the other examples.  delete it?
 */
//@{

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <utility>

#include "linbox/field/modular.h"
//#include "linbox/blackbox/sparse0.h"
#include "linbox/blackbox/sparse1.h"

using namespace LinBox;
using namespace std;

int main(int argc, char* argv[])
{

	if (argc != 3)
	{	cerr << "usage: " << argv[0] << " in_file out_file" << endl;
		return -1;
	}
        char* in_file = argv[1];
        char* out_file = argv[2];

        typedef Givaro::Modular<uint32_t>  Field;
	typedef Field::Element Element;
	typedef Field::RandIter RandIter;
	typedef std::list< pair<size_t, Element> > Row;
	typedef std::vector<Element> Vector;

        Field K(7);

        ofstream out_stream(out_file);
        ifstream in_stream(in_file);

//	SparseMatrix<Field, Row, Vector>  A(K,4,4);
//      A.read(in_stream);
//	A.write(out_stream);

        SparseBlackBoxDom< Field > A(K) ;

        A.read(in_stream);
        A.write(out_stream);

}
//@}

// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,:0,t0,+0,=s
// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:

