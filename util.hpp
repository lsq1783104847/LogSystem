#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace log_system
{
    namespace Util
    {
#define default_dir_mode 0775
        // 检查传入的文件路径path是否是个正确的路径,要求正确路径中不能有 "//"
        // 如果是正确路径(绝对路径，相对路径均可)就将其补充完善并返回，如果不是正确路径就返回空字符串
        std::string is_path(const std::string &path)
        {
            if (path.size() == 0 || path == "/")
                return path;
            size_t rpos = 0, lpos = 0;
            std::string ret;
            if (path[0] != '/')
                ret += "./";
            else
                lpos = 1;
            while (lpos < path.size())
            {
                rpos = path.find_first_of('/', lpos);
                if (rpos == lpos)
                    return "";
                if (rpos == std::string::npos)
                    break;
                lpos = rpos + 1;
            }
            ret += path;
            if (ret[ret.size() - 1] == '/')
                ret.pop_back();
            return ret;
        }
        // 检测文件路径path是否存在,不存在或路径错误都返回false
        bool file_exist(const std::string &path)
        {
            std::string ret = is_path(path);
            if (ret == "")
                return false;
            if (access(ret.c_str(), F_OK) == 0)
                return true;
            else
                return false;
        }
        // 根据传入的文件的path路径返回该文件所处的目录
        // 如果传入的路径有问题就返回空串,如果传入的是根目录则也返回根目录
        std::string file_dir(const std::string &path)
        {
            std::string ret = is_path(path);
            if (ret == "")
                return ret;
            size_t pos = ret.find_last_of('/'); // 经过is_path()的调用，ret中一定有'/',且除了根目录ret不以'/'结尾
            return ret.substr(0, pos + 1);
        }
        // 根据path路径创建目录，如果已经存在就直接返回真
        bool create_dir(const std::string &path)
        {
            std::string ret = is_path(path);
            if (ret == "")
                return false;
            if (file_exist(ret) == true)
                return true;

            size_t lpos = 0, rpos = 0;
            while (lpos < ret.size())
            {
                rpos = ret.find_first_of('/', lpos);
                lpos = rpos == std::string::npos ? rpos : rpos + 1;
                std::string sstr = ret.substr(0, lpos);
                if (file_exist(sstr) == true)
                    continue;
                else
                {
                    if (mkdir(sstr.c_str(), default_dir_mode) == -1)
                        return false;
                }
            }
            return true;
        }
        // 根据路径返回文件名，如果path为根目录或者路径有问题就返回空串
        std::string get_fname(const std::string &path)
        {
            std::string ret = is_path(path);
            if (ret == "" || ret == "/")
                return "";
            if (ret[ret.size() - 1] == '/')
                ret.pop_back();
            size_t pos = ret.find_last_of('/');
            return ret.substr(pos + 1);
        }
        // 传入相对路径，返回绝对路径，要求传入的相对路径是正确的，如果发生错误则返回空串
        // 如果传入的就是绝对路径，那么就不做转化返回该路径,且返回的绝对路径中不含"."和".."
        std::string path_transform(const std::string &path)
        {
            std::string ret = is_path(path);
            if (ret == "")
                return "";

            std::string absolute_path = "/";
            size_t lpos = 1, rpos = 1;

            if (ret[0] != '/')
            {
                char *p = getcwd(NULL, 0);
                if (p == nullptr)
                    return "";
                absolute_path = p;
                free(p);
                lpos = 0, rpos = 0;
            }

            while (lpos < ret.size())
            {
                rpos = ret.find_first_of('/', lpos);
                std::string sstr = ret.substr(lpos, (rpos == std::string::npos ? rpos : rpos - lpos));
                lpos = (rpos == std::string::npos ? rpos : rpos + 1);
                if (sstr == ".")
                    continue;
                else if (sstr == "..")
                    absolute_path = file_dir(absolute_path);
                else
                {
                    if (absolute_path[absolute_path.size() - 1] != '/')
                        absolute_path.push_back('/');
                    absolute_path += sstr;
                }
            }

            if (absolute_path[absolute_path.size() - 1] == '/' && absolute_path != "/")
                absolute_path.pop_back();
            return absolute_path;
        }
    }
}

#endif