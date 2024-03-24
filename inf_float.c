#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct node{
    unsigned int data;//0~999999999
    struct node *next;
    struct node *prev;
} node;
typedef struct
{
    struct node *head;
    struct node *curr_pos;
    size_t curr_index;
    size_t size;
    int sign;
    int exp;//10^exp
} inf_float; // considered it smaller than 1.0

void init(inf_float *);
void destroy(inf_float *);
void append(inf_float *, unsigned int);
node* getLocation(inf_float *, intptr_t);
void insert(inf_float *, intptr_t, unsigned int);
void insert_head(inf_float *, unsigned int);

void LoadFromString(inf_float*, char*);
void FloatToString(inf_float*, char*);
void FloatToStringN(inf_float *, char *);
inf_float* inf_float_copy(inf_float *);
void quickNormalizeSelf(inf_float *);

int compare(inf_float *, inf_float *);

inf_float* add(inf_float *, inf_float *);
inf_float* addN(inf_float *, inf_float *);
inf_float* substract(inf_float *, inf_float *);
inf_float* substractN(inf_float *, inf_float *);
inf_float* multiply(inf_float *, inf_float *);
inf_float* multiply_int(inf_float *, unsigned int);
inf_float* divide(inf_float *, inf_float *);

void init(inf_float *a){
    a->head = NULL;
    a->curr_pos = NULL;
    a->curr_index = 0;
    a->size = 0;
    a->sign = 1;
    a->exp = 0;
}
void destroy(inf_float *a){
    if(a->head == NULL){
        return;
    }
    node *temp = a->head;
    a->head = NULL;
    do
    {
        node* next=temp;
        temp=temp->next;
        free(next);
    }
    while (temp);
}
void append(inf_float *a, unsigned int data)
{
    //insert at tail
    node *new_node = (node *)malloc(sizeof(node));
    if(a->size == 0){
        a->head = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
        new_node->data = data;
        a->curr_pos = new_node;
        a->curr_index = 0;
        a->size = 1;
        return;
    }
    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = getLocation(a, a->size-1);
    new_node->prev->next = new_node;
    a->size++;
}
node* getLocation(inf_float *a, intptr_t pos){
    if(pos<0 || pos>=a->size){
        return NULL;
    }
    if(a->size && (!a->curr_index || !a->curr_pos)){
        a->curr_pos = a->head;
        for (intptr_t i = 0; i < pos && a->curr_pos; i++)
        {
            a->curr_pos = a->curr_pos->next;
        }
        a->curr_index = pos;
        return a->curr_pos;
    }
    intptr_t offset = pos - a->curr_index;
    if(offset == 0){
        return a->curr_pos;
    }
    else if(offset > 0){
        for (size_t i = 0; i < offset && a->curr_pos && a->curr_pos->next; i++)
        {
            a->curr_pos = a->curr_pos->next;
        }
        a->curr_index = pos;
        return a->curr_pos;
    }
    else{
        for (size_t i = 0; i < -offset && a->curr_pos && a->curr_pos->prev; i++)
        {
            a->curr_pos = a->curr_pos->prev;
        }
        a->curr_index = pos;
        return a->curr_pos;
    }
}

void insert(inf_float *a, intptr_t pos, unsigned int data)
{
    a->curr_pos = NULL;
    node *location = getLocation(a, pos);
    if(location == NULL){
        return;
    }
    node *new_node = (node *)malloc(sizeof(node));
    if(a->size == 0){
        a->head = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
        new_node->data = data;
        a->curr_pos = new_node;
        a->curr_index = 0;
        a->size = 1;
        return;
    }
    new_node->data = data;
    new_node->next = location;
    new_node->prev = location->prev;
    location->prev->next = new_node;
    location->prev = new_node;
    a->size++;
}

void insert_head(inf_float *a, unsigned int data)
{
    //insert at head
    a->curr_pos = NULL;
    node *new_node = (node *)malloc(sizeof(node));
    if(a->size == 0){
        a->head = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
        new_node->data = data;
        a->curr_pos = new_node;
        a->curr_index = 0;
        a->size = 1;
        return;
    }
    new_node->data = data;
    new_node->next = a->head;
    new_node->prev = NULL;
    a->head->prev = new_node;
    a->head = new_node;
    a->size++;
}
inf_float* inf_float_copy(inf_float *a){
    inf_float *result = (inf_float *)malloc(sizeof(inf_float));
    init(result);
    for (intptr_t i = (intptr_t)a->size - 1; i >= 0; i--)
    {
        node *A = getLocation(a, i);
        append(result, A->data);
    }
    result->sign = a->sign;
    return result;
}


