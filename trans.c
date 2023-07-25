struct table
{
	char *label;
	int addr;
	struct table *next;
} *head, *tail;

void add(char *label, int addr)
{
	struct table *node = (struct table *)malloc(sizeof(struct table));
	node->label = label;
	node->addr = addr;
	node->next = NULL;
	if (head == NULL)
	{
		head = node;
		tail = node;
	}
	else
	{
		tail->next = node;
		tail = node;
	}
}
int getAddr(char *label)
{
	struct table *temp = head;
	while (temp != NULL)
	{
		if (strcmp(temp->label, label) == 0)
			return temp->addr;
		temp = temp->next;
	}
	return -1;
}
