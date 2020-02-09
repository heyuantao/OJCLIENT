
###Accepted
答案(C的版本)
``` C
#include<stdio.h> 
#include<math.h>
int main()
{
	const double pi =acos(-1.0);
	double r,h,s1,s2,s;
	scanf("%lf%lf",&h,&r);
	s1=pi*r*r;
	s2=2*pi*r*h;
	s=s1*2.0+s2;
	printf("Area=%.2f\n",s);
	return 0;
}
```
###Accepted
答案(JAVA的版本)
```JAVA
import java.util.*;
import java.lang.Math;
public class Main {
    public static void main(String[] args) {
        Scanner s=new Scanner(System.in);
        while(s.hasNext()){
            String str=s.nextLine();
            String[] line_list = str.split(" ");
            double h,r;
            h = Double.parseDouble(line_list[0]);
            r = Double.parseDouble(line_list[1]);
            double s1,s2,area;
            s1=Math.PI*r*r;
            s2=2*Math.PI*r*h;
            area=s1*2.0+s2;
            String ss = String.format("%1.2f", area);
            System.out.println("Area="+ss);
        }
    }
}
```

