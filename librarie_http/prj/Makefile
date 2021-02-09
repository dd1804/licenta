IDIR=include
INETWORKDIR=libs/network/include
SRCDIR=src
OUTROOT= build
CC=clang++

#---------------------------------------------------------------------
# configs
#---------------------------------------------------------------------
ifdef APPNAME
ifdef DEBUG
NAME=$(APPNAME)_debug.out
OUTDIR=$(OUTROOT)/debug
NETWORKLIB=network_d
CFLAGS=-g -std=c++17 -I$(IDIR) -I$(INETWORKDIR) -fstandalone-debug -DDEBUG
else
ifdef RELEASE
NAME=$(APPNAME)
OUTDIR=$(OUTROOT)/release
NETWORKLIB=network
CFLAGS=-std=c++17 -I$(IDIR) -I$(INETWORKDIR)
else
$(error no build type defined)
endif
endif
else
$(error no application name defined)
endif
#---------------------------------------------------------------------

LDIR=$(OUTDIR)/libs
ODIR=$(OUTDIR)/obj

LIBS=-pthread

_DEPS = log.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_NETWORKDEPS = client.h server.h http_request.h http_response.h thread_pool.h
NETWORKDEPS = $(patsubst %,$(INETWORKDIR)/%,$(_NETWORKDEPS))

_OBJ = main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS) $(NETWORKDEPS)
	$(CC) $< -c -o $@ $(CFLAGS)

all: directories build

build: $(OBJ)
	$(CC) -o $(OUTDIR)/$(NAME) $^ $(CFLAGS) $(LIBS) -L$(LDIR) -l$(NETWORKLIB)

MKDIR_P = mkdir -p

.PHONY: directories

directories: ${OUTDIR} ${ODIR}

${OUTDIR}:
	${MKDIR_P} ${OUTDIR}

${ODIR}:
	${MKDIR_P} ${ODIR}

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(OUTDIR)/$(NAME) *~ core $(INCDIR)/*~ 
