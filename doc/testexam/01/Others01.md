##错误答案测试用例
### Wrong Answer 答案错误
```  C
#include<stdio.h> 
#include<math.h>
int main(){
  printf("hello");
}
```

### Comiple Error 编译错误
```C
#include <stdio.h>
#include <stdlib.h>
int main(){
    abc();
}
```

### Memory Limit 内存超限

```C
#include <stdio.h>
#include <stdlib.h>
int main(){
    char *p;
    while(1){
        p = (char *)malloc(sizeof(char *)*100);
    }
}
```

###  Output Limit 输出超限
```C
int main(){
    while(1){
        printf("Endless output !");
    }
}
```

### Time Limit 超时
```C
int main(){
    while(1){
        
    }
}
```

### Part Error 答案部分正确
```C
#include <stdio.h>
#include<math.h>
int main()
{
	const double pi =acos(-1.0);
	double r,h,s1,s2,s;
	scanf("%lf%lf",&h,&r);
	s1=pi*r*r;
	s2=2*pi*r*h;
	s=s1*2.0+s2;
	printf("Area=9896.02");    //274.89
	return 0;
}
```