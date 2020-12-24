#include "tools.h"
#include <stdio.h>
#include "bank.h"
#include <signal.h>
#define BUF_SIZE (4096)
void open_acc();
void login();
void destory();
void unlock();
void save();
void transfer();
void pwd();
void take();
void query();

int bank_id = 10000;
int svr_fd;

//ctrl c杀死进程
void sigint(int signum)
{
	close(svr_fd);
	printf("关闭");
	exit(0);
}
void *server(void *arg)
{
	int cli_fd = *(int *)arg;
	Account *acc = malloc(sizeof(Account));
	for (;;)
	{
		char buf[4096];
		int recv_size = read(cli_fd, buf, BUF_SIZE);
		if (0 >= recv_size)
		{
			printf("客户端%d退出\n", cli_fd);
			close(cli_fd);
			pthread_exit(NULL);
		}
		sscanf(buf, "%d %s %s %s %f %d", &acc->type, acc->bank, acc->card, acc->pwd, &acc->balance, &acc->lock);
		switch (acc->type)
		{
		case M_OPEN:
			open_acc(acc, buf);
			break; //开户
		case M_LOGIN:
			login(acc, buf);
			break; //登录
		case M_DESTORY:
			destory(acc, buf);
			break; //销户
		case M_UNLOCK:
			unlock(acc, buf);
			break; //解锁
		case M_SAVE:
			save(acc, buf);
			break; //存钱
		case M_TAKE:
			take(acc, buf);
			break; //取款
		case M_QUERY:
			query(acc, buf);
			break; //查询
		case M_TRANSFER:
			transfer(acc, buf);
			break; //转账
		case M_REPASS:
			pwd(acc, buf);
			break; //改密码
		}
		write(cli_fd, buf, strlen(buf) + 1);
	}
}

//开户
void open_acc(Account *acc, char *buf)
{
	sprintf(acc->bank, "%d", bank_id);
	sprintf(acc->pwd, "123456");
	acc->lock = 0;
	char path[256];
	sprintf(path, "usr/%s", acc->bank);
	int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
	if (0 > fd)
	{
		error("open");
		sprintf(buf, "服务器升级");
		return;
	}
	sprintf(buf, "开户成功,卡号:%s 密码:%s", acc->bank, acc->pwd);
	bank_id++;
	write(fd, acc, sizeof(Account));
	close(fd);
}
//登录
void login(Account *acc, char *buf)
{
	char path[256] = {};
	sprintf(path, "usr/%s", acc->bank);
	// 判断银行卡号是否正确
	if (0 != access(path, F_OK))
	{
		sprintf(buf, "N:卡号不存在，请检查!");
		return;
	}

	int fd = open(path, O_RDWR);
	if (0 > fd)
	{
		error("open");
		sprintf(buf, "N:服务器正在升级，登陆失败!");
		return;
	}
	//读取文件内容
	Account bcc = {};
	read(fd, &bcc, sizeof(Account));

	//判断此帐号是否被锁定
	if (bcc.lock >= 3)
	{
		sprintf(buf, "N:此账号已经锁定，请解锁!");
		return;
	}

	if (strcmp(acc->pwd, bcc.pwd))
	{
		bcc.lock++;
		sprintf(buf, "N:密码错误");
		if (bcc.lock >= 3)
		{
			sprintf(buf, "N:此账号已经锁定，请解锁!");
		}
	}
	else
	{
		bcc.lock = 0;
		sprintf(buf, "Y:恭喜您登陆成功!");
	}
	lseek(fd, 0, SEEK_SET);
	write(fd, &bcc, sizeof(Account));
	close(fd);
}
//销户
void destory(Account *acc, char *buf)
{
	char path[256] = {};
	sprintf(path, "usr/%s", acc->bank);
	// 判断银行卡号是否正确
	if (0 != access(path, F_OK))
	{
		puts("此卡号不存在");
		sprintf(buf, "N:卡号不存在，请检查!");
		return;
	}

	int fd = open(path, O_RDWR);
	if (0 > fd)
	{
		error("open");
		sprintf(buf, "销户失败");
		return;
	}

	Account bcc = {};
	read(fd, &bcc, sizeof(Account));
	close(fd);

	if (strcmp(bcc.card, acc->card))
	{
		sprintf(buf, "身份证号不正确");
		return;
	}

	if (strcmp(bcc.pwd, acc->pwd))
	{
		sprintf(buf, "密码不正确");
		return;
	}

	if (remove(path))
	{
		error("remove");
		sprintf(buf, "销户失败");
		return;
	}

	sprintf(buf, "成功");
}
//解锁
void unlock(Account *acc, char *buf)
{
	char path[256] = {};
	sprintf(path, "usr/%s", acc->bank);
	// 判断银行卡号是否正确
	if (0 != access(path, F_OK))
	{
		sprintf(buf, "卡号不存在");
		return;
	}

	int fd = open(path, O_RDWR);
	if (0 > fd)
	{
		error("open");
		sprintf(buf, "解锁失败!");
		return;
	}

	Account bcc = {};
	read(fd, &bcc, sizeof(Account));
	if (strcmp(bcc.card, acc->card))
	{
		sprintf(buf, "解锁失败!");
		return;
	}

	if (strcmp(bcc.pwd, acc->pwd))
	{
		sprintf(buf, "解锁失败!");
		return;
	}

	bcc.lock = 0;
	lseek(fd, 0, SEEK_SET);
	write(fd, &bcc, sizeof(Account));
	close(fd);

	sprintf(buf, "解锁成功!");
}

