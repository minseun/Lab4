#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int strerror_r(int errnum, char *strerrbuf, size_t buflen) {
    char *buf;
    int error1, error2, error3;
    sigset_t maskblock, maskold;

    if (sigfillset(&maskblock) == -1 || sigprocmask(SIG_SETMASK, &maskblock, &maskold) == -1)
        return errno;

    if ((error1 = pthread_mutex_lock(&lock)) != 0) {
        sigprocmask(SIG_SETMASK, &maskold, NULL);
        return error1;
    }

    buf = strerror(errnum);
    
    if (strlen(buf) >= buflen)
        error1 = ERANGE;
    else
        strcpy(strerrbuf, buf);

    error2 = pthread_mutex_unlock(&lock);
    error3 = sigprocmask(SIG_SETMASK, &maskold, NULL);

    return error1 ? error1 : (error2 ? error2 : error3);
}

int perror_r(const char *s) {
    int error1, error2;
    sigset_t maskblock, maskold;

    if (sigfillset(&maskblock) == -1 || sigprocmask(SIG_SETMASK, &maskblock, &maskold) == -1)
        return errno;

    if ((error1 = pthread_mutex_lock(&lock)) != 0) {
        sigprocmask(SIG_SETMASK, &maskold, NULL);
        return error1;
    }

    perror(s);

    error1 = pthread_mutex_unlock(&lock);
    error2 = sigprocmask(SIG_SETMASK, &maskold, NULL);

    return error1 ? error1 : error2;
}

int main() {
    char buf[256];
    int errnum = 2;

    int result = strerror_r(errnum, buf, sizeof(buf));
    if (result == 0) {
        printf("Error message: %s\n", buf);
    } else {
        printf("strerror_r failed with error code: %d\n", result);
    }

    perror_r("Custom error message");

    return 0;
}
