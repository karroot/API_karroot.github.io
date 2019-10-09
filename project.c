#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>


#define KEY_SIZE (35)
#define MAX_CMD_SIZE (105)
#define DEBUG_ON (0)

#define DEBUG(line)                 \
  {                                 \
    if (DEBUG_ON) {                 \
      printf("%s ", CurrentDate()); \
      line;                         \
    }                               \
  }

//-------------------
typedef char* NodeKey;
typedef char* Relation;

typedef struct NodeCounter {
    int counter;
    NodeKey key;
} NodeCounter;



//---LIST---------

typedef struct LNODE {
    char* key;
    void* value;
    struct LNODE* next;
    struct LNODE* prev;
} LNODE;

typedef LNODE* LIST;

LIST list_create_node( char* key,  void* value);
void list_print(const LIST head);
LIST list_search(LIST head, const char* k);
LIST list_add_head(LIST head,  char* key,  void* k);
int list_delete(LIST* head_ref, const char* k);


LIST list_create_node( char* key,  void* value) {
    LIST new_node = (LIST)malloc(sizeof(LNODE));
   // printf("%d\n", sizeof(LNODE));
    new_node->key = key;
    new_node->value = value;
    new_node->next = NULL;
    new_node->prev = NULL;
    return new_node;
}

void list_print(const LIST head) {
    LIST x = head;
    while (x != NULL) {
        printf("%s, ", x->key);
        x = x->next;
    }
    printf("\n");
}


LIST list_search(LIST head, const char* k) {
    LIST x = head;
    while (x != NULL) {
        if (strcmp(x->key, k) == 0)
            return x;
        else
            x = x->next;
    }
    return NULL;
}


LIST list_add_head(LIST head,  char* key,  void* k) {
    LIST new_head = list_create_node(key, k);
    new_head->key = key;
    if (head == NULL) return new_head;

    new_head->next = head;
    head->prev = new_head;
    head = new_head;
    return head;
}


void list_destroy(LIST head)
{
    while(head)
    {
        LIST next = head->next;
        free(head);
        head = next;
    }
}

LIST list_add_node_head(LIST head, LNODE* new_head) {

    new_head->next = NULL;
    new_head->prev = NULL;
    if (head == NULL) return new_head;

    new_head->next = head;
    head->prev = new_head;
    head = new_head;
    return head;
}

int list_delete(LIST* head_ref, const char* k) {
    LIST head = *head_ref;
    if (head == NULL) return 0;

    LIST node_k = list_search(head, k);
    if (node_k == NULL) return 0;

    if (node_k->prev != NULL) {
        LIST node_k_prev = node_k->prev;
        node_k_prev->next = node_k->next;
    }

    if (node_k->next != NULL) {
        LIST node_k_next = node_k->next;
        node_k_next->prev = node_k->prev;
    }
    if (node_k->prev == NULL)  // sto aggiornando la testa
        *head_ref = node_k->next;

    return 1;
}


//---------------

#define HASHTABLE_THREAHOLD (0.8)
#define HASHTABLE_FACTOR (2)
#define HASHTABLE_INITIAL_CAP (1)

/**
 * Compute the hash value for the given string.
 * Implements the djb k=33 hash function.
 */
unsigned  hashtable_hash( const void*  str_v) {
    const char* str = (char*)str_v;
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}


struct hashtable;

typedef unsigned(*HashFunction)(const void*) ;



typedef struct hashtable
{
    unsigned size;
    unsigned capacity;

    LIST* data;

} hashtable;

void hashtable_set( hashtable* t,  char* key,  void* value);

hashtable* hashtable_create() {
    hashtable* ret = malloc(sizeof(hashtable));
    
    ret->size = 0;
    ret->capacity = HASHTABLE_INITIAL_CAP;
    ret->data = calloc(ret->capacity, sizeof(LIST));
    return ret;
}


