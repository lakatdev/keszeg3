#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../constants.h"

#define STACK_SIZE 256

typedef struct {
    unsigned short type;
    unsigned short argsize;
    unsigned int args;
} instruction;

int* variables;
int** arrays;
int* array_lengths;
char** strings;
int* string_lengths;

int stack_top = -1;
int stack[STACK_SIZE];

instruction* instructions;
char* arguments;

int execution_state = 0;

void execute(instruction*);

/*
    Convert byte pointer to int pointer
*/
int* parse_int_pointer(char* bytes) {
    return (int*)bytes;
}

/*
    Push int value to stack
*/
void push(int value) {
    if (stack_top == STACK_SIZE - 1) {
        return;
    }
    stack[++stack_top] = value;
}

/*
    Pop value from stack
*/
int pop() {
    if (stack_top == -1) {
        return -1;
    }
    return stack[stack_top--];
}

/*
    Finds the end of the "if" statement
*/
int findEnd() {
    int newifs = 1;
    int seek_position = execution_state;

    while (newifs > 0) {
        seek_position++;

        if (instructions[seek_position].type >= IF_M_NN && instructions[seek_position].type <= IF_ME_VV) {
            newifs++;
        }

        if (instructions[seek_position].type == END) {
            newifs--;
        }
    }
    return seek_position;
}

/*
    Main function
    Reads input file and sends it for execution
*/
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Provide sufficient amount of arguments!\n");
        return 1;
    }

    unsigned int length;
    FILE* fptr = fopen(argv[1], "rb");
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    rewind(fptr);

    char* buffer = malloc(length);
    fread(buffer, length, 1, fptr);
    fclose(fptr);

    int* num_instructions = parse_int_pointer(buffer);
    int* num_vars = parse_int_pointer(&buffer[4]);
    int* num_arrays = parse_int_pointer(&buffer[8]);
    int* num_strings = parse_int_pointer(&buffer[12]);

    variables = malloc(sizeof(int) * *num_vars);
    arrays = malloc(sizeof(int*) * *num_arrays);
    memset(arrays, 0, sizeof(int*) * *num_arrays);
    strings = malloc(sizeof(char*) * *num_strings);
    memset(strings, 0, sizeof(char*) * *num_strings);

    array_lengths = malloc(sizeof(int) * *num_arrays);
    memset(array_lengths, 0, sizeof(int) * *num_arrays);

    string_lengths = malloc(sizeof(int) * *num_strings);
    memset(string_lengths, 0, sizeof(int) * *num_strings);

    instructions = malloc(*num_instructions * sizeof(instruction));
    memcpy(instructions, &buffer[sizeof(int) * 4], *num_instructions * sizeof(instruction));

    arguments = malloc(length - sizeof(int) * 4 - *num_instructions * sizeof(instruction));
    memcpy(arguments, &buffer[sizeof(int) * 4 + *num_instructions * sizeof(instruction)], length - sizeof(int) * 4 - *num_instructions * sizeof(instruction));

    for (execution_state = 0; execution_state < *num_instructions; execution_state++) {
        execute(&instructions[execution_state]);
        if (execution_state == -1) {
            break;
        }
    }

    for (int i = 0; i < *num_arrays; i++) {
        if (arrays[i] != NULL) {
            free(arrays[i]);
        }
    }

    for (int i = 0; i < *num_strings; i++) {
        if (strings[i] != NULL) {
            free(strings[i]);
        }
    }

    free(buffer);
    free(variables);
    free(arrays);
    free(array_lengths);
    free(strings);
    free(string_lengths);

    return 0;
}

