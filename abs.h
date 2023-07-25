#include <string.h>
#include <stdio.h>

typedef enum Nodetype
{
	VARIABLE,
	CONSTANT,
	ARRAY,
	READ,
	WRITE,
	CONNECTOR,
	OPERATOR,
	WHILE,
	DOWHILE,
	REPEATUNTIL,
	IF,
	BREAK,
	CONTINUE,
	FUNCTION,
	BODY,
	RET,
	FUNCTIONCALL
} Nodetype;

typedef enum Type
{
	INTEGER,
	BOOLEAN,
	STRING,
	VOID,
} Type;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct tnode
{
	int val;
	Type type;
	char *name;
	Nodetype nodetype;
	char *s;
	struct gstnode *gstentry;
	struct tnode *left, *right, *mid, *nextarg, *arglist;
} tnode;

struct tnode *leafvar(char *, char *);
struct tnode *leafnum(Type, int, char *);
struct tnode *leafarray(char *, struct tnode *, char *);
struct tnode *makenode(Nodetype, Type, struct tnode *, struct tnode *, struct tnode *, struct gstnode *, char *);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct gstnode
{
	char *s;
	int size;
	Type type;
	char *name;
	int flabel;
	int binding;
	Nodetype typeofvar;
	struct lsttable *lst;
	struct gstnode *next;
	struct paramnode *phead;
};
struct gstnode *gstlookup(char *);
void gstchangetype(struct tnode *, Type);
struct gstnode *gstinstall(char *, Type, int, Nodetype, struct paramnode *);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct lstnode
{
	char *name;
	Type type;
	int binding;
	struct lstnode *next;
};
struct lsttable
{
	struct lstnode *head, *tail;
	int size;
};

struct lsttable *lstinittable();
void lstprint(struct lsttable *);
struct lsttable *lstcopy(struct lsttable *);
struct lsttable *lstdelete(struct lsttable *);
struct lstnode *lstinitnode(char *, Type, int);
struct lstnode *lstookup(struct lsttable *, char *);
void lstchangetype(struct lsttable *, struct tnode *, Type);
struct lsttable *lstinstall(struct lsttable *, char *, Type);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct paramnode
{
	char *name;
	Type type;
	Nodetype typeofvar;
	struct paramnode *next;
};

struct paramnode *paraminstall(struct paramnode *, char *, Type, Nodetype);
struct paramnode *paramdelete(struct paramnode *);
struct paramnode *paramcopy(struct paramnode *);
int paramgetcount(struct paramnode *);
int paramcheck(struct paramnode *, struct paramnode *);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct tnode *argappend(struct tnode *, struct tnode *);
int checktnodeparam(struct paramnode *, struct tnode *);
struct tnode *paramtoarg(struct paramnode *);
struct paramnode *argtoparam(struct tnode *);
void argdelete(struct tnode *);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int codegen(struct tnode *, struct lsttable *, FILE *);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int getaddr(struct tnode *, struct lsttable *, FILE *);
int getarrayaddr(struct tnode *, struct lsttable *, FILE *);
int pushargs(struct tnode *, int, struct lsttable *, FILE *);
struct lsttable *lstparaminstall(struct lsttable *, struct paramnode *);
struct tnode *tnodechangetypegst(struct tnode *, Type);
struct tnode *tnodechangetypelst(struct lsttable *, struct tnode *, Type);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int getSP();
int getreg();
void freereg();
int getLabel();
int setFlabel();
int allocate(int);
int getFlabel(char name);


/////////////////////////////////////////////////////////////////////////

void ASTPrint(struct tnode *);
void GSTPrint();
void LSTPrint(struct lsttable *);
void ParamPrint(struct paramnode *);
