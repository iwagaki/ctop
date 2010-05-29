/*
  Copyright (c) 2009,2010 iwagaki@users.sourceforge.net

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <unistd.h>
#include <sys/times.h>
#include <dirent.h>
#include <algorithm>
#include <list>

#include "p.h"
#include "verify.h"
#include "FileDescriptor.h"
#include "OptionParserMain.h"


using namespace std;

bool& g_isPerformance = OptionParser::createOption(false, "performance", "Set the scaling governer to performance");
bool& g_isOndemand = OptionParser::createOption(false, "ondemand", "Set the scaling governer to ondemand");
bool& g_isDynamic = OptionParser::createOption(false, "dynamic", "Show 5 loaded processes");
const char*& g_pKeyword = OptionParser::createOption((const char*)0, "search", "Search <keyword> in running processes");

#define PROCESS_NAME_SIZE 20

struct StatTime
{
    StatTime() : user(0), nice(0), sys(0), idle(0)
    {
    }

    long long total() const
    {
        return user + nice + sys;
    }

    void sub(StatTime& target)
    {
        user -= target.user;
        nice -= target.nice;
        sys  -= target.sys;
        idle -= target.idle;
    }

    int pid;
    long long user;
    long long nice;
    long long sys;
    long long idle;
};


void getProcessName(char* pProcessName, int size, char* id)
{
    char fileName[256];
    char procName[256];

    {
        if (snprintf(fileName, 256, "/proc/%s/status", id) < 0)
            VERIFY(0);

        FileReader file(fileName);
        VERIFY(file != 0);

        if (fscanf(file, "Name: %255s",
                   procName // 1
                ) != 1)
            VERIFY(0);
    }

    if (strncmp(procName, "chrome", 256) == 0)
    {
        if (snprintf(fileName, 256, "/proc/%s/cmdline", id) < 0)
            VERIFY(0);

        FileReader file(fileName);
        VERIFY(file != 0);

        memset(procName, 0, 256);
        if (fread(procName, 1, 255, file) == 0)
            VERIFY(0);
    }

    int len = strlen(procName);

    if (size > len)
        size = len;
        
    strncpy(pProcessName, &procName[len - size], len + 1);
}



class ProcStat
{
public:
    virtual ~ProcStat() {}
    virtual void update() = 0;
    virtual void print() = 0;

    long long total() const
    {
        return m_usageTime.total();
    }

    const char* name() const
    {
        return m_processName;
    }

protected:
    virtual const char* getProcFileName() = 0;
    virtual bool readCurrentStat(StatTime& time, FILE* file) = 0;

    void updateStat()
    {
        StatTime currentTime;

        getCurrentStat(currentTime);
        m_usageTime = currentTime;
        m_usageTime.sub(m_lastTime);
        m_lastTime = currentTime;
    }

    void getCurrentStat(StatTime& time)
    { 
        FileReader file(getProcFileName());

        if(file.fp() != 0)
            if (readCurrentStat(time, file))
            {
//                abort();
            }
    }

    StatTime m_lastTime;
    StatTime m_usageTime;

    char m_processName[PROCESS_NAME_SIZE + 1];
    static long long m_progressTime;
};

long long ProcStat::m_progressTime = 0;

bool greater(const ProcStat* lhs, const ProcStat* rhs)
{
    return lhs->total() > rhs->total();
}


class ProcTotalStat : public ProcStat
{
public:
    ProcTotalStat()
    {
        getCurrentStat(m_lastTime);
    }

    // bool operator <(const StatTime& rhs) const
    // {
    //     return m_usageTime.total() < rhs.total();
    // }

    void update()
    {
        updateStat();
        m_progressTime = m_usageTime.total() + m_usageTime.idle;
    }

    void print()
    {
        if (m_progressTime <= 0)
            printf("ROLLBACK       ,  ");
        else if (m_usageTime.user < 0)
            printf("utime < 0      ,  ");
        else if (m_usageTime.nice < 0)
            printf("nice < 0       ,  ");
        else if (m_usageTime.sys < 0)
            printf("sys < 0        ,  ");
        else
        {
            printf("%3lld,%3lld,%3lld,%3lld,  ",
                   m_usageTime.total() * 100 / m_progressTime,
                   m_usageTime.user * 100 / m_progressTime,
                   m_usageTime.nice * 100 / m_progressTime,
                   m_usageTime.sys * 100 / m_progressTime);
        }
    }

private:
    const char* getProcFileName()
    {
        return "/proc/stat";
    }

    bool readCurrentStat(StatTime& time, FILE* file)
    {
        char dummy[255];

        return (fscanf(file, "%255s %lld %lld %lld %lld",
                       dummy, // 1
                       &time.user, // 2
                       &time.nice, // 3
                       &time.sys, // 4
                       &time.idle // 5
                    ) != 5);
    }
};


class ProcIdStat : public ProcStat
{
public:
    ProcIdStat(char* id)
    {
        getProcessName(m_processName, 20, id);

        if (snprintf(m_procFileName, PROC_FILE_NAME_SIZE, "/proc/%s/stat", id) < 0)
            VERIFY(0);

        getCurrentStat(m_lastTime);
    }

    void update()
    {
        updateStat();
    }

    void print()
    {
        if (m_progressTime <= 0)
            printf("ROLLBACK   ,  ");
        else if (m_usageTime.user < 0)
            printf("utime < 0  ,  ");
        else if (m_usageTime.sys < 0)
            printf("sys < 0    ,  ");
        else
        {
            printf("%4d,%3lld,%3lld,%3lld,  ",
                   m_usageTime.pid,
                   m_usageTime.total() * 100 / m_progressTime,
                   m_usageTime.user * 100 / m_progressTime,
                   m_usageTime.sys * 100 / m_progressTime);
        }
    }

private:
    const char* getProcFileName()
    {
        return m_procFileName;
    }

    bool readCurrentStat(StatTime& time, FILE* file)
    {
        long long dummy1;
        char dummy2[256];
    
        time.nice = 0;

        return (fscanf(file, "%d %255s %255s %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",
                       &time.pid, // 1
                       dummy2, // 2
                       dummy2, // 3
                       &dummy1, // 4
                       &dummy1, // 5
                       &dummy1, // 6
                       &dummy1, // 7
                       &dummy1, // 8
                       &dummy1, // 9
                       &dummy1, // 10
                       &dummy1, // 11
                       &dummy1, // 12
                       &dummy1, // 13
                       &time.user, // 14
                       &time.sys // 15
                    ) != 15);
    }

    static const int PROC_FILE_NAME_SIZE=256;
    char m_procFileName[PROC_FILE_NAME_SIZE];
};


int getCpuFreq()
{
    FileReader file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    int freq;

    if (fscanf(file, "%d", &freq) != 1)
        VERIFY(0);

    return freq;
}


void setDCVS(bool isMax)
{
    FileReader file("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");

    const char performance[] = "performance\n";
    const char ondemand[] = "ondemand\n";

    if (isMax)
    {
        if (fwrite(performance, 1, sizeof(performance), file) != sizeof(performance))
            VERIFY(0);
    }
    else
    {
        if (fwrite(ondemand, 1, sizeof(ondemand), file) != sizeof(ondemand))
            VERIFY(0);
    }
}


void searchProcess(const char* tagName)
{
    // ad-hoc

    DIR *pDir;
    struct dirent *pEntry;
//    char fileName[256];
    char processName[256];
    
    pDir = opendir("/proc/");
 
    while ((pEntry = readdir(pDir)) != 0)
    {
        if ((pEntry->d_type == DT_DIR) && (pEntry->d_name[0] >= '0') && (pEntry->d_name[0] <= '9'))
        {
            getProcessName(processName, 256, pEntry->d_name);

            // if (snprintf(fileName, 256, "/proc/%s/cmdline", pEntry->d_name) < 0)
            //     VERIFY(0);
            
            // FileReader file(fileName);

            // memset(processName, 0, 256);

            // if (fread(processName, 1, 255, file) != 0)
            {
                char* pStr = processName;

                while (strlen(pStr) >= strlen(tagName))
                {
                    if (strncmp(pStr, tagName, strlen(tagName)) == 0)
                    {
                        printf("%s %s\n", pEntry->d_name, processName);
                    }
                    pStr++;
                }
            }
        }
    }
 
    closedir(pDir);
}

void registerAllProcs(list<ProcStat*>& procList)
{
    // ad-hoc

    DIR *pDir;
    struct dirent *pEntry;

    pDir = opendir("/proc/");
    pid_t pid = getpid();

    while ((pEntry = readdir(pDir)) != 0)
    {
        if ((pEntry->d_type == DT_DIR) && (pEntry->d_name[0] >= '0') && (pEntry->d_name[0] <= '9'))
        {
            if (atoi(pEntry->d_name) != pid)
                procList.push_back(new ProcIdStat(pEntry->d_name));
        }
    }
 
    closedir(pDir);
}


int main(int argc, char** argv)
{
    OptionParser::parse(argc, argv);

    if (g_isOndemand || g_isPerformance)
    {
        VERIFY((g_isPerformance && g_isPerformance) == false)
        setDCVS(g_isPerformance);
        return 0;
    }

    if (g_pKeyword != 0)
    {
        searchProcess(g_pKeyword);
        return 0;
    }
    
    list <ProcStat*> procList;
    list <ProcStat*> dynamicProcList;

    procList.push_back(new ProcTotalStat);

    for (vector<char*>::iterator i = OptionParser::m_argumentList.begin(); i != OptionParser::m_argumentList.end(); ++i)
    {
        procList.push_back(new ProcIdStat(*i));
    }

    if (g_isDynamic)
        registerAllProcs(dynamicProcList);

    for (;;)
    {
        sleep(1);

        printf("%4d, ", getCpuFreq() / 1000);

        for (list<ProcStat*>::iterator i = procList.begin(); i != procList.end(); ++i)
            (*i)->update();

        for (list<ProcStat*>::iterator i = procList.begin(); i != procList.end(); ++i)
            (*i)->print();

        if (g_isDynamic)
        {
            for (list<ProcStat*>::iterator i = dynamicProcList.begin(); i != dynamicProcList.end(); ++i)
                (*i)->update();

            dynamicProcList.sort(greater);

            int j = 0;
            for (list<ProcStat*>::iterator i = dynamicProcList.begin(); i != dynamicProcList.end() && j < 5 ; ++i, j++)
            {
                if ((*i)->total() <= 0)
                    break;

                printf("%20s, ", (*i)->name());
                (*i)->print();
            }
        }

        printf("\n");
    }
}