void quickNormalizeSelf(inf_float *a){
    size_t bufSize = 9 * a->size + 1;
    char *buf = malloc(bufSize);
    int exp = a->exp;
    FloatToStringN(a, buf);
    size_t start = 0;
    while (buf[start] == '0')
    {
        start++;
        exp--;
    }
    bufSize--;
    while (bufSize && buf[--bufSize] == '0')
    {
        buf[bufSize] = '\0';
    }
    
    destroy(a);
    LoadFromString(a, buf + start);
    free(buf);
    a->exp = exp;
}
inf_float* add(inf_float *a, inf_float *b){
    inf_float *result;
    if(a->sign == b->sign){
        result = addN(a,b);
        result->sign = a->sign;
    }
    else{
        result = substractN(a,b);
    }
    return result;
}
inf_float* addN(inf_float *a, inf_float *b){
    //now consider a and b are INT
    inf_float *result = (inf_float *)malloc(sizeof(inf_float));
    init(result);
    unsigned int carry = 0;
    size_t size = a->size > b->size ? a->size : b->size;
    for (intptr_t i = (intptr_t)size - 1; i >=0; i--)
    {
        node *A = getLocation(a, i);
        node *B = getLocation(b, i);
        unsigned int sum = 0;
        if(A)
            sum += A->data;
        if(B)
            sum += B->data;
        sum += carry;
        carry = sum / 1000000000;
        insert_head(result, sum % 1000000000);
    }
    if(carry){
        insert_head(result, carry);
    }
    return result;
}
inf_float* substract(inf_float *a, inf_float *b){
    inf_float *result;
    if(a->sign == b->sign){
        result = substractN(a,b);
        result->sign *= a->sign;
    }
    else{
        result = addN(a,b);
        result->sign = a->sign;
    }
    return result;
}
inf_float* substractN(inf_float *a_, inf_float *b_){
    //now consider a and b are INT
    inf_float *a = a_;
    inf_float *b = b_;
    inf_float *result = (inf_float *)malloc(sizeof(inf_float));
    init(result);
    unsigned int carry = 0;
    size_t size = a->size > b->size ? a->size : b->size;
    for (intptr_t i = (intptr_t)size - 1; i >=0; i--)
    {
        node *A = getLocation(a, i);
        node *B = getLocation(b, i);
        int sum = 0;
        if(A)
            sum += A->data;
        if(B)
            sum -= B->data;
        sum -= carry;
        if(sum < 0){
            sum += 1000000000;
            carry = 1;
        }
        else{
            carry = 0;
        }
        insert_head(result, sum % 1000000000);
    }
    result->sign = carry ? -1 : 1;
    return result;
}

inf_float *multiply_int(inf_float *a, unsigned int b)
{
    inf_float *result = (inf_float *)malloc(sizeof(inf_float));
    init(result);
    unsigned int carry = 0;
    for (intptr_t i = (intptr_t)a->size - 1; i >= 0; i--)
    {
        node *A = getLocation(a, i);
        unsigned long long int product = (unsigned long long int)A->data * (unsigned long long int)b + (unsigned long long int)carry;
        carry = (unsigned int)(product / 1000000000);
        insert_head(result, (unsigned int)(product % 1000000000));
    }
    if(carry){
        insert_head(result, carry);
    }
    return result;
}

inf_float* multiply(inf_float *a, inf_float *b){
    inf_float *result = (inf_float *)malloc(sizeof(inf_float));
    init(result);
    size_t k = 0;
    size_t sumLen = a->size + b->size;
    for (intptr_t i = (intptr_t)b->size - 1; i >= 0; i--)
    {
        node *B = getLocation(b, i);
        inf_float *temp = multiply_int(a, B->data);

        for (size_t j = 0; j < k; j++)
        {
            append(temp, 0);
        }
        intptr_t D = (intptr_t)sumLen - temp->size;
        for (size_t j = 0; j < D; j++)
        {
            insert_head(temp,0);
        }

        k++;
        node* R = add(result, temp);
        destroy(result);
        result = R;
        destroy(temp);
    }
    intptr_t D = (intptr_t)sumLen - result->size;
    for(intptr_t i=0;i < D;i++){
        insert_head(result,0);
    }
    result->sign = a->sign * b->sign;
    return result;
}

inf_float* divide(inf_float *a, inf_float *b){
    //now consider a and b are INT
    inf_float *result = (inf_float *)malloc(sizeof(inf_float));
    init(result);
    
}