void hashtable_rehash_all(hashtable* t )
{
    const unsigned old_capacity = t->capacity;
    unsigned  new_capacity = (double)old_capacity*(double)HASHTABLE_FACTOR;
    if(new_capacity==0)
        new_capacity = 2;
    LIST* data = t->data;

    t->size = 0;
    t->capacity = new_capacity;
    t->data =  calloc(new_capacity, sizeof(LIST));


    for (int i = 0; i < old_capacity; ++i)
    {
        if(data[i] != NULL)
        {
            LIST h = data[i];
            while(h)
            {
                hashtable_set(t, h->key, h->value);
                h = h->next;
            }
            list_destroy(data[i]);
        }
    }
    free(data);
}

unsigned get_index(const hashtable* t, const void* v)
{
    return hashtable_hash(v) % t->capacity;
}

void hashtable_set( hashtable* t,  char* key,  void* value)
{
    unsigned h = get_index(t, key);
    LIST list_of_key = t->data[h];
    LIST node = list_search(list_of_key, key);

    if(node == NULL)
    {
        const float lf = t->size/t->capacity;
        if(lf > HASHTABLE_THREAHOLD)
            hashtable_rehash_all(t);

        h = get_index(t, key);
        list_of_key = t->data[h];
        t->data[h] = list_add_head(list_of_key, key,  value);

        t->size++;
    }
}




/**
 * Return the item associated with the given key, or NULL if not found.
 */
void* hashtable_get(hashtable* t, const char* const key)
{
    const unsigned index = get_index(t, key);
    LIST list_of_key = t->data[index];
    LIST node = list_search(list_of_key, key);
    if(node)
        return node->value;
    return NULL;
}

bool hashtable_is_key_present(hashtable* t, char* k) {
    return hashtable_get(t, k) != NULL;
}

int hashtable_remove(hashtable* t, const char* key)
{
    const int index = get_index(t, key);
    const int rem = list_delete(&(t->data[index]) , key);
    t->size-= rem;
    return rem;
}

void hashtable_destroy(hashtable* t) {

}




//-------------------
struct binary_tree ;
struct binary_tree_node;
typedef int  (*binary_tree_node_cmp_f) ( void *a, void *b);
typedef void  (*binary_tree_node_free_f) (void* self);
typedef void  (*binary_tree_value_f) (void* self);


typedef struct binary_tree_node
{
    void *value;
    struct binary_tree_node* left;
    struct binary_tree_node* right;
} binary_tree_node;


typedef struct binary_tree
{
    struct binary_tree_node* root;
    binary_tree_node_cmp_f cmp;
    binary_tree_node_free_f free_fn;
    size_t             size;
}binary_tree;

unsigned binary_tree_size(binary_tree* t){
    return t->size;
}

binary_tree* binary_tree_create(binary_tree_node_cmp_f fn, binary_tree_node_free_f free_fn)
{
    binary_tree* ret = malloc(sizeof(binary_tree));
    ret->root = NULL;
    ret->cmp = fn;
    ret->free_fn = free_fn;
    ret->size = 0;
    return ret;

}

binary_tree_node* binary_tree_node_create(void* value) {
    binary_tree_node* ret =  malloc(sizeof(binary_tree_node));
    ret->value = value;
    ret->left = NULL;
    ret->right = NULL;
    return ret;
}

binary_tree_node* binary_tree_insert_node(binary_tree* tree, binary_tree_node* root, binary_tree_node* node)
{
    if(tree->root == NULL){
        tree->root = node;
        tree->size++;
        return tree->root;
    }
    else if(root == NULL){
        return node;
    }
    else
    {
        if(tree->cmp(root->value, node->value ) < 0)
            root->left = binary_tree_insert_node(tree, root->left, node);
        else
            root->right = binary_tree_insert_node(tree, root->right, node);
        tree->size++;
        return root;
    }

}

binary_tree_node* binary_tree_find_node(binary_tree*tree, binary_tree_node* root, void* value)
{
    if(!tree->root || root == NULL)
        return NULL;
    const int cmp_val = tree->cmp(root->value, value);
    if(cmp_val == 0)
        return root;
    if(cmp_val < 0 )
        return binary_tree_find_node(tree, root->left, value);
    else
        return binary_tree_find_node(tree, root->right, value);
}

void* binary_tree_find(binary_tree*tree, void* value){
    binary_tree_node* ret = binary_tree_find_node(tree, tree->root, value);
    if(ret != NULL)
        return ret->value;
    else
        return NULL;
}


