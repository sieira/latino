#include "vm.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "utils.h"

/*
#define lat_writestring(s,l)	fwrite((s), sizeof(char), (l), stdout)
#define lat_writeline()	(lat_writestring("\n", 1), fflush(stdout))
*/

lat_vm *lat_make_vm()
{
	//FIX
	lat_vm *ret = (lat_vm *)malloc(sizeof(lat_vm));
	ret->stack = make_list();
	ret->all_objects = make_list();
	memset(ret->regs, 0, 256);
	memset(ret->ctx_stack, 0, 256);
	ret->ctx_stack[0] = lat_instance(ret);
	ret->ctx_stack_pointer = 0;
	lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, "imprimir"), lat_define_c_function(ret, lat_print));
	lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, "+"), lat_define_c_function(ret, lat_add));
	/*lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, "-"), lat_define_c_function(ret, lat_sub));
	lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, "*"), lat_define_c_function(ret, lat_mul));
	lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, "/"), lat_define_c_function(ret, lat_div));
	lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, "%"), lat_define_c_function(ret, lat_mod));
	lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, "!="), lat_define_c_function(ret, lat_neq));
	lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, "=="), lat_define_c_function(ret, lat_eq));*/
	lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, "<"), lat_define_c_function(ret, lat_lt));
	/*lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, "<="), lat_define_c_function(ret, lat_lte));
	lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, ">"), lat_define_c_function(ret, lat_gt));
	lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, ">="), lat_define_c_function(ret, lat_gte));
	lat_set_ctx(lat_get_current_ctx(ret), lat_str(ret, "gc"), lat_define_c_function(ret, lat_gc));*/
	return ret;
}

void lat_push_stack(lat_vm *vm, lat_object *o)
{
	insert_list(vm->stack, (void *)o);
}

lat_object *lat_pop_stack(lat_vm *vm)
{
	list_node *n = vm->stack->next;
	if (n->data == NULL) {
		log_err("Stack is empty");
		exit(1);
	}
	else {
		n->prev->next = n->next;
		n->next->prev = n->prev;
		lat_object *ret = (lat_object *)n->data;		
		free(n);
		return ret;
	}
}

void lat_push_list(lat_object *list, lat_object *o)
{
	insert_list(list->data.list, (void *)o);
}

lat_object *lat_pop_list(lat_object *list)
{
	list_node *n = ((list_node *)list)->next;
	if (n->data == NULL) {
		log_err("List is empty");
		exit(1);
	}
	else {
		n->prev->next = n->next;
		n->next->prev = n->prev;
		lat_object *ret = (lat_object *)n->data;
		return ret;
	}
}

void lat_push_ctx(lat_vm *vm)
{
	if (vm->ctx_stack_pointer >= 255) {
		log_err("Namespace stack overflow");
		exit(1);
	}
	//vm->ctx_stack[vm->ctx_stack_pointer + 1] = lat_clone_object(vm, vm->ctx_stack[vm->ctx_stack_pointer]);
	//FIX
	vm->ctx_stack[vm->ctx_stack_pointer + 1] = vm->ctx_stack[vm->ctx_stack_pointer];
	vm->ctx_stack_pointer++;
}

void lat_pop_ctx(lat_vm *vm)
{
	if (vm->ctx_stack_pointer == 0) {
		log_err("Namespace stack underflow");
		exit(1);
	}
	lat_delete_object(vm, vm->ctx_stack[vm->ctx_stack_pointer--]);
}

void lat_push_predefined_ctx(lat_vm *vm, lat_object *ctx)
{
	if (vm->ctx_stack_pointer >= 255) {
		log_err("Namespace stack overflow");
		exit(1);
	}
	vm->ctx_stack[++vm->ctx_stack_pointer] = ctx;
}

lat_object *lat_pop_predefined_ctx(lat_vm *vm)
{
	if (vm->ctx_stack_pointer == 0) {
		log_err("Namespace stack underflow");
		exit(1);
	}
	return vm->ctx_stack[vm->ctx_stack_pointer--];
}

lat_object *lat_get_current_ctx(lat_vm *vm)
{
	return vm->ctx_stack[vm->ctx_stack_pointer];
}

void lat_gc_add_object(lat_vm *vm, lat_object *o)
{
	insert_list(vm->all_objects, (void *)o);
}

