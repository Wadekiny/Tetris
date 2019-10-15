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
//��ӡ���� ���ع�꺯��
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

void SetColor(unsigned short, unsigned short); //������ɫ
void GotoXy(int,int); //���궨λ

void GetNext(); //�õ���һ������
int GetBit(uint16_t, int); //�õ�λ

void Delay();  //��ʱ����
void Deliver();//����
bool DetectCrash();//�����ײ
void DetectKey(); //������
void DetectClean();//�������
void Dead();//������
void Init(); //��ʼ��

void PrintNext(); //��ӡ��һ������
void PrintPool(); //��ӡ��Ϸ��
void PrintFrame();//��ӡ�߿�
void PrintNumber();

void RotateTeris(); //��ת
void SlipTeris(); //ˮƽ�ƶ�
void RemoveTeris();//�Ƴ�����
void AddTeris();  //��ӷ���

void ChangeMessage(); //�ı���Ϸ����
void Restart();		//���¿�ʼ��Ϸ

int main()
{
    const char masterAccount[] = "tuanzhishu";
    const char sbAccount[] = "shenyaodi";
    char account[100];
    int i, j;
    int x,y;
    SetConsoleTitleA("һ��bug�Ķ���˹����");
    printf("please input the account\n");
    printf("account: ");
    gets(account);
    if (!strcmp(account, sbAccount))
    {
        printf("\n��̫ɵ���� ��Ϸ������\n");
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

void GotoXy(int startX,int startY)	//���Ͻ�00 ���½�y0 ���Ͻ�x0 ���½�xy
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

int GetBit(uint16_t line,int i)	//�õ�line�ĵ�iλ ���ұ�Ϊ��0λ
{
    int flag = 1;
    flag &= line >> i;
    return flag;
}

void GetNext() //�õ���һ��
{
    int randKind;
    int randDirection;
    randKind = (int)rand()%7;
    randDirection = (int)rand()%4;
    message.kind[1] = randKind;
    message.direction[1] = randDirection;
    message.next = teris[randKind][randDirection];
}

void Deliver()	//����
{
    message.now = message.next;
    message.kind[0] = message.kind[1];
    message.direction[0] = message.direction[1];
    GetNext();
}

void PrintNext()	//����Ļ�ϴ�ӡ��һ������
{
    int i,flag=3;
    GotoXy(29,2);   //��λ

    //����16λ �õ�λ���ӡС����
    for (i=15; i>=0; i--)
    {
        if (GetBit(message.next,i))
        {
            printf("��");
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

void PrintPool()	//��ӡ��Ϸ���ڲ����
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
                printf("��");
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

void PrintFrame()	//��ӡ���
{
    //��ӡ��Ϸ�ر߽�
    SetColor(15,0);
    int i;
    GotoXy(0,0);
    for(i=0; i<21; i++)
    {
        printf("��\n");
    }
    GotoXy(0,20);
    for(i=0; i<12; i++)
    {
        printf("��");
    }
    for(i=0; i<21; i++)
    {
        GotoXy(22,i);
        printf("��");
    }

    //��ӡ��һ������
    SetColor(14,0);
    PrintNext();

    //��ӡ������Ϣ
    GotoXy(24,0);
    SetColor(10,0);
    printf("%4s----NEXT----","");
    GotoXy(24,10);
    printf("%4sSCORE: %d     ","",message.score);
    GotoXy(24,12);
    printf("   �� ��ǰ�ȼ�: %d    ",message.rank);
    GotoXy(24,13);
    printf("   �� W ��ת");
    GotoXy(24,14);
    printf("   �� S ����");
    GotoXy(24,15);
    printf("   �� A ����ƽ��");
    GotoXy(24,16);
    printf("   �� D ����ƽ��");
    GotoXy(24,17);
    printf("   �� P ��ͣ");
    GotoXy(24,18);
    printf("   �� + �� - �����ٶ�");
    GotoXy(24,20);
    SetColor(9,0);
    printf("%3sע���л����뷨","");
    GotoXy(24,22);
    SetColor(8,0);
    printf("   Design by Whosecube");
}

void Init()		//��ʼ��
{
    int i,j;
    srand((int)time(NULL));

    message.score = 0;	 //��ʼ������
    message.time = 35;
    message.rank = 0;
    for (i=0; i<=END; i++)  //��ʼ����Ϸ��
    {
        message.pool[i] = 0xE007;
    }
    message.pool[END+1] = 0xFFFF;

    //��ʼ���ɰ�ɫ
    for (i=0; i<20; i++)
    {
        for (j=0; j<10; j++)
        {
            message.color[i][j] = 7;
        }
    }

    //��ʼ��now �� next
    GetNext();
    Deliver();

    //���궨��������
    message.coordX = 3;
    message.coordY = -4;

    AddTeris();	//�ŵ�������

    //��ʼ��control
    control.pause = false;
    control.left = false;
    control.right = false;
    control.rotate = false;
    control.gogogo = false;
}

void DetectKey()    //������
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

void DetectClean()  //�������
{
    int temp[END+2];
    int cnt = 0;
    int i, j, k;
    int line = 0;

    //temp��ʱ�洢message.pool
    for (i=END; i>=BEGIN; i--)
    {
        temp[i] = message.pool[i];
    }

    //Ѱ���������еģ���ǰ������ȫ�������ƶ�
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
            message.pool[BEGIN] = 0xE007;  //BEGIN������ΪE007

            //����������ɫ
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

    //��ӡ����Ч��
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
                    printf("��");
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

    //�ӷ�
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

bool DetectCrash()  //���'��ʱ��xy��'�Ƿ��ص��� ture��ײ
{
    int line, column;
    int x = message.coordX;
    int y = message.coordY;
    uint16_t NOW = message.now;

    if(x+3<0 || x>9 || y>END)
    {
        return true;
    }

    //����Ϸ���е����ݰ�λ�� ���Ϊ1˵����ײ
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

void Dead()     //������
{
    char key;

    SetColor(14,0);
    GotoXy(9,10);
    printf("��GG��");

    GotoXy(0,22);
    printf("�Ƿ����¿�ʼ��ϷY/N");

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

void ChangeMessage()	//����control �ı� message
{
    if (control.pause)
    {
        SetColor(14,0);
        GotoXy(0,22);
        printf("�����������");
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

void AddTeris()		//�������� ��ӣ���ʱ����������Ϸ��
{
    uint16_t temp = message.now;
    int x = message.coordX;
    int y = message.coordY;
    int i,j;

    //ͨ�� ��λ�� ��������Ϸ��
    //��һ��
    message.pool[y+4] |=  (((temp >> 12) & 0x000f) << (9-x));
    //�ڶ���
    message.pool[y+5] |=  (((temp >>  8) & 0x000f) << (9-x));
    //������
    message.pool[y+6] |=  (((temp >>  4) & 0x000f) << (9-x));
    //������
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

void RemoveTeris()	//�������� �Ƴ���Ϸ��
{
    uint16_t temp = message.now;
    int x = message.coordX;
    int y = message.coordY;
    message.pool[y + 4] &=  ~(((temp >> 12) & 0x000f) << (9-x));
    message.pool[y + 5] &=  ~(((temp >> 8) & 0x000f) << (9-x));
    message.pool[y + 6] &=  ~(((temp >> 4) & 0x000f) << (9-x));
    message.pool[y + 7] &=  ~(((temp >> 0) & 0x000f) << (9-x));
}

void SlipTeris()	//ƽ�� ���¡���������Ͻǡ�xy���� add
{
    RemoveTeris();	//�Ѿ�remove�� �ɸ���xyֱ�Ӽ����ײ
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

void RotateTeris()		//��ת ���µ�ǰ�������� add
{
    uint16_t temp = message.direction[0];	//�õ���ǰ�Ƕ�
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

    if (DetectCrash())	//��ת����ײ ��ԭ
    {
        message.direction[0] = temp;
    }

    //��now��ֵ16���� ����now
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

void Restart()		//���¿�ʼ��Ϸ
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