binary_tree_node* binary_tree_min(binary_tree*tree, binary_tree_node* root)
{
    if(!tree->root || root == NULL)
        return NULL;
    if(!root->left)
        return root;
    return binary_tree_min(tree, root->left);
}


binary_tree_node* binary_tree_max(binary_tree*tree, binary_tree_node* root)
{
    if(!tree->root || root == NULL)
        return NULL;
    if(!root->right)
        return root;
    return binary_tree_max(tree, root->left);
}

binary_tree_node* binary_tree_delete_node(binary_tree* tree, binary_tree_node* root, void* value, binary_tree_node** deleted) {
    if(tree->root == NULL)
        return NULL;
    if(root == NULL)
        return NULL;
    const int cmp_val = tree->cmp(root->value, value);
    if(cmp_val < 0)
        root->left = binary_tree_delete_node(tree, root->left, value, deleted);
    else if (cmp_val > 0)
        root->right = binary_tree_delete_node(tree, root->right,value, deleted);

    else {
        // Case 1:  No child
        if(root->left == NULL && root->right == NULL) {
            *deleted = root;
            //tree->free_fn(root->value);
            free(root);
            root = NULL;
        }
            //Case 2: One child
        else if(root->left == NULL)
        {
            binary_tree_node *temp = root;
            *deleted = root;
            root = root->right;

            //tree->free_fn(temp->value);
            free(temp);
        }
        else if(root->right == NULL) {
            binary_tree_node *temp = root;
            *deleted = root;
            root = root->left;

            //tree->free_fn(temp->value);
            free(temp);
        }
            // case 3: 2 children
        else {
            binary_tree_node *temp = binary_tree_min(tree, root->right);
            root->value = temp->value;
            root->right = binary_tree_delete_node(tree, root->right,temp->value, deleted);
        }
        tree->size--;
    }
    return root;
}

binary_tree_node* binary_tree_delete(binary_tree* tree, void* value)
{
    binary_tree_node* deleled;
    tree->root = binary_tree_delete_node(tree, tree->root, value, &deleled);
    return tree->root;
}

binary_tree_node* binary_tree_delete_get_deleted(binary_tree* tree, void* value, binary_tree_node** node)
{
    tree->root = binary_tree_delete_node(tree, tree->root, value, node);
    return tree->root;
}


void binary_tree_insert(binary_tree* tree, void* value){
    assert(tree);
    binary_tree_insert_node(tree, tree->root, binary_tree_node_create(value));
}



void binary_tree_dealloc(binary_tree_node_free_f free_f, binary_tree_node* self) {
    if (self) {
        free_f(self->value);
    }
}

void tree_visit_in_order_node(binary_tree_node* root, binary_tree_value_f fn)
{
    if(root == NULL)
        return;
    tree_visit_in_order_node(root->left, fn);

    fn(root->value);

    tree_visit_in_order_node(root->right, fn);
}

void binary_tree_visit_in_order(binary_tree* tree, binary_tree_value_f fn){
    return tree_visit_in_order_node(tree->root, fn);
}

void tree_visit_best_node(binary_tree_node* root, binary_tree_value_f fn, const int limit)
{
    if(root == NULL)
        return;
    NodeCounter* nc = root->value;


    tree_visit_best_node(root->left, fn, limit);

    if(nc->counter < limit)
        return;
    fn(root->value);

    tree_visit_best_node(root->right, fn, limit);
    //printf("%s %d",nc->key, nc->counter);
}


binary_tree_node* predecesor(binary_tree* tree, binary_tree_node* n, binary_tree_node* parent ){
     binary_tree_node* r = binary_tree_max(tree, n->right);
     if(r)
        return r;
     
     return parent;
}

void tree_visit_best(binary_tree* tree, binary_tree_value_f fn)
{
    binary_tree_node* node = binary_tree_min(tree, tree->root);
    NodeCounter* ncnode = node->value;

    tree_visit_best_node(tree->root, fn, ncnode->counter);
    printf("%d", ncnode->counter);
    

}


