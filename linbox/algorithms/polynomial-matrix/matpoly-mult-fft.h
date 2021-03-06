// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2013  Pascal Giorgi
 *                     Romain Lebreton
 *
 * Written by Pascal Giorgi   <pascal.giorgi@lirmm.fr>
 *            Romain Lebreton <lebreton@lirmm.fr>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 */

#ifndef __LINBOX_matpoly_mult_ftt_H
#define __LINBOX_matpoly_mult_ftt_H

#include "linbox/util/error.h"
#include "linbox/util/debug.h"
#include "linbox/util/timer.h"
#include <linbox/randiter/random-fftprime.h>
#include <linbox/randiter/random-prime.h>
#include "linbox/integer.h"
#include <givaro/zring.h>
#include "linbox/ring/modular.h"
#include "givaro/givtimer.h"

#ifdef FFT_PROFILER
#include <iostream>
#ifndef FFT_PROF_LEVEL
int  FFT_PROF_LEVEL=1;
#endif
Givaro::Timer mychrono[3];
#define FFT_PROF_MSG_SIZE 35
#define FFT_PROFILE_START(lvl)  mychrono[lvl].clear();mychrono[lvl].start();

#define FFT_PROFILING(lvl,msg)						\
  if (lvl>=FFT_PROF_LEVEL) {						\
    mychrono[lvl].stop();std::cout<<"FFT("<<lvl<<"):";			\
    std::cout.width(FFT_PROF_MSG_SIZE);std::cout<<std::left<<msg<<" : "; \
    std::cout.precision(6);std::cout<<mychrono[lvl]<<std::endl;		\
    mychrono[lvl].clear();mychrono[lvl].start();			\
  }
  
#ifdef HAVE_OPENMP								
#define FFT_PROFILE_GET(lvl,x)						\
  // mychrono.stop();(x)+=mychrono.realtime();mychrono.clear();mychrono.start();
#else
#define FFT_PROFILE_GET(lvl,x)						\
  mychrono[lvl].stop();(x)+=mychrono[lvl].usertime();mychrono[lvl].clear();mychrono[lvl].start();
#endif
#define FFT_PROFILE(lvl,msg,x)						\
  if ((lvl)>=FFT_PROF_LEVEL) {						\
			      std::cout<<"FFT: ";			\
			      std::cout.width(FFT_PROF_MSG_SIZE);std::cout<<std::left<<msg<<" : "; \
			      std::cout.precision(6);std::cout<<x<<" s"<<std::endl; \
  }
#else
#define FFT_PROFILE_START(lvl)
#define FFT_PROFILING(lvl,msg)
#define FFT_PROFILE_GET(lv,x)
#define FFT_PROFILE(lvl,msg,x)
#endif // FFT_PROFILER


#ifndef FFT_DEG_THRESHOLD   
#define FFT_DEG_THRESHOLD   4
#endif

namespace LinBox
{
// generic handler for multiplication using FFT
  template <class Field>
    class PolynomialMatrixFFTMulDomain {
  public:
    inline const Field & field() const;

    PolynomialMatrixFFTMulDomain (const Field& F);

    template<typename Matrix1, typename Matrix2, typename Matrix3>
      void mul (Matrix1 &c, const Matrix2 &a, const Matrix3 &b);

    template<typename Matrix1, typename Matrix2, typename Matrix3>
      void midproduct (Matrix1 &c, const Matrix2 &a, const Matrix3 &b, bool smallLeft=true, size_t n0=0,size_t n1=0);
  };
		
	
  //class PolynomialMatrixFFTPrimeMulDomain ;                         // Mul in Zp[x] with p <2^32, (fflas, fourier)
		
  // template <class T>
  // class PolynomialMatrixFFTMulDomain<Givaro::Modular<T> > ;        // Mul in Zp[x] with p^2 storable in type T

  // template<>
  // class PolynomialMatrixFFTMulDomain<Givaro::ZRing<integer> >;  // Mul in Z[x]

  // template <>
  // class PolynomialMatrixFFTMulDomain<Givaro::Modular<integer> > ;           // Mul in Zp[x] with p multiprecision

  // get the maximum prime for fft with modular<double> (matrix dim =k, nbr point = pts)
  uint64_t maxFFTPrimeValue(uint64_t k, uint64_t pts) {
    uint64_t prime_max=std::sqrt( (1ULL<<53) /k)+1;
    size_t c=1;
    const int fct=24;
    while (c<k && prime_max < (1UL<<26) && prime_max< pts*fct){
      prime_max=std::sqrt( (1ULL<<53) /(k/c))+1;
      c<<=1;
    }

    //std::cout<<"maxFFTPrime: pts -> "<<pts<<std::endl;
    //std::cout<<"maxFFTPrime: replacing "<<k<<" -> "<<k/c<<std::endl;
	  
    if (c>=k){
      std::cout<<"MatPoly FFT (maxPrimeValue): impossible to find enough FFT Prime\n";
      std::terminate();
    }
	  
    return std::min(prime_max, uint64_t(Givaro::Modular<double>::maxCardinality()));
  }

  void getFFTPrime(uint64_t prime_max, size_t lpts, integer bound, std::vector<integer> &bas, size_t k, size_t d){
	  
    RandomFFTPrime RdFFT(prime_max);
    size_t nbp=0;
	  
    if (!RdFFT.generatePrimes(lpts,bound,bas)){ // not enough FFT prime found 
      integer MM=1;
      for(std::vector<integer>::size_type i=0;i<bas.size();i++){
	MM*=bas[i];
	//std::cout<<bas[i]<<std::endl;
      }
	    
      // compute max bitsize for prime allowing three prime fft
      integer prime_max_tp=MM/uint64_t(d*k);
      while (k>1 && prime_max_tp<100) {k/=2;prime_max_tp*=2;}
      if (k<=1) {std::cout<<"getFFTPrime error: impossible to have enough primes satisfying constraints: FFLAS prime (<2^26) and FFT (2^"<<lpts<<")\n";}
	
      RandomPrimeIter Rd(std::min(prime_max_tp.bitsize()/2,integer(prime_max).bitsize())-1);
#ifdef VERBOSE_FFT
      std::cout<<"MM="<<MM<<std::endl;
      std::cout<<"normal primemax: "<<prime_max_tp<<" "<<prime_max<<std::endl;
      std::cout<<"normal prime bitmax: "<<std::min(prime_max_tp.bitsize()/2,integer(prime_max).bitsize()-1)<<std::endl;
#endif
      integer tmp;
      do {
	do {Rd.random(tmp);}
	while (MM%tmp==0 || tmp>prime_max);
	bas.push_back(tmp);
	nbp++;
	MM*=tmp;
      } while (MM<bound);	
    }
#ifdef VERBOSE_FFT      
    std::cout<<"MatPoly Multiprecision FFT : using "<<bas.size()-nbp<<" FFT primes and "<<nbp<<" normal primes "<<std::endl;
#endif
    for(auto i: bas)
      if (i>prime_max) std::cout<<"ERROR\n";
  }

	
} // end of namespace LinBox

#include "linbox/algorithms/polynomial-matrix/matpoly-mult-fft-wordsize-fast.inl"
#include "linbox/algorithms/polynomial-matrix/matpoly-mult-fft-wordsize-three-primes.inl"
#include "linbox/algorithms/polynomial-matrix/matpoly-mult-fft-multiprecision.inl"
#include "linbox/algorithms/polynomial-matrix/matpoly-mult-fft-recint.inl"
#include "linbox/algorithms/polynomial-matrix/matpoly-mult-fft-wordsize.inl"

#endif // __LINBOX_matpoly_mult_ftt_H