/*
void lat_gc(lat_vm *vm)
{
	for (int i = vm->ctx_stack_pointer; i >= 0; i--) {
		lat_mark_object(vm->ctx_stack[i], 1);
	}

	list_node *c;
	lat_object *cur;
	for (c = vm->all_objects->next; c != NULL; c = c->next) {
		if (c->data != NULL) {
			cur = (lat_object *)c->data;
			if (cur->marked == 0) {
				lat_delete_object(vm, cur);
				list_node *prev = c->prev;
				c->prev->next = c->next;
				c->next->prev = c->prev;
				free(c);
				c = prev;
			}
			else if (cur->marked == 1) {
				cur->marked = 0;
			}
		}
	}
	vm->regs[255] = lat_str(vm, "Trash compactor completing operation");
}
*/

lat_object *lat_define_function(lat_vm *vm, lat_bytecode *inslist)
{
	lat_object *ret = lat_func(vm);
	//FIX
	lat_function *fval = (lat_function *)malloc(sizeof(lat_function));
	fval->bcode = inslist;
	ret->data.func = fval;
	return ret;
}

lat_object *lat_define_c_function(lat_vm *vm, void(*function)(lat_vm *vm))
{
	lat_object *ret = lat_cfunc(vm);
	ret->data.cfunc = function;
	return ret;
}

void lat_nth_list(lat_vm *vm)
{
	lat_object *index = lat_pop_stack(vm);
	int i = lat_get_int_value(index);
	lat_object *list = lat_pop_stack(vm);
	list_node *l = list->data.list;
	int counter = 0;
	list_node *c;
	for (c = l->next; c->next != NULL; c = c->next) {
		if (c->data != NULL) {
			if (counter == i) {
				vm->regs[255] = (lat_object *)c->data;
				return;
			}
			counter++;
		}
	}
	log_err("Lista: indice fuera de rango");
	exit(1);
}

void lat_print_list(lat_vm *vm, list_node *l){
	fprintf(stdout, "%s", "[ ");
	if (l != NULL && length_list(l) > 0) {
		list_node *c;
		for (c = l->next; c != NULL; c = c->next) {
			if (c->data != NULL) {
				lat_object *o = ((lat_object *)c->data);
				printf("\ntype %i, obj_ref: %x\n", o->type, o);
				if (o->type == T_LIST){
					lat_print_list(vm, o->data.list);
					if (c->next->data){
						fprintf(stdout, "%s", ", ");
					}
				}
				else{
					if (o->type != NULL){
						lat_push_stack(vm, o);
						lat_print(vm);
						if (c->next->data){
							fprintf(stdout, "%s", ", ");
						}
					}
				}
			}
		}
	}
	fprintf(stdout, "%s", " ]");
}

void lat_print(lat_vm *vm)
{
	lat_object *in = lat_pop_stack(vm);
	if (in->type == T_NULL) {
		//fprintf(stdout, "%s\n", "NULO");
		fprintf(stdout, "%s", "NULO");
	}else if (in->type == T_INSTANCE) {
		//fprintf(stdout, "%s\n", "Objeto");
		fprintf(stdout, "%s", "Objeto");
	}else if (in->type == T_CHAR) {
		//fprintf(stdout, "%c\n", lat_get_char_value(in));
		fprintf(stdout, "%c", lat_get_char_value(in));
	}else if (in->type == T_INT) {
		//fprintf(stdout, "%d\n", lat_get_int_value(in));
		fprintf(stdout, "%d", lat_get_int_value(in));
	}else if (in->type == T_DOUBLE) {
		//fprintf(stdout, "%lf\n", lat_get_double_value(in));
		fprintf(stdout, "%lf", lat_get_double_value(in));
	}else if (in->type == T_STR) {
		//fprintf(stdout, "\"%s\"\n", lat_get_str_value(in));
		fprintf(stdout, "%s", lat_get_str_value(in));
	}else if (in->type == T_BOOL) {
		//fprintf(stdout, "%i\n", lat_get_bool_value(in));
		fprintf(stdout, "%i", lat_get_bool_value(in));
	}else if (in->type == T_LIST){
		//fprintf(stdout, "%s\n", "List");
		lat_print_list(vm, in->data.list);
		fprintf(stdout, "%s\n", "");
	}else if (in->type == T_FUNC) {
		//fprintf(stdout, "%s\n", "Funcion");
		fprintf(stdout, "%s", "Funcion");
	}else if (in->type == T_CFUNC) {
		//fprintf(stdout, "%s\n", "C_Funcion");
		fprintf(stdout, "%s", "C_Funcion");
	}else if (in->type == T_STRUCT) {
		//fprintf(stdout, "%s\n", "Objeto");
		fprintf(stdout, "%s", "Struct");
	}else {
		fprintf(stdout, "Tipo desconocido %d\n", in->type);
	}
	vm->regs[255] = in;
	//free(in);
}

