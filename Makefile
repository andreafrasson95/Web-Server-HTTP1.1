
COMP=gcc
DIR:=bin
OBJS := $(addprefix $(DIR)/,webserver.o)
EXEC =$(addprefix $(DIR)/,webserver)

all: $(EXEC)

$(EXEC) : $(OBJS)
	$(COMP) -o $@ $^

$(DIR)/%.o : %.c
	$(COMP) -c -o $@ $<

$(OBJS): | $(DIR)

$(DIR):
	mkdir $(DIR)

clean:
	rm $(EXEC) $(OBJS)
