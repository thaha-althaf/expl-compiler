//#include "abs.h"

struct gstnode *ghead = NULL;
int sp = 4096;
extern int yylineno;
//////////////////////////////////////////////////////////////////////////////////////////

int allocate(int size)
{
	int addr = sp;
	sp += size;
	return addr;
}
int getsp()
{
	return sp;
}

int k = -1;
int getreg()
{
	k++;
	return k;
}
void freereg()
{
	k--;
}

int label = 0;
int flabel = 1;
int setFLabel()
{
	return flabel++;
}

int getFLabel(char *name)
{
	struct gstnode *gst = ghead;
	while (gst)
	{
		if (strcmp(gst->name, name) == 0)
			if (gst->flabel != -1)
				return gst->flabel;
			else
			{
				printf("Error: Function \"%s\" declared as a variable\n", name);
				exit(1);
			}
		gst = gst->next;
	}
	printf("Error: Function \"%s\" not declared\n", name);
}

int getlabel()
{
	return label++;
}

//////////////////////////////////////////////////////////////////////////////////////////

struct paramnode *paraminstall(struct paramnode *phead, char *name, Type type, Nodetype typeofvar)
{
	struct paramnode *new_node = (struct paramnode *)malloc(sizeof(struct paramnode));
	new_node->name = name;
	new_node->type = type;
	new_node->typeofvar = typeofvar;
	new_node->next = NULL;
	if (phead == NULL)
		return new_node;
	struct paramnode *temp = phead;
	while (temp->next != NULL)
		temp = temp->next;
	temp->next = new_node;
	return phead;
}

struct paramnode *paramdelete(struct paramnode *phead)
{
	struct paramnode *temp = phead;
	while (temp != NULL)
	{
		phead = phead->next;
		free(temp);
		temp = phead;
	}
	return NULL;
}

struct paramnode *paramcopy(struct paramnode *phead)
{
	struct paramnode *new_head = NULL;
	struct paramnode *temp = phead;
	while (temp != NULL)
	{
		new_head = paraminstall(new_head, temp->name, temp->type, temp->typeofvar);
		temp = temp->next;
	}
	return new_head;
}

int paramgetcount(struct paramnode *phead)
{
	int count = 0;
	struct paramnode *temp = phead;
	while (temp != NULL)
	{
		count++;
		temp = temp->next;
	}
	return count;
}

