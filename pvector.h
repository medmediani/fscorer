/***************************************************************************
*    pvector.h
*    Part of fscorer	: a parallel phrase scoring tool 
*                         for extra large corpora
*    copyright            : (C) 2012 by Mohammed Mediani
*    email                : mohammed.mediani@kit.edu
****************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it  under the terms of the GNU Lesser General Public License version  *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 *   USA                                                                   *
 ***************************************************************************/
#include <algorithm>
#include <vector>

#include <stxxl/io>
#include <stxxl/mng>
#include <stxxl/ksort>
#include <stxxl/sort>
#include <stxxl/vector>

#include <omp.h>

using std::cout;

#ifndef _PVECTOR_H_
#define _PVECTOR_H_


#ifdef USE_STD_VECTORS
#define stxxl std
#endif

#ifndef MEM_BLOCK
#define MEM_BLOCK (512 * 1024 * 1024)
#endif

#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 256
#endif

namespace pvectors{
template<class BType>
void erase(vector<BType> *vec,typename vector<BType>::iterator pos)
{
	typename vector<BType>::reverse_iterator last=vec->rbegin();
	BType temp=*pos;
	*pos=*last;
	vec->resize(vec->size()-1);
}

template<class T>
struct less {
  bool operator()(T x, T y) { return x<y; }
};

template<class T>
bool def_comp(T& x,T& y){ return x<y; };

template<class BType>
class pvector
{
public:
    typedef BType  value_type;
//    const unsigned block_size_bytes = sizeof(BType) * 4096 * 12;
    typedef typename stxxl::VECTOR_GENERATOR<value_type, 2, 4, 
            1024 * 1024 * 8, stxxl::striping>::result stxxl_vec;
	typedef bool (*comparator)( BType&,BType&);
	typedef typename stxxl_vec::iterator elements_iterator;
        typedef typename stxxl_vec::const_iterator elements_const_iterator;

	typedef  std::vector<stxxl_vec > pvector_type;
	typedef typename pvector_type::iterator pvector_iterator;
	typedef pvector_iterator iterator;
	typedef  std::vector<std::pair<pvector_iterator,elements_iterator> > vecs_itrs_type;
	typedef typename vecs_itrs_type::iterator vecs_itrs_iterator;

	class merge_iterator
	{
		private:
			pvector<BType> * self;
			vecs_itrs_type _itrs;
			vecs_itrs_iterator  _cur_itr;
			comparator compare;
			bool first_inc;
		public:
			merge_iterator():self(NULL),first_inc(false),compare(def_comp<BType>)
			{
				_cur_itr=_itrs.end();
			}

			merge_iterator(pvector<BType> * vec):self(vec),first_inc(false),compare(def_comp<BType>)
			{
				_cur_itr=_itrs.end();
				
			}
			
			merge_iterator(const merge_iterator & other):self(other.self),_cur_itr(other._cur_itr),_itrs(other._itrs),
															first_inc(other.first_inc),compare(other.compare)
			{
			//cout<<"CUR="<<*(_cur_itr->second)<<endl;		
			}
			~merge_iterator()
			{
				//delete [] _itrs;
			}
			merge_iterator & operator=(const merge_iterator & other)
			{
				//cout<<"Assiging\n";
				self=other.self;
				//_cur_itr=other._cur_itr;
				_itrs=other._itrs;
				first_inc=other.first_inc;
				compare=other.compare;
				if( !_itrs.empty()){
					_cur_itr=_itrs.begin();
					first_inc=true;
					for(vecs_itrs_iterator  i=_cur_itr+1;i!=_itrs.end();++i)
						if(compare(*(i->second),*(_cur_itr->second)))
							_cur_itr=i;	
				}
				//cout<<"CUR="<<*(_cur_itr->second)<<endl;
				return *this;
			}
			operator elements_iterator()
			{
				return _cur_itr->second;
			}
                        
                        bool operator ==(const merge_iterator & other)
                        {
                            /*
                            if(self==other.self)
                                cout<<"Self equal\t";
                            
                            if( _cur_itr->second==other._cur_itr->second)
                                cout<<"_cur_itr equal\t";
                            
                            if(_itrs==other._itrs)
                                cout<<"_itrs equal\t";
                            */
                            return self==other.self && _cur_itr->second==other._cur_itr->second;
                        }
                        bool operator !=(const merge_iterator & other)
                        {
                            return self!=other.self || _cur_itr->second!=other._cur_itr->second;
                        }
                        
