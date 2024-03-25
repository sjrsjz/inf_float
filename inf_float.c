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
void LoadFromStringN(inf_float*, char*);
void FloatToString(inf_float*, char*);
void FloatToStringN(inf_float *, char *);
inf_float* inf_float_copy(inf_float *);
void quickNormalizeSelf(inf_float *);
void quickRShift10(inf_float *, size_t);

int compare(inf_float *, inf_float *);
int compareN(inf_float *, inf_float *);

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

void quickRShift10(inf_float *a, size_t k){
    size_t bufSize = 9 * a->size + 1 + k;
    int sign = a->sign;
    char *buf = malloc(bufSize);
    int k0 = k;
    while (k0--)
    {
        *(buf + k0) = '0';
    }    
    int exp = a->exp;
    FloatToStringN(a, buf + k);
    destroy(a);
    LoadFromStringN(a, buf);
    free(buf);
    a->sign = sign;
    a->exp = exp;
}
void quickNormalizeSelf(inf_float *a){
    size_t bufSize = 9 * a->size + 1;
    int sign = a->sign;
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
    LoadFromStringN(a, buf + start);
    free(buf);
    a->sign = sign;
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
        result->sign *= a->sign;
    }
    return result;
}
inf_float* addN(inf_float *a_, inf_float *b_){
    inf_float *a = a_, *b = b_;

    int exp_diff = a->exp - b->exp;
    int max_exp = a->exp > b->exp ? a->exp : b->exp;
    if (exp_diff > 0)
    {
        b = inf_float_copy(b_);
        quickRShift10(b, exp_diff);
    }
    else if (exp_diff < 0)
    {
        a = inf_float_copy(a_);
        quickRShift10(a, -exp_diff);
    }
    
    inf_float *result = (inf_float *)malloc(sizeof(inf_float));
    init(result);
    unsigned int carry = 0;
    size_t size = a->size > b->size ? a->size : b->size;
    
    for (intptr_t i = (intptr_t)size - 1; i >= 0; i--) {
        node *A = getLocation(a, i);
        node *B = getLocation(b, i);
        unsigned int sum = 0;
        if (A)
            sum += A->data;
        if (B)
            sum += B->data;
        sum += carry;
        carry = sum / 1000000000;
        insert_head(result, sum % 1000000000);
    }
    result->exp = max_exp;
    if (carry) {
        insert_head(result, carry);
        result->exp+=9;
    }
    if(exp_diff > 0){
        destroy(b);
    }
    else if(exp_diff < 0){
        destroy(a);
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
    inf_float *a0 = a_;
    inf_float *b0 = b_;
    inf_float *result = (inf_float *)malloc(sizeof(inf_float));
    init(result);
    unsigned int carry = 0;
    int exp_diff = a_->exp - b_->exp;
    int max_exp = a_->exp > b_->exp ? a_->exp : b_->exp;
    
    if (exp_diff > 0)
    {
        b0 = inf_float_copy(b_);
        quickRShift10(b0, exp_diff);
    }
    else if (exp_diff < 0)
    {
        a0 = inf_float_copy(a_);
        quickRShift10(a0, -exp_diff);
    }
    inf_float *a = a0;
    inf_float *b = b0;
    if(compareN(a,b) < 0){
        inf_float *temp = a;
        a = b;
        b = temp;
        result->sign = -1;
    }
    size_t size = a->size > b->size ? a->size : b->size;
    for (intptr_t i = (intptr_t)size - 1; i >=0; i--)
    {
        node *A = getLocation(a, i);
        node *B = getLocation(b, i);
        long long int sum = 0;
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
        insert_head(result, (unsigned int)(sum % 1000000000));
    }
    result->exp = max_exp;
    if(exp_diff > 0){
        destroy(b0);
    }
    else if(exp_diff < 0){
        destroy(a0);
    }
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
    result->exp = a->exp + b->exp;
    return result;
}

inf_float* divide(inf_float *a, inf_float *b){
    //now consider a and b are INT
    inf_float *result = (inf_float *)malloc(sizeof(inf_float));
    init(result);
    
}

void LoadFromStringN(inf_float *a, char *buf)
{
    destroy(a);
    init(a);
    unsigned int N = strlen(buf) / 9;
    unsigned int M = strlen(buf) % 9;
    for (unsigned int i = 0; i < N ; i++)
    {
        unsigned int num = 0;
        unsigned int E = 1e8;
        for (int j = 0; j < 9; j++)
        {
            num += (unsigned int)(buf[i * 9 + j] - '0') * E;
            E /= 10;
        }
        append(a, num);
    }
    unsigned int num = 0;
    unsigned int E = 1e8;
    for (unsigned int i = 0; i < M ; i++)
    {
        num += (unsigned int)(buf[N * 9 + i] - '0') * E;
        E /= 10;
    }
    append(a, num);
}


int getIntFromStr(char *buf, int *offset){
    int num = 0;
    if(buf[*offset] == '-'){
        (*offset)++;
        return -getIntFromStr(buf, offset);
    }
    while (buf[*offset] >= '0' && buf[*offset] <= '9')
    {
        num = num * 10 + buf[*offset] - '0';
        (*offset)++;
    }
    return num;
}

void LoadFromString(inf_float* a, char* buf){
    destroy(a);
    init(a);
    int exp = 0;
    int offset = 0;
    int exp_offset = 0;
    int start_float = 0;

    if(buf[0] == '-'){
        a->sign = -1;
        offset = 1;
    }
    while (buf[offset] == '0' || buf[offset] == '.')
    {
        exp_offset -= start_float;
        start_float = start_float | buf[offset++] == '.';
    }
    size_t size = strlen(buf + offset);
    unsigned int N = size / 9;
    unsigned int M = size % 9;
    for (unsigned int i = 0; i < N && !exp; i++)
    {

        unsigned int num = 0;
        unsigned int E = 1e8;
        for(int j=0;j<9;j++){
            switch(buf[i * 9 + j]){
                case 'e':
                    int k = 0;
                    a->exp = getIntFromStr(buf + i * 9 + j + offset + 1, &k);
                    exp = 1;
                    goto label;
                case '.':
                    start_float = 1;
                    continue;
            }
            num += (unsigned int)(buf[i * 9 + offset + j] - '0') * E;
            E /= 10;
            if (!start_float)
                exp_offset++;
        }
        label:
        append(a,num); 
    }
    unsigned int num = 0;
    unsigned int E = 1e8;
    for (unsigned int i = 0; i < M && !exp; i++)
    {
        switch (buf[N * 9 + offset + i])
        {
        case 'e':
            int k = 0;
            a->exp = getIntFromStr(buf + N * 9 + i + offset + 1, &k);
            exp = 1;
            goto label2;
        case '.':
            start_float = 1;
            continue;
        }
        num += (unsigned int)(buf[N * 9 + offset + i] - '0') * E;
        E /= 10;
        if (!start_float)
            exp_offset++;
    }
    label2:
    if(num) append(a,num);
    a->exp += exp_offset;
}

int compareN(inf_float *a, inf_float *b){
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

int compare(inf_float *a, inf_float *b)
{
    inf_float *A = inf_float_copy(a);
    inf_float *B = inf_float_copy(b);
    quickNormalizeSelf(A);
    quickNormalizeSelf(B);
    if(A->sign > B->sign){
        return 1;
    }
    if(A->sign < B->sign){
        return -1;
    }
    if (A->sign == -1)
    {
        if (A->exp > B->exp)
        {
            return -1;
        }
        if (A->exp < B->exp)
        {
            return 1;
        }
        return -compareN(a, b);
    }    
    if(A->exp > B->exp){
        return 1;
    }
    if(A->exp < B->exp){
        return -1;
    }
    return compareN(a,b);
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
    int exp = 0;
    if(a->sign == -1){
        buf[0] = '-';
        offset = 1;
    }
    buf[offset] = '0';
    offset++;
    buf[offset] = '.';
    offset++;
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
    printf("The first number:\t");
    scanf("%s", str);
    LoadFromString(&a, str);
    printf("The second number:\t");
    scanf("%s", str);
    LoadFromString(&b, str);

    FloatToString(&a, str);
    printf("The first number:\t%s\n", str);
    FloatToString(&b, str);
    printf("The second number:\t%s\n", str);

    inf_float *result = add(&a, &b);
    quickNormalizeSelf(result);
    printf("Compare \t\t%d\n", compare(&a, &b));
    FloatToString(result, str);
    printf("Add \t\t\t%s\n", str);
    destroy(result);
    result = substract(&a, &b);
    quickNormalizeSelf(result);
    FloatToString(result, str);
    printf("Sub \t\t\t%s\n", str);
    destroy(result);
    result = multiply(&a, &b);
    quickNormalizeSelf(result);
    FloatToString(result, str);
    printf("Mul \t\t\t%s\n", str);
    destroy(result);
    result = divide(&a, &b);
    quickNormalizeSelf(result);
    FloatToString(result, str);
    printf("Div \t\t\t%s\n", str);
    destroy(result);

    destroy(&a);
    destroy(&b);
}