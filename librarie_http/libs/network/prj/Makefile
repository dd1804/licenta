IDIR=libs/network/include
IINTDIR=libs/network/src/internal/include
SRCDIR=libs/network/src
SRCINTDIR=libs/network/src/internal/src
OUTROOT= build
CC=clang++

#---------------------------------------------------------------------
# configs
#---------------------------------------------------------------------
ifdef DEBUG
NAME=libnetwork_d.a
OUTDIR=$(OUTROOT)/debug/libs
CFLAGS=-g -std=c++17 -I$(IDIR) -I$(IINTDIR) -fstandalone-debug -DDEBUG
else
ifdef RELEASE
NAME=libnetwork.a
OUTDIR=$(OUTROOT)/release/libs
CFLAGS=-std=c++17 -I$(IDIR) -I$(IINTDIR)
else
$(error no build type defined)
endif
endif
#---------------------------------------------------------------------

ODIR=$(OUTDIR)/network/obj

_DEPS = client.h server.h http_request.h http_response.h thread_pool.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = client.o server.o thread_pool.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_INTDEPS = client_impl.h server_impl.h url.h tcp_connection.h utils.h
INTDEPS = $(patsubst %,$(IINTDIR)/%,$(_INTDEPS))

_INTOBJ = client_impl.o server_impl.o url.o tcp_connection.o
INTOBJ = $(patsubst %,$(ODIR)/%,$(_INTOBJ))


$(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS) $(INTDEPS)
	$(CC) $< -c -o $@ $(CFLAGS)

$(ODIR)/%.o: $(SRCINTDIR)/%.cpp $(DEPS) $(INTDEPS)
	$(CC) $< -c -o $@ $(CFLAGS)

all: directories build

build: $(OBJ) $(INTOBJ)
	ar rcs $(OUTDIR)/$(NAME) $^

MKDIR_P = mkdir -p

.PHONY: directories

directories: ${OUTDIR} ${ODIR}

${OUTDIR}:
	${MKDIR_P} ${OUTDIR}

${ODIR}:
	${MKDIR_P} ${ODIR}

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o $(OUTDIR)/$(NAME) *.gch 