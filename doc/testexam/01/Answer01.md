
###Accepted
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