//存款
void save(Account *acc, char *buf)
{
	char path[256] = {};
	sprintf(path, "usr/%s", acc->bank);

	int fd = open(path, O_RDWR);
	if (0 > fd)
	{
		error("open");
		sprintf(buf, "存款失败!");
		return;
	}

	Account bcc = {};
	read(fd, &bcc, sizeof(Account));

	bcc.balance += acc->balance;
	lseek(fd, 0, SEEK_SET);
	write(fd, &bcc, sizeof(Account));
	close(fd);

	sprintf(buf, "当前余额为:%g!", bcc.balance);
}

//取款
void take(Account *acc, char *buf)
{
	char path[256] = {};
	sprintf(path, "usr/%s", acc->bank);

	int fd = open(path, O_RDWR);
	if (0 > fd)
	{
		error("open");
		sprintf(buf, "取款失败!");
		return;
	}

	Account bcc = {};
	read(fd, &bcc, sizeof(Account));

	if (bcc.balance < acc->balance)
	{
		sprintf(buf, "余额不足当前余额为:%g!", bcc.balance);
		return;
	}

	bcc.balance -= acc->balance;
	lseek(fd, 0, SEEK_SET);
	write(fd, &bcc, sizeof(Account));
	close(fd);

	sprintf(buf, "取款成功，当前余额为:%g!", bcc.balance);
}
//转账
void transfer(Account *acc, char *buf)
{
	char src_path[256] = {}, dest_path[256];

	sprintf(src_path, "usr/%s", acc->bank);
	sprintf(dest_path, "usr/%s", acc->card);

	// 判断银行卡号是否正确
	if (0 != access(dest_path, F_OK))
	{
		sprintf(buf, "卡号不存在");
		return;
	}

	int src_fd = open(src_path, O_RDWR);
	int dest_fd = open(dest_path, O_RDWR);
	if (0 > src_fd || 0 > dest_fd)
	{
		error("open");
		sprintf(buf, "转账失败");
		return;
	}

	Account src_acc = {}, dest_acc = {};
	read(src_fd, &src_acc, sizeof(Account));
	read(dest_fd, &dest_acc, sizeof(Account));

	if (src_acc.balance < acc->balance)
	{
		sprintf(buf, "余额不足当前余额为:%g!", src_acc.balance);
		return;
	}

	src_acc.balance -= acc->balance;
	dest_acc.balance += acc->balance;

	lseek(src_fd, 0, SEEK_SET);
	lseek(dest_fd, 0, SEEK_SET);
	write(src_fd, &src_acc, sizeof(Account));
	write(dest_fd, &dest_acc, sizeof(Account));
	close(src_fd);
	close(dest_fd);

	sprintf(buf, "转账成功，当前余额为:%g!", src_acc.balance);
}

//改密码
void pwd(Account *acc, char *buf)
{
	char path[256] = {};
	sprintf(path, "usr/%s", acc->bank);

	int fd = open(path, O_WRONLY);
	if (0 > fd)
	{
		error("open");
		sprintf(buf, "失败!");
		return;
	}

	Account bcc = {};
	read(fd, &bcc, sizeof(Account));
	strcpy(bcc.pwd, acc->pwd);
	write(fd, &bcc, sizeof(Account));

	sprintf(buf, "修改密码成功!");
}
//查询
void query(Account *acc, char *buf)
{
	char path[256] = {};
	sprintf(path, "usr/%s", acc->bank);

	int fd = open(path, O_RDONLY);
	if (0 > fd)
	{
		error("open");
		sprintf(buf, "失败!");
		return;
	}

	Account bcc = {};
	read(fd, &bcc, sizeof(Account));
	close(fd);

	sprintf(buf, "当前余额为:%g!", bcc.balance);
}

int main()
{
	signal(SIGINT, sigint);
	//printf("创建socket对象...\n");
	svr_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > svr_fd)
	{
		perror("socket");
		return -1;
	}

	//准备通信地址(自己)
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8765);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	socklen_t addrlen = sizeof(addr);

	//绑定socket对象与地址
	if (bind(svr_fd, (struct sockaddr *)&addr, addrlen))
	{
		perror("bind");
		return -1;
	}

	//设置监听和排除数量
	if (listen(svr_fd, 10))
	{
		perror("listen");
		return -1;
	}
	for (;;)
	{
		int cli_fd = accept(svr_fd, (struct sockaddr *)&addr, &addrlen);
		if (0 > cli_fd)
		{
			perror("accept");
			return -1;
		}
		pthread_t tid;
		pthread_create(&tid, NULL, server, &cli_fd);
	}
}
