#include<stdio.h>
#include<windows.h>
#include<conio.h>
#include<time.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<string.h>

#define BEGIN 4
#define END 23
//打印函数 隐藏光标函数
#define Print() PrintPool(); PrintFrame()
#define HideCursor() CONSOLE_CURSOR_INFO cursor_info = {1, 0};SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info)

struct GAME_MESSAGE
{
    int score;
    int coordX;
    int coordY;
    int kind[2];
    int direction[2];
    uint16_t pool[END+2];
    uint16_t now;
    uint16_t next;
    int time;
    int rank;
    int color[20][10];
} message;

struct CONTROL_MESSAGE
{
    bool pause;
    bool left;
    bool right;
    bool rotate;
    bool gogogo;
} control;

uint16_t teris[7][4] =
{
    // kind O 0
    {0x0660,0x0660,0x0660,0x0660},
    // kind S 1
    {0x006C,0x0462,0x006C,0x0462},
    // kind Z 2
    {0x0063,0x0264,0x0063,0x0264},
    // kind L 3
    {0x0223,0x0074,0x0622,0x0170},
    // kind J 4
    {0x0226,0x0470,0x0322,0x0071},
    // kind I 5
    {0x00F0,0x2222,0x00F0,0x2222},
    // kind T 6
    {0x0072,0x0262,0x0270,0x0232},
};

int terisColor[7] = {8,9,10,11,12,13,14};
bool waigua = false;

void SetColor(unsigned short, unsigned short); //设置颜色
void GotoXy(int,int); //坐标定位

void GetNext(); //得到下一个方块
int GetBit(uint16_t, int); //得到位

void Delay();  //定时下落
void Deliver();//传递
bool DetectCrash();//检测碰撞
void DetectKey(); //检测键盘
void DetectClean();//检测消行
void Dead();//死掉了
void Init(); //初始化

void PrintNext(); //打印下一个方块
void PrintPool(); //打印游戏池
void PrintFrame();//打印边框
void PrintNumber();

void RotateTeris(); //旋转
void SlipTeris(); //水平移动
void RemoveTeris();//移除方块
void AddTeris();  //添加方块

void ChangeMessage(); //改变游戏数据
void Restart();		//重新开始游戏

int main()
{
    const char masterAccount[] = "tuanzhishu";
    const char sbAccount[] = "shenyaodi";
    char account[100];
    int i, j;
    int x,y;
    SetConsoleTitleA("一堆bug的俄罗斯方块");
    printf("please input the account\n");
    printf("account: ");
    gets(account);
    if (!strcmp(account, sbAccount))
    {
        printf("\n你太傻比了 游戏崩溃了\n");
        getch();
        exit(0);
    }
    if (!strcmp(account, masterAccount))
    {
        waigua = true;
    }

    system("mode con cols=50");
    system("mode con lines=23");

    HideCursor();
    Restart();
    return 0;
}

void GotoXy(int startX,int startY)	//左上角00 左下角y0 右上角x0 右下角xy
{
    HANDLE handle;
    COORD pos;
    pos.X = startX;
    pos.Y = startY;
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(handle,pos);
}

void SetColor(unsigned short ForeColor,unsigned short BackGroundColor)
{
    HANDLE handle=GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle,ForeColor+BackGroundColor*0x10);
}

int GetBit(uint16_t line,int i)	//得到line的第i位 最右边为第0位
{
    int flag = 1;
    flag &= line >> i;
    return flag;
}

void GetNext() //得到下一个
{
    int randKind;
    int randDirection;
    randKind = (int)rand()%7;
    randDirection = (int)rand()%4;
    message.kind[1] = randKind;
    message.direction[1] = randDirection;
    message.next = teris[randKind][randDirection];
}

void Deliver()	//传递
{
    message.now = message.next;
    message.kind[0] = message.kind[1];
    message.direction[0] = message.direction[1];
    GetNext();
}

void PrintNext()	//在屏幕上打印下一个方块
{
    int i,flag=3;
    GotoXy(29,2);   //定位

    //对于16位 得到位后打印小方块
    for (i=15; i>=0; i--)
    {
        if (GetBit(message.next,i))
        {
            printf("■");
        }
        else
        {
            printf("%2s","");
        }
        if (i%4 == 0)
        {
            GotoXy(29,flag++);
        }
    }
}

void PrintPool()	//打印游戏池内部情况
{
    int i,j;
    int line=0;
    GotoXy(2,line++);
    SetColor(14,0);
    for (i=BEGIN; i<=END; i++)
    {
        for (j=12; j>=3; j--)
        {
            if (GetBit(message.pool[i],j))
            {
                SetColor(message.color[i-BEGIN][12-j],0);
                printf("■");
            }
            else
            {
                printf("%2s","");
            }
            if (j == 3)
            {
                GotoXy(2,line++);
            }
        }
    }
}

