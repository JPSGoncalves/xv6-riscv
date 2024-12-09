#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
        int ret, sec = 0;

        if(argc <= 1){
                printf("%s: needs number of seconds argument\n", argv[0]);
                exit(1);
        }

        sec = atoi(argv[1]);

        ret = sleep(sec);

        printf("%s: ret %d\n", argv[0], ret);
        exit(ret);
}
