#ifndef VARS_H
#define VARS_H

typedef struct {
    char* name;
    int name_length;
} variable_t;

variable_t* vars;
int variables_length = 0;

variable_t* arrs;
int arrays_length = 0;

variable_t* strs;
int strings_length = 0;

/*
    Returns ID of variable for bytecode use
*/
int get_var_id(char* name, int length)
{
    for (int i = 0; i < variables_length; i++) {
        if (memcmp(vars[i].name, name, length) == 0) {
            return i;
        }
    }

    variables_length++;
    vars = realloc(vars, variables_length * sizeof(variable_t));

    vars[variables_length - 1].name = malloc(length);
    memcpy(vars[variables_length - 1].name, name, length);
    vars[variables_length - 1].name_length = length;

    return variables_length - 1;
}

/*
    Returns ID of array for bytecode use
*/
int get_arr_id(char* name, int length)
{
    for (int i = 0; i < arrays_length; i++) {
        if (memcmp(arrs[i].name, name, length) == 0) {
            return i;
        }
    }

    arrays_length++;
    arrs = realloc(arrs, arrays_length * sizeof(variable_t));

    arrs[arrays_length - 1].name = malloc(length);
    memcpy(arrs[arrays_length - 1].name, name, length);
    arrs[arrays_length - 1].name_length = length;

    return arrays_length - 1;
}

/*
    Returns ID of string for bytecode use
*/
int get_str_id(char* name, int length)
{
    for (int i = 0; i < strings_length; i++) {
        if (memcmp(strs[i].name, name, length) == 0) {
            return i;
        }
    }

    strings_length++;
    strs = realloc(strs, strings_length * sizeof(variable_t));

    strs[strings_length - 1].name = malloc(length);
    memcpy(strs[strings_length - 1].name, name, length);
    strs[strings_length - 1].name_length = length;

    return strings_length - 1;
}

#endif
