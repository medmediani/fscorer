/***************************************************************************
*    crono.h
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

#ifndef _CRONO_H
#define _CRONO_H

#include <sys/time.h>
#include <string>

class Crono
{
	private:
		/*clock_t*/double s_clocks;
		/*clock_t*/double e_clocks;
		//bool started;
		double _span;
	public:
		Crono():s_clocks(0),e_clocks(0),/*started(false),*/_span(0.0){}
		void start()
		{
			timeval current;

                        gettimeofday(&current, NULL);
                        s_clocks=current.tv_sec+(current.tv_usec/1000000.0);

			//if(!started){
			//	s_clocks=time(NULL);//clock();
				//started=true;
			//}
		}
		void stop()
		{	
			timeval current;
			gettimeofday(&current, NULL);		
			e_clocks=current.tv_sec+(current.tv_usec/1000000.0);//time(NULL);//clock();
			_span+=double(e_clocks-s_clocks);//CLOCKS_PER_SEC;
		}
		void reset()
		{
			//started=false;
			_span=0.0;
		}
		double span()
		{	
			return _span;
		}
                double span_plus(double plus)
                {
                    return _span+plus;
                }
                string formatted_span(double plus)
                {
                    double old_span=_span;
                    _span+=plus;
                    string res=formatted_span();
                    _span=old_span;
                    return res;
                }
		string formatted_span()
		{
			int total=int(_span);
			int hr=total/3600;
			int mn=(total%3600)/60;
			float sc=(total%3600)%60 + _span-total;
			char str[100];
			if(hr)
				sprintf(str,"%dh%dm%0.4fs",hr,mn,sc);
			else if(mn)
				sprintf(str,"%dm%0.4fs",mn,sc);
			else
				sprintf(str,"%0.4fs",sc);
			return string(str);
		}
};
#endif