void lat_clone(lat_vm *vm)
{
	lat_object *ns = lat_pop_stack(vm);
	vm->regs[255] = lat_clone_object(vm, ns);
}

void lat_cons(lat_vm *vm)
{
	lat_object *list = lat_pop_stack(vm);
	lat_object *elem = lat_pop_stack(vm);
	list_node *ret = make_list();
	insert_list(ret, (void *)elem);
	ret->next->next = list->data.list;
	list->data.list->prev = ret->next;
	vm->regs[255] = lat_list(vm, ret);
}

void lat_add(lat_vm *vm)
{
	lat_object *a = lat_pop_stack(vm);
	lat_object *b = lat_pop_stack(vm);
	if ((a->type != T_INT && a->type != T_DOUBLE) || (b->type != T_INT && b->type != T_DOUBLE)) {
		log_err("Intento de aplicar operador \"+\" en tipos invalidos");
		exit(1);
	}
	if (a->type == T_DOUBLE || b->type == T_DOUBLE) {
		//vm->regs[255] = lat_double(vm, lat_get_double_value(a) + lat_get_double_value(b));
		a->data.d = lat_get_double_value(a) + lat_get_double_value(b);
		vm->regs[255] = a;		
	}
	else {
		//vm->regs[255] = lat_int(vm, lat_get_int_value(a) + lat_get_int_value(b));
		a->data.i = lat_get_int_value(a) + lat_get_int_value(b);
		vm->regs[255] = a;		
	}
}

void lat_sub(lat_vm *vm)
{
	lat_object *a = lat_pop_stack(vm);
	lat_object *b = lat_pop_stack(vm);
	if ((a->type != T_INT && a->type != T_DOUBLE) || (b->type != T_INT && b->type != T_DOUBLE)) {
		log_err("Attempt to apply operator \"-\" on invalid types");
		exit(1);
	}
	if (a->type == T_DOUBLE || b->type == T_DOUBLE) {
		//vm->regs[255] = lat_double(vm, lat_get_double_value(a) - lat_get_double_value(b));
		a->data.d = lat_get_double_value(a) - lat_get_double_value(b);
		vm->regs[255] = a;		
	}
	else {
		//vm->regs[255] = lat_int(vm, lat_get_int_value(a) - lat_get_int_value(b));
		a->data.i = lat_get_int_value(a) - lat_get_int_value(b);
		vm->regs[255] = a;		
	}
}

void lat_mul(lat_vm *vm)
{
	lat_object *a = lat_pop_stack(vm);
	lat_object *b = lat_pop_stack(vm);
	if ((a->type != T_INT && a->type != T_DOUBLE) || (b->type != T_INT && b->type != T_DOUBLE)) {
		log_err("Attempt to apply operator \"*\" on invalid types");
		exit(1);
	}
	if (a->type == T_DOUBLE || b->type == T_DOUBLE) {
		//vm->regs[255] = lat_double(vm, lat_get_double_value(a) * lat_get_double_value(b));
		a->data.d = lat_get_double_value(a) * lat_get_double_value(b);
		vm->regs[255] = a;		
	}
	else {
		//vm->regs[255] = lat_int(vm, lat_get_int_value(a) * lat_get_int_value(b));
		a->data.i = lat_get_int_value(a) * lat_get_int_value(b);
		vm->regs[255] = a;		
	}
}

