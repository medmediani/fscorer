/***************************************************************************
*    merger.h
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
#include <omp.h>
#include <stdio.h>
#include "tqueue.h"
#include "pvector.h"

#ifndef _MERGER_H
#define _MERGER_H

#define DEF_MERGE_POOL_SIZE 1500


using namespace pvectors;
using namespace std;

template <class T>
class Merger
{
public:
    typedef T  BaseType;
//    typedef TQueue<BaseType> base_container_t;
    
//    typedef vector<base_container_t * > merge_pool_t;
//    typedef typename merge_pool_t::iterator merge_pool_iterator;
    
    typedef  std::multimap<BaseType, unsigned/*base_container_t */ > merge_interface_t;
    typedef typename merge_interface_t::iterator merge_interface_iterator;
    typedef typename pvector<T>::elements_const_iterator elements_const_iterator;
    typedef typename pvector<T>::elements_iterator elements_iterator;
private:

//    base_container_t * merge_pool;
    pvector<T> * orig;
    elements_iterator * positions;
    unsigned * assoc_threads;
    merge_interface_t merge_interface;
    
//
    inline void release_all() 
    {
//        delete [] merge_pool;
//        if(positions)
//            for(int i =0;i<orig->nb_sub_vecs();++i)
//                positions[i].close_cursor();
        delete [] positions;
        delete [] assoc_threads;
    }
    
public:
    //Merger();
    Merger(pvector<T> * pv_ptr=NULL):orig(pv_ptr),positions(NULL),assoc_threads(NULL)
    {
        if(!orig)
            return;
//        merge_pool=new base_container_t[orig->nb_sub_vecs()];
        positions=new elements_iterator[orig->nb_sub_vecs()];
        assoc_threads=new unsigned[orig->nb_sub_vecs()];
//        for(int i =0;i<orig->nb_sub_vecs();++i)
//            merge_pool[i].realloc(pool_size);
//
    }

    Merger(const Merger<T> & other):orig(other.orig),merge_interface(other.merge_interface)
    {
        release_all();
//        
//        delete [] merge_pool;
//        
//         if(positions)
//            for(int i =0;i<orig->nb_sub_vecs();++i)
//                positions[i].close_cursor();
//
//        delete [] positions;
//        delete [] assoc_threads;
        
//        merge_pool=new base_container_t[orig->nb_sub_vecs()];
        positions=new elements_iterator[orig->nb_sub_vecs()];
        assoc_threads=new unsigned[orig->nb_sub_vecs()];
        memcpy(assoc_threads,other.assoc_threads,orig->nb_sub_vecs()*sizeof(unsigned));
        for(int i =0;i<orig->nb_sub_vecs();++i){
//            merge_pool[i]=other.merge_pool[i];
            positions[i]=other.positions[i];
//            assoc_threads[i]=other.assoc_threads[i];
        }

    }
    
    ~Merger()
    {
        release_all();
        
//        delete [] merge_pool;
//        if(positions)
//            for(int i =0;i<orig->nb_sub_vecs();++i)
//                positions[i].close_cursor();
//        delete [] positions;
//        delete [] assoc_threads;
    }
  

//    pair<base_container_t *,elements_iterator * >
//    task_assoc(unsigned ind)
//    {
//        return make_pair(&merge_pool[ind],&positions[ind]);
//    }
    void init()
    {
        for(int i =0;i<orig->nb_sub_vecs();++i)
            init(i);
    }
    void init(unsigned ind)
    {

//        cout<<omp_get_thread_num()<<" initializing: "<<ind<<endl;
        if(ind>=orig->nb_sub_vecs())
            return;
//        cout<<omp_get_thread_num()<<" creating env: "<<ind<<endl;
        positions[ind]=orig->begin_of(ind);
        assoc_threads[ind]=omp_get_thread_num();

//        cout<<omp_get_thread_num()<<" registering: "<<ind<<endl;
//        orig->register_one(ind);
//        cout<<omp_get_thread_num()<<" registered: "<<ind<<endl;

        if(positions[ind]==orig->end_of(ind)){
//            merge_pool[ind].set_done();
            return;
        }


        #pragma omp critical(merge_interface_insert)
        {
            merge_interface.insert(make_pair(*positions[ind],/*&merge_pool[ind]*/ind));
        }
        ++positions[ind];
//        for(++positions[ind];positions[ind]!=orig->end_of(ind);++positions[ind])
//            try{
//            merge_pool[ind].push(*positions[ind]);
//        }catch(FullException & e){
//            return;
//        }

//        positions[ind].close_cursor();
//        merge_pool[ind].set_done();


//        for(merge_pool_iterator itr=merge_pool->begin();itr!=merge_pool->end();++itr)
//                try{
//                    merge_interface.insert(make_pair(((*itr)->pop()),*itr));
//                }catch( EmptyException & e){
//
//                }
     
    }
    
    Merger<T> & operator=(const Merger<T> & other)
    {
        if(orig->nb_sub_vecs()!=other.orig->nb_sub_vecs()){
//            delete [] merge_pool;
//            delete [] positions;
//          
            release_all();
            
//            merge_pool=new base_container_t[other.orig->nb_sub_vecs()];
            positions=new elements_iterator[other.orig->nb_sub_vecs()];
            assoc_threads=new unsigned[orig->nb_sub_vecs()];
        }
        orig=other.orig;
        merge_interface=other.merge_interface;

        for(int i =0;i<orig->nb_sub_vecs();++i){
//            merge_pool[i]=other.merge_pool[i];
            positions[i]=other.positions[i];
            assoc_threads[i]=other.assoc_threads[i];
        }
        return *this;
    }
    
    bool operator ==(const Merger<T>& other)
    {
        return /*merge_pool==other.merge_pool && */ merge_interface==other.merge_interface;
    }
    
    bool operator !=(const Merger<T>& other)
    {
        return /*merge_pool!=other.merge_pool &&  */merge_interface!=other.merge_interface;
    }    
    
    operator bool()
    {
        return !merge_interface.empty();
//        if(!merge_interface.empty())
//            return true;

//        for(int i =0;i<orig->nb_sub_vecs();++i)
//            if(!merge_pool[i].done())
//                return true;

//        return false;
    }

