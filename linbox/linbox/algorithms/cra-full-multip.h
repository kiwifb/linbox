// ======================================================================= //
// Time-stamp: <09 Mar 07 18:54:23 Jean-Guillaume.Dumas@imag.fr> 
// ======================================================================= //
#ifndef __LINBOX_CRA_FULL_MULTIP_H
#define __LINBOX_CRA_FULL_MULTIP_H

#include "linbox/util/timer.h"
#include <stdlib.h>
#include "linbox/integer.h"
#include "linbox/solutions/methods.h"
#include <vector>
#include <utility>

#include "linbox/algorithms/lazy-product.h"

namespace LinBox {
    
    template<class Domain_Type>
    struct FullMultipCRA {
        typedef Domain_Type			Domain;
        typedef typename Domain::Element DomainElement;
        typedef FullMultipCRA<Domain> 		Self_t;

    protected:
        std::vector< double >           	RadixSizes_;
        std::vector< LazyProduct >      	RadixPrimeProd_;
        std::vector< std::vector<Integer> >    	RadixResidues_;
        std::vector< bool >             	RadixOccupancy_;
        double                			LOGARITHMIC_UPPER_BOUND;
       

    public:
        FullMultipCRA(const double b) : LOGARITHMIC_UPPER_BOUND(b) {}
        
        template<template<class T> class Vect>
        void initialize (const Domain& D, const Vect<DomainElement>& e) {
            RadixSizes_.resize(1);
            RadixPrimeProd_.resize(1);
            RadixResidues_.resize(1);
            RadixOccupancy_.resize(1); RadixOccupancy_.front() = false;
            progress(D, e);
        }
        
        template<template<class T> class Vect>
        void progress (const Domain& D, const Vect<DomainElement>& e) {         
            std::vector< double >::iterator  _dsz_it = RadixSizes_.begin();
            std::vector< LazyProduct >::iterator _mod_it = RadixPrimeProd_.begin();
            std::vector< std::vector<Integer> >::iterator _tab_it = RadixResidues_.begin();
            std::vector< bool >::iterator    _occ_it = RadixOccupancy_.begin();
            std::vector<Integer> ri(e.size()); LazyProduct mi; double di;
            if (*_occ_it) {
                typename Vect<DomainElement>::const_iterator  e_it = e.begin();
                std::vector<Integer>::iterator       ri_it = ri.begin();
                std::vector<Integer>::const_iterator t0_it = _tab_it->begin();
                for( ; e_it != e.end(); ++e_it, ++ri_it, ++ t0_it)
                    fieldreconstruct(*ri_it, D, *e_it, *t0_it, (*_mod_it).operator()() );
                Integer tmp; D.characteristic(tmp);
                di = *_dsz_it + log(double(tmp)); 
                mi.mulin(tmp);
                mi.mulin(*_mod_it);
                *_occ_it = false;
            } else {
                Integer tmp; D.characteristic(tmp);
                _mod_it->initialize(tmp);
                *_dsz_it = log(double(tmp));
                typename Vect<DomainElement>::const_iterator e_it = e.begin();
                _tab_it->resize(e.size());
                std::vector<Integer>::iterator t0_it= _tab_it->begin();
                for( ; e_it != e.end(); ++e_it, ++ t0_it)
                    D.convert(*t0_it, *e_it);
                *_occ_it = true;
                return;
            }
         
            for(++_dsz_it, ++_mod_it, ++_tab_it, ++_occ_it ; _occ_it != RadixOccupancy_.end() ; ++_dsz_it, ++_mod_it, ++_tab_it, ++_occ_it) {
                if (*_occ_it) {
                    std::vector<Integer>::iterator      ri_it = ri.begin();
                    std::vector<Integer>::const_iterator t_it= _tab_it->begin();
                    for( ; ri_it != ri.end(); ++ri_it, ++ t_it)
                        smallbigreconstruct(*ri_it, mi(), *t_it, _mod_it->operator()()); 
                    mi.mulin(*_mod_it);
                    di += *_dsz_it;
                    *_occ_it = false;
                } else {
                    *_dsz_it = di;
                    *_mod_it = mi;
                    *_tab_it = ri;
                    *_occ_it = true;
                    return;
                }
            }
                // Everything was occupied
            RadixSizes_.push_back( di );
            RadixResidues_.push_back( ri );
            RadixPrimeProd_.push_back( mi );
            RadixOccupancy_.push_back ( true );
        }

         
       