/*
void tree_visit_best(binary_tree* tree, binary_tree_value_f fn)
{
    binary_tree_node* parent = tree->root;
    binary_tree_node* node = binary_tree_min(tree, tree->root);
    NodeCounter* ncnode = node->value;
    fn(ncnode);
    
    binary_tree_node* pred = predecessor(tree, node, parent);
    if(pred == NULL)
        return;
    NodeCounter* ncpred = pred->value;

    while(node != NULL && pred != NULL && ncpred->counter >= ncnode->counter)
    {
        
        node = pred;
        pred = predecesor(tree, node);
        fn(node->value);
    }

}*/



//-------------------

























#define DEBUG_IN DEBUG(  printf("START of %s\n", __PRETTY_FUNCTION__); );
#define DEBUG_OUT DEBUG( printf("END of %s\n", __PRETTY_FUNCTION__) ;);


inline static const char* CurrentDate() {
    time_t now;
    time(&now);
    static char buf[100] = {0};
    strftime(buf, sizeof(buf), "[%Y-%m-%d:%H:%M:%S] ", localtime(&now));
    return buf;
}


typedef struct binary_tree* NodeSetByRel;

//----------------------------

typedef struct vector_string
{
    unsigned capacity;
    unsigned size;
    char** data;
} vector_string;


#define INITIAL_SIZE (2)

int cmpfunc( const void *a, const void *b) {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
}

void vector_string_sort(vector_string* vec) {
    qsort(vec->data, vec->size, sizeof(char*), cmpfunc);
}

vector_string* vector_string_create(const unsigned size)
{
    DEBUG_IN;

    vector_string* ret = malloc(sizeof(vector_string));
    ret->data = calloc(size, sizeof(char*));
    ret->capacity = size;
    ret->size = 0;

    DEBUG_OUT;

    return ret;
}

vector_string* vector_string_create_default()
{
    return vector_string_create(INITIAL_SIZE);
}

char* vector_string_find(const vector_string* vec, const char* v)
{
    DEBUG_IN;
    for(unsigned i = 0; i < vec->size; i++)
    {
        if(strcmp(vec->data[i], v) == 0)
            return vec->data[i];
    }
    return NULL;
}

bool vector_string_exists(const vector_string* vec, const char* v)
{
    DEBUG_IN;
    bool res = vector_string_find(vec,v) != NULL;

    DEBUG_OUT;
    return res;
}

void vector_string_destroy(vector_string* vec)
{
    DEBUG_IN;
    for(int i = 0 ; i < vec->size; i++)
    {
        free(vec->data[i]);
        vec->data[i] = NULL;
        vec->size--;
    }
    DEBUG_OUT;
}

void vector_string_copy(vector_string* from, vector_string* to) {
    DEBUG_IN;

    assert(from->size <= to->capacity);

    vector_string_destroy(to);

    assert(to->size == 0);

    for(int i = 0 ; i < from->size; i++)
    {
        //to->data[i] = strdup(from->data[i]);
        to->data[i] = from->data[i];
        to->size++;
    }

    DEBUG_OUT;
}



vector_string* vector_string_resize(vector_string* vec)
{
    DEBUG_IN;

    const int new_capacity = vec->capacity*2;
    vec->data = realloc(vec->data, new_capacity*sizeof(char*));
    vec->capacity = new_capacity;
    /*vector_string* ret = vector_string_create(new_capacity);
    vector_string_copy(vec, ret);
    vector_string_destroy(vec);
    vec->data = ret->data;
    vec->size = ret->size;
    vec->capacity = ret->capacity;
    free(ret);*/

    DEBUG_OUT;
    return vec;

}

void vector_string_push_back(vector_string* vec,  char* v) {
    DEBUG_IN;
    if(vec->capacity <= vec->size)
        vector_string_resize(vec);
   // vec->data[vec->size] = strdup(v);
     vec->data[vec->size] = v;
    vec->size++;
    DEBUG_OUT;
}

void vector_string_push_back_sorted(vector_string* vec,  char* v)
{
    DEBUG_IN;

    if(vec->capacity <= vec->size)
        vector_string_resize(vec);

    int pos = 0;
    while(pos < vec->size && strcmp(vec->data[pos], v) < 0 )
        pos++;
    for(int i = vec->size-1 ; i >= pos ; i--)
        vec->data[i+1] = vec->data[i];

    //vec->data[pos] = strdup(v);
    vec->data[pos] = v;
    vec->size++;
    DEBUG_OUT;
}