void lat_div(lat_vm *vm)
{
	lat_object *a = lat_pop_stack(vm);
	lat_object *b = lat_pop_stack(vm);
	if ((a->type != T_INT && a->type != T_DOUBLE) || (b->type != T_INT && b->type != T_DOUBLE)) {
		log_err("Attempt to apply operator \"\\\" on invalid types");
		exit(1);
	}
	if (a->type == T_DOUBLE || b->type == T_DOUBLE) {
		double tmp = lat_get_double_value(b);
		if (tmp == 0){
			log_err("Division por cero");
			exit(1);
		}
		else{
			//vm->regs[255] = lat_double(vm, lat_get_double_value(a) / tmp);
			a->data.d = lat_get_double_value(a) / tmp;
			vm->regs[255] = a;		
		}
	}
	else {
		int tmp = lat_get_int_value(b);
		if (tmp == 0){
			log_err("Division por cero");
			exit(1);
		}
		else{
			//vm->regs[255] = lat_int(vm, lat_get_int_value(a) / tmp);
			a->data.i = lat_get_int_value(a) / tmp;
			vm->regs[255] = a;		
		}
	}
}

void lat_mod(lat_vm *vm)
{
	lat_object *a = lat_pop_stack(vm);
	lat_object *b = lat_pop_stack(vm);
	if (a->type != T_INT || b->type != T_INT) {
		log_err("Attempt to apply operator \"%%\" on invalid types");
		exit(1);
	}
	int tmp = lat_get_int_value(b);
	if (tmp == 0){
		log_err("Modulo por cero");
		exit(1);
	}
	else{
		//vm->regs[255] = lat_int(vm, lat_get_int_value(a) % tmp);
		a->data.i = lat_get_int_value(a) % tmp;
		vm->regs[255] = a;		
	}
}

void lat_neq(lat_vm *vm)
{
	lat_object *a = lat_pop_stack(vm);
	lat_object *b = lat_pop_stack(vm);
	if ((a->type != T_INT && a->type != T_DOUBLE) || (b->type != T_INT && b->type != T_DOUBLE)) {
		log_err("Attempt to apply operator \"==\" on invalid types");
		exit(1);
	}
	if (a->type == T_DOUBLE || b->type == T_DOUBLE) {
		vm->regs[255] = lat_bool(vm, lat_get_double_value(a) != lat_get_double_value(b));
	}
	else {
		vm->regs[255] = lat_bool(vm, lat_get_int_value(a) != lat_get_int_value(b));
	}
}

void lat_eq(lat_vm *vm)
{
	lat_object *a = lat_pop_stack(vm);
	lat_object *b = lat_pop_stack(vm);
	if ((a->type != T_INT && a->type != T_DOUBLE) || (b->type != T_INT && b->type != T_DOUBLE)) {
		log_err("Attempt to apply operator \"==\" on invalid types");
		exit(1);
	}
	if (a->type == T_DOUBLE || b->type == T_DOUBLE) {
		vm->regs[255] = lat_bool(vm, lat_get_double_value(a) == lat_get_double_value(b));
	}
	else {
		vm->regs[255] = lat_bool(vm, lat_get_int_value(a) == lat_get_int_value(b));
	}
}

void lat_lt(lat_vm *vm)
{
	lat_object *a = lat_pop_stack(vm);
	lat_object *b = lat_pop_stack(vm);
	if ((a->type != T_INT && a->type != T_DOUBLE) || (b->type != T_INT && b->type != T_DOUBLE)) {
		log_err("Attempt to apply operator \"<\" on invalid types");
		exit(1);
	}
	if (a->type == T_DOUBLE || b->type == T_DOUBLE) {
		vm->regs[255] = lat_bool(vm, lat_get_double_value(a) < lat_get_double_value(b));
	}
	else {
		vm->regs[255] = lat_bool(vm, lat_get_int_value(a) < lat_get_int_value(b));
	}
}

void lat_lte(lat_vm *vm)
{
	lat_object *a = lat_pop_stack(vm);
	lat_object *b = lat_pop_stack(vm);
	if ((a->type != T_INT && a->type != T_DOUBLE) || (b->type != T_INT && b->type != T_DOUBLE)) {
		log_err("Attempt to apply operator \"<=\" on invalid types");
		exit(1);
	}
	if (a->type == T_DOUBLE || b->type == T_DOUBLE) {
		vm->regs[255] = lat_bool(vm, lat_get_double_value(a) <= lat_get_double_value(b));
	}
	else {
		vm->regs[255] = lat_bool(vm, lat_get_int_value(a) <= lat_get_int_value(b));
	}
}