void LoadFromString(inf_float* a, char* buf){
    destroy(a);
    init(a);
    unsigned int N = strlen(buf) / 9;
    unsigned int M = strlen(buf) % 9;
    int exp = 0;
    for (unsigned int i = 0; i < N && !exp; i++)
    {
        unsigned int num = 0;
        unsigned int E = 1e8;
        for(int j=0;j<9;j++){
            if(buf[i * 9 + j] == 'e'){
                fscanf(buf + i * 9 + j + 1, "%d", &a->exp);
                exp = 1;
                break;
            }
            num += (unsigned int)(buf[i * 9 + j] - '0') * E;
            E /= 10;
        }
        append(a,num);
    }
    unsigned int num = 0;
    unsigned int E = 1e8;
    for (unsigned int i = 0; i < M && !exp; i++)
    {
        num += (unsigned int)(buf[N * 9 + i] - '0') * E;
        E /= 10;
    }
    append(a,num);
}

int compare(inf_float *a, inf_float *b){
    for (intptr_t i = 0; i < a->size; i--)
    {
        node *A = getLocation(a, i);
        node *B = getLocation(b, i);
        unsigned int numA = 0;
        unsigned int numB = 0;
        if(A){
            numA = A->data;
        }
        if(B){
            numB = B->data;
        }
        if(numA > numB){
            return 1;
        }
        if(numA < numB){
            return -1;
        }
    }
    return 0;
}
void FloatToStringN(inf_float *a, char *buf)
{
    unsigned int N = a->size;
    unsigned int M = a->size * 9;
    for (int i = 0; i < N; i++)
    {
        node *A = getLocation(a, i);
        unsigned int num = A->data;
        unsigned int E = 1e8;
        for (unsigned int j = 0; j < 9; j++)
        {
            buf[i * 9 + j ] = num / E + '0';
            num %= E;
            E /= 10;
        }
    }
    node *A = getLocation(a, N - 1);
    unsigned int num = 0;
    if(A){
        num = A->data;
    }
    unsigned int E = 1e8;
    for (unsigned int i = 0; i < M % 9; i++)
    {
        buf[N * 9 + i] = num / E + '0';
        num %= E;
        E /= 10;
    }
    buf[M] = '\0';
}
void FloatToString(inf_float *a, char* buf){
    unsigned int N = a->size;
    unsigned int M = a->size * 9;
    int offset = 0;
    if(a->sign == -1){
        buf[0] = '-';
        offset = 1;
    }
    for(int i=0;i<N;i++){
        node *A = getLocation(a, i);
        unsigned int num = A->data;
        unsigned int E = 1e8;
        for (unsigned int j = 0; j < 9; j++)
        {
            buf[i * 9 + j + offset] = num / E + '0';
            num %= E;
            E /= 10;
        }
    }
    node *A = getLocation(a, N-1);
    unsigned int num = A->data;
    unsigned int E = 1e8;
    for (unsigned int i = 0; i < M % 9; i++)
    {
        buf[N * 9 + i + offset] = num / E + '0';
        num %= E;
        E /= 10;
    }
    if(a->exp){
        buf[M + offset] = 'e';
        sprintf(buf + M + 1 + offset, "%d", (int)a->exp);
        return;
    }
    buf[M] = '\0';
}


void main(){
    //test inf_float
    inf_float a, b;
    init(&a);
    init(&b);

    char str[5000];
    printf("Please input two float numbers:\n");
    printf("The first number:\t0.");
    scanf("%s", str);
    LoadFromString(&a, str);
    printf("The second number:\t0.");
    scanf("%s", str);
    LoadFromString(&b, str);

    inf_float *result = add(&a, &b);
    quickNormalizeSelf(result);
    printf("Compare \t\t%d\n", compare(&a, &b));
    FloatToString(result, str);
    printf("Add \t\t\t0.%s\n", str);
    destroy(result);
    result = substract(&a, &b);
    quickNormalizeSelf(result);
    FloatToString(result, str);
    printf("Sub \t\t\t0.%s\n", str);
    destroy(result);
    result = multiply(&a, &b);
    quickNormalizeSelf(result);
    FloatToString(result, str);
    printf("Mul \t\t\t0.%s\n", str);
    destroy(result);
    result = divide(&a, &b);
    quickNormalizeSelf(result);
    FloatToString(result, str);
    printf("Div \t\t\t0.%s\n", str);
    destroy(result);

    destroy(&a);
    destroy(&b);
}