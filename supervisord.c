#include <stdio.h>
#include <glob.h>
#include <stdlib.h>

#include "ini.h"
#include "stdstring.h"
#include "list.h"

#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))

typedef struct program {
    char *name;
    char *command;
    struct list_head list;
} program_t;

static LIST_HEAD(program_list);

static program_t* parse_conf(char* conf_file)
{
    char section[256] = {0};
    int s;

    for (s = 0; ini_getsection(s, section, sizearray(section), conf_file) > 0; s++) {
        if (string_starts_with(section, "program:")) {
            char *tokens[2];
            printf("seciton:%s\n", section);
            size_t count = string_split(section, ":", tokens, 2);
            if (count == 2) {
                program_t* prog = malloc(sizeof(program_t));
                memset(prog, 0, sizeof(*prog));
                if (!prog) {
                    fprintf(stderr, "Unable to allocate memory for program!\n");
                    return NULL;
                }
                prog->name = strdup(tokens[1]);
                return prog;
            }
            for(int j=0; j<count; j++)
                free(tokens[j]);
        }
    }

    return NULL;
}

static void clear_programs()
{
    program_t *prog;
    while(!list_empty(&program_list)) {
        prog = list_first_entry(&program_list, program_t, list);
        if (prog->name) free(prog->name);
        if (prog->command) free(prog->command);
        list_del(&prog->list);
        free(prog);
    }
}

static void append_program(program_t *prog)
{
    list_add_tail(&prog->list, &program_list);
}

int main()
{
    char *conf="/home/wwd/all/libwwd/src/tools/c/0025_c_supervisord/conf.d/daemon-configs/conf.d/*.conf";
    glob_t globres;
    int i;

    if (glob(conf, 0, NULL, &globres))
        return -1;
    for(i = 0; i < globres.gl_pathc; i++) {
        printf("%s\n", globres.gl_pathv[i]);
        program_t *prog = parse_conf(globres.gl_pathv[i]);
        if (prog) {
            append_program(prog);
        }
    }
    globfree(&globres);
    clear_programs();
    return 0;
}