void lat_gt(lat_vm *vm)
{
	lat_object *a = lat_pop_stack(vm);
	lat_object *b = lat_pop_stack(vm);
	if ((a->type != T_INT && a->type != T_DOUBLE) || (b->type != T_INT && b->type != T_DOUBLE)) {
		log_err("Attempt to apply operator \">\" on invalid types");
		exit(1);
	}
	if (a->type == T_DOUBLE || b->type == T_DOUBLE) {
		vm->regs[255] = lat_bool(vm, lat_get_double_value(a) > lat_get_double_value(b));
	}
	else {
		vm->regs[255] = lat_bool(vm, lat_get_int_value(a) > lat_get_int_value(b));
	}
}

void lat_gte(lat_vm *vm)
{
	lat_object *a = lat_pop_stack(vm);
	lat_object *b = lat_pop_stack(vm);
	if ((a->type != T_INT && a->type != T_DOUBLE) || (b->type != T_INT && b->type != T_DOUBLE)) {
		log_err("Attempt to apply operator \">=\" on invalid types");
		exit(1);
	}
	if (a->type == T_DOUBLE || b->type == T_DOUBLE) {
		vm->regs[255] = lat_bool(vm, lat_get_double_value(a) >= lat_get_double_value(b));
	}
	else {
		vm->regs[255] = lat_bool(vm, lat_get_int_value(a) >= lat_get_int_value(b));
	}
}

lat_object *lat_not(lat_vm *vm, lat_object *o)
{
	if (o->type != T_BOOL && o->type != T_INT) {
		log_err("Attempt to negate invalid type");
		exit(1);
	}
	return lat_bool(vm, !lat_get_bool_value(o));
	/*o->data.b = !o->data.b;
	return o;*/
}

lat_bytecode lat_bc(lat_ins i, int a, int b, void *meta)
{
	lat_bytecode ret;
	ret.ins = i;
	ret.a = a;
	ret.b = b;
	ret.meta = meta;
	return ret;
}

/*
TODO: Remove
char * getOpIns(enum lat_ins ins)
{
	return ins_str[ins];
}*/

