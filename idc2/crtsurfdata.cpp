#include "_public.h"

// 全国气象站点参数结构体
struct st_stcode
{
  char provname[31];
  char obtid[11];
  char obtname[31];
  double lat;
  double lon;
  double height;
};

vector<struct st_stcode> vstcode;

bool LoadSTCode(const char *inifile);

struct  st_surfdata
{
  char obtid[11];
  char ddatetime[21];
  int t;
  int p;
  int u;
  int wd;
  int wf;
  int r;
  int vis;
};

vector<struct st_surfdata> vssurfdata;
char strddatetime[21]; // 观测数据的时间
// 模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中
void CrtSurfData();

// 把容器vsurfdata中的全国气象站点分钟观测数据写入文件
bool CrtSurfFile(const char *outpath,const char *datafmt);

CLogFile logfile; // 日志类

int main(int argc,char *argv[])
{
  if (argc!=5) 
  {
    // 如果参数非法，给出帮助文档。
    printf("Using:./crtsurfdata5 inifile outpath logfile datafmt\n");
    printf("Example:/project/idc1/bin/crtsurfdata5 /project/idc1/ini/stcode.ini /tmp/idc/surfdata /log/idc/crtsurfdata5.log xml,json,csv\n\n");

    printf("inifile 全国气象站点参数文件名。\n");
    printf("outpath 全国气象站点数据文件存放的目录。\n");
    printf("logfile 本程序运行的日志文件名。\n");
    printf("datafmt 生成数据文件的格式，支持xml、json和csv三种格式，中间用逗号分隔。\n\n");

    return -1;
  }

  // 打开程序的日志文件
  if(logfile.Open(argv[3],"a+",false)==false)
  {
    printf("logfile.Open(%s) failed.\n",argv[3]); return -1;
  }
  logfile.Write("crtsurfdata 开始运行")；

  if(LoadSTCode(argv[1])==false) return -1;

  // 模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中
  CrtSurfData();

  if(strstr(argv[4],"xml")!=0) CrtSurfFile(argv[2],"xml");
  if(strstr(argv[4],"json")!=0) CrtSurfFile(argv[2],"json");
  if(strstr(argv[4],"csv")!=0) CrtSurfFile(argv[2],"csv");

  logfile.WriteEx("crtsurfdata 运行结束");

  return 0;
}

// 把站点参数文件中加载到vstcode容器中。 
bool LoadSTCode(const char *inifile)
{
  CFile File;
  if(File.Open(inifile,"r")==false)
  {
    logfile.Write("File.Open(%s) failed.\n",inifile); return false;
  }

  char strBuffer[301];
  CCmdStr CmdStr;
  struct st_stcode stcode;
  
  while(true)
  {
    // 从站点参数文件中读取一行，如果已读取完毕，跳出循环
    if(File.Fgets(strBuffer,300,true)==false) break;
    // 把读取到的一行拆分
    CmdStr.SplitToCmd(strBuffer,",",true);
    if(CmdStr.CmdCount() != 6) continue;
    // 把站点参数的每个数据项保存到站点参数结构体中
    memset(&stcode,0,sizeof(struct st_stcode));
    CmdStr.GetValue(0, stcode.provname,30);
    CmdStr.GetValue(1, stcode.obtid,10);
    CmdStr.GetValue(2, stcode.obtname,30);
    CmdStr.GetValue(3, stcode.lat);
    CmdStr.GetValue(4, stcode.lon);
    CmdStr.GetValue(5, stcode.height);
    // 把站点参数据结构体放入站点参数容器中
    vstcode.push_back(stcode);
  }

  return true;
}

// 模拟生成全国气象站点分钟观测数据，存放在vsurfdata容器中。
void CrtSurfData()
{
  // 播随机数种子
  srand(time(0));
  // 获取当前时间，当作观测时间
  memset(strddatetime,0,sizeof(strddatetime));
  LocalTime(strddatetime,"yyyymmddhh24miss");
  struct st_surfdata stsurfdata;
  // 遍历气象站点参数的vstcode容器
  for (int ii=0;ii<vstcode.size();ii++)
  {
    memset(&stsurfdata,0,sizeof(struct st_surfdata));

    // 用随机数填充分钟观测数据的结构体
    strncpy(stsurfdata.obtid,vstcode[ii].obtid,10);
    strncpy(stsurfdata.ddatetime,strddatetime,14);
    stsurfdata.t = rand()%351;
    stsurfdata.p = rand()%265 + 10000;
    stsurfdata.u = rand()%100 + 1;
    stsurfdata.wd = rand()%360;
    stsurfdata.wf = rand()%150;
    stsurfdata.r  = rand()%16;
    stsurfdata.vis = rand()%5001 + 100000;

    // 把观测数据的结构体放入vsurfdata容器
    vssurfdata.push_back(stsurfdata);
  }
}

bool CrtSurfFile(const char *outpath,const char *datafmt)
{
  CFile File;

  // 拼接生成数据的文件名
  char strFileName[301];
  sprintf(strFileName,"%s/SURF_ZH_%s_%d.%s",outpath,strddatetime,getpid(),datafmt);
  // 打开文件
  if(File.OpenForRename(strFileName,"w")==false)
  {
    logfile.Write("File.OpenForRename(%s) failed.\n",strFileName);
  }

  if(strcmp(datafmt,"csv")==0) File.Fprintf("站点代码,数据时间,气温,气压,相对湿度,风向,风速,降雨量,能见度\n");
  if(strcmp(datafmt,"xml")==0) File.Fprintf("<data>\n");
  if(strcmp(datafmt,"json")==0) File.Fprintf("{\"data\":[\n");
  // 遍历存放观测数据的vsurfdata容器
  for(int ii=0;ii<vssurfdata.size();ii++)
  {

  }
  if(strcmp(datafmt,"xml")==0) File.Fprintf("</data>\n");
  if(strcmp(datafmt,"json")==0) File.Fprintf("]}\n");
  // 关闭文件
  File.CloseAndRename();

  logfile.Write("生成数据文件%s成功,数据时间%s,记录数%d.\n",strFileName,strddatetime,vssurfdata.size());
  return true;
}