/*
    Executes given instruction
*/
void execute(instruction* instruction) {
    switch (instruction->type) {
        case (SET_N): {
            int* var = parse_int_pointer(&arguments[instruction->args]);
            int* val = parse_int_pointer(&arguments[instruction->args + 4]);

            variables[*var] = *val;
            break;
        }
        case (SET_V): {
            int* var = parse_int_pointer(&arguments[instruction->args]);
            int* val = parse_int_pointer(&arguments[instruction->args + 4]);

            variables[*var] = variables[*val];
            break;
        }
        case (IF_M_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 > *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_M_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 > variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_M_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] > *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_M_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] > variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_L_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 < *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_L_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 < variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_L_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] < *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_L_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] < variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_E_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 == *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_E_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 == variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_E_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] == *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_E_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] == variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_D_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 != *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_D_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 != variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_D_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] != *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_D_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] != variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_LE_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 <= *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_LE_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 <= variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_LE_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] <= *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_LE_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] <= variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_ME_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 >= *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_ME_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(*c1 >= variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_ME_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] >= *c2)) {
                execution_state = findEnd();
            }

            break;
        }
        case (IF_ME_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            if (!(variables[*c1] >= variables[*c2])) {
                execution_state = findEnd();
            }

            break;
        }
        case (ADD_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = *c2 + *c3;

            break;
        }
        case (ADD_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = *c2 + variables[*c3];

            break;
        }
        case (ADD_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = variables[*c2] + *c3;

            break;
        }
        case (ADD_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = variables[*c2] + variables[*c3];

            break;
        }
        case (FREE_ARRAY): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);

            if (arrays[*c1] != NULL) {
                free(arrays[*c1]);
                array_lengths[*c1] = 0;
                arrays[*c1] = NULL;
            }

            break;
        }
        case (SUB_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = *c2 - *c3;

            break;
        }
        case (SUB_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = *c2 - variables[*c3];

            break;
        }
        case (SUB_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = variables[*c2] - *c3;

            break;
        }
        case (SUB_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = variables[*c2] - variables[*c3];

            break;
        }
        case (MUL_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = *c2 * *c3;

            break;
        }
        case (MUL_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = *c2 * variables[*c3];

            break;
        }
        case (MUL_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = variables[*c2] * *c3;

            break;
        }
        case (MUL_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = variables[*c2] * variables[*c3];

            break;
        }
        case (DIV_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = *c2 / *c3;

            break;
        }
        case (DIV_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = *c2 / variables[*c3];

            break;
        }
        case (DIV_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = variables[*c2] / *c3;

            break;
        }
        case (DIV_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = variables[*c2] / variables[*c3];

            break;
        }
        case (PRINT_NUM_N): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            printf("%d", *c1);

            break;
        }
        case (PRINT_NUM_V): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            printf("%d", variables[*c1]);
            
            break;
        }
        case (PRINT_ASCII_N): {
            char* c1 = &arguments[instruction->args];
            printf("%c", *c1);

            break;
        }
        case (PRINT_ASCII_V): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            printf("%c", variables[*c1]);
            
            break;
        }
        case (PRINT_CONST_STRING): {
            char* str = malloc(instruction->argsize + 1);
            memcpy(str, &arguments[instruction->args], instruction->argsize);
            str[instruction->argsize] = '\0';
            printf(str);
            free(str);

            break;
        }
        case (INPUT_NUM): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            scanf("%d", &variables[*c1]);
            break;
        }
        case (INPUT_ASCII): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            scanf(" %c", &variables[*c1]);
            break;
        }
        case (MOD_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = *c2 % *c3;

            break;
        }
        case (MOD_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = *c2 % variables[*c3];

            break;
        }
        case (MOD_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = variables[*c2] % *c3;

            break;
        }
        case (MOD_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            variables[*c1] = variables[*c2] % variables[*c3];

            break;
        }
        case (ARRSET_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (*c2 < 0) {
                break;
            }
            else if (*c2 >= array_lengths[*c1]) {
                if (arrays[*c1] != NULL) {
                    arrays[*c1] = realloc(arrays[*c1], sizeof(int) * (*c2 + 1));
                }
                else {
                    arrays[*c1] = malloc(sizeof(int) * (*c2 + 1));
                }
                array_lengths[*c1] = *c2 + 1;
            }

            arrays[*c1][*c2] = *c3;

            break;
        }
        case (ARRSET_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (*c2 < 0) {
                break;
            }
            else if (*c2 >= array_lengths[*c1]) {
                if (arrays[*c1] != NULL) {
                    arrays[*c1] = realloc(arrays[*c1], sizeof(int) * (*c2 + 1));
                }
                else {
                    arrays[*c1] = malloc(sizeof(int) * (*c2 + 1));
                }
                array_lengths[*c1] = *c2 + 1;
            }

            arrays[*c1][*c2] = variables[*c3];

            break;
        }
        case (ARRSET_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (variables[*c2] < 0) {
                break;
            }
            else if (variables[*c2] >= array_lengths[*c1]) {
                if (arrays[*c1] != NULL) {
                    arrays[*c1] = realloc(arrays[*c1], sizeof(int) * (variables[*c2] + 1));
                }
                else {
                    arrays[*c1] = malloc(sizeof(int) * (variables[*c2] + 1));
                }
                array_lengths[*c1] = variables[*c2] + 1;
            }

            arrays[*c1][variables[*c2]] = *c3;

            break;
        }
        case (ARRSET_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (variables[*c2] < 0) {
                break;
            }
            else if (variables[*c2] >= array_lengths[*c1]) {
                if (arrays[*c1] != NULL) {
                    arrays[*c1] = realloc(arrays[*c1], sizeof(int) * (variables[*c2] + 1));
                }
                else {
                    arrays[*c1] = malloc(sizeof(int) * (variables[*c2] + 1));
                }
                array_lengths[*c1] = variables[*c2] + 1;
            }

            arrays[*c1][variables[*c2]] = variables[*c3];

            break;
        }
        case (ARRGET_N): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (*c3 >= array_lengths[*c2] || *c3 < 0) {
                variables[*c1] = 0;
                break;
            }
            variables[*c1] = arrays[*c2][*c3];

            break;
        }
        case (ARRGET_V): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (variables[*c3] >= array_lengths[*c2] || variables[*c3] < 0) {
                variables[*c1] = 0;
                break;
            }
            variables[*c1] = arrays[*c2][variables[*c3]];

            break;
        }
        case (INC_N): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            variables[*c1] += *c2;
            break;
        }
        case (INC_V): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            variables[*c1] += variables[*c2];
            break;
        }
        case (DEC_N): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            variables[*c1] -= *c2;
            break;
        }
        case (DEC_V): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);

            variables[*c1] -= variables[*c2];
            break;
        }
        case (JUMP): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);

            execution_state = *c1 - 1;
            break;
        }
        case (EXEC): {
            char* str = malloc(instruction->argsize + 1);
            memcpy(str, &arguments[instruction->args], instruction->argsize);
            str[instruction->argsize] = '\0';
            system(str);
            free(str);

            break;
        }
        case (LOAD_ARRAY): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);

            char* str = malloc(instruction->argsize + 1 - 4);
            memcpy(str, &arguments[instruction->args + 4], instruction->argsize - 4);
            str[instruction->argsize - 4] = '\0';

            unsigned int length;
            FILE* load = fopen(str, "rb");
            fseek(load, 0, SEEK_END);
            length = ftell(load);
            rewind(load);

            int* buffer = malloc(length);
            fread(buffer, length, 1, load);
            fclose(load);

            int arr_length = *buffer;

            if (arr_length != array_lengths[*c1]) {
                arrays[*c1] = realloc(arrays[*c1], arr_length);
                array_lengths[*c1] = arr_length;
            }

            memcpy(arrays[*c1], &buffer[1], length - 4);
            
            free(buffer);
            free(str);

            break;
        }
        case (SAVE_ARRAY): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);

            char* str = malloc(instruction->argsize + 1 - 4);
            memcpy(str, &arguments[instruction->args + 4], instruction->argsize - 4);
            str[instruction->argsize - 4] = '\0';

            FILE* save = fopen(str, "wb");
            fwrite(&array_lengths[*c1], 4, 1, save);
            fwrite(arrays[*c1], sizeof(int) * array_lengths[*c1], 1, save);
            fclose(save);

            free(str);

            break;
        }
        case (LOAD_STRING): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);

            char* str = malloc(instruction->argsize + 1 - 4);
            memcpy(str, &arguments[instruction->args + 4], instruction->argsize - 4);
            str[instruction->argsize - 4] = '\0';

            unsigned int length;
            FILE* load = fopen(str, "rb");
            fseek(load, 0, SEEK_END);
            length = ftell(load);
            rewind(load);

            int* buffer = malloc(length);
            fread(buffer, length, 1, load);
            fclose(load);

            int arr_length = *buffer;

            if (arr_length != string_lengths[*c1]) {
                strings[*c1] = realloc(strings[*c1], arr_length);
                string_lengths[*c1] = arr_length;
            }

            memcpy(strings[*c1], &buffer[1], length - 4);
            
            free(buffer);
            free(str);
            
            break;
        }
        case (SAVE_STRING): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);

            char* str = malloc(instruction->argsize + 1 - 4);
            memcpy(str, &arguments[instruction->args + 4], instruction->argsize - 4);
            str[instruction->argsize - 4] = '\0';

            FILE* save = fopen(str, "wb");
            fwrite(&string_lengths[*c1], 4, 1, save);
            fwrite(strings[*c1], sizeof(char) * string_lengths[*c1], 1, save);
            fclose(save);

            free(str);
            
            break;
        }
        case (PUSHJUMP): {
            push(execution_state);
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            execution_state = *c1 - 1;
            break;
        }
        case (RET): {
            execution_state = pop();
            break;
        }
        case (ARRSIZE): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            variables[*c1] = array_lengths[*c2];
            break;
        }
        case (INPUT_STRING): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);

            char buffer[256]; //string input can only read up to 254 characters
            fgets(buffer, 256, stdin);

            int string_len = strlen(buffer) - 1;
            if (strings[*c1] != NULL) {
                strings[*c1] = realloc(strings[*c1], sizeof(int) * (string_lengths[*c1] + string_len));
            }
            else {
                strings[*c1] = malloc(sizeof(int) * (string_lengths[*c1] + string_len));
            }
            
            for (int i = 0; i < string_len && buffer[i] != '\n'; i++) {
                strings[*c1][string_lengths[*c1] + i] = buffer[i];
            }

            string_lengths[*c1] += string_len;
            break;
        }
        case (CAT): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);

            int string_len = instruction->argsize - 4;
            char* str = malloc(string_len);
            memcpy(str, &arguments[instruction->args + 4], string_len);

            if (strings[*c1] != NULL) {
                strings[*c1] = realloc(strings[*c1], sizeof(int) * (string_lengths[*c1] + string_len));
            }
            else {
                strings[*c1] = malloc(sizeof(int) * (string_lengths[*c1] + string_len));
            }
            
            for (int i = 0; i < string_len; i++) {
                strings[*c1][string_lengths[*c1] + i] = str[i];
            }

            string_lengths[*c1] += string_len;
            break;
        }
        case (STRSIZE): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            variables[*c1] = string_lengths[*c2];
            break;
        }
        case (STRSET_NN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (*c2 < 0) {
                break;
            }
            else if (*c2 >= string_lengths[*c1]) {
                if (strings[*c1] != NULL) {
                    strings[*c1] = realloc(strings[*c1], sizeof(char) * (*c2 + 1));
                }
                else {
                    strings[*c1] = malloc(sizeof(char) * (*c2 + 1));
                }
                string_lengths[*c1] = *c2 + 1;
            }

            strings[*c1][*c2] = *c3;

            break;
        }
        case (STRSET_NV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (*c2 < 0) {
                break;
            }
            else if (*c2 >= string_lengths[*c1]) {
                if (strings[*c1] != NULL) {
                    strings[*c1] = realloc(strings[*c1], sizeof(char) * (*c2 + 1));
                }
                else {
                    strings[*c1] = malloc(sizeof(char) * (*c2 + 1));
                }
                string_lengths[*c1] = *c2 + 1;
            }

            strings[*c1][*c2] = variables[*c3];

            break;
        }
        case (STRSET_VN): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (variables[*c2] < 0) {
                break;
            }
            else if (variables[*c2] >= string_lengths[*c1]) {
                if (strings[*c1] != NULL) {
                    strings[*c1] = realloc(strings[*c1], sizeof(char) * (variables[*c2] + 1));
                }
                else {
                    strings[*c1] = malloc(sizeof(char) * (variables[*c2] + 1));
                }
                string_lengths[*c1] = variables[*c2] + 1;
            }

            strings[*c1][variables[*c2]] = *c3;

            break;
        }
        case (STRSET_VV): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (variables[*c2] < 0) {
                break;
            }
            else if (variables[*c2] >= string_lengths[*c1]) {
                if (strings[*c1] != NULL) {
                    strings[*c1] = realloc(strings[*c1], sizeof(char) * (variables[*c2] + 1));
                }
                else {
                    strings[*c1] = malloc(sizeof(char) * (variables[*c2] + 1));
                }
                string_lengths[*c1] = variables[*c2] + 1;
            }

            strings[*c1][variables[*c2]] = variables[*c3];

            break;
        }
        case (STRGET_N): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (*c3 >= string_lengths[*c2] || *c3 < 0) {
                variables[*c1] = 0;
                break;
            }
            variables[*c1] = strings[*c2][*c3];

            break;
        }
        case (STRGET_V): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            int* c2 = parse_int_pointer(&arguments[instruction->args + 4]);
            int* c3 = parse_int_pointer(&arguments[instruction->args + 8]);

            if (variables[*c3] >= string_lengths[*c2] || variables[*c3] < 0) {
                variables[*c1] = 0;
                break;
            }
            variables[*c1] = strings[*c2][variables[*c3]];

            break;
        }
        case (PRINT_STRING): {
            int* c1 = parse_int_pointer(&arguments[instruction->args]);
            for (int i = 0; i < string_lengths[*c1]; i++) {
                printf("%c", strings[*c1][i]);
            }
            break;
        }
    }
}
