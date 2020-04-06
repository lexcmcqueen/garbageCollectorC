/*

    @original creator: Munificent on Github

    @student: Lexi McQueen

 */

#include <stdio.h>
#include <stdlib.h>

#define STACK_MAX 256
#define INITIAL__GC_THRESHOLD 5

/*
 *  A minimal virtual machine
 *  contains an array(?) stack of size 256 
 */
typedef struct _VM {
    /* The total number of currently allocated objects */
    int numObjects;

    /* The number of objects required to trigger a GC. */
    int maxObjects; 

    /* The first object in the list of all objects */
    Object* firstObject;
    Object* stack[STACK_MAX];
    int stackSize;
} VM;

/*
 *  defining a little language:
 *  dynamically typed: only ints and pairs
 *  this enum: identifies the object's type
 */

typedef enum {
    OBJ_INT,
    OBJ_PAIR
} ObjectType;

// a pair can be anything, two ints, int and another pair

/*
 *  Object Struct
 *  Extended to be a node in the linked list keeping track of the references 
 *  to objects distinct from the semantics of the language user.
 */
typedef struct sObject {
    /* The next object in the list of all objects */
    struct sObject *next;

    unsigned char marked;
    ObjectType type;

    union {
        /* OBJ_INT */
        int value;

        /* OBJ_PAIR */
        struct {
            struct sObject* head;
            struct sObject* tail;
        };
    };
} Object;

/*
 *  Function: newVM()
 *  Returns:  an initialized, empty VM
 */
VM* newVM() {
    VM* vm = malloc(sizeof(VM));
    vm->stackSize = 0;

    vm->numObjects = 0;
    vm->maxObjects = INITIAL__GC_THRESHOLD;
    return vm;
}

/*
 *  Function: push(), takes a pointer to a VM object, and a pointer to an sObject.
 *  Returns:  
 */

void push(VM* vm, Object* value) {
    assert(vm->stackSize  < STACK_MAX, "Stack overflow!");
    vm->stack[vm->stackSize++] = value;
}

/*
 *  Function: pop(), takes a pointer to a VM object.
 *  Returns:  a virtual machine's stack whose size is decremented by the size of the value 
 */

Object* pop(VM* vm) {
    assert(vm->stackSize > 0, "Stack underflow!");
    return vm->stack[--vm->stackSize];
}

/*
 *  Function: newObject(), takes a pointer to a VM object, and type.
 *  Returns:  a newly initialized object of said type
 */

Object* newObject(VM* vm, ObjectType type) {
    if(vm->numObjects == vm->maxObjects) gc(vm);

    Object* object = newObject(vm, OBJ_INT);
    object->type = type;
    object->marked = 0;

    /* Insert it into the list of allocated objects. */
    object->next = vm->firstObject;
    vm->firstObject = object;

    ++vm->numObjects;
    return object;
}

/*
 *  Function: pushInt(), takes a pointer to a VM object, and an int.
 *  Returns:  
 */
void pushInt(VM* vm, int intValue) {
    Object* object = newObject(vm, OBJ_INT);
    object->value = intValue;
    push(vm, object);
}

/*
 *  Function: pushPair(), takes a pointer to a VM object.
 *  Returns:  a freshly created pair(?) object
 */
void pushPair(VM* vm) { 
    Object* object = newObject(vm, OBJ_PAIR);
    object->tail = pop(vm);
    object->head = pop(vm);

    push(vm, object);
    return object;
}

/*
 *  Function: markAll(), takes a pointer to a VM object.
 *            walks through the stack and 'marks' all variables in memory
 */
void markAll(VM* vm) {
    for(int i = 0; i < vm->stackSize; ++i) {
        mark(vm->stack[i]);
    }
}

/*
 *  Function: mark(), takes a pointer to an Object.
 *            Marks the OBJECT ITSELF as reachable.
 *            Still need to handle references in objects, 
 *            since reachability is recursive.
 */
void mark(Object* object) {
    // If it's marked, we're good. Check this base case. 
    if (object->marked) return;

    object->marked = 1;

    if(object->type == OBJ_PAIR) {
        mark(object->head);
        mark(object->tail);
    }
}

/*
 *  Function: sweep(), takes a pointer to a VM object.
 */
void sweep(VM* vm) {
    
    Object** object = &vm->firstObject;
    while(*object) {
        if(!(*object)->marked) {
            /* This object wasn't reached, so we'll remove and free it. */
            Object* unreached = *object;

            *object = unreached->next;
            vm->numObjects--;
            free(unreached);
        } else {
            /* This object WAS reached, so unmark it, 
            and move to the next one  */
            (*object)->marked = 0;
            object = &(*object)->next;
        }
    }
}

/*
 *  Function: gc(), takes a pointer to a VM object.
 *             calls the markAll function, and the sweep function.
 */
void gc(VM* vm) {
    int numObjects = vm->numObjects;

    markAll(vm);
    sweep(vm);

    vm->maxObjects = vm->numObjects * 2;
}
int main() {

    return 0;
}