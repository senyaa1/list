#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "list.h"
#include "stack.h"

static list_status_t list_increase_alloc(list_t* list)
{
	list_elem_t* new_alloc = (list_elem_t*)realloc(list->elements, list->size * 2 * sizeof(list_elem_t));

	if(!new_alloc) return LIST_ERR_ALLOC;

	list->elements = new_alloc;
	list->size *= 2;

	return LIST_OK;
}

static list_status_t list_maybe_increase_alloc(list_t* list)
{
	if(list->cnt < list->size - 1) return LIST_OK;
	
	return list_increase_alloc(list);
}


list_status_t list_ctor(list_t* list, size_t initial_size)
{
	if(!list) return  LIST_ERR_ARGNULL;
	if(list->elements) return LIST_ERR_ALLOC;

	list->size = initial_size;
	list->elements = (list_elem_t*)calloc(list->size + 1, sizeof(list_elem_t));
	list->cnt = 0;
	list->free = (stack_t){0};
	STACK_INIT(&list->free, sizeof(int), initial_size);

	for(int i = initial_size - 1; i > 0; i--)
	{
		stack_push(&list->free, &i);
		list->elements[i].next = 0;
		list->elements[i].prev = 0;
	}

	list->elements[0].data = 0xbabecafe;
	list->elements[0].next = 0;
	list->elements[0].prev = 0;

	list->elements[0].used = 1;

	return LIST_OK;
}

list_status_t list_dtor(list_t* list)
{
	if(!list) return LIST_ERR_ARGNULL;

	stack_dtor(&list->free);

	free(list->elements);
	memset(list, 0, sizeof(list_t));

	return LIST_OK;
}

list_elem_t* list_next(list_t* list, list_elem_t* elem)
{
	if(elem->next == 0)
		return 0;

	return &list->elements[elem->next];
}

list_elem_t* list_prev(list_t* list, list_elem_t* elem)
{
	if(elem->prev == 0)
		return 0;

	return &list->elements[elem->prev];
}

list_elem_t* list_begin(list_t* list)
{
	return &list->elements[list->elements[0].next];
}

list_elem_t* list_end(list_t* list)
{
	return &list->elements[list->elements[0].prev];
}

list_status_t list_chk(list_t* list)
{
	
}

list_status_t list_dump(list_t* list)
{
	FILE *dot_file = fopen(GRAPH_SRC_FILENAME, "w");

	fprintf(dot_file, "digraph G {\n");
	fprintf(dot_file, "node [shape=box];\n");
	fprintf(dot_file, "rankdir=LR;\n");

	for(int i = 0; i <= list->size; i++)
	{
		list_elem_t elem = list->elements[i];
		if(!elem.used) continue;

		fprintf(dot_file, "\"%d\";\n", elem.data);
	}

	// for(list_elem_t elem = *list_begin(list); list_next(list, &elem) != 0; elem = *list_next(list, &elem))
	for(int i = 0; i <= list->size; i++)
	{
		list_elem_t elem = list->elements[i];
		if(!elem.used) continue;

		if(list_next(list, &elem) != 0)
			fprintf(dot_file, "\"%d\" -> \"%d\"[color=green];\n", elem.data, list_next(list, &elem)->data);

		if(list_prev(list, &elem) != 0)
			fprintf(dot_file, "\"%d\" -> \"%d\"[color=red];\n", elem.data, list_prev(list, &elem)->data);
	}

	fprintf(dot_file, "}\n");
	fclose(dot_file);

	system("dot -Tpng " GRAPH_SRC_FILENAME " -o " GRAPH_FILENAME);

	FILE *log_file = fopen(LOG_FILENAME, "w");

	fprintf(log_file, "<pre>\n");
	fprintf(log_file, "List [0x%p]: \n", list);
	fprintf(log_file, "\telements: [0x%p]\n", list->elements);
	fprintf(log_file, "\tsize: %ld\n", list->size);
	fprintf(log_file, "\tcnt: %ld\n", list->cnt);
	fprintf(log_file, "</pre>\n");
	fprintf(log_file, "<img src=\"" GRAPH_FILENAME "\">");

	fclose(log_file);

	char* cmd_buf = (char*)calloc(1024, sizeof(char));
	char* cwd = (char*)calloc(512, sizeof(char));
	getcwd(cwd, 512);

	sprintf(cmd_buf, "$BROWSER file:///%s/%s", cwd, LOG_FILENAME);

	system(cmd_buf);

	free(cmd_buf);
	free(cwd);

	return LIST_OK;
}

list_status_t list_insert_before(list_t* list, int index, list_data_t data)
{
	if(list_maybe_increase_alloc(list)) 
		return LIST_ERR_ALLOC;

	int insertion_index = 0;
	stack_pop(&list->free, &insertion_index);

	list_elem_t* elem = &list->elements[insertion_index];
	elem->data = data;
	elem->used = true;

	list_elem_t* next = &list->elements[index];
	list_elem_t* prev = &list->elements[next->prev];

	elem->next = index;
	elem->prev = next->prev;

	next->prev = insertion_index;
	prev->next = insertion_index;

	list->cnt++;
	
	return LIST_OK;
}

list_status_t list_insert_after(list_t* list, int index, list_data_t data)
{
	if(list_maybe_increase_alloc(list)) 
		return LIST_ERR_ALLOC;

	int insertion_index = 0;
	stack_pop(&list->free, &insertion_index);

	list_elem_t* elem = &list->elements[insertion_index];
	elem->data = data;
	elem->used = true;

	list_elem_t* prev = &list->elements[index];
	list_elem_t* next = &list->elements[prev->next];

	elem->next = prev->next;
	elem->prev = index;

	prev->next = insertion_index;
	next->prev = insertion_index;

	list->cnt++;

	return LIST_OK;
}

list_status_t list_insert_head(list_t* list, list_data_t data)
{
	return list_insert_before(list, list->elements[0].next, data);
}

list_status_t list_insert_tail(list_t* list, list_data_t data)
{
	return list_insert_after(list, list->elements[0].prev, data);
}

list_status_t list_remove_at(list_t* list, int index)
{
	list_elem_t* elem = &list->elements[index];

	list->elements[elem->prev].next = elem->next;
	list->elements[elem->next].prev = elem->prev;

	memset(elem, 0, sizeof(list_elem_t));

	stack_push(&list->free, &index);

	list->cnt--;

	return LIST_OK;
}

list_status_t list_remove_head(list_t* list)
{
	return list_remove_at(list, list->elements[0].next);
}

list_status_t list_remove_tail(list_t* list)
{
	return list_remove_at(list, list->elements[0].prev);
}

