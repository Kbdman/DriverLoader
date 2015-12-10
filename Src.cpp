#include <stdio.h>
#include <Windows.h>
#include <string.h>
#include <string>
using namespace std;
const char* getDriverName(const char* filepath)
{
	const char* res=strrchr(filepath,'\\');
	if (res == NULL)
		return filepath;
	return res+1;
}
string MakePathName(const char * filePath)
{
	const char* res = strrchr(filePath, '\\');
	if (res == NULL)
	{
		int bufsize=GetCurrentDirectory(0, NULL);
		char* pbuf = (char*)calloc(bufsize+10, 1);
		if (pbuf == NULL)
		{
			printf("Alloc memory for buf failed\n");
			exit(0);
		}
		GetCurrentDirectory(bufsize, pbuf);
		printf("Get Work Directory:%s...\n",pbuf);
		string res=pbuf;
		res += "\\";
		res += filePath;
		free(pbuf);
		return res;
	}
	return filePath;
	
}
SC_HANDLE CreateServ(SC_HANDLE scmhandle, const char * ServiceName, const char* DriverName)
{
	if (fopen(DriverName, "r") == NULL)
	{
		printf("Cannot find file:%s\n", DriverName);
		return NULL;
	}
	return CreateService(scmhandle, ServiceName, ServiceName, SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,SERVICE_ERROR_NORMAL,
		DriverName, NULL, NULL, NULL, NULL, NULL);
}
int main(int argc,char** argv)
{
	
	if (argc < 2)
	{
		printf("Command Not Correct!\n");
		return 0;
	}
	SC_HANDLE schandle= OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schandle == NULL)
	{
		printf("open SCManager Error!  ErrCode: %d\n",GetLastError());
		return 0;
	}

	const char* DriverName = getDriverName(argv[1]);
	string pathName = MakePathName(argv[1]);
	printf("filePath=%s\n",pathName.c_str());
	SC_HANDLE hSer = CreateServ(schandle,DriverName,pathName.c_str());

	if (hSer==NULL)
	{
		DWORD ErrorCode = GetLastError();
		if (ErrorCode != ERROR_SERVICE_EXISTS)
		{
			printf("Create Service Error!  ErrCode: %d\n", ErrorCode);
			if (ErrorCode == 123)
				printf("%s\n", pathName.c_str());
			return 0;
		}
		printf("Service with same name Exsits,try to delete it \n");
		SC_HANDLE  old_Service =OpenService(schandle,DriverName,DELETE|SERVICE_STOP);
		if (old_Service == NULL)
		{
			printf("Open Service Failed! ErrCode:%d\n",GetLastError());
			return 0;
		}
		SERVICE_STATUS sta = {0};
		if (!ControlService(old_Service, SERVICE_CONTROL_STOP, &sta))
		{
			printf("Try stop service,but failed.\n");
		}
		if (!DeleteService(old_Service))
		{
			printf("Delete Service Failed! ErrCode:%d\n", GetLastError());
			return 0;
		}
		CloseServiceHandle(old_Service);
		hSer = CreateServ(schandle, DriverName, pathName.c_str());
		if (hSer == NULL)
		{
			printf("Create Service Error!  ErrCode: %d\n", GetLastError());
			return 0;
		}
	}
	CloseServiceHandle(hSer);
	printf("Create Success\n");
	hSer = OpenService(schandle, DriverName, SERVER_ALL_ACCESS|SERVICE_START);
	if (hSer == NULL)
	{
		printf("Open Service Failed! ErrCode:%d\n", GetLastError());
		return 0;
	}
	if (StartService(hSer, NULL, NULL) == NULL)
		printf("Start Service Failed!\nErrCode:%d\n",GetLastError());

}