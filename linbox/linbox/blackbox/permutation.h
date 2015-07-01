/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* linbox/blackbox/permutation.h
 * Copyright (C) 2001 Bradford Hovinen
 *
 * Written by Bradford Hovinen <hovinen@cis.udel.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __PERMUTATION_H
#define __PERMUTATION_H

#include <vector>


#include "linbox/util/debug.h"
#include "linbox-config.h"
#include <linbox/blackbox/blackbox-interface.h>

#ifdef __LINBOX_XMLENABLED

#include "linbox/util/xml/linbox-reader.h"
#include "linbox/util/xml/linbox-writer.h"

#include <iostream>
#include <string>

#endif


// Namespace in which all LinBox library code resides
namespace LinBox
{

	/** \brief size is n.

\ingroup blackbox
	 * @param Storage \ref{LinBox} dense or sparse vector of field elements
	 */
    template<class _Field, class Storage = std::vector< long > >
	class Permutation : public  BlackboxInterface 
	{
            const _Field& _F;
	    public:
            typedef _Field         Field;
            typedef typename Field::Element Element;
		/** Constructor from a vector of indices
		 * This constructor creates a permutation matrix based on a vector of indices
		 * @param indices Vector of indices representing the permutation
		 */
            Permutation (Storage & indices, const Field& F = Field()) : _F(F), _indices (indices)
		{}

		/** Constructor from a dimension
		 * This constructor creates an n x n permutation matrix, initialized to be the identity
		 * @param n The dimension of hte matrix to create
		 */
		Permutation (int n, const Field& F = Field()) : _F(F)
		{
			typename Storage::value_type i;

			_indices.resize (n);

			for (i = 0; i < n; i++)
				_indices[i] = i;
		}

		/* Copy constructor.
		 * Creates new black box objects in dynamic memory.
		 * @param M constant reference to compose black box matrix
		 */
		Permutation (const Permutation &M)
			: _F(M._F),_indices (M._indices)
		{}

#ifdef __LINBOX_XMLENABLED
		Permutation(LinBox::Reader &R)
		{
			if(!R.expectTagName("MatrixOver")) return;
                        if(!R.expectChildTag()) return;
                        R.traverseChild();

                        if(!R.expectTagName("permutation") || !R.expectTagNumVector(_indices)) return;

			R.upToParent();
                        return;
		}
#endif


		// Destructor
		~Permutation (void) {}

		/* Application of BlackBox permutation matrix.
		 * y= P*x.
		 * Requires one vector conforming to the \ref{LinBox}
		 * vector {@link Archetypes archetype}.
		 * Required by abstract base class.
		 * @return reference to vector y containing output.
		 * @param  x constant reference to vector to contain input
		 */
		/// #y \leftarrow Px#.
		template<class OutVector, class InVector>
		inline OutVector &apply (OutVector &y, const InVector &x) const
		{
			size_t i;

			linbox_check (x.size () == _indices.size ());

			// resizing y is now forbidden - bds //y.resize (x.size ());
			linbox_check (y.size () == _indices.size ());

			for (i = 0; i < x.size(); ++i)
				_F.assign(y[_indices[i]], x[i]);

			return y;
		}

		/* Application of BlackBox permutation matrix transpose.
		 * y= transpose(P)*x, equivalently y= P^-1*x
		 * Requires one vector conforming to the \ref{LinBox}
		 * vector {@link Archetypes archetype}.
		 * Required by abstract base class.
		 * @return reference to vector y containing output.
		 * @param  x constant reference to vector to contain input
		 */
		/// #y^T \leftarrow x^T P#.
		template<class OutVector, class InVector>
		inline OutVector &applyTranspose (OutVector &y, const InVector &x) const
		{
			size_t i;

			linbox_check (x.size () == _indices.size ());

			// resizing y is now forbidden - bds //y.resize (x.size ());
			linbox_check (y.size () == _indices.size ());

			for (i = 0; i < _indices.size (); ++i)
				_F.assign(y[i], x[_indices[i]]);

			return y;
		}


            template<typename _Tp1>
            struct rebind
            { typedef Permutation<_Tp1, Storage> other; };



		/* Retreive row dimensions of BlackBox matrix.
		 * This may be needed for applying preconditioners.
		 * Required by abstract base class.
		 * @return integer number of rows of black box matrix.
		 */
		size_t rowdim (void) const
		{
			return _indices.size ();
		}
    
		/* Retreive column dimensions of BlackBox matrix.
		 * Required by abstract base class.
		 * @return integer number of columns of black box matrix.
		 */
		size_t coldim (void) const 
		{
			return _indices.size ();
		}

		/** Add a transposition to the matrix
		 */
		void permute (size_t row1, size_t row2) 
		{
			linbox_check (row1 >= 0 && row1 < _indices.size ());
			linbox_check (row2 >= 0 && row2 < _indices.size ());

			_swap (_indices[row1], _indices[row2]);
		}

            const Field& field() { return _F; }

#ifdef __LINBOX_XMLENABLED

		std::ostream &write(std::ostream &out) const
		{
			LinBox::Writer W;
			if( toTag(W) ) 
				W.write(out);
		
			return out;
		}

		bool toTag(LinBox::Writer &W) const
		{
			std::string s;
			W.setTagName("MatrixOver");
			W.setAttribute("rows", LinBox::Writer::numToString(s, _indices.size()));
			W.setAttribute("cols", LinBox::Writer::numToString(s, _indices.size()));
			W.setAttribute("implDetail", "permutation");
			
			W.addTagChild();
			W.setTagName("permutation");
			W.addNumericalList(_indices);
			W.upToParent();

			return true;
		}
#endif


	    private:

		void _swap(typename Storage::value_type &x, typename Storage::value_type &y) const
		{
			typename Storage::value_type temp = x;
			x = y;
			y = temp;
		}

		// Vector of indices
		Storage _indices;

	}; // template <Vector> class Permutation

} // namespace LinBox

#endif // __PERMUTATION_H
