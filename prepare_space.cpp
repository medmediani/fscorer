/***************************************************************************
*    prepare_space.cpp
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
/***************************************************************************
 *  utils/createdisks.cpp
 *
 *  Part of the STXXL. See http://stxxl.sourceforge.net
 *
 *  Copyright (C) 2003 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2007 Andreas Beckmann <beckmann@mpi-inf.mpg.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#ifdef USE_GZIP
 #undef USE_GZIP
#endif

#include "processdir.h"
#include "prepare_space.h"



using stxxl::request_ptr;
using stxxl::file;
using stxxl::timestamp;



using namespace std;

#define MB (1024 * 1024)
#define GB (1024 * 1024 * 1024)


string tmp_name(string root,bool prepend_root)
{
    ostringstream name;
                       
    char hostname[MAXHOSTNAMELEN];                        
    gethostname(hostname, MAXHOSTNAMELEN);   
    if(prepend_root)
        name<<root<<PATH_SEP;//create in /tmp dir
    name<<hostname<<"_"<<abs(getpid());//<<"_"<<time(NULL);
    return name.str();
}

void prepare(stxxl::int64 size_in_mb,const str_vec_t &  dnames)
{
    string cfg_f_name=tmp_name();
    cfg_f_name.append(STXXL_CFG_EXT);

    FILE * f=fopen(cfg_f_name.c_str(),"w");

    cout<<"Preparing the STXXL temporary working space...\n\n";

    for(str_vec_t::const_iterator dname_ptr=dnames.begin();dname_ptr!=dnames.end();++dname_ptr){
        
//        string file_name=tmp_name(*dname_ptr);
        string disk_file(tmp_name(*dname_ptr));

//    file_name.append(STXXL_CFG_EXT);

//    FILE * f=fopen(string(file_name).append(STXXL_CFG_EXT).c_str(),"w");
//    mkdir(file_name.str().c_str(),0744);
//    #pragma omp parallel private(disk_file),shared(file_name,size_in_mb)
//    {
//        disk_file<<file_name<<'_' <<omp_get_thread_num()<<STXXL_EXT;
        disk_file.append(STXXL_EXT);
        cout<<"Adding disk file: "<<disk_file<<endl;
//        disk_file<<STXXL_EXT;
        if(size_in_mb > 0)
             create_space(size_in_mb, disk_file.c_str() );
//        if(size_in_mb/omp_get_num_threads() > 0)
//            create_space(size_in_mb/omp_get_num_threads(), disk_file.str().c_str() );
//        #pragma omp critical(cfg_file_write)
//        {
//            cout<<omp_get_thread_num()<<": My temp file name: "<<disk_file.str()<<endl<<"My temp file size: "<<size_in_mb/omp_get_num_threads()<<endl;
            fprintf(f,"disk=%s,%ld,syscall\n",disk_file.c_str(),size_in_mb);
    }
//    }
    fclose(f);
//    file_name.append(STXXL_CFG_EXT);
//    cout<<"Config file name: "<<file_name<<endl;
//    exit(0);
    cout<<"\nSTXXL configuration file: "<<cfg_f_name<<endl;
    setenv(STXXL_CFG_ENV_VAR,cfg_f_name.c_str(),1);
}
void remove_tmp(const str_vec_t & dnames)
{
    string cfg_f_name=tmp_name();
    cfg_f_name.append(STXXL_CFG_EXT);
    for(str_vec_t::const_iterator dname_ptr=dnames.begin();dname_ptr!=dnames.end();++dname_ptr){

//        string file_name=tmp_name(*dname_ptr);
        string disk_file(tmp_name(*dname_ptr));

//    #pragma omp parallel private(disk_file),shared(file_name)
//    {
//        disk_file<<file_name<<'_' <<omp_get_thread_num()<<STXXL_EXT;
        remove_tree(disk_file.append(STXXL_EXT));//remove_tree(disk_file.str());//append(STXXL_EXT));
    }
        
    remove_tree(cfg_f_name);//file_name.append(STXXL_CFG_EXT));
    
}

void create_space(stxxl::int64 size_in_mb, const char * fname,... )
{
//    if (argc < 3)
//    {
//        STXXL_MSG("Usage: " << argv[0] << " filesize_in_MB filename1 [filename2 [filename3 ...]]");
//        return -1;
//    }
    

    cout<<"Preparing temporary working space...\n";

    stxxl::int64 offset = 0;
    stxxl::int64 length = size_in_mb * MB;
    stxxl::int64 endpos = offset + length;

    std::vector<std::string> disks_arr;
    
    va_list file_list;
    va_start( file_list, fname );
    while(fname){
        unlink(fname);
        std::cout << "# Add disk: " << fname << " ("<<size_in_mb<<" MB)"<<std::endl;
        disks_arr.push_back(fname);
        fname=va_arg(file_list, const char *);
    }
    va_end(file_list);

    const unsigned ndisks = disks_arr.size();

    unsigned buffer_size = 256 * MB;
    const unsigned buffer_size_int = buffer_size / sizeof(int);

    unsigned chunks = 2;
    const unsigned chunk_size = buffer_size / chunks;
    const unsigned chunk_size_int = chunk_size / sizeof(int);

    unsigned i = 0, j = 0;

    request_ptr * reqs = new request_ptr[ndisks * chunks];
    file ** disks = new file *[ndisks];
    int * buffer = (int *)stxxl::aligned_alloc<BLOCK_ALIGN>(buffer_size * ndisks);
#ifdef WATCH_TIMES
    double * r_finish_times = new double[ndisks];
    double * w_finish_times = new double[ndisks];
#endif

    for (i = 0; i < ndisks * buffer_size_int; i++)
        buffer[i] = i;

    for (i = 0; i < ndisks; i++)
    {
#ifdef BOOST_MSVC
 #ifdef RAW_ACCESS
        disks[i] = new stxxl::wincall_file(disks_arr[i],
                                           file::CREAT | file::RDWR | file::DIRECT, i);
 #else
        disks[i] = new stxxl::wincall_file(disks_arr[i],
                                           file::CREAT | file::RDWR, i);
 #endif
#else
 #ifdef RAW_ACCESS
        disks[i] = new stxxl::syscall_file(disks_arr[i],
                                           file::CREAT | file::RDWR | file::DIRECT, i);
 #else
        disks[i] = new stxxl::syscall_file(disks_arr[i],
                                           file::CREAT | file::RDWR, i);
 #endif
#endif
    }

    while (offset < endpos)
    {
        const unsigned current_block_size = length ? std::min<stxxl::int64>(buffer_size, endpos - offset) : buffer_size;
        const unsigned current_chunk_size = current_block_size / chunks;

        std::cout << "Disk offset " << std::setw(7) << offset / MB << " MB: " << std::fixed;

        double begin = timestamp(), end;

#ifndef DO_ONLY_READ
        for (i = 0; i < ndisks; i++)
        {
            for (j = 0; j < chunks; j++)
                reqs[i * chunks + j] =
                    disks[i]->awrite(buffer + buffer_size_int * i + j * chunk_size_int,
                                     offset + j * current_chunk_size,
                                     current_chunk_size,
                                     stxxl::default_completion_handler());
        }

 #ifdef WATCH_TIMES
        watch_times(reqs, ndisks, w_finish_times);
 #else
        wait_all(reqs, ndisks * chunks);
 #endif

        end = timestamp();

/*
   std::cout << "WRITE\nDisks: " << ndisks
        <<" \nElapsed time: "<< end-begin
        << " \nThroughput: "<< int(1e-6*(buffer_size*ndisks)/(end-begin))
        << " Mb/s \nPer one disk:"
        << int(1e-6*(buffer_size)/(end-begin)) << " Mb/s"
        << std::endl;
*/

 #ifdef WATCH_TIMES
        out_stat(begin, end, w_finish_times, ndisks, disks_arr);
 #endif
        std::cout << std::setw(7) << int(1e-6 * (current_block_size) / (end - begin)) << " MB/s,";
