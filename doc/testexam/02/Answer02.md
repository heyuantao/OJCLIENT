###Java代码
```JAVA
import java.util.*;
public class Main{
    public static void main(String args[]){
        int n,i,t;
        int[] s = new int[100];
        Scanner cin = new Scanner(System.in);
        while (cin.hasNext()){
            n = cin.nextInt();
            if(n==0){
                return;
            }
            for(i=1;i<5;i++){
                s[i]=i;
            }
            if(n>=5){
                for(i=5;i<=n;i++){   //第四年后
                    s[i]=s[i-3]+s[i-1];
                }
            }
            System.out.println(s[n]);
        }
    }
}
```


###C代码
```C
int main(){
    int n,i,s[100],t;       
    while (scanf("%d",&n)!=EOF&&n!=0){
        for(i=1;i<5;i++){
           s[i]=i;
        }
        if(n>=5){
           for(i=5;i<=n;i++){   //第四年后
              s[i]=s[i-3]+s[i-1];
           }
        }
        printf("%d\n",s[n]);
    }
    return 0;
}

```