void vector_string_remove(vector_string* vec, const char* c) {
    int pos = 0;
    for (int i = 0; i < vec->size; ++i)
    {
        if(strcmp(vec->data[i], c)==0)
        {
            pos = i;
        }
    }
    for(int i = pos+1 ; i < vec->size; i++)
    {
        vec->data[i-1] = vec->data[i];
    }
    vec->size--;
}

char* vector_string_at(const vector_string* vec, const int i)
{
    return vec->data[i];
}

//---------------------------------------------

typedef struct GraphNode
{
    char* key;
    hashtable* out; //NodeKey -> vector<Relation>
    hashtable* in; //NodeKey -> vector<Relation>
    hashtable* in_per_rel; //Relation -> int
} GraphNode;


GraphNode* graphnode_create(char* k)
{
    DEBUG_IN;
    GraphNode* gn = malloc(sizeof(GraphNode));
    gn->out = hashtable_create();
    gn->in = hashtable_create();
    gn->in_per_rel = hashtable_create();
    gn->key = k;
    return gn;
    DEBUG_OUT;
}

bool graphnode_have_relation_with(const GraphNode* from, const NodeKey to,  const Relation rel) {
    DEBUG_IN;

    const vector_string* outrels = hashtable_get(from->out, to);
    if(outrels != NULL)
    {
        return vector_string_exists(outrels, rel);
    }

    DEBUG_OUT;
    return false;

}

vector_string* graphnode_get_out_to_node(const GraphNode* node, const NodeKey key)
{

    if(!hashtable_is_key_present(node->out,key))
    {
        hashtable_set(node->out, key, vector_string_create_default());
    }
    assert(hashtable_is_key_present(node->out,key));
    return hashtable_get(node->out, key);
}

vector_string* graphnode_get_in_from_node(const GraphNode* node, const NodeKey key)
{
    if(!hashtable_is_key_present(node->in,key)) {
        hashtable_set(node->in, key, vector_string_create_default());
    }
    assert(hashtable_is_key_present(node->in,key));
    return hashtable_get(node->in, key);
}


int graphnode_add_in_per_rel(const GraphNode* gn, const Relation rel, const int add) {
    DEBUG_IN;
    int* counter = NULL;
    int oldcounter = 0;
    if(!hashtable_is_key_present(gn->in_per_rel, rel))
    {
        counter = malloc(sizeof(int));
        *counter= 1;
        hashtable_set(gn->in_per_rel, rel, counter);
        return oldcounter;
    }
    counter = hashtable_get(gn->in_per_rel, rel);
    oldcounter = *counter;
    assert(*counter >= 0);
    (*counter)+=add;

    DEBUG_OUT;

    return oldcounter;
}

int graphnode_increment_in_per_rel(const GraphNode* gn, const Relation rel)
{
    return graphnode_add_in_per_rel(gn, rel, 1);
}


int graphnode_decrement_in_per_rel(const GraphNode* gn, const Relation rel)
{
    return graphnode_add_in_per_rel(gn, rel, -1);
}




//---------------------------

int nodecounter_compare( void* av, void* bv)
{
    NodeCounter *a = av;
    NodeCounter *b = bv;
    const int cmp_counter = (a->counter > b->counter) - (a->counter < b->counter);
    if(cmp_counter != 0 )
        return cmp_counter;

    const int scmp = strcmp(a->key, b->key);
    if(scmp < 0)
        return 1;
    if(scmp >0)
        return -1;
    return scmp;
}


NodeCounter* nodecounter_create(const int cnt,  NodeKey k)
{
    NodeCounter* ret = malloc(sizeof(NodeCounter));
    ret->counter = cnt;
    //ret->key = strdup(k);
    ret->key = k;
    return ret;
}
//----------------------------
typedef struct Graph {
    hashtable* nodes; //Nodekey -> GraphNode
    hashtable* forest; //Relation -> NodeSetByRel
    vector_string* relations;
} Graph;

