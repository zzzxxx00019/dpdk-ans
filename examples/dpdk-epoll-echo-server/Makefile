ifeq ($(RTE_SDK),)
$(error "Please define RTE_SDK environment variable")
endif

ifeq ($(RTE_ANS),)
$(error "Please define RTE_ANS environment variable")
endif

ifeq ($(RTE_TARGET),)
$(error "Please define RTE_TARGET environment variable")
endif

CC = gcc
RM = rm -f

CFLAGS += -O3 \
	  -I$(RTE_ANS)/librte_ans/include \
          -I$(RTE_ANS)/librte_anssock/include

LDLIBS += $(RTE_ANS)/librte_anssock/librte_anssock.a \
          -L$(RTE_SDK)/$(RTE_TARGET)/lib \
          -Wl,--whole-archive -Wl,-lrte_mbuf -Wl,-lrte_mempool_ring -Wl,-lrte_mempool -Wl,-lrte_ring -Wl,-lrte_eal -Wl,-lrte_kvargs -Wl,--no-whole-archive -Wl,-export-dynamic \
          -lrt -pthread -ldl -lnuma

OBJS = epoll_echo_server.o 
TARGET = epoll_echo_server

$(TARGET):$(OBJS)                      
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS) $(LDLIBS)
        
$(OBJS):%.o:%.c          
	$(CC) -c $(CFLAGS) $< -o $@        
clean:                              
	-$(RM) $(TARGET) $(OBJS) 