#endif


#ifndef NOREAD
        begin = timestamp();

        for (i = 0; i < ndisks; i++)
        {
            for (j = 0; j < chunks; j++)
                reqs[i * chunks + j] = disks[i]->aread(buffer + buffer_size_int * i + j * chunk_size_int,
                                                       offset + j * current_chunk_size,
                                                       current_chunk_size,
                                                       stxxl::default_completion_handler());
        }

 #ifdef WATCH_TIMES
        watch_times(reqs, ndisks, r_finish_times);
 #else
        wait_all(reqs, ndisks * chunks);
 #endif

        end = timestamp();

/*
   std::cout << "READ\nDisks: " << ndisks
        <<" \nElapsed time: "<< end-begin
        << " \nThroughput: "<< int(1e-6*(buffer_size*ndisks)/(end-begin))
        << " Mb/s \nPer one disk:"
        << int(1e-6*(buffer_size)/(end-begin)) << " Mb/s"
            << std::endl;
*/

        std::cout << int(1e-6 * (current_block_size) / (end - begin)) << " MB/s" << std::endl;

#ifdef WATCH_TIMES
        out_stat(begin, end, r_finish_times, ndisks, disks_arr);
#endif

        if (CHECK_AFTER_READ) {
            for (int i = 0; unsigned(i) < ndisks * buffer_size_int; i++)
            {
                if (buffer[i] != i)
                {
                    int ibuf = i / buffer_size_int;
                    int pos = i % buffer_size_int;

                    std::cout << "Error on disk " << ibuf << " position " << std::hex << std::setw(8) << offset + pos * sizeof(int)
                              << "  got: " << std::hex << std::setw(8) << buffer[i] << " wanted: " << std::hex << std::setw(8) << i
                              << std::dec << std::endl;

                    i = (ibuf + 1) * buffer_size_int; // jump to next
                }
            }
        }
#else
        std::cout << std::endl;
#endif

        offset += current_block_size;
    }

    for (i = 0; i < ndisks; i++)
        delete disks[i];

    delete[] reqs;
    delete[] disks;
    stxxl::aligned_dealloc<BLOCK_ALIGN>(buffer);

#ifdef WATCH_TIMES
    delete[] r_finish_times;
    delete[] w_finish_times;
#endif

//    return 0;
}
