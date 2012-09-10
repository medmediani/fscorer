/***************************************************************************
*    out.h
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

#include <fcntl.h>
//#include "scorer_types.h"
#include "common.h"
#include "crono.h"
#include "pvector.h"

#ifndef _OUT_H
#define	_OUT_H
//#ifdef File
//#undef  File
//#endif

typedef double (*smoother_t)(cooc_type, global_score_type *);


#define CHOOSE_SMOOTHERS(smoothing,ssmoother,tsmoother) switch(smoothing){\
												case NO_SMOOTHING:\
														ssmoother=&s_no_smooth;\
														tsmoother=&t_no_smooth;\
														break;\
												case KN_SMOOTHING:\
														ssmoother=&s_kn_smooth;\
														tsmoother=&t_kn_smooth;\
														break;\
												case WB_SMOOTHING:\
														ssmoother=&s_wb_smooth;\
														tsmoother=&t_wb_smooth;\
														break;\
												case GT_SMOOTHING:\
														ssmoother=&s_gt_smooth;\
														tsmoother=&t_gt_smooth;\
														break;\
												}
												



// string format_entry(cooc_type,global_score_type * gs, unsigned  =0);
void save_to_file(File,pcooc_vector_t * ,global_score_type * , unsigned,  unsigned =0,
        string ="",pindicator_vector_t * spairs=NULL);
void save_to_stream(ostream & ,pcooc_vector_t * ,global_score_type * , unsigned,  unsigned =0,
        string ="",pindicator_vector_t * spairs=NULL);
void psave_to_file(int,pcooc_vector_t * ,global_score_type * , unsigned, unsigned =0,
        string ="",pindicator_vector_t * spairs=NULL);

#endif	/* _OUT_H */

