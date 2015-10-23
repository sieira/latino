#include <stdio.h>

#include "latino.h"
#include "ast.h"
#include "utils.h"

char *strdup0(char *s)
{
    size_t len = strlen(s);
    char *p;
	//FIX
    p = (char *)malloc(len + 1);
    if (p) {
        strncpy(p, s, len);
    }
    p[len] = '\0';
    return p;
}

/*
char *strndup0(const char *s, size_t n)
{
    size_t i;
    const char *p = s;
    char *new;
    for (i = 0; i < n && *p; i++, p++)
        ;
    new = (char *) malloc(i + 1);
    if (new) {
        memcpy(new, s, i);
        new[i] = '\0';
    }
    return new;
}
*/

/*FIXME: For hexadecimal*/
char * parse_string(const char *s, size_t n){
	//FIX
	char *ret = malloc(sizeof(n) + 1);
	int j = 0;
	for (size_t i = 0; i < n; i++)
	{
		int c;
		switch (s[i])
		{
		case '\\':
		{
			switch (s[i+1])
			{
			case 'a': c = '\n'; i++; goto save;
			case 'b': c = '\n'; i++; goto save;
			case 'f': c = '\n'; i++; goto save;
			case 'n': c = '\n'; i++; goto save;
			case 'r': c = '\n'; i++; goto save;
			case 't': c = '\t'; i++; goto save;
			case 'v': c = '\n'; i++; goto save;
			default: break;
			}
		}break;
		default:
			c = s[i];
			break;
		}		
	save:
		ret[j] = c;
		j++;
	}
	ret[j] = '\0';
	return ret;
}

list_node *make_list_node(void *d)
{
	//FIX
	list_node *ret = (list_node *)malloc(sizeof(list_node));
	ret->data = d;
	return ret;
}

list_node *make_list()
{
	//FIX
	list_node *start = (list_node *)malloc(sizeof(list_node));
	list_node *end = (list_node *)malloc(sizeof(list_node));
	start->prev = NULL;
	start->next = end;
	start->data = NULL;
	end->prev = start;
	end->next = NULL;
	end->data = NULL;
	return start;
}

int find_list(list_node *l, void *data)
{
	list_node *c;
	for (c = l; c->next != NULL; c = c->next) {
		if (c->data == data) {
			return 1;
		}
	}
	return 0;
}

void insert_list(list_node *l, void *data)
{
	list_node *ins = (list_node *)malloc(sizeof(list_node));
	ins->data = data;
	ins->next = l->next;
	l->next = ins;
	ins->next->prev = ins;
	ins->prev = l;
	/*if (l) {
		printf("length_list=%i\n", length_list(l));
	}*/
}

/*
void remove_list(list_node *l, void *data)
{
	list_node *c;
	for (c = l; c->next != NULL; c = c->next) {
		if (c->data == data) {
			c->prev->next = c->next;
			c->next->prev = c->prev;
		}
	}
}
*/

int length_list(list_node *l)
{
	int a = 0;
	list_node *c;
	for (c = l; c->next != NULL; c = c->next) {
		if (c->data != NULL) {
			a++;
		}
	}
	return a;
}

hash_map *make_hash_map()
{
	//FIX
	hash_map *ret = (hash_map *)malloc(sizeof(hash_map));
	int c;
	for (c = 0; c < 256; c++) {
		ret->buckets[c] = NULL;
	}
	return ret;
}

int hash(char *key)
{
	int h = 5381;

	unsigned char c;
	for (c = *key; c != '\0'; c = *++key)
		h = h * 33 + c;

	return abs(h % 256);
}

void *get_hash(hash_map *m, char *key)
{
	list_node *cur = m->buckets[hash(key)];
	if (cur == NULL) return NULL;
	for (; cur->next != NULL; cur = cur->next) {
		if (cur->data != NULL) {
			if (strcmp(key, ((hash_val *)cur->data)->key) == 0) {
				return ((hash_val *)cur->data)->val;
			}
		}
	}
	return NULL;
}

void set_hash(hash_map *m, char *key, void *val)
{
	//FIX
	hash_val *hv = (hash_val *)malloc(sizeof(hash_val));
	strcpy(hv->key, key);
	hv->val = val;
	int hk = hash(key);
	if (m->buckets[hk] == NULL) {
		m->buckets[hk] = make_list();
	}
	else {
		list_node *c;
		for (c = m->buckets[hk]; c->next != NULL; c = c->next) {
			if (c->data != NULL) {
				if (strcmp(((hash_val *)c->data)->key, key) == 0) {					
					free(c->data);
					c->data = (void *)hv;
					/*FIX: memory leak*/
					//free(hv);
					return;
				}
			}
		}
	}
	insert_list(m->buckets[hk], (void *)hv);
}

/*
hash_map *copy_hash(hash_map *m)
{
	hash_map *ret = make_hash_map();
	int i;
	for (i = 0; i < 256; i++) {
		list_node *c = m->buckets[i];
		if (c != NULL) {
			for (; c->next != NULL; c = c->next) {
				if (c->data != NULL) {
					set_hash(ret, ((hash_val *)c->data)->key, ((hash_val *)c->data)->val);
				}
			}
		}
	}
	return ret;
}
*/