#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>

extern int64_t max_nom_buf;
extern int64_t now_nom_buf;

std::string char2string(const char *var)
{
std::string res = "";
	int i = 2;
	while(var[i] != 0)
		{
			i++;
		}
	res.append(var, 0, i);
	return res;
}

int create_dir(const char *name_dir)
{
	if(mkdir(name_dir, 0777) == -1){
		//printf("\n Some problem in create dir: %s \n", name_dir);
	}
	else
	{
		// printf("\n Dir: %s  is creat \n", name_dir);
	}
return 0;
}

void last_dir(std::string path)
{
	std::string dir_name = "";
	std::string next_path = "";
	int ind = path.length();
	int len = 0;
	while((ind != 0)&&(path[ind] != '/'))
		{
			ind--;
			len++;
		}
	if(ind != 0) 
		{
			dir_name.append(path, ind+1, len-1);
			next_path.append(path, 0, ind);
			last_dir(next_path);
		}
		else
		{
			if(path[0] == '/')
				{
					dir_name.append(path, ind+1, len-1);
				}
				else
				{
					dir_name.append(path, ind, len);
				}
		}
	create_dir(dir_name.c_str());
	chdir(dir_name.c_str());				
}

int64_t sizefile(std::string src_file)
{
    // Имя исходного файла
	// std::string src_file = src;
    // Создание потока для работы с исходным файлом
    std::ifstream ifs(src_file.c_str(), std::ios::binary);
    // Подсчет размер исходного файла
    ifs.seekg(0, std::ios::end);
    //std::ios::pos_type 
    unsigned long int src_size = ifs.tellg();
	ifs.close();
	return src_size;
}
 
int copyfile(const char *src, const char *out)
{
    // Размер буфера 
    const int buf_size = 4096; //16384;
    // Имя исходного файла
    std::string src_file = src;
    std::string out_file = out;
    // Создание потока для работы с исходным файлом
    std::ifstream ifs(src_file.c_str(), std::ios::binary);
    // Подсчет размер исходного файла
    ifs.seekg(0, std::ios::end);
    std::ios::pos_type src_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    // Количество целых проходов (для частей файлы, которые полностью умещаются в буфер)
    size_t a_mount = src_size / buf_size;
    // Остаток (остаток файла)
    size_t b_mount = src_size % buf_size;

    size_t max_prog = a_mount + 1;
	size_t now_prog = 0;
    
    // Создание потока для файла-копии
    std::ofstream ofs(out_file.c_str(), std::ios::binary);
    // Это буфер
    char buf[buf_size];
	//std::cout << "Copy " << src << " to " << out << "\n";
    // Цикл по числу полных проходов
    for(size_t i = 0; i < a_mount; ++i)
    {
        ifs.read(buf, buf_size);
        ofs.write(buf, buf_size);
		now_nom_buf+=buf_size;
		now_prog++;
    }
    // Если есть остаток
    if(b_mount != 0)
    {
        ifs.read(buf, b_mount);
        ofs.write(buf, b_mount);
        now_nom_buf+=b_mount;
    }
    ifs.close();
    ofs.close();
    return 0;
}