Graph* graph_create()
{
    DEBUG_IN;

    Graph* g = malloc(sizeof(Graph));
    g->nodes = hashtable_create();
    g->forest = hashtable_create();
    g->relations = vector_string_create(1);

    DEBUG_OUT;
    return g;
}

void graph_destroy(Graph* g)
{
    DEBUG_IN;
    assert(g->nodes != NULL);
    hashtable_destroy(g->nodes);
    DEBUG_OUT;
}

bool graph_node_exists(const Graph* g, const NodeKey  key)
{
    DEBUG_IN;
    const bool ans =  hashtable_is_key_present(g->nodes, key);
    DEBUG_OUT;
    return ans;
}

GraphNode* graph_get_node(const Graph* g, const NodeKey key) {
    return hashtable_get(g->nodes, key);
}

NodeSetByRel graph_get_tree(Graph* g, const Relation rel)
{
    NodeSetByRel setidrel = NULL;
    if(!hashtable_is_key_present(g->forest, rel)) {
        setidrel = binary_tree_create(nodecounter_compare, free);
        hashtable_set(g->forest, rel, setidrel);
    }

    NodeSetByRel ret =  hashtable_get(g->forest, rel);
    return ret;
    if(!ret)
        printf("errore\n");

}

void graph_add_to_counter_from_tree(
        NodeSetByRel tree,
        const int current_counter,
        NodeKey k,
        const int add)
{

    NodeCounter* nc = NULL;
    
    if(current_counter > 0) {
        nc = binary_tree_find(tree, &(NodeCounter) {
                current_counter, k
        });
        assert(nc != NULL);
        assert(nc->counter > 0);
        //binary_tree_delete_get_deleted(tree, nc, &deleted);
        //nc = deleted->value;
        binary_tree_delete(tree, nc);
        //rb_tree_remove(tree,nc);
    } else {
        assert(add > 0);
        nc = malloc(sizeof(NodeCounter));
        nc->counter = current_counter;
        //nc->key = strdup(k);
        nc->key = (k);
    }
    if(current_counter+add > 0) {

        nc->counter=current_counter + add;
        binary_tree_insert(tree, nc);
       /* if(deleted){
            //deleted->value = nc;
           // binary_tree_insert_node(tree,tree->root, deleted);
        }else{
        }*/
        //rb_tree_insert(tree,nc);
    }
}

void graph_increment_counter_from_tree(
        NodeSetByRel tree,
        const int current_counter,
        NodeKey k)
{
    graph_add_to_counter_from_tree(tree, current_counter, k, 1);
}

void graph_decrement_counter_from_tree(
        NodeSetByRel tree,
        const int current_counter,
        NodeKey k
)
{
    graph_add_to_counter_from_tree(tree, current_counter, k, -1);
}

Relation graph_add_relation(Graph* g, Relation rel)
{
    Relation rel1 = vector_string_find(g->relations, rel);
    if(!rel1)
    {
        rel1 = strdup(rel);
        vector_string_push_back_sorted(g->relations, rel1);
        //vector_string_sort(g->relations);
    }
    return rel1;
}

//-------------------------------

void handle_addent(Graph* g, char* cmd)
{
    DEBUG_IN;

    char* ident = strtok(cmd, "\"");
    ident = strtok(NULL, "\"");

    if(!graph_node_exists(g, ident))
    {
        ident = strdup(ident);
        hashtable_set(g->nodes, ident, graphnode_create(ident) );
    }

    DEBUG_OUT;
}


//-------------------------------

