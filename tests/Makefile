#---------------------------------------------------------------------------
# Author: Cedric Adjih, Hana Baccouch
#---------------------------------------------------------------------------
# Copyright 2015-2017 Inria
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#---------------------------------------------------------------------------

-include Makefile.local

#--------------------------------------------------

CFLAGS += -Wall -g -fPIC

#------------------------------

SRCS = swif_api_encoder_test.c swif_rlc_api_encoder_test.c

OBJS = $(SRCS:.c=.o)

#---------------------------------------------------------------------------
# Main targets
#---------------------------------------------------------------------------

all: swif_api_encoder_test swif_rlc_api_encoder_test

swif_api_encoder_test: swif_api_encoder_test.o ../src/libswif.a 
	${CC} ${CFLAGS} -o $@ $< -lswif -lcunit -lm -L. -L../src/

swif_rlc_api_encoder_test: swif_rlc_api_encoder_test.o ../src/libswif.a 
	${CC} ${CFLAGS} -o $@ $< -lswif -lcunit -L. -L../src/

#---------------------------------------------------------------------------
# Dependencies
# http://stackoverflow.com/questions/97338/gcc-dependency-generation-for-a-different-output-directory
#---------------------------------------------------------------------------

DEPS = $(SRCS:.c=.d)

-include $(DEPS)

%.d : %.c %.h
	$(CC) $(CCFLAGS) -MF"$@" -MG -MM -MP -MT"$@" -MT"$(<:.c=.o)" "$<"

#---------------------------------------------------------------------------
# Cleaning
#---------------------------------------------------------------------------

clean:
	rm -f *.a *.so *.o *.d *~

really-clean: clean
	rm -f swif_api_encoder_test
	rm -f swif_rlc_api_encoder_test
#---------------------------------------------------------------------------
