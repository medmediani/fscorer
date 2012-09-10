/***************************************************************************
*    main.cpp
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


#include <sys/sysinfo.h>
#include <omp.h>
#include <set>
#include <math.h>
#include <unistd.h>
#include <limits.h>

#include "common.h"
#include "tqueue.h"
#include "crono.h"
#include "strspec.h"
#include "anyoption.h"
//#include "tableout.h"
#include "load.h"
#include "score.h"
#include "prepare_space.h"
#include "ngrams.h"

#include "out.h"


void create_phrase_table(File lex_file,File revlex_file, const char* extract_file,
                        pcooc_vector_t ** cooc_vec_ptr,
                        global_score_type & g,unsigned long mem=MEM_BLOCK,
						 unsigned lagg=MAX_AGG,
                        const char * testset=NULL,const char * ngramsfname=NULL,pindicator_vector_t * pairs2keep = NULL,
						bool fast_creation=true
						)
{
    pcooc_vector_t * cooc_vec=*cooc_vec_ptr;

    Crono timer;

    cout<<"\n============================\n"
	        "Loading lexical dictionaries\n"
		    "============================\n"<<endl;
    LexMap * lex= new LexMap,
            *revlex= new LexMap;

    timer.start();

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            create_lex_map(lex_file,lex);
        }
        #pragma omp section
        {
            create_lex_map(revlex_file,revlex);
        }
    }

    timer.stop();

    cout<<"Done loading lexical dictionaries!\n";
    cout<<"Lexical dictionaries loaded in "<<timer.formatted_span()<<"\n";
    //printmap (&lex);
    cout<<"Lexical dictionary (source-target) has "<<lex->size()<<" entries.\nLexical dictionary (target-source) has "
                        <<revlex->size()<<" entries."<<endl;

    cout<<"\n======================\n"
	        "Loading phrase pairs\n"
			"======================\n"<<endl;          
    
    timer.reset();
    timer.start();

	if(fast_creation)
		pcreate_maps(extract_file,lex,revlex,cooc_vec);//(file_list,cooc_vec);
	else
		create_maps(fopen(extract_file,"r"),lex,revlex,cooc_vec);
  

    timer.stop();
    cout<<"Total number of pairs found in the extract file is: "<<cooc_vec->size()<<endl;
	cooc_vec->print_sizes();
    //We don't need the lexical maps anymore,,
    //Lexical scores have been already computed for the phrases

    delete lex;
    delete revlex;
//
    cout<<"Done loading!\n";
    cout<<"Phrase pairs loaded in "<<timer.formatted_span()<<endl;
    
    cout<<"\n=====================\n"
	        "Scoring phrases\n"
			"=====================\n"<<endl;
			
    score(cooc_vec_ptr,&g,mem,lagg, testset,ngramsfname,pairs2keep);
    cooc_vec=*cooc_vec_ptr;

}



char * get_testset_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 'm' );
    if(fname==NULL)
        return opt->getValue( "match-set" ) ;
    return fname; 
}

char * get_ngrams_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 'n' );
    if(fname==NULL)
        return opt->getValue( "ngrams" ) ;
    return fname; 
}

char * get_lex_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 'l' );
    if(fname==NULL)
        return opt->getValue( "lex" ) ;
    return fname;
}

char * get_revlex_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 'L' );
    if(fname==NULL)
        return opt->getValue( "revlex" ) ;
    return fname;
}

char * get_extract_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 'e' );
    if(fname==NULL)
        return opt->getValue( "extract" ) ;
    return fname;
}


char * get_output_dir(AnyOption * opt)
{
    char * dirname=opt->getValue( 'd' );
    if(dirname==NULL)
        return opt->getValue( "output-dir" ) ;

    return dirname;
}

char * get_stat_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 's' );
    if(fname==NULL)
        return opt->getValue( "stat" ) ;
    return fname;
}

char * get_output_fname(AnyOption * opt)
{
    char * fname=opt->getValue( 'o' );
    if(fname==NULL)
        return opt->getValue( "output" ) ;
    return fname;
}
void get_work_dirs(AnyOption * opt,str_vec_t & dirs)
{
    char * list_str=opt->getValue( 'w' );
    if(list_str==NULL)
        list_str=opt->getValue( "work-dir" );
    if(list_str==NULL){
        dirs.push_back(DEFAULT_TMP);
        return;
    }

    const char *name_sep =",;";
    char * name,*savepos;

    name=strtok_r(list_str,name_sep,&savepos);
    while(name){
        dirs.push_back(name);
        name=strtok_r(NULL,name_sep,&savepos);
    }


}

unsigned long get_size(AnyOption * opt,char short_opt='t',
                        const char * long_opt="tmp",unsigned long default_val=DEFAULT_TMP_SIZE)
{
    char * sizestr=opt->getValue( short_opt );
    if(sizestr==NULL)
        sizestr=opt->getValue( long_opt ) ;
    
    if(sizestr==NULL)
        return default_val;
    unsigned long size=strtoul(sizestr,NULL,10);
    
    if(size==0)
        return default_val;
    return size;        
}

string def_stat4pt(char * ptfname)
{
   string name(ptfname);
   int pos=name.rfind(PATH_SEP);
   if(pos==string::npos)
       pos=0;
   else
       ++pos;
   return name.substr(0,pos).append(DEFAULT_STAT);
   
}

int get_format(AnyOption * opt)
{
    char * fname=opt->getValue( 'f' );
    if(fname==NULL)
        fname=opt->getValue( "format" ) ;
    if(fname==NULL)
        return ABRIDGED_MOSES;
    
    else if(!strcmp(fname,"amoses")|| !strcmp(fname,"a"))
        return ABRIDGED_MOSES;
    else if(!strcmp(fname,"moses")|| !strcmp(fname,"m"))
        return MOSES;
    return ABRIDGED_MOSES;
}

int get_smoothing(AnyOption * opt)
{
    char * sname=opt->getValue( 'r' );
    if(sname==NULL)
        sname=opt->getValue( "relative-frequency-smoothing" ) ;
    if(sname==NULL)
        return KN_SMOOTHING;
    
    else if(!strcmp(sname,"none") || !strcmp(sname,"n"))
        return NO_SMOOTHING;
    else if(!strcmp(sname,"kneser-ney")|| !strcmp(sname,"kn"))
        return KN_SMOOTHING;
	else if(!strcmp(sname,"witten-bell")|| !strcmp(sname,"wb"))
		return WB_SMOOTHING;
	else if(!strcmp(sname,"good-turing")|| !strcmp(sname,"gt"))
		return GT_SMOOTHING;
	
    return KN_SMOOTHING;
}


int get_lex_aggregation(AnyOption * opt)
{
    char * sname=opt->getValue( 'a' );
    if(sname==NULL)
        sname=opt->getValue( "lexical-score-aggregation" ) ;
    if(sname==NULL)
        return MAX_AGG;
    
    else if(!strcmp(sname,"max") || !strcmp(sname,"m"))
        return MAX_AGG;
    else if(!strcmp(sname,"occurrence")|| !strcmp(sname,"o"))
        return OCC_AGG;
	else if(!strcmp(sname,"average")|| !strcmp(sname,"a"))
		return AVG_AGG;
	
    return MAX_AGG;
}

int main(int argc, char *argv[])
{
    Crono timer;
    
    AnyOption *opt = new AnyOption();


    string usage_str("Usage: ");
    usage_str.append(argv[0]).append(" [<options>]");

    opt->addUsage( "" );
    opt->addUsage( usage_str.c_str() );
    opt->addUsage( "" );
    opt->addUsage( "Options: " );
    opt->addUsage( " -h  --help\t\tPrints this help " );    
    opt->addUsage( " -o  --output\t\tOutput file name, slower if 'stdout' (default: 'stdout')" );

    opt->addUsage( " -l  --lex\t\tSource-target lexical file name(default: '"DEFAULT_LEX"')" );
    opt->addUsage( " -L  --revlex\t\tTarget-source lexical file name (default: '"DEFAULT_REVLEX"')" );
    opt->addUsage( " -e  --extract\t\tExtract file name (default: '"DEFAULT_EXTRACT"')" );
    
    opt->addUsage( " -n  --ngrams\t\tOptionally write only matching source phrases to the provided ngram list (default: '')" );
    opt->addUsage( " -m  --match-set\t\tOptionally write only matching source phrases to the provided test set (default: '')" );
    opt->addUsage( " -b  --mem-block\tThe memory used by STXXL while sorting per thread in MB (default: 512 MB)" );
    opt->addUsage( " -s  --stat\t\tFile name where the global counts will be saved (default: '"DEFAULT_STAT"')" );
    opt->addUsage( " -d  --output-dir\tOutput directory. This directory name will be prepended to all output file names, if given (default: '')" );
    opt->addUsage( " -f  --format\t\t{amoses|moses|a|m} phrase table format (default: 'amoses')" ); 
    opt->addUsage( " -c  --compress\t\tWhether the output table will be zipped or not, slower writing if true (default: false)" );
    opt->addUsage( " -p  --prepare\t\tWhether to prepare the working space (default: false)" );
    opt->addUsage( " -w  --work-dir\t\tIf {-p|--prepare} is true, this option specifies the temporary working directory(ies), in which STXXL files should be created (default: /tmp), comma-separated if many" );
    opt->addUsage( " -t  --tmp\t\tIf {-p|--prepare} is true, this option specifies the size of the temporary space in MB (default: 0 MB)" );
    opt->addUsage( " -k  --keep-tmp\t\tIf {-p|--prepare} is true, this flag states whether to remove the temporary allocated space or leave it for further usage (default: false)" );
    opt->addUsage( " -u  --load-unzipped\t\tFor fast loading. This supposes the extract file is unzipped (default: false)" );
	
	opt->addUsage( " -r  --relative-frequency-smoothing\t\t{none|kneser-ney|witten-bell|good-turing|n|kn|wb|gt} Smoothing technique for relative probability computation. (default: kneser-ney)" );
	
	opt->addUsage( " -a  --lexical-score-aggregation\t\t{max|occurrence|average|m|o|a} lexical score selection method: maximum, most occurring, or the average. (default: max)" );
	
    opt->addUsage( "" );
    
    opt->setFlag(  "help", 'h' ); 
    opt->setFlag(  "compress", 'c' ); 
    opt->setFlag(  "prepare", 'p' ); 
    opt->setFlag(  "keep-tmp", 'k' );
    opt->setFlag(  "load-unzipped", 'u' );
	 
    opt->setOption(  "lex", 'l' );
    opt->setOption(  "revlex", 'L' );
    opt->setOption(  "extract", 'e' );
    
    opt->setOption(  "mem-block", 'b' );
    opt->setOption(  "output-dir", 'd' );
    opt->setOption(  "output", 'o' );
    opt->setOption(  "format", 'f' );
    opt->setOption(  "work-dir", 'w' );
    opt->setOption(  "tmp", 't' );
    opt->setOption(  "stat", 's' );
	opt->setOption(  "relative-frequency-smoothing", 'r' );
    opt->setOption(  "lexical-score-aggregation", 'a' );
	
    opt->setOption(  "ngrams", 'n' );
    opt->setOption(  "match-set", 'm' );
    
    opt->processCommandArgs( argc, argv );
//     if( ! opt->hasOptions()) { /* print usage if no options */
//         opt->printUsage();
//         delete opt;
//         return 0;
//     }
    if( opt->getFlag( "help" ) || opt->getFlag( 'h' ) ){
        opt->printUsage();
        exit(0);
    }
    
    ostream * table_out_strm=NULL;
    File output;
    

    string out_dir;
    try{

        out_dir=string(get_output_dir(opt));


    }catch(logic_error){

        out_dir=string("");
    }
    
    if(out_dir !=""){
        if(*out_dir.rend()!=PATH_SEP)
            out_dir.append(1,PATH_SEP);
        cout<<"Output directory: '"<<out_dir<<"'"<<endl;
        mkdir(out_dir.c_str(),0744);
    }

    string output_fn;
    try{
        output_fn=string(get_output_fname(opt));
    }catch(logic_error){
        output_fn=string("");
    }

