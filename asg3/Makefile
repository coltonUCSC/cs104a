# Colton Willey
# cwwilley@ucsc.edu
# 
# Jacob Janowski
# jnjanows@ucsc.edu

GPP      = g++ -std=gnu++14 -g -O0 -Wall -Wextra
MKDEP    = g++ -std=gnu++14 -MM

MKFILE   = Makefile
DEPFILE  = Makefile.dep
SOURCES  = string_set.cpp main.cpp lyutils.cpp auxlib.cpp yylex.cpp \
yyparse.cpp astree.cpp
HEADERS  = string_set.h lyutils.h yyparse.h
OBJECTS  = ${SOURCES:.cpp=.o} 
EXECBIN  = oc
SRCFILES = ${HEADERS} ${SOURCES} ${MKFILE}
LSOURCES = scanner.l
YSOURCES = parser.y
CLGEN = yylex.cpp
HYGEN = yyparse.h
CYGEN = yyparse.cpp
GEN = ${CLGEN} ${HYGEN} ${CYGEN}
LREPORT = yylex.output
YREPORT = yyparse.output

all : ${GEN} ${EXECBIN} 

${CLGEN} : ${LSOURCES}
	flex --outfile=${CLGEN} ${LSOURCES} 2>${LREPORT}

${CYGEN} ${HYGEN} : ${YSOURCES}
	bison --defines=${HYGEN} --output=${CYGEN} ${YSOURCES}

${EXECBIN} : ${OBJECTS} ${CLGEN}
	${GPP} ${OBJECTS} -o ${EXECBIN}

%.o : %.cpp
	${GPP} -c $<

ci :
	cid + ${SRCFILES}

clean :
	-rm ${OBJECTS} ${DEPFILE}

spotless : clean
	- rm ${EXECBIN} ${CYGEN} ${CLGEN} Listing*.ps Listing*.pdf \
	test.out HEADER* *.output list

#${DEPFILE} :
#	${MKDEP} ${SOURCES} >${DEPFILE}
#
#dep :
#	- rm ${DEPFILE}
#	${MAKE} --no-print-directory ${DEPFILE}
#
#include ${DEPFILE}

test : ${EXECBIN}
	${EXECBIN} * * * >test.out 2>&1

lis : test
	mkpspdf Listing.ps ${SRCFILES} ${DEPFILE} test.out

again : ${SRCFILES}
	make --no-print-directory spotless dep ci test lis
