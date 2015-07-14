# ライフゲームコンピュータにおける論理ゲートのデモンストレーション

GTK_CFLAGS = $(shell pkg-config --cflags gtk+-2.0)
GTK_LIBS   = $(shell pkg-config --libs gtk+-2.0)
GTHREAD_CFLAGS = $(shell pkg-config --cflags gthread-2.0)
GTHREAD_LIBS   = $(shell pkg-config --libs gthread-2.0)

CC      := gcc
#CFLAGS  := -std=gnu99 -W -Wall -g -ggdb $(GTK_CFLAGS) $(GTHREAD_CFLAGS)
CFLAGS  := -std=gnu99 -W -Wall -O3 -msse2 -funroll-loops $(GTK_CFLAGS) $(GTHREAD_CFLAGS)
LDFLAGS :=  $(GTK_LIBS) $(GTHREAD_LIBS)
TARGET  := gameoflife
OBJECTS := main.o pattern.o

.SUFFIXES: .c .o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.h pattern.h
pattern.o: main.h pattern.h

clean:
	rm -f $(TARGET) $(OBJECTS)