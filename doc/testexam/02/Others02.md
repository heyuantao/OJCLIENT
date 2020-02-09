##错误答案测试用例
### Wrong Answer 答案错误
```JAVA
import java.util.*;
public class Main{
    public static void main(String args[]){
        int n;
        Scanner cin = new Scanner(System.in);
        while (cin.hasNext()){
            n = cin.nextInt();
            if(n==0){
                return;
            }
            System.out.println("hello");
        }
    }
}
```

### Comiple Error 编译错误
```JAVA
import java.util.*;
public class Main{
    public static void main(String args[]){
        int n;
        Scanner cin = new Scanner(System.in);
        while (cin.hasNext()){
            n = cin.nextInt();
            if(n==0){
                return;
            }
            abc();
        }
    }
}
```

### Memory Limit 内存超限
无测试用例



### Output Limit 输出超限
```JAVA
import java.util.*;
public class Main{
    public static void main(String args[]){
        int n;
        Scanner cin = new Scanner(System.in);
        while (cin.hasNext()){
            n = cin.nextInt();
            if(n==0){
                return;
            }
            while(true){
                System.out.println("hello");
            }
        }
    }
}
```

### Time Limit 时间超限
```JAVA
import java.util.*;
public class Main{
    public static void main(String args[]){
        int n;
        Scanner cin = new Scanner(System.in);
        while (cin.hasNext()){
            n = cin.nextInt();
            if(n==0){
                return;
            }
            while(true){
                int[] p = new int[10];
            }
        }
    }
}
```

### Part Error 答案部分正确
```JAVA
import java.util.*;
public class Main{
    public static void main(String args[]){
        int n;
        Scanner cin = new Scanner(System.in);
        while (cin.hasNext()){
            n = cin.nextInt();
            if(n==0){
                return;
            }
            if(n==2){
                System.out.println("2");
                continue;
            }
            if(n==4){
                System.out.println("4");
                continue;
            }
            if(n==5){
                System.out.println("6");
                continue;
            }
            System.out.println("100");
        }
    }
}
```