void PrintFrame()	//打印框架
{
    //打印游戏池边界
    SetColor(15,0);
    int i;
    GotoXy(0,0);
    for(i=0; i<21; i++)
    {
        printf("■\n");
    }
    GotoXy(0,20);
    for(i=0; i<12; i++)
    {
        printf("■");
    }
    for(i=0; i<21; i++)
    {
        GotoXy(22,i);
        printf("■");
    }

    //打印下一个方块
    SetColor(14,0);
    PrintNext();

    //打印辅助信息
    GotoXy(24,0);
    SetColor(10,0);
    printf("%4s----NEXT----","");
    GotoXy(24,10);
    printf("%4sSCORE: %d     ","",message.score);
    GotoXy(24,12);
    printf("   ● 当前等级: %d    ",message.rank);
    GotoXy(24,13);
    printf("   ● W 旋转");
    GotoXy(24,14);
    printf("   ● S 下落");
    GotoXy(24,15);
    printf("   ● A 向左平移");
    GotoXy(24,16);
    printf("   ● D 向右平移");
    GotoXy(24,17);
    printf("   ● P 暂停");
    GotoXy(24,18);
    printf("   ● + 或 - 控制速度");
    GotoXy(24,20);
    SetColor(9,0);
    printf("%3s注意切换输入法","");
    GotoXy(24,22);
    SetColor(8,0);
    printf("   Design by Whosecube");
}

void Init()		//初始化
{
    int i,j;
    srand((int)time(NULL));

    message.score = 0;	 //初始化分数
    message.time = 35;
    message.rank = 0;
    for (i=0; i<=END; i++)  //初始化游戏池
    {
        message.pool[i] = 0xE007;
    }
    message.pool[END+1] = 0xFFFF;

    //初始化成白色
    for (i=0; i<20; i++)
    {
        for (j=0; j<10; j++)
        {
            message.color[i][j] = 7;
        }
    }

    //初始化now 和 next
    GetNext();
    Deliver();

    //坐标定到缓冲区
    message.coordX = 3;
    message.coordY = -4;

    AddTeris();	//放到缓冲区

    //初始化control
    control.pause = false;
    control.left = false;
    control.right = false;
    control.rotate = false;
    control.gogogo = false;
}

void DetectKey()    //检测键盘
{
    char key;
    if (kbhit())
    {
        key = getch();
        if (!control.pause)
        {
            switch(key)
            {
            case 'w':
            case 'W':
                control.rotate = true;
                break;
            case 'a':
            case 'A':
                control.left = true;
                break;
            case 'd':
            case 'D':
                control.right = true;
                break;
            case 'p':
            case 'P':
                control.pause = true;
                break;
            case 's':
            case 'S':
                control.gogogo = true;
                break;
            case '-':
            case '_':
                message.time += 5;
                message.rank--;
                break;
            case '=':
            case '+':
                if (message.time >= 5)
                {
                    message.time -= 5;
                    message.rank++;
                    break;
                }
            }
        }
    }
}

void DetectClean()  //检测消行
{
    int temp[END+2];
    int cnt = 0;
    int i, j, k;
    int line = 0;

    //temp临时存储message.pool
    for (i=END; i>=BEGIN; i--)
    {
        temp[i] = message.pool[i];
    }

    //寻找满足消行的，当前行以上全部向下移动
    for (i=END; i>=BEGIN; i--)
    {
        if (message.pool[i] == 0xFFFF)
        {
            cnt++;
            message.pool[i] = 0xE007;
            for (j=i; j>BEGIN; j--)
            {
                message.pool[j] = message.pool[j-1];
            }
            message.pool[BEGIN] = 0xE007;  //BEGIN行重置为E007

            //重新配置颜色
            for (j=i-BEGIN; j>0; j--)
            {
                for (k=0; k<10; k++)
                {
                    message.color[j][k] = message.color[j-1][k];
                }
            }

            i++;
        }
    }

    //打印消行效果
    if (cnt)
    {
        GotoXy(2,line++);
        for (i=BEGIN; i<=END; i++)
        {
            if (temp[i] == 0xFFFF)
            {
                SetColor(15,0);
            }
            else
            {
                SetColor(7,0);
            }
            for (j=12; j>=3; j--)
            {

                if (GetBit(temp[i],j))
                {
                    printf("■");
                }
                else
                {
                    printf("%2s","");
                }
                if (j == 3)
                {
                    GotoXy(2,line++);
                }
            }
        }
        Sleep(400);
    }

    //加分
    switch(cnt)
    {
    case 1:
        message.score += 100;
        break;
    case 2:
        message.score += 200;
        break;
    case 3:
        message.score += 400;
        break;
    case 4:
        message.score += 800;
        break;
    }
}

bool DetectCrash()  //检测'此时（xy）'是否重叠！ ture碰撞
{
    int line, column;
    int x = message.coordX;
    int y = message.coordY;
    uint16_t NOW = message.now;

    if(x+3<0 || x>9 || y>END)
    {
        return true;
    }

    //和游戏池中的数据按位与 结果为1说明碰撞
    for (line=0; line<4; line++)
    {
        for (column=0; column<4; column++)
        {
            if (GetBit(message.pool[y+4+line],12-x-column) & GetBit(NOW,15-(line*4+column)) )
            {
                return true;
            }
        }
    }
    return false;
}

