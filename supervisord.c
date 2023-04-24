#include <stdio.h>
#include <glob.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "ini.h"
#include "stdstring.h"
#include "list.h"

#define sizearray(a)  (sizeof(a) / sizeof((a)[0]))

typedef struct program {
    char *name;
    char *command;
    bool is_autostart;
    bool is_autorestart;
    bool is_running;
    pid_t pid;
    struct list_head list;
} program_t;

static LIST_HEAD(program_list);
static char *conf_file = "/home/wwd/all/libwwd/src/tools/c/0025_c_supervisord/conf.d/*.conf";
static int exiting = 0;
static int got_sigchld = 0;

static void handle_signal(int sig)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            exiting = 1;
            break;
        case SIGCHLD:
            got_sigchld = 1;
            break;
    }
}

static int start_program(program_t *prog)
{
    char *args[1] = {NULL};

    if (prog->is_running)
        return 0;
    
    int child_pid;
    switch (child_pid = fork()) {
        case -1: 
            return -1;
        case 0:
            // TBD
            execv(prog->command, args);
            exit(1);
        default:
            prog->is_running = true;
            prog->pid = child_pid;
            printf("[Start] program: %s %s pid=%d\n", prog->name, prog->command, prog->pid);
            break;
    }
    return 0;
}

static program_t* get_program(pid_t pid)
{
    program_t *prog;
    list_for_each_entry(prog, &program_list, list) {
        if (prog->pid == pid) {
            return prog;
        }
    }
    return NULL;
}

static void handle_sigchld()
{
    program_t *prog;
    int killed_pid;

    do {
        killed_pid = waitpid(-1, 0, WNOHANG);
        if (killed_pid > 0) {
            prog = get_program(killed_pid);

            if (prog) {
                prog->is_running = 0;
                prog->pid = 0;
                printf("[Exited] program: %s pid=%d\n", prog->name, killed_pid);

                if (prog->is_autorestart) {
                    start_program(prog);
                }
            }
        }
    } while (killed_pid > 0);
}

static program_t* parse_conf(char* conf_file)
{
    char section[256] = {0};
    char key[256] = {0};
    char command[256] = {0};
    int s, k;

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
                for (k = 0; ini_getkey(section, k, key, sizearray(key), conf_file) > 0; k++) {
                    if (!strncmp(key, "command", strlen("command"))) {
                        ini_gets(section, key, "dummy", command, sizearray(command), conf_file);
                        prog->command = strdup(command);
                    }
                } 
                prog->name = strdup(tokens[1]);
                prog->is_autostart = true;
                prog->is_autorestart = false;
                prog->is_running = false;

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
        if (prog->is_running) {
            printf("Kill child process: %s pid: %d\n", prog->name, prog->pid);
            kill(prog->pid, SIGKILL);
        }
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

static int reload_conf(void)
{
    glob_t globres;
    int i;

    clear_programs();
    if (glob(conf_file, 0, NULL, &globres))
        return -1;
    for(i = 0; i < globres.gl_pathc; i++) {
        printf("%s\n", globres.gl_pathv[i]);
        program_t *prog = parse_conf(globres.gl_pathv[i]);
        if (prog) {
            append_program(prog);
        }
    }
    globfree(&globres);
    return 0;
}

static void cleanup()
{
    clear_programs();
}

int main()
{
    /* load config file */
    if (reload_conf() != 0) {
        return 1;
    }

    /* install signal handler */
    struct sigaction handler;
    handler.sa_handler = handle_signal;
    sigfillset(&handler.sa_mask);
    handler.sa_flags=0;
    sigaction(SIGINT,&handler,0);
    sigaction(SIGTERM,&handler,0);
    sigaction(SIGHUP,&handler,0);
    handler.sa_flags = SA_RESTART|SA_NOCLDSTOP;
    sigaction(SIGCHLD,&handler,0);

    program_t *prog;
    list_for_each_entry(prog, &program_list, list) {
        if (prog->is_autostart) {
            start_program(prog);
        }
    }

    while (!exiting) {
        sleep(1);
        if (got_sigchld) {
            handle_sigchld();
            got_sigchld = 0;
        }
    }

    printf("cleanup...\n");
    cleanup();
    return 0;
}