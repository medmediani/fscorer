/***************************************************************************
*    read_file_th.h
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

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <assert.h>
#include <exception>

#ifndef READ_FILE_TH_H
#define	READ_FILE_TH_H

using namespace std;

class EmptyStreamException:public exception
{
	virtual const char* what() const throw()
	{
		return "Attempt to read from an empty stream";
	}

};


struct shared_args{
    char * fname;
    unsigned long long fsize;
    string ** residues;
    
};

struct private_args{
    string  fline;
    char * buf;
    int fh,
        bytesread;
    unsigned long long off;
    string line;
    istringstream strm;
    unsigned long long quota;
    string prev;
    
};

string get_lines(shared_args * ,private_args * );

#endif	/* READ_FILE_TH_H */

