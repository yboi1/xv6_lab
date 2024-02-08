#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
find(char* path, const char* target){
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: argument error... %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0 ){
        fprintf(2, "cannot fstat  %s error...\n", path);
        close(fd);
        return;
    }

     // 判断目录长度                    // 为何可以不加括号
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
        printf("ls: path too long\n");
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        // 目录中文件的个数
        if(de.inum == 0)    continue;

        // 将整个儿目录拷贝到p 中
        memmove(p, de.name, DIRSIZ);    //添加路径名称
        p[DIRSIZ] = 0;                  // 字符串结束标志

        if(stat(buf, &st) < 0){
            printf("find: cannot stat %s\n", buf);
            continue;
        }

        // 判断题目要求
        if(st.type==T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..")!=0){
            find(p, target);
        }
        else if(strcmp(p, target) == 0){
            printf("%s ", buf);
        }
                // }
                // if((strcmp(fmtname(buf), target)) == 0){
                //     printf("same !!!\n");
                //     printf("%s  ", fmtname(buf));
                // }
            
    }    
    printf("\n");
    close(fd);           
}

int main(int argc, char* argv[]){
    if(argc<3){
        fprintf(1, "argument num error...\n");
    }
    else{
        find(argv[1], argv[2]);
    }
    exit(0);
}