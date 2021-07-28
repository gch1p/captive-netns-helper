#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <getopt.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

#define myerror(f_, ...) { \
        fprintf(stderr, (f_), ##__VA_ARGS__); \
        rc = 1; \
        goto end; \
    }

#define stderror(prefix_) myerror(prefix_ ": %s.\n", strerror(errno)); \


static void usage(char* progname)
{
    printf("Usage: %s OPTIONS COMMAND [ARGS...]\n\n", progname);
    printf("Options:\n"
           "    -h, --help: show this help\n"
           "    -n, --nameserver NAMESERVER\n"
           "    -f, --ns-file FILE\n"
           "    -u, --uid UID\n"
           "    -g, --gid GID\n"
           "    -e, --env VAR=VALUE\n"
           );
}

int main(int argc, char* argv[])
{
    int rc = 0;
    int temp_fd = 0;
    int ns_fd = 0;
    uid_t uid = 0;
    uid_t gid = 0;
    char cwd[PATH_MAX];
    char temp_name[PATH_MAX] = {0};
    char *ns_file = NULL;
    char *nameserver = NULL;
    bool ismounted;

    int envi = 0;
    size_t envn = sizeof(char*)*(argc/2);
    char **envp = malloc(envn);
    memset(envp, 0, envn);

    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    int opt;
    struct option long_options[] = {
        {"help",       no_argument,       NULL, 'h'},
        {"nameserver", required_argument, NULL, 'n'},
        {"ns-file",    required_argument, NULL, 'f'},
        {"uid",        required_argument, NULL, 'u'},
        {"gid",        required_argument, NULL, 'g'},
        {"env",        required_argument, NULL, 'e'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "+hn:f:u:g:", long_options, NULL)) != EOF) {
        switch (opt) {
        case 'h':
            usage(argv[0]);
            return 0;

        case 'n':
            nameserver = optarg;
            break;

        case 'f':
            ns_file = optarg;
            break;

        case 'u':
            uid = (uid_t)atoi(optarg);
            break;

        case 'g':
            gid = (uid_t)atoi(optarg);
            break;

        case 'e':
            if (envi == envn-1)
                fprintf(stderr, "warn: skipping --env %s\n", optarg);
            else
                envp[envi++] = optarg;
            break;

        default:
            break;
        }
    }

    if (nameserver == NULL)
        myerror("error: --nameserver is required\n");

    if (ns_file == NULL)
        myerror("error: --ns-file is required\n");

    if (geteuid() != 0)
        myerror("error: you must be root.\n");

    if (unshare(CLONE_NEWNS) == -1)
        stderror("unshare");

    /* save cwd */
    getcwd(cwd, PATH_MAX);

    /* create temp file */
    strcpy(temp_name, "/tmp/capresolv.XXXXXX");
    temp_fd = mkstemp(temp_name);
    dprintf(temp_fd, "nameserver %s\n", nameserver);
    close(temp_fd);
    temp_fd = 0;

    chmod(temp_name, 0644);

    /* mount /etc/resolv.conf */
    if (mount(temp_name, "/etc/resolv.conf", NULL, MS_BIND, NULL) == -1)
        stderror("mount");
    ismounted = true;

    ns_fd = open(ns_file, O_RDONLY);
    if (ns_fd == -1)
        stderror("open");

    /* change to netns */
    if (setns(ns_fd, CLONE_NEWNET) == -1)
        stderror("setns");

    pid_t pid = fork();
    if (!pid) {
        /* change real and effective user and group (group first, then user) */
        if (gid != getegid()) {
            if (setregid(gid, gid) == -1)
                stderror("setregid");
        }

        if (uid != geteuid()) {
            if (setreuid(uid, uid) == -1)
                myerror("setreuid");
        }

        /* restore cwd */
        if (chdir(cwd) == -1)
            stderror("chdir");

        /* launch program */
        if (execvpe(argv[optind], (char *const *)argv+optind, envp) == -1)
            stderror("execvpe");

        return 0;
    }

    waitpid(pid, NULL, 0);

end:
    if (ismounted)
        umount(temp_name);

    if (temp_fd > 0)
        close(temp_fd);

    if (ns_fd > 0)
        close(ns_fd);

    if (temp_name[0] != 0)
        unlink(temp_name);

    return rc;
}