void Dead()     //死掉了
{
    char key;

    SetColor(14,0);
    GotoXy(9,10);
    printf("你GG了");

    GotoXy(0,22);
    printf("是否重新开始游戏Y/N");

    while (key = getch())
    {
        if (key == 'Y' || key == 'y')
        {
            GotoXy(0,22);
            printf("                        ");
            Restart();
        }
        else if (key == 'N' || key == 'n')
        {
            exit(0);
        }
    }
}

void ChangeMessage()	//根据control 改变 message
{
    if (control.pause)
    {
        SetColor(14,0);
        GotoXy(0,22);
        printf("按任意键继续");
        getch();
        control.pause = false;
        GotoXy(0,22);
        printf("                                      ");
    }

    if (control.gogogo)
    {
        while (1)
        {
            RemoveTeris();
            message.coordY++;
            if (DetectCrash())
            {
                message.coordY--;
                AddTeris();
                break;
            }
            else
            {
                AddTeris();
            }
        }
    }
    SlipTeris();
    if (control.rotate)
    {
        RotateTeris();
    }
    control.left = false;
    control.right = false;
    control.rotate = false;
    control.gogogo = false;
}

void Delay()
{
    int i,j;
    if (!waigua)
    {
        if (message.rank >=0 && message.score / (1000 * (message.rank+1)) && message.time >= 0)
        {
            message.rank++;
            message.time -= 5;
        }
    }

    for (i=0; i<message.time; i++)
    {
        DetectKey();
        ChangeMessage();
        Print();
        // PrintNumber();
    }
}

void AddTeris()		//根据坐标 添加（临时锁定）到游戏池
{
    uint16_t temp = message.now;
    int x = message.coordX;
    int y = message.coordY;
    int i,j;

    //通过 按位或 锁定到游戏池
    //第一行
    message.pool[y+4] |=  (((temp >> 12) & 0x000f) << (9-x));
    //第二行
    message.pool[y+5] |=  (((temp >>  8) & 0x000f) << (9-x));
    //第三行
    message.pool[y+6] |=  (((temp >>  4) & 0x000f) << (9-x));
    //第四行
    message.pool[y+7] |=  (((temp >>  0) & 0x000f) << (9-x));

    for (i=0; i<4; i++)
    {
        for (j=0; j<4; j++)
        {
            if (GetBit(temp << i*4 >> 12, 3-j))
            {
                if (y+i >= 0 && x+j >= 0)
                {
                    message.color[y+i][x+j] = terisColor[message.kind[0]];
                }
            }
        }
    }
}

void RemoveTeris()	//根据坐标 移除游戏池
{
    uint16_t temp = message.now;
    int x = message.coordX;
    int y = message.coordY;
    message.pool[y + 4] &=  ~(((temp >> 12) & 0x000f) << (9-x));
    message.pool[y + 5] &=  ~(((temp >> 8) & 0x000f) << (9-x));
    message.pool[y + 6] &=  ~(((temp >> 4) & 0x000f) << (9-x));
    message.pool[y + 7] &=  ~(((temp >> 0) & 0x000f) << (9-x));
}

void SlipTeris()	//平移 更新‘方块的左上角’xy坐标 add
{
    RemoveTeris();	//已经remove掉 可根据xy直接检测碰撞
    if(control.right && !control.left)
    {
        message.coordX++;
        if (DetectCrash())
        {
            message.coordX--;
        }
    }
    else if (control.left && !control.right)
    {
        message.coordX--;
        if (DetectCrash())
        {
            message.coordX++;
        }
    }
    AddTeris();
}

void RotateTeris()		//旋转 更新当前方向数组 add
{
    uint16_t temp = message.direction[0];	//得到当前角度
    RemoveTeris();

    if (message.direction[0] == 3)
    {
        message.direction[0] = 0;
    }
    else
    {
        message.direction[0]++;
    }
    message.now = teris[message.kind[0]][message.direction[0]];

    if (DetectCrash())	//旋转后碰撞 复原
    {
        message.direction[0] = temp;
    }

    //给now赋值16进制 更新now
    message.now = teris[message.kind[0]][message.direction[0]];
    AddTeris();
}

void PrintNumber()
{
    GotoXy(0,0);
    int i,j;
    printf("\n");
    for (i=0; i<=END+1; i++)
    {
        for (j=15; j>=0; j--)
            printf("%d ",GetBit(message.pool[i],j));
        printf("\n");
    }
}

void Restart()		//重新开始游戏
{
    int i;

    Init();
    if (waigua)
    {
        message.score = 10000;
    }
    Print();
    while (1)
    {
        Delay();
        RemoveTeris();
        message.coordY++;
        if (DetectCrash())
        {
            message.coordY--;
            AddTeris();
            DetectClean();
            for (i=3; i>=0; i--)
            {
                if (message.pool[i] != 0xE007)
                {
                    Dead();
                }
            }
            Deliver();
            message.coordX = 3;
            message.coordY = -4;
            message.score += 10;
            Print();
        }
        else
        {
            AddTeris();
            Print();
        }
    }
}
