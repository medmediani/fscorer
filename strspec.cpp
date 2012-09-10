/***************************************************************************
*    strspec.cpp
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
#include "strspec.h"

using namespace std;
void reverse_chars(char * str,int start,int end)
{
    if(start<0)
        start=0;
    if (end>=strlen(str)||end==-1)
        end=strlen(str)-1;
    if(end<=start)
        return;
    char * b=str+start;
    char * e=str+end;
    for(;b<e;++b,--e){
        *b ^=  *e;
        *e ^= *b ;
        *b ^= *e;
    }
	
}

void flip_around(char * str,char c,int start,int end)
{
	
	if(start<0)
		start=0;
	if (end>=strlen(str)||end==-1)
		end=strlen(str)-1;
        if(end<=start)
		return;
	//cout<<"reversing\n"	;
	reverse_chars(str,start,end);
	for(int i=start;i<=end;++i)
		if(str[i]==c){
			reverse_chars(str,start,i-1);
			reverse_chars(str,i+1,end);
			break;
		}			
}

void invert_alignment(char * al)
{
	char whitespace[10]=" \t\n\0";
	for(int start=0,end=0;end<=strlen(al);end++)
		if(strchr(whitespace,al[end])){
		//	cout<<"flipping\n";
			flip_around(al,'-',start,end-1);
			start=end+1;			
		}
}


void tokenize_in_vec(char * str,str_vec * vec)
{
    const char *whitespace =" \t\n\f\r\0";
    char* word,
    * savepos;
	
    word = strtok_r(str,whitespace,&savepos);
    while (word) {
        vec->push_back(word);
        word = strtok_r(NULL,whitespace,&savepos);			
    }
}


void get_alignments(char * str,aligns * amap)
{
    char *whitespace =(char *) " -\t\n\f\r\0";
    char* word,
    * savepos;
	
    word = strtok_r(str,whitespace,&savepos);
    while (word) {
        unsigned short first=atoi(word);			
		
        word = strtok_r(NULL,whitespace,&savepos);
        unsigned short second=atoi(word);
        amap->insert(make_pair(first,second));
        word = strtok_r(NULL,whitespace,&savepos);			
    }
}


pair<string,string> split(string str,string delim)
{
    size_t fpos=str.find(delim);
    size_t spos= fpos==string::npos? string::npos : fpos+delim.length();
    while(spos!=string::npos && str.substr(spos,delim.length())==delim)
        spos+=delim.length();
    if(spos==string::npos)
        return make_pair(str.substr(0,fpos),"");//pos=str.length();
    return make_pair(str.substr(0,fpos),str.substr(spos));
}

pair<string,string> rsplit(string str,string delim)
{
    size_t spos=str.rfind(delim);
                   
    size_t fpos= spos==string::npos? string::npos : spos+delim.length();
    while(spos!=string::npos && spos>=delim.length() && str.substr(spos-delim.length(),delim.length())==delim)
        spos-=delim.length();
    if(spos==string::npos)
        if(fpos==string::npos)
            return  make_pair("",str);
        else
            return make_pair("",str.substr(fpos));//pos=str.length();
    
    return make_pair(str.substr(0,spos),str.substr(fpos));
}
pair<string,string> split(string str,char delim)
{
    return split(str,string(1,delim));
}
pair<string,string> rsplit(string str,char delim)
{
    return rsplit(str,string(1,delim));
}
//
//int main(int argc,char ** argv)
//{
//    string name="ngrams";
//    pair<string,string> sp=rsplit(name,"/");
//    
//    cout <<"first part: "<<sp.first<<" second: "<<sp.second<<endl;
////    
////	char orig[100]="1-2 3-4 45-56 ";
////	char whitespace[10]=" \t\n\0";
////	char c='-';
////	int start=0,end=0;
////	cout<<"Orig before:"<<orig<<endl;
////	for(;end<=strlen(orig);end++)
////		if(strchr(whitespace,orig[end])){
////		//	cout<<"flipping\n";
////			flip_around(orig,c,start,end-1);
////			start=end+1;			
////		}
////	cout<<"Orig after:"<<orig<<endl;
//	
//		return 0;
//}
