&emsp;&emsp;该软件是判题机，所谓判题机就是根据用户提交的代码和测试数据来判定代码正确性的程序。
判题机一般用于程序设计语言的学习和考试系统，例如用于ACM竞赛的HUSTOJ就有一款用C语言编写的判题机。  

&emsp;&emsp;本软件的思路来源正是HUSTOJ，由于HUSTOJ的判题机代码使用C语言来编写，且组织比较凌乱
不利于二次开发。作者根据其思路用C++语言重写了判题机，由于考虑到判题机的特殊工作原理，该判题机使用
HTTP来与其他系统进行对接。在开发的过程中作者简化了其中的某些机制，即没有使用“内存盘”的方案，这样在
某种程度上会影响到性能，但既然考虑到分布式部署的方案，可以用多个判题机来解决性能问题。

&emsp;&emsp;软件安装方式如下  

1.该软件仅仅在ubuntu16.04（64）系统下编译并测试，在编译前请按照如下的C++库，即Boost和Casablanca。

apt-get install libboost-dev libboost-system-dev libboost-filesystem-dev libcpprest2.8 libcpprest-dev

2.添加判题的用户,即ID为1536和名字为judge的用户  

/usr/sbin/useradd -m -u 1536 judge

3.设置权限，即确保"judge"用户对"/home/judge/runx"（x为某个数字）有写权限  

4.编译软件，软件使用cmake进行构建，编译完成后会生成"DAEMON"和"JUDGE"两个可执行程序,把这两个文件复制到"/usr/local/bin/"目录下,并测试下是否能判题。该文件内容参考doc/judge.conf.example。

JUDGE 1001 0 

其中1001为solution的编号,0为任务编号，即该任务在/home/judge/run0/的目录下运行

5.创建配置文件，即创建"/home/judge/etc/judge.conf"。"DAEMON"和"JUDGE"会使用这个配置文件来与外部系统进行通信。