			operator bool()
			{				
				return _cur_itr!=_itrs.end();
			}
			merge_iterator & operator ++()//prefix ++
			{
				if(!first_inc && !_itrs.empty()){
					_cur_itr=_itrs.begin();
					first_inc=true;
				for(vecs_itrs_iterator  i=_cur_itr+1;i!=_itrs.end();++i)
					if(compare(*(i->second),*(_cur_itr->second)))
						_cur_itr=i;	
				}
				if(_cur_itr==_itrs.end())
					return *this;
				if(++(_cur_itr->second)==_cur_itr->first->end())
					/**/erase(&_itrs,_cur_itr);

				if(_itrs.empty()){
					_cur_itr=_itrs.end();
					return *this;
				}
				_cur_itr=_itrs.begin();

				for(vecs_itrs_iterator i=_cur_itr+1;i!=_itrs.end();++i){

					if(compare(*(i->second),*(_cur_itr->second)))
						_cur_itr=i;		
				}

				return 	*this;	
			}
			
			merge_iterator operator ++(int)//post ++
			{
				merge_iterator temp=*this;
				++(*this);
				return temp;
			
			}
			void start(comparator comp)
			{
				if(comp)
					compare=comp;
				
				_itrs.clear();
				for(pvector_iterator v=self->vecs.begin();v!=self->vecs.end();++v)
					if(v->begin()!=v->end())
						_itrs.push_back(make_pair(v,v->begin()));	
					
				if(_itrs.empty()){
					_cur_itr=_itrs.end();
					return;
				}
				
				_cur_itr=_itrs.begin();
				for(vecs_itrs_iterator  i=_cur_itr+1;i!=_itrs.end();++i)
					if(compare(*(i->second),*(_cur_itr->second)))
						_cur_itr=i;		
				
//cout<<"CUR@start="<<*_cur_itr->second<<endl;

			}
			const comparator compare_func()
			{ 
				return compare;
			}
			BType operator *()
			{
				return *(_cur_itr->second);				
			}
			elements_iterator operator->() const 
			{
				return _cur_itr->second;
			}
		
	};
	private:
		/*stxxl::*/pvector_type vecs;
		merge_iterator merger;
		bool merge_started;
		//unsigned n_vecs
		
		

	public:
		pvector(unsigned n=1):vecs(n),merger(this),merge_started(false)
		{
			//if(n>0)
			//	vecs=new vector<BType>[n];
		};
		pvector(unsigned n, unsigned size):vecs(n),merger(this),merge_started(false)
		{
			for(pvector_iterator v=vecs.begin();v!=vecs.end();++v)
				v->reserve(size);
			//if(n>0)
			//	vecs=new vector<BType>(size)[n];	
		}
		pvector(const pvector<BType> & other):vecs(other.vecs),merger(this),merge_started(other.merge_started)
		{			
		}
                unsigned nb_sub_vecs()
                {
                    return vecs.size();
                }
		pvector_iterator begin()
		{
			return vecs.begin();
		}
		elements_iterator begin_of(unsigned vec_ind)
		{
			return vecs[vec_ind].begin();
		}
		
		pvector_iterator end()
		{
			return vecs.end();
		}
		elements_iterator end_of(unsigned vec_ind)
		{
			return vecs[vec_ind].end();
		}
		
		void clear()
		{
			for(pvector_iterator v=vecs.begin();v!=vecs.end();++v)
				v->clear();
			//vecs.clear();
		}
		unsigned long size() 
		{
			unsigned long total_size=0;
			for(pvector_iterator v=vecs.begin();v!=vecs.end();++v)
				total_size+=v->size();
			return total_size;
		}
		merge_iterator get_merge_iterator(comparator comp=def_comp<BType>)
		{
			if(!merge_started){
				merger.start(comp);
				merge_started=true;
			}
			return merger;
		}
		void restart_merge(comparator comp=NULL)
		{
			merger.start(comp);
			merge_started=true;
		}
		
		void sort(comparator bf=def_comp<BType>)
		{//omp_set_dynamic(0);			
			//if(bf==NULL);			
			//	bf=def_comp<BType>;
			#pragma omp parallel 
			{
			//shared(vecs)
			for(pvector_iterator v=vecs.begin();v!=vecs.end();++v)
				#pragma omp single nowait
				{ 
				std::sort(v->begin(),v->end(),bf);
				}
			}
		}
		
		void push_back(pvector_iterator vec,const BType& elem)
		{
			vec->push_back(elem);
		}
		
		void push_back(unsigned vec_ind,const BType& elem)
		{
			//cout<<"writing: "<<elem<<" into vector: "<<vec_ind<<endl;
			vecs[vec_ind].push_back(elem);
		}
		/*
		const pvector_type& operator[](unsigned vec_ind)
		{
			return vecs[vec_ind];
		}
		*/
		~pvector()
		{
			//if (vec)
			//	delete [] vecs;
		}
		
		void print_sizes()
		{
			cout<<"Sizes of partial vectors:\n";
			unsigned i=0;
			for(pvector_iterator v=vecs.begin();v!=vecs.end();++v,++i)
				cout<<i<<":"<<v->size()<<"\t";
			cout<<endl;
		}
		