        template<template<class T> class Vect>
        Vect<Integer>& result (Vect<Integer> &d){
            d.resize( (RadixResidues_.front()).size() );
            std::vector< LazyProduct >::iterator          _mod_it = RadixPrimeProd_.begin();
            std::vector< std::vector< Integer > >::iterator _tab_it = RadixResidues_.begin();
            std::vector< bool >::iterator                _occ_it = RadixOccupancy_.begin();
            LazyProduct Product;
            for( ; _occ_it != RadixOccupancy_.end() ; ++_mod_it, ++_tab_it, ++_occ_it) {
                if (*_occ_it) {
                    Product = *_mod_it;
                    std::vector<Integer>::iterator t0_it = d.begin();
                    std::vector<Integer>::iterator t_it = _tab_it->begin();
                    if (++_occ_it == RadixOccupancy_.end()) {
                        for( ; t0_it != d.end(); ++t0_it, ++t_it)
                            normalize(*t0_it = *t_it, *t_it, _mod_it->operator()());
                        RadixPrimeProd_.resize(1);
                        return d;
                    } else {
                        for( ; t0_it != d.end(); ++t0_it, ++t_it)
                            *t0_it  = *t_it;
                        ++_mod_it; ++_tab_it; 
                        break;
                    }
                }
            }
            for( ; _occ_it != RadixOccupancy_.end() ; ++_mod_it, ++_tab_it, ++_occ_it) {
                if (*_occ_it) {
                    std::vector<Integer>::iterator t0_it = d.begin();
                    std::vector<Integer>::const_iterator t_it = _tab_it->begin();
                    for( ; t0_it != d.end(); ++t0_it, ++t_it)
                        normalizesmallbigreconstruct(*t0_it, Product(), *t_it, _mod_it->operator()() );
                    Product.mulin(*_mod_it);
                }
            }
            RadixPrimeProd_.resize(1);
            RadixPrimeProd_.front() = Product;
            return d;
        }
      
        bool terminated() {
            double logm(0.0);
            std::vector< double >::iterator _dsz_it = RadixSizes_.begin();
            std::vector< bool >::iterator _occ_it = RadixOccupancy_.begin();
            for( ; _occ_it != RadixOccupancy_.end() ; ++_dsz_it, ++_occ_it) {
                if (*_occ_it) {
                    logm = *_dsz_it;
                    ++_dsz_it; ++_occ_it;
                    break;
                }
            }
            for( ; _occ_it != RadixOccupancy_.end() ; ++_dsz_it, ++_occ_it) {
                if (*_occ_it) {
                    logm += *_dsz_it;
                }
            }
            return logm > LOGARITHMIC_UPPER_BOUND;
        }

       

        bool noncoprime(const Integer& i) const {
            std::vector< LazyProduct >::const_iterator _mod_it = RadixPrimeProd_.begin();
            std::vector< bool >::const_iterator    _occ_it = RadixOccupancy_.begin();
            for( ; _occ_it != RadixOccupancy_.end() ; ++_mod_it, ++_occ_it)
                if ((*_occ_it) && (_mod_it->noncoprime(i))) return true;
            return false;
        }



      
    protected:
        
        Integer& smallbigreconstruct(Integer& u1, const Integer& m1, const Integer& u0, const Integer& m0) {
            Integer tmp=u1;
            inv(u1, m0, m1);      // res <-- m0^{-1} mod m1
            tmp -= u0;            // tmp <-- (u1-u0)
            u1 *= tmp;            // res <-- (u1-u0)( m0^{-1} mod m1 )   
            u1 %= m1;      //                    res <  m1 
            u1 *= m0;             // res <-- (u1-u0)( m0^{-1} mod m1 ) m0      and res <= (m0m1-m0)
            return u1 += u0;      // res <-- u0 + (u1-u0)( m0^{-1} mod m1 ) m0 and res <  m0m1
        }

        Integer& normalize(Integer& u1, Integer& tmp, const Integer& m1) {
            if (u1 < 0)
                tmp += m1;
            else
                tmp -= m1;
            return ((absCompare(u1,tmp) > 0)? u1 = tmp : u1 );
        }

        Integer& normalizesmallbigreconstruct(Integer& u1, const Integer& m1, const Integer& u0, const Integer& m0) {
            Integer tmp=u1;
            inv(u1, m0, m1);   // res <-- m0^{-1} mod m1
            tmp -= u0;      // tmp <-- (u1-u0)
            u1 *= tmp;      // res <-- (u1-u0)( m0^{-1} mod m1 )   
            u1 %= m1;
            normalize(u1, tmp=u1, m1);         // Normalization
            u1 *= m0;          // res <-- (u1-u0)( m0^{-1} mod m1 ) m0       and res <= (m0m1-m0)
            return u1 += u0;   // res <-- u0 + (u1-u0)( m0^{-1} mod m1 ) m0  and res <  m0m1
        }

        Integer& fieldreconstruct(Integer& res, const Domain& D1, const DomainElement& u1, const Integer& r0, const Integer& P0) {
            DomainElement u0, m0; D1.init(u0, r0);
            if (D1.areEqual(u1, u0))
                return res=r0;
            else
                return fieldreconstruct(res, D1, u1, u0, D1.init(m0, P0), r0, P0);
        }

        Integer& fieldreconstruct(Integer& res, const Domain& D1, const DomainElement& u1, DomainElement& u0, DomainElement& m0, const Integer& r0, const Integer& P0) {
                // u0 and m0 are modified
            D1.negin(u0);   	// u0 <-- -u0
            D1.addin(u0,u1);   	// u0 <-- u1-u0
            D1.invin(m0);   	// m0 <-- m0^{-1} mod m1
            D1.mulin(u0, m0);   // u0 <-- (u1-u0)( m0^{-1} mod m1 )
            D1.convert(res, u0);// res <-- (u1-u0)( m0^{-1} mod m1 )         and res <  m1
            res *= P0;      	// res <-- (u1-u0)( m0^{-1} mod m1 ) m0      and res <= (m0m1-m0)
            return res += r0;	// res <-- u0 + (u1-u0)( m0^{-1} mod m1 ) m0 and res <  m0m1
        }

       
    };
    
}


#endif