// Determining the output file name
    if( output_fn != ""  ){
        output_fn=string (out_dir).append(output_fn);
        cout << "Output file name: '" << output_fn;
        if(opt->getFlag( "compress" ) || opt->getFlag( 'c' )){
            output=fopen(string(output_fn).append(".gz").c_str(),"w");
            cout<<".gz";
        }else
            table_out_strm=new ofstream(output_fn);
        cout<<"'"<<endl;
    }else{
        if(opt->getFlag( "compress" ) || opt->getFlag( 'c' )){
            output=fdopen(dup(1),"w");
            dup2(2,1);
        }else{
            table_out_strm=new ostream(cout.rdbuf());
            cout.rdbuf(cerr.rdbuf());
        }
        
        cout<<"No output file name was specified!\nThe output is redirected to 'stderr' and the phrase table will be written to 'stdout'\n";

    }
    
    if( output_fn=="" && (opt->getFlag( "compress" ) || opt->getFlag( 'c' ))) {
        cout<<"Output table will be zipped\n";
    }
    //Fast read??
    bool par_read= (opt->getFlag( "load-unzipped" ) || opt->getFlag( 'u' ));
	if(par_read)
		cout<<"Every thread will read directly from the file\n";
	else
		cout<<"Reading will performed as producer/consumer\n";
    
    /****************************************************************/
	
	//Find whether the necessary files are available
    char  def_lex[]=DEFAULT_LEX;
    char def_revlex[]=DEFAULT_REVLEX;
    char def_ext[]=DEFAULT_EXTRACT;

    char * lex_fn=get_lex_fname(opt);
    if( lex_fn != NULL  )
        cout << "Source-target lexical file name: "  ;
    else{
        lex_fn=def_lex;
        cout << "Source-target lexical file name not specified, taking the default: "  ;
        
    }
    cout<<"'"<< lex_fn<<"'"<< endl;
    char * revlex_fn=get_revlex_fname(opt);
    if( revlex_fn != NULL  )
        cout << "Target-source lexical file name: "  ;
    else{
        revlex_fn=def_revlex;
        cout << "Target-source lexical file name not specified, taking the default: "  ;
        
    }
    cout<< "'"<<revlex_fn<<"'"<< endl;

    char * extract_fn=get_extract_fname(opt);
    if( extract_fn != NULL  )
        cout << "Extract file name: "  ;
    else{
        extract_fn=def_ext;
        cout << "Extract file name not specified, taking the default: "  ;
        
    }
    cout<<"'"<< extract_fn<< "'"<<endl;
	
    File lex_f=fopen(lex_fn,"r");
	if(!lex_f)
		cerr<<"Could not open file: '"<<lex_fn<<"'\n";
    File revlex_f=fopen(revlex_fn,"r");
	if(!revlex_f)
		cerr<<"Could not open file: '"<<revlex_fn<<"'\n";

    File extract_f=fopen(extract_fn,"r");
    if(!extract_f)
		cerr<<"Could not open file: '"<<extract_fn<<"'\n";

    if(!lex_f || !revlex_f || !extract_f)
        return -1;
    
    fclose(extract_f);
	
    /**********************************************************************/
	//determine where to store the global counts
    char default_stat[]=DEFAULT_STAT;
    
    string  stat_fn;//=get_stat_fname(opt);

    try{
        stat_fn=string(get_stat_fname(opt));
    }catch(logic_error){
        stat_fn=string("");
    }

    if( stat_fn != ""  ){
        stat_fn=string (out_dir).append(stat_fn);

        cout << "Stat file name: "  ;
    }
    else{
        stat_fn=string (out_dir).append(default_stat);
        cout << "Stat file name not specified, saving to: "  ;

    }
    cout<<"'"<< stat_fn<< "'"<<endl;
    /**********************************************************************/
	//Find the format in which we will write
	
    int table_format=get_format(opt);
    //if( table_format!= NULL  )
    cout << "Format: ";
    switch(table_format){        
        case ABRIDGED_MOSES: cout<<"'abridged moses'";break;
        case MOSES:cout<<"'extended moses'";break;
        default:cout<<"unknown";
    }
    
    cout<<endl;
  
    global_score_type gs;

  /**********************************************************************/
	//Find the smoothing technique
	
    int smoothing=get_smoothing(opt);
    //if( table_format!= NULL  )
    cout << "Smoothing: ";
    switch(smoothing){        
        case NO_SMOOTHING: cout<<"'none'";break;
        case KN_SMOOTHING:cout<<"'kneser-ney'";break;
		case WB_SMOOTHING:cout<<"'witten-bell'";break;
		case GT_SMOOTHING:cout<<"'good-turing'";break;
        default:cout<<"unknown";
    }
    
    cout<<endl;
   /**********************************************************************/
	//Find the lexical score selection method
	
    int lex_agg=get_lex_aggregation(opt);
    //if( table_format!= NULL  )
    cout << "Lexical score selection method: ";
    switch(lex_agg){        
        case MAX_AGG: cout<<"'maximum'";break;
        case OCC_AGG:cout<<"'most occurring'";break;
		case AVG_AGG:cout<<"'average'";break;
		
        default:cout<<"unknown";
    }
    
    cout<<endl;
   /**********************************************************************/
	// Finding the number of threads
    char nump[3];
    sprintf(nump,"%d",omp_get_num_procs());
    setenv("OMP_NUM_THREADS",nump,0);
    cout<<"Number of threads: "<<getenv("OMP_NUM_THREADS")<<endl;
    /**********************************************************************/
	// Finding the sorting memory
    unsigned long mem_block=get_size(opt,'b',"mem-block",DEFAULT_MEM_BLOCK);
    cout<<"STXXL sorting memory size: "<<mem_block<<" MiB"<<endl;
    mem_block *= 1024*1024; //from mega to bytes
    /**********************************************************************/
	// Finding the test set ngrams to which we have to match the source phrases
    string suboutput_fn("");
    
    if(get_ngrams_fname(opt)){
        cout<<"Ngram list: "<<get_ngrams_fname(opt)<<endl;
        suboutput_fn=rsplit(get_ngrams_fname(opt),PATH_SEP).second;
    }else if(get_testset_fname(opt)){
        cout<<"Testset: "<<get_testset_fname(opt)<<endl;
        suboutput_fn=rsplit(get_testset_fname(opt),PATH_SEP).second;
    }
    
    if(!suboutput_fn.empty()){
		if(opt->getFlag( "compress" ) || opt->getFlag( 'c' )){
			suboutput_fn.append(".subpt.stxxl.gz");
		}else{
			suboutput_fn.append(".subpt.stxxl");
		}
        suboutput_fn=string (out_dir).append(suboutput_fn);
        cout<<"Subsampled output phrase table: '"<<suboutput_fn<<"'"<<endl;
    }
    
    //Fast write ??
    bool par_write=!( output_fn=="" || opt->getFlag( "compress" ) || opt->getFlag( 'c' ));
    if(par_write)
        cout<<"Parallel low level writing will be performed"<<endl;
    /**********************************************************************/
	//Preparing the STXXL temporary space  
    str_vec_t working_dirs;

    bool remove_tmp_at_exit=false;
	
    if( opt->getFlag( "prepare" ) || opt->getFlag( 'p' ) ){
        get_work_dirs(opt,working_dirs);
//        working_dir=get_tmp_working_dir(opt);
        cout<<"Working directory (ies): ";//<<working_dir<<"'\n";
        for(str_vec_t::const_iterator dname_ptr=working_dirs.begin();dname_ptr!=working_dirs.end();++dname_ptr)
            cout<<"'"<<*dname_ptr<<"', ";
        cout<<endl;

        unsigned long tmp_size=get_size(opt);
        cout<<"Working space size: "<<tmp_size<<" MiB"<<(tmp_size>0? "\n":" (Auto-grow)\n");
        
        remove_tmp_at_exit=!(opt->getFlag( "keep-tmp" ) || opt->getFlag( 'k' ));
        cout<<"The disk space allocated will be "<<(remove_tmp_at_exit? "deallocated":"kept")<<" after finishing\n";
        
        cout<<"\n=============================================\n\n";
        timer.start();
        prepare(tmp_size,working_dirs);
        timer.stop();
        cout<<"\nPreparing the disk working space took "<<timer.formatted_span()<<endl<<endl;
        timer.reset();
    }

   
    //Creating the phrase table
    pcooc_vector_t * phrase_cooc=new pcooc_vector_t(NB_WORKERS);
    
    pindicator_vector_t * pairs2keep=NULL;
    if(get_testset_fname(opt) != NULL || get_ngrams_fname(opt)!= NULL)
        pairs2keep=new pindicator_vector_t (NB_WORKERS);
    
   
    //
    timer.start();
    create_phrase_table(lex_f,revlex_f,extract_fn,&phrase_cooc,gs,mem_block,lex_agg,get_testset_fname(opt),get_ngrams_fname(opt),pairs2keep,par_read);
	gs.estimate_global_params();
    cout<<"\n========================================\nGlobal parameters\n========================================\n"<<endl;
    cout.precision(2);
    cout<<"N1:\t"<<setw(20)<<gs.big_n1<<" ("<<100*(double)gs.big_n1/gs.all<<"%)"<<endl;
    cout<<"N2:\t"<<setw(20)<<gs.big_n2<<" ("<<100*(double)gs.big_n2/gs.all<<"%)"<<endl;
    cout<<"N3:\t"<<setw(20)<<gs.big_n3<<" ("<<100*(double)gs.big_n3/gs.all<<"%)"<<endl;
    cout<<"N4:\t"<<setw(20)<<gs.big_n4<<" ("<<100*(double)gs.big_n4/gs.all<<"%)"<<endl;
	cout<<"Co-occurrences:\t"<<setw(20)<<gs.all_occurs<<endl;
    cout<<"#Pairs:\t"<<setw(20)<<gs.all<<endl;

    cout<<"\n========================================\n";

    cout<<"\n============================"<<endl<<
            "Writing to disk"<<endl<<
            "============================\n\n";
    Crono writing;
    writing.start();

    if(par_write){
        int fho=open(output_fn.c_str(),O_WRONLY | O_CREAT,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
        psave_to_file(fho,phrase_cooc,&gs,smoothing,table_format,suboutput_fn,pairs2keep);
        close(fho);
    }else{
		if(opt->getFlag( "compress" ) || opt->getFlag( 'c' ))
			save_to_file(output,phrase_cooc,&gs,smoothing,table_format,suboutput_fn,pairs2keep);
		else
			save_to_stream(*table_out_strm,phrase_cooc,&gs,smoothing,table_format,suboutput_fn,pairs2keep);

//         switch(table_format){
//             case ABRIDGED_MOSES:
//                  if(opt->getFlag( "compress" ) || opt->getFlag( 'c' ))
//                     dump4moses_restricted(phrase_cooc,&gs,smoothing,output);
//                 else
//                     dump4moses_restricted(phrase_cooc,&gs,smoothing,*table_out_strm);
//                 break;
//    
//             case MOSES:
//                 if(opt->getFlag( "compress" ) || opt->getFlag( 'c' ))
//                     dump4moses(phrase_cooc,&gs,smoothing,output);
//                 else
//                     dump4moses(phrase_cooc,&gs,smoothing,*table_out_strm);
//                 break;
// 			
// 		}
    }
    
    writing.stop();
    
    cout<<"Table successfully written ";
    if(opt->getFlag( "compress" ) || opt->getFlag( 'c' ))
        cout<<"(and compressed) ";
    if(output_fn=="")
        cout<<"to 'stdout'";
    else
        cout<<"to "<<"'"<<output_fn<<(opt->getFlag( "compress" ) || opt->getFlag( 'c' )? ".gz'":"'");
    cout<<". Writing took "<<writing.formatted_span()<<endl;
//    cout<<endl;

    //  cout<<"finalizing!\n";
    timer.stop();
    cout<<"\nDone creating the phrase table!\nThe whole process took "<<timer.formatted_span()<<endl;

     cout<<"\nSaving the global scores to '"<<stat_fn<<"'...\n";
     string rename_stat=string(stat_fn).append(".").append(tmp_name("",false)).append(".old");
     
//     string sf(stat_fn);
     if(rename(stat_fn.c_str(),rename_stat.c_str())==0)
         cout<<"The file '"<<stat_fn.c_str()<<"' existed before and was renamed to '"<<rename_stat<<"'\n";
    
    ofstream stat_f(stat_fn,ios::out);
    if(!stat_f)
		cerr<<"Could not open file: '"<<stat_fn<<"' for writing\n";
    else{
        stat_f<<"N1 = "<< gs.big_n1<<"\nN2 = "<<gs.big_n2 <<"\nN3 = "<<gs.big_n3
                <<"\nN4 = "<< gs.big_n4<<"\nall = "<< gs.all<<"\n";
        stat_f.close();
    }
    cout<<"Realeasing resources...\n";

    delete phrase_cooc;
    
    if(pairs2keep)
        delete pairs2keep;
    
    if(remove_tmp_at_exit && ! working_dirs.empty()){
        cout<<"Releasing disk temporary space...\n";
        remove_tmp(working_dirs);
    }
    
    if( output_fn != ""  ){
        //         cout<<"closing output file\n";
        if(opt->getFlag( "compress" ) || opt->getFlag( 'c' ))
            fclose(output);
        else
            ((ofstream *)table_out_strm)->close();
    }else{
        //         cout<<"restoring stdout\n";
        if(opt->getFlag( "compress" ) || opt->getFlag( 'c' )){
            fclose(output);
            //dup2(fileno(output),1);
        }else
            cout.rdbuf(table_out_strm->rdbuf());
    }
    
    delete opt;

    if(table_out_strm)
        delete table_out_strm;
    cout<<"Scoring successfully finishing!\n";
    return 0;
}
