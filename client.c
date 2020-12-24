#include "tools.h"
#include <stdio.h>
#include "bank.h"
#define BUF_SIZE (4096)

void sec_menu(void);
void save(void);
void take(void);
void query(void);
void transfer(void);
void pwd(void);
Account acc = {1, "1", "1", "1", 1, 0};
int cli_fd;
// 主菜单
void main_menu(void)
{
	system("clear");
	puts("1、  开户\n");
	puts("2、  登陆\n");
	puts("3、  销户\n");
	puts("4、  解锁\n");
	puts("0、  退出\n");
}
void open_acc(void)
{
	acc.type = M_OPEN;
	printf("请输入身份证号：");
	get_str(acc.card, 10);
	// printf("请输入开户金额：");
	// scanf("%f", &acc.balance);
}
// 登陆
void login(void)
{
	acc.type = M_LOGIN;
	printf("请输入银行卡号：");
	get_str(acc.bank, 20);
	printf("请输入密码：");
	get_pass(acc.pwd, 7, true);

	char msg[4096];
	sprintf(msg, "%d %s %s %s %f %d", acc.type, acc.bank, acc.card, acc.pwd, acc.balance, acc.lock);
	//发送数据
	int ret_size = write(cli_fd, msg, strlen(msg) + 1);

	//接收返回信息
	int recv_size = read(cli_fd, msg, sizeof(msg));
	//打印是否登录成功信息
	puts(msg);
	anykey_continue();
	if ('Y' != msg[0])
	{
		return;
	}
	for (;;)
	{
		sec_menu();
		switch (get_cmd('1', '6') - '0' + 4)
		{
		case M_SAVE:
			save();
			break;
		case M_TAKE:
			take();
			break;
		case M_QUERY:
			query();
			break;
		case M_TRANSFER:
			transfer();
			break;
		case M_REPASS:
			pwd();
			break;
		default:
			exit(EXIT_SUCCESS);
			break;
		}
		char buf[4096];
		sprintf(buf, "%d %s %s %s %f %d", acc.type, acc.bank, acc.card, acc.pwd, acc.balance, acc.lock);
		//发送数据
		write(cli_fd, buf, strlen(buf) + 1);
		//接收返回信息
		read(cli_fd, buf, sizeof(msg));
		// 显服务端执行的结果
		printf("%s\n", buf);
		anykey_continue();
	}
}

// 销户
void destory(void)
{
	acc.type = M_DESTORY;
	printf("请输入银行卡号：");
	get_str(acc.bank, 20);
	printf("请输入身份证号：");
	get_str(acc.card, 20);
	printf("请输入密码：");
	get_pass(acc.pwd, 7, true);
}

// 解锁
void unlock(void)
{
	acc.type = M_UNLOCK;
	printf("请输入银行卡号：");
	get_str(acc.bank, 20);
	printf("请输入身份证号：");
	get_str(acc.card, 20);
	printf("请输入密码：");
	get_pass(acc.pwd, 7, true);
}

// 子菜单
void sec_menu(void)
{
	system("clear");
	puts("1、存款		2、取款");
	puts("3、查询		4、转账");
	puts("5、改密		6、退出");
}

// 存款
void save(void)
{
	acc.type = M_SAVE;
	printf("请输入存款金额：");
	scanf("%f", &acc.balance);
}

// 取款
void take(void)
{
	acc.type = M_TAKE;
	printf("请输入取款金额：");
	scanf("%f", &acc.balance);
}

// 查询
void query(void)
{
	acc.type = M_QUERY;
}

// 转账
void transfer(void)
{
	acc.type = M_TRANSFER;
	printf("请输入目标银行卡号：");
	get_str(acc.card, 20);
	printf("请输入转账金额：");
	scanf("%f", &acc.balance);
}

// 改密
void pwd(void)
{
	char pass1[10] = {}, pass2[10] = {};
	printf("请输入新密码：");
	get_pass(pass1, 10, true);
	printf("请再次输入新密码：");
	get_pass(pass2, 10, true);
	if (strcmp(pass1, pass2))
	{
		puts("两次输入的密码不相同，修改失败!");
	}
	else
	{
		acc.type = M_REPASS;
		strcpy(acc.pwd, pass1);
	}
}
int main()
{
	// 创建socket
	cli_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > cli_fd)
	{
		perror("socket");
		return -1;
	}

	// 准备地址
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8765);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// 连接
	if (connect(cli_fd, (struct sockaddr *)&addr, sizeof(addr)))
	{
		perror("connect");
		return -1;
	}
	for (;;)
	{
		while (1)
		{
			main_menu();
			switch (get_cmd('0', '7') - '0')
			{
			case M_EXIT:
				return 0;
				break; // 退出
			case M_OPEN:
				open_acc();
				break; // 开户
			case M_LOGIN:
				login();
				continue; // 登陆
			case M_DESTORY:
				destory();
				break; // 销户
			case M_UNLOCK:
				unlock();
				break; // 解锁
			}
			char buf[4096];
			sprintf(buf, "%d %s %s %s %f %d", acc.type, acc.bank, acc.card, acc.pwd, acc.balance, acc.lock);
			//发送数据
			int ret_size = write(cli_fd, buf, strlen(buf) + 1);
			//接收返回信息
			int recv_size = read(cli_fd, buf, 4096);
			printf("%s\n", buf);
			anykey_continue();
		}
	}
}