void lat_call_func(lat_vm *vm, lat_object *func)
{
	if (func->type == T_FUNC) {
		lat_push_ctx(vm);
		//lat_set_ctx(lat_get_current_ctx(vm), lat_str(vm, "$"), func);
		lat_set_ctx(lat_get_current_ctx(vm), lat_str(vm, "$"), func);
		//printf("\t>>func %s, %p:\n", &func->data);
		lat_bytecode *inslist = ((lat_function *)func->data.func)->bcode;
		lat_bytecode cur;
		int pos;
		for (pos = 0, cur = inslist[pos]; cur.ins != OP_END; cur = inslist[++pos]) {
#if DEBUG_VM
			printf("%6i\t", pos);
#endif
			switch (cur.ins) {
			case OP_END:
#if DEBUG_VM
				printf("END");
#endif
				return;
				break;
			case OP_NOP:
#if DEBUG_VM
				printf("NOP");
#endif
				break;
			case OP_PUSH:
				lat_push_stack(vm, vm->regs[cur.a]);
#if DEBUG_VM
				printf("PUSH r%i", cur.a);
#endif
				break;
			case OP_POP:
				vm->regs[cur.a] = lat_pop_stack(vm);
				//lat_gc(vm);
#if DEBUG_VM
				printf("POP r%i", cur.a);				
#endif
				break;
			case OP_GET:
				//vm->regs[cur.a] = lat_get_ctx(vm->regs[cur.b], vm->regs[cur.a]);
				vm->regs[cur.a] = lat_get_ctx(vm->regs[cur.b], vm->regs[cur.a]);
#if DEBUG_VM
				printf("GET r%i r%i", cur.a, cur.b);
#endif
				break;
			case OP_SET:			
				//lat_set_ctx(vm->regs[cur.b], lat_str(vm, (char *)cur.meta), vm->regs[cur.a]);
				lat_set_ctx(vm->regs[cur.b], ((lat_object *)cur.meta), vm->regs[cur.a]);
#if DEBUG_VM
				printf("SET r%i r%i", cur.b, cur.a);
#endif
				break;
			case OP_STORECHAR:
				vm->regs[cur.a] = lat_char(vm, cur.b);
#if DEBUG_VM
				printf("STORECHAR r%i, %c", cur.a, cur.b);
#endif
				break;
			case OP_STOREINT:
			{
				//vm->regs[cur.a] = lat_int(vm, cur.b);
				vm->regs[cur.a] = ((lat_object *)cur.meta);
#if DEBUG_VM
				printf("STOREINT r%i, %i", cur.a, cur.b);
#endif
			}
				break;
			case OP_STOREDOUBLE:
				vm->regs[cur.a] = lat_double(vm, *((double *)cur.meta));
#if DEBUG_VM
				printf("STOREDOUBLE r%i, %d", cur.a, *((double *)cur.meta));
#endif
				break;
			case OP_STORESTR:
			{				
				//vm->regs[cur.a] = lat_str(vm, (char *)cur.meta);
				vm->regs[cur.a] = ((lat_object *)cur.meta);
#if DEBUG_VM
				printf("STORESTR r%i, %s", cur.a, ((lat_object *)cur.meta)->data.str);
#endif
			}
				break;
			case OP_STOREBOOL:
				//vm->regs[cur.a] = lat_bool(vm, cur.b);
				vm->regs[cur.a] =  ((lat_object *)cur.meta);
#if DEBUG_VM
				printf("STOREBOOL r%i, %i", cur.a, (int *)cur.meta);
#endif
				break;
			case OP_STORELIST:
				vm->regs[cur.a] = lat_list(vm, make_list());
#if DEBUG_VM
				printf("STORELIST r%i, %s", cur.a, "make_list");
#endif
				break;
			case OP_PUSHLIST:
				lat_push_list(vm->regs[cur.a], vm->regs[cur.b]);
#if DEBUG_VM
				printf("PUSHLIST r%i, r%i", cur.a, cur.b);
#endif
				break;
			case OP_POPLIST:
				vm->regs[cur.a] = lat_pop_list(vm->regs[cur.b]);
#if DEBUG_VM
				printf("POPLIST r%i, r%i", cur.a, cur.b);
#endif
				break;
			case OP_MOV:
				vm->regs[cur.a] = vm->regs[cur.b];
#if DEBUG_VM
				printf("MOV r%i, r%i", cur.a, cur.b);
#endif
				break;
			case OP_GLOBALNS:
				vm->regs[cur.a] = vm->ctx_stack[0];
				break;
			case OP_LOCALNS:
				vm->regs[cur.a] = lat_get_current_ctx(vm);
				//lat_gc(vm);
#if DEBUG_VM
				printf("LOCALNS r%i", cur.a);
#endif
				break;
			case OP_FN:
				vm->regs[cur.a] = lat_define_function(vm, (lat_bytecode *)cur.meta);
#if DEBUG_VM
				printf("FN %i", (char *)cur.meta);
#endif
				break;
			case OP_NS:
#if DEBUG_VM
				printf("NS r%i", cur.a);
#endif
				//vm->regs[cur.a] = lat_clone_object(vm, lat_get_current_ctx(vm));
				vm->regs[cur.a] = lat_get_current_ctx(vm);
				lat_push_predefined_ctx(vm, vm->regs[cur.a]);
				break;
			case OP_ENDNS:
#if DEBUG_VM
				printf("ENDNS r%i", cur.a);
#endif
				vm->regs[cur.a] = lat_pop_predefined_ctx(vm);
				break;
			case OP_JMP:				
				pos = cur.a - 1;
#if DEBUG_VM
				printf("JMP %i", pos);
#endif
				break;
			case OP_JMPIF:
#if DEBUG_VM
				printf("JMPIF r%i %i", cur.b, (cur.a -1) );
#endif
				if (lat_get_bool_value(vm->regs[cur.b])) {
					pos = cur.a - 1;
				}
				break;
			case OP_CALL:
#if DEBUG_VM
				printf("CALL r%i\n>> ", cur.a);
#endif
				lat_call_func(vm, vm->regs[cur.a]);
				//lat_gc(vm);
				break;
			case OP_NOT:
#if DEBUG_VM
				printf("NOT r%i", cur.a);
#endif
				vm->regs[cur.a] = lat_not(vm, vm->regs[cur.a]);
				//((lat_object *)vm->regs[cur.a])->data.b = !((lat_object *)vm->regs[cur.a])->data.b;
				break;
			}
#if DEBUG_VM
			printf("\n");
#endif
		}
		lat_pop_ctx(vm);
	}
	else if (func->type == T_CFUNC) {
		((void(*)(lat_vm *))(func->data.func))(vm);
	}
	else {
		debug("func->type: %d", func->type);
		log_err("Object not a function");
		exit(1);
	}
	/*free memory in garbage collector*/
	//lat_gc(vm);
}