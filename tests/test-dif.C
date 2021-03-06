/* tests/test-dif.C
 * Copyright (C) LinBox
 * +- modified from tests/test-sum.C
 * transform by bds
 *
 * It will be desirable to keep test-sum and test-dif in sync.
 * ------------------------------------
 *
 *
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
 *.
 */


/*! @file  tests/test-dif.C
 * @ingroup tests
 * @brief  no doc
 * @test NO DOC
 */



#include "linbox/linbox-config.h"

#include <iostream>
#include <fstream>


#include "linbox/util/commentator.h"
#include "linbox/vector/stream.h"
#include "linbox/field/archetype.h"
#include "linbox/ring/modular.h"
#include "linbox/vector/vector-domain.h"
#include "linbox/blackbox/diagonal.h"
#include "linbox/blackbox/dif.h"

#include "test-common.h"
#include "test-generic.h"

using namespace LinBox;

/* Test 1: Application of zero matrix onto random vectors
 *
 * Construct a random diagonal matrix, then construct
 * the difference of the matrix and itself. Apply to random vectors and check that
 * the result is zero.
 *
 * F - Field over which to perform computations
 * n - Dimension to which to make matrix
 *
 * Return true on success and false on failure
 */

template <class Field, class Vector>
static bool testZeroApply (Field &F, VectorStream<Vector> &stream1, VectorStream<Vector> &stream2)
{
	typedef Diagonal <Field> Blackbox;

	commentator().start ("Testing zero apply", "testZeroApply", stream1.m ());

	bool ret = true;
	// bool iter_passed = true;

	Vector d1(F), d2(F), v(F), w(F), zero(F);
	VectorDomain<Field> VD (F);

	VectorWrapper::ensureDim (zero, stream1.dim ());
	VectorWrapper::ensureDim (d1, stream1.dim ());
	VectorWrapper::ensureDim (d2, stream1.dim ());
	VectorWrapper::ensureDim (v, stream1.dim ());
	VectorWrapper::ensureDim (w, stream2.dim ());

	while (stream1) {
		commentator().startIteration ((unsigned)stream1.j ());
		bool iter_passed = true;

		stream1.next (d1);
		//VD.mul (d2, d1, neg_one);

		Blackbox D1 (d1); // , D2 (d2);
		Dif <Blackbox, Blackbox> A (D1, D1);

		ostream &report = commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION);
		report << "Diagonal matrix:  ";
		VD.write (report, d1);
		report << endl;

		//report << "Negative diagonal matrix:  ";
		//VD.write (report, d2);
		//report << endl;

		stream2.reset ();

		while (stream2) {
			stream2.next (w);

			report << "Input vector:  ";
			VD.write (report, w);
			report << endl;

			A.apply (v, w);

			report << "Output vector:  ";
			VD.write (report, v);
			report << endl;

			if (!VD.isZero (v))
				ret = iter_passed = false;
		}

		if (!iter_passed)
			commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
				<< "ERROR: Vector is not zero" << endl;

		commentator().stop ("done");
		commentator().progress ();
	}

	commentator().stop (MSG_STATUS (ret), (const char *) 0, "testZeroApply");

	return ret;
}

#if 0

/* Test 2: Random transpose
 *
 * Compute a random diagonal matrix and use the transpose test in test-generic.h
 * to check consistency of transpose apply.
 *
 * F - Field over which to perform computations
 * n - Dimension to which to make matrix
 * iterations - Number of random vectors to which to apply matrix
 *
 * Return true on success and false on failure
 */

template <class Field>
static bool testRandomTranspose (Field &F, size_t n, int iterations)
{
	typedef vector <typename Field::Element> Vector;
	typedef Diagonal <Field> Blackbox;

	commentator().start ("Testing random transpose", "testRandomTranspose", iterations);

	Vector d(n);
	typename Field::RandIter r (F);

	for (int i = 0; i < n; i++)
		r.random (d[i]);

	Blackbox D (F, d);

	ostream &report = commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION);

	report << "Diagonal vector: ";
	printVector<Field> (F, report, d);

	bool ret = testTranspose<Field> (F, D, iterations);

	commentator().stop (MSG_STATUS (ret), (const char *) 0, "testRandomTranspose");

	return ret;
}

#endif

int main (int argc, char **argv)
{
	bool pass = true;

	static size_t n = 10;
	static integer q = 101;
	static int iterations1 = 2; // was 100
	static int iterations2 = 1;

	static Argument args[] = {
		{ 'n', "-n N", "Set dimension of test matrices to NxN.", TYPE_INT,     &n },
		{ 'q', "-q Q", "Operate over the \"field\" GF(Q) [1].", TYPE_INTEGER, &q },
		{ 'i', "-i I", "Perform each test for I iterations.",   TYPE_INT,     &iterations1 },
		{ 'j', "-j J", "Apply test matrix to J vectors.",         TYPE_INT,     &iterations2 },
		END_OF_ARGUMENTS
	};

	typedef Givaro::Modular<int32_t> Field;

	parseArguments (argc, argv, args);
	Field F (101);

	commentator().start("Matrix dif black box test suite", "dif");

	// Make sure some more detailed messages get printed
	commentator().getMessageClass (INTERNAL_DESCRIPTION).setMaxDepth (2);

    Field::RandIter gen(F);
    
	RandomDenseStream<Field> stream1 (F, gen, n, (unsigned int)iterations1), stream2 (F, gen, n, (unsigned int)iterations2);

	if (!testZeroApply (F, stream1, stream2)) pass = false;

	commentator().stop("Matrix dif black box test suite");
	return pass ? 0 : -1;
}

// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
