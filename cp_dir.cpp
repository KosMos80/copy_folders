#include <iostream>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <dirent.h> 

#include "copy_file.h"

int64_t max_nom_buf = 0;
int64_t now_nom_buf = 0;

volatile int d_count = 0;
volatile int f_count = 0;
volatile int max_length = 0;
volatile int64_t allsize = 0;


struct path_type { char path[250];};
path_type *dirs = new path_type[0];
path_type *fls = new path_type[0];

std::string path_from = "";

int proces_copy = 0;

typedef void(*fp)(void);

#define INTERVAL 1000  // интервал таймера  -- 1000 us - 1 ms
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

int first_dir(std::string path_now, int vetka = 0)
{
	std::string path_temp = path_from;
	path_temp.append(path_now);
	chdir(path_temp.c_str());
	DIR *dir = opendir(path_temp.c_str());
	//std::cout << path_now.c_str() << std::endl;
	if(dir)
		{
		dirent *ent;
		while((ent=readdir(dir))!=NULL)
		  {
	    if(strcmp("..",ent->d_name)&&strcmp(".",ent->d_name))
	        {
			  std::string new_path = path_now;
			  new_path.append("/");
			  new_path.append(ent->d_name);
			  int length = new_path.length();
			  if(max_length < length) max_length = length;
			  if(ent->d_type == 4) // eshe folder in folder
				{
					first_dir(new_path.c_str(), vetka);
				}
				else
				{
					//std::cout << " F " << new_path << std::endl;
					std::string path_temp_file = path_from;
					path_temp_file.append(new_path);
					int64_t fsize = sizefile(path_temp_file.c_str());
					allsize = allsize + fsize;
					if(vetka)
						{
							strcpy(fls[f_count].path, new_path.c_str());
							//std::cout << " F " << new_path << " " << fsize/1024 << "kB" << std::endl;
						}
					f_count++;
				}
		    }
		  }
		d_count++;  
		closedir(dir);
		}
		else 
		{
			std::cout << d_count << " " << " Warning! Folder not open : " << path_now << std::endl;
		}
	//std::cout << path_now << std::endl;
	if(vetka)
		{
			strcpy(dirs[d_count - 1].path, path_now.c_str());
			//std::cout << d_count << "-" << path_now << std::endl;
		}
}

void send_comm(const char *parent_dir, const char *buffrom)
{
	path_from = "";
	path_from.append(buffrom);
	std::string path;
	std::string nm;
	// count files, folders and AllSize
	std::cout << "Count files, folders and AllSize" << std::endl;
	d_count = 0;
	f_count = 0;
	first_dir("", 0);

	std::cout << std::endl;
	std::cout << " All size files: " << allsize << std::endl;
	std::cout << " Total count files and dir: " << f_count + d_count << std::endl;
	max_nom_buf = allsize; // max limit for progress bar
	
	dirs = new path_type[d_count];
	fls = new path_type[f_count];
	
	std::cout << "Preparation for copying" << std::endl;
	
	f_count = 0;
	d_count = 0;
	
	first_dir("", 1);

	// Create dir tree 
	std::cout << "Folder tree creation" << std::endl;
	for(int ind = 0; ind < d_count; ind++)
	{
		chdir(parent_dir);
		last_dir(dirs[ind].path);
	}
	delete[] dirs;
	
	std::string from_path = buffrom;
	from_path.append("/");
	std::string to_path = "";
	to_path = char2string(parent_dir);
	to_path.append("/");
	
	now_nom_buf = 0;
	proces_copy = 1;
	
	// copy files from fls-array
	std::cout << "Copying files ..."<< std::endl;
	for(int ind = 0; ind < f_count; ind++)
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
const int PATHMAX = 128;
char buffrom[PATHMAX] = "";	
char buffer[PATHMAX] = "";

if(getcwd(buffer, sizeof(buffer)) != NULL) 
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
