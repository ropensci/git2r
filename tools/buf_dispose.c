#include <git2.h>

int main()
{
    git_buf buf = {0};
    git_buf_dispose(&buf);
    return 0;
}
