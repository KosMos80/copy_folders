#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <signal.h> 

#include "copy_file.h"

int64_t max_nom_buf = 0;
int64_t now_nom_buf = 0;

int proces_copy = 0;

typedef void(*fp)(void);

#define INTERVAL 1000 //00  // интервал таймера  -- 100000 us - 100 ms
struct itimerval tout_val;

static void timerevent(void)
	{  //выполняемая процyдера прерываний
     setitimer(ITIMER_REAL, &tout_val,0);
	// тело 	
	if(proces_copy)
		{
			int progres = now_nom_buf * 100 / max_nom_buf;
			std::cout << "\r";
			std::cout << progres << "% ";
			for(int i = 0; i < progres; i++) std::cout << "*";
			if(proces_copy == 2) proces_copy = 0;
		}
    }
	
void settimer() { // настройка времени таймера
    tout_val.it_interval.tv_sec = 0; 
    tout_val.it_interval.tv_usec = 0; 
    tout_val.it_value.tv_sec = 0;  
    tout_val.it_value.tv_usec = INTERVAL; 
    setitimer(ITIMER_REAL, &tout_val,0);
    }

void registertimer() 
	{ 
    fp ptr = &timerevent; 
    signal(SIGALRM,(void(*)(int))ptr); 
    /* set the Alarm signal capture */
     }
//////////////////////////////////////////////////

std::string deldotslash(char *var)
{
std::string res = "";
	int i = 2;
	while(var[i] != 0)
		{
			i++;
		}
	res.append(var, 2, i-3);
	return res;
}

void send_comm(const char *parent_dir, const char *buffrom)
{
	const char *comm = "find";
	char var[128];
	FILE *fp;
	std::string path;
	int64_t allsize = 0;
	std::string nm;
	fp = popen(comm, "r");
	int fl = 0; 
	int dr = 0;
	// count files, folders and AllSize
	std::cout << "Count files, folders and AllSize" << std::endl;
	while(fgets(var, sizeof(var) - 1, fp) != 0)
		{
		path = deldotslash(var);
		//std::cout << path << std::endl;
		int64_t fsize = sizefile(path);
		if(fsize < 2147483647)
			{
			allsize = allsize + fsize;
			fl++;
			}
			else
			{
			dr++;
			}
		}
		std::cout << std::endl;
		std::cout << " All size files: " << allsize << std::endl;
		std::cout << " Total count files and dir: " << fl + dr << std::endl;
		max_nom_buf = allsize; // max limit for progress bar
	pclose(fp);
	
	std::cout << "Preparation for copying" << std::endl;
	
	struct path_type{ char path[128];};
	path_type *dirs = new path_type[dr+1];
	path_type *fls = new path_type[fl+1];
	
	fp = popen(comm, "r");
	dr = 0;
	fl = 0;
	while(fgets(var, sizeof(var) - 1, fp) != 0)
		{
		path = deldotslash(var);
		unsigned long int fsize = sizefile(path);
		if(fsize < 2147483647)
			{
				// create list of files
				strcpy(fls[fl].path, path.c_str());
				fl++;
			}
			else
			{
				// create list of folders
				strcpy(dirs[dr].path, path.c_str());
				dr++;
			}
		}
	pclose(fp);
	
	// Create dir tree 
	std::cout << "Folder tree creation" << std::endl;
	for(int ind = 0; ind < dr; ind++)
	{
		last_dir(dirs[ind].path);
		chdir(parent_dir);
	}
	delete[] dirs;

	std::cout << "Copying files ..."<< std::endl;	
	std::string from_path = buffrom;
	from_path.append("/");
	std::string to_path = "";
	to_path = char2string(parent_dir);
	to_path.append("/");
	
	now_nom_buf = 0;
	proces_copy = 1;
	
	// copy files from fls-array

	for(int ind = 0; ind < fl; ind++)
	{
		std::string src = from_path;
		std::string out = to_path;
		std::string file_path = char2string(fls[ind].path);
		src.append(file_path);
		out.append(file_path);
		copyfile(src.c_str(), out.c_str());
	}
	delete[] fls;
	proces_copy = 2;
}


int main(void){
const int PATH_MAX = 64;
char buffrom[PATH_MAX] = "";	
char buffer[PATH_MAX] = "";

if (getcwd(buffer, sizeof(buffer)) != NULL) 
	   {
		   std::cout << std::endl;
		   std::cout << "Current working directory (full path): "<< buffer << std::endl;
	   }
	
std::cout << std::endl;
std::cout << "Enter Source folder (full path):" << std::endl;
std::cin >> buffrom; std::cout << std::endl;
std::cout << "Enter Destination folder (full path):" << std::endl;
std::cin >> buffer; std::cout << std::endl;

if(chdir(buffrom) == 0)
	{
	settimer(); 
	registertimer(); // set interrupt for progress bar

	chdir("/");
	last_dir(buffer); // create destination folder

	chdir(buffrom); // change folder to source
	send_comm(buffer, buffrom); // copy folders

	while(proces_copy) {}; // wait while progress = 100%
	std::cout << std::endl;
	}
	else
	{
		// wrong source folder
	std::cout << " Incorrect source ("<< buffrom <<
	 ") Check folder and try again" << std::endl;
	}
return 0;
}
