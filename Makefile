#***************************************************************************
#    Makefile
#    Part of fscorer	: a parallel phrase scoring tool 
#                         for extra large corpora
#    copyright            : (C) 2012 by Mohammed Mediani
#    email                : mohammed.mediani@kit.edu
#***************************************************************************/

#***************************************************************************
 #   This library is free software; you can redistribute it and/or modify  *
 #   it  under the terms of the GNU Lesser General Public License version  *
 #   2.1 as published by the Free Software Foundation.                     *
 #                                                                         *
 #   This library is distributed in the hope that it will be useful, but   *
 #   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 #   Lesser General Public License for more details.                       *
 #                                                                         *
 #   You should have received a copy of the GNU Lesser General Public      *
 #   License along with this library; if not, write to the Free Software   *
 #   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
 #   USA                                                                   *
 #**************************************************************************/

## Here we include the STXXL compile environment variables
include /project/mt/user/mmediani/tools/stxxl_trunk/pmstxxl.mk
STXXL_PATH= /project/mt/user/mmediani/tools/stxxl_trunk
###############################################################################


OPENMP_OPTIONS ?= -fopenmp
COMPILER_OPTIONS =  $(STXXL_CPPFLAGS)
COMPILER_OPTIONS += $(STXXL_CPPFLAGS_PARALLEL_MODE)
COMPILER_OPTIONS += $(OPENMP_OPTIONS)
COMPILER_OPTIONS += -I include
COMPILER_OPTIONS += -std=c++0x
COMPILER_OPTIONS += -O3  -DUSE_GZIP -DMAX_KEY_LENGTH=250


LINKER_OPTIONS =  $(STXXL_LDLIBS)
LINKER_OPTIONS += $(OPENMP_OPTIONS)
LINKER_OPTIONS += -O3 -Xlinker -zmuldefs -lz

###############################################################################

.PHONY: all

all: fscore

###############################################################################

ONAMES_SCORE = 	score		\
		main		\
		strspec		\
		aligns		\
		anyoption	\
		processdir	\
		prepare_space	\
		load		\
		read_file_th	\
		ngrams		\
		out

mk_OFILES            = $(addsuffix .o,$(1))

OFILES_SCORE	     = $(call mk_OFILES,$(ONAMES_SCORE))


fscore: Makefile $(OFILES_SCORE)  ${STXXL_PATH}/lib/libstxxl.a
	${CXX} $(OFILES_SCORE) -o $@ ${LINKER_OPTIONS}

%.o: %.cpp Makefile 
	${CXX} -c -o $@ $< ${COMPILER_OPTIONS} 

###############################################################################

.PHONY: clean

RM = rm -f

DEP = makefile.dep

clean:
	$(RM) $(DEP)
	$(RM) *.o

.PHONY: distclean

distclean: clean
	$(RM) fscore
	
###############################################################################

FILES = $(wildcard *.cpp)

$(DEP): Makefile 
	@echo "Creating makefile.dep"
	@echo '# DO NOT EDIT THIS FILE -- it is automatically generated' > $@
	@echo '' >> $@

	@for file in $(FILES); \
	do \
		${CXX} -MM -c $$file ${COMPILER_OPTIONS}; \
		echo ' '; \
	done | \
	awk 'BEGIN {RS=" "} (! /^\//) {if($$0!="\\\n"){\
	        if($$0 ~ /:/) {\
	                sub(/\n/,"",$$0);\
		        printf "\n%s \\\n", $$0\
	        } else {\
	                if($$0 ~ /\n/){\
	                        print "",$$0\
	                } else {\
	                        print "",$$0,"\\"\
	                }\
	        }\
	}}' - >> $@

-include $(DEP)

###############################################################################

HEADERS  = $(wildcard *.h)

%.hct.cpp:
	echo '#include "$*.h"' > $@

header-compile-test: $(HEADERS:.h=.hct.o)

header-compile-test-clean:
	$(RM) *.hct.*