# Makefile — build thư viện lkfx + ví dụ
#
# Build thư viện:   make
# Build example:    make example
# Dọn dẹp:         make clean

CC      = arm-rockchip830-linux-uclibcgnueabihf-gcc
CFLAGS  = -O2 -Wall -Wextra -I.
LDFLAGS = -lm

# Nguồn thư viện
LIB_SRC = lkfx_display.c lkfx_gfx.c lkfx_font.c lkfx_input.c lkfx_widget.c
LIB_OBJ = $(LIB_SRC:.c=.o)
LIB     = liblkfx.a

.PHONY: all example clean

all: $(LIB)

$(LIB): $(LIB_OBJ)
	ar rcs $@ $^
	@echo ">>> Built $(LIB)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build ví dụ
example: $(LIB) example.o
	$(CC) $(CFLAGS) -o example example.o -L. -llkfx $(LDFLAGS)
	@echo ">>> Built example"

# Sử dụng trong project của bạn:
# my_app: my_app.o $(LIB_OBJ)
#     $(CC) $(CFLAGS) -o my_app my_app.o $(LIB_OBJ) $(LDFLAGS)

clean:
	rm -f $(LIB_OBJ) $(LIB) example.o example
