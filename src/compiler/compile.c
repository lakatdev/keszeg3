#include <stdio.h>
#include <stdlib.h>
#include "../constants.h"
#include "utils.h"
#include "flags.h"
#include "binary.h"
#include "vars.h"

void remove_comments(char* buffer, int length);
int clear_buffer(char* buffer, unsigned int length);
lines_list_t parse_lines(char* buffer, unsigned int length);
void precompile(lines_list_t* lines);
void process_flags(lines_list_t* lines);
void process_code(lines_list_t* lines);
void handle_line(line_t* line);
void print_lines(lines_list_t* lines);
void insert_entrypoint(lines_list_t* lines);

/*
    Exits program with provided error message
*/
void error(const char* msg) {
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

/*
    Main function
    Reads input file and sends it for processing
*/
int main(int argv, char** argc) {
    if (argv < 3) {
        error("Provide sufficient amount of arguments!");
    }

    FILE* fptr = fopen(argc[1], "r");
    fseek(fptr, 0, SEEK_END);

    unsigned int length = ftell(fptr);
    rewind(fptr);

    char* buffer = malloc(length);
    fread(buffer, length, 1, fptr);
    fclose(fptr);

    remove_comments(buffer, length);
    length = clear_buffer(buffer, length);

    lines_list_t lines = parse_lines(buffer, length);
    free(buffer);

    insert_entrypoint(&lines);
    precompile(&lines);
    process_flags(&lines);
    process_code(&lines);

    merge();

    FILE* save = fopen(argc[2], "wb");
    fwrite(&lines.length, 4, 1, save);
    fwrite(&variables_length, 4, 1, save);
    fwrite(&arrays_length, 4, 1, save);
    fwrite(&strings_length, 4, 1, save);

    fwrite(instructions, instructions_length, 1, save);

    fclose(save);

    return 0;
}

/*
    Converts a character to lowercase if it is uppercase
*/
int tolower(int c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    } 
    else {
        return c;
    }
}

/*
    Compares two strings case insensitively
*/
bool case_insensitive_compare(const char* str1, const char* str2, int len1, int len2) {
    if (len1 != len2) {
        return false;
    }

    for (int i = 0; i < len1; i++) {
        if (tolower(str1[i]) != tolower(str2[i])) {
            return false;
        }
    }
    return true;
}

/*
    Replaces comments with whitespaces
*/
void remove_comments(char* buffer, int length) {
    for (int i = 0; i < length; i++) {
        if (buffer[i] == '#') {
            for (int j = i; j < length; j++) {
                if (buffer[j] == '\n') {
                    break;
                }
                else {
                    buffer[j] = ' ';
                }
            }
        }
    }
}

/*
    Clears the buffer from unnecessary characters and returns its length
*/
int clear_buffer(char* buffer, unsigned int length) {
    replace_char(buffer, length, '\t', ' ');
    replace_char(buffer, length, '\r', '\n');

    int length_before = 0;

    while (length_before != length) {
        length_before = length;
        length = remove_double_chars(buffer, length, ' ', ' ', ' ');
        length = remove_double_chars(buffer, length, '\n', '\n', '\n');
        length = remove_double_chars(buffer, length, '\n', ' ', '\n');
        length = remove_double_chars(buffer, length, ' ', '\n', '\n');
    }

    bool modified = false;

    if (buffer[length - 1] == '\n' || buffer[length - 1] == ' ') {
        buffer = realloc(buffer, length);
        length--;
        buffer[length] = '\0';
        modified = true;
    }

    if (buffer[0] == '\n' || buffer[0] == ' ') {
        memmove(buffer, buffer + 1, length);
        length--;
        buffer[length] = '\0';
        modified = true;
    }

    if (!modified) {
        buffer = realloc(buffer, length + 1);
        buffer[length] = '\0';
    }

    return length;
}