void handle_delent(Graph* g, char* cmd)
{
    DEBUG_IN;

    char* ident = strtok(cmd, "\"");
    ident = strtok(NULL, "\"");

    GraphNode* n = graph_get_node(g,ident);
    /*if(!graph_node_exists(g, ident))
        return;*/
    if(n == NULL)
        return;


    hashtable* nout = n->out;
    for (int j = 0; j < nout->capacity; j++) {
        //printf("%p ", nout->body[j].key);
        if (nout->data[j]  != NULL)
        {
            LIST h = nout->data[j];
            while(h) {
                NodeKey node2 = h->key;
                GraphNode* n2 = graph_get_node(g, node2);
                hashtable_remove(n2->in, ident);



                vector_string* vec = hashtable_get(nout, node2);
                for(int i = 0 ; i < vec->size ; i++)
                {
                    Relation idrel = vector_string_at(vec, i);
                    const int node2_current_counter = graphnode_decrement_in_per_rel(n2, idrel);
                    NodeSetByRel setidrel = graph_get_tree(g, idrel);
                    graph_decrement_counter_from_tree(setidrel, node2_current_counter, node2);

                }
                h = h->next;
            }
        }
    }

    hashtable* nin = n->in;
    for (int j = 0; j < nin->capacity; j++) {
        if (nin->data[j] != NULL)
        {
            LIST h = nin->data[j];
            while(h)
            {
                NodeKey node2 = h->key;
                GraphNode* n2 = graph_get_node(g, node2);
                hashtable_remove(n2->out, ident);

                h = h->next;
            }
        }
    }

    hashtable* forest = g->forest;
    for (int j = 0; j < forest->capacity; j++) {
        if (forest->data[j] != NULL)
        {
            LIST h = forest->data[j];
            while(h)
            {
                Relation idrel = h->key;
                int* oldcounter = hashtable_get(n->in_per_rel, idrel);
                if(oldcounter && *oldcounter > 0) {
                    NodeSetByRel tree = hashtable_get(forest, idrel);

                    /*int ret = rb_tree_remove(tree, &(NodeCounter) {
                        *oldcounter, ident
                    });
                    assert(ret > 0);
                    */
                    binary_tree_delete(tree, &(NodeCounter) {
                            *oldcounter, ident
                    });
                }
                h = h->next;
            }
        }
    }
    hashtable_remove(g->nodes, ident);

    DEBUG_OUT;
}


void handle_addrel(Graph* g, char* cmd)
{
    DEBUG_IN;


    strtok(cmd, "\"");  // cmd

    NodeKey ident1 = strtok(NULL, "\"");
    strtok(NULL, "\"");
    NodeKey ident2 = strtok(NULL, "\"");
    strtok(NULL, "\"");
    Relation idrel = strtok(NULL, "\"");

    DEBUG(printf("Aggiungo relazione tra: (%s -> %s, %s)\n", ident1, ident2,
                 idrel));

    /*if(!graph_node_exists(g,ident1) || !graph_node_exists(g,ident2))
        return;*/

    GraphNode* n1 = graph_get_node(g,ident1);
    if(n1== NULL || graphnode_have_relation_with(n1, ident2, idrel))
        return;

    GraphNode* n2 = graph_get_node(g,ident2);
    if(n2 == NULL)
        return;

    ident1 = n1->key;
    ident2 = n2->key;
    idrel = graph_add_relation(g, idrel);

    vector_string* n1out_to_n2 = graphnode_get_out_to_node(n1, ident2);
    vector_string* n2in_from_n2 = graphnode_get_in_from_node(n2, ident1);

    vector_string_push_back(n1out_to_n2, idrel);
    vector_string_push_back(n2in_from_n2, idrel);

    const int node2_current_counter = graphnode_increment_in_per_rel(n2,idrel);

    NodeSetByRel setidrel = graph_get_tree(g, idrel);
    assert(setidrel != NULL);
    graph_increment_counter_from_tree(setidrel, node2_current_counter, ident2);

    DEBUG_OUT;
}




//----------------------------------
void handle_delrel(Graph* g, char* cmd)
{
    DEBUG_IN;

    strtok(cmd, "\"");  // cmd


    char* const ident1 = strtok(NULL, "\"");
    strtok(NULL, "\"");
    char* const ident2 = strtok(NULL, "\"");
    strtok(NULL, "\"");
    char* const idrel = strtok(NULL, "\"");

    DEBUG(printf("Rimuovo relazione tra: (%s -> %s, %s)\n", ident1, ident2,
                 idrel));

    /*if(!graph_node_exists(g,ident1) || !graph_node_exists(g,ident2))
        return;*/

    GraphNode* n1 = graph_get_node(g,ident1);
    GraphNode* n2 = graph_get_node(g,ident2);

    if(n1 == NULL || n2 == NULL || !graphnode_have_relation_with(n1, ident2, idrel))
        return;

    vector_string* n1out_to_n2 = graphnode_get_out_to_node(n1, ident2);
    vector_string* n2in_from_n2 = graphnode_get_in_from_node(n2, ident1);

    vector_string_remove(n1out_to_n2, idrel);
    vector_string_remove(n2in_from_n2, idrel);

    const int node2_current_counter = graphnode_decrement_in_per_rel(n2,idrel);

    NodeSetByRel setidrel = graph_get_tree(g, idrel);

    graph_decrement_counter_from_tree(setidrel, node2_current_counter, ident2);
}



