# Service 
定义软件服务的文件通常是systemd服务单元文件，具有.service后缀。具体目录如下：
  > /etc/systemd/system/
  > /usr/lib/systemd/system
  > /lib/systemd/system
  *注意：它们之间是有优先级的，自行查阅资料
## 格式说明
service文件通常由三部分组成：
  > [Unit]：定义与Unit类型无关的通用选项，用于提供unit的描述信息、unit行为及依赖关系等
  > [Service]：与特定类型相关的专用选项，此处为Service类型
  > [Install]：定义由“systemctl enable”以及"systemctl disable“命令在实现服务启用或禁用时用到的一些选项
  .注意: 以#开头的行后面的内容会被认为是注释[1、yes、on、true代表开启，0、no、off、false代表关闭],
         时间单位默认是秒，可以用毫秒（ms）分钟（m）等须显式说明
  ^使用方式 key=value
### Unit
  Option:
    [Description]: 环境配置文件，用来指定当前服务启动的环境变量
    [ExecStart]: 指定服务启动时执行的命令或脚本
    [ExecStartPre]: 指定服务启动前执行的命令或脚本
    [ExecStartPost]: 指定服务启动后执行的命令或脚本
    [ExecStop]: 指明停止服务要运行的命令或脚本
    [ExecStopPost]: 指定服务停止之后执行的命令或脚本
    [RestartSec]: 指定服务在重启时等待的时间，单位为秒
    [ExecReload]: 指明重启服务要运行的命令或脚本
    [Restart]: 当设定Restart=1 时，则当次daemon服务意外终止后，会再次自动启动此服务，具体看下列类型
    [PrivateTmp]: 设定为yes时，会在生成/tmp/systemd-private-UUID-NAME.service-XXXXX/tmp/目录
    [KillMode]: 指定停止的方式，具体见下面
    [Restart]: 指定重启时的类型，具体见下面
    [Type]: 指定启动类型，具体见下面
  Type:[OPTION]
    [simple]: 指定ExecStart字段的进程为主进程
    [forking]: 指定以fork() 子进程执行ExecStart字段的进程
    [oneshot]: 执行一次
    [notify]: 启动后发送会发送通知信号通知systemd
    [idle]: 等其他任务结束后才运行
  Restart:[OPTION]
    [no]: 退出后不会重启
    [on-success]: 当进程正常退出时(退出码为0) 执行重启
    [on-failure]: 当进程不正常退出时(退出码不为0) 执行重启
    [on-abnormal]: 当被信号终止和超时执行重启
    [on-abort]: 当收到没有捕捉到的信号终止时执行重启
    [on-watchdog]: 当看门狗超时时执行重启
    [always]: 一直重启
  KillMode:[OPTION]
    [control-group]: 杀掉当前进程中所有的进程
    [process]: 杀掉当前进程的主进程
    [mixed]: 主进程将收到 SIGTERM 信号，子进程收到 SIGKILL 信号
    [none]: 不杀掉任何进程

### Install
  Option:
    [Alias]: 别名，可使用systemctl command Alias.service
    [RequiredBy]: 被哪些units所依赖，强依赖
    [WantedBy]: 被哪些units所依赖，弱依赖
    [Also]: 安装本服务的时候还要安装别的相关服务
  .注意: 一般填为WantedBy=multi-user.target
.注意: 对于新创建的unit文件获取修改unit文件需要重现加载systemd的配置，使用systemctl daemon-reload

*命令讲解
  Description: 服务命令systemctl {name 是服务名称 创建文件时 name.service}
  开机启动服务: systemctl enabled [name]
  启动服务: systemctl start [name]
  停止服务: systemctl stop [name]
  重启服务：systemctl restart [name]
  查看服务状态：systemctl status [name]
  重写加载配置：systemctl daemon-reload
  [其余命令请查阅资料]