/*
    Returns the lines array with usable structure
    Needs the buffer and its length as input
*/
lines_list_t parse_lines(char* buffer, unsigned int length) {

    lines_list_t ret;

    int lines_count = count_chars(buffer, length, '\n') + 1;
    int last_line_end = -1;

    line_t* lines = malloc(sizeof(line_t) * lines_count);

    for (int i = 0; i < lines_count; i++) {
        int line_end = get_char_pos(buffer, length, '\n', i);

        int line_length = line_end - last_line_end;

        int tokens_count = count_chars(&buffer[last_line_end], line_length,  ' ') + 1;

        lines[i].length = tokens_count;
        lines[i].data = malloc(sizeof(token_t) * tokens_count);

        if (tokens_count > 1) {
            lines[i].data[0].length = (last_line_end + get_char_pos(&buffer[last_line_end], line_length, ' ', 0)) - (last_line_end) - 1;
        }
        else {
            lines[i].data[0].length = line_length - 1;
        }

        lines[i].data[0].data = malloc(lines[i].data[0].length);
        memcpy(lines[i].data[0].data, &buffer[last_line_end + 1], lines[i].data[0].length);

        for (int j = 1; j < tokens_count; j++) {
            if (j < tokens_count - 1) {
                lines[i].data[j].length = (last_line_end + get_char_pos(&buffer[last_line_end], line_length, ' ', j)) -
                    (last_line_end + get_char_pos(&buffer[last_line_end], line_length, ' ', j - 1)) - 1;
            }
            else {
                lines[i].data[j].length = (line_length) - (get_char_pos(&buffer[last_line_end], line_length, ' ', j - 1)) - 1;
            }
            lines[i].data[j].data = malloc(lines[i].data[j].length);
            memcpy(lines[i].data[j].data, &buffer[last_line_end + get_char_pos(&buffer[last_line_end], line_length, ' ', j - 1) + 1], lines[i].data[j].length);
        }

        last_line_end = line_end;
    }

    ret.data = lines;
    ret.length = lines_count;

    return ret;
}

/*
    Finds the end of given loop or if statement
*/
int find_end(lines_list_t* lines, int position) {
    int newifs = 1;
    int seek_position = position + 1;

    while (newifs > 0) {
        seek_position++;

        int token_length = lines->data[seek_position].data[0].length;
        char token[token_length];
        memcpy(token, lines->data[seek_position].data[0].data, token_length);

        if (case_insensitive_compare("if", token, strlen("if"), token_length) ||
            case_insensitive_compare("while", token, strlen("while"), token_length)) {
            newifs++;
        }
        if (case_insensitive_compare("end", token, strlen("end"), token_length)) {
            newifs--;
        }
    }
    return seek_position;
}

/*
    Inserts "jump main" to the beginning of the code
*/
void insert_entrypoint(lines_list_t* lines) {
    line_t entryline;
    entryline.length = 2;
    entryline.data = malloc(sizeof(token_t) * 2);

    entryline.data[0].data = malloc(4);
    entryline.data[0].length = 4;
    memcpy(entryline.data[0].data, "jump", 4);

    entryline.data[1].data = malloc(4);
    entryline.data[1].length = 4;
    memcpy(entryline.data[1].data, "main", 4);

    insert_line(lines, 0, entryline);
}

/*
    Converts higher level functionalities to compilable code
*/
void precompile(lines_list_t* lines) {
    for (int i = 0; i < lines->length; i++) {

        int token_length = lines->data[i].data[0].length;
        char token[token_length];
        memcpy(token, lines->data[i].data[0].data, token_length);

        if (case_insensitive_compare("while", token, strlen("while"), token_length)) {
            lines->data[i].data[0].data = realloc(lines->data[i].data[0].data, 2);
            memcpy(lines->data[i].data[0].data, "if", 2);
            lines->data[i].data[0].length = 2;

            line_t flagline;
            flagline.length = 2;
            flagline.data = malloc(sizeof(token_t) * 2);

            flagline.data[0].data = malloc(4);
            flagline.data[0].length = 4;
            memcpy(flagline.data[0].data, "flag", 4);

            flagline.data[1].data = malloc(24);
            flagline.data[1].length = 24;
            random_string(flagline.data[1].data, 24);

            insert_line(lines, i, flagline);

            line_t jumpline;
            jumpline.length = 2;
            jumpline.data = malloc(sizeof(token_t) * 2);

            jumpline.data[0].data = malloc(4);
            jumpline.data[0].length = 4;
            memcpy(jumpline.data[0].data, "jump", 4);

            jumpline.data[1].data = malloc(24);
            jumpline.data[1].length = 24;
            memcpy(jumpline.data[1].data, flagline.data[1].data, 24);

            insert_line(lines, find_end(lines, i), jumpline);

            precompile(lines);
        }
        else if (case_insensitive_compare("rout", token, strlen("rout"), token_length)) {
            lines->data[i].data[0].data = realloc(lines->data[i].data[0].data, 4);
            memcpy(lines->data[i].data[0].data, "flag", 4);
            lines->data[i].data[0].length = 4;
            precompile(lines);
        }
    }
}

