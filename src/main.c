#include <stdio.h>
#include <stdlib.h>

#include "list.h"

int main()
{
	list_t list = { 0 };
	list_ctor(&list, 14);

	list_insert_tail(&list, 1);
	list_insert_tail(&list, 2);
	list_insert_tail(&list, 3);

	list_insert_before(&list, 3, 22);
	list_insert_after(&list, 3, 55);

	list_remove_tail(&list);
	list_remove_head(&list);

	for(int i = 10; i < 15; i++)
	{
		list_insert_tail(&list, i);
	}

	list_dump(&list);

	list_dtor(&list);
	return 0;
}