		template<class B> 
		friend ostream &operator<<(ostream &,pvector<B> &);
		template <class B, class Compare>
		friend void sort ( pvector<B> * , Compare,unsigned long );
                template <class B>
		friend void uniquify ( pvector<B> * );
		template <class B>
		friend void sort ( pvector<B> & ,unsigned );	
		template <class B, class Compare>
		friend void sort ( pvector_iterator  , pvector_iterator, Compare,unsigned  );
		template <class B, class Compare>
  		friend void sort (pvector<B> & ,unsigned ,unsigned , Compare ,unsigned  );
};

template <class BType>
ostream &operator<<(ostream &strm,pvector<BType> & pv)
{
	for(typename pvector<BType>::pvector_iterator v=pv.vecs.begin();v!=pv.vecs.end();++v){
		strm<<"========\n";
		for(typename pvector<BType>::elements_iterator i=v->begin();i!=v->end();++i)
			strm<<*i<<"\n";
		strm<<endl;
	}
	return strm;
};

template <class BType, class Compare>
  void sort (pvector<BType> & vec,unsigned ind_start,unsigned ind_end, Compare comp,unsigned mem=MEM_BLOCK )
{
	int ind;
	#pragma omp parallel for private(ind,ind_start,ind_end,comp,mem)
				//shared(vecs)			
			for( ind=ind_start;ind<=ind_end;++ind){
			//cout<<"calling std sort";
				//#pragma omp single nowait
				{ 
				#ifdef USE_STD_VECTORS
				cout<<"calling std sort";
					std::sort(vec.vecs[ind].begin(),vec.vecs[ind].end(),comp);
				#else
				cout<<"Calling stxxl\n";
					stxxl::sort(vec.vecs[ind].begin(),vec.vecs[ind].end(),comp,mem);
				#endif
				}
			}
	

};

template <class BTypeIterator, class Compare>
  void sort (BTypeIterator start,BTypeIterator end, Compare comp,unsigned mem=MEM_BLOCK )
{
	BTypeIterator v;
	#pragma omp parallel private(v,start,end,comp,mem)
	{
		
			//shared(vecs)			
			for( v=start;v!=end;++v){
			//cout<<"calling std sort";
				#pragma omp single nowait
				{ 
				#ifdef USE_STD_VECTORS
				cout<<"calling std sort";
					std::sort(v->begin(),v->end(),comp);
				#else
				cout<<"Calling stxxl\n";
					stxxl::sort(v->begin(),v->end(),comp,mem);
				#endif
				}
			}
	}

};
/*
template <class BTypeIterator>
 void sort ( BTypeIterator start,BTypeIterator end,unsigned mem=MEM_BLOCK)
 {
     sort(start,end,std::less<BType>(),mem);
 }
 */
template <class BType, class Compare>
  void sort ( pvector<BType> * pv, Compare comp,unsigned long mem=MEM_BLOCK )
{
	int i;
         for( i=0;i<pv->vecs.size();++i)
        {
          //  #pragma omp critical(sort)
				{
	//				cout<<"Thread "<<omp_get_thread_num()<<" sorts vector "<<i<<endl;
					pv->vecs[i].flush();
				}
        }
	#pragma omp parallel for firstprivate(pv),private(i)
	//{
		//shared(vecs)
			//for(typename pvector<BType>::pvector_iterator v=pv.vecs.begin();v!=pv.vecs.end();++v){
			for( i=0;i<pv->vecs.size();++i)

				//#pragma omp single nowait
				{
					if(pv->vecs[i].size()<2)
						continue;
				 
				#ifdef USE_STD_VECTORS
				cout<<"calling std sort";
					std::sort(pv->vecs[i].begin(),pv->vecs[i].end(),comp);
				#else
				
				#pragma omp critical(sort)
				{
					cout<<"Thread "<<omp_get_thread_num()<<" sorts vector "<<i<<endl;
//					pv->vecs[i].flush();
				}

					stxxl::sort(pv->vecs[i].begin(),pv->vecs[i].end(),comp,mem);//omp_get_num_threads());
				//}
				#endif
				}
	//		}
//	}
}
template <class BType>
 void uniquify ( pvector<BType> * pv)
{
    int i;
    #pragma omp parallel for firstprivate(pv),private(i)  
    for( i=0;i<pv->vecs.size();++i){
//        typename pvector<BType>::elements_iterator  new_end=std::unique(pv->vecs[i].begin(),pv->vecs[i].end());
        pv->vecs[i].resize(std::unique(pv->vecs[i].begin(),pv->vecs[i].end())-pv->vecs[i].begin());
    }       
    
}

template <class BType>
 void sort ( pvector<BType>& pv,unsigned mem=MEM_BLOCK)
 {
     sort(pv,std::less<BType>(),mem);
 }
 }
#endif