/*
    Loads flags into global array
*/
void process_flags(lines_list_t* lines) {
    for (int i = 0; i < lines->length; i++) {
        int token_length = lines->data[i].data[0].length;
        char token[token_length];
        memcpy(token, lines->data[i].data[0].data, token_length);

        if (case_insensitive_compare("flag", token, strlen("flag"), token_length)) {
            add_flag(lines->data[i].data[1].data, lines->data[i].data[1].length, i);
        }
    }

    for (int i = 0; i < flags_size; i++) {
        remove_line(lines, flags[i].position - i);
        set_flag(flags[i].name, flags[i].name_length, flags[i].position - i);
    }
}

/*
    Goes through lines one-by-one and sends them for handling
*/
void process_code(lines_list_t* lines) {
    for (int i = 0; i < lines->length; i++) {
        handle_line(&lines->data[i]);
    }
}

/*
    Prints lines_list_t type as readable code to stdout
*/
void print_lines(lines_list_t* lines) {
    for (int i = 0; i < lines->length; i++) {
        printf("%d (%d): ", i + 1, lines->data[i].length);
        for (int j = 0; j < lines->data[i].length; j++) {
            char tmp[lines->data[i].data[j].length + 1];
            tmp[lines->data[i].data[j].length] = '\0';
            memcpy(tmp, lines->data[i].data[j].data, lines->data[i].data[j].length);
            printf("%s ", tmp);
        }
        printf("\n");
    }
}

