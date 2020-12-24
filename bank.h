#ifndef BANK_H
#define BANK_H

#define M_EXIT		0 // 退出
#define M_OPEN 		1 // 开户
#define M_LOGIN 	2 // 登陆
#define M_DESTORY 	3 // 销户
#define M_UNLOCK	4 // 解锁
#define M_SAVE 		5 // 存款
#define M_TAKE 		6 // 取款
#define M_QUERY 	7 // 查询
#define M_TRANSFER 	8 // 转账
#define M_REPASS	9 // 改密 

typedef struct Account
{
	int 	type;	//用户选择什么操作
	char 	bank[20];	// 银行卡号
	char 	card[20];	// 身份证号
	char 	pwd[20];	// 密码
	float 	balance;	// 余额
	int	lock;		// 锁定
}Account;

#endif//BANK_H
