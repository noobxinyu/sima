/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   filesystem.hpp
 * Author: xli
 *
 * Created on January 30, 2018, 3:43 PM
 */

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#include <boost/filesystem.hpp>

namespace utils
{
  namespace filesystem
  {

    inline void create_directory(const std::string& path)
    {
      boost::filesystem::path boost_path(path);
      if (!boost::filesystem::exists(boost_path))
      {
        boost::system::error_code ec;
        while (!boost::filesystem::create_directories(boost_path, ec))
        {
          sleep(1);
        }
      }
    }

    inline bool exist(const std::string& path)
    {
      boost::filesystem::path boost_path(path);
      return boost::filesystem::exists(boost_path) && boost::filesystem::file_size(boost_path);
    }

    inline std::string exec(std::stringstream& stream)
    {
      std::string result;
      FILE* pipe = popen(stream.str().c_str(), "r");
      if (pipe)
      {
        char buf[256];
        while (!feof(pipe))
        {
          if (fgets(buf, 256, pipe))
          {
            result += std::string(buf);
          }
        }
        return result;
      } else
      {
        std::cerr << "popen failed: " << stream.str() << std::endl;
        return "";
      }
    }

    template <typename... T>
    inline std::string exec(std::stringstream& stream, const std::string& argu, T... args)
    {
      stream << " " << argu;
      return exec(stream, args...);
    }

    template <typename... T>
    inline std::string exec(const std::string& cmd, T... args)
    {
      std::stringstream stream;
      stream << cmd;
      return exec(stream, args...);
    }

    inline bool endswith(const char* src, const char* suffix)
    {
      return 0 == strcmp(src + strlen(src) - strlen(suffix), suffix);
    }

    inline bool caseendswith(const char* src, const char* suffix)
    {
      return 0 == strcasecmp(src + strlen(src) - strlen(suffix), suffix);
    }

    class PipeFile
    {
    public:

      PipeFile() : fp_(NULL), isp_(true)
      {
      }

      PipeFile(const char* fn, bool write=false)
      {
        open(fn, write);
      }

      void open(const char* fn, bool write=false)
      {
        isp_ = true;
        fp_ = fopen(fn, write ? "w" : "r");
        if (!fp_) return;
        if (caseendswith(fn, ".zip"))
        {
          if (write)
            fp_ = popen((std::string("gzip >") + fn).c_str(), "w");
          else
            fp_ = popen((std::string("zcat ") + fn).c_str(), "r");
        } else if (caseendswith(fn, ".gz"))
        {
          if (write)
            fp_ = popen((std::string("gzip >") + fn).c_str(), "w");
          else
            fp_ = popen((std::string("gunzip -c ") + fn).c_str(), "r");
        } else if (caseendswith(fn, ".xz"))
        {
          if (write)
            fp_ = popen((std::string("xz >") + fn).c_str(), "w");
          else
            fp_ = popen((std::string("xz -d -c ") + fn).c_str(), "r");
        } else if (caseendswith(fn, ".zz"))
        {
          if (write)
            fp_ = popen((std::string("zstd >") + fn).c_str(), "w");
          else
            fp_ = popen((std::string("zstd -d -c --priority=rt ") + fn).c_str(), "r");
        } else
          isp_ = false;
      }

      operator bool()
      {
        return fp_ != NULL;
      }

      void close()
      {
        if (!fp_) return;
        if (isp_) pclose(fp_);
        else fclose(fp_);
        fp_ = NULL;
      }

      ~PipeFile()
      {
        close();
      }

      FILE* file()
      {
        return fp_;
      }

      int fd()
      {
        return fileno(fp_);
      }

      bool isp()
      {
        return isp_;
      }

    private:
      FILE* fp_;
      bool isp_;
    } ;
  }
}

#endif /* FILESYSTEM_HPP */