/*
    Handles given line
*/
void handle_line(line_t* line) {

    if (line->length == 3 && case_insensitive_compare("=", line->data[1].data, strlen("="), line->data[1].length)) {
        if (is_number(line->data[2].data, line->data[2].length)) {
            int args[2] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length))};
            add_instruction(SET_N, 8, add_int_arguments(args, 2));
        }
        else {
            int args[2] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length)};
            add_instruction(SET_V, 8, add_int_arguments(args, 2));
        }
    }
    else if (case_insensitive_compare("if", line->data[0].data, strlen("if"), line->data[0].length)) {
        int condition_length = line->data[2].length;
        char condition[condition_length];
        memcpy(&condition, line->data[2].data, condition_length);

        if (case_insensitive_compare(">", condition, strlen(">"), condition_length)) {
            if (is_number(line->data[1].data, line->data[1].length)) {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_M_NN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_M_NV, 8, add_int_arguments(args, 2));
                }
            }
            else {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_M_VN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_M_VV, 8, add_int_arguments(args, 2));
                }
            }
        }
        else if (case_insensitive_compare("<", condition, strlen("<"), condition_length)) {
            if (is_number(line->data[1].data, line->data[1].length)) {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_L_NN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_L_NV, 8, add_int_arguments(args, 2));
                }
            }
            else {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_L_VN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_L_VV, 8, add_int_arguments(args, 2));
                }
            }
        }
        else if (case_insensitive_compare("=", condition, strlen("="), condition_length)) {
            if (is_number(line->data[1].data, line->data[1].length)) {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_E_NN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_E_NV, 8, add_int_arguments(args, 2));
                }
            }
            else {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_E_VN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_E_VV, 8, add_int_arguments(args, 2));
                }
            }
        }
        else if (case_insensitive_compare("!=", condition, strlen("!="), condition_length)) {
            if (is_number(line->data[1].data, line->data[1].length)) {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_D_NN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_D_NV, 8, add_int_arguments(args, 2));
                }
            }
            else {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_D_VN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_D_VV, 8, add_int_arguments(args, 2));
                }
            }
        }
        else if (case_insensitive_compare("<=", condition, strlen("<="), condition_length)) {
            if (is_number(line->data[1].data, line->data[1].length)) {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_LE_NN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_LE_NV, 8, add_int_arguments(args, 2));
                }
            }
            else {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_LE_VN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_LE_VV, 8, add_int_arguments(args, 2));
                }
            }
        }
        else if (case_insensitive_compare(">=", condition, strlen(">="), condition_length)) {
            if (is_number(line->data[1].data, line->data[1].length)) {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_ME_NN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {to_int(insert_null(line->data[1].data, line->data[1].length)), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_ME_NV, 8, add_int_arguments(args, 2));
                }
            }
            else {
                if (is_number(line->data[3].data, line->data[3].length)) {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), to_int(insert_null(line->data[3].data, line->data[3].length))};
                    add_instruction(IF_ME_VN, 8, add_int_arguments(args, 2));
                }
                else {
                    int args[2] = {get_var_id(line->data[1].data, line->data[1].length), get_var_id(line->data[3].data, line->data[3].length)};
                    add_instruction(IF_ME_VV, 8, add_int_arguments(args, 2));
                }
            }
        }
    }
    else if (line->length == 5 && case_insensitive_compare("+", line->data[3].data, strlen("+"), line->data[3].length)) {
        if (is_number(line->data[2].data, line->data[2].length)) {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(ADD_NN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(ADD_NV, 12, add_int_arguments(args, 3));
            }
        }
        else {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(ADD_VN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(ADD_VV, 12, add_int_arguments(args, 3));
            }
        }
    }
    else if (line->length == 5 && case_insensitive_compare("-", line->data[3].data, strlen("-"), line->data[3].length)) {
        if (is_number(line->data[2].data, line->data[2].length)) {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(SUB_NN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(SUB_NV, 12, add_int_arguments(args, 3));
            }
        }
        else {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(SUB_VN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(SUB_VV, 12, add_int_arguments(args, 3));
            }
        }
    }
    else if (line->length == 5 && case_insensitive_compare("*", line->data[3].data, strlen("*"), line->data[3].length)) {
        if (is_number(line->data[2].data, line->data[2].length)) {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(MUL_NN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(MUL_NV, 12, add_int_arguments(args, 3));
            }
        }
        else {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(MUL_VN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(MUL_VV, 12, add_int_arguments(args, 3));
            }
        }
    }
    else if (line->length == 5 && case_insensitive_compare("/", line->data[3].data, strlen("/"), line->data[3].length)) {
        if (is_number(line->data[2].data, line->data[2].length)) {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(DIV_NN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(DIV_NV, 12, add_int_arguments(args, 3));
            }
        }
        else {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(DIV_VN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(DIV_VV, 12, add_int_arguments(args, 3));
            }
        }
    }
    else if (case_insensitive_compare("print", line->data[0].data, strlen("print"), line->data[0].length)) {
        int mode_length = line->data[1].length;
        char mode[mode_length];
        memcpy(&mode, line->data[1].data, mode_length);

        if (case_insensitive_compare("$", mode, strlen("$"), mode_length)) {
            if (is_number(line->data[2].data, line->data[2].length)) {
                int args = to_int(insert_null(line->data[2].data, line->data[2].length));
                add_instruction(PRINT_NUM_N, 4, add_int_arguments(&args, 1));
            }
            else {
                int args = get_var_id(line->data[2].data, line->data[2].length);
                add_instruction(PRINT_NUM_V, 4, add_int_arguments(&args, 1));
            }
        }
        else if (case_insensitive_compare("ascii", mode, strlen("ascii"), mode_length)) {
            if (is_number(line->data[2].data, line->data[2].length)) {
                char args = to_int(insert_null(line->data[2].data, line->data[2].length)) & 0xFF;
                add_instruction(PRINT_ASCII_N, 1, add_bytes_argument(&args, 1));
            }
            else {
                int args = get_var_id(line->data[2].data, line->data[2].length);
                add_instruction(PRINT_ASCII_V, 4, add_int_arguments(&args, 1));
            }
        }
        else if (case_insensitive_compare("const", mode, strlen("const"), mode_length)) {
            char* tmp = malloc(0);
            int length = 0;
            for (int i = 2; i < line->length; i++) {
                tmp = realloc(tmp, length + line->data[i].length);
                memcpy(&tmp[length], line->data[i].data, line->data[i].length);
                length += line->data[i].length;

                if (i != line->length - 1) {
                    tmp = realloc(tmp, length + 1);
                    tmp[length] = ' ';
                    length++;
                }
            }
            length = convert_chars(tmp, length);
            add_instruction(PRINT_CONST_STRING, length, add_bytes_argument(tmp, length));
            free(tmp);
        }
        else if (case_insensitive_compare("string", mode, strlen("string"), mode_length)) {
            int args = get_str_id(line->data[2].data, line->data[2].length);
            add_instruction(PRINT_STRING, 4, add_int_arguments(&args, 1));
        }
    }
    else if (case_insensitive_compare("input", line->data[0].data, strlen("input"), line->data[0].length)) {
        int mode_length = line->data[1].length;
        char mode[mode_length];
        memcpy(&mode, line->data[1].data, mode_length);

        if (case_insensitive_compare("$", mode, strlen("$"), mode_length)) {
            int args = get_var_id(line->data[2].data, line->data[2].length);
            add_instruction(INPUT_NUM, 4, add_int_arguments(&args, 1));
        }
        else if (case_insensitive_compare("ascii", mode, strlen("ascii"), mode_length)) {
            int args = get_var_id(line->data[2].data, line->data[2].length);
            add_instruction(INPUT_ASCII, 4, add_int_arguments(&args, 1));
        }
        else if (case_insensitive_compare("string", mode, strlen("string"), mode_length)) {
            int args = get_str_id(line->data[2].data, line->data[2].length);
            add_instruction(INPUT_STRING, 4, add_int_arguments(&args, 1));
        }
    }
    else if (line->length == 5 && case_insensitive_compare("%", line->data[3].data, strlen("%"), line->data[3].length)) {
        if (is_number(line->data[2].data, line->data[2].length)) {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(MOD_NN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(MOD_NV, 12, add_int_arguments(args, 3));
            }
        }
        else {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(MOD_VN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(MOD_VV, 12, add_int_arguments(args, 3));
            }
        }
    }
    else if (case_insensitive_compare("end", line->data[0].data, strlen("end"), line->data[0].length)) {
        add_instruction(END, 0, 0);
    }
    else if (line->length == 5 && case_insensitive_compare("<=", line->data[3].data, strlen("<="), line->data[3].length)) {
        if (is_number(line->data[2].data, line->data[2].length)) {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_arr_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(ARRSET_NN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_arr_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(ARRSET_NV, 12, add_int_arguments(args, 3));
            }
        }
        else {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_arr_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(ARRSET_VN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_arr_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(ARRSET_VV, 12, add_int_arguments(args, 3));
            }
        }
    }
    else if (line->length == 5 && case_insensitive_compare("<=", line->data[1].data, strlen("<="), line->data[1].length)) {
        if (is_number(line->data[4].data, line->data[4].length)) {
            int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_arr_id(line->data[2].data, line->data[2].length), to_int(insert_null(line->data[4].data, line->data[4].length))};
            add_instruction(ARRGET_N, 12, add_int_arguments(args, 3));
        }
        else {
            int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_arr_id(line->data[2].data, line->data[2].length), get_var_id(line->data[4].data, line->data[4].length)};
            add_instruction(ARRGET_V, 12, add_int_arguments(args, 3));
        }
    }
    else if (line->length >= 2 && case_insensitive_compare("+=", line->data[1].data, strlen("+="), line->data[1].length)) {
        if (line->length >= 3) {
            if (is_number(line->data[2].data, line->data[2].length)) {
                int args[2] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length))};
                add_instruction(INC_N, 8, add_int_arguments(args, 2));
            }
            else {
                int args[2] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length)};
                add_instruction(INC_V, 8, add_int_arguments(args, 2));
            }
            
        }
        else {
            int args[2] = {get_var_id(line->data[0].data, line->data[0].length), 1};
            add_instruction(INC_N, 8, add_int_arguments(args, 2));
        }
    }
    else if (line->length >= 2 && case_insensitive_compare("-=", line->data[1].data, strlen("-="), line->data[1].length)) {
        if (line->length >= 3) {
            if (is_number(line->data[2].data, line->data[2].length)) {
                int args[2] = {get_var_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length))};
                add_instruction(DEC_N, 8, add_int_arguments(args, 2));
            }
            else {
                int args[2] = {get_var_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length)};
                add_instruction(DEC_V, 8, add_int_arguments(args, 2));
            }
            
        }
        else {
            int args[2] = {get_var_id(line->data[0].data, line->data[0].length), 1};
            add_instruction(DEC_N, 8, add_int_arguments(args, 2));
        }
    }
    else if (case_insensitive_compare("jump", line->data[0].data, strlen("jump"), line->data[0].length)) {
        int args = get_flag_position(line->data[1].data, line->data[1].length);
        add_instruction(JUMP, 4, add_int_arguments(&args, 1));
    }
    else if (case_insensitive_compare("free", line->data[0].data, strlen("free"), line->data[0].length)) {
        int mode_length = line->data[1].length;
        char mode[mode_length];
        memcpy(&mode, line->data[1].data, mode_length);

        if (case_insensitive_compare("array", mode, strlen("array"), mode_length)) {
            int args = get_arr_id(line->data[2].data, line->data[2].length);
            add_instruction(FREE_ARRAY, 4, add_int_arguments(&args, 1));
        }
        else if (case_insensitive_compare("string", mode, strlen("string"), mode_length)) {
            int args = get_str_id(line->data[2].data, line->data[2].length);
            add_instruction(FREE_STRING, 4, add_int_arguments(&args, 1));
        }
        
    }
    else if (case_insensitive_compare("exec", line->data[0].data, strlen("exec"), line->data[0].length)) {
        char* tmp = malloc(0);
        int length = 0;
        for (int i = 1; i < line->length; i++) {
            tmp = realloc(tmp, length + line->data[i].length);
            memcpy(&tmp[length], line->data[i].data, line->data[i].length);
            length += line->data[i].length;

            if (i != line->length - 1) {
                tmp = realloc(tmp, length + 1);
                tmp[length] = ' ';
                length++;
            }
        }
        length = convert_chars(tmp, length);
        add_instruction(EXEC, length, add_bytes_argument(tmp, length));
        free(tmp);
    }
    else if (case_insensitive_compare("load", line->data[0].data, strlen("load"), line->data[0].length)) {
        int mode_length = line->data[1].length;
        char mode[mode_length];
        memcpy(&mode, line->data[1].data, mode_length);

        char* tmp = malloc(0);
        int length = 0;
        for (int i = 3; i < line->length; i++) {
            tmp = realloc(tmp, length + line->data[i].length);
            memcpy(&tmp[length], line->data[i].data, line->data[i].length);
            length += line->data[i].length;

            if (i != line->length - 1) {
                tmp = realloc(tmp, length + 1);
                tmp[length] = ' ';
                length++;
            }
        }
        length = convert_chars(tmp, length);

        if (case_insensitive_compare("array", mode, strlen("array"), mode_length)) {
            int args[1] = {get_arr_id(line->data[2].data, line->data[2].length)};
            add_instruction(LOAD_ARRAY, 4 + length, add_int_arguments(args, 1));
        }
        else if (case_insensitive_compare("string", mode, strlen("string"), mode_length)) {
            int args[1] = {get_str_id(line->data[2].data, line->data[2].length)};
            add_instruction(LOAD_STRING, 4 + length, add_int_arguments(args, 1));
        }

        add_bytes_argument(tmp, length);
        free(tmp);
    }
    else if (case_insensitive_compare("save", line->data[0].data, strlen("save"), line->data[0].length)) {
        int mode_length = line->data[1].length;
        char mode[mode_length];
        memcpy(&mode, line->data[1].data, mode_length);

        char* tmp = malloc(0);
        int length = 0;
        for (int i = 3; i < line->length; i++) {
            tmp = realloc(tmp, length + line->data[i].length);
            memcpy(&tmp[length], line->data[i].data, line->data[i].length);
            length += line->data[i].length;

            if (i != line->length - 1) {
                tmp = realloc(tmp, length + 1);
                tmp[length] = ' ';
                length++;
            }
        }
        length = convert_chars(tmp, length);

        if (case_insensitive_compare("array", mode, strlen("array"), mode_length)) {
            int args[1] = {get_arr_id(line->data[2].data, line->data[2].length)};
            add_instruction(SAVE_ARRAY, 4 + length, add_int_arguments(args, 1));
        }
        else if (case_insensitive_compare("string", mode, strlen("string"), mode_length)) {
            int args[1] = {get_str_id(line->data[2].data, line->data[2].length)};
            add_instruction(SAVE_STRING, 4 + length, add_int_arguments(args, 1));
        }

        add_bytes_argument(tmp, length);
        free(tmp);
    }
    else if (case_insensitive_compare("call", line->data[0].data, strlen("call"), line->data[0].length)) {
        int args = get_flag_position(line->data[1].data, line->data[1].length);
        add_instruction(PUSHJUMP, 4, add_int_arguments(&args, 1));
    }
    else if (case_insensitive_compare("return", line->data[0].data, strlen("return"), line->data[0].length)) {
        add_instruction(RET, 0, 0);
    }
    else if (case_insensitive_compare("arrsize", line->data[0].data, strlen("arrsize"), line->data[0].length)) {
        int args[2] = {get_var_id(line->data[1].data, line->data[1].length), get_arr_id(line->data[2].data, line->data[2].length)};
        add_instruction(ARRSIZE, 8, add_int_arguments(args, 2));
    }
    else if (case_insensitive_compare("cat", line->data[0].data, strlen("cat"), line->data[0].length)) {
        int args = {get_str_id(line->data[1].data, line->data[1].length)};

        char* tmp = malloc(0);
        int length = 0;
        for (int i = 2; i < line->length; i++) {
            tmp = realloc(tmp, length + line->data[i].length);
            memcpy(&tmp[length], line->data[i].data, line->data[i].length);
            length += line->data[i].length;

            if (i != line->length - 1) {
                tmp = realloc(tmp, length + 1);
                tmp[length] = ' ';
                length++;
            }
        }

        length = convert_chars(tmp, length);
        add_instruction(CAT, 4 + length, add_int_arguments(&args, 1));
        add_bytes_argument(tmp, length);
        free(tmp);
    }
    else if (line->length == 5 && case_insensitive_compare("<-", line->data[3].data, strlen("<-"), line->data[3].length)) {
        if (is_number(line->data[2].data, line->data[2].length)) {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_str_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(STRSET_NN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_str_id(line->data[0].data, line->data[0].length), to_int(insert_null(line->data[2].data, line->data[2].length)), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(STRSET_NV, 12, add_int_arguments(args, 3));
            }
        }
        else {
            if (is_number(line->data[4].data, line->data[4].length)) {
                int args[3] = {get_str_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), to_int(insert_null(line->data[4].data, line->data[4].length))};
                add_instruction(STRSET_VN, 12, add_int_arguments(args, 3));
            }
            else {
                int args[3] = {get_str_id(line->data[0].data, line->data[0].length), get_var_id(line->data[2].data, line->data[2].length), get_var_id(line->data[4].data, line->data[4].length)};
                add_instruction(STRSET_VV, 12, add_int_arguments(args, 3));
            }
        }
    }
    else if (line->length == 5 && case_insensitive_compare("<-", line->data[1].data, strlen("<-"), line->data[1].length)) {
        if (is_number(line->data[4].data, line->data[4].length)) {
            int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_str_id(line->data[2].data, line->data[2].length), to_int(insert_null(line->data[4].data, line->data[4].length))};
            add_instruction(STRGET_N, 12, add_int_arguments(args, 3));
        }
        else {
            int args[3] = {get_var_id(line->data[0].data, line->data[0].length), get_str_id(line->data[2].data, line->data[2].length), get_var_id(line->data[4].data, line->data[4].length)};
            add_instruction(STRGET_V, 12, add_int_arguments(args, 3));
        }
    }
    else if (case_insensitive_compare("strsize", line->data[0].data, strlen("strsize"), line->data[0].length)) {
        int args[2] = {get_var_id(line->data[1].data, line->data[1].length), get_str_id(line->data[2].data, line->data[2].length)};
        add_instruction(STRSIZE, 8, add_int_arguments(args, 2));
    }
}
