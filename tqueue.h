/***************************************************************************
*    tqueue.h
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
#include <assert.h>
#include <exception>

#ifndef _TQUEUE_H
#define _TQUEUE_H

using namespace std;
class EmptyException:public exception
{
	virtual const char* what() const throw()
	{
		return "There was an attempt to dequeue while queue is empty";
	}

};

class FullException:public exception
{
	virtual const char* what() const throw()
	{
		return "There was an attempt to enqueue while queue is full";
	}

};

template <class BaseType>
class TQueue
{
	public:
		TQueue();
		TQueue(unsigned);
		TQueue(const TQueue<BaseType> &);
		~TQueue(){
			//if(items)
				delete [] items;
		}
		void realloc(unsigned );
		void push(const BaseType &);
		BaseType pop();
		unsigned size(){
			return total;
		};
		bool empty(){
			return total==0;
		};
		bool full(){
			return total==max_total;
		}
	
	private:
		unsigned front,back;
		unsigned total;
		unsigned max_total;
		BaseType * items;
		
	template<class BT> friend std::ostream & operator << (std::ostream & , const TQueue<BT> & );
};

template <class BaseType>
TQueue<BaseType>::TQueue():front(0),back(0),total(0)
{
	max_total=omp_get_num_procs();
	//cout<<"Allocating "<<max_total<<" elements"<<endl;
	items=new BaseType[max_total];
}

template <class BaseType>
TQueue<BaseType>::TQueue(unsigned nitems):front(0),back(0),total(0),max_total(nitems)
{
	//cout<<"Allocating "<<max_total<<" elements"<<endl;
	items=new BaseType[max_total];
}

template <class BaseType>
TQueue<BaseType>::TQueue(const TQueue<BaseType> & aqueue):front(aqueue.front),
					back(aqueue.back),total(aqueue.total),max_total(aqueue.max_total)
{
	//cout<<"Allocating "<<max_total<<" elements"<<endl;
	items=new BaseType[max_total];
	for(unsigned i=0;i<max_total;i++)
		items[i]=aqueue.items[i];
}

template <class BaseType>
void TQueue<BaseType>::realloc(unsigned newsize)
{
	if(newsize == max_total)
		return;
	//if(items)
	delete [] items;
	max_total=newsize;
	//cout<<"Allocating "<<max_total<<" elements"<<endl;
	items=new BaseType[max_total];	
}

template <class BaseType>	
void TQueue<BaseType>::push(const BaseType & item)
{ 
	//cout<<"Pushing..."<<endl;
	//cout<<total<<endl;
	 bool isFull;
	
	#pragma omp critical(items_access)
	{
		#pragma omp critical(total_access)
	 	{
			 isFull=total>=max_total;
	 	}
		
		if(!isFull){
			items[back]=item;
			back=++back %max_total;
			
			#pragma omp critical(total_access)
	 		{
				total++;
	 		}
		}
	}
	if(isFull)
		throw FullException();
	
}

template <class BaseType>	
BaseType  TQueue<BaseType>::pop()
{
	//cout<<"Popping..."<<endl;
	//cout<<"front: "<<front<<"\ttotal: "<<total<<"\tmax total: "<<max_total<<endl;
	bool isEmpty;
	BaseType item;
	#pragma omp critical(front_access)
	{
		#pragma omp critical(total_access)
		 {
			 isEmpty=total<=0;
	 	 }
		
		if(!isEmpty){
			item=items[front];
			front=++front % max_total;
			
			#pragma omp critical(total_access)
	 		{
				total--;
	 		}
		}
	}
	if (isEmpty)
		throw EmptyException();
	
	return item;//s[pf];
}	
template <class BaseType>
std::ostream & operator << (std::ostream & o, const TQueue<BaseType> & obj)
{
	o<<std::endl;
	for(unsigned i=0;i<obj.max_total;i++)
		o<<obj.items[i]<<"\t";
    
    return o;
}
void print_time(const char * msg)
{
	cout<<msg<<" at: "<<omp_get_wtime()<<endl;
}
#endif