int paramcheck(struct paramnode *phead1, struct paramnode *phead2)
{
	if (paramgetcount(phead1) != paramgetcount(phead2))
		return 0;
	struct paramnode *temp1 = phead1;
	struct paramnode *temp2 = phead2;
	while (temp1 != NULL && temp2 != NULL)
	{
		if (temp1->type != temp2->type || temp1->typeofvar != temp2->typeofvar)
			return 0;
		temp1 = temp1->next;
		temp2 = temp2->next;
	}
	if (temp1 == NULL && temp2 == NULL)
		return 1;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////

struct lstnode *lstinitnode(char *name, Type type, int binding)
{
	struct lstnode *new_node = (struct lstnode *)malloc(sizeof(struct lstnode));
	new_node->name = name;
	new_node->type = type;
	new_node->binding = binding;
	new_node->next = NULL;
	return new_node;
}

struct lsttable *lstinittable()
{
	struct lsttable *new_table = (struct lsttable *)malloc(sizeof(struct lsttable));
	new_table->head = NULL;
	new_table->tail = NULL;
	new_table->size = 0;
	return new_table;
}

struct lstnode *lstlookup(struct lsttable *table, char *name)
{
	struct lstnode *temp = table->head;
	while (temp)
	{
		if (strcmp(temp->name, name) == 0)
			return temp;
		temp = temp->next;
	}
	return NULL;
}

struct lsttable *lstinstall(struct lsttable *table, char *name, Type type)
{
	struct lstnode *new_node = lstlookup(table, name);
	if (new_node != NULL)
	{
		printf("Error: Variable \"%s\" redeclared (lst)\n", name);
		exit(1);
	}
	table->size++;
	if (table->head == NULL)
	{
		table->head = table->tail = lstinitnode(name, type, table->size);
	}
	else
	{
		table->tail->next = lstinitnode(name, type, table->size);
		table->tail = table->tail->next;
	}
	return table;
}

struct lsttable *lstdelete(struct lsttable *table)
{
	struct lstnode *temp = table->head;
	while (temp)
	{
		struct lstnode *temp2 = temp;
		temp = temp->next;
		free(temp2);
	}
	table = lstinittable();
	return table;
}

struct lsttable *lstcopy(struct lsttable *table)
{
	struct lsttable *new_table = lstinittable();
	struct lstnode *temp = table->head;
	while (temp)
	{
		if (new_table->head == NULL)
		{
			new_table->head = new_table->tail = lstinitnode(temp->name, temp->type, temp->binding);
		}
		else
		{
			new_table->tail->next = lstinitnode(temp->name, temp->type, temp->binding);
			new_table->tail = new_table->tail->next;
		}
		temp = temp->next;
	}
	new_table->size = table->size;
	return new_table;
}

void lstchangetype(struct lsttable *table, struct tnode *root, Type type)
{
	struct lstnode *temp = lstlookup(table, root->name);
	if (temp == NULL)
	{
		printf("Error: Variable \"%s\" undeclared (lst)\n", root->name);
		exit(1);
	}
	temp->type = type;
}

//////////////////////////////////////////////////////////////////////////////////////////

struct gstnode *gstlookup(char *name)
{
	struct gstnode *temp = ghead;
	while (temp != NULL)
	{
		if (strcmp(temp->name, name) == 0)
			return temp;
		temp = temp->next;
	}
	return NULL;
}

struct gstnode *gstinstall(char *name, Type type, int size, Nodetype typeofvar, struct paramnode *phead)
{
	struct gstnode *new_node = gstlookup(name);
	if (new_node != NULL)
	{
		printf("Variable \"%s\" already declared\n", name);
		exit(1);
	}
	new_node = (struct gstnode *)malloc(sizeof(struct gstnode));
	new_node->name = name;
	new_node->type = type;
	new_node->size = size;
	new_node->typeofvar = typeofvar;
	if (typeofvar == FUNCTION)
	{
		new_node->binding = -1;
		if (strcmp(name, "main") == 0)
			new_node->flabel = 0;
		else
			new_node->flabel = setFLabel();
	}
	else
	{
		new_node->binding = allocate(size);
		new_node->flabel = -1;
	}
	new_node->phead = phead;
	if (ghead == NULL)
	{
		ghead = new_node;
	}
	else
	{
		struct gstnode *temp = ghead;
		while (temp->next != NULL)
			temp = temp->next;
		temp->next = new_node;
	}
	return ghead;
}

void gstchangetype(struct tnode *root, Type type)
{
	struct gstnode *temp = gstlookup(root->name);
	if (temp == NULL)
	{
		printf("Variable \"%s\" not declared\n", root->name);
		exit(1);
	}
	temp->type = type;
}

//////////////////////////////////////////////////////////////////////////////////////////

struct tnode *leafvar(char *name, char *s)
{
	struct tnode *temp;
	temp = (struct tnode *)malloc(sizeof(struct tnode));
	temp->s = (char *)malloc(sizeof(char) * strlen(s));
	temp->s = strdup(s);
	temp->nodetype = VARIABLE;
	temp->name = malloc(sizeof(char) * strlen(name));
	temp->name = strdup(name);
	temp->gstentry = gstlookup(name);
	if (temp->gstentry)
		temp->type = temp->gstentry->type;
	temp->left = NULL;
	temp->right = NULL;
	temp->mid = NULL;
	return temp;
}

struct tnode *leafnum(Type type, int n, char *s)
{
	struct tnode *temp;
	temp = (struct tnode *)malloc(sizeof(struct tnode));
	temp->s = (char *)malloc(sizeof(char) * strlen(s));
	temp->s = strdup(s);
	temp->nodetype = CONSTANT;
	temp->val = n;
	temp->type = type;
	temp->gstentry = NULL;
	temp->left = NULL;
	temp->right = NULL;
	temp->mid = NULL;
	temp->name = NULL;
	return temp;
}

struct tnode *leafarray(char *name, struct tnode *l, char *s)
{
	struct tnode *temp = (struct tnode *)malloc(sizeof(struct tnode *));
	temp->s = (char *)malloc(sizeof(char) * strlen(s));
	temp->s = strdup(s);
	temp->nodetype = ARRAY;
	temp->name = (char *)malloc(sizeof(char) * strlen(name));
	temp->name = strdup(name);
	temp->gstentry = gstlookup(name);
	temp->type = temp->gstentry->type;
	temp->left = l;
	temp->right = NULL;
	temp->mid = NULL;
	return temp;
}

struct tnode *makenode(Nodetype nodetype, Type type, struct tnode *l, struct tnode *m, struct tnode *r, struct gstnode *gst, char *s)
{
	struct tnode *temp;
	temp = (struct tnode *)malloc(sizeof(struct tnode));
	if (nodetype == OPERATOR && type == INTEGER)
	{
		if (l->type != INTEGER || r->type != INTEGER)
		{
			printf("ERROR line :%d %s type mismatch 1\n", yylineno, s);
			exit(1);
		}
	}

	if (nodetype == WHILE || nodetype == IF)
	{
		if (l->type != BOOLEAN)
		{
			printf("ERROR : type mismatch 2 \n");
			printf("%s", s);
			exit(1);
		}
	}
	temp->s = (char *)malloc(sizeof(char) * strlen(s));
	temp->s = strdup(s);

	temp->type = type;
	temp->nodetype = nodetype;
	temp->left = l;
	temp->mid = m;
	temp->right = r;
	temp->gstentry = gst;
	temp->name = (char *)NULL;
	return temp;
}

struct tnode *tnodechangetypegst(struct tnode *root, Type type)
{
	if (root != NULL)
	{
		root->left = tnodechangetypegst(root->left, type);
		root->right = tnodechangetypegst(root->right, type);
		if (root->nodetype == VARIABLE)
		{
			gstchangetype(root, type);
		}
		root = NULL;
		free(root);
	}
}

struct tnode *tnodechangetypelst(struct lsttable *table, struct tnode *root, Type type)
{
	if (root != NULL)
	{
		root->left = tnodechangetypelst(table, root->left, type);
		root->right = tnodechangetypelst(table, root->right, type);
		if (root->nodetype == VARIABLE)
		{
			lstchangetype(table, root, type);
		}
		root = NULL;
		free(root);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

struct tnode *tnodeargappend(struct tnode *head, struct tnode *arg)
{
	if (head == NULL)
		return arg;
	struct tnode *curr = head;
	while (curr->nextarg)
		curr = curr->nextarg;
	curr->nextarg = arg;
	return head;
}

int checktnodeparam(struct paramnode *phead, struct tnode *Ahead)
{
	struct paramnode *curr_param = phead;
	struct tnode *curr_arg = Ahead;
	while (curr_param && curr_arg)
	{
		if (curr_param->type != curr_arg->type)
			return 0;
		curr_param = curr_param->next;
		curr_arg = curr_arg->nextarg;
	}
	if (curr_param || curr_arg)
		return 0;
	return 1;
}

struct tnode *paramtoarg(struct paramnode *phead)
{
	struct tnode *head = NULL;
	struct paramnode *curr = phead;
	while (curr)
	{
		struct tnode *new_node = makenode(VARIABLE, curr->type, NULL, NULL, NULL, NULL, curr->name);
		head = tnodeargappend(head, new_node);
		curr = curr->next;
	}
	return head;
}

struct paramnode *argtoparam(struct tnode *Ahead)
{
	struct paramnode *phead = NULL;
	struct tnode *curr = Ahead;
	while (curr)
	{
		phead = paraminstall(phead, curr->name, curr->type, curr->nodetype);
		curr = curr->nextarg;
	}
	return phead;
}

void argdelete(struct tnode *head)
{
	if (head == NULL)
		return;
	argdelete(head->nextarg);
	free(head);
}

//////////////////////////////////////////////////////////////////////////////////////////

int inwhile = 0;
int labelIn[100], labelOut[100];

int codegen(struct tnode *t, struct lsttable *lst, FILE *target)
{
	int p, q, loc;
	if (t->nodetype == CONSTANT)
	{
		if (t->type == INTEGER)
		{
			p = getreg();
			fprintf(target, "MOV R%d, %d\n", p, t->val);
			return p;
		}
		else if (t->type == STRING)
		{
			p = getreg();
			fprintf(target, "MOV R%d, %s\n", p, t->s);
			return p;
		}
	}
	else if (t->nodetype == VARIABLE)
	{
		p = getreg();
		loc = getaddr(t, lst, target);
		fprintf(target, "MOV R%d, [R%d]\n", p, loc);
		freereg();
		return p;
	}
	else if (t->nodetype == READ)
	{
		p = getreg();
		if (t->left->nodetype == VARIABLE)
		{
			loc = getaddr(t->left, lst, target);
			fprintf(target, "MOV R%d, R%d\n", p, loc);
			freereg();
		}
		else if (t->left->nodetype == ARRAY)
		{
			loc = getarrayaddr(t->left, lst, target);
			fprintf(target, "MOV R%d, R%d\n", p, loc);
			freereg();
		}
		q = getreg();
		fprintf(target, "MOV R%d, \"Read\"\n", q);
		fprintf(target, "PUSH R%d\n", q);
		fprintf(target, "MOV R%d, -1\n", q);
		fprintf(target, "PUSH R%d\n", q);
		fprintf(target, "PUSH R%d\n", p);
		fprintf(target, "PUSH R%d\n", q);
		fprintf(target, "PUSH R%d\n", q);
		fprintf(target, "CALL 0\n");
		fprintf(target, "POP R%d\n", q);
		fprintf(target, "POP R%d\n", q);
		fprintf(target, "POP R%d\n", q);
		fprintf(target, "POP R%d\n", q);
		fprintf(target, "POP R%d\n", q);
		fprintf(target, "BRKP\n");
		freereg();
		freereg();
		return -1;
	}
	else if (t->nodetype == WRITE)
	{
		p = codegen(t->left, lst, target);
		q = getreg();
		fprintf(target, "MOV R%d, \"Write\"\n", q);
		fprintf(target, "PUSH R%d\n", q);
		fprintf(target, "MOV R%d, -2\n", q);
		fprintf(target, "PUSH R%d\n", q);
		fprintf(target, "PUSH R%d\n", p);
		fprintf(target, "PUSH R%d\n", q);
		fprintf(target, "PUSH R%d\n", q);
		fprintf(target, "CALL 0\n");
		fprintf(target, "POP R%d\n", q);
		fprintf(target, "POP R%d\n", q);
		fprintf(target, "POP R%d\n", q);
		fprintf(target, "POP R%d\n", q);
		fprintf(target, "POP R%d\n", q);
		fprintf(target, "BRKP\n");
		freereg();
		freereg();
		return -1;
	}
	else if (t->nodetype == CONNECTOR)
	{
		p = codegen(t->left, lst, target);
		q = codegen(t->right, lst, target);
		return -1;
	}
	else if (t->nodetype == OPERATOR)
	{
		if (t->type == BOOLEAN)
		{
			p = codegen(t->left, lst, target);
			q = codegen(t->right, lst, target);
			if (strcmp(t->s, ">") == 0)
				fprintf(target, "GT R%d, R%d\n", p, q);
			else if (strcmp(t->s, "<") == 0)
				fprintf(target, "LT R%d, R%d\n", p, q);
			else if (strcmp(t->s, "<=") == 0)
				fprintf(target, "LE R%d, R%d\n", p, q);
			else if (strcmp(t->s, ">=") == 0)
				fprintf(target, "GE R%d, R%d\n", p, q);
			else if (strcmp(t->s, "==") == 0)
				fprintf(target, "EQ R%d, R%d\n", p, q);
			else if (strcmp(t->s, "!=") == 0)
				fprintf(target, "NE R%d, R%d\n", p, q);
			else if (strcmp(t->s, "&&") == 0)
				fprintf(target, "MUL R%d, R%d\n", p, q);
			else if (strcmp(t->s, "||") == 0)
				fprintf(target, "ADD R%d, R%d\n", p, q);
			freereg();
			return p;
		}
		if (t->s[0] == '=')
		{
			p = codegen(t->right, lst, target);
			if (t->left->nodetype == VARIABLE)
			{
				loc = getaddr(t->left, lst, target);
				fprintf(target, "MOV [R%d], R%d\n", loc, p);
				freereg();
			}
			else if (t->left->nodetype == ARRAY)
			{
				loc = getarrayaddr(t->left, lst, target);
				fprintf(target, "MOV [R%d], R%d\n", loc, p);
				freereg();
			}
			freereg();
			return -1;
		}
		else
		{
			p = codegen(t->left, lst, target);
			q = codegen(t->right, lst, target);
			if (t->s[0] == '+')
				fprintf(target, "ADD R%d, R%d\n", p, q);
			else if (t->s[0] == '-')
				fprintf(target, "SUB R%d, R%d\n", p, q);
			else if (t->s[0] == '*')
				fprintf(target, "MUL R%d, R%d\n", p, q);
			else if (t->s[0] == '/')
				fprintf(target, "DIV R%d, R%d\n", p, q);
			else if (t->s[0] == '%')
				fprintf(target, "MOD R%d, R%d\n", p, q);
			freereg();
			return p;
		}
	}
	else if (t->nodetype == WHILE)
	{
		int u, v;
		inwhile++;
		u = getlabel();
		v = getlabel();
		labelIn[u] = u;
		labelOut[v] = v;
		fprintf(target, "L%d:\n", u);
		p = codegen(t->left, lst, target);
		fprintf(target, "JZ R%d, L%d\n", p, v);
		freereg();
		p = codegen(t->right, lst, target);
		fprintf(target, "JMP L%d\n", u);
		fprintf(target, "L%d:\n", v);
		inwhile--;
		return -1;
	}
	else if (t->nodetype == IF)
	{
		int r, s;
		p = codegen(t->left, lst, target);
		s = getlabel();
		fprintf(target, "JZ R%d, L%d\n", p, s);
		freereg();
		p = codegen(t->mid, lst, target);
		if (t->right != NULL)
		{
			r = getlabel();
			fprintf(target, "JMP L%d\n", r);
			fprintf(target, "L%d:\n", s);
			p = codegen(t->right, lst, target);
			fprintf(target, "L%d:\n", r);
		}
		else
		{
			fprintf(target, "L%d:\n", s);
		}
		return -1;
	}
	else if (t->nodetype == BREAK)
	{
		if (inwhile)
			fprintf(target, "JMP L%d\n", labelOut[inwhile]);
		return -1;
	}
	else if (t->nodetype == CONTINUE)
	{
		if (inwhile)
			fprintf(target, "JMP L%d\n", labelIn[inwhile]);
		return -1;
	}
	else if (t->nodetype == REPEATUNTIL)
	{
		int u, v;
		u = getlabel();
		v = getlabel();
		labelIn[u] = u;
		labelOut[v] = v;
		fprintf(target, "L%d:\n", u);
		inwhile++;
		p = codegen(t->left, lst, target);
		inwhile--;
		p = codegen(t->right, lst, target);
		fprintf(target, "JZ R%d, L%d\n", p, u);
		fprintf(target, "L%d:\n", v);
		freereg();
		return p;
	}
	else if (t->nodetype == DOWHILE)
	{
		int u, v;
		u = getlabel();
		v = getlabel();
		labelIn[u] = u;
		labelOut[v] = v;
		fprintf(target, "L%d:\n", u);
		inwhile++;
		p = codegen(t->left, lst, target);
		inwhile--;
		p = codegen(t->right, lst, target);
		fprintf(target, "JNZ R%d, L%d\n", p, u);
		fprintf(target, "L%d:\n", v);
		freereg();
		return p;
	}
	else if (t->nodetype == ARRAY)
	{
		p = getreg();
		loc = getarrayaddr(t, lst, target);
		fprintf(target, "MOV R%d, [R%d]\n", p, loc);
		freereg();
		return p;
	}
	else if (t->nodetype == FUNCTION)
	{
		struct gstnode *curr = gstlookup(t->s);
		fprintf(target, "F%d:\n", curr->flabel);
		fprintf(target, "PUSH BP\n");
		fprintf(target, "MOV BP, SP\n");
		fprintf(target, "ADD SP, %d\n", lst->size - paramgetcount(argtoparam(t->right)));
		if (t->left)
			p = codegen(t->left, lst, target);
		return -1;
	}

	else if (t->nodetype == FUNCTIONCALL)
	{
		int regs = k;
		for (int i = 0; i <= regs; i++)
		{
			fprintf(target, "PUSH R%d\n", i);
			freereg();
		}
		int numArgs = 0;
		if (t->arglist)
			numArgs = pushargs(t->arglist, numArgs, lst, target);
		p = getreg();
		fprintf(target, "PUSH R%d\n", p);
		freereg();
		int f = getFLabel(t->name);
		fprintf(target, "CALL F%d\n", f);
		for (int i = regs; i >= 0; i--)
			getreg();
		p = getreg();
		fprintf(target, "POP R%d\n", p);
		q = getreg();
		for (int i = 0; i < numArgs; i++)
			fprintf(target, "POP R%d\n", q);
		freereg();
		for (int i = regs; i >= 0; i--)
		{
			fprintf(target, "POP R%d\n", i);
		}
		return p;
	}

	else if (t->nodetype == RET)
	{
		p = getreg();
		q = codegen(t->left, lst, target);
		fprintf(target, "MOV R%d, BP\n", p);
		fprintf(target, "SUB R%d, 2\n", p);
		fprintf(target, "MOV [R%d], R%d\n", p, q);
		freereg();
		freereg();
		fprintf(target, "MOV SP, BP\n");
		fprintf(target, "POP BP\n");
		fprintf(target, "RET\n");
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////

int getaddr(struct tnode *t, struct lsttable *lst, FILE *target)
{
	char *name = t->name;
	struct lstnode *l = lstlookup(lst, name);
	if (l != NULL)
	{
		int p = getreg();
		fprintf(target, "MOV R%d, BP\n", p);
		fprintf(target, "ADD R%d, %d\n", p, l->binding);
		return p;
	}
	else
	{
		struct gstnode *g = gstlookup(name);
		int p = getreg();
		fprintf(target, "MOV R%d, %d\n", p, g->binding);
		return p;
	}
}

int getarrayaddr(struct tnode *t, struct lsttable *lst, FILE *target)
{
	struct gstnode *g = gstlookup(t->name);
	int p = getreg();
	fprintf(target, "MOV R%d, %d\n", p, g->binding);
	int q = codegen(t->left, lst, target);
	fprintf(target, "ADD R%d, R%d\n", p, q);
	freereg();
	return p;
}

int pushargs(struct tnode *root, int numArgs, struct lsttable *lst, FILE *target)
{
	if (root)
	{
		numArgs++;
		numArgs = pushargs(root->nextarg, numArgs, lst, target);
		int p = codegen(root, lst, target);
		fprintf(target, "PUSH R%d\n", p);
		freereg();
	}
	return numArgs;
}

struct lsttable *lstparaminstall(struct lsttable *table, struct paramnode *phead)
{
	struct paramnode *curr = phead;
	int i = -3;
	while (curr)
	{
		table = lstinstall(table, curr->name, curr->type);
		table->tail->binding = i;
		i--;
		curr = curr->next;
	}
	return table;
}


/////////////////////////////////////////////////////////////////////////////
void LSTPrint(struct lsttable *table)
{
    struct lstnode *temp = table->head;
    printf("LST:\n");
    while (temp)
    {
        printf("%s\t", temp->name);
        if (temp->type == 0)
            printf("int\t");
        else if (temp->type == 2)
            printf("str\t");
        printf("%d\t", temp->binding);
        temp = temp->next;
        printf("\n");
    }
}


void GSTPrint()
{
    char *type;
    char *typeofvar;
    struct gstnode *temp = ghead;
    printf("%-12s\t%-5s\t%-5s\t%-10s\t%-7s\n", "Name", "Type", "Size", "TypeofVar", "Binding");
    while (temp != NULL)
    {
        if (temp->type == INTEGER)
        {
            type = (char *)malloc(sizeof(char) * 4);
            strcpy(type, "int");
        }
        else if (temp->type == STRING)
        {
            type = (char *)malloc(sizeof(char) * 4);
            strcpy(type, "str");
        }

        if (temp->typeofvar == ARRAY)
        {
            typeofvar = (char *)malloc(sizeof(char) * 13);
            strcpy(typeofvar, "array");
        }
        else if (temp->typeofvar == VARIABLE)
        {
            typeofvar = (char *)malloc(sizeof(char) * 13);
            strcpy(typeofvar, "variable");
        }
        else if (temp->typeofvar == FUNCTION)
        {
            typeofvar = (char *)malloc(sizeof(char) * 13);
            strcpy(typeofvar, "function");
        }

        printf("%-12s\t%-5s\t%-5d\t%-10s\t%-7d\n", temp->name, type, temp->size, typeofvar, temp->binding);
        temp = temp->next;
    }
}





void ASTPrint(struct tnode *root)
{
    if (root->nodetype == VARIABLE)
    {
        printf("%s", root->name);
    }

    else if (root->nodetype == CONSTANT)
    {
        if (root->type == INTEGER)
            printf("%d", root->val);
        else if (root->type == STRING)
            printf("%s", root->s);
    }

    else if (root->nodetype == READ)
    {
        printf("read(");
        ASTPrint(root->left);
        printf(")");
    }

    else if (root->nodetype == WRITE)
    {
        printf("write(");
        ASTPrint(root->left);
        printf(")");
    }

    else if (root->nodetype == CONNECTOR)
    {
        ASTPrint(root->left);
        printf(";\n");
        ASTPrint(root->right);
    }

    else if (root->nodetype == OPERATOR)
    {
        if (root->type == BOOLEAN)
        {
            ASTPrint(root->left);
            printf("%s", root->s);
            ASTPrint(root->right);
        }
        else
        {
            ASTPrint(root->left);
            printf("%s", root->s);
            ASTPrint(root->right);
        }
    }

    else if (root->nodetype == WHILE)
    {
        printf("while(");
        ASTPrint(root->left);
        printf(")\n");
        ASTPrint(root->right);
    }

    else if (root->nodetype == IF)
    {
        printf("if(");
        ASTPrint(root->left);
        printf(")\n");
        ASTPrint(root->mid);
        printf(";\n");
        if (root->right != NULL)
        {
            printf("else\n");
            ASTPrint(root->right);
        }
    }

    else if (root->nodetype == ARRAY)
    {
        printf("%s[", root->name);
        ASTPrint(root->left);
        printf("]");
    }

    else if (root->nodetype == FUNCTION)
    {
        printf("%s(", root->gstentry->name);
        struct tnode *curr = root->right;
        while (curr)
        {
            printf("%s", curr->s);
            curr = curr->nextarg;
            if (curr)
                printf(", ");
        }
        if (root->left != NULL)
        {
            printf(") {\n");
            ASTPrint(root->left);
            printf("}\n");
        }
        else
        {
            struct tnode *curr = root->arglist;
            while (curr)
            {
                printf("%s", curr->s);
                curr = curr->nextarg;
                if (curr)
                    printf(", ");
            }
            printf(")");
        }
    }

    else if (root->nodetype == RET)
    {
        printf("return ");
        ASTPrint(root->left);
        printf(";\n");
    }
}






void ParamPrint(struct paramnode *Phead)
{
    struct paramnode *temp = Phead;
    while (temp != NULL)
    {
        printf("%s\t", temp->name);
        if (temp->type == 0)
            printf("int\t");
        else if (temp->type == 2)
            printf("str\t");
        temp = temp->next;
    }
    printf("\n");
}