//-----------------------------------

#if 0
bool tree_print_best(NodeSetByRel t)
{
    struct rb_iter *iter = rb_iter_create();
    if (iter) {
        NodeCounter *nc = rb_iter_last(iter, t);
        const int counter = nc->counter;
        for (; nc && counter == nc->counter; nc = rb_iter_prev(iter)) {
            printf("\"%s\" ", nc->key);
            /* fputs("\"",stdout);
             fputs(nc->key,stdout);
             fputs("\" ",stdout);*/
        }
        printf("%d", counter);
        //fputs("%d", counter);
    }
    rb_iter_dealloc(iter);
}
#endif

void visit_fn(void * v)
{
    NodeCounter* nc = v;
    printf("\"%s\" ", nc->key);

}

void tree_print_best(NodeSetByRel t)
{
    tree_visit_best(t, visit_fn);
}


void handle_report(Graph* g)
{
    DEBUG_IN;

    const vector_string* relations = g->relations;
    if(relations->size <= 0 ) {
        fputs("none\n", stdout);

    } else {
        bool print_none = true;
        for(int i = 0 ; i < relations->size ; i++)
        {
            const Relation rel = vector_string_at(relations,i);
            NodeSetByRel t = hashtable_get(g->forest, rel);
            if(t != NULL && binary_tree_size(t) > 0 && t->root != NULL)
            {
                print_none = false;
                /*fputs("\"",stdout);
                fputs(rel,stdout);
                fputs("\" ",stdout);*/
                printf("\"%s\" ", rel);
                tree_print_best(t);
                //fputs("; ", stdout);
                printf("; ");
            }
        }
        if(print_none) {
            //fputs("none\n", stdout);
            printf("none\n");
        }
        else {
            printf("\n");
            // fputs("\n", stdout);
        }
    }
    DEBUG_OUT;
}
//-------------------------------

/**
 * funzione per gestire input
 * @param cmd stringa di input
 */
int handle_command(Graph* g,  char* cmd) {
    DEBUG_IN;

    DEBUG(printf("STO ESEGUENDO IL COMANDO: %s\n", cmd));

    const char* const addent_cmd = "addent";
    const char* const addrel_cmd = "addrel";
    const char* const delent_cmd = "delent";
    const char* const delrel_cmd = "delrel";
    const char* const report_cmd = "report";
    const char* const end_cmd = "end";

    if (strncmp(cmd, addent_cmd, strlen(addent_cmd)) == 0) {
        handle_addent(g, cmd);
    } else if (strncmp(cmd, addrel_cmd, strlen(addrel_cmd)) == 0) {
        handle_addrel(g,  cmd);
    } else if (strncmp(cmd, report_cmd, strlen(report_cmd)) == 0) {
        handle_report(g);
    } else if (strncmp(cmd, delrel_cmd, strlen(delrel_cmd)) == 0) {
        handle_delrel(g,  cmd);
    } else if (strncmp(cmd, delent_cmd, strlen(delent_cmd)) == 0) {
        handle_delent(g,  cmd);
    } else if (strncmp(cmd, end_cmd, strlen(end_cmd)) == 0) {
        return -1;
    }
    return 1;
    DEBUG_OUT;
}



int main() {
    Graph* g = graph_create();

    char cmd[MAX_CMD_SIZE];
    while (fgets(cmd, MAX_CMD_SIZE, stdin)) {
        if(handle_command(g, cmd) == -1)
            break;
    }

    graph_destroy(g);
    return 0;
}