//    void fill_queue(unsigned ind)//,double percent=1)
//    {
//        if(ind>=orig->nb_sub_vecs())
//            return;
//        if(/*merge_pool[ind].done() || */positions[ind]==orig->end_of(ind))
//            return;
//
//        /*
//        #pragma omp critical(merge_interface_insert)
//        {
//            merge_interface.insert(make_pair(*positions[ind],&merge_pool[ind]));
//        }
//
//         */
////        unsigned isize=merge_pool[ind].size(),
////                portion2fill=percent*merge_pool[ind].max_size();
//        for(;positions[ind]!=orig->end_of(ind);++positions[ind]){
//            try{
//                merge_pool[ind].push(*positions[ind]);
////                if(merge_pool[ind].size()-isize>=portion2fill)
////                    return;
//            }catch(FullException & e){
////                cout<<"queue "<<ind<<" is full!\n";
//                return;
//            }
//        }
//        positions[ind].close_cursor();
//        merge_pool[ind].set_done();
//    }
//
    void print_assoc()
    {
        for(int i =0;i<orig->nb_sub_vecs();++i)
            cout<<i<<":"<<assoc_threads[i]<<"\t";
    }

//    void refresh()//double percent=1)
//    {
//        for(int i =0;i<orig->nb_sub_vecs();++i)
//                 if(assoc_threads[i]==omp_get_thread_num())
//                     fill_queue(i);//,percent);
//    }

    Merger<T> &  operator ++()
    {
//         if(merge_interface.empty()){
//             cerr<<"Thread "<<omp_get_thread_num()<<" encountered an empty interface... refreshing (This seriously degrades performance!)\n";
//             refresh();//PORTION);
//
//             for(int i =0;i<orig->nb_sub_vecs();++i)
//                 while(true)
//                     try{
//                         merge_interface.insert(make_pair(merge_pool[i].pop(),i/*&merge_pool[i]*/));
//                         break;
//                     }catch(EmptyException &e){
//                         if(merge_pool[i].done())
//                             break;
//                     }
//         }

         if(merge_interface.empty())
             return *this;
         
         unsigned /*base_container_t */ q=merge_interface.begin()->second;
         
         merge_interface.erase(merge_interface.begin());
         if(positions[q]==orig->end_of(q))
             return *this;
         #pragma omp critical(merge_interface_insert)
        {
            merge_interface.insert(make_pair(*positions[q],q));
        }
         ++positions[q];


//         while(true){
//             try{
//                 merge_interface.insert(make_pair(merge_pool[q].pop(),q));
//                 break;
//             }catch(EmptyException & e){
//                 if(merge_pool[q].done())
//                     break;
//                 else
//                     if(assoc_threads[q]==omp_get_thread_num()){
//                         cerr<<"Thread "<<omp_get_thread_num()<<" consumed its queue and is refreshing it.. If this happens often performance may get decreased!\n";
//                         fill_queue(q);//,PORTION);
//                     }
//             }
//         }
         
         return *this;         
    }
    
    Merger<T> operator ++(int)//post ++
    {
        Merger<T> temp=*this;
        ++(*this);
        return temp;        
    }

    BaseType operator *()
    {
        return merge_interface.begin()->first;
    }
    const BaseType  *  operator->() //const
    {
        //                            cout<<omp_get_thread_num()<<" Accessing:";
        //                            cout<<_cur_itr->second->first<<endl;
        return &(merge_interface.begin()->first);//_cur_itr->second;
    }
    
};
#endif