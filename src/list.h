#include <stdint.h>

typedef int32_t list_data_t;

typedef enum list_status
{
	LIST_OK			= 0,
	LIST_ERR_ALLOC		= (1 << 1),
	LIST_ERR_EMPTY		= (2 << 1),
	LIST_ERR_NOTFOUND	= (3 << 1),
} list_status_t;

typedef struct list_elem
{
	list_data_t data;
	int next;
	int prev;
} list_elem_t;

typedef struct list
{
	list_elem_t* elements;
	int head;
	int tail;
	int free;
} list_t;

int list_insert_before(list_t* list, int index, list_data_t elem);
int list_insert_after(list_t* list, int index, list_data_t elem);
int list_insert_head(list_t* list, list_data_t elem);
int list_insert_tail(list_t* list, list_data_t elem);

int list_remove_at(list_t* list, int index);
int list_remove_head(list_t* list);
int list_remove_tail(list_t* list);

int list_chk(list_t* list);
int list_dump(list_